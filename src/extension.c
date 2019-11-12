/* Copyright (C) 2006-2013, 2015, 2018 P. F. Chimento
 * This file is part of GNOME Inform 7.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>
#include "extension.h"
#include "app.h"
#include "configfile.h"
#include "document.h"
#include "error.h"
#include "file.h"
#include "lang.h"

typedef struct _I7ExtensionPrivate I7ExtensionPrivate;
struct _I7ExtensionPrivate
{
	/* Built-in extension or not */
	gboolean readonly;
	/* View with elastic tabstops (not saved) */
	gboolean elastic;
};

#define I7_EXTENSION_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), I7_TYPE_EXTENSION, I7ExtensionPrivate))
#define I7_EXTENSION_USE_PRIVATE(o,n) I7ExtensionPrivate *n = I7_EXTENSION_PRIVATE(o)

G_DEFINE_TYPE(I7Extension, i7_extension, I7_TYPE_DOCUMENT);

/* SIGNAL HANDLERS */

static void
on_heading_depth_value_changed(GtkRange *range, I7Extension *extension)
{
	double value = gtk_range_get_value(range);
	i7_document_set_headings_filter_level(I7_DOCUMENT(extension), (gint)value);
}

/* Save window size */
static void
save_extwindow_size(GtkWindow *window)
{
	I7App *theapp = i7_app_get();
	GSettings *state = i7_app_get_state(theapp);
	int w, h;

	gtk_window_get_size(window, &w, &h);
	g_settings_set(state, PREFS_STATE_EXT_WINDOW_SIZE, "(ii)", w, h);
}

static gboolean
on_extensionwindow_delete_event(GtkWidget *window, GdkEvent *event)
{
	if(i7_document_verify_save(I7_DOCUMENT(window))) {
		save_extwindow_size(GTK_WINDOW(window));
		i7_app_remove_document(i7_app_get(), I7_DOCUMENT(window));
		return FALSE;
	}
	return TRUE;
}

static void
on_notebook_switch_page(GtkNotebook *notebook, GtkWidget *page, unsigned page_num, I7Extension *extension)
{
	if(page_num != I7_SOURCE_VIEW_TAB_CONTENTS)
		return;
	i7_document_reindex_headings(I7_DOCUMENT(extension));
}

static void
on_headings_row_activated(GtkTreeView *view, GtkTreePath *path, GtkTreeViewColumn *column, I7Extension *extension)
{
	GtkTreePath *real_path = i7_document_get_child_path(I7_DOCUMENT(extension), path);
	i7_document_show_heading(I7_DOCUMENT(extension), real_path);
	gtk_notebook_set_current_page(GTK_NOTEBOOK(extension->sourceview->notebook), I7_SOURCE_VIEW_TAB_SOURCE);
}

static void
on_previous_action_notify_sensitive(GObject *action, GParamSpec *paramspec, I7Extension *extension)
{
	gboolean sensitive;
	g_object_get(action, "sensitive", &sensitive, NULL);
	if(sensitive)
		gtk_widget_show(extension->sourceview->previous);
	else
		gtk_widget_hide(extension->sourceview->previous);
}

static void
on_next_action_notify_sensitive(GObject *action, GParamSpec *paramspec, I7Extension *extension)
{
	gboolean sensitive;
	g_object_get(action, "sensitive", &sensitive, NULL);
	if(sensitive)
		gtk_widget_show(extension->sourceview->next);
	else
		gtk_widget_hide(extension->sourceview->next);
}

/* IMPLEMENTATIONS OF VIRTUAL FUNCTIONS */

static gchar *
i7_extension_extract_title(I7Document *document, gchar *text)
{
	I7App *app = i7_app_get();
	GMatchInfo *match = NULL;

	if(!g_regex_match(app->regices[I7_APP_REGEX_EXTENSION], text, 0, &match)) {
		g_match_info_free(match);
		return g_strdup(_("Untitled"));
	}

	gchar *title = g_match_info_fetch_named(match, "title");
	g_match_info_free(match);
	if(!title)
		return g_strdup(_("Untitled"));
	return title;
}

