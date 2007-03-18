/*  Copyright 2006 P.F. Chimento
 *  This file is part of GNOME Inform 7.
 * 
 *  GNOME Inform 7 is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  GNOME Inform 7 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNOME Inform 7; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#include <gnome.h>
#include <string.h>

#include "appwindow.h"
#include "file.h"
#include "interface.h"
#include "support.h"
#include "story.h"
#include "tabsettings.h"
#include "html.h"
#include "findreplace.h"
#include "prefs.h"
#include "skein.h"
#include "error.h"
#include "configfile.h"
#include "windowlist.h"
#include "compile.h"
#include "tabgame.h"
#include "inspector.h"
#include "searchwindow.h"
#include "taberrors.h"
#include "tabsource.h"

/* Gets a pointer to either the left or the right notebook in this window */
GtkNotebook *get_notebook(GtkWidget *thiswidget, int right) {
    return GTK_NOTEBOOK(lookup_widget(thiswidget,
      right? "notebook_r" : "notebook_l"));
}

/* Tells whether the focus is in the left or the right notebook */
int get_current_notebook(GtkWidget *thiswidget) {
    GtkWidget *focus = gtk_container_get_focus_child(
      GTK_CONTAINER(lookup_widget(thiswidget, "facingpages")));
    if(focus == lookup_widget(thiswidget, "notebook_l"))
        return LEFT;
    else if(focus == lookup_widget(thiswidget, "notebook_r"))
        return RIGHT;
    return LEFT; /* who cares */
}

/* Chooses an appropriate notebook to display tab number newtab in. (From the
Windows source.) */
int choose_notebook(GtkWidget *thiswidget, int newtab) {
    int left = gtk_notebook_get_current_page(get_notebook(thiswidget, LEFT));
	int right = gtk_notebook_get_current_page(get_notebook(thiswidget, RIGHT));

	/* If either panel is showing the same as the new, use that */
	if(right == newtab)
		return RIGHT;
	else if(left == newtab)
		return LEFT;
	/* Always try to use the left panel for source */
	if (newtab == TAB_SOURCE)
		return LEFT;
	/* If the right panel is not source, use that */
	if (right != TAB_SOURCE)
		return RIGHT;
	/* Use the left panel unless that is source too */
	return (left == TAB_SOURCE)? RIGHT : LEFT;
}

/* Returns the path to filename in the application data directory. */
gchar *get_datafile_path(const gchar *filename) {
    gchar *real_filename = g_build_filename("gnome-inform7", filename, NULL);
    gchar *path = gnome_program_locate_file(gnome_program_get(), 
      GNOME_FILE_DOMAIN_APP_DATADIR, real_filename, TRUE, NULL);
    g_free(real_filename);
    if(path)
        return path;
    error_dialog(NULL, NULL, "An application file, %s, was not found. Please "
      "reinstall GNOME Inform 7.", filename);
    return NULL;
}

/* Returns TRUE if filename exists in the data directory, otherwise FALSE.
Used when we do not necessarily want to return an error if it does not. */
gboolean check_datafile(const gchar *filename) {
    gchar *real_filename = g_build_filename("gnome-inform7", filename, NULL);
    gchar *path = gnome_program_locate_file(gnome_program_get(), 
      GNOME_FILE_DOMAIN_APP_DATADIR, real_filename, TRUE, NULL);
    g_free(real_filename);
    if(path)
        return TRUE;
    return FALSE;
}

/* Displays the message text in the status bar of the current window. */
void display_status_message(GtkWidget *thiswidget, const gchar *message) {
    gnome_appbar_set_status(GNOME_APPBAR(lookup_widget(thiswidget,
      "main_appbar")), message);
}

void display_status_busy(GtkWidget *thiswidget) {
    gtk_progress_bar_pulse(gnome_appbar_get_progress(GNOME_APPBAR(lookup_widget(
      thiswidget, "main_appbar"))));
}

/* The following function is from Damian Iverleigh's patch to GtkContainer,
http://mail.gnome.org/archives/gtk-devel-list/2001-October/msg00516.html */

/**
 * gtk_container_get_focus_child:
 * @container: a #GtkContainer
 * 
 * Retrieves the currently focused child of the container. See
 * gtk_container_set_focus_child().
 *
 * Return value: pointer to the widget of the focused child (NULL
 * if none is set
 **/
GtkWidget *
gtk_container_get_focus_child (GtkContainer *container)
{
    g_return_val_if_fail (container != NULL, NULL);
    g_return_val_if_fail (GTK_IS_CONTAINER (container), NULL);

    return container->focus_child;
}

/* 
 * CALLBACKS
 */

