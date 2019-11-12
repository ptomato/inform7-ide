/* Copyright (C) 2006-2015, 2021 P. F. Chimento
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

#include "config.h"

#include <errno.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>
#include <libchimara/chimara-glk.h>
#include <libchimara/chimara-if.h>
#include <osxcart/plist.h>
#include <osxcart/rtf.h>
#include <webkit2/webkit2.h>

#include "app.h"
#include "builder.h"
#include "configfile.h"
#include "document.h"
#include "error.h"
#include "file.h"
#include "lang.h"
#include "node.h"
#include "panel.h"
#include "placeholder-entry.h"
#include "searchwindow.h"
#include "skein.h"
#include "skein-view.h"
#include "source-view.h"
#include "story.h"
#include "story-private.h"

enum {
	PROP_0,
	PROP_STORY_FORMAT,
	PROP_MAKE_BLORB,
	PROP_NOBBLE_RNG,
	PROP_ELASTIC_TABSTOPS
};

G_DEFINE_TYPE(I7Story, i7_story, I7_TYPE_DOCUMENT);

/* SIGNAL HANDLERS */

/* Defined in story-skein.c */
void on_skein_modified(I7Skein *, I7Story *);
void on_node_activate(I7Skein *, I7Node *, I7Story *);
void on_node_popup(I7SkeinView *, I7Node *);
void on_differs_badge_activate(I7Skein *, I7Node *, I7Story *);
void on_labels_changed(I7Skein *, I7Panel *);
void on_show_node(I7Skein *, I7SkeinShowNodeReason, I7Node *, I7Panel *);
/* Defined in story-game.c */
void on_game_started(ChimaraGlk *, I7Story *);
void on_game_stopped(ChimaraGlk *, I7Story *);
void on_game_command(ChimaraIF *, gchar *, gchar *, I7Story *);
gchar *load_blorb_resource(guint32, guint32, I7Story *);

static void
on_heading_depth_value_changed(GtkRange *range, I7Story *story)
{
	double value = gtk_range_get_value(range);
	gtk_range_set_value(GTK_RANGE(story->panel[LEFT]->sourceview->heading_depth), value);
	gtk_range_set_value(GTK_RANGE(story->panel[RIGHT]->sourceview->heading_depth), value);
	i7_document_set_headings_filter_level(I7_DOCUMENT(story), (gint)value);
}

/* Save window size and slider position */
static void
save_storywindow_size(I7Story *story)
{
	I7App *theapp = i7_app_get();
	GSettings *state = i7_app_get_state(theapp);
	gint w, h, x, y;

	gtk_window_get_size(GTK_WINDOW(story), &w, &h);
	g_settings_set(state, PREFS_STATE_WINDOW_SIZE, "(ii)", w, h);
	g_settings_set_int(state, PREFS_STATE_DIVIDER_POS, gtk_paned_get_position(GTK_PANED(story->facing_pages)));

	/* Also save the notepad window */
	gtk_window_get_size(GTK_WINDOW(story->notes_window), &w, &h);
	gtk_window_get_position(GTK_WINDOW(story->notes_window), &x, &y);
	g_settings_set(state, PREFS_STATE_NOTEPAD_SIZE, "(ii)", w, h);
	g_settings_set(state, PREFS_STATE_NOTEPAD_POS, "(ii)", x, y);
}

static gboolean
on_storywindow_delete_event(GtkWidget *window, GdkEvent *event)
{
	if(!i7_document_verify_save(I7_DOCUMENT(window)))
		return TRUE; /* Interrupt the signal if the user cancelled */

	save_storywindow_size(I7_STORY(window));

	gtk_widget_destroy(I7_STORY(window)->notes_window);
	i7_story_stop_running_game(I7_STORY(window));

	GFile *file = i7_document_get_file(I7_DOCUMENT(window));
	if(file) {
		delete_build_files(I7_STORY(window));
		g_object_unref(file);
	}

	i7_app_remove_document(i7_app_get(), I7_DOCUMENT(window));

	return FALSE;
}

static void
on_panel_select_view(I7Panel *panel, I7PanelPane pane, I7Story *story)
{
	i7_story_show_pane(story, pane);
}

static void
on_panel_display_docpage(I7Panel *panel, gchar *uri, I7Story *story)
{
	I7StoryPanel side = i7_story_choose_panel(story, I7_PANE_DOCUMENTATION);
	webkit_web_view_load_uri(WEBKIT_WEB_VIEW(story->panel[side]->tabs[I7_PANE_DOCUMENTATION]), uri);
	gtk_notebook_set_current_page(GTK_NOTEBOOK(story->panel[side]->notebook), I7_PANE_DOCUMENTATION);
}

static void
on_panel_display_extensions_docpage(I7Panel *panel, char *uri, I7Story *story)
{
	I7StoryPanel side = i7_story_choose_panel(story, I7_PANE_EXTENSIONS);
	webkit_web_view_load_uri(WEBKIT_WEB_VIEW(story->panel[side]->tabs[I7_PANE_EXTENSIONS]), uri);
	gtk_notebook_set_current_page(GTK_NOTEBOOK(story->panel[side]->notebook), I7_PANE_EXTENSIONS);
}

static void
ignore_script_finish(WebKitWebView *webview, GAsyncResult *res, void *data) {
	g_autoptr(GError) error = NULL;
	g_autoptr(WebKitJavascriptResult) js_result = webkit_web_view_run_javascript_finish(webview, res, &error);
	/* ignore error */
	if (!js_result)
		g_warning("Error refreshing window.location.search: %s", error->message);
}

static void
on_panel_display_index_page(I7Panel *panel, I7PaneIndexTab tabnum, char *param, I7Story *story)
{
	I7StoryPanel side = i7_story_choose_panel(story, I7_PANE_INDEX);

	/* If a ?param was requested in the URI, then navigate there before showing
	the page - this doesn't completely eliminate the flash of the page changing,
	but it helps */
	if(param != NULL) {
		char *script = g_strconcat("window.location.search = '", param, "'", NULL);
		webkit_web_view_run_javascript(WEBKIT_WEB_VIEW(story->panel[side]->index_tabs[tabnum]),
		    script, NULL, (GAsyncReadyCallback)ignore_script_finish, NULL);
		g_free(script);
	}

	gtk_notebook_set_current_page(GTK_NOTEBOOK(story->panel[side]->tabs[I7_PANE_INDEX]), tabnum);
	gtk_notebook_set_current_page(GTK_NOTEBOOK(story->panel[side]->notebook), I7_PANE_INDEX);
}

static void
on_source_notebook_switch_page(GtkNotebook *notebook, GtkWidget *page, unsigned page_num, I7Story *story)
{
	if(page_num != I7_SOURCE_VIEW_TAB_CONTENTS)
		return;
	i7_document_reindex_headings(I7_DOCUMENT(story));
}

static void
on_headings_row_activated(GtkTreeView *view, GtkTreePath *path, GtkTreeViewColumn *column, I7Story *story)
{
	I7StoryPanel side = (view == GTK_TREE_VIEW(story->panel[LEFT]->source_tabs[I7_SOURCE_VIEW_TAB_CONTENTS]))? LEFT : RIGHT;
	GtkTreePath *real_path = i7_document_get_child_path(I7_DOCUMENT(story), path);
	i7_document_show_heading(I7_DOCUMENT(story), real_path);
	gtk_notebook_set_current_page(GTK_NOTEBOOK(story->panel[side]->tabs[I7_PANE_SOURCE]), I7_SOURCE_VIEW_TAB_SOURCE);
}

static void
on_previous_action_notify_sensitive(GObject *action, GParamSpec *paramspec, I7Story *story)
{
	gboolean sensitive;
	g_object_get(action, "sensitive", &sensitive, NULL);
	if(sensitive) {
		gtk_widget_show(story->panel[LEFT]->sourceview->previous);
		gtk_widget_show(story->panel[RIGHT]->sourceview->previous);
	} else {
		gtk_widget_hide(story->panel[LEFT]->sourceview->previous);
		gtk_widget_hide(story->panel[RIGHT]->sourceview->previous);
	}
}

