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
	I7Document *document;  /* ref, released on dispose */
};

G_DEFINE_TYPE(I7SearchBar, i7_search_bar, GTK_TYPE_SEARCH_BAR);

enum  {
	PROP_0,
	PROP_DOCUMENT,
};

/* THE "SEARCH ENGINE" */

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

static gboolean
find(GtkTextBuffer *buffer, const char *text, I7SearchFlags flags, GtkTextIter *match_start, GtkTextIter *match_end)
{
	GtkTextIter iter;
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

	if(!find_no_wrap(&iter, text, forward, gtk_flags, search_type, match_start, match_end))
	{
		/* Wrap around to the beginning or end */
		if(forward)
			gtk_text_buffer_get_start_iter(buffer, &iter);
		else
			gtk_text_buffer_get_end_iter(buffer, &iter);
		if(!find_no_wrap(&iter, text, forward, gtk_flags, search_type, match_start, match_end))
			return FALSE;
	}
	return TRUE;
}

/* CALLBACKS */

void
on_close_button_clicked(GtkButton *button, I7SearchBar *self)
{
	gtk_search_bar_set_search_mode(GTK_SEARCH_BAR(self), FALSE);
}

void
on_findbar_entry_search_changed(GtkSearchEntry *entry, I7SearchBar *self)
{
	i7_document_unhighlight_quicksearch(self->document);
	const char *search_text = gtk_entry_get_text(GTK_ENTRY(entry));
	bool found = i7_document_find_text(self->document, search_text, I7_SEARCH_CONTAINS | I7_SEARCH_IGNORE_CASE);
	i7_search_bar_set_not_found(self, !found);
}

void
on_findbar_entry_stop_search(GtkSearchEntry *entry, I7SearchBar *self)
{
	i7_document_unhighlight_quicksearch(self->document);
}

void
on_replace_button_clicked(GtkButton *button, I7SearchBar *self)
{
	const char *search_text = gtk_entry_get_text(GTK_ENTRY(self->entry));
	const char *replace_text = gtk_entry_get_text(self->replace_entry);
	gboolean ignore_case = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->ignore_case));
	gboolean restrict_search = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->restrict_search));
	I7SearchFlags flags = gtk_combo_box_get_active(GTK_COMBO_BOX(self->search_type))
		| (ignore_case? I7_SEARCH_IGNORE_CASE : 0)
		| (restrict_search? I7_SEARCH_RESTRICT : 0);
	GtkTextIter start, end;
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(i7_document_get_buffer(self->document));

	gtk_text_buffer_get_selection_bounds(buffer, &start, &end);
	gchar *selected = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

	/* if the text is already selected, then replace it, otherwise "find" again
	 to select the text */
	if(!(ignore_case? strcasecmp(selected, search_text) : strcmp(selected, search_text))) {
		/* Replacing counts as one action for Undo */
		gtk_text_buffer_begin_user_action(buffer);
		gtk_text_buffer_delete(buffer, &start, &end);
		gtk_text_buffer_insert(buffer, &start, replace_text, -1);
		gtk_text_buffer_end_user_action(buffer);
	}
	g_free(selected);

	/* Find the next occurrence of the text */
	i7_document_find_in_source(self->document, search_text, flags);
}

void
on_replace_all_button_clicked(GtkButton *button, I7SearchBar *self)
{
	const char *search_text = gtk_entry_get_text(GTK_ENTRY(self->entry));
	const char *replace_text = gtk_entry_get_text(self->replace_entry);
	gboolean ignore_case = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->ignore_case));
	gboolean restrict_search = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->restrict_search));
	I7SearchFlags search_type = gtk_combo_box_get_active(GTK_COMBO_BOX(self->search_type));
	GtkTextIter cursor, start, end;
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(i7_document_get_buffer(self->document));
	GtkTextSearchFlags gtk_flags = GTK_TEXT_SEARCH_TEXT_ONLY
		| (ignore_case? GTK_TEXT_SEARCH_CASE_INSENSITIVE : 0)
		| (restrict_search? GTK_TEXT_SEARCH_VISIBLE_ONLY : 0);

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
i7_search_bar_get_property(GObject *object, unsigned prop_id, GValue *value, GParamSpec *pspec)
{
	I7SearchBar *self = I7_SEARCH_BAR(object);

	switch(prop_id) {
		case PROP_DOCUMENT:
			g_value_set_object(value, self->document);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(self, prop_id, pspec);
	}
}

static void
i7_search_bar_set_property(GObject *object, unsigned prop_id, const GValue *value, GParamSpec *pspec)
{
	I7SearchBar *self = I7_SEARCH_BAR(object);

	switch(prop_id) {
		case PROP_DOCUMENT:
			g_assert(self->document == NULL);  /* construct only */
			self->document = g_value_dup_object(value);
			g_return_if_fail(self->document != NULL);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(self, prop_id, pspec);
	}
}

static void
i7_search_bar_dispose(GObject *object)
{
	I7SearchBar *self = I7_SEARCH_BAR(object);

	g_clear_object(&self->document);

	G_OBJECT_CLASS(i7_search_bar_parent_class)->dispose(object);
}

