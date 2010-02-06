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

#include <gnome.h>
#include <gtkspell/gtkspell.h>

#include "interface.h"
#include "support.h"

#include "appmenu.h"
#include "appwindow.h"
#include "compile.h"
#include "configfile.h"
#include "datafile.h"
#include "error.h"
#include "file.h"
#include "findreplace.h"
#include "history.h"
#include "html.h"
#include "inspector.h"
#include "prefs.h"
#include "skein.h"
#include "story.h"
#include "tabgame.h"
#include "tabsource.h"
#include "windowlist.h"

void
on_new_activate(GtkMenuItem *menuitem, gpointer data)
{
    GtkWidget *new_dialog = create_new_dialog();
    gtk_widget_show(new_dialog);
}

void
on_open_activate(GtkMenuItem *menuitem, gpointer data)
{
    Story *thestory;
        
    /* Create a file chooser for *.inform. It actually selects folders, because
    that's what .inform projects are. */
    GtkWidget *dialog = gtk_file_chooser_dialog_new(_("Open Project"),
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
        if(thestory != NULL)
            gtk_widget_show(thestory->window);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

#ifndef SUCKY_GNOME
void
on_open_recent_activate(GtkRecentChooser *chooser, gpointer data)
{
    GError *err;
    GtkRecentInfo *item = gtk_recent_chooser_get_current_item(chooser);
    g_return_if_fail(gtk_recent_info_has_application(item, "GNOME Inform 7"));
    gchar *filename;
    if((filename = g_filename_from_uri(
      gtk_recent_info_get_uri(item), NULL, &err)) == NULL) {
        g_warning(_("Cannot get filename from URI: %s"), err->message);
        g_error_free(err);
    }
    
    if(gtk_recent_info_has_group(item, "inform7_project")) {
        gchar *trash = g_path_get_dirname(filename); /* Remove "story.ni" */
        gchar *projectdir = g_path_get_dirname(trash); /* Remove "Source" */
        Story *thestory = open_project(projectdir);
        if(thestory != NULL)
            gtk_widget_show(thestory->window);
        g_free(trash);
        g_free(projectdir);
    } else if(gtk_recent_info_has_group(item, "inform7_extension")) {
        Extension *ext = open_extension(filename);
        if(ext != NULL)
            gtk_widget_show(ext->window);
    } else
        g_warning(_("Recent manager file does not have tag"));
    g_free(filename);    
    gtk_recent_info_unref(item);
}
#else
void
on_open_recent_activate(GtkFileChooser *chooser, gpointer data)
{
}
#endif /* SUCKY_GNOME */

void
on_install_extension_activate(GtkMenuItem *menuitem, gpointer data)
{
    /* Open the Preferences window */
    GtkWidget *dialog = create_prefs_dialog();
    gtk_widget_show(dialog);
    
    /* Select the Extensions tab */
    gtk_notebook_set_current_page(
      GTK_NOTEBOOK(lookup_widget(dialog, "prefs_notebook")), TAB_EXTENSIONS);
    
    /* Pretend the user clicked the Add button */
    on_prefs_i7_extension_add_clicked(
      GTK_BUTTON(lookup_widget(dialog, "prefs_i7_extension_add")), NULL);
}

void
on_open_extension_activate(GtkMenuItem *menuitem, gchar *path)
{
    Extension *ext;
    ext = open_extension(path);
    if(ext != NULL)
        gtk_widget_show(ext->window);
}

void
on_close_activate(GtkMenuItem *menuitem, gpointer data)
{
    Story *thestory = get_story(GTK_WIDGET(menuitem));

    /* Check if we have saved our project, then, if this was the only window
    open, ask if we want to quit the application. */
    if(verify_save(GTK_WIDGET(menuitem))) {
        if(get_num_app_windows() == 1 && do_quit_dialog() != GTK_RESPONSE_YES)
            return;
        delete_story(thestory);

        if(get_num_app_windows() == 0)
            gtk_main_quit();
    }
}

void
on_save_activate(GtkMenuItem *menuitem, gpointer data)
{
    Story *thestory = get_story(GTK_WIDGET(menuitem));

    /* If this is a new project, Save As instead.*/
    if(thestory->filename == NULL)
        on_save_as_activate(menuitem, data);
    else {
        save_project(GTK_WIDGET(menuitem), thestory->filename);
        display_status_message(GTK_WIDGET(menuitem), _("Project saved."));
    }
}

void
on_save_as_activate(GtkMenuItem *menuitem, gpointer data)
{
    Story *thestory = get_story(GTK_WIDGET(menuitem));

    /* Create a file chooser */
    GtkWidget *dialog = gtk_file_chooser_dialog_new (_("Save File"),
      GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(menuitem))),
      GTK_FILE_CHOOSER_ACTION_SAVE,
      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER(dialog),
      TRUE);

    if (thestory->filename == NULL) {
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog),
          _("Untitled document"));
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
        display_status_message(GTK_WIDGET(menuitem), _("Project saved."));
        g_free(filename);
        g_free(path);
    }

    gtk_widget_destroy (dialog);
}