/* Set up things whenever an app_window is created. */ 
void
after_app_window_realize               (GtkWidget       *widget,
                                        gpointer         user_data)
{
    /* Create the Open Recent submenu */
    GtkRecentManager *manager = gtk_recent_manager_get_default();
    GtkWidget *recent_menu = gtk_recent_chooser_menu_new_for_manager(manager);
    GtkRecentFilter *filter = gtk_recent_filter_new();
    gtk_recent_filter_add_application(filter, "GNOME Inform 7");
    gtk_recent_chooser_set_filter(GTK_RECENT_CHOOSER(recent_menu), filter);
    g_signal_connect(recent_menu, "item-activated",
      G_CALLBACK(on_open_recent_activate), NULL);
    gtk_menu_item_set_submenu(
      GTK_MENU_ITEM(lookup_widget(widget, "open_recent")),
      recent_menu);
    
    /* Create a text buffer for the Progress, Debugging and I6 text views */
    GtkTextBuffer *buffer = gtk_text_buffer_new(NULL);
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(lookup_widget(widget,
      "compiler_output_l")), buffer);
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(lookup_widget(widget,
      "compiler_output_r")), buffer);
    buffer = gtk_text_buffer_new(NULL);
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(lookup_widget(widget,
      "debugging_l")), buffer);
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(lookup_widget(widget,
      "debugging_r")), buffer);
    GtkSourceBuffer *i6buffer = create_inform6_source_buffer();
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(lookup_widget(widget,
      "inform6_l")), GTK_TEXT_BUFFER(i6buffer));
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(lookup_widget(widget,
      "inform6_r")), GTK_TEXT_BUFFER(i6buffer));
        
    /* Set the Errors/Progress to a monospace font */
    PangoFontDescription *font = pango_font_description_new();
    pango_font_description_set_family(font, "Bitstream Vera Sans Mono,"
      "Monospace,Courier New");
    pango_font_description_set_size(font, 10 * PANGO_SCALE);
    gtk_widget_modify_font(lookup_widget(widget, "compiler_output_l"), font);
    gtk_widget_modify_font(lookup_widget(widget, "compiler_output_r"), font);
    pango_font_description_free(font);
    
    /* Set the slider position of the panes at 50% */
    gint slider_pos;
    g_object_get(GTK_PANED(lookup_widget(widget, "facingpages")), 
      "max-position", &slider_pos, NULL);
    slider_pos *= 0.5;
    gtk_paned_set_position(GTK_PANED(lookup_widget(widget, "facingpages")),
      slider_pos);
    
    /* Add extra pages in "Errors" if the user has them turned on */
    if(config_file_get_bool("Debugging", "ShowLog"))
        add_debug_tabs(widget);
    
    /* Load the documentation index page in "Documentation" */
    gchar *htmlfile = get_datafile_path("doc/index.html");
    html_load_file(GTK_HTML(lookup_widget(widget, "docs_l")), htmlfile);
    html_load_file(GTK_HTML(lookup_widget(widget, "docs_r")), htmlfile);
    g_free(htmlfile);
    
    /* Turn the left page to "Source" */
    gtk_notebook_set_current_page(get_notebook(widget, LEFT),
      TAB_SOURCE);
    /* Turn the right page to "Documentation" */
    gtk_notebook_set_current_page(get_notebook(widget, RIGHT),
      TAB_DOCUMENTATION);
    
    /* Show the inspector window if necessary */
    if(config_file_get_bool("Settings", "InspectorVisible")) {
        extern GtkWidget *inspector_window;
        gtk_widget_show(inspector_window);
    }
}

/* Whenever a main window receives the focus, make the inspector display that
   story's data */
gboolean
on_app_window_focus_in_event           (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data)
{
    refresh_inspector(get_story(widget));
    return FALSE;
}

void
on_new_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    GtkWidget *new_dialog = create_new_dialog();
    gtk_widget_show(new_dialog);
}


void
on_open_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    struct story *thestory;
        
    /* Create a file chooser for *.inform. It actually selects folders, because
    that's what .inform projects are. */
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Open Project",
      GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(menuitem))),
      GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, "*.inform");
    gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog), filter);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        gchar *filename = gtk_file_chooser_get_filename(
          GTK_FILE_CHOOSER(dialog));
        thestory = open_project(filename);
        if(thestory == NULL)
            return;
        g_free(filename);
        gtk_widget_show(thestory->window);
    }
    gtk_widget_destroy(dialog);
}

