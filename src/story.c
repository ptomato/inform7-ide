/* This file is part of GNOME Inform 7.
 * Copyright (c) 2006-2009 P. F. Chimento <philip.chimento@gmail.com>
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

#include <errno.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <webkit/webkit.h>
#include <gtksourceview/gtksourceiter.h>
#include "osxcart/plist.h"
#include "osxcart/rtf.h"
#include "libchimara/chimara-glk.h"
#include "libchimara/chimara-if.h"
#include "story.h"
#include "story-private.h"
#include "app.h"
#include "builder.h"
#include "colorscheme.h"
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

enum {
	PROP_0,
	PROP_STORY_FORMAT,
	PROP_MAKE_BLORB,
	PROP_NOBBLE_RNG,
	PROP_ELASTIC_TABS
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
    gint w, h, x, y;
    gtk_window_get_size(GTK_WINDOW(story), &w, &h);
    config_file_set_int(PREFS_APP_WINDOW_WIDTH, w);
    config_file_set_int(PREFS_APP_WINDOW_HEIGHT, h);
    config_file_set_int(PREFS_SLIDER_POSITION, gtk_paned_get_position(GTK_PANED(story->facing_pages)));
	/* Also save the notepad window */
	gtk_window_get_size(GTK_WINDOW(story->notes_window), &w, &h);
	gtk_window_get_position(GTK_WINDOW(story->notes_window), &x, &y);
	config_file_set_int(PREFS_NOTEPAD_WIDTH, w);
	config_file_set_int(PREFS_NOTEPAD_HEIGHT, h);
	config_file_set_int(PREFS_NOTEPAD_X, x);
	config_file_set_int(PREFS_NOTEPAD_Y, y);
}

static gboolean
on_storywindow_delete_event(GtkWidget *window, GdkEvent *event)
{
    if(!i7_document_verify_save(I7_DOCUMENT(window)))
		return TRUE; /* Interrupt the signal if the user cancelled */
	
	save_storywindow_size(I7_STORY(window));

	gtk_widget_destroy(I7_STORY(window)->notes_window);
	i7_story_stop_running_game(I7_STORY(window));
	if(i7_document_get_path(I7_DOCUMENT(window)))
	    delete_build_files(I7_STORY(window));

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
	webkit_web_view_open(WEBKIT_WEB_VIEW(story->panel[side]->tabs[I7_PANE_DOCUMENTATION]), uri);
	gtk_notebook_set_current_page(GTK_NOTEBOOK(story->panel[side]->notebook), I7_PANE_DOCUMENTATION);
}

static void
on_source_notebook_switch_page(GtkNotebook *notebook, GtkNotebookPage *page, guint page_num, I7Story *story)
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
		case PROP_ELASTIC_TABS:
			i7_document_set_elastic_tabs(I7_DOCUMENT(story), g_value_get_boolean(value));
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
		case PROP_ELASTIC_TABS:
			g_value_set_boolean(value, i7_story_get_elastic_tabs(story));
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
	return g_strdup("Untitled");
}

/* Save story, in the previous location if it exists, otherwise ask for a new
location */
static gboolean
i7_story_save(I7Document *document)
{
	gchar *filename = i7_document_get_path(document);
	if(filename && g_file_test(filename, G_FILE_TEST_EXISTS)
		&& g_file_test(filename, G_FILE_TEST_IS_DIR))
		i7_document_save_as(document, filename);
	else {
		gchar *newname = get_filename_from_save_dialog(filename);
		if(!newname)
			return FALSE;
		i7_document_set_path(document, newname);
		i7_document_save_as(document, newname);
		g_free(newname);
	}
	if(filename)
		g_free(filename);
	return TRUE;
}

