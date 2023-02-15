/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2008-2010, 2021 Philip Chimento <philip.chimento@gmail.com>
 */

#include "config.h"

#include <string.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

#include "document.h"
#include "searchbar.h"
#include "searchwindow.h"
#include "toast.h"

struct _I7SearchBar {
	GtkSearchBar parent;

	/* template children */
	GtkSearchEntry *entry;
	GtkButton *find_next;
	GtkButton *find_previous;
	GtkCheckButton *ignore_case;
	GtkButton *replace;
	GtkButton *replace_all;
	GtkBox *replace_box;
	GtkEntry *replace_entry;
	GtkToggleButton *replace_mode_button;
	GtkCheckButton *restrict_search;
	GtkLabel *search_label;
	GtkBox *search_options_box;
	GtkToggleButton *search_options_button;
	GtkComboBoxText *search_type;

	/* private */
	GtkWidget *searched_view;  /* ref, released on dispose */
	unsigned long webview_found_handler;
	unsigned long webview_not_found_handler;
};

G_DEFINE_TYPE(I7SearchBar, i7_search_bar, GTK_TYPE_SEARCH_BAR);

enum {
	MAYBE_SHOW_ENTIRE_SOURCE,
	LAST_SIGNAL
};

static unsigned i7_search_bar_signals[LAST_SIGNAL] = { 0 };

static void
set_can_find(I7SearchBar *self, bool can_find)
{
	gtk_widget_set_sensitive(GTK_WIDGET(self->find_previous), can_find);
	gtk_widget_set_sensitive(GTK_WIDGET(self->find_next), can_find);
	gtk_widget_set_sensitive(GTK_WIDGET(self->replace), can_find);
	gtk_widget_set_sensitive(GTK_WIDGET(self->replace_all), can_find);
}

static void
set_not_found(I7SearchBar *self, bool not_found)
{
	set_can_find(self, !not_found);

	GtkStyleContext *style = gtk_widget_get_style_context(GTK_WIDGET(self->entry));
	if (not_found) {
		gtk_style_context_add_class(style, "error");
	} else {
		gtk_style_context_remove_class(style, "error");
	}
}

I7SearchFlags
get_search_flags(I7SearchBar *self)
{
	bool ignore_case = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->ignore_case));
	bool restrict_search = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->restrict_search));
	return gtk_combo_box_get_active(GTK_COMBO_BOX(self->search_type))
		| (ignore_case? I7_SEARCH_IGNORE_CASE : 0)
		| (restrict_search? I7_SEARCH_RESTRICT : 0);
}

/* THE "SEARCH ENGINE" */

/* The search functions are specialized for widget types.
 *
 * start_search() - start a new search, setting up any search controllers that
 *   are necessary. (GtkTextView search is stateless.) Then highlight the first
 *   match. Called when the search text is changed.
 * do_search() - find the next or previous match.
 * end_search() - done, unhighlight any matches. Called when search is closed.
 */

gboolean
find_no_wrap(const GtkTextIter *startpos, const char *text, gboolean forward, GtkTextSearchFlags flags,
    I7SearchFlags search_type, GtkTextIter *match_start, GtkTextIter *match_end)
{
	if(search_type == I7_SEARCH_CONTAINS)
		return forward?
			gtk_text_iter_forward_search(startpos, text, flags, match_start, match_end, NULL)
			: gtk_text_iter_backward_search(startpos, text, flags, match_start, match_end, NULL);

	GtkTextIter start, end, searchfrom = *startpos;
	while(forward?
		gtk_text_iter_forward_search(&searchfrom, text, flags, &start, &end, NULL)
		: gtk_text_iter_backward_search(&searchfrom, text, flags, &start, &end, NULL))
	{
		if(search_type == I7_SEARCH_FULL_WORD && gtk_text_iter_starts_word(&start) && gtk_text_iter_ends_word(&end)) {
			*match_start = start;
			*match_end = end;
			return TRUE;
		} else if(search_type == I7_SEARCH_STARTS_WORD && gtk_text_iter_starts_word(&start)) {
			*match_start = start;
			*match_end = end;
			return TRUE;
		}
		searchfrom = forward? end : start;
	}
	return FALSE;
}