static void
on_next_action_notify_sensitive(GObject *action, GParamSpec *paramspec, I7Story *story)
{
	gboolean sensitive;
	g_object_get(action, "sensitive", &sensitive, NULL);
	if(sensitive) {
		gtk_widget_show(story->panel[LEFT]->sourceview->next);
		gtk_widget_show(story->panel[RIGHT]->sourceview->next);
	} else {
		gtk_widget_hide(story->panel[LEFT]->sourceview->next);
		gtk_widget_hide(story->panel[RIGHT]->sourceview->next);
	}
}

void
on_facing_pages_set_focus_child(GtkContainer *container, GtkWidget *child, I7Story *story)
{
	if(child)
		I7_STORY_PRIVATE(story)->last_focused = child;
	/* Do not save the pointer if it is NULL: that means the focus left the
	 widget */
}

void
on_search_entry_activate(GtkEntry *entry, I7Story *story)
{
	const gchar *text = gtk_entry_get_text(entry);

	GtkWidget *search_window = i7_search_window_new(I7_DOCUMENT(story), text, TRUE, I7_SEARCH_CONTAINS);
	i7_search_window_search_documentation(I7_SEARCH_WINDOW(search_window));
	i7_search_window_done_searching(I7_SEARCH_WINDOW(search_window));
}

void
on_search_entry_icon_press(GtkEntry *entry, GtkEntryIconPosition icon_pos, GdkEvent *event)
{
	gtk_entry_set_text(entry, "");
}

/* OVERRIDES */

static void
i7_story_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	I7Story *story = I7_STORY(object);

	switch(prop_id)
	{
		case PROP_STORY_FORMAT:
			i7_story_set_story_format(story, (I7StoryFormat)g_value_get_uint(value));
			break;
		case PROP_MAKE_BLORB:
			i7_story_set_create_blorb(story, g_value_get_boolean(value));
			break;
		case PROP_NOBBLE_RNG:
			i7_story_set_nobble_rng(story, g_value_get_boolean(value));
			break;
		case PROP_ELASTIC_TABSTOPS:
			i7_document_set_elastic_tabstops(I7_DOCUMENT(story), g_value_get_boolean(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
i7_story_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	I7Story *story = I7_STORY(object);

	switch(prop_id)
	{
		case PROP_STORY_FORMAT:
			g_value_set_uint(value, i7_story_get_story_format(story));
			break;
		case PROP_MAKE_BLORB:
			g_value_set_boolean(value, i7_story_get_create_blorb(story));
			break;
		case PROP_NOBBLE_RNG:
			g_value_set_boolean(value, i7_story_get_nobble_rng(story));
			break;
		case PROP_ELASTIC_TABSTOPS:
			g_value_set_boolean(value, i7_story_get_elastic_tabstops(story));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

/* IMPLEMENTATIONS OF VIRTUAL FUNCTIONS */

static gchar *
i7_story_extract_title(I7Document *document, gchar *text)
{
	if(text[0] == '\"') {
		/* Look for the title followed by the author name */
		gchar *found = strstr(text, "\" by ");
		if(found)
			return g_strndup(text, (gsize)(++found - text));

		/* Look for the title on its own */
		found = strrchr(text, '\"');
		if(found)
			return g_strndup(text, (gsize)(++found - text));
	}
	return g_strdup(_("Untitled"));
}

/* Save story, in the previous location if it exists, otherwise ask for a new
location */
static gboolean
i7_story_save(I7Document *document)
{
	GFile *file = i7_document_get_file(document);
	if(file && file_exists_and_is_dir(file)) {
		i7_document_save_as(document, file);
		g_object_unref(file);
	} else {
		GFile *new_file = i7_document_run_save_dialog(document, file);
		if(!new_file)
			return FALSE;
		i7_document_set_file(document, new_file);
		i7_document_save_as(document, new_file);
		g_object_unref(new_file);
	}
	return TRUE;
}

/* Get a URI from a story GFile suitable for passing to the GtkRecentManager.
Add story.ni as the actual file to open, in case any other application wants to
open it. Free return value when done. */
static char *
get_recent_uri_for_story_file(GFile *file)
{
	GFile *source_file = g_file_get_child(file, "Source");
	GFile *story_file = g_file_get_child(source_file, "story.ni");
	g_object_unref(source_file);
	char *uri = g_file_get_uri(story_file);
	g_object_unref(story_file);
	return uri;
}

/* Update the list of recently used files */
static void
update_recent_story_file(I7Story *story, GFile *file)
{
	GtkRecentManager *manager = gtk_recent_manager_get_default();
	char *uri = get_recent_uri_for_story_file(file);

	/* We use the groups "inform7_project", "inform7_extension", and
	 "inform7_builtin" to determine how to open a file from the recent manager */
	char *groups[] = { "inform7_project", NULL };
	GtkRecentData recent_data = {
		NULL, NULL, "text/x-natural-inform", "Inform 7",
		"gnome-inform7 %f", NULL, FALSE
	};

	/* The display name is the project directory */
	recent_data.display_name = file_get_display_name(file);

	/* Use the story title and author as the description,
	retrieved from the first line of the text */
	GtkTextIter start, end;
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(i7_document_get_buffer(I7_DOCUMENT(story)));
	gtk_text_buffer_get_iter_at_line(buffer, &start, 0);
	gtk_text_buffer_get_iter_at_line(buffer, &end, 0);
	gtk_text_iter_forward_to_line_end(&end);
	recent_data.description = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

	recent_data.groups = groups;
	gtk_recent_manager_add_full(manager, uri, &recent_data);

	g_free(recent_data.display_name);
	g_free(recent_data.description);
	g_free(uri);
}

/* Remove a file from the recently used list of files, e.g. if it failed to
open */
static void
remove_recent_story_file(GFile *file)
{
	GtkRecentManager *manager = gtk_recent_manager_get_default();
	char *uri = get_recent_uri_for_story_file(file);
	gtk_recent_manager_remove_item(manager, uri, NULL);
	/* ignore error */
	g_free(uri);
}

/* Save story in the given directory  */
static void
i7_story_save_as(I7Document *document, GFile *file)
{
	I7StoryPrivate *priv = I7_STORY_PRIVATE(document);
	GError *err = NULL;

	i7_document_display_status_message(document, _("Saving project..."), FILE_OPERATIONS);

	i7_document_stop_file_monitor(document);

	/* Create the project directory if it does not already exist */
	GFile *build_file = g_file_get_child(file, "Build");
	GFile *index_file = g_file_get_child(file, "Index");
	GFile *source_file = g_file_get_child(file, "Source");
	if(!make_directory_unless_exists(file, NULL, &err)
		|| !make_directory_unless_exists(build_file, NULL, &err)
		|| !make_directory_unless_exists(index_file, NULL, &err)
		|| !make_directory_unless_exists(source_file, NULL, &err))
	{
		IO_ERROR_DIALOG(GTK_WINDOW(document), file, err, "creating project directory");
		g_object_unref(build_file);
		g_object_unref(index_file);
		g_object_unref(source_file);
		return;
	}
	g_object_unref(build_file);
	g_object_unref(index_file);

	/* Save the source */
	gchar *text = i7_document_get_source_text(document);
	/* Write text to file */
	GFile *story_file = g_file_get_child(source_file, "story.ni");
	g_object_unref(source_file);

	if(!g_file_replace_contents(story_file, text, strlen(text), NULL, FALSE, G_FILE_CREATE_NONE, NULL, NULL, &err)) {
		error_dialog_file_operation(GTK_WINDOW(document), story_file, err, I7_FILE_ERROR_SAVE, NULL);
		g_object_unref(story_file);
		g_free(text);
		return;
	}
	g_free(text);

	gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(i7_document_get_buffer(document)), FALSE);

	update_recent_story_file(I7_STORY(document), file);

	/* Start file monitoring again */
	i7_document_monitor_file(document, story_file);
	g_object_unref(story_file);

	/* Save the skein */
	GFile *skein_file = g_file_get_child(file, "Skein.skein");
	if(!i7_skein_save(priv->skein, skein_file, &err)) {
		error_dialog(GTK_WINDOW(document), err, _("There was an error saving the Skein. Your story will still be saved. Problem: "));
		err = NULL;
	}
	g_object_unref(skein_file);

	/* Save the notes */
	GFile *notes_file = g_file_get_child(file, "notes.rtf");
	if(!rtf_text_buffer_export_file(priv->notes, notes_file, NULL, &err)) {
		error_dialog(GTK_WINDOW(document), err, _("There was an error saving the Notepad. Your story will still be saved. Problem: "));
		err = NULL;
	}
	gtk_text_buffer_set_modified(priv->notes, FALSE);
	g_object_unref(notes_file);

	/* Save the project settings */
	GFile *settings_file = g_file_get_child(file, "Settings.plist");
	if(!plist_write_file(priv->settings, settings_file, NULL, &err)) {
		error_dialog(GTK_WINDOW(document), err, _("There was an error saving the project settings. Your story will still be saved. Problem: "));
		err = NULL;
	}
	g_object_unref(settings_file);

	/* Delete the build files from the project directory */
	delete_build_files(I7_STORY(document));

	/* Set the folder icon to be the Inform 7 project icon */
	file_set_custom_icon(file, "application-x-inform");
	GFile *materials_file = i7_story_get_materials_file(I7_STORY(document));
	if(file_exists_and_is_dir(materials_file))
		file_set_custom_icon(materials_file, "application-x-inform-materials");
	g_object_unref(materials_file);

	i7_document_set_modified(document, FALSE);

	i7_document_remove_status_message(document, FILE_OPERATIONS);
}

static GFile *
i7_story_run_save_dialog(I7Document *document, GFile *default_file)
{
	/* Create a file chooser */
	GtkWidget *dialog = gtk_file_chooser_dialog_new(_("Save File"),
		GTK_WINDOW(document), GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
		NULL);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);

	if (default_file) {
		if (g_file_query_exists(default_file, NULL)) {
			gtk_file_chooser_set_file(GTK_FILE_CHOOSER(dialog), default_file, NULL); /* ignore errors */
		} else {
			/* gtk_file_chooser_set_file() doesn't set the name if the file
			doesn't exist, i.e. the user created a new document */
			gtk_file_chooser_set_current_folder_file(GTK_FILE_CHOOSER(dialog), default_file, NULL); /* ignore errors */
			char *display_name = file_get_display_name(default_file);
			gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), display_name);
			g_free(display_name);
		}
	}

	GtkFileFilter *filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filter, "*.inform");
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog), filter);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		GFile *file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));
		char *basename = g_file_get_basename(file);

		/* Make sure it has a .inform suffix */
		if(!g_str_has_suffix(basename, ".inform")) {
			char *newbasename = g_strconcat(basename, ".inform", NULL);
			GFile *parent = g_file_get_parent(file);

			g_object_unref(file);
			file = g_file_get_child(parent, newbasename);

			g_free(basename);
			basename = newbasename;

			g_object_unref(parent);
		}

		gtk_widget_destroy(dialog);

		/* For "Select folder" mode, we must do our own confirmation */
		/* Adapted from gtkfilechooserdefault.c */
		/* Sourcefile is a workaround: if you type a new folder into the file
		selection dialog, GTK will create that folder automatically and it
		will then already exist */
		GFile *sourcefile = g_file_get_child(file, "Source");
		if (g_file_query_exists(file, NULL) && g_file_query_exists(sourcefile, NULL)) {
			dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
				_("A project named \"%s\" already exists. Do you want to replace it?"), basename);
			gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
				_("Replacing it will overwrite its contents."));
			gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
			GtkWidget *button = gtk_button_new_with_mnemonic(_("_Replace"));
			gtk_widget_set_can_default(button, TRUE);
			gtk_button_set_image(GTK_BUTTON(button), gtk_image_new_from_stock(GTK_STOCK_SAVE_AS, GTK_ICON_SIZE_BUTTON));
			gtk_widget_show(button);
			gtk_dialog_add_action_widget(GTK_DIALOG(dialog), button, GTK_RESPONSE_ACCEPT);
			gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog),
				GTK_RESPONSE_ACCEPT, GTK_RESPONSE_CANCEL,
				-1);
			gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);

			int response = gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);

			if (response != GTK_RESPONSE_ACCEPT) {
				g_free(basename);
				g_object_unref(sourcefile);
				return i7_story_run_save_dialog(document, default_file);
			}
		}

		g_free(basename);
		g_object_unref(sourcefile);
		return file;
	} else {
		gtk_widget_destroy(dialog);
		return NULL;
	}
}