void
on_revert_activate(GtkMenuItem *menuitem, gpointer data)
{
    Story *thestory = get_story(GTK_WIDGET(menuitem));
    
    if(thestory->filename == NULL)
        return; /* No saved version to revert to */        
    if(!gtk_text_buffer_get_modified(GTK_TEXT_BUFFER(thestory->buffer)))
        return; /* Text has not changed since last save */
    
    /* Ask if the user is sure */
    GtkWidget *revert_dialog = gtk_message_dialog_new_with_markup(
      GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(menuitem))),
      GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING, GTK_BUTTONS_NONE,
      _("Are you sure you want to revert to the last saved version?"));
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(revert_dialog),
      _("All unsaved changes will be lost."));
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
on_quit_activate(GtkMenuItem *menuitem, gpointer data)
{
    if(do_quit_dialog() != GTK_RESPONSE_YES)
        return;
    close_all_windows();
    gtk_main_quit();
}

void
on_undo_activate(GtkMenuItem *menuitem, gpointer data)
{
    Story *thestory = get_story(GTK_WIDGET(menuitem));

    if(gtk_source_buffer_can_undo(thestory->buffer))
        gtk_source_buffer_undo(thestory->buffer);
}

void
on_redo_activate(GtkMenuItem *menuitem, gpointer data)
{
    Story *thestory = get_story(GTK_WIDGET(menuitem));

    if(gtk_source_buffer_can_redo(thestory->buffer))
        gtk_source_buffer_redo(thestory->buffer);
}

void
on_cut_activate(GtkMenuItem *menuitem, gpointer data)
{
    GtkWidget *widget = get_focused_widget(GTK_WIDGET(menuitem));
    if(GTK_IS_HTML(widget)) { /* only copy */
        int length = 0;
        gchar *text = gtk_html_get_selection_html(GTK_HTML(widget), &length);
        gchar *plaintext = html_to_plain_text(text);
        gtk_clipboard_set_text(gtk_clipboard_get(GDK_NONE), plaintext, length);
        g_free(text);
        g_free(plaintext);
    } else if(GTK_IS_LABEL(widget)) /* only copy */
        g_signal_emit_by_name(G_OBJECT(widget), "copy-clipboard", NULL);
    else if(GTK_IS_ENTRY(widget) || GTK_IS_TEXT_VIEW(widget))
        g_signal_emit_by_name(G_OBJECT(widget), "cut-clipboard", NULL);
    else /* If we don't know how to cut from it, just cut from the source */
        g_signal_emit_by_name(G_OBJECT(lookup_widget(GTK_WIDGET(menuitem),
        "source_l")), "cut-clipboard", NULL);
}