void
on_open_recent_activate                (GtkRecentChooser *chooser,
                                        gpointer         user_data)
{
    GError *err;
    GtkRecentInfo *item = gtk_recent_chooser_get_current_item(chooser);
    g_return_if_fail(gtk_recent_info_has_application(item, "GNOME Inform 7"));
    gchar *filename;
    if((filename = g_filename_from_uri(
      gtk_recent_info_get_uri(item), NULL, &err)) == NULL) {
        g_warning("Cannot get filename from URI: %s", err->message);
        g_error_free(err);
    }
    
    if(gtk_recent_info_has_group(item, "inform7_project")) {
        gchar *trash = g_path_get_dirname(filename); /* Remove "story.ni" */
        gchar *projectdir = g_path_get_dirname(trash); /* Remove "Source" */
        struct story *thestory = open_project(projectdir);
        gtk_widget_show(thestory->window);
        g_free(trash);
        g_free(projectdir);
    } else if(gtk_recent_info_has_group(item, "inform7_extension")) {
        struct extension *ext = open_extension(filename);
        gtk_widget_show(ext->window);
    } else
        g_warning("Recent manager file does not have tag");
    g_free(filename);    
    gtk_recent_info_unref(item);
}

void
on_install_extension_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    /* Create a file chooser that can select multiple files */
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
      "Select the extensions to install",
      GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(menuitem))),
      GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);
    
    if (gtk_dialog_run(GTK_DIALOG (dialog)) != GTK_RESPONSE_ACCEPT) {
        gtk_widget_destroy(dialog);
        return;
    }

    /* Iterate through the list of files returned by the file chooser and
    install them. */    
    GSList *extlist = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));
    GSList *iter;
    for(iter = extlist; iter != NULL; iter = g_slist_next(iter)) {
        install_extension((gchar *)iter->data);
        g_free(iter->data);
    }
    
    g_slist_free(extlist);
    gtk_widget_destroy(dialog);
}


void
on_open_extension_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    struct extension *ext;
        
    /* Create a file chooser that starts in the extensions directory */
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Open Extension",
      GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(menuitem))),
      GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),
      get_extension_path(NULL, NULL));
    
    if (gtk_dialog_run(GTK_DIALOG (dialog)) != GTK_RESPONSE_ACCEPT) {
        gtk_widget_destroy(dialog);
        return;
    }
    
    gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    ext = open_extension(filename);
    g_free(filename);
    if(ext != NULL)
        gtk_widget_show(ext->window);
    gtk_widget_destroy(dialog);
}


void
on_close_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    struct story *thestory = get_story(GTK_WIDGET(menuitem));

    /* Check if we have saved our project, then, if this was the only window
    open, ask if we want to quit the application. */
    if(verify_save(GTK_WIDGET(menuitem))) {
        if(get_num_app_windows() == 1) {
            GtkWidget *dialog = gtk_message_dialog_new(
              GTK_WINDOW(thestory->window), GTK_DIALOG_DESTROY_WITH_PARENT,
              GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "Quit GNOME Inform 7?");
            gint result = gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            if(result != GTK_RESPONSE_YES)
                return;
        }
        delete_story(thestory);

        if(get_num_app_windows() == 0)
            gtk_main_quit();
    }
}


void
on_save_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    struct story *thestory = get_story(GTK_WIDGET(menuitem));

    /* If this is a new project, Save As instead.*/
    if(thestory->filename == NULL)
        on_save_as_activate(menuitem, user_data);
    else {
        save_project(GTK_WIDGET(menuitem), thestory->filename);
        display_status_message(GTK_WIDGET(menuitem), "Project saved.");
    }
}


void
on_save_as_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    struct story *thestory = get_story(GTK_WIDGET(menuitem));

    /* Create a file chooser */
    GtkWidget *dialog = gtk_file_chooser_dialog_new ("Save File",
      GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(menuitem))),
      GTK_FILE_CHOOSER_ACTION_SAVE,
      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER(dialog),
      TRUE);

    if (thestory->filename == NULL) {
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog),
          "Untitled document");
    } else
        gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog),     
          thestory->filename);

    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, "*.inform");
    gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog), filter);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        gchar *path = gtk_file_chooser_get_filename(
          GTK_FILE_CHOOSER(dialog));
        gchar *filename = g_strconcat(path, ".inform", NULL);
        set_story_filename(thestory, filename);
        save_project(GTK_WIDGET(menuitem), thestory->filename);
        display_status_message(GTK_WIDGET(menuitem), "Project saved.");
        g_free(filename);
        g_free(path);
    }

    gtk_widget_destroy (dialog);
}


void
on_revert_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    struct story *thestory = get_story(GTK_WIDGET(menuitem));
    
    if(thestory->filename == NULL)
        return; /* No saved version to revert to */        
    if(!gtk_text_buffer_get_modified(GTK_TEXT_BUFFER(thestory->buffer)))
        return; /* Text has not changed since last save */
    
    /* Ask if the user is sure */
    GtkWidget *revert_dialog = gtk_message_dialog_new_with_markup(
      GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(menuitem))),
      GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING, GTK_BUTTONS_NONE,
      "Are you sure you want to revert to the last saved version?");
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(revert_dialog),
      "All unsaved changes will be lost.");
    gtk_dialog_add_buttons(GTK_DIALOG(revert_dialog),
      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_REVERT_TO_SAVED, GTK_RESPONSE_OK,
      NULL);
    gint result = gtk_dialog_run(GTK_DIALOG(revert_dialog));
    gtk_widget_destroy(revert_dialog);
    if(result != GTK_RESPONSE_OK)
        return; /* Only go on if the user clicked revert */
    
    /* Store the filename, close the window and reopen it */
    gchar *filename = g_strdup(thestory->filename);
    delete_story(thestory);
    thestory = open_project(filename);
    g_free(filename);
    gtk_widget_show(thestory->window);
}