static void
do_search_textview(I7SearchBar *self, I7SearchFlags flags)
{
	g_assert(GTK_IS_TEXT_VIEW(self->searched_view));

	const char *text = gtk_entry_get_text(GTK_ENTRY(self->entry));

	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(self->searched_view));
	GtkTextIter start, end, iter;

	bool ignore_case = flags & I7_SEARCH_IGNORE_CASE;
	bool forward = !(flags & I7_SEARCH_REVERSE);
	bool restrict_search = flags & I7_SEARCH_RESTRICT;
	I7SearchFlags search_type = flags & I7_SEARCH_ALGORITHM_MASK;
	GtkTextSearchFlags gtk_flags = GTK_TEXT_SEARCH_TEXT_ONLY
		| (ignore_case? GTK_TEXT_SEARCH_CASE_INSENSITIVE : 0)
		| (restrict_search? GTK_TEXT_SEARCH_VISIBLE_ONLY : 0);

	/* Start the search at the end or beginning of the selection */
	if(forward)
		gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_selection_bound(buffer));
	else
		gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));

	if (!find_no_wrap(&iter, text, forward, gtk_flags, search_type, &start, &end)) {
		/* Wrap around to the beginning or end */
		if(forward)
			gtk_text_buffer_get_start_iter(buffer, &iter);
		else
			gtk_text_buffer_get_end_iter(buffer, &iter);
		if (!find_no_wrap(&iter, text, forward, gtk_flags, search_type, &start, &end)) {
			set_not_found(self, true);
			return;
		}
	}
	set_not_found(self, false);

	/* We may have searched the invisible regions, so let the document decide
	 * whether to go back to showing the entire source if the text is
	 * invisible. */
	if (!(flags & I7_SEARCH_RESTRICT))
		g_signal_emit(self, i7_search_bar_signals[MAYBE_SHOW_ENTIRE_SOURCE], /* detail = */ 0, &start, &end);

	gtk_text_buffer_select_range(buffer, &start, &end);
	gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(self->searched_view), &start, 0.25, FALSE, 0.0, 0.0);
}

static void
do_search_webview(WebKitWebView *view, bool forward)
{
	WebKitFindController *controller = webkit_web_view_get_find_controller(view);
	if (forward)
		webkit_find_controller_search_next(controller);
	else
		webkit_find_controller_search_previous(controller);
}

static void
do_search(I7SearchBar *self, bool forward)
{
	if (GTK_IS_TEXT_VIEW(self->searched_view)) {
		I7SearchFlags flags = get_search_flags(self) | (forward? 0 : I7_SEARCH_REVERSE);
		do_search_textview(self, flags);
	} else if (WEBKIT_IS_WEB_VIEW(self->searched_view)) {
		do_search_webview(WEBKIT_WEB_VIEW(self->searched_view), forward);
	}
}

static void
end_search_textview(I7SearchBar *self)
{
	GtkTextView *view = GTK_TEXT_VIEW(self->searched_view);

	/* Remove selection */
	GtkTextIter start;
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(view);
	if (gtk_text_buffer_get_selection_bounds(buffer, &start, NULL))
		gtk_text_buffer_place_cursor(buffer, &start);
}

static void
end_search_webview(I7SearchBar *self)
{
	WebKitWebView *view = WEBKIT_WEB_VIEW(self->searched_view);

	WebKitFindController *controller = webkit_web_view_get_find_controller(view);
	webkit_find_controller_search_finish(controller);

	if (self->webview_found_handler != 0)
		g_signal_handler_disconnect(controller, self->webview_found_handler);
	self->webview_found_handler = 0;
	if (self->webview_not_found_handler != 0)
		g_signal_handler_disconnect(controller, self->webview_not_found_handler);
	self->webview_not_found_handler = 0;
}

static void
end_search(I7SearchBar *self)
{
	if (GTK_IS_TEXT_VIEW(self->searched_view))
		end_search_textview(self);
	else if (WEBKIT_IS_WEB_VIEW(self->searched_view))
		end_search_webview(self);
}

static void
start_search_textview(I7SearchBar *self, I7SearchFlags flags)
{
	do_search_textview(self, flags);
}

static void
on_webview_found_text(WebKitFindController *controller, unsigned count, I7SearchBar *self)
{
	set_not_found(self, false);
}

static void
on_webview_failed_to_find_text(WebKitFindController *controller, I7SearchBar *self)
{
	set_not_found(self, true);
}