static GtkTextView *
i7_story_get_default_view(I7Document *document)
{
	return GTK_TEXT_VIEW(I7_STORY(document)->panel[LEFT]->source_tabs[I7_SOURCE_VIEW_TAB_SOURCE]);
}

static void
i7_story_scroll_to_selection(I7Document *document)
{
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(i7_document_get_buffer(document));
	int side = i7_story_choose_panel(I7_STORY(document), I7_PANE_SOURCE);
	GtkTextView *view = GTK_TEXT_VIEW(I7_STORY(document)->panel[side]->source_tabs[I7_SOURCE_VIEW_TAB_SOURCE]);
	gtk_notebook_set_current_page(GTK_NOTEBOOK(I7_STORY(document)->panel[side]->notebook), I7_PANE_SOURCE);
	gtk_text_view_scroll_to_mark(view, gtk_text_buffer_get_insert(buffer), 0.25, FALSE, 0.0, 0.0);
	gtk_widget_grab_focus(GTK_WIDGET(view));
}

/* Only update the tabs in this main window */
static void
i7_story_update_tabs(I7Document *document)
{
	if(!I7_IS_STORY(document))
		return;
	I7Story *story = I7_STORY(document);
	i7_panel_update_tabs(story->panel[LEFT]);
	i7_panel_update_tabs(story->panel[RIGHT]);
}

/* Update the fonts in this main window, but not the
widgets that only need their font size updated */
static void
i7_story_update_fonts(I7Document *document)
{
	if(!I7_IS_STORY(document))
		return;
	I7Story *story = I7_STORY(document);
	I7_STORY_USE_PRIVATE(story, priv);
	i7_panel_update_fonts(story->panel[LEFT]);
	i7_panel_update_fonts(story->panel[RIGHT]);
	PangoFontDescription *font = get_font_description();
	i7_skein_set_font(priv->skein, font);
	pango_font_description_free(font);
	update_font(story->notes_view);
}

/* Update only the font sizes in this main window */
static void
i7_story_update_font_sizes(I7Document *document)
{
	if(!I7_IS_STORY(document))
		return;
	I7Story *story = I7_STORY(document);
	I7_STORY_USE_PRIVATE(story, priv);
	i7_panel_update_font_sizes(story->panel[LEFT]);
	i7_panel_update_font_sizes(story->panel[RIGHT]);
	PangoFontDescription *font = get_font_description();
	i7_skein_set_font(priv->skein, font);
	pango_font_description_free(font);
}

static void
i7_story_expand_headings_view(I7Document *document)
{
	I7Story *story = I7_STORY(document);
	gtk_tree_view_expand_all(GTK_TREE_VIEW(story->panel[LEFT]->source_tabs[I7_SOURCE_VIEW_TAB_CONTENTS]));
	gtk_tree_view_expand_all(GTK_TREE_VIEW(story->panel[RIGHT]->source_tabs[I7_SOURCE_VIEW_TAB_CONTENTS]));
}

static void
i7_story_set_contents_display(I7Document *document, I7ContentsDisplay display)
{
	I7Story *story = I7_STORY(document);
	i7_source_view_set_contents_display(story->panel[LEFT]->sourceview, display);
	i7_source_view_set_contents_display(story->panel[RIGHT]->sourceview, display);
}