/* Update the list of recently used files */
static void
update_recent_story_file(I7Story *story, gchar *filename, const gchar *directory)
{
	GError *err = NULL;
	GtkRecentManager *manager = gtk_recent_manager_get_default();
	/* Add story.ni as the actual file to open, in case any other application
	wants to open it, and set the display name to the project directory */
	gchar *file_uri;
	if((file_uri = g_filename_to_uri(filename, NULL, &err)) == NULL) {
		/* fail discreetly */
		WARN(_("Cannot convert project filename to URI"), err);
		g_error_free(err);
		err = NULL; /* clear error */
	} else {
		/* We use the groups "inform7_project", "inform7_extension", and 
		 "inform7_builtin" to determine how to open a file from the recent manager */
		gchar *groups[] = { "inform7_project", NULL };
		GtkRecentData recent_data = {
			NULL, NULL, "text/x-natural-inform", "GNOME Inform 7",
			"gnome-inform7 %f", NULL, FALSE
		};
		recent_data.display_name = g_filename_display_basename(directory);
		/* Use the story title and author as the description,
		retrieved from the first line of the text */
		GtkTextIter start, end;
		GtkTextBuffer *buffer = GTK_TEXT_BUFFER(i7_document_get_buffer(I7_DOCUMENT(story)));
		gtk_text_buffer_get_iter_at_line(buffer, &start, 0);
		gtk_text_buffer_get_iter_at_line(buffer, &end, 0);
		gtk_text_iter_forward_to_line_end(&end);
		recent_data.description = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
		
		recent_data.groups = groups;
		gtk_recent_manager_add_full(manager, file_uri, &recent_data);
		g_free(recent_data.display_name);
		g_free(recent_data.description);
	}
	g_free(file_uri);	
}

/* Save story in the given directory  */
static void 
i7_story_save_as(I7Document *document, gchar *directory) 
{
	I7StoryPrivate *priv = I7_STORY_PRIVATE(document);
	GError *err = NULL;

	i7_document_display_status_message(document, _("Saving project..."), FILE_OPERATIONS);

	i7_document_stop_file_monitor(document);
	
	/* Create the project directory if it does not already exist */
	gchar *build_dir = g_build_filename(directory, "Build", NULL);
	gchar *index_dir = g_build_filename(directory, "Index", NULL);
	gchar *source_dir = g_build_filename(directory, "Source", NULL);
	if(g_mkdir_with_parents(directory, 0777) 
		|| g_mkdir_with_parents(build_dir, 0777)
		|| g_mkdir_with_parents(index_dir, 0777)
		|| g_mkdir_with_parents(source_dir, 0777)) 
	{
		error_dialog(GTK_WINDOW(document), NULL, _("Error creating project directory: %s"), g_strerror(errno));
		g_free(build_dir);
		g_free(index_dir);
		g_free(source_dir);
		return;
	}
	g_free(build_dir);
	g_free(index_dir);
	
	/* Save the source */
	gchar *text = i7_document_get_source_text(document);
	/* Write text to file */
	gchar *filename = g_build_filename(source_dir, "story.ni", NULL);
	g_free(source_dir);
	if(!g_file_set_contents(filename, text, -1, &err)) {
		error_dialog(GTK_WINDOW(document), err, _("Error saving file '%s': "), filename);
		g_free(filename);
		g_free(text);
		return;
	}
	gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(i7_document_get_buffer(document)), FALSE);
	g_free(text);

	update_recent_story_file(I7_STORY(document), filename, directory);
	
	/* Start file monitoring again */
	i7_document_monitor_file(document, filename);
	g_free(filename);
	
	/* Save the skein */
	filename = g_build_filename(directory, "Skein.skein", NULL);
	if(!i7_skein_save(priv->skein, filename, &err)) {
		error_dialog(GTK_WINDOW(document), err, _("There was an error saving the Skein. Your story will still be saved. Problem: "));
		err = NULL;
	}
	g_free(filename);
	/* skein_save(thestory->theskein, directory);*/

	/* Save the notes */
	filename = g_build_filename(directory, "notes.rtf", NULL);
	if(!rtf_text_buffer_export(priv->notes, filename, &err)) {
		error_dialog(GTK_WINDOW(document), err, _("There was an error saving the Notepad. Your story will still be saved. Problem: "));
		err = NULL;
	}
	gtk_text_buffer_set_modified(priv->notes, FALSE);
	g_free(filename);
	
	/* Save the project settings */
	filename = g_build_filename(directory, "Settings.plist", NULL);
	if(!plist_write(priv->settings, filename, &err)) {
		error_dialog(GTK_WINDOW(document), err, _("There was an error saving the project settings. Your story will still be saved. Problem: "));
		err = NULL;
	}
	g_free(filename);

	/* Delete the build files from the project directory */
	delete_build_files(I7_STORY(document));
	
	i7_document_set_modified(document, FALSE);
	
	i7_document_remove_status_message(document, FILE_OPERATIONS);
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
		case I7_PANE_GAME:
			return panel->tabs[I7_PANE_GAME];
		case I7_PANE_SOURCE:
			return panel->source_tabs[gtk_notebook_get_current_page(GTK_NOTEBOOK(panel->tabs[I7_PANE_SOURCE]))];
		case I7_PANE_ERRORS:
			return panel->errors_tabs[gtk_notebook_get_current_page(GTK_NOTEBOOK(panel->tabs[I7_PANE_ERRORS]))];
		case I7_PANE_INDEX:
			return panel->index_tabs[gtk_notebook_get_current_page(GTK_NOTEBOOK(panel->tabs[I7_PANE_INDEX]))];
		case I7_PANE_DOCUMENTATION:
			return panel->tabs[I7_PANE_DOCUMENTATION];
		default:
			g_assert_not_reached();
	}
}