static void
start_search_webview(I7SearchBar *self, const char *text, I7SearchFlags flags)
{
	WebKitFindController *controller = webkit_web_view_get_find_controller(WEBKIT_WEB_VIEW(self->searched_view));

	WebKitFindOptions webkit_flags = WEBKIT_FIND_OPTIONS_WRAP_AROUND |
		((flags & I7_SEARCH_IGNORE_CASE)? WEBKIT_FIND_OPTIONS_CASE_INSENSITIVE : 0) |
		((flags & I7_SEARCH_STARTS_WORD)? WEBKIT_FIND_OPTIONS_AT_WORD_STARTS : 0);

	self->webview_found_handler =
		g_signal_connect_object(controller, "found-text", G_CALLBACK(on_webview_found_text), self, /* flags = */ 0);
	self->webview_not_found_handler =
		g_signal_connect_object(controller, "failed-to-find-text", G_CALLBACK(on_webview_failed_to_find_text), self, /* flags = */ 0);

	webkit_find_controller_search(controller, text, webkit_flags, /* max matches = */ G_MAXUINT);
}

static void
start_search(I7SearchBar *self)
{
	const char *text = gtk_entry_get_text(GTK_ENTRY(self->entry));

	if (*text == '\0') {
		/* If the text is blank, unhighlight everything and return so the find
		 * entry doesn't stay red on a WebView */
		end_search(self);
		set_not_found(self, false);
		set_can_find(self, false);
		return;
	}

	I7SearchFlags flags = get_search_flags(self);

	if (GTK_IS_TEXT_VIEW(self->searched_view))
		start_search_textview(self, flags);
	else if (WEBKIT_IS_WEB_VIEW(self->searched_view))
		start_search_webview(self, text, flags);
}

static void
finish_activate(I7SearchBar *self)
{
	bool has_text = *gtk_entry_get_text(GTK_ENTRY(self->entry)) == '\0';
	set_can_find(self, has_text);

	gtk_search_bar_set_search_mode(GTK_SEARCH_BAR(self), TRUE);
	gtk_widget_grab_focus(GTK_WIDGET(self->entry));
}

/* CALLBACKS */

void
on_close_button_clicked(GtkButton *button, I7SearchBar *self)
{
	gtk_search_bar_set_search_mode(GTK_SEARCH_BAR(self), FALSE);
}

void
on_find_next(void *unused, I7SearchBar *self)
{
	do_search(self, /* forward = */ true);
}

void
on_find_previous(void *unused, I7SearchBar *self)
{
	do_search(self, /* forward = */ false);
}

void
on_replace_button_clicked(GtkButton *button, I7SearchBar *self)
{
	g_assert(GTK_IS_TEXT_VIEW(self->searched_view));

	const char *search_text = gtk_entry_get_text(GTK_ENTRY(self->entry));
	const char *replace_text = gtk_entry_get_text(self->replace_entry);
	I7SearchFlags flags = get_search_flags(self);
	GtkTextIter start, end;
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(self->searched_view));

	gtk_text_buffer_get_selection_bounds(buffer, &start, &end);
	gchar *selected = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

	/* if the text is already selected, then replace it, otherwise "find" again
	 to select the text */
	if ((flags & I7_SEARCH_IGNORE_CASE)? strcasecmp(selected, search_text) : strcmp(selected, search_text) == 0) {
		/* Replacing counts as one action for Undo */
		gtk_text_buffer_begin_user_action(buffer);
		gtk_text_buffer_delete(buffer, &start, &end);
		gtk_text_buffer_insert(buffer, &start, replace_text, -1);
		gtk_text_buffer_end_user_action(buffer);
	}
	g_free(selected);

	/* Find the next occurrence of the text */
	do_search_textview(self, flags);
}

void
on_replace_all_button_clicked(GtkButton *button, I7SearchBar *self)
{
	g_assert(GTK_IS_TEXT_VIEW(self->searched_view));

	const char *search_text = gtk_entry_get_text(GTK_ENTRY(self->entry));
	const char *replace_text = gtk_entry_get_text(self->replace_entry);
	I7SearchFlags flags = get_search_flags(self);;
	GtkTextIter cursor, start, end;
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(self->searched_view));
	GtkTextSearchFlags gtk_flags = GTK_TEXT_SEARCH_TEXT_ONLY
		| ((flags & I7_SEARCH_IGNORE_CASE)? GTK_TEXT_SEARCH_CASE_INSENSITIVE : 0)
		| ((flags & I7_SEARCH_RESTRICT)? GTK_TEXT_SEARCH_VISIBLE_ONLY : 0);
	I7SearchFlags search_type = flags & I7_SEARCH_ALGORITHM_MASK;

	/* Replace All counts as one action for Undo */
	gtk_text_buffer_begin_user_action(buffer);

	gtk_text_buffer_get_start_iter(buffer, &cursor);

	while (find_no_wrap(&cursor, search_text, /* forwards = */ TRUE, gtk_flags, search_type, &start, &end)) {
		/* delete preserves start and end iterators */
		gtk_text_buffer_delete(buffer, &start, &end);
		/* Save end position */
		GtkTextMark *tempmark = gtk_text_buffer_create_mark(buffer, NULL, &end, FALSE);
		gtk_text_buffer_insert(buffer, &start, replace_text, -1);
		/* Continue from end position, so as to avoid a loop if replace text
		 contains search text */
		gtk_text_buffer_get_iter_at_mark(buffer, &cursor, tempmark);
		gtk_text_buffer_delete_mark(buffer, tempmark);
	}

	gtk_text_buffer_end_user_action(buffer);
}