/* Returns the currently focused view in either the left or right pane. Returns
NULL if there is not really a view, like on the Settings tab. */
static GtkWidget *
get_focus_view(I7Story *story)
{
	I7_STORY_USE_PRIVATE(story, priv);
	I7Panel *panel = I7_PANEL(priv->last_focused);
	if(!panel)
		panel = story->panel[LEFT];
	switch(gtk_notebook_get_current_page(GTK_NOTEBOOK(panel->notebook))) {
		case I7_PANE_SETTINGS:
		case I7_PANE_SKEIN:
		case I7_PANE_TRANSCRIPT:
			return NULL;
		case I7_PANE_STORY:
			return panel->tabs[I7_PANE_STORY];
		case I7_PANE_SOURCE:
			return panel->source_tabs[gtk_notebook_get_current_page(GTK_NOTEBOOK(panel->tabs[I7_PANE_SOURCE]))];
		case I7_PANE_RESULTS:
			return panel->results_tabs[gtk_notebook_get_current_page(GTK_NOTEBOOK(panel->tabs[I7_PANE_RESULTS]))];
		case I7_PANE_INDEX:
			return panel->index_tabs[gtk_notebook_get_current_page(GTK_NOTEBOOK(panel->tabs[I7_PANE_INDEX]))];
		case I7_PANE_DOCUMENTATION:
			return panel->tabs[I7_PANE_DOCUMENTATION];
		case I7_PANE_EXTENSIONS:
			return panel->tabs[I7_PANE_EXTENSIONS];
		default:
			g_assert_not_reached();
	}
}

static gboolean
do_search(GtkTextView *view, const gchar *text, gboolean forward, const GtkTextIter *startpos, GtkTextIter *start, GtkTextIter *end)
{
	if(forward)
		return gtk_text_iter_forward_search(startpos, text, GTK_TEXT_SEARCH_VISIBLE_ONLY | GTK_TEXT_SEARCH_TEXT_ONLY | GTK_TEXT_SEARCH_CASE_INSENSITIVE, start, end, NULL);
	return gtk_text_iter_backward_search(startpos, text, GTK_TEXT_SEARCH_VISIBLE_ONLY | GTK_TEXT_SEARCH_TEXT_ONLY | GTK_TEXT_SEARCH_CASE_INSENSITIVE, start, end, NULL);
}

static gboolean
i7_story_highlight_search(I7Document *document, const gchar *text, gboolean forward)
{
	if(*text == '\0') {
		/* If the text is blank, unhighlight everything and return TRUE so the
		find entry doesn't stay red on a WebView */
		i7_document_unhighlight_quicksearch(document);
		return TRUE;
	}

	GtkWidget *focus = get_focus_view(I7_STORY(document));
	if(!focus)
		return TRUE;

	if(GTK_IS_TREE_VIEW(focus)) {
		/* Headings view is visible, switch back to source code view */
		I7Panel *panel = I7_PANEL(I7_STORY_PRIVATE(document)->last_focused);
		gtk_notebook_set_current_page(GTK_NOTEBOOK(panel->tabs[I7_PANE_SOURCE]), I7_SOURCE_VIEW_TAB_SOURCE);
		focus = panel->source_tabs[I7_SOURCE_VIEW_TAB_SOURCE];
		gtk_widget_grab_focus(document->findbar_entry);
	}

	i7_document_set_highlighted_view(document, focus);

	if(GTK_IS_TEXT_VIEW(focus)) {
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
	} else if(WEBKIT_IS_WEB_VIEW(focus)) {
		WebKitFindController *controller = webkit_web_view_get_find_controller(WEBKIT_WEB_VIEW(focus));
		webkit_find_controller_search(controller, text, WEBKIT_FIND_OPTIONS_CASE_INSENSITIVE | WEBKIT_FIND_OPTIONS_WRAP_AROUND, /* max matches? */ 0);
	} /* else do nothing */
	return TRUE;
}

static void
i7_story_set_spellcheck(I7Document *document, gboolean spellcheck)
{
	i7_source_view_set_spellcheck(I7_STORY(document)->panel[LEFT]->sourceview, spellcheck);
	i7_source_view_set_spellcheck(I7_STORY(document)->panel[RIGHT]->sourceview, spellcheck);
}

static void
i7_story_check_spelling(I7Document *document)
{
	i7_source_view_check_spelling(I7_STORY(document)->panel[LEFT]->sourceview);
	i7_source_view_check_spelling(I7_STORY(document)->panel[RIGHT]->sourceview);
}

/* In addition to the logic in i7_document_can_revert(), also require that the
story's GFile points to a directory. */
static gboolean
i7_story_can_revert(I7Document *document)
{
	GFile *file = i7_document_get_file(document);
	gboolean retval = (g_file_query_file_type(file, G_FILE_QUERY_INFO_NONE, NULL) == G_FILE_TYPE_DIRECTORY);
	g_object_unref (file);
	return retval;
}

static void
i7_story_revert(I7Document *document)
{
	GFile *file = i7_document_get_file(document);
	i7_story_open(I7_STORY(document), file);
	g_object_unref(file);
}

/* TYPE SYSTEM */

static void
story_init_panel(I7Story *self, I7Panel *panel, PangoFontDescription *font)
{
	I7_STORY_USE_PRIVATE(self, priv);

	gtk_widget_show(GTK_WIDGET(panel));

	/* Connect other signals and properties */
	g_signal_connect(panel->sourceview->heading_depth, "value-changed", G_CALLBACK(on_heading_depth_value_changed), self);
	g_signal_connect(panel->z8, "toggled", G_CALLBACK(on_z8_button_toggled), self);
	g_signal_connect(panel->glulx, "toggled", G_CALLBACK(on_glulx_button_toggled), self);
	g_object_bind_property(self, "create-blorb", panel->blorb, "active", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
	g_object_bind_property(self, "nobble-rng", panel->nobble_rng, "active", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
	g_signal_connect(panel->tabs[I7_PANE_SOURCE], "switch-page", G_CALLBACK(on_source_notebook_switch_page), self);
	g_signal_connect(panel->source_tabs[I7_SOURCE_VIEW_TAB_CONTENTS], "row-activated", G_CALLBACK(on_headings_row_activated), self);
	g_signal_connect(panel, "select-view", G_CALLBACK(on_panel_select_view), self);
	g_signal_connect(panel, "paste-code", G_CALLBACK(on_panel_paste_code), self);
	g_signal_connect(panel, "jump-to-line", G_CALLBACK(on_panel_jump_to_line), self);
	g_signal_connect(panel, "display-docpage", G_CALLBACK(on_panel_display_docpage), self);
	g_signal_connect(panel, "display-extensions-docpage", G_CALLBACK(on_panel_display_extensions_docpage), self);
	g_signal_connect(panel, "display-index-page", G_CALLBACK(on_panel_display_index_page), self);
	g_signal_connect(priv->skein, "labels-changed", G_CALLBACK(on_labels_changed), panel);
	g_signal_connect(priv->skein, "show-node", G_CALLBACK(on_show_node), panel);
	g_signal_connect(panel->tabs[I7_PANE_SKEIN], "node-menu-popup", G_CALLBACK(on_node_popup), NULL);
	g_signal_connect(panel->tabs[I7_PANE_STORY], "started", G_CALLBACK(on_game_started), self);
	g_signal_connect(panel->tabs[I7_PANE_STORY], "stopped", G_CALLBACK(on_game_stopped), self);
	g_signal_connect(panel->tabs[I7_PANE_STORY], "command", G_CALLBACK(on_game_command), self);

	/* Connect various models to various views */
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(panel->source_tabs[I7_SOURCE_VIEW_TAB_SOURCE]), GTK_TEXT_BUFFER(i7_document_get_buffer(I7_DOCUMENT(self))));
	gtk_tree_view_set_model(GTK_TREE_VIEW(panel->source_tabs[I7_SOURCE_VIEW_TAB_CONTENTS]), i7_document_get_headings(I7_DOCUMENT(self)));
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(panel->results_tabs[I7_RESULTS_TAB_PROGRESS]), priv->progress);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(panel->results_tabs[I7_RESULTS_TAB_DEBUGGING]), priv->debug_log);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(panel->results_tabs[I7_RESULTS_TAB_INFORM6]), GTK_TEXT_BUFFER(priv->i6_source));
	i7_skein_view_set_skein(I7_SKEIN_VIEW(panel->tabs[I7_PANE_SKEIN]), priv->skein);
	gtk_tree_view_set_model(GTK_TREE_VIEW(panel->tabs[I7_PANE_TRANSCRIPT]), GTK_TREE_MODEL(priv->skein));
	
	/* Set the Results/Progress to a monospace font */
	gtk_widget_modify_font(GTK_WIDGET(panel->results_tabs[I7_RESULTS_TAB_PROGRESS]), font);

	/* Connect the Previous Section and Next Section actions to the up and down buttons */
	gtk_activatable_set_related_action(GTK_ACTIVATABLE(panel->sourceview->previous), I7_DOCUMENT(self)->previous_section);
	gtk_activatable_set_related_action(GTK_ACTIVATABLE(panel->sourceview->next), I7_DOCUMENT(self)->next_section);

	/* Set the Blorb resource-loading callback */
	chimara_glk_set_resource_load_callback(CHIMARA_GLK(panel->tabs[I7_PANE_STORY]), (ChimaraResourceLoadFunc)load_blorb_resource, self, NULL);
}