static void
i7_extension_set_contents_display(I7Document *document, I7ContentsDisplay display)
{
	i7_source_view_set_contents_display(I7_EXTENSION(document)->sourceview, display);
}

/* Save extension, in the previous location if it exists and is not a directory,
otherwise ask for a new location */
static gboolean
i7_extension_save(I7Document *document)
{
	if(I7_EXTENSION_PRIVATE(document)->readonly) {
		GtkWidget *dialog = gtk_message_dialog_new_with_markup(GTK_WINDOW(document), GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
			_("<big><b>You are editing a built-in Inform extension.</b></big>"));
		gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(dialog),
			_("You are not allowed to overwrite the extensions built into "
			"Inform. Instead, select <i>Save As...</i> or <i>Save a Copy...</i>"
			" from the <i>File</i> menu to save a copy of the extension to a "
			"different file. You can then install the extension to the local "
			"extensions directory by selecting <i>Install Extension</i> from "
			"the <i>File</i> menu, and the compiler will use that extension "
			"instead of the built-in one."));
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		return FALSE;
	}

	GFile *file = i7_document_get_file(document);
	if(file && g_file_query_exists(file, NULL))
		i7_document_save_as(document, file);
	else {
		GFile *newfile = get_file_from_save_dialog(document, file);
		if(!newfile)
			return FALSE;
		i7_document_set_file(document, newfile);
		i7_document_save_as(document, newfile);
		g_object_unref(newfile);
	}
	if(file)
		g_object_unref(file);
	return TRUE;
}

/* Update the list of recently used files */
static void
update_recent_extension_file(I7Extension *extension, GFile *file, gboolean readonly)
{
	GtkRecentManager *manager = gtk_recent_manager_get_default();
	char *uri = g_file_get_uri(file);

	/* We use the groups "inform7_project", "inform7_extension", and
	 "inform7_builtin" to determine how to open a file from the recent manager */
	char *groups_readonly[] = { "inform7_builtin", NULL };
	char *groups_regular[] = { "inform7_extension", NULL };
	GtkRecentData recent_data = {
		NULL, NULL, "text/x-natural-inform", "Inform 7",
		"gnome-inform7", NULL, FALSE
	};

	recent_data.display_name = file_get_display_name(file);

	/* Use the "begins here" line as the description,
	 retrieved from the first line of the text */
	GtkTextIter start, end;
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(i7_document_get_buffer(I7_DOCUMENT(extension)));
	gtk_text_buffer_get_iter_at_line(buffer, &start, 0);
	gtk_text_buffer_get_iter_at_line(buffer, &end, 0);
	gtk_text_iter_forward_to_line_end(&end);
	recent_data.description = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
	recent_data.groups = readonly? groups_readonly : groups_regular;
	gtk_recent_manager_add_full(manager, uri, &recent_data);

	g_free(recent_data.display_name);
	g_free(recent_data.description);
	g_free(uri);
}

/* Remove a file from the recently used list of files, e.g. if it failed to
open */
static void
remove_recent_extension_file(GFile *file)
{
	GtkRecentManager *manager = gtk_recent_manager_get_default();
	char *uri = g_file_get_uri(file);
	gtk_recent_manager_remove_item(manager, uri, NULL);
	/* ignore error */
	g_free(uri);
}

/* Save extension in the given directory  */
static void
i7_extension_save_as(I7Document *document, GFile *file)
{
	GError *err = NULL;

	i7_document_display_status_message(document, _("Saving project..."), FILE_OPERATIONS);

	i7_document_stop_file_monitor(document);

	/* Save the source */
	gchar *text = i7_document_get_source_text(document);
	/* Write text to file */
	if(!g_file_replace_contents(file, text, strlen(text), NULL, FALSE, G_FILE_CREATE_NONE, NULL, NULL, &err)) {
		error_dialog_file_operation(GTK_WINDOW(document), file, err, I7_FILE_ERROR_SAVE, NULL);
		g_free(text);
		return;
	}
	g_free(text);

	update_recent_extension_file(I7_EXTENSION(document), file, FALSE);

	/* Start file monitoring again */
	i7_document_monitor_file(document, file);

	i7_document_set_modified(document, FALSE);

	i7_document_remove_status_message(document, FILE_OPERATIONS);
}