void
on_search_files_entry_changed(GtkEditable *editable, I7Document *self)
{
	const gchar *text = gtk_entry_get_text(GTK_ENTRY(editable));
	gboolean text_not_empty = !(text == NULL || strlen(text) == 0);
	gtk_widget_set_sensitive(self->search_files_find, text_not_empty);
}

void
on_search_files_find_clicked(GtkButton *button, I7Document *self)
{
	const char *text = gtk_entry_get_text(GTK_ENTRY(self->search_files_entry));
	gboolean ignore_case = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->search_files_ignore_case));
	I7SearchFlags flags = gtk_combo_box_get_active(GTK_COMBO_BOX(self->search_files_type))
			| (ignore_case? I7_SEARCH_IGNORE_CASE : 0);

	/* Close the dialog */
	gtk_widget_hide(self->search_files_dialog);

	GtkWidget *search_window = i7_search_window_new(self, text, flags);
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->search_files_project)))
		i7_search_window_search_project(I7_SEARCH_WINDOW(search_window));
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->search_files_extensions)))
		i7_search_window_search_extensions(I7_SEARCH_WINDOW(search_window));
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->search_files_documentation)))
		i7_search_window_search_documentation(I7_SEARCH_WINDOW(search_window));
	i7_search_window_done_searching(I7_SEARCH_WINDOW(search_window));
}

static void
on_webview_get_selection(WebKitWebView *view, GAsyncResult *result, I7SearchBar *self)
{
	g_autoptr(GError) err = NULL;
	g_autoptr(WebKitJavascriptResult) js_result = webkit_web_view_run_javascript_finish(view, result, &err);
	if (js_result == NULL) {
		g_warning("Failed to get selection in web page: %s", err->message);
		finish_activate(self);
		return;
	}

	JSCValue *js_value = webkit_javascript_result_get_js_value(js_result);
	JSCException *exception = jsc_context_get_exception(jsc_value_get_context(js_value));
	if (exception) {
		g_warning("Exception from getting selection in web page: %s", jsc_exception_get_message(exception));
		finish_activate(self);
		return;
	}

	if (!jsc_value_is_string(js_value)) {
		g_warning("Non-string value from getting selection in web page");
		finish_activate(self);
		return;
	}

	g_autofree char *selected_text = jsc_value_to_string(js_value);
	if (*selected_text != '\0')
		gtk_entry_set_text(GTK_ENTRY(self->entry), selected_text);

	finish_activate(self);
}

/* TYPE SYSTEM */

static void
i7_search_bar_init(I7SearchBar *self)
{
	gtk_widget_init_template(GTK_WIDGET(self));

	gtk_search_bar_connect_entry(GTK_SEARCH_BAR(self), GTK_ENTRY(self->entry));
	g_object_bind_property(self->replace_mode_button, "active",
		self->replace_entry, "visible",
		G_BINDING_SYNC_CREATE);
	g_object_bind_property(self->replace_mode_button, "active",
		self->replace_box, "visible",
		G_BINDING_SYNC_CREATE);
	g_object_bind_property(self->search_options_button, "active",
		self->search_options_box, "visible",
		G_BINDING_SYNC_CREATE);
}

static void
i7_search_bar_dispose(GObject *object)
{
	I7SearchBar *self = I7_SEARCH_BAR(object);

	g_clear_object(&self->searched_view);

	G_OBJECT_CLASS(i7_search_bar_parent_class)->dispose(object);
}