static gboolean
do_search(GtkTextView *view, const gchar *text, gboolean forward, const GtkTextIter *startpos, GtkTextIter *start, GtkTextIter *end)
{
	if(GTK_IS_SOURCE_VIEW(view)) {
		if(forward)
			return gtk_source_iter_forward_search(startpos, text, GTK_SOURCE_SEARCH_VISIBLE_ONLY | GTK_SOURCE_SEARCH_TEXT_ONLY | GTK_SOURCE_SEARCH_CASE_INSENSITIVE, start, end, NULL);
		return gtk_source_iter_backward_search(startpos, text, GTK_SOURCE_SEARCH_VISIBLE_ONLY | GTK_SOURCE_SEARCH_TEXT_ONLY | GTK_SOURCE_SEARCH_CASE_INSENSITIVE, start, end, NULL);
	}
	if(forward)
		return gtk_text_iter_forward_search(startpos, text, GTK_TEXT_SEARCH_VISIBLE_ONLY | GTK_TEXT_SEARCH_TEXT_ONLY, start, end, NULL);
	return gtk_text_iter_backward_search(startpos, text, GTK_TEXT_SEARCH_VISIBLE_ONLY | GTK_TEXT_SEARCH_TEXT_ONLY, start, end, NULL);
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
		if(webkit_web_view_mark_text_matches(WEBKIT_WEB_VIEW(focus), text, FALSE, 0) == 0)
			return FALSE;
		webkit_web_view_set_highlight_text_matches(WEBKIT_WEB_VIEW(focus), TRUE);
		webkit_web_view_search_text(WEBKIT_WEB_VIEW(focus), text, FALSE, forward, TRUE);
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

/* TYPE SYSTEM */

static void
story_init_panel(I7Story *self, I7Panel *panel, PangoFontDescription *font)
{
	I7_STORY_USE_PRIVATE(self, priv);
	
	gtk_widget_show(GTK_WIDGET(panel));
	
	/* Connect other signals */
	g_signal_connect(panel->sourceview->heading_depth, "value-changed", G_CALLBACK(on_heading_depth_value_changed), self);
	g_signal_connect(panel->z5, "toggled", G_CALLBACK(on_z5_button_toggled), self);
	g_signal_connect(panel->z8, "toggled", G_CALLBACK(on_z8_button_toggled), self);
	g_signal_connect(panel->z6, "toggled", G_CALLBACK(on_z6_button_toggled), self);
	g_signal_connect(panel->glulx, "toggled", G_CALLBACK(on_glulx_button_toggled), self);
	g_signal_connect(panel->blorb, "toggled", G_CALLBACK(on_blorb_button_toggled), self);
	g_signal_connect(panel->nobble_rng, "toggled", G_CALLBACK(on_nobble_rng_button_toggled), self);
	g_signal_connect(panel->tabs[I7_PANE_SOURCE], "switch-page", G_CALLBACK(on_source_notebook_switch_page), self);
	g_signal_connect(panel->source_tabs[I7_SOURCE_VIEW_TAB_CONTENTS], "row-activated", G_CALLBACK(on_headings_row_activated), self);
	g_signal_connect(panel, "select-view", G_CALLBACK(on_panel_select_view), self);
	g_signal_connect(panel, "paste-code", G_CALLBACK(on_panel_paste_code), self);
	g_signal_connect(panel, "jump-to-line", G_CALLBACK(on_panel_jump_to_line), self);
	g_signal_connect(panel, "display-docpage", G_CALLBACK(on_panel_display_docpage), self);
	g_signal_connect(priv->skein, "labels-changed", G_CALLBACK(on_labels_changed), panel);
	g_signal_connect(priv->skein, "show-node", G_CALLBACK(on_show_node), panel);
	g_signal_connect(panel->tabs[I7_PANE_SKEIN], "node-menu-popup", G_CALLBACK(on_node_popup), NULL);
	g_signal_connect(panel->tabs[I7_PANE_GAME], "started", G_CALLBACK(on_game_started), self);
	g_signal_connect(panel->tabs[I7_PANE_GAME], "stopped", G_CALLBACK(on_game_stopped), self);
	g_signal_connect(panel->tabs[I7_PANE_GAME], "command", G_CALLBACK(on_game_command), self);

	/* Connect various models to various views */
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(panel->source_tabs[I7_SOURCE_VIEW_TAB_SOURCE]), GTK_TEXT_BUFFER(i7_document_get_buffer(I7_DOCUMENT(self))));
	gtk_tree_view_set_model(GTK_TREE_VIEW(panel->source_tabs[I7_SOURCE_VIEW_TAB_CONTENTS]), i7_document_get_headings(I7_DOCUMENT(self)));
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(panel->errors_tabs[I7_ERRORS_TAB_PROGRESS]), priv->progress);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(panel->errors_tabs[I7_ERRORS_TAB_DEBUGGING]), priv->debug_log);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(panel->errors_tabs[I7_ERRORS_TAB_INFORM6]), GTK_TEXT_BUFFER(priv->i6_source));
	i7_skein_view_set_skein(I7_SKEIN_VIEW(panel->tabs[I7_PANE_SKEIN]), priv->skein);

	/* Set the Errors/Progress to a monospace font */
	gtk_widget_modify_font(GTK_WIDGET(panel->errors_tabs[I7_ERRORS_TAB_PROGRESS]), font);
	
	/* Connect the Previous Section and Next Section actions to the up and down buttons */
	gtk_action_connect_proxy(I7_DOCUMENT(self)->previous_section, panel->sourceview->previous);
	gtk_action_connect_proxy(I7_DOCUMENT(self)->next_section, panel->sourceview->next);

	/* Set the Blorb resource-loading callback */
	chimara_glk_set_resource_load_callback(CHIMARA_GLK(panel->tabs[I7_PANE_GAME]), (ChimaraResourceLoadFunc)load_blorb_resource, self);
}