void
on_quit_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    GtkWidget *dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_QUESTION,
      GTK_BUTTONS_YES_NO, "Quit GNOME Inform 7?");
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    if(result != GTK_RESPONSE_YES)
        return;

    close_all_windows();
    gtk_main_quit();
}


void
on_undo_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    struct story *thestory = get_story(GTK_WIDGET(menuitem));

    if(gtk_source_buffer_can_undo(thestory->buffer))
        gtk_source_buffer_undo(thestory->buffer);
}


void
on_redo_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    struct story *thestory = get_story(GTK_WIDGET(menuitem));

    if(gtk_source_buffer_can_redo(thestory->buffer))
        gtk_source_buffer_redo(thestory->buffer);
}


void
on_cut_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    g_signal_emit_by_name(
      G_OBJECT(lookup_widget(GTK_WIDGET(menuitem), "source_l")),
      "cut-clipboard", NULL);
}


void
on_copy_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    g_signal_emit_by_name(
      G_OBJECT(lookup_widget(GTK_WIDGET(menuitem), "source_l")),
      "copy-clipboard", NULL);
}


void
on_paste_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    g_signal_emit_by_name(
      G_OBJECT(lookup_widget(GTK_WIDGET(menuitem), "source_l")),
      "paste-clipboard", NULL);
}


void
on_select_all_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    g_signal_emit_by_name(
      G_OBJECT(lookup_widget(GTK_WIDGET(menuitem), "source_l")),
      "select-all", TRUE, NULL);
}


void
on_find_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    GtkWidget *dialog = create_find_dialog();
    /* Connect the button clicked signals and send our current story as
    user_data, so we can access it from within the find window. */
    gpointer thestory = (gpointer)get_story(GTK_WIDGET(menuitem));
    g_signal_connect((gpointer)lookup_widget(dialog, "find_next"),
      "clicked", G_CALLBACK(on_find_next_clicked), thestory);
    g_signal_connect((gpointer)lookup_widget(dialog, "find_previous"),
      "clicked", G_CALLBACK(on_find_previous_clicked), thestory);
    g_signal_connect((gpointer)lookup_widget(dialog, "find_replace_find"),
      "clicked", G_CALLBACK(on_find_replace_find_clicked), thestory);
    g_signal_connect((gpointer)lookup_widget(dialog, "find_replace_all"),
      "clicked", G_CALLBACK(on_find_replace_all_clicked), thestory);
    gtk_widget_show(dialog);
}


void
on_preferences_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    GtkWidget *dialog = create_prefs_dialog();
    gtk_widget_show(dialog);
}


void
on_shift_selection_right_activate      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    /* Adapted from gtksourceview.c */
    struct story *thestory = get_story(GTK_WIDGET(menuitem));
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(thestory->buffer);
    GtkTextIter start, end;
    gtk_text_buffer_get_selection_bounds(buffer, &start, &end);
    gint start_line = gtk_text_iter_get_line(&start);
    gint end_line = gtk_text_iter_get_line(&end);
    gint i;

    /* if the end of the selection is before the first character on a line,
    don't indent it */
    if((gtk_text_iter_get_visible_line_offset(&end) == 0)
      && (end_line > start_line))
        end_line--;

    gtk_text_buffer_begin_user_action(buffer);
    for(i = start_line; i <= end_line; i++) {
        GtkTextIter iter;
        gtk_text_buffer_get_iter_at_line(buffer, &iter, i);

        /* don't add indentation on empty lines */
        if(gtk_text_iter_ends_line(&iter))
            continue;

        gtk_text_buffer_insert(buffer, &iter, "\t", -1);
	}
	gtk_text_buffer_end_user_action(buffer);
}