static void
i7_story_init(I7Story *self)
{
	I7_STORY_USE_PRIVATE(self, priv);
	I7App *theapp = i7_app_get();
	GSettings *state = i7_app_get_state(theapp);
	GSettings *prefs = i7_app_get_prefs(theapp);
	priv->skein_settings = g_settings_new(SCHEMA_SKEIN);
	GError *error = NULL;
	int w, h, x, y;

	/* Build the interface */
	GFile *file = i7_app_get_data_file_va(theapp, "ui", "story.ui", NULL);
	GtkBuilder *builder = create_new_builder(file, self);
	g_object_unref(file);

	/* Make the action groups */
	priv->story_action_group = GTK_ACTION_GROUP(load_object(builder, "story_actions"));
	priv->compile_action_group = GTK_ACTION_GROUP(load_object(builder, "compile_actions"));

	/* Build the menus and toolbars from the GtkUIManager file */
	gtk_ui_manager_insert_action_group(I7_DOCUMENT(self)->ui_manager, priv->story_action_group, 0);
	gtk_ui_manager_insert_action_group(I7_DOCUMENT(self)->ui_manager, priv->compile_action_group, 1);
	file = i7_app_get_data_file_va(theapp, "ui", "story.uimanager.xml", NULL);
	char *path = g_file_get_path(file);
	gtk_ui_manager_add_ui_from_file(I7_DOCUMENT(self)->ui_manager, path, &error);
	g_free(path);
	g_object_unref(file);
	if(error)
		ERROR(_("Building menus failed"), error);
	GtkWidget *menu = gtk_ui_manager_get_widget(I7_DOCUMENT(self)->ui_manager, "/StoryMenubar");
	I7_DOCUMENT(self)->toolbar = gtk_ui_manager_get_widget(I7_DOCUMENT(self)->ui_manager, "/MainToolbar");
	gtk_widget_set_no_show_all(I7_DOCUMENT(self)->toolbar, TRUE);
	i7_document_add_menus_and_findbar(I7_DOCUMENT(self));

	/* Set the initial visible state of the toolbar based on the most recent
	choice the user made */
	gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(I7_DOCUMENT(self)->view_toolbar),
		g_settings_get_boolean(state, PREFS_STATE_SHOW_TOOLBAR));

	/* Build the rest of the interface */
	gtk_box_pack_start(GTK_BOX(I7_DOCUMENT(self)->box), menu, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(I7_DOCUMENT(self)->box), I7_DOCUMENT(self)->toolbar, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(I7_DOCUMENT(self)->box), I7_DOCUMENT(self)->findbar, FALSE, FALSE, 0);
	GtkToolItem *search_toolitem = gtk_tool_item_new();
	GtkWidget *search_entry = i7_placeholder_entry_new(_("Documentation"));
	gtk_container_add(GTK_CONTAINER(search_toolitem), search_entry);
	/* "activate" is a keybinding signal, but what else am I supposed to connect to? */
	g_signal_connect(search_entry, "activate", G_CALLBACK(on_search_entry_activate), self);
	gtk_widget_show_all(GTK_WIDGET(search_toolitem));
	gtk_toolbar_insert(GTK_TOOLBAR(I7_DOCUMENT(self)->toolbar), search_toolitem, 6);
	/* Add icons to the entry */
	gtk_entry_set_icon_from_stock(GTK_ENTRY(search_entry), GTK_ENTRY_ICON_PRIMARY, GTK_STOCK_FIND);
	gtk_entry_set_icon_from_stock(GTK_ENTRY(search_entry), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_CLEAR);
	gtk_entry_set_icon_activatable(GTK_ENTRY(search_entry), GTK_ENTRY_ICON_SECONDARY, TRUE);
	g_signal_connect(search_entry, "icon-press", G_CALLBACK(on_search_entry_icon_press), NULL);

	/* Save public pointers to other widgets */
	LOAD_WIDGET(facing_pages);
	LOAD_WIDGET(notes_window);
	LOAD_WIDGET(notes_view);
	LOAD_WIDGET(skein_spacing_dialog);
	LOAD_WIDGET(skein_spacing_horizontal);
	LOAD_WIDGET(skein_spacing_vertical);
	LOAD_WIDGET(skein_spacing_use_defaults);
	LOAD_WIDGET(skein_trim_dialog);
	LOAD_WIDGET(skein_trim_slider);

	/* Set up the signals to do the menu hints in the statusbar */
	i7_document_attach_menu_hints(I7_DOCUMENT(self), GTK_MENU_BAR(menu));

	/* Build the Open Extensions menu */
	i7_app_update_extensions_menu(theapp);

	/* Build the two panels */
	self->panel[LEFT] = I7_PANEL(i7_panel_new());
	self->panel[RIGHT] = I7_PANEL(i7_panel_new());
	GFile *docs_file = i7_app_get_data_file_va(theapp, "Documentation", "index.html", NULL);
	i7_panel_reset_queue(self->panel[LEFT], I7_PANE_SOURCE, I7_SOURCE_VIEW_TAB_SOURCE, NULL);
	i7_panel_goto_docpage(self->panel[LEFT], docs_file);
	i7_panel_reset_queue(self->panel[RIGHT], I7_PANE_DOCUMENTATION, 0, docs_file);
	g_object_unref(docs_file);
	gtk_paned_pack1(GTK_PANED(self->facing_pages), GTK_WIDGET(self->panel[LEFT]), TRUE, FALSE);
	gtk_paned_pack2(GTK_PANED(self->facing_pages), GTK_WIDGET(self->panel[RIGHT]), TRUE, FALSE);
	gtk_box_pack_start(GTK_BOX(I7_DOCUMENT(self)->box), self->facing_pages, TRUE, TRUE, 0);

	/* Builder object not needed anymore */
	g_object_unref(builder);

	/* Set the last saved window size and slider position */
	g_settings_get(state, PREFS_STATE_WINDOW_SIZE, "(ii)", &w, &h);
	gtk_window_resize(GTK_WINDOW(self), w, h);
	gtk_paned_set_position(GTK_PANED(self->facing_pages), g_settings_get_int(state, PREFS_STATE_DIVIDER_POS));

	/* Create the private properties */
	priv->notes = gtk_text_buffer_new(NULL);
	priv->last_focused = GTK_WIDGET(self->panel[LEFT]);
	priv->compile_finished_callback = NULL;
	priv->compile_finished_callback_data = NULL;
	priv->copy_blorb_dest_file = NULL;
	priv->compiler_output_file = NULL;
	priv->test_me = FALSE;
	priv->manifest = NULL;
	
	/* Set up the Skein */
	priv->skein = i7_skein_new();
	g_object_set(priv->skein,
		"unlocked-color", "#6865FF",
		NULL);
	g_signal_connect(priv->skein, "node-activate", G_CALLBACK(on_node_activate), self);
	g_signal_connect(priv->skein, "differs-badge-activate", G_CALLBACK(on_differs_badge_activate), self);
	g_signal_connect(priv->skein, "modified", G_CALLBACK(on_skein_modified), self);
	priv->skein_settings = g_settings_new("com.inform7.IDE.preferences.skein");
	g_settings_bind(priv->skein_settings, "horizontal-spacing",
		gtk_range_get_adjustment(GTK_RANGE(self->skein_spacing_horizontal)), "value", G_SETTINGS_BIND_DEFAULT);
	g_settings_bind(priv->skein_settings, "vertical-spacing",
		gtk_range_get_adjustment(GTK_RANGE(self->skein_spacing_vertical)), "value", G_SETTINGS_BIND_DEFAULT);

	/* Set up the Notes window */
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(self->notes_view), priv->notes);
	gtk_window_set_keep_above(GTK_WINDOW(self->notes_window), TRUE);
	g_settings_get(state, PREFS_STATE_NOTEPAD_SIZE, "(ii)", &w, &h);
	g_settings_get(state, PREFS_STATE_NOTEPAD_POS, "(ii)", &x, &y);
	gtk_window_resize(GTK_WINDOW(self->notes_window), w, h);
	gtk_window_move(GTK_WINDOW(self->notes_window), x, y);
	if(g_settings_get_boolean(state, PREFS_STATE_SHOW_NOTEPAD))
		gtk_widget_show(self->notes_window);

	/* Set up the Natural Inform highlighting */
	GtkSourceBuffer *buffer = i7_document_get_buffer(I7_DOCUMENT(self));
	set_buffer_language(buffer, "inform7");
	gtk_source_buffer_set_style_scheme(buffer, i7_app_get_current_color_scheme(theapp));

	/* Create a text buffer for the Progress, Debugging and I6 text views */
	priv->progress = gtk_text_buffer_new(NULL);
	priv->debug_log = gtk_text_buffer_new(NULL);
	priv->i6_source = create_inform6_source_buffer();

	/* Create a monospace font description for the Results/Progress views */
	PangoFontDescription *font = get_desktop_monospace_font();
	pango_font_description_set_size(font, get_font_size(font));

	/* Do the default settings */
	priv->settings = create_default_settings();

	/* Do panel-specific stuff to the left and then the right panel */
	i7_story_foreach_panel(self, (I7PanelForeachFunc)story_init_panel, font);
	pango_font_description_free(font);

	/* Now the buffers and models are owned by the views, so dereference them */
	g_object_unref(buffer);
	g_object_unref(i7_document_get_headings(I7_DOCUMENT(self)));
	g_object_unref(priv->notes);
	g_object_unref(priv->progress);
	g_object_unref(priv->debug_log);
	g_object_unref(priv->i6_source);

	/* Set up the Previous Section and Next Section actions to synch with the buttons */
	g_signal_connect(I7_DOCUMENT(self)->previous_section, "notify::sensitive", G_CALLBACK(on_previous_action_notify_sensitive), self);
	g_signal_connect(I7_DOCUMENT(self)->next_section, "notify::sensitive", G_CALLBACK(on_next_action_notify_sensitive), self);
	/* For some reason this needs to be triggered even if the buttons are set to invisible in Glade */
	gtk_action_set_sensitive(I7_DOCUMENT(self)->previous_section, FALSE);
	gtk_action_set_sensitive(I7_DOCUMENT(self)->next_section, FALSE);

	/* Add extra pages in "Results" if the user has them turned on */
	if(g_settings_get_boolean(prefs, PREFS_SHOW_DEBUG_LOG))
		i7_story_add_debug_tabs(I7_DOCUMENT(self));

	/* Connect the widgets on the Settings pane to the settings properties */
	g_object_bind_property(self->panel[LEFT]->z8, "active", self->panel[RIGHT]->z8, "active", G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(self->panel[LEFT]->glulx, "active", self->panel[RIGHT]->glulx, "active", G_BINDING_BIDIRECTIONAL);
	g_signal_connect(self, "notify::story-format", G_CALLBACK(on_notify_story_format), NULL);
	g_signal_connect(self, "notify::elastic-tabstops", G_CALLBACK(on_notify_elastic_tabstops), NULL);

	/* Set font sizes, etc. */
	i7_document_update_fonts(I7_DOCUMENT(self));

	/* Set spell checking */
	gboolean spell_check_default = g_settings_get_boolean(state, PREFS_STATE_SPELL_CHECK);
	gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(I7_DOCUMENT(self)->autocheck_spelling), spell_check_default);
	i7_document_set_spellcheck(I7_DOCUMENT(self), spell_check_default);

	/* Make the Skein dialogs transient */
	gtk_window_set_transient_for(GTK_WINDOW(self->skein_spacing_dialog), GTK_WINDOW(self));
	gtk_window_set_transient_for(GTK_WINDOW(self->skein_trim_dialog), GTK_WINDOW(self));
	
	/* Create a callback for the delete event */
	g_signal_connect(self, "delete-event", G_CALLBACK(on_storywindow_delete_event), NULL);
}