static void
i7_story_init(I7Story *self)
{
	I7_STORY_USE_PRIVATE(self, priv);
	I7App *theapp = i7_app_get();
	GError *error = NULL;

	/* Build the interface */
	gchar *filename = i7_app_get_datafile_path(theapp, "ui/story.ui");
	GtkBuilder *builder = create_new_builder(filename, self);
	g_free(filename);

	/* Make the action groups. This for-loop is a temporary fix
	and can be removed once Glade supports adding actions and accelerators to an
	action group. */
	const gchar *actions[] = { 
		"play_menu", "",
		"replay_menu", "",
		"release_menu", "",
		"show_pane_menu", "",
		"view_notepad", "<shift><ctrl>N",
		"refresh_index", "<ctrl>I",
		"go", "<ctrl>R",
		"replay", "<alt><ctrl>R",
		"show_last_command_skein", "<shift><ctrl>L",
		"stop", "<shift><ctrl>Q",
		"release", "<shift><ctrl>R",
		"save_debug_build", "",
		"test_me", "<alt><ctrl>T",
		"export_ifiction_record", "",
		"show_source", "<ctrl>F2",
		"show_errors", "<ctrl>F3",
		"show_index", "<ctrl>F4",
		"show_skein", "<ctrl>F5",
		"show_transcript", "<ctrl>F6",
		"show_game", "<ctrl>F7",
		"show_documentation", "<ctrl>F8",
		"show_settings", "<ctrl>F9",
		"show_index_menu", "",
		"show_actions", "<ctrl>3",
		"show_contents", "<ctrl>4",
		"show_kinds", "<ctrl>5",
		"show_phrasebook", "<ctrl>6",
		"show_rules", "<ctrl>7",
		"show_scenes", "<ctrl>8",
		"show_world", "<ctrl>9",
		"open_materials_folder", "<alt><ctrl>M",
		"help_contents", "F1", 
		"help_license", "",
		"help_extensions", "",
		"help_recipe_book", "<shift><ctrl>F1",
		NULL
	};
	/* Temporary "unimplemented" group */
	const gchar *unimplemented[] = {
		"import_into_skein", "",
		"show_last_command", "<alt><ctrl>L",
		"previous_changed_command", "",
		"next_changed_command", "",
		"previous_difference", "",
		"next_difference", "",
		"next_difference_skein", "",
		"play_all_blessed", "<shift><ctrl><alt>R",
		NULL
	};
	add_actions(builder, &(priv->story_action_group), "story_actions", actions);
	add_actions(builder, &(priv->unimplemented_action_group), "unimplemented_actions", unimplemented);

	/* One exception: the "stop" action starts out insensitive */
	GtkAction *stop = gtk_action_group_get_action(priv->story_action_group, "stop");
	gtk_action_set_sensitive(stop, FALSE);

	/* Build the menus and toolbars from the GtkUIManager file */
	gtk_ui_manager_insert_action_group(I7_DOCUMENT(self)->ui_manager, priv->story_action_group, 0);
	gtk_ui_manager_insert_action_group(I7_DOCUMENT(self)->ui_manager, priv->unimplemented_action_group, 0);
	filename = i7_app_get_datafile_path(theapp, "ui/story.uimanager.xml");
	gtk_ui_manager_add_ui_from_file(I7_DOCUMENT(self)->ui_manager, filename, &error);
	g_free(filename);
	if(error)
		ERROR(_("Building menus failed"), error);
	GtkWidget *menu = gtk_ui_manager_get_widget(I7_DOCUMENT(self)->ui_manager, "/StoryMenubar");
	I7_DOCUMENT(self)->toolbar = gtk_ui_manager_get_widget(I7_DOCUMENT(self)->ui_manager, "/MainToolbar");
	gtk_widget_set_no_show_all(I7_DOCUMENT(self)->toolbar, TRUE);
	i7_document_add_menus_and_findbar(I7_DOCUMENT(self));
	
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
	/* Add icons to the entry, but only if compiled and linked with >= 2.16 */
#if GTK_CHECK_VERSION(2,16,0)
	if(gtk_check_version(2, 16, 0) == NULL) {
		gtk_entry_set_icon_from_stock(GTK_ENTRY(search_entry), GTK_ENTRY_ICON_PRIMARY, GTK_STOCK_FIND);
		gtk_entry_set_icon_from_stock(GTK_ENTRY(search_entry), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_CLEAR);
		gtk_entry_set_icon_activatable(GTK_ENTRY(search_entry), GTK_ENTRY_ICON_SECONDARY, TRUE);
		g_signal_connect(search_entry, "icon-press", G_CALLBACK(on_search_entry_icon_press), NULL);
	}
#endif /* GTK_CHECK_VERSION(2,16,0) */
	
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
	gchar *docs = i7_app_get_datafile_path(theapp, "Documentation/index.html");
	i7_panel_reset_queue(self->panel[LEFT], I7_PANE_SOURCE, I7_SOURCE_VIEW_TAB_SOURCE, NULL);
	i7_panel_reset_queue(self->panel[RIGHT], I7_PANE_DOCUMENTATION, 0, docs);
	g_free(docs);
	gtk_paned_pack1(GTK_PANED(self->facing_pages), GTK_WIDGET(self->panel[LEFT]), TRUE, FALSE);
	gtk_paned_pack2(GTK_PANED(self->facing_pages), GTK_WIDGET(self->panel[RIGHT]), TRUE, FALSE);
	gtk_box_pack_start(GTK_BOX(I7_DOCUMENT(self)->box), self->facing_pages, TRUE, TRUE, 0);

	/* Builder object not needed anymore */
	g_object_unref(builder);

	/* Set the last saved window size and slider position */
    gtk_window_resize(GTK_WINDOW(self), config_file_get_int(PREFS_APP_WINDOW_WIDTH), config_file_get_int(PREFS_APP_WINDOW_HEIGHT));
    gtk_paned_set_position(GTK_PANED(self->facing_pages), config_file_get_int(PREFS_SLIDER_POSITION));
	
	/* Create the private properties */
	priv->notes = gtk_text_buffer_new(NULL);
	priv->last_focused = GTK_WIDGET(self->panel[LEFT]);
	priv->compile_finished_callback = NULL;
	priv->compile_finished_callback_data = NULL;
	priv->copyblorbto = NULL;
	priv->compiler_output = NULL;
	priv->test_me = FALSE;
	priv->manifest = NULL;

	/* Set up the Skein */
	priv->skein = i7_skein_new();
	g_object_set(priv->skein, 
		"vertical-spacing", 75.0, 
		"unlocked-color", "#6865FF",
		NULL);
	g_signal_connect(priv->skein, "node-activate", G_CALLBACK(on_node_activate), self);
	g_signal_connect(priv->skein, "differs-badge-activate", G_CALLBACK(on_differs_badge_activate), self);
	g_signal_connect(priv->skein, "modified", G_CALLBACK(on_skein_modified), self);
	gtk_range_set_value(GTK_RANGE(self->skein_spacing_horizontal), (gdouble)config_file_get_int(PREFS_HORIZONTAL_SPACING));
	gtk_range_set_value(GTK_RANGE(self->skein_spacing_vertical), (gdouble)config_file_get_int(PREFS_VERTICAL_SPACING));
	
	/* Set up the Notes window */
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(self->notes_view), priv->notes);
	gtk_window_set_keep_above(GTK_WINDOW(self->notes_window), TRUE);
	gtk_window_resize(GTK_WINDOW(self->notes_window), config_file_get_int(PREFS_NOTEPAD_WIDTH), config_file_get_int(PREFS_NOTEPAD_HEIGHT));
	gtk_window_move(GTK_WINDOW(self->notes_window), config_file_get_int(PREFS_NOTEPAD_X), config_file_get_int(PREFS_NOTEPAD_Y));
	/* Toggling the action will show the window if appropriate */
	gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(gtk_action_group_get_action(priv->story_action_group, "view_notepad")), config_file_get_bool(PREFS_NOTEPAD_VISIBLE));
	
	/* Set up the Natural Inform highlighting */
	GtkSourceBuffer *buffer = i7_document_get_buffer(I7_DOCUMENT(self));
	set_buffer_language(buffer, "inform7");
	set_highlight_styles(buffer);

	/* Create a text buffer for the Progress, Debugging and I6 text views */
	priv->progress = gtk_text_buffer_new(NULL);
	priv->debug_log = gtk_text_buffer_new(NULL);
	priv->i6_source = create_inform6_source_buffer();

	/* Create a monospace font description for the Errors/Progress views */
	gchar *family = config_file_get_string(DESKTOP_PREFS_MONOSPACE_FONT);
	PangoFontDescription *font = pango_font_description_from_string(family);
	g_free(family);
	pango_font_description_set_size(font, get_font_size(font));

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

	/* Add extra pages in "Errors" if the user has them turned on */
	if(config_file_get_bool(PREFS_DEBUG_LOG_VISIBLE))
		i7_story_add_debug_tabs(I7_DOCUMENT(self));

	/* Do the default settings */
	priv->settings = create_default_settings();
	/* Connect the widgets on the Settings pane to the settings properties */
	g_signal_connect(self, "notify::story-format", G_CALLBACK(on_notify_story_format), NULL);
	g_signal_connect(self, "notify::create-blorb", G_CALLBACK(on_notify_create_blorb), NULL);
	g_signal_connect(self, "notify::nobble-rng", G_CALLBACK(on_notify_nobble_rng), NULL);
	g_signal_connect(self, "notify::elastic-tabs", G_CALLBACK(on_notify_elastic_tabs), NULL);
	
	/* Set font sizes, etc. */
	i7_document_update_fonts(I7_DOCUMENT(self));

	/* Set spell checking */
	gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(I7_DOCUMENT(self)->autocheck_spelling), config_file_get_bool(PREFS_SPELL_CHECK_DEFAULT));
	i7_document_set_spellcheck(I7_DOCUMENT(self), config_file_get_bool(PREFS_SPELL_CHECK_DEFAULT));
	
	/* Create a callback for the delete event */
	g_signal_connect(self, "delete-event", G_CALLBACK(on_storywindow_delete_event), NULL);
}