static GtkTextView *
i7_extension_get_default_view(I7Document *document)
{
	return GTK_TEXT_VIEW(I7_EXTENSION(document)->sourceview->source);
}

static void
i7_extension_scroll_to_selection(I7Document *document)
{
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(i7_document_get_buffer(document));
	GtkTextView *view = GTK_TEXT_VIEW(I7_EXTENSION(document)->sourceview->source);
	gtk_notebook_set_current_page(GTK_NOTEBOOK(I7_EXTENSION(document)->sourceview->notebook), I7_SOURCE_VIEW_TAB_SOURCE);
	gtk_text_view_scroll_to_mark(view, gtk_text_buffer_get_insert(buffer), 0.25, FALSE, 0.0, 0.0);
	gtk_widget_grab_focus(GTK_WIDGET(view));
}

/* Only update the tabs in this extension window */
static void
i7_extension_update_tabs(I7Document *document)
{
	if(!I7_IS_EXTENSION(document))
		return;
	g_idle_add((GSourceFunc)update_tabs, GTK_SOURCE_VIEW(I7_EXTENSION(document)->sourceview->source));
}

static gboolean
update_font_tabs(GtkSourceView *view)
{
	update_font(GTK_WIDGET(view));
	update_tabs(view);
	return FALSE; /* one-shot idle function */
}

/* Update the fonts in this extension window, but not the
widgets that only need their font size updated */
static void
i7_extension_update_fonts(I7Document *document)
{
	if(!I7_IS_EXTENSION(document))
		return;
	g_idle_add((GSourceFunc)update_font_tabs, GTK_SOURCE_VIEW(I7_EXTENSION(document)->sourceview->source));
}

static void
i7_extension_update_font_sizes(I7Document *document)
{
	return; /* No font sizes to update */
}

static void
i7_extension_expand_headings_view(I7Document *document)
{
	gtk_tree_view_expand_all(GTK_TREE_VIEW(I7_EXTENSION(document)->sourceview->headings));
}

static gboolean
do_search(GtkTextView *view, const gchar *text, gboolean forward, const GtkTextIter *startpos, GtkTextIter *start, GtkTextIter *end)
{
	if(forward)
		return gtk_text_iter_forward_search(startpos, text, GTK_TEXT_SEARCH_VISIBLE_ONLY | GTK_TEXT_SEARCH_TEXT_ONLY | GTK_TEXT_SEARCH_CASE_INSENSITIVE, start, end, NULL);
	return gtk_text_iter_backward_search(startpos, text, GTK_TEXT_SEARCH_VISIBLE_ONLY | GTK_TEXT_SEARCH_TEXT_ONLY | GTK_TEXT_SEARCH_CASE_INSENSITIVE, start, end, NULL);
}