static void
i7_story_finalize(GObject *self)
{
	I7_STORY_USE_PRIVATE(self, priv);
	if(priv->copy_blorb_dest_file)
		g_object_unref(priv->copy_blorb_dest_file);
	if(priv->compiler_output_file)
		g_object_unref(priv->compiler_output_file);
	if(priv->settings)
		plist_object_free(priv->settings);
	if(priv->manifest)
		plist_object_free(priv->manifest);
	G_OBJECT_CLASS(i7_story_parent_class)->finalize(self);
}

static void
i7_story_class_init(I7StoryClass *klass)
{
	/* Parent class overrides */
	I7DocumentClass *document_class = I7_DOCUMENT_CLASS(klass);
	document_class->extract_title = i7_story_extract_title;
	document_class->set_contents_display = i7_story_set_contents_display;
	document_class->get_default_view = i7_story_get_default_view;
	document_class->save = i7_story_save;
	document_class->save_as = i7_story_save_as;
	document_class->run_save_dialog = i7_story_run_save_dialog;
	document_class->scroll_to_selection = i7_story_scroll_to_selection;
	document_class->update_tabs = i7_story_update_tabs;
	document_class->update_fonts = i7_story_update_fonts;
	document_class->update_font_sizes = i7_story_update_font_sizes;
	document_class->expand_headings_view = i7_story_expand_headings_view;
	document_class->highlight_search = i7_story_highlight_search;
	document_class->set_spellcheck = i7_story_set_spellcheck;
	document_class->check_spelling = i7_story_check_spelling;
	document_class->set_elastic_tabstops = i7_story_set_elastic_tabstops;
	document_class->can_revert = i7_story_can_revert;
	document_class->revert = i7_story_revert;

	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->set_property = i7_story_set_property;
	object_class->get_property = i7_story_get_property;
	object_class->finalize = i7_story_finalize;

	/* Properties */
	g_object_class_install_property(object_class, PROP_STORY_FORMAT,
		g_param_spec_uint("story-format", "Story Format",
			"IFOutputSettings->IFSettingZCodeVersion", 5, 256, 256,
			G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS));
	g_object_class_install_property(object_class, PROP_MAKE_BLORB,
		g_param_spec_boolean("create-blorb", "Create Blorb file on release",
			"IFOutputSettings->IFSettingCreateBlorb", TRUE,
			G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS));
	g_object_class_install_property(object_class, PROP_NOBBLE_RNG,
		g_param_spec_boolean("nobble-rng", "Nobble RNG",
			"IFOutputSettings->IFSettingNobbleRNG", FALSE,
			G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS));
	g_object_class_install_property(object_class, PROP_ELASTIC_TABSTOPS,
		g_param_spec_boolean("elastic-tabstops", "Elastic Tabstops",
			"IFMiscSettings->IFSettingElasticTabs", FALSE,
			G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS));

	/* Private data */
	g_type_class_add_private(klass, sizeof(I7StoryPrivate));
}