static void
i7_story_finalize(GObject *self)
{
	I7_STORY_USE_PRIVATE(self, priv);
	g_free(priv->copyblorbto);
	g_free(priv->compiler_output);
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
	document_class->scroll_to_selection = i7_story_scroll_to_selection;
	document_class->update_tabs = i7_story_update_tabs;
	document_class->update_fonts = i7_story_update_fonts;
	document_class->update_font_sizes = i7_story_update_font_sizes;
	document_class->expand_headings_view = i7_story_expand_headings_view;
	document_class->highlight_search = i7_story_highlight_search;
	document_class->set_spellcheck = i7_story_set_spellcheck;
	document_class->check_spelling = i7_story_check_spelling;
	document_class->set_elastic_tabs = i7_story_set_elastic_tabs;
	
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->set_property = i7_story_set_property;
	object_class->get_property = i7_story_get_property;
	object_class->finalize = i7_story_finalize;

	/* Properties */
	g_object_class_install_property(object_class, PROP_STORY_FORMAT,
	    g_param_spec_uint("story-format", "Story Format",
		    "IFOutputSettings->IFSettingZCodeVersion", 5, 256, 8,
		    G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS));
	g_object_class_install_property(object_class, PROP_MAKE_BLORB,
	    g_param_spec_boolean("create-blorb", "Create Blorb file on release",
		    "IFOutputSettings->IFSettingCreateBlorb", TRUE,
		    G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS));
	g_object_class_install_property(object_class, PROP_NOBBLE_RNG,
	    g_param_spec_boolean("nobble-rng", "Nobble RNG",
		    "IFOutputSettings->IFSettingNobbleRNG", FALSE,
		    G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS));
	g_object_class_install_property(object_class, PROP_ELASTIC_TABS,
	    g_param_spec_boolean("elastic-tabs", "Elastic Tabs",
		    "IFMiscSettings->IFSettingElasticTabs", FALSE,
		    G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS));

	/* Private data */
	g_type_class_add_private(klass, sizeof(I7StoryPrivate));
}