void
on_copy_activate(GtkMenuItem *menuitem, gpointer data)
{
    GtkWidget *widget = get_focused_widget(GTK_WIDGET(menuitem));
    if(GTK_IS_HTML(widget)) {
        int length = 0;
        gchar *text = gtk_html_get_selection_html(GTK_HTML(widget), &length);
        gchar *plaintext = html_to_plain_text(text);
        gtk_clipboard_set_text(gtk_clipboard_get(GDK_NONE), plaintext, length);
        g_free(text);
        g_free(plaintext);
    } else if(GTK_IS_ENTRY(widget) || GTK_IS_LABEL(widget)
              || GTK_IS_TEXT_VIEW(widget))
        g_signal_emit_by_name(G_OBJECT(widget), "copy-clipboard", NULL);
    else /* If we don't know how to copy from it, just copy from the source */
        g_signal_emit_by_name(G_OBJECT(lookup_widget(GTK_WIDGET(menuitem),
        "source_l")), "copy-clipboard", NULL);
}

void
on_paste_activate(GtkMenuItem *menuitem, gpointer data)
{
    GtkWidget *widget = get_focused_widget(GTK_WIDGET(menuitem));
    if(GTK_IS_ENTRY(widget) || GTK_IS_TEXT_VIEW(widget))
        g_signal_emit_by_name(G_OBJECT(widget), "paste-clipboard", NULL);
    else /* If we don't know how to paste to it, just paste to the source */
        g_signal_emit_by_name(G_OBJECT(lookup_widget(GTK_WIDGET(menuitem),
        "source_l")), "paste-clipboard", NULL);
}

void
on_select_all_activate(GtkMenuItem *menuitem, gpointer data)
{
    g_signal_emit_by_name(
      G_OBJECT(lookup_widget(GTK_WIDGET(menuitem), "source_l")),
      "select-all", TRUE, NULL);
}

void
on_find_activate(GtkMenuItem *menuitem, gpointer data)
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
on_autocheck_spelling_activate(GtkMenuItem *menuitem, gpointer data)
{
    gboolean check = gtk_check_menu_item_get_active(
      GTK_CHECK_MENU_ITEM(menuitem));
    gtk_widget_set_sensitive(
      lookup_widget(GTK_WIDGET(menuitem), "check_spelling"), check);
    config_file_set_bool("IDESettings", "SpellCheckDefault", check);
    /* Set the default for new windows to whatever the user chose last */
    
    GtkTextView *left = 
        GTK_TEXT_VIEW(lookup_widget(GTK_WIDGET(menuitem), "source_l"));
    GtkTextView *right = 
        GTK_TEXT_VIEW(lookup_widget(GTK_WIDGET(menuitem), "source_r"));
    if(check) {
        GError *err = NULL;
        if(!gtkspell_new_attach(left, NULL, &err) 
          || !gtkspell_new_attach(right, NULL, &err))
            error_dialog(
              GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(menuitem))),
              err, _("Error initializing spell checking: "));
    } else {
        gtkspell_detach(gtkspell_get_from_text_view(left));
        gtkspell_detach(gtkspell_get_from_text_view(right));
        /* BUG in GtkSpell: removing the second one generates error messages */
    }
}

void
on_check_spelling_activate(GtkMenuItem *menuitem, gpointer data)
{
    GtkSpell *spellchecker = gtkspell_get_from_text_view(
      GTK_TEXT_VIEW(lookup_widget(GTK_WIDGET(menuitem), "source_l")));
    if(spellchecker)
        gtkspell_recheck_all(spellchecker);
}

void
on_preferences_activate(GtkMenuItem *menuitem, gpointer data)
{
    extern GtkWidget *prefs_dialog;
    gtk_window_present(GTK_WINDOW(prefs_dialog));
}

void
on_shift_selection_right_activate(GtkMenuItem *menuitem, gpointer data)
{
    Story *thestory = get_story(GTK_WIDGET(menuitem));
    shift_selection_right(GTK_TEXT_BUFFER(thestory->buffer));
}

void
on_shift_selection_left_activate(GtkMenuItem *menuitem, gpointer data)
{
    Story *thestory = get_story(GTK_WIDGET(menuitem));
    shift_selection_left(GTK_TEXT_BUFFER(thestory->buffer));
}

void
on_comment_out_selection_activate(GtkMenuItem *menuitem, gpointer data)
{
	Story *thestory = get_story(GTK_WIDGET(menuitem));
	comment_out_selection(GTK_TEXT_BUFFER(thestory->buffer));
}