static gboolean
i7_extension_highlight_search(I7Document *document, const gchar *text, gboolean forward)
{
	if(*text == '\0') {
		/* If the text is blank, unhighlight everything and return TRUE */
		i7_document_unhighlight_quicksearch(document);
		return TRUE;
	}

	GtkWidget *focus = I7_EXTENSION(document)->sourceview->source;

	if(gtk_notebook_get_current_page(GTK_NOTEBOOK(I7_EXTENSION(document)->sourceview->notebook)) == I7_SOURCE_VIEW_TAB_CONTENTS) {
		/* Headings view is visible, switch back to source code view */
		gtk_notebook_set_current_page(GTK_NOTEBOOK(I7_EXTENSION(document)->sourceview->notebook), I7_SOURCE_VIEW_TAB_SOURCE);
		gtk_widget_grab_focus(document->findbar_entry);
	}

	i7_document_set_highlighted_view(document, focus);

	/* Source view and text view */
	GtkTextIter iter, start, end;
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(focus));

	/* Start the search at either the beginning or end of the selection
	 depending on the direction */
	GtkTextMark *startmark = forward? gtk_text_buffer_get_selection_bound(buffer) : gtk_text_buffer_get_insert(buffer);
	gtk_text_buffer_get_iter_at_mark(buffer, &iter, startmark);
	if(!do_search(GTK_TEXT_VIEW(focus), text, forward, &iter, &start, &end)) {
		if(forward)
			gtk_text_buffer_get_start_iter(buffer, &iter);
		else
			gtk_text_buffer_get_end_iter(buffer, &iter);
		if(!do_search(GTK_TEXT_VIEW(focus), text, forward, &iter, &start, &end))
			return FALSE;
	}
	gtk_text_buffer_select_range(buffer, &start, &end);
	gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(focus), gtk_text_buffer_get_insert(buffer), 0.25, FALSE, 0.0, 0.0);

	return TRUE;
}

static void
i7_extension_set_spellcheck(I7Document *document, gboolean spellcheck)
{
	i7_source_view_set_spellcheck(I7_EXTENSION(document)->sourceview, spellcheck);
}

static void
i7_extension_check_spelling(I7Document *document)
{
	i7_source_view_check_spelling(I7_EXTENSION(document)->sourceview);
}

static void
i7_extension_set_elastic_tabstops(I7Document *document, gboolean elastic)
{
	I7_EXTENSION_USE_PRIVATE(document, priv);
	priv->elastic = elastic;
	i7_source_view_set_elastic_tabstops(I7_EXTENSION(document)->sourceview, elastic);
}

static void
i7_extension_revert(I7Document *document)
{
	I7_EXTENSION_USE_PRIVATE(document, priv);
	GFile *file = i7_document_get_file(document);
	i7_extension_open(I7_EXTENSION(document), file, priv->readonly);
	g_object_unref(file);
}

/* TYPE SYSTEM */