void
on_shift_selection_left_activate       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    /* Adapted from gtksourceview.c */
    struct story *thestory = get_story(GTK_WIDGET(menuitem));
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(thestory->buffer);
    GtkTextIter start, end;
    gtk_text_buffer_get_selection_bounds(buffer, &start, &end);
    gint start_line = gtk_text_iter_get_line(&start);
    gint end_line = gtk_text_iter_get_line(&end);
    gint i;

    /* if the end of the selection is before the first character on a line,
    don't indent it */
	if((gtk_text_iter_get_visible_line_offset(&end) == 0)
      && (end_line > start_line))
        end_line--;

    gtk_text_buffer_begin_user_action(buffer);
    for(i = start_line; i <= end_line; i++) {
        GtkTextIter iter, iter2;

        gtk_text_buffer_get_iter_at_line(buffer, &iter, i);

        if(gtk_text_iter_get_char(&iter) == '\t') {
            iter2 = iter;
            gtk_text_iter_forward_char(&iter2);
            gtk_text_buffer_delete(buffer, &iter, &iter2);
        }
    }
    gtk_text_buffer_end_user_action(buffer);
}


void
on_renumber_all_sections_activate      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    struct story *thestory = get_story(GTK_WIDGET(menuitem));
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(thestory->buffer);
    
    /* Renumbering sections counts as one action for Undo */
    gtk_text_buffer_begin_user_action(buffer);
    renumber_sections(buffer);
    gtk_text_buffer_end_user_action(buffer);
}


void
on_refresh_index_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    struct story *thestory = get_story(GTK_WIDGET(menuitem));
    /* Stop the project if running */
    stop_project(thestory);
    /* Save the project */
    on_save_activate(menuitem, user_data);
    /* Compile, not for release, show the index instead of running */
    thestory->release = FALSE;
    thestory->run = FALSE;
    compile_project(thestory);
}


void
on_go_activate                         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    struct story *thestory = get_story(GTK_WIDGET(menuitem));
    /* Stop the project if running */
    stop_project(thestory);
    /* Save the project */
    on_save_activate(menuitem, user_data);
    /* Compile, not for release, do run afterwards */
    thestory->release = FALSE;
    thestory->run = TRUE;
    /* Reset the skein to the beginning */
    thestory->theskein = reset_skein(thestory->theskein,
      &(thestory->skein_ptr));
    compile_project(thestory);
}


void
on_replay_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    struct story *thestory = get_story(GTK_WIDGET(menuitem));
    /* Stop the project if running */
    stop_project(thestory);
    /* Save the project */
    on_save_activate(menuitem, user_data);
    /* Compile, not for release, do run afterwards */
    thestory->release = FALSE;
    thestory->run = TRUE;
    compile_project(thestory);
}


void
on_stop_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    /* Stop the project if running */
    stop_project(get_story(GTK_WIDGET(menuitem)));
}


void
on_release_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    struct story *thestory = get_story(GTK_WIDGET(menuitem));
    /* Stop the project if running */
    stop_project(thestory);
    /* Save the project */
    on_save_activate(menuitem, user_data);
    /* Compile for release, then do not run */
    thestory->release = TRUE;
    thestory->run = FALSE;
    compile_project(thestory);
}


void
on_show_inspectors_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    extern GtkWidget *inspector_window;
    gtk_widget_show(inspector_window);
    config_file_set_bool("Settings", "InspectorVisible", TRUE);
}


void
on_show_source_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    GtkNotebook *notebook = get_notebook(GTK_WIDGET(menuitem),
      get_current_notebook(GTK_WIDGET(menuitem)));
    gtk_notebook_set_current_page(notebook, TAB_SOURCE);
}


void
on_show_errors_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    GtkNotebook *notebook = get_notebook(GTK_WIDGET(menuitem),
      get_current_notebook(GTK_WIDGET(menuitem)));
    gtk_notebook_set_current_page(notebook, TAB_ERRORS);
}


void
on_show_index_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    GtkNotebook *notebook = get_notebook(GTK_WIDGET(menuitem),
      get_current_notebook(GTK_WIDGET(menuitem)));
    gtk_notebook_set_current_page(notebook, TAB_INDEX);
}


void
on_show_skein_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    GtkNotebook *notebook = get_notebook(GTK_WIDGET(menuitem),
      get_current_notebook(GTK_WIDGET(menuitem)));
    gtk_notebook_set_current_page(notebook, TAB_SKEIN);
}


void
on_show_transcript_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    GtkNotebook *notebook = get_notebook(GTK_WIDGET(menuitem),
      get_current_notebook(GTK_WIDGET(menuitem)));
    gtk_notebook_set_current_page(notebook, TAB_TRANSCRIPT);
}


void
on_show_game_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    GtkNotebook *notebook = get_notebook(GTK_WIDGET(menuitem),
      get_current_notebook(GTK_WIDGET(menuitem)));
    gtk_notebook_set_current_page(notebook, TAB_GAME);
}


void
on_show_documentation_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    GtkNotebook *notebook = get_notebook(GTK_WIDGET(menuitem),
      get_current_notebook(GTK_WIDGET(menuitem)));
    gtk_notebook_set_current_page(notebook, TAB_DOCUMENTATION);
}


void
on_show_settings_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    GtkNotebook *notebook = get_notebook(GTK_WIDGET(menuitem),
      get_current_notebook(GTK_WIDGET(menuitem)));
    gtk_notebook_set_current_page(notebook, TAB_SETTINGS);
}