/* PUBLIC FUNCTIONS */

I7Story *
i7_story_new(I7App *app, const gchar *filename, const gchar *title, const gchar *author)
{
	/* Building all the WebkitWebViews can take more than a second */
	i7_app_set_busy(app, TRUE);
	I7Story *story = I7_STORY(g_object_new(I7_TYPE_STORY, NULL));
	i7_app_set_busy(app, FALSE);

	i7_document_set_path(I7_DOCUMENT(story), filename);
	
    gchar *text = g_strconcat("\"", title, "\" by \"", author, "\"\n", NULL);
	i7_document_set_source_text(I7_DOCUMENT(story), text);
    i7_document_set_modified(I7_DOCUMENT(story), TRUE);
	
	/* Add document to global list */	
	i7_app_register_document(app, I7_DOCUMENT(story));
	
	/* Bring window to front */
	gtk_widget_show(GTK_WIDGET(story));
	gtk_window_present(GTK_WINDOW(story));
	return story;
}

I7Story *
i7_story_new_from_file(I7App *app, const gchar *filename)
{
	gchar *fullpath = expand_initial_tilde(filename);
	I7Document *dupl = i7_app_get_already_open(app, fullpath);
	
	if(dupl && I7_IS_STORY(dupl)) {
		gtk_window_present(GTK_WINDOW(dupl));
		g_free(fullpath);
		return NULL;
	}
	
	/* Building all the WebkitWebViews can take more than a second */
	i7_app_set_busy(app, TRUE);
	I7Story *story = I7_STORY(g_object_new(I7_TYPE_STORY, NULL));
	i7_app_set_busy(app, FALSE);
	if(!i7_story_open(story, fullpath)) {
		g_free(fullpath);
		g_object_unref(story);
		return NULL;
	}
	g_free(fullpath);

	/* Add document to global list */	
	i7_app_register_document(app, I7_DOCUMENT(story));

	/* Bring window to front */
	gtk_widget_show(GTK_WIDGET(story));
	gtk_window_present(GTK_WINDOW(story));
	
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

	GtkFileFilter *filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filter, "*.inform");
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(chooser), filter);

	if(gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_ACCEPT) {
		gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));
		story = i7_story_new_from_file(app, filename);
        g_free(filename);
    }
	
	gtk_widget_destroy(chooser);
	return story;
}