static void
i7_extension_init(I7Extension *self)
{
	I7_EXTENSION_USE_PRIVATE(self, priv);
	GError *error = NULL;
	I7App *theapp = i7_app_get();
	GSettings *state = i7_app_get_state(theapp);

	priv->readonly = FALSE;

	/* Build the menus and toolbars from the GtkUIManager file */
	GFile *file = i7_app_get_data_file_va(i7_app_get(), "ui", "extension.uimanager.xml", NULL);
	char *path = g_file_get_path(file);
	gtk_ui_manager_add_ui_from_file(I7_DOCUMENT(self)->ui_manager, path, &error);
	g_free(path);
	g_object_unref(file);
	if(error)
		ERROR(_("Building menus failed"), error);
	GtkWidget *menu = gtk_ui_manager_get_widget(I7_DOCUMENT(self)->ui_manager, "/ExtensionMenubar");
	I7_DOCUMENT(self)->toolbar = gtk_ui_manager_get_widget(I7_DOCUMENT(self)->ui_manager, "/ExtensionToolbar");
	gtk_widget_set_no_show_all(I7_DOCUMENT(self)->toolbar, TRUE);
	i7_document_add_menus_and_findbar(I7_DOCUMENT(self));

	/* Build the rest of the interface */
	gtk_box_pack_start(GTK_BOX(I7_DOCUMENT(self)->box), menu, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(I7_DOCUMENT(self)->box), I7_DOCUMENT(self)->toolbar, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(I7_DOCUMENT(self)->box), I7_DOCUMENT(self)->findbar, FALSE, FALSE, 0);

	/* Create source view */
	self->sourceview = I7_SOURCE_VIEW(i7_source_view_new());
	gtk_widget_show(GTK_WIDGET(self->sourceview));
	gtk_box_pack_start(GTK_BOX(I7_DOCUMENT(self)->box), GTK_WIDGET(self->sourceview), TRUE, TRUE, 0);

	/* Set up the signals to do the menu hints in the statusbar */
	i7_document_attach_menu_hints(I7_DOCUMENT(self), GTK_MENU_BAR(menu));

	/* Build the Open Extensions menu */
	i7_app_update_extensions_menu(theapp);

	/* Set the last saved window size */
	int w, h;
	g_settings_get(state, PREFS_STATE_EXT_WINDOW_SIZE, "(ii)", &w, &h);
	gtk_window_resize(GTK_WINDOW(self), w, h);

	/* Set up the Natural Inform highlighting */
	GtkSourceBuffer *buffer = i7_document_get_buffer(I7_DOCUMENT(self));
	set_buffer_language(buffer, "inform7x");
	gtk_source_buffer_set_style_scheme(buffer, i7_app_get_current_color_scheme(theapp));

	/* Connect other signals */
	g_signal_connect(self->sourceview->heading_depth, "value-changed", G_CALLBACK(on_heading_depth_value_changed), self);
	g_signal_connect(self->sourceview->notebook, "switch-page", G_CALLBACK(on_notebook_switch_page), self);
	g_signal_connect(self->sourceview->headings, "row-activated", G_CALLBACK(on_headings_row_activated), self);

	/* Connect various models to various views */
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(self->sourceview->source), GTK_TEXT_BUFFER(buffer));
	gtk_tree_view_set_model(GTK_TREE_VIEW(self->sourceview->headings), i7_document_get_headings(I7_DOCUMENT(self)));

	/* Connect the Previous Section and Next Section actions to the up and down buttons */
	gtk_activatable_set_related_action(GTK_ACTIVATABLE(self->sourceview->previous), I7_DOCUMENT(self)->previous_section);
	gtk_activatable_set_related_action(GTK_ACTIVATABLE(self->sourceview->next), I7_DOCUMENT(self)->next_section);

	/* We don't need to keep a reference to the buffer and model anymore */
	g_object_unref(buffer);
	g_object_unref(i7_document_get_headings(I7_DOCUMENT(self)));

	/* Set up the Previous Section and Next Section actions to synch with the buttons */
	g_signal_connect(I7_DOCUMENT(self)->previous_section, "notify::sensitive", G_CALLBACK(on_previous_action_notify_sensitive), self);
	g_signal_connect(I7_DOCUMENT(self)->next_section, "notify::sensitive", G_CALLBACK(on_next_action_notify_sensitive), self);
	/* For some reason this needs to be triggered even if the buttons are set to invisible in Glade */
	gtk_action_set_sensitive(I7_DOCUMENT(self)->previous_section, FALSE);
	gtk_action_set_sensitive(I7_DOCUMENT(self)->next_section, FALSE);

	/* Set font sizes, etc. */
	i7_document_update_fonts(I7_DOCUMENT(self));

	/* Set spell checking */
	gboolean spell_check_default = g_settings_get_boolean(state, PREFS_STATE_SPELL_CHECK);
	gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(I7_DOCUMENT(self)->autocheck_spelling), spell_check_default);
	i7_document_set_spellcheck(I7_DOCUMENT(self), spell_check_default);

	/* Create a callback for the delete event */
	g_signal_connect(self, "delete-event", G_CALLBACK(on_extensionwindow_delete_event), NULL);
}

static void
i7_extension_finalize(GObject *self)
{
	G_OBJECT_CLASS(i7_extension_parent_class)->finalize(self);
}