void
on_uncomment_selection_activate(GtkMenuItem *menuitem, gpointer data)
{
	Story *thestory = get_story(GTK_WIDGET(menuitem));
	uncomment_selection(GTK_TEXT_BUFFER(thestory->buffer));
}

void
on_renumber_all_sections_activate(GtkMenuItem *menuitem, gpointer data)
{
    Story *thestory = get_story(GTK_WIDGET(menuitem));
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(thestory->buffer);
    
    /* Renumbering sections counts as one action for Undo */
    gtk_text_buffer_begin_user_action(buffer);
    renumber_sections(buffer);
    gtk_text_buffer_end_user_action(buffer);
}

void
on_refresh_index_activate(GtkMenuItem *menuitem, gpointer data)
{
    Story *thestory = get_story(GTK_WIDGET(menuitem));
    /* Stop the project if running */
    stop_project(thestory);
    /* Save the project */
    on_save_activate(menuitem, data);
    /* Compile, and show the index instead of running */
    thestory->action = COMPILE_REFRESH_INDEX;
    compile_project(thestory);
}

void
on_go_activate(GtkMenuItem *menuitem, gpointer data)
{
    Story *thestory = get_story(GTK_WIDGET(menuitem));
    /* Stop the project if running */
    stop_project(thestory);
    /* Save the project */
    on_save_activate(menuitem, data);
    /* Compile and run */
    thestory->action = COMPILE_RUN;
    /* Reset the skein to the beginning */
    skein_reset(thestory->theskein, TRUE);
    compile_project(thestory);
}

void
on_test_me_activate(GtkMenuItem *menuitem, gpointer data)
{
	Story *thestory = get_story(GTK_WIDGET(menuitem));
	/* Stop the project if running */
	stop_project(thestory);
	/* Save the project */
	on_save_activate(menuitem, data);
	/* Compile and run "test me" */
	thestory->action = COMPILE_TEST_ME;
	/* Reset the skein to the beginning */
	skein_reset(thestory->theskein, TRUE);
	compile_project(thestory);
}

void
on_replay_activate(GtkMenuItem *menuitem, gpointer data)
{
    Story *thestory = get_story(GTK_WIDGET(menuitem));
    /* Stop the project if running */
    stop_project(thestory);
    /* Save the project */
    on_save_activate(menuitem, data);
    /* Compile and run */
    thestory->action = COMPILE_RUN;
    /* Reset the play pointer to the beginning of the skein */
    skein_reset(thestory->theskein, FALSE);
    compile_project(thestory);
}

void
on_stop_activate(GtkMenuItem *menuitem, gpointer data)
{
    /* Stop the project if running */
    stop_project(get_story(GTK_WIDGET(menuitem)));
}

void
on_release_activate(GtkMenuItem *menuitem, gpointer data)
{
    Story *thestory = get_story(GTK_WIDGET(menuitem));
    /* Stop the project if running */
    stop_project(thestory);
    /* Save the project */
    on_save_activate(menuitem, data);
    /* Compile for release, and do not run */
    thestory->action = COMPILE_RELEASE;
    compile_project(thestory);
}

void
on_save_debug_build_activate(GtkMenuItem *menuitem, gpointer data)
{
    Story *thestory = get_story(GTK_WIDGET(menuitem));
    /* Stop the project if running */
    stop_project(thestory);
    /* Save the project */
    on_save_activate(menuitem, data);
    /* Compile, not for release, and save the output file */
    thestory->action = COMPILE_SAVE_DEBUG_BUILD;
    compile_project(thestory);
}