static void
i7_search_bar_class_init(I7SearchBarClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->dispose = i7_search_bar_dispose;

	i7_search_bar_signals[MAYBE_SHOW_ENTIRE_SOURCE] = g_signal_new("maybe-show-entire-source",
		G_OBJECT_CLASS_TYPE(klass), G_SIGNAL_NO_RECURSE,
		/* class_offset = */ 0, /* accumulator = */ NULL, NULL,
		/* marshaller = */ NULL, G_TYPE_NONE,
		2, GTK_TYPE_TEXT_ITER, GTK_TYPE_TEXT_ITER);

	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	gtk_widget_class_set_template_from_resource(widget_class, "/com/inform7/IDE/ui/searchbar.ui");
	gtk_widget_class_bind_template_child(widget_class, I7SearchBar, entry);
	gtk_widget_class_bind_template_child(widget_class, I7SearchBar, find_next);
	gtk_widget_class_bind_template_child(widget_class, I7SearchBar, find_previous);
	gtk_widget_class_bind_template_child(widget_class, I7SearchBar, ignore_case);
	gtk_widget_class_bind_template_child(widget_class, I7SearchBar, replace);
	gtk_widget_class_bind_template_child(widget_class, I7SearchBar, replace_all);
	gtk_widget_class_bind_template_child(widget_class, I7SearchBar, replace_box);
	gtk_widget_class_bind_template_child(widget_class, I7SearchBar, replace_entry);
	gtk_widget_class_bind_template_child(widget_class, I7SearchBar, replace_mode_button);
	gtk_widget_class_bind_template_child(widget_class, I7SearchBar, restrict_search);
	gtk_widget_class_bind_template_child(widget_class, I7SearchBar, search_label);
	gtk_widget_class_bind_template_child(widget_class, I7SearchBar, search_options_box);
	gtk_widget_class_bind_template_child(widget_class, I7SearchBar, search_options_button);
	gtk_widget_class_bind_template_child(widget_class, I7SearchBar, search_type);
	gtk_widget_class_bind_template_callback(widget_class, end_search);
	gtk_widget_class_bind_template_callback(widget_class, on_close_button_clicked);
	gtk_widget_class_bind_template_callback(widget_class, on_find_next);
	gtk_widget_class_bind_template_callback(widget_class, on_find_previous);
	gtk_widget_class_bind_template_callback(widget_class, on_replace_all_button_clicked);
	gtk_widget_class_bind_template_callback(widget_class, on_replace_button_clicked);
	gtk_widget_class_bind_template_callback(widget_class, start_search);
}

/* PUBLIC FUNCTIONS */

I7SearchBar *
i7_search_bar_new(void)
{
	return I7_SEARCH_BAR(g_object_new(I7_TYPE_SEARCH_BAR, NULL));
}

void
i7_search_bar_activate(I7SearchBar *self, bool replace_mode, bool can_restrict, GtkWidget *view, const char *descr)
{
	g_return_if_fail(GTK_IS_WIDGET(view));
	g_clear_object(&self->searched_view);
	self->searched_view = g_object_ref(view);

	bool can_replace = GTK_IS_TEXT_VIEW(view) && gtk_text_view_get_editable(GTK_TEXT_VIEW(view));
	gtk_widget_set_visible(GTK_WIDGET(self->replace_mode_button), can_replace);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->replace_mode_button), replace_mode);

	gtk_widget_set_visible(GTK_WIDGET(self->restrict_search), can_restrict);

	if (descr == NULL) {
		gtk_widget_hide(GTK_WIDGET(self->search_label));
	} else {
		g_autofree char *text = g_strdup_printf(_("Searching <b>%s</b>"), descr);
		gtk_label_set_markup(GTK_LABEL(self->search_label), text);
		gtk_widget_show(GTK_WIDGET(self->search_label));
	}

	if (GTK_IS_TEXT_VIEW(view)) {
		GtkTextIter start, end;
		GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
		if (gtk_text_buffer_get_selection_bounds(buffer, &start, &end)) {
			g_autofree char *selected_text = gtk_text_buffer_get_text(buffer, &start, &end, /* include hidden = */ FALSE);
			if (*selected_text != '\0')
				gtk_entry_set_text(GTK_ENTRY(self->entry), selected_text);
		}
	} else if (WEBKIT_IS_WEB_VIEW(view)) {
		webkit_web_view_run_javascript(WEBKIT_WEB_VIEW(view),
			"window.getSelection().toString()", /* cancellable = */ NULL,
			(GAsyncReadyCallback)on_webview_get_selection, self);
		return;
	}

	finish_activate(self);
}