static void
i7_search_bar_class_init(I7SearchBarClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->get_property = i7_search_bar_get_property;
	object_class->set_property = i7_search_bar_set_property;
	object_class->dispose = i7_search_bar_dispose;

	g_object_class_install_property(object_class, PROP_DOCUMENT,
		g_param_spec_object("document", "Document", "Document window to be searched", I7_TYPE_DOCUMENT,
			G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	gtk_widget_class_set_template_from_resource(widget_class, "/com/inform7/IDE/ui/searchbar.ui");
	gtk_widget_class_bind_template_child(widget_class, I7SearchBar, entry);
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
	gtk_widget_class_bind_template_callback(widget_class, on_close_button_clicked);
	gtk_widget_class_bind_template_callback(widget_class, on_findbar_entry_search_changed);
	gtk_widget_class_bind_template_callback(widget_class, on_findbar_entry_stop_search);
	gtk_widget_class_bind_template_callback(widget_class, on_replace_all_button_clicked);
	gtk_widget_class_bind_template_callback(widget_class, on_replace_button_clicked);
}

/* PUBLIC FUNCTIONS */

I7SearchBar *
i7_search_bar_new(I7Document *document)
{
	g_return_val_if_fail(I7_IS_DOCUMENT(document), NULL);
	return I7_SEARCH_BAR(g_object_new(I7_TYPE_SEARCH_BAR,
		"document", document,
		NULL));
}

void
i7_search_bar_activate(I7SearchBar *self, bool replace_mode)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->replace_mode_button), replace_mode);
	gtk_search_bar_set_search_mode(GTK_SEARCH_BAR(self), TRUE);
	gtk_widget_grab_focus(GTK_WIDGET(self->entry));
}

const char *
i7_search_bar_get_search_string(I7SearchBar *self)
{
	return gtk_entry_get_text(GTK_ENTRY(self->entry));
}

void
i7_search_bar_set_target_description(I7SearchBar *self, const char *descr)
{
	if (descr == NULL) {
		gtk_widget_hide(GTK_WIDGET(self->search_label));
		return;
	}

	g_autofree char *text = g_strdup_printf(_("Searching <b>%s</b>"), descr);
	gtk_label_set_markup(GTK_LABEL(self->search_label), text);
	gtk_widget_show(GTK_WIDGET(self->search_label));
}

bool
i7_document_find_text(I7Document *self, const char *text, I7SearchFlags flags)
{
	return I7_DOCUMENT_GET_CLASS(self)->find_text(self, text, flags);
}

void
i7_document_unhighlight_quicksearch(I7Document *self)
{
	GtkWidget *focus = i7_document_get_highlighted_view(self);
	if(!focus)
		return;

	if(GTK_IS_TEXT_VIEW(focus)) {
		/* Remove selection */
		GtkTextIter start;
		GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(focus));
		if(gtk_text_buffer_get_selection_bounds(buffer, &start, NULL))
			gtk_text_buffer_place_cursor(buffer, &start);
	} else if(WEBKIT_IS_WEB_VIEW(focus)) {
		WebKitFindController *controller = webkit_web_view_get_find_controller(WEBKIT_WEB_VIEW(focus));
		webkit_find_controller_search_finish(controller);
	}

	i7_document_set_highlighted_view(self, NULL);
}

void
i7_search_bar_set_not_found(I7SearchBar *self, bool not_found)
{
	GAction *find_previous = g_action_map_lookup_action(G_ACTION_MAP(self->document), "find-previous");
	g_simple_action_set_enabled(G_SIMPLE_ACTION(find_previous), !not_found);
	GAction *find_next = g_action_map_lookup_action(G_ACTION_MAP(self->document), "find-next");
	g_simple_action_set_enabled(G_SIMPLE_ACTION(find_next), !not_found);

	GtkStyleContext *style = gtk_widget_get_style_context(GTK_WIDGET(self->entry));
	if(not_found) {
		gtk_style_context_add_class(style, "error");
	} else {
		gtk_style_context_remove_class(style, "error");
	}

	gtk_widget_set_sensitive(GTK_WIDGET(self->replace), !not_found);
	gtk_widget_set_sensitive(GTK_WIDGET(self->replace_all), !not_found);
}

void
i7_search_bar_set_can_replace(I7SearchBar *self, bool can_replace)
{
	gtk_widget_set_visible(GTK_WIDGET(self->replace_mode_button), can_replace);
}

void
i7_search_bar_set_can_restrict(I7SearchBar *self, bool can_restrict)
{
	gtk_widget_set_visible(GTK_WIDGET(self->restrict_search), can_restrict);
}

void
i7_document_find_in_source(I7Document *self, const char *text, I7SearchFlags flags)
{
	GtkTextIter start, end;
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(i7_document_get_buffer(self));

	if (!find(buffer, text, flags, &start, &end)) {
		i7_search_bar_set_not_found(I7_SEARCH_BAR(self->findbar), true);
		return;
	}
	i7_search_bar_set_not_found(I7_SEARCH_BAR(self->findbar), false);

	/* We may have searched the invisible regions, so if the found text is
	 invisible, go back to showing the entire source. */
	if(i7_document_iter_is_invisible(self, &start) || i7_document_iter_is_invisible(self, &end))
		i7_document_show_entire_source(self);

	gtk_text_buffer_select_range(buffer, &start, &end);
	i7_document_scroll_to_selection(self);
}