static void
i7_extension_class_init(I7ExtensionClass *klass)
{
	I7DocumentClass *document_class = I7_DOCUMENT_CLASS(klass);
	document_class->extract_title = i7_extension_extract_title;
	document_class->set_contents_display = i7_extension_set_contents_display;
	document_class->get_default_view = i7_extension_get_default_view;
	document_class->save = i7_extension_save;
	document_class->save_as = i7_extension_save_as;
	document_class->scroll_to_selection = i7_extension_scroll_to_selection;
	document_class->update_tabs = i7_extension_update_tabs;
	document_class->update_fonts = i7_extension_update_fonts;
	document_class->update_font_sizes = i7_extension_update_font_sizes;
	document_class->expand_headings_view = i7_extension_expand_headings_view;
	document_class->highlight_search = i7_extension_highlight_search;
	document_class->set_spellcheck = i7_extension_set_spellcheck;
	document_class->check_spelling = i7_extension_check_spelling;
	document_class->set_elastic_tabstops = i7_extension_set_elastic_tabstops;
	document_class->revert = i7_extension_revert;

	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = i7_extension_finalize;

	g_type_class_add_private(klass, sizeof(I7ExtensionPrivate));
}

/* PUBLIC FUNCTIONS */

I7Extension *
i7_extension_new(I7App *app, GFile *file, const char *title, const char *author)
{
	I7Extension *extension = I7_EXTENSION(g_object_new(I7_TYPE_EXTENSION, NULL));

	i7_document_set_file(I7_DOCUMENT(extension), file);

	gchar *text = g_strconcat(title, " by ", author, " begins here.\n\n", title, " ends here.\n", NULL);
	i7_document_set_source_text(I7_DOCUMENT(extension), text);
	i7_document_set_modified(I7_DOCUMENT(extension), TRUE);

	/* Add document to global list */
	i7_app_register_document(app, I7_DOCUMENT(extension));

	/* Bring window to front */
	gtk_widget_show(GTK_WIDGET(extension));
	gtk_window_present(GTK_WINDOW(extension));
	return extension;
}

I7Extension *
i7_extension_new_from_file(I7App *app, GFile *file, gboolean readonly)
{
	I7Document *dupl = i7_app_get_already_open(app, file);
	if(dupl && I7_IS_EXTENSION(dupl)) {
		gtk_window_present(GTK_WINDOW(dupl));
		return NULL;
	}

	I7Extension *extension = I7_EXTENSION(g_object_new(I7_TYPE_EXTENSION, NULL));
	if(!i7_extension_open(extension, file, readonly)) {
		gtk_widget_destroy(GTK_WIDGET(extension));
		return NULL;
	}

	/* Add document to global list */
	i7_app_register_document(app, I7_DOCUMENT(extension));

	/* Bring window to front */
	gtk_widget_show(GTK_WIDGET(extension));
	gtk_window_present(GTK_WINDOW(extension));
	return extension;
}

/**
 * i7_extension_open:
 * @extension: the extension object
 * @file: a #GFile to open
 * @readonly: whether to open @file as a built-in extension or not
 *
 * Opens the extension from @file, and returns success.
 *
 * Returns: %TRUE if successful, %FALSE if not.
 */
gboolean
i7_extension_open(I7Extension *extension, GFile *file, gboolean readonly)
{
	i7_document_set_file(I7_DOCUMENT(extension), file);

	/* If it was a built-in extension, set it read-only */
	i7_extension_set_read_only(extension, readonly);

	/* Read the source */
	char *text = read_source_file(file);
	if(!text)
		goto fail;

	update_recent_extension_file(extension, file, readonly);

	/* Watch for changes to the source file */
	i7_document_monitor_file(I7_DOCUMENT(extension), file);

	/* Write the source to the source buffer, clearing the undo history */
	i7_document_set_source_text(I7_DOCUMENT(extension), text);
	g_free(text);

	GtkTextIter start;
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(i7_document_get_buffer(I7_DOCUMENT(extension)));
	gtk_text_buffer_get_start_iter(buffer, &start);
	gtk_text_buffer_place_cursor(buffer, &start);

	i7_document_set_modified(I7_DOCUMENT(extension), FALSE);

	return TRUE;

fail:
	remove_recent_extension_file(file);
	return FALSE;
}

void
i7_extension_set_read_only(I7Extension *extension, gboolean readonly)
{
	I7_EXTENSION_PRIVATE(extension)->readonly = readonly;
}