I7Story *
i7_story_new_from_uri(I7App *app, const gchar *uri)
{
	GError *error = NULL;
	I7Story *story = NULL;
	
	gchar *filename;
	if((filename = g_filename_from_uri(uri, NULL, &error)) == NULL) {
		WARN_S(_("Cannot get filename from URI"), uri, error);
		g_error_free(error);
		return NULL;
	}
	
	gchar *trash = g_path_get_dirname(filename); /* Remove "story.ni" */
	gchar *projectdir = g_path_get_dirname(trash); /* Remove "Source" */
	g_free(trash);
	story = i7_story_new_from_file(app, projectdir);		
	g_free(projectdir);
	return story;
}

/* Read a project directory, loading all the appropriate files into story and 
returning success */
gboolean
i7_story_open(I7Story *story, const gchar *directory) 
{
	I7_STORY_USE_PRIVATE(story, priv);
	GError *err = NULL;

	gchar *source_dir = g_build_filename(directory, "Source", NULL);    

	i7_document_set_path(I7_DOCUMENT(story), directory);

	/* Read the source */
	gchar *filename = g_build_filename(source_dir, "story.ni", NULL);
	g_free(source_dir);
	gchar *text = read_source_file(filename);
	if(!text)
		return FALSE;

	update_recent_story_file(story, filename, directory);
	
	/* Watch for changes to the source file */
	i7_document_monitor_file(I7_DOCUMENT(story), filename);
	g_free(filename);
	
	/* Write the source to the source buffer, clearing the undo history */
	i7_document_set_source_text(I7_DOCUMENT(story), text);
	g_free(text);
	
	/* Read the skein */
	filename = g_build_filename(directory, "Skein.skein", NULL);
	if(!i7_skein_load(priv->skein, filename, &err)) {
		error_dialog(GTK_WINDOW(story), err, _("This project's Skein was not found, or it was unreadable."));
		err = NULL;
	}
	g_free(filename);

	/* Read the notes */
	filename = g_build_filename(directory, "notes.rtf", NULL);
	if(!rtf_text_buffer_import(priv->notes, filename, NULL)) {
		/* Don't fail if the file is unreadable, instead just make some blank notes */
		gtk_text_buffer_set_text(priv->notes, "", -1);
	}
	g_free(filename);
	/* Remove all the formatting tags, we don't do formatting in this editor */
	GtkTextIter start, end;
	gtk_text_buffer_get_bounds(priv->notes, &start, &end);
	gtk_text_buffer_remove_all_tags(priv->notes, &start, &end);
	gtk_text_buffer_set_modified(priv->notes, FALSE);
	
	/* Read the settings */
	filename = g_build_filename(directory, "Settings.plist", NULL);
	plist_object_free(priv->settings);
	priv->settings = plist_read(filename, &err);
	if(!priv->settings) {
		priv->settings = create_default_settings();
		error_dialog(GTK_WINDOW(story), err, 
			_("Could not open the project's settings file, '%s'. "
			"Using default settings."), filename);
	}
	g_free(filename);
	/* Update the GUI with the new settings */
	on_notify_story_format(story);
	on_notify_create_blorb(story);
	on_notify_nobble_rng(story);
	on_notify_elastic_tabs(story);
	
	/* Load index tabs if they exist */
	i7_story_reload_index_tabs(story, FALSE);

	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(i7_document_get_buffer(I7_DOCUMENT(story)));
	gtk_text_buffer_get_start_iter(buffer, &start);
	gtk_text_buffer_place_cursor(buffer, &start);

	i7_document_set_modified(I7_DOCUMENT(story), FALSE);
	
	return TRUE;
}