void
on_open_materials_folder_activate(GtkMenuItem *menuitem, gpointer data)
{
	GError *err = NULL;
	Story *thestory = get_story(GTK_WIDGET(menuitem));
	if(thestory->filename == NULL) {
		error_dialog(GTK_WINDOW(thestory->window), NULL,
		    _("The story has not been saved yet, so there is no Materials "
			  "folder to open."));
		return;
	}

	gchar *base = g_path_get_basename(thestory->filename);
	gchar *path = g_path_get_dirname(thestory->filename);
	g_assert(g_str_has_suffix(base, ".inform"));
	gchar *title = g_strndup(base, strlen(base) - 7); /* lose extension */
	g_free(base);

	/* Create directory if it doesn't exist */
	gchar *materialsname = g_strconcat(title, " Materials", NULL);
	g_free(title);
	gchar *materialspath = g_build_filename(path, materialsname, NULL);
	if(g_mkdir_with_parents(materialspath, 0777) == -1) {
		error_dialog(GTK_WINDOW(thestory->window), NULL,
          _("Error creating Materials folder: %s"), g_strerror(errno));
		g_free(materialsname);
		g_free(materialspath);
		g_free(path);
		return;
	}
	g_free(materialspath);
	
	/* Open Materials folder */
	/* Slashes are correct here, not directory separators */
	gchar *url = g_strconcat("file://", path, "/", materialsname, NULL);
    g_free(path);
	g_free(materialsname);
	if(!gnome_url_show(url, &err))
		error_dialog(GTK_WINDOW(thestory->window), err, 
		    _("Could not open Materials folder: "));
	g_free(url);
}

void
on_export_ifiction_record_activate(GtkMenuItem *menuitem, gpointer data)
{
	Story *thestory = get_story(GTK_WIDGET(menuitem));
    /* Stop the project if running */
    stop_project(thestory);
    /* Save the project */
    on_save_activate(menuitem, data);
    /* Compile, and save the iFiction metadata instead of running */
    thestory->action = COMPILE_SAVE_IFICTION;
    compile_project(thestory);
}

void
on_show_inspectors_activate(GtkMenuItem *menuitem, gpointer data)
{
    extern GtkWidget *inspector_window;
    gtk_widget_show(inspector_window);
    config_file_set_bool("IDESettings", "InspectorVisible", TRUE);
}

void
on_show_source_activate(GtkMenuItem *menuitem, gpointer data)
{
    GtkNotebook *notebook = get_current_notebook(GTK_WIDGET(menuitem));
    gtk_notebook_set_current_page(notebook, TAB_SOURCE);
}

void
on_show_errors_activate(GtkMenuItem *menuitem, gpointer data)
{
    GtkNotebook *notebook = get_current_notebook(GTK_WIDGET(menuitem));
    gtk_notebook_set_current_page(notebook, TAB_ERRORS);
}

void
on_show_index_activate(GtkMenuItem *menuitem, gpointer data)
{
    GtkNotebook *notebook = get_current_notebook(GTK_WIDGET(menuitem));
    gtk_notebook_set_current_page(notebook, TAB_INDEX);
}

void
on_show_skein_activate(GtkMenuItem *menuitem, gpointer data)
{
    GtkNotebook *notebook = get_current_notebook(GTK_WIDGET(menuitem));
    gtk_notebook_set_current_page(notebook, TAB_SKEIN);
}

void
on_show_transcript_activate(GtkMenuItem *menuitem, gpointer data)
{
    GtkNotebook *notebook = get_current_notebook(GTK_WIDGET(menuitem));
    gtk_notebook_set_current_page(notebook, TAB_TRANSCRIPT);
}

void
on_show_game_activate(GtkMenuItem *menuitem, gpointer data)
{
    GtkNotebook *notebook = get_current_notebook(GTK_WIDGET(menuitem));
    gtk_notebook_set_current_page(notebook, TAB_GAME);
}

void
on_show_documentation_activate(GtkMenuItem *menuitem, gpointer data)
{
    GtkNotebook *notebook = get_current_notebook(GTK_WIDGET(menuitem));
    gtk_notebook_set_current_page(notebook, TAB_DOCUMENTATION);
}

void
on_show_settings_activate(GtkMenuItem *menuitem, gpointer data)
{
    GtkNotebook *notebook = get_current_notebook(GTK_WIDGET(menuitem));
    gtk_notebook_set_current_page(notebook, TAB_SETTINGS);
}