void
on_switch_sides_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    int targetnotebook = !get_current_notebook(GTK_WIDGET(menuitem));
    int targettab = gtk_notebook_get_current_page(
      get_notebook(GTK_WIDGET(menuitem), targetnotebook));
    gchar *targetpage, *targetwidgetname;
    
    switch(targettab) {
        case TAB_SOURCE:
            targetpage = g_strdup("source"); break;
        case TAB_ERRORS:
            targetpage = g_strdup("errors_notebook"); break;
        case TAB_INDEX:
            targetpage = g_strdup("index_notebook"); break;
        case TAB_SKEIN:
            targetpage = g_strdup("skein"); break;
        case TAB_TRANSCRIPT:
            targetpage = g_strdup("transcript"); break;
        case TAB_GAME:
            targetpage = g_strdup("game"); break;
        case TAB_DOCUMENTATION:
            targetpage = g_strdup("docs"); break;
        case TAB_SETTINGS:
            targetpage = g_strdup("z5_button"); break;
        default:
            return;
    }
    
    targetwidgetname = g_strconcat(targetpage,
      (targetnotebook == LEFT)? "_l" : "_r", NULL);
    gtk_widget_grab_focus(
      lookup_widget(GTK_WIDGET(menuitem), targetwidgetname));
    
    g_free(targetpage);
    g_free(targetwidgetname);
}


void
on_next_sub_panel_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    int currentnotebook = get_current_notebook(GTK_WIDGET(menuitem));
    int currentpage = gtk_notebook_get_current_page(
      get_notebook(GTK_WIDGET(menuitem), currentnotebook));
    
    /* Choose the appropriate sub page depending on whether we are in Errors
    or Index */
    if(currentpage == TAB_ERRORS) {
        GtkNotebook *subnotebook = GTK_NOTEBOOK(lookup_widget(
          GTK_WIDGET(menuitem),
          (currentnotebook == LEFT)? "errors_notebook_l": "errors_notebook_r"));
        /* The last page of the notebook is different, depending on whether we
        are showing the extra debug tabs */
        if(config_file_get_bool("Debugging", "ShowLog")) {
            if(gtk_notebook_get_current_page(subnotebook) == TAB_ERRORS_INFORM6)
                gtk_notebook_set_current_page(subnotebook, TAB_ERRORS_FIRST);
            else
                gtk_notebook_next_page(subnotebook);
        } else {
            if(gtk_notebook_get_current_page(subnotebook) == TAB_ERRORS_LAST)
                gtk_notebook_set_current_page(subnotebook, TAB_ERRORS_FIRST);
            else
                gtk_notebook_next_page(subnotebook);
        }
    } else if(currentpage == TAB_INDEX) {
        GtkNotebook *subnotebook = GTK_NOTEBOOK(lookup_widget(
          GTK_WIDGET(menuitem),
          (currentnotebook == LEFT)? "index_notebook_l" : "index_notebook_r"));
        if(gtk_notebook_get_current_page(subnotebook) == TAB_INDEX_LAST)
            gtk_notebook_set_current_page(subnotebook, TAB_INDEX_FIRST);
        else
            gtk_notebook_next_page(subnotebook);
    }
    /* else do nothing */
}


void
on_show_actions_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    int right = choose_notebook(GTK_WIDGET(menuitem), TAB_INDEX);
    gtk_notebook_set_current_page(get_notebook(GTK_WIDGET(menuitem), right),
      TAB_INDEX);
    gtk_notebook_set_current_page(
      GTK_NOTEBOOK(lookup_widget(GTK_WIDGET(menuitem),
      right? "index_notebook_r" : "index_notebook_l")),
      TAB_INDEX_ACTIONS);
}


void
on_show_contents_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    int right = choose_notebook(GTK_WIDGET(menuitem), TAB_INDEX);
    gtk_notebook_set_current_page(get_notebook(GTK_WIDGET(menuitem), right),
      TAB_INDEX);
    gtk_notebook_set_current_page(
      GTK_NOTEBOOK(lookup_widget(GTK_WIDGET(menuitem),
      right? "index_notebook_r" : "index_notebook_l")),
      TAB_INDEX_CONTENTS);
}


void
on_show_kinds_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    int right = choose_notebook(GTK_WIDGET(menuitem), TAB_INDEX);
    gtk_notebook_set_current_page(get_notebook(GTK_WIDGET(menuitem), right),
      TAB_INDEX);
    gtk_notebook_set_current_page(
      GTK_NOTEBOOK(lookup_widget(GTK_WIDGET(menuitem),
      right? "index_notebook_r" : "index_notebook_l")),
      TAB_INDEX_KINDS);
}