/* Chooses an appropriate pane to display tab number newtab in. (From the
Windows source.) */
I7StoryPanel 
i7_story_choose_panel(I7Story *story, I7PanelPane newpane)
{
	/* If either panel is showing the same as the new, use that */
    int right = gtk_notebook_get_current_page(GTK_NOTEBOOK(story->panel[RIGHT]->notebook));
	if(right == newpane)
		return RIGHT;
    int left = gtk_notebook_get_current_page(GTK_NOTEBOOK(story->panel[LEFT]->notebook));
	if(left == newpane)
		return LEFT;
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
i7_story_show_docpage(I7Story *story, const gchar *file)
{
	I7StoryPanel side = i7_story_choose_panel(story, I7_PANE_DOCUMENTATION);
	i7_panel_goto_docpage(story->panel[side], file);
	gtk_notebook_set_current_page(GTK_NOTEBOOK(story->panel[side]->notebook), I7_PANE_DOCUMENTATION);
}

void
i7_story_show_docpage_at_anchor(I7Story *story, const gchar *file, const gchar *anchor)
{
	I7StoryPanel side = i7_story_choose_panel(story, I7_PANE_DOCUMENTATION);
	i7_panel_goto_docpage_at_anchor(story->panel[side], file, anchor);
	gtk_notebook_set_current_page(GTK_NOTEBOOK(story->panel[side]->notebook), I7_PANE_DOCUMENTATION);
}

/* Work out the location of the Materials folder, adapted from OS X source.
 Free string when done. */
gchar *
i7_story_get_materials_path(I7Story *story)
{
	gchar *projectpath = i7_document_get_path(I7_DOCUMENT(story));
	gchar *base = g_path_get_basename(projectpath);
	g_assert(g_str_has_suffix(base, ".inform"));
	gchar *projectname = g_strndup(base, strlen(base) - 7); /* lose extension */
	g_free(base);

	gchar *materialsname = g_strconcat(projectname, " Materials", NULL);
	gchar *materialsdir = g_path_get_dirname(projectpath);
	gchar *materialspath = g_build_filename(materialsdir, materialsname, NULL);

	g_free(projectpath);
	g_free(projectname);
	g_free(materialsname);
	g_free(materialsdir);

	return materialspath;
}

/* Return the extension of the output file of this story. 
 Do not free return value. */
const gchar *
i7_story_get_extension(I7Story *story)
{
    switch(i7_story_get_story_format(story)) {
        case I7_STORY_FORMAT_Z5:
            return "z5";
        case I7_STORY_FORMAT_Z6:
            return "z6";
        case I7_STORY_FORMAT_Z8:
            return "z8";
        case I7_STORY_FORMAT_GLULX:
            return "ulx";
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