/* PUBLIC FUNCTIONS */

I7Story *
i7_story_new(I7App *app, GFile *file, const char *title, const char *author)
{
	/* Can take a while for old versions of WebKit */
	i7_app_set_busy(app, TRUE);
	I7Story *story = I7_STORY(g_object_new(I7_TYPE_STORY, NULL));

	i7_document_set_file(I7_DOCUMENT(story), file);

	gchar *text = g_strconcat("\"", title, "\" by \"", author, "\"\n", NULL);
	i7_document_set_source_text(I7_DOCUMENT(story), text);
	i7_document_set_modified(I7_DOCUMENT(story), TRUE);

	/* Add document to global list */
	i7_app_register_document(app, I7_DOCUMENT(story));

	i7_app_set_busy(app, FALSE);

	/* Bring window to front */
	gtk_widget_show(GTK_WIDGET(story));
	gtk_window_present(GTK_WINDOW(story));
	return story;
}

I7Story *
i7_story_new_from_file(I7App *app, GFile *file)
{
	GFile *real_file;

	/* Make sure the GFile doesn't refer to story.ni */
	char *basename = g_file_get_basename(file);
	if(strcmp(basename, "story.ni") == 0) {
		real_file = g_file_resolve_relative_path(file, "../..");
	} else {
		real_file = g_object_ref(file);
	}
	g_free(basename);

	I7Document *dupl = i7_app_get_already_open(app, real_file);

	if(dupl && I7_IS_STORY(dupl)) {
		gtk_window_present(GTK_WINDOW(dupl));
		g_object_unref(real_file);
		return NULL;
	}

	/* Loading a large story file can take a while */
	i7_app_set_busy(app, TRUE);
	I7Story *story = I7_STORY(g_object_new(I7_TYPE_STORY, NULL));
	if(!i7_story_open(story, real_file)) {
		gtk_widget_destroy(GTK_WIDGET(story));
		g_object_unref(real_file);
		i7_app_set_busy(app, FALSE);
		return NULL;
	}

	/* Add document to global list */
	i7_app_register_document(app, I7_DOCUMENT(story));

	i7_app_set_busy(app, FALSE);

	/* Bring window to front */
	gtk_widget_show(GTK_WIDGET(story));
	gtk_window_present(GTK_WINDOW(story));

	g_object_unref(real_file);

	return story;
}

I7Story *
i7_story_new_from_dialog(I7App *app)
{
	I7Story *story = NULL;

	/* Create a file chooser for *.inform. It actually selects folders, because
	that's what Inform projects are. */
	GtkWidget *chooser = gtk_file_chooser_dialog_new(_("Open Project"), NULL, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
		NULL);

	//GtkFileFilter *filter = gtk_file_filter_new();
	//gtk_file_filter_add_pattern(filter, "*.inform");
	//gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(chooser), filter);

	if(gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_ACCEPT) {
		GFile *file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(chooser));
		story = i7_story_new_from_file(app, file);
		g_object_unref(file);
	}

	gtk_widget_destroy(chooser);
	return story;
}

/* Read a project directory, loading all the appropriate files into story and
returning success */
gboolean
i7_story_open(I7Story *story, GFile *file)
{
	I7_STORY_USE_PRIVATE(story, priv);
	GError *err = NULL;

	GFile *source_file = g_file_get_child(file, "Source");
	GFile *story_file = g_file_get_child(source_file, "story.ni");
	g_object_unref(source_file);

	g_object_ref(file);

	/* Make sure that the file has the proper extension */
	char *display_name = file_get_display_name(file);
	if(!g_str_has_suffix(display_name, ".inform")) {

		if(g_file_query_exists(story_file, NULL)) {
			/* This seems to be an Inform project, but with the wrong extension */
			GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(story), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			    GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
			    _("This project doesn't have a .inform extension."));
			gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			    _("This extension is required for Inform story files to work "
				    "on all platforms. The project will be renamed."));
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);

			/* Rename the file */
			char *new_name = g_strconcat(display_name, ".inform", NULL);
			GFile *old_file = file;
			file = g_file_set_display_name(file, new_name, NULL, &err);
			if(file == NULL) {
				IO_ERROR_DIALOG(GTK_WINDOW(story), old_file, err, _("renaming the project file to a .inform extension"));
				file = old_file;
				goto fail;
			}
			g_object_unref(old_file);
			g_free(new_name);

			g_object_unref(story_file);
			source_file = g_file_get_child(file, "Source");
			story_file = g_file_get_child(source_file, "story.ni");
			g_object_unref(source_file);

		} else {
			/* This doesn't seem to be an Inform project */
			char *path = g_file_get_path(file);
			GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(story), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			    GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
			    _("The file \"%s\" doesn't seem to be an Inform story file."), display_name);
			gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			    _("Make sure you are opening the correct file. This file was "
				"located at: %s"), path);
			g_free(path);
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
			goto fail;
		}
	}
	g_free(display_name);

	i7_document_set_file(I7_DOCUMENT(story), file);

	/* Read the source */
	char *text = read_source_file(story_file);
	if(!text)
		goto fail2;

	update_recent_story_file(story, file);

	/* Watch for changes to the source file */
	i7_document_monitor_file(I7_DOCUMENT(story), story_file);
	g_object_unref(story_file);

	/* Write the source to the source buffer, clearing the undo history */
	i7_document_set_source_text(I7_DOCUMENT(story), text);
	g_free(text);

	/* Read the skein */
	GFile *skein_file = g_file_get_child(file, "Skein.skein");
	if(!i7_skein_load(priv->skein, skein_file, &err)) {
		error_dialog(GTK_WINDOW(story), err, _("This project's Skein was not found, or it was unreadable."));
		err = NULL;
	}
	g_object_unref(skein_file);

	/* Read the notes */
	GFile *notes_file = g_file_get_child(file, "notes.rtf");
	if(!rtf_text_buffer_import_file(priv->notes, notes_file, NULL, NULL)) {
		/* Don't fail if the file is unreadable, instead just make some blank notes */
		gtk_text_buffer_set_text(priv->notes, "", -1);
	}
	g_object_unref(notes_file);
	/* Remove all the formatting tags, we don't do formatting in this editor */
	GtkTextIter start, end;
	gtk_text_buffer_get_bounds(priv->notes, &start, &end);
	gtk_text_buffer_remove_all_tags(priv->notes, &start, &end);
	gtk_text_buffer_set_modified(priv->notes, FALSE);

	/* Read the settings */
	GFile *settings_file = g_file_get_child(file, "Settings.plist");
	plist_object_free(priv->settings);
	priv->settings = plist_read_file(settings_file, NULL, &err);
	if(!priv->settings) {
		priv->settings = create_default_settings();
		error_dialog(GTK_WINDOW(story), err,
			_("Could not open the project's settings file. "
			"Using default settings."));
	}
	g_object_unref(settings_file);

	/* Silently convert old Z5 or Z6 projects to Z8 */
	PlistObject *story_format = plist_object_lookup(priv->settings, "IFOutputSettings", "IFSettingZCodeVersion", -1);
	if(story_format) {
		int format = plist_object_get_integer(story_format);
		if(format == I7_STORY_FORMAT_Z5 || format == I7_STORY_FORMAT_Z6)
			plist_object_set_integer(story_format, I7_STORY_FORMAT_Z8);
	}

	/* Update the GUI with the new settings */
	g_object_notify(G_OBJECT(story), "story-format");
	g_object_notify(G_OBJECT(story), "create-blorb");
	g_object_notify(G_OBJECT(story), "nobble-rng");
	g_object_notify(G_OBJECT(story), "elastic-tabstops");

	/* Load index tabs if they exist */
	i7_story_reload_index_tabs(story, FALSE);

	/* Check if the story uses the old-style name for the Materials folder, and if
	so, quietly rename it */
	GFile *materials_file = i7_story_get_materials_file(story);
	if(!g_file_query_exists(materials_file, NULL)) {
		GFile *old_materials_file = i7_story_get_old_materials_file(story);
		if(g_file_query_exists(old_materials_file, NULL)) {
			GError *error = NULL;
			gboolean success = g_file_move(old_materials_file, materials_file,
				G_FILE_COPY_NO_FALLBACK_FOR_MOVE /* fallback doesn't support moving folders */,
				NULL, NULL, NULL, &error);
			if(!success) {
				/* fail silently */
				g_message("Error copying old Materials folder to new one: %s", error->message);
				g_clear_error(&error);
			} else {
				file_set_custom_icon(materials_file, "application-x-inform-materials");
			}
		}
		g_object_unref(old_materials_file);
	}
	g_object_unref(materials_file);

	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(i7_document_get_buffer(I7_DOCUMENT(story)));
	gtk_text_buffer_get_start_iter(buffer, &start);
	gtk_text_buffer_place_cursor(buffer, &start);

	i7_document_set_modified(I7_DOCUMENT(story), FALSE);

	g_object_unref(file);
	return TRUE;