void
on_show_phrasebook_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    int right = choose_notebook(GTK_WIDGET(menuitem), TAB_INDEX);
    gtk_notebook_set_current_page(get_notebook(GTK_WIDGET(menuitem), right),
      TAB_INDEX);
    gtk_notebook_set_current_page(
      GTK_NOTEBOOK(lookup_widget(GTK_WIDGET(menuitem),
      right? "index_notebook_r" : "index_notebook_l")),
      TAB_INDEX_PHRASEBOOK);
}


void
on_show_rules_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    int right = choose_notebook(GTK_WIDGET(menuitem), TAB_INDEX);
    gtk_notebook_set_current_page(get_notebook(GTK_WIDGET(menuitem), right),
      TAB_INDEX);
    gtk_notebook_set_current_page(
      GTK_NOTEBOOK(lookup_widget(GTK_WIDGET(menuitem),
      right? "index_notebook_r" : "index_notebook_l")),
      TAB_INDEX_RULES);
}


void
on_show_scenes_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    int right = choose_notebook(GTK_WIDGET(menuitem), TAB_INDEX);
    gtk_notebook_set_current_page(get_notebook(GTK_WIDGET(menuitem), right),
      TAB_INDEX);
    gtk_notebook_set_current_page(
      GTK_NOTEBOOK(lookup_widget(GTK_WIDGET(menuitem),
      right? "index_notebook_r" : "index_notebook_l")),
      TAB_INDEX_SCENES);
}


void
on_show_world_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    int right = choose_notebook(GTK_WIDGET(menuitem), TAB_INDEX);
    gtk_notebook_set_current_page(get_notebook(GTK_WIDGET(menuitem), right),
      TAB_INDEX);
    gtk_notebook_set_current_page(
      GTK_NOTEBOOK(lookup_widget(GTK_WIDGET(menuitem),
      right? "index_notebook_r" : "index_notebook_l")),
      TAB_INDEX_WORLD);
}


void
on_inform_help_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    int panel = choose_notebook(GTK_WIDGET(menuitem), TAB_DOCUMENTATION);
    gchar *file = get_datafile_path("doc/index.html");
    html_load_file(GTK_HTML(lookup_widget(GTK_WIDGET(menuitem),
      (panel == LEFT)? "docs_l" : "docs_r")), file);
    g_free(file);

    gtk_notebook_set_current_page(get_notebook(GTK_WIDGET(menuitem), panel),
      TAB_DOCUMENTATION);
}


void
on_gnome_notes_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    int panel = choose_notebook(GTK_WIDGET(menuitem), TAB_DOCUMENTATION);
    gchar *file = get_datafile_path("doc/gnome/gnome.html");
    html_load_file(GTK_HTML(lookup_widget(GTK_WIDGET(menuitem),
      (panel == LEFT)? "docs_l" : "docs_r")), file);
    g_free(file);

    gtk_notebook_set_current_page(get_notebook(GTK_WIDGET(menuitem), panel),
      TAB_DOCUMENTATION);
}


void
on_license_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    int panel = choose_notebook(GTK_WIDGET(menuitem), TAB_DOCUMENTATION);
    gchar *file = get_datafile_path("doc/licenses/license.html");
    html_load_file(GTK_HTML(lookup_widget(GTK_WIDGET(menuitem),
      (panel == LEFT)? "docs_l" : "docs_r")), file);
    g_free(file);

    gtk_notebook_set_current_page(get_notebook(GTK_WIDGET(menuitem), panel),
      TAB_DOCUMENTATION);
}


void
on_help_extensions_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    int panel = choose_notebook(GTK_WIDGET(menuitem), TAB_DOCUMENTATION);
    gchar *file = g_build_filename(g_get_home_dir(), ".wine", "drive_c",
      "Inform", "Documentation", "Extensions.html", NULL);
    html_load_file(GTK_HTML(lookup_widget(GTK_WIDGET(menuitem),
      (panel == LEFT)? "docs_l" : "docs_r")), file);
    g_free(file);

    gtk_notebook_set_current_page(get_notebook(GTK_WIDGET(menuitem), panel),
      TAB_DOCUMENTATION);
}


void
on_recipe_book_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    int panel = choose_notebook(GTK_WIDGET(menuitem), TAB_DOCUMENTATION);
    gchar *file = get_datafile_path("doc/recipes.html");
    html_load_file(GTK_HTML(lookup_widget(GTK_WIDGET(menuitem),
      (panel == LEFT)? "docs_l" : "docs_r")), file);
    g_free(file);

    gtk_notebook_set_current_page(get_notebook(GTK_WIDGET(menuitem), panel),
      TAB_DOCUMENTATION);
}


void
on_about_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    GtkWidget *about_window;

    about_window = create_about_window();
    gtk_widget_show(about_window);
}