void
on_switch_sides_activate(GtkMenuItem *menuitem, gpointer data)
{
    int targetnotebook = !get_current_notebook_side(GTK_WIDGET(menuitem));
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
on_next_sub_panel_activate(GtkMenuItem *menuitem, gpointer data)
{
    int currentnotebook = get_current_notebook_side(GTK_WIDGET(menuitem));
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
        int lastpage = config_file_get_bool("IDESettings", "DebugLogVisible")?
            TAB_ERRORS_INFORM6 : TAB_ERRORS_LAST;
        if(gtk_notebook_get_current_page(subnotebook) == lastpage)
            gtk_notebook_set_current_page(subnotebook, TAB_ERRORS_FIRST);
        else
            gtk_notebook_next_page(subnotebook);
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
on_show_actions_activate(GtkMenuItem *menuitem, gpointer data)
{
    int right = choose_notebook(GTK_WIDGET(menuitem), TAB_INDEX);
    gtk_notebook_set_current_page(
      GTK_NOTEBOOK(lookup_widget(GTK_WIDGET(menuitem),
      right? "index_notebook_r" : "index_notebook_l")),
      TAB_INDEX_ACTIONS);
    gtk_notebook_set_current_page(get_notebook(GTK_WIDGET(menuitem), right),
      TAB_INDEX);
}

void
on_show_contents_activate(GtkMenuItem *menuitem, gpointer data)
{
    int right = choose_notebook(GTK_WIDGET(menuitem), TAB_INDEX);
    gtk_notebook_set_current_page(
      GTK_NOTEBOOK(lookup_widget(GTK_WIDGET(menuitem),
      right? "index_notebook_r" : "index_notebook_l")),
      TAB_INDEX_CONTENTS);
    gtk_notebook_set_current_page(get_notebook(GTK_WIDGET(menuitem), right),
      TAB_INDEX);
}

void
on_show_kinds_activate(GtkMenuItem *menuitem, gpointer user_data)
{
    int right = choose_notebook(GTK_WIDGET(menuitem), TAB_INDEX);
    gtk_notebook_set_current_page(
      GTK_NOTEBOOK(lookup_widget(GTK_WIDGET(menuitem),
      right? "index_notebook_r" : "index_notebook_l")),
      TAB_INDEX_KINDS);
    gtk_notebook_set_current_page(get_notebook(GTK_WIDGET(menuitem), right),
      TAB_INDEX);
}

void
on_show_phrasebook_activate(GtkMenuItem *menuitem, gpointer data)
{
    int right = choose_notebook(GTK_WIDGET(menuitem), TAB_INDEX);
    gtk_notebook_set_current_page(
      GTK_NOTEBOOK(lookup_widget(GTK_WIDGET(menuitem),
      right? "index_notebook_r" : "index_notebook_l")),
      TAB_INDEX_PHRASEBOOK);
    gtk_notebook_set_current_page(get_notebook(GTK_WIDGET(menuitem), right),
      TAB_INDEX);
}

void
on_show_rules_activate(GtkMenuItem *menuitem, gpointer data)
{
    int right = choose_notebook(GTK_WIDGET(menuitem), TAB_INDEX);
    gtk_notebook_set_current_page(
      GTK_NOTEBOOK(lookup_widget(GTK_WIDGET(menuitem),
      right? "index_notebook_r" : "index_notebook_l")),
      TAB_INDEX_RULES);
    gtk_notebook_set_current_page(get_notebook(GTK_WIDGET(menuitem), right),
      TAB_INDEX);
}

void
on_show_scenes_activate(GtkMenuItem *menuitem, gpointer data)
{
    int right = choose_notebook(GTK_WIDGET(menuitem), TAB_INDEX);
    gtk_notebook_set_current_page(
      GTK_NOTEBOOK(lookup_widget(GTK_WIDGET(menuitem),
      right? "index_notebook_r" : "index_notebook_l")),
      TAB_INDEX_SCENES);
    gtk_notebook_set_current_page(get_notebook(GTK_WIDGET(menuitem), right),
      TAB_INDEX);
}

void
on_show_world_activate(GtkMenuItem *menuitem, gpointer data)
{
    int right = choose_notebook(GTK_WIDGET(menuitem), TAB_INDEX);
    gtk_notebook_set_current_page(
      GTK_NOTEBOOK(lookup_widget(GTK_WIDGET(menuitem),
      right? "index_notebook_r" : "index_notebook_l")),
      TAB_INDEX_WORLD);
    gtk_notebook_set_current_page(get_notebook(GTK_WIDGET(menuitem), right),
      TAB_INDEX);
}

void
on_inform_help_activate(GtkMenuItem *menuitem, gpointer data)
{
    int panel = choose_notebook(GTK_WIDGET(menuitem), TAB_DOCUMENTATION);
    gchar *file = get_datafile_path_va("Documentation", "index.html", NULL);
    html_load_file(GTK_HTML(lookup_widget(GTK_WIDGET(menuitem),
      (panel == LEFT)? "docs_l" : "docs_r")), file);
    history_push_docpage(get_story(GTK_WIDGET(menuitem)), panel, file);
    g_free(file);

    gtk_notebook_set_current_page(get_notebook(GTK_WIDGET(menuitem), panel),
      TAB_DOCUMENTATION);
}

void
on_license_activate(GtkMenuItem *menuitem, gpointer data)
{
    int panel = choose_notebook(GTK_WIDGET(menuitem), TAB_DOCUMENTATION);
    gchar *file = get_datafile_path_va("Documentation", "licenses",
      "license.html", NULL);
    html_load_file(GTK_HTML(lookup_widget(GTK_WIDGET(menuitem),
      (panel == LEFT)? "docs_l" : "docs_r")), file);
    history_push_docpage(get_story(GTK_WIDGET(menuitem)), panel, file);
    g_free(file);

    gtk_notebook_set_current_page(get_notebook(GTK_WIDGET(menuitem), panel),
      TAB_DOCUMENTATION);
}

void
on_report_a_bug_activate(GtkMenuItem *menuitem, gpointer data)
{
	GError *err = NULL;
	if(!gnome_url_show("http://inform7.com/contribute/report", &err))
		error_dialog(NULL, err, 
			_("The page \"%s\" should have opened in your browser:"),
		    "http://inform7.com/contribute/report");
}

void
on_help_extensions_activate(GtkMenuItem *menuitem, gpointer data)
{
    int panel = choose_notebook(GTK_WIDGET(menuitem), TAB_DOCUMENTATION);
    gchar *file = g_build_filename(g_get_home_dir(), "Inform", "Documentation",
      "Extensions.html", NULL);
    html_load_file(GTK_HTML(lookup_widget(GTK_WIDGET(menuitem),
      (panel == LEFT)? "docs_l" : "docs_r")), file);
    history_push_docpage(get_story(GTK_WIDGET(menuitem)), panel, file);
    g_free(file);

    gtk_notebook_set_current_page(get_notebook(GTK_WIDGET(menuitem), panel),
      TAB_DOCUMENTATION);
}

void
on_recipe_book_activate(GtkMenuItem *menuitem, gpointer data)
{
    int panel = choose_notebook(GTK_WIDGET(menuitem), TAB_DOCUMENTATION);
    gchar *file = get_datafile_path_va("Documentation", "Rindex.html", NULL);
    html_load_file(GTK_HTML(lookup_widget(GTK_WIDGET(menuitem),
      (panel == LEFT)? "docs_l" : "docs_r")), file);
    history_push_docpage(get_story(GTK_WIDGET(menuitem)), panel, file);
    g_free(file);

    gtk_notebook_set_current_page(get_notebook(GTK_WIDGET(menuitem), panel),
      TAB_DOCUMENTATION);
}

void
on_visit_inform7com_activate(GtkMenuItem *menuitem, gpointer data)
{
	GError *err = NULL;
	if(!gnome_url_show("http://inform7.com/", &err))
		error_dialog(NULL, err, 
			_("The page \"%s\" should have opened in your browser:"),
		    "http://inform7.com/");
}

void
on_about_activate(GtkMenuItem *menuitem, gpointer data)
{
    GtkWidget *about_window;

    about_window = create_about_window();
    gtk_widget_show(about_window);
}