fail:
	g_free(display_name);
fail2:
	remove_recent_story_file(file);
	g_object_unref(file);
	return FALSE;
}

/**
 * i7_story_get_focus_panel:
 * @story: the story
 *
 * Tries to find out which panel is currently focused.
 *
 * Returns: a value from #I7StoryPanel, %RIGHT or %LEFT, or -1 if neither side
 * was focused.
 */
int
i7_story_get_focus_panel(I7Story *story)
{
	I7Panel *focus_child = I7_PANEL(gtk_container_get_focus_child(GTK_CONTAINER(story->facing_pages)));
	if(focus_child == story->panel[LEFT])
		return LEFT;
	if(focus_child == story->panel[RIGHT])
		return RIGHT;
	return -1;
}

/**
 * i7_story_choose_panel:
 * @story: the story
 * @newpane: the #I7PanelPane that we want to display
 *
 * Chooses an appropriate panel to display pane number @newpane in.
 *
 * Returns: a value from #I7StoryPanel, %RIGHT or %LEFT.
 */
I7StoryPanel
i7_story_choose_panel(I7Story *story, I7PanelPane newpane)
{
	int left = gtk_notebook_get_current_page(GTK_NOTEBOOK(story->panel[LEFT]->notebook));
	int right = gtk_notebook_get_current_page(GTK_NOTEBOOK(story->panel[RIGHT]->notebook));
	int originating_panel = i7_story_get_focus_panel(story);

	/* If both panels are showing the same thing and that thing is also where we
	want to go, then use the same panel where the action originated in order to
	minimize disruption */
	if(G_UNLIKELY(left == right && left == newpane && originating_panel != -1))
		return originating_panel;

	/* If either panel is showing the same as the new, use that */
	if(right == newpane)
		return RIGHT;
	if(left == newpane)
		return LEFT;
	/* If the Skein is showing, use the other side for the Transcript */
	if(newpane == I7_PANE_TRANSCRIPT) {
		if(right == I7_PANE_SKEIN)
			return LEFT;
		if(left == I7_PANE_SKEIN)
			return RIGHT;
	}
	/* Always try to use the left panel for source */
	if(newpane == I7_PANE_SOURCE)
		return LEFT;
	/* If the right panel is not source, use that */
	if(right != I7_PANE_SOURCE)
		return RIGHT;
	/* Use the left panel unless that is source too */
	return (left == I7_PANE_SOURCE)? RIGHT : LEFT;
}

void
i7_story_show_pane(I7Story *story, I7PanelPane pane)
{
	I7StoryPanel side = i7_story_choose_panel(story, pane);
	gtk_notebook_set_current_page(GTK_NOTEBOOK(story->panel[side]->notebook), pane);
}

void
i7_story_show_tab(I7Story *story, I7PanelPane pane, gint tab)
{
	I7StoryPanel side = i7_story_choose_panel(story, pane);
	gtk_notebook_set_current_page(GTK_NOTEBOOK(story->panel[side]->tabs[pane]), tab);
	gtk_notebook_set_current_page(GTK_NOTEBOOK(story->panel[side]->notebook), pane);
}

void
i7_story_show_docpage(I7Story *story, GFile *file)
{
	I7StoryPanel side = i7_story_choose_panel(story, I7_PANE_DOCUMENTATION);
	i7_panel_goto_docpage(story->panel[side], file);
	gtk_notebook_set_current_page(GTK_NOTEBOOK(story->panel[side]->notebook), I7_PANE_DOCUMENTATION);
}

void
i7_story_show_docpage_at_anchor(I7Story *story, GFile *file, const char *anchor)
{
	I7StoryPanel side = i7_story_choose_panel(story, I7_PANE_DOCUMENTATION);
	i7_panel_goto_docpage_at_anchor(story->panel[side], file, anchor);
	gtk_notebook_set_current_page(GTK_NOTEBOOK(story->panel[side]->notebook), I7_PANE_DOCUMENTATION);
}

/* Helper function: work out the project file's root name given the project file
itself. Free return value when done. */
static char *
project_file_to_root_name(GFile *project_file)
{
	char *base = g_file_get_basename(project_file);
	g_assert(g_str_has_suffix(base, ".inform"));
	char *root_name = g_strndup(base, strlen(base) - 7); /* lose extension */
	g_free(base);
	return root_name;
}

/* Helper function: append suffix to project file's root name to form the
Materials folder name. Returns a GFile, unref when done. */
static GFile *
story_to_materials_file(I7Story *story, const char *suffix)
{
	GFile *project_file = i7_document_get_file(I7_DOCUMENT(story));
	char *projectname = project_file_to_root_name(project_file);
	char *materialsname = g_strconcat(projectname, suffix, NULL);
	GFile *parent = g_file_get_parent(project_file);
	GFile *materials_file = g_file_get_child(parent, materialsname);

	g_object_unref(parent);
	g_object_unref(project_file);
	g_free(projectname);
	g_free(materialsname);

	return materials_file;
}

/**
 * i7_story_get_old_materials_file:
 * @story: the story
 *
 * Work out the old-style location of the Materials folder; this function is for
 * the purposes of renaming the old-style Materials folder to the new-style.
 * Adapted from OS X source.
 *
 * Returns: (transfer full): a #GFile pointing to the old-style Materials
 * folder. The file itself may not exist. Unref when done.
 */
GFile *
i7_story_get_old_materials_file(I7Story *story)
{
	return story_to_materials_file(story, " Materials");
}

/**
 * i7_story_get_materials_file:
 * @story: the story
 *
 * Work out the location of the Materials folder.
 * Adapted from OS X source.
 *
 * Returns: (transfer full): a #GFile pointing to the Materials folder.
 * The file itself may not exist. Unref when done.
 */
GFile *
i7_story_get_materials_file(I7Story *story)
{
	return story_to_materials_file(story, ".materials");
}

/* Return the extension of the output file of this story.
 Do not free return value. */
const gchar *
i7_story_get_extension(I7Story *story)
{
	switch(i7_story_get_story_format(story)) {
		case I7_STORY_FORMAT_Z8:
			return "z8";
		case I7_STORY_FORMAT_GLULX:
			return "ulx";
		default:
			;
	}
	g_assert_not_reached();
	return "error";
}

/* Execute @func for both panels */
void
i7_story_foreach_panel(I7Story *story, I7PanelForeachFunc func, gpointer data)
{
	int side;
	/* Execute for both panels */
	for(side = LEFT; side < I7_STORY_NUM_PANELS; side++)
		func(story, story->panel[side], data);
}