void
on_go_toolbutton_clicked               (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
    on_go_activate(GTK_MENU_ITEM(lookup_widget(GTK_WIDGET(toolbutton),
      "go")), user_data);
}


void
on_replay_toolbutton_clicked           (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
    on_replay_activate(GTK_MENU_ITEM(lookup_widget(GTK_WIDGET(toolbutton),
      "replay")), user_data);
}


void
on_stop_toolbutton_clicked             (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
    on_stop_activate(GTK_MENU_ITEM(lookup_widget(GTK_WIDGET(toolbutton),
      "stop")), user_data);
}


void
on_release_toolbutton_clicked          (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
    on_release_activate(GTK_MENU_ITEM(lookup_widget(GTK_WIDGET(toolbutton),
      "release")), user_data);
}

void
on_source_search_activate              (GtkEntry        *entry,
                                        gpointer         user_data)
{
    struct story *thestory = get_story(GTK_WIDGET(entry));
    find(GTK_TEXT_BUFFER(thestory->buffer), gtk_entry_get_text(entry), TRUE,
      TRUE, FIND_CONTAINS, FALSE);
    /* Do not free or modify the strings from gtk_entry_get_text */
    scroll_text_view_to_cursor(
      GTK_TEXT_VIEW(lookup_widget(thestory->window, "source_l")));
    scroll_text_view_to_cursor(
      GTK_TEXT_VIEW(lookup_widget(thestory->window, "source_r")));
}

void
on_docs_search_activate                (GtkEntry        *entry,
                                        gpointer         user_data)
{
    const gchar *search_text = gtk_entry_get_text(entry);
    GList *list = search_doc(search_text, TRUE /*ignore case*/, FIND_CONTAINS);
    GtkWidget *search_window = new_search_window(
      gtk_widget_get_toplevel(GTK_WIDGET(entry)), search_text, list);
    gtk_widget_show(search_window);
}


gboolean
on_source_search_focus                 (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data)
{
    gtk_editable_delete_text(GTK_EDITABLE(widget), 0, -1);
    return TRUE;
}


gboolean
on_docs_search_focus                   (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data)
{
    gtk_editable_delete_text(GTK_EDITABLE(widget), 0, -1);
    return FALSE;
}

void
on_help_toolbutton_clicked             (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
    on_inform_help_activate(GTK_MENU_ITEM(lookup_widget(GTK_WIDGET(toolbutton),
      "inform_help")), user_data);
}

gboolean
on_app_window_delete_event             (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
    if(verify_save(widget)) {
        struct story *thestory = get_story(GTK_WIDGET(widget));

        if(get_num_app_windows() == 1) {
            GtkWidget *dialog = gtk_message_dialog_new(
              GTK_WINDOW(thestory->window), GTK_DIALOG_DESTROY_WITH_PARENT,
              GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "Quit GNOME Inform 7?");
            gint result = gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            if(result != GTK_RESPONSE_YES)
                return TRUE;
        }

        delete_story(thestory);

        if(get_num_app_windows() == 0) {
            gtk_main_quit();
        }
        return FALSE;
    }
    return TRUE;
}

/* This function is called when our app_window is involuntarily destroyed while
the story has not been freed. It will ask if we want to save, but not offer a
choice to cancel. */
void
on_app_window_destroy                  (GtkObject       *object,
                                        gpointer         user_data)
{
    struct story *thestory = get_story(GTK_WIDGET(object));
    if(thestory == NULL)
        return;
    
    if(gtk_text_buffer_get_modified(GTK_TEXT_BUFFER(thestory->buffer))
      || gtk_text_buffer_get_modified(thestory->notes)) {
        gchar *filename = g_path_get_basename(thestory->filename);
        GtkWidget *save_changes_dialog = gtk_message_dialog_new_with_markup(
          GTK_WINDOW(thestory->window),
          GTK_DIALOG_DESTROY_WITH_PARENT,
          GTK_MESSAGE_WARNING,
          GTK_BUTTONS_NONE,
          "<b><big>Save changes to project '%s' before closing?</big></b>",
          filename);
        gtk_message_dialog_format_secondary_text(
          GTK_MESSAGE_DIALOG(save_changes_dialog),
          "If you don't save, your changes will be lost.");
        gtk_dialog_add_buttons(GTK_DIALOG(save_changes_dialog),
          "Close _without saving", GTK_RESPONSE_REJECT,
          GTK_STOCK_SAVE, GTK_RESPONSE_OK,
          NULL);
        gint result = gtk_dialog_run(GTK_DIALOG(save_changes_dialog));
        gtk_widget_destroy(save_changes_dialog);
        if(result == GTK_RESPONSE_OK)
            on_save_activate(GTK_MENU_ITEM(lookup_widget(GTK_WIDGET(object),
            "save")), NULL);
        g_free(filename);
    }
    delete_story(thestory);
}
