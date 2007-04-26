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
#include <glib/gprintf.h>
#include <glib/gstdio.h>
#include <gtksourceview/gtksourcebuffer.h>
#include <ctype.h>
#include <libgnomevfs/gnome-vfs.h>
#include <string.h>

#include "support.h"
#include "callbacks.h"

#include "appwindow.h"
#include "compile.h"
#include "configfile.h"
#include "datafile.h"
#include "error.h"
#include "extension.h"
#include "extwindow.h"
#include "file.h"
#include "rtf.h"
#include "story.h"
#include "tabindex.h"
#include "tabsettings.h"

/*
 * FUNCTIONS FOR SETTING UP FILE MONITORS
 */

static GnomeVFSMonitorHandle *monitor_file(const gchar *filename,
  GnomeVFSMonitorCallback callback, gpointer data) {
    GnomeVFSMonitorHandle *retval;
    GError *err;
    gchar *file_uri;

    if((file_uri = g_filename_to_uri(filename, NULL, &err)) == NULL) {
        /* fail discreetly */
        g_warning("Cannot convert project filename to URI: %s", err->message);
        g_error_free(err);
        return NULL;
    }
    if(gnome_vfs_monitor_add(&retval, file_uri, GNOME_VFS_MONITOR_FILE,
      callback, data) != GNOME_VFS_OK) {
        g_warning("Could not start file monitor for %s", file_uri);
        g_free(file_uri);
        return NULL;
    }
    g_free(file_uri);
    return retval;
}

static void project_changed(GnomeVFSMonitorHandle *handle,
  const gchar *monitor_uri, const gchar *info_uri,
  GnomeVFSMonitorEventType event_type, gpointer data) {
    struct story *thestory = (struct story *)data;
    
    if(event_type == GNOME_VFS_MONITOR_EVENT_CHANGED
      || event_type == GNOME_VFS_MONITOR_EVENT_CREATED) {
        /* g_file_set_contents works by deleting and creating */
        GtkWidget *dialog = gtk_message_dialog_new(
        (GtkWindow *)thestory->window, GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
        "The game's source code has been modified from outside Inform.\n"
        "Do you want to reload it?");
        if(gtk_dialog_run((GtkDialog *)dialog) == GTK_RESPONSE_YES) {
            GError *err = NULL;
            gchar *filename, *text;
            
            /* Read the source */
            if((filename = g_filename_from_uri(info_uri, NULL, &err)) == NULL) {
                g_warning("Cannot get filename from URI: %s", err->message);
                g_error_free(err);
                gtk_widget_destroy(dialog);
                return;
            }
            if(!g_file_get_contents(filename, &text, NULL, &err)) {
                error_dialog((GtkWindow *)thestory->window, err,
                "Could not open the project's source file, '%s'.\n\n"
                "Make sure that this file has not been deleted or renamed. ",
                filename);
                g_error_free(err);
                gtk_widget_destroy(dialog);
                g_free(filename);
                return;
            }
    
            /* Write the source to the source buffer, clearing the undo history */
            gtk_source_buffer_begin_not_undoable_action(thestory->buffer);
            gtk_text_buffer_set_text(
              GTK_TEXT_BUFFER(thestory->buffer), text, -1);
            gtk_source_buffer_end_not_undoable_action(thestory->buffer);
            gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(thestory->buffer),
              FALSE);
            
            g_free(filename);
            g_free(text);
        }
        gtk_widget_destroy(dialog);
    }
}

static void extension_changed(GnomeVFSMonitorHandle *handle,
  const gchar *monitor_uri, const gchar *info_uri,
  GnomeVFSMonitorEventType event_type, gpointer data) {
    struct extension *ext = (struct extension *)data;
      
    if(event_type == GNOME_VFS_MONITOR_EVENT_CHANGED
      || event_type == GNOME_VFS_MONITOR_EVENT_CREATED) {
        GtkWidget *dialog = gtk_message_dialog_new((GtkWindow *)ext->window,
        GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
        "The extension's source code has been modified from outside Inform.\n"
        "Do you want to reload it?");
        if(gtk_dialog_run((GtkDialog *)dialog) == GTK_RESPONSE_YES) {
            GError *err = NULL;
            gchar *filename, *text;
            
            /* Read the source */
            if((filename = g_filename_from_uri(info_uri, NULL, &err)) == NULL) {
                g_warning("Cannot get filename from URI: %s", err->message);
                g_error_free(err);
                gtk_widget_destroy(dialog);
                return;
            }
            if(!g_file_get_contents(filename, &text, NULL, &err)) {
                error_dialog((GtkWindow *)ext->window, err,
                "Could not open the extension '%s': ", filename);
                g_error_free(err);
                gtk_widget_destroy(dialog);
                g_free(filename);
                return;
            }
        
            /* Put the text in the source buffer, clearing the undo history */
            gtk_source_buffer_begin_not_undoable_action(ext->buffer);
            gtk_text_buffer_set_text(GTK_TEXT_BUFFER(ext->buffer), text, -1);
            gtk_source_buffer_end_not_undoable_action(ext->buffer);
            gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(ext->buffer), FALSE);
            
            g_free(filename);
            g_free(text);
        }
        gtk_widget_destroy(dialog);
    }
}

/*
 * FUNCTIONS FOR SAVING AND LOADING STUFF
 */

/* If the document is not saved, ask the user whether he/she wants to save it.
Returns TRUE if we can proceed, FALSE if the user cancelled. */
gboolean verify_save(GtkWidget *thiswidget) {
    struct story *thestory = get_story(thiswidget);
    
    if(gtk_text_buffer_get_modified(GTK_TEXT_BUFFER(thestory->buffer))
      || gtk_text_buffer_get_modified(thestory->notes)) {
        GtkWidget *save_changes_dialog = gtk_message_dialog_new_with_markup(
          GTK_WINDOW(gtk_widget_get_toplevel(thiswidget)),
          GTK_DIALOG_DESTROY_WITH_PARENT,
          GTK_MESSAGE_WARNING,
          GTK_BUTTONS_NONE,
          "<b><big>Save changes to project before closing?</big></b>");
        gtk_message_dialog_format_secondary_text(
          GTK_MESSAGE_DIALOG(save_changes_dialog),
          "If you don't save, your changes will be lost.");
        gtk_dialog_add_buttons(GTK_DIALOG(save_changes_dialog),
          "Close _without saving", GTK_RESPONSE_REJECT,
          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
          GTK_STOCK_SAVE, GTK_RESPONSE_OK,
          NULL);
        gint result = gtk_dialog_run(GTK_DIALOG(save_changes_dialog));
        gtk_widget_destroy(save_changes_dialog);
        switch(result) {
          case GTK_RESPONSE_OK: /* save */
            on_save_activate(GTK_MENU_ITEM(lookup_widget(thiswidget, "save")),
              NULL);
            /* fallthrough; */
          case GTK_RESPONSE_REJECT: /* quit without saving */
            return TRUE;
          default: /* various ways of cancelling the dialog */
            return FALSE;
        }
    }
    return TRUE;
}

/* Read a project directory, loading all the appropriate files into a new
story struct and returning that */
struct story *open_project(gchar *directory) {
    gchar *source_dir = g_build_filename(directory, "Source", NULL);    
    GError *err = NULL;
    gchar *filename, *text;

    struct story *thestory = new_story();
    set_story_filename(thestory, directory);

    /* Read the source */
    filename = g_build_filename(source_dir, "story.ni", NULL);
    g_free(source_dir);
    if(!g_file_get_contents(filename, &text, NULL, &err)) {
        error_dialog(NULL, err, "Could not open the project's source file, "
          "'%s'.\n\nMake sure that this file has not been deleted or renamed. ",
          filename);
        g_free(filename);
        delete_story(thestory);
        return NULL;
    }
    
    /* Update the list of recently used files */
#if !defined(SUCKY_GNOME)
    GtkRecentManager *manager = gtk_recent_manager_get_default();
    /* Add story.ni as the actual file to open, in case any other application
    wants to open it, and set the display name to the project directory */
    gchar *file_uri;
    if((file_uri = g_filename_to_uri(filename, NULL, &err)) == NULL) {
        /* fail discreetly */
        g_warning("Cannot convert project filename to URI: %s", err->message);
        g_error_free(err);
    } else {
        GtkRecentData *recent_data = g_new0(GtkRecentData, 1);
        recent_data->display_name = g_filename_display_basename(directory);
        recent_data->description = g_filename_display_basename(directory);
        recent_data->mime_type = g_strdup("text/x-natural-inform");
        recent_data->app_name = g_strdup("GNOME Inform 7");
        recent_data->app_exec = g_strdup("gnome-inform7 \f");
        /* We use the groups "inform7_project" and "inform7_extension" to
        determine how to open a file from the recent manager */
        recent_data->groups = g_new(gchar *, 2);
        recent_data->groups[0] = g_strdup("inform7_project");
        recent_data->groups[1] = NULL;
        recent_data->is_private = FALSE;
        gtk_recent_manager_add_full(manager, file_uri, recent_data);
        g_strfreev(recent_data->groups);
        g_free(recent_data);
    }
    g_free(file_uri);
#endif
    
    /* Watch for changes to the source file */
    thestory->monitor = monitor_file(filename, project_changed,
      (gpointer)thestory);
    g_free(filename);
    
    /* Write the source to the source buffer, clearing the undo history */
    gtk_source_buffer_begin_not_undoable_action(thestory->buffer);
    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(thestory->buffer), text, -1);
    gtk_source_buffer_end_not_undoable_action(thestory->buffer);
    g_free(text);
    
    /* Read the skein: TODO */

    /* Read the notes */
    filename = g_build_filename(directory, "notes.rtf", NULL);
    if(!g_file_get_contents(filename, &text, NULL, NULL)) {
        /* Don't fail if the file is unreadable; instead, just make some blank notes */
        text = g_strdup(
          "{\\rtf1\\mac\\ansicpg10000\\cocoartf824\\cocoasubrtf410\n"
          "{\\fonttbl}\n"
          "{\\colortbl;\\red255\\green255\\blue255;}\n"
          "}");
    }
    g_free(filename);
    gtk_text_buffer_set_rtf_text(GTK_TEXT_BUFFER(thestory->notes), text);
    g_free(text);
    
    /* Read the settings */
    filename = g_build_filename(directory, "Settings.plist", NULL);
    if(g_file_get_contents(filename, &text, NULL, &err)) {
        gchar **lines = g_strsplit_set(text, "\n\r", -1);
        g_free(text);
        gchar **ptr;
        for(ptr = lines; *ptr != NULL; ptr++) {
            if(strstr(*ptr, "<key>IFSettingCreateBlorb</key>")) {
                if(++ptr == NULL)
                    break;
                if(strstr(*ptr, "<true/>"))
                    thestory->make_blorb = TRUE;
            } else if(strstr(*ptr, "<key>IFSettingZCodeVersion</key>")) {
                if(++ptr == NULL)
                    break;
                int version;
                if(sscanf(*ptr, " <integer> %d </integer> ", &version) == 1
                  && (version == FORMAT_Z5 || version == FORMAT_Z6 
                  || version == FORMAT_Z8 || version == FORMAT_GLULX))
                    thestory->story_format = version;
            }
        }
        g_strfreev(lines);
    } else
        error_dialog(NULL, NULL, "Could not open the project's settings file, "
          "'%s'. Using default settings.", filename);
    g_free(filename);    

    /* Load index tabs if they exist and update settings */
    reload_index_tabs(thestory, FALSE);
    update_settings(thestory);

    GtkTextIter start;
    gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(thestory->buffer), &start);
    gtk_text_buffer_place_cursor(GTK_TEXT_BUFFER(thestory->buffer), &start);
    gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(thestory->buffer), FALSE);
    gtk_text_buffer_set_modified(thestory->notes, FALSE);
    
    return thestory;
}

/* Save the project being edited in the topwindow of thiswidget */
void save_project(GtkWidget *thiswidget, gchar *directory) {
    gchar *build_dir = g_build_filename(directory, "Build", NULL);
    gchar *index_dir = g_build_filename(directory, "Index", NULL);
    gchar *source_dir = g_build_filename(directory, "Source", NULL);
    GError *err = NULL;
    gchar *filename, *text;
    struct story *thestory = get_story(thiswidget);

    gnome_vfs_monitor_cancel(thestory->monitor);
    
    /* Create the project directory if it does not already exist */
    if(g_mkdir_with_parents(directory, 0777) 
      || g_mkdir_with_parents(build_dir, 0777)
      || g_mkdir_with_parents(index_dir, 0777)
      || g_mkdir_with_parents(source_dir, 0777)) {
        error_dialog(GTK_WINDOW(gtk_widget_get_toplevel(thiswidget)), NULL,
          "Error creating project directory: %s", g_strerror(errno));
        g_free(build_dir);
        g_free(index_dir);
        g_free(source_dir);
        return;
    }
    g_free(build_dir);
    g_free(index_dir);
    
    /* Save the source */
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(thestory->buffer), &start, &end);
    /* Get text in UTF-8 */
    text = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(thestory->buffer), &start,
      &end, FALSE);
    
    /* Write text to file */
    filename = g_build_filename(source_dir, "story.ni", NULL);
    g_free(source_dir);
    if(!g_file_set_contents(filename, text, -1, &err)) {
        error_dialog(GTK_WINDOW(gtk_widget_get_toplevel(thiswidget)), err,
          "Error saving file '%s': ", filename);
        g_free(filename);
        g_free(text);
        return;
    }
    g_free(text);

    /* Start file monitoring again */
    thestory->monitor = monitor_file(filename, project_changed,
      (gpointer)thestory);
    g_free(filename);
    
    /* Save the skein */
    /* TODO */

    /* Save the notes */
    gtk_text_buffer_get_bounds(thestory->notes, &start, &end);
    text = gtk_text_buffer_get_rtf_text(thestory->notes, &start, &end);
    filename = g_build_filename(directory, "notes.rtf", NULL);
    if(!g_file_set_contents(filename, text, -1, &err)) {
        error_dialog(GTK_WINDOW(gtk_widget_get_toplevel(thiswidget)), err,
          "Error saving file '%s': ", filename);
        g_free(filename);
        g_free(text);
        return;
    }
    g_free(filename);
    g_free(text);
    
    /* Save the project settings */
    filename = g_build_filename(directory, "Settings.plist", NULL);
    gchar format_string[3];
    g_sprintf(format_string, "%d", thestory->story_format);
    text = g_strconcat(
      "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
      "<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" "
        "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
      "<plist version=\"1.0\">\n"
      "<dict>\n"
      "\t<key>IFCompilerOptions</key>\n"
      "\t<dict>\n"
      "\t\t<key>IFSettingNaturalInform</key>\n"
      "\t\t<true/>\n"
      "\t</dict>\n"
      "\t<key>IFLibrarySettings</key>\n"
      "\t<dict>\n"
      "\t\t<key>IFSettingLibraryToUse</key>\n"
      "\t\t<string>Natural</string>\n"
      "\t</dict>\n"
      "\t<key>IFOutputSettings</key>\n"
      "\t<dict>\n"
      "\t\t<key>IFSettingCreateBlorb</key>\n"
      "\t\t<", thestory->make_blorb ? "true" : "false", "/>\n"
      "\t\t<key>IFSettingZCodeVersion</key>\n"
      "\t\t<integer>", format_string, "</integer>\n"
      "\t</dict>\n"
      "</dict>\n"
      "</plist>\n", NULL);
    if(!g_file_set_contents(filename, text, -1, &err)) {
        error_dialog(GTK_WINDOW(gtk_widget_get_toplevel(thiswidget)), err,
          "Error saving file '%s': ", filename);
        g_free(filename);
        g_free(text);
        return;
    }
    g_free(filename);
    g_free(text);

    /* Delete the build files from the project directory */
    delete_build_files(thestory);
    
    gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(thestory->buffer), FALSE);
    gtk_text_buffer_set_modified(thestory->notes, FALSE);
    config_file_set_string("Settings", "LastProject", directory);
}

/* A version of verify_save for the extension editing window */
gboolean verify_save_ext(GtkWidget *thiswidget) {
    struct extension *ext = get_ext(thiswidget);
    
    if(gtk_text_buffer_get_modified(GTK_TEXT_BUFFER(ext->buffer))) {
        GtkWidget *save_changes_dialog = gtk_message_dialog_new_with_markup(
          GTK_WINDOW(gtk_widget_get_toplevel(thiswidget)),
          GTK_DIALOG_DESTROY_WITH_PARENT,
          GTK_MESSAGE_WARNING,
          GTK_BUTTONS_NONE,
          "<b><big>Save changes to '%s' before closing?</big></b>",
          ext->filename);
        gtk_message_dialog_format_secondary_text(
          GTK_MESSAGE_DIALOG(save_changes_dialog),
          "If you don't save, your changes will be lost.");
        gtk_dialog_add_buttons(GTK_DIALOG(save_changes_dialog),
          "Close _without saving", GTK_RESPONSE_REJECT,
          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
          GTK_STOCK_SAVE, GTK_RESPONSE_OK,
          NULL);
        gint result = gtk_dialog_run(GTK_DIALOG(save_changes_dialog));
        gtk_widget_destroy(save_changes_dialog);
        switch(result) {
          case GTK_RESPONSE_OK: /* save */
            on_xsave_activate(GTK_MENU_ITEM(lookup_widget(thiswidget, "xsave")),
              NULL);
            /* fallthrough; */
          case GTK_RESPONSE_REJECT: /* quit without saving */
            return TRUE;
          default: /* various ways of cancelling the dialog */
            return FALSE;
        }
    }
    return TRUE;
}

/* Reads an extension file to check whether it is a valid Inform 7 extension,
and returns the extension name and author stored in the locations pointed to by
'name' and 'author'. If the function returns TRUE, they must be freed
afterwards. This function was adapted from the Windows source. */
static gboolean is_valid_extension(const gchar *text, gchar **thename,
gchar **theauthor) {
    g_return_val_if_fail(text != NULL, FALSE);
    
    /* Separate the first line of the extension */
    /* '\r' is added to the string to recognize other systems' return
    characters; even though the ni compiler doesn't seem to do that. */
    gchar *firstline = g_strndup(text, strcspn(text, "\n\r"));
    
    /* Make sure the file is not binary; there has GOT to be a better way! */
    gchar *pntr;
    for(pntr = firstline; *pntr != '\0'; pntr++)
        if(!isprint(*pntr)) {
            g_free(firstline);
            return FALSE; /* file is binary */
        }
    
    gchar **tokens = g_strsplit_set(g_strstrip(firstline), " \t", 0);
    g_free(firstline);
    gchar **ptr = tokens;
    
    if(ptr == NULL)
        return FALSE;
    /* Skip "Version XXXX of" */
    if(!strcmp(ptr[0], "Version")) {
        if(ptr[1] == NULL || ptr[2] == NULL || strcmp(ptr[2], "of")) {
            g_strfreev(tokens);
            return FALSE;
        }
        ptr += 3;
    }
    
    /* Skip "The" */
    if(ptr[0] != NULL && (!strcmp(ptr[0], "The") || !strcmp(ptr[0], "the")))
        ptr++;
    
    gchar *name = NULL;
    while(ptr[0] != NULL && strcmp(ptr[0], "by")) {
        if(name) {
            gchar *newname = g_strconcat(name, " ", ptr[0], NULL);
            g_free(name);
            name = newname;
        } else
            name = g_strdup(ptr[0]);
        ptr++;
    }
    ptr++; /* Skip "by" */
    
    gchar *author = NULL;
    while(ptr[0] != NULL &&
      strcmp(ptr[0], "begins") && strcmp(ptr[0], "begin")) {
        /* Author's name ends before "begin(s) here", or at end of line */
        if(author) {
            gchar *newauthor = g_strconcat(author, " ", ptr[0], NULL);
            g_free(author);
            author = newauthor;
        } else
            author = g_strdup(ptr[0]);
        ptr++;
    }
    g_strfreev(tokens);
    
    if(!name) {
        if(!author)
            g_free(author);
        return FALSE;
    }
    if(!author) {
        g_free(name);
        return FALSE;
    }
    *thename = g_strdup(name);
    *theauthor = g_strdup(author);
    g_free(name);
    g_free(author);
    return TRUE;
}

/* Opens the extension from filename, and returns a new extension struct. */
struct extension *open_extension(gchar *filename) {
    GError *err = NULL;
    gchar *text;

    struct extension *ext = new_ext();
    set_ext_filename(ext, filename);

    /* Read the source */
    if(!g_file_get_contents(filename, &text, NULL, &err)) {
        error_dialog(NULL, err, "Could not open the extension '%s': ",filename);
        delete_ext(ext);
        return NULL;
    }

    /* Put the text in the source buffer, clearing the undo history */
    gtk_source_buffer_begin_not_undoable_action(ext->buffer);
    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(ext->buffer), text, -1);
    gtk_source_buffer_end_not_undoable_action(ext->buffer);

    GtkTextIter start;
    gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(ext->buffer), &start);
    gtk_text_buffer_place_cursor(GTK_TEXT_BUFFER(ext->buffer), &start);
    gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(ext->buffer), FALSE);
    
    /* Update the list of recently used files */
#if !defined(SUCKY_GNOME)
    GtkRecentManager *manager = gtk_recent_manager_get_default();
    gchar *file_uri;
    if((file_uri = g_filename_to_uri(filename, NULL, &err)) == NULL) {
        /* fail discreetly */
        g_warning("Cannot convert extension filename to URI: %s", err->message);
        g_error_free(err);
    } else {
        GtkRecentData *recent_data = g_new0(GtkRecentData, 1);
        recent_data->display_name = g_filename_display_basename(filename);
        recent_data->description = g_filename_display_basename(filename);
        recent_data->mime_type = g_strdup("text/x-natural-inform");
        recent_data->app_name = g_strdup("GNOME Inform 7");
        recent_data->app_exec = g_strdup("gnome-inform7");
        /* We use the groups "inform7_project" and "inform7_extension" to
        determine how to open a file from the recent manager */
        recent_data->groups = g_new(gchar *, 2);
        recent_data->groups[0] = g_strdup("inform7_extension");
        recent_data->groups[1] = NULL;
        recent_data->is_private = FALSE;
        gtk_recent_manager_add_full(manager, file_uri, recent_data);
        g_strfreev(recent_data->groups);
        g_free(recent_data);
        g_free(file_uri);
    }
#endif
    
    ext->monitor = monitor_file(filename, extension_changed, (gpointer)ext);
    
    return ext;
}

/* Write the extension being edited in the topwindow of thiswidget to disk. */
void save_extension(GtkWidget *thiswidget) {
    GError *err = NULL;
    gchar *text;
    struct extension *ext = get_ext(thiswidget);

    /* Stop file monitoring */
    gnome_vfs_monitor_cancel(ext->monitor);
    
    /* Save the text */
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(ext->buffer), &start);
    gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(ext->buffer), &end);
    /* Get text in UTF-8 */
    text = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(ext->buffer), &start,
      &end, FALSE);
    
    /* Write text to file */
    if(!g_file_set_contents(ext->filename, text, -1, &err)) {
        error_dialog(GTK_WINDOW(gtk_widget_get_toplevel(thiswidget)), NULL,
          "Error saving file '%s': ", ext->filename);
        g_free(text);
        return;
    }
    g_free(text);

    ext->monitor = monitor_file(ext->filename, extension_changed,
      (gpointer)ext);
    
    gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(ext->buffer), FALSE);
}

/* Install the extension at filename into the user's extensions dir */
void install_extension(const gchar *filename) {
    g_return_if_fail(filename != NULL);
    
    gchar *text;
    GError *err = NULL;
    
    if(!g_file_get_contents(filename, &text, NULL, &err)) {
        error_dialog(NULL, err, "Error reading file '%s': ", filename);
        g_error_free(err);
        return;
    }
    
    /* Make sure the file is actually an Inform 7 extension */
    gchar *name = NULL;
    gchar *author = NULL;
    if(!is_valid_extension(text, &name, &author)) {
        error_dialog(NULL, NULL, "The file '%s' does not seem to be an "
          "extension. Extensions should be saved as UTF-8 text format files, "
          "and should start with a line of one of these forms:\n\n<Extension> "
          "by <Author> begins here.\nVersion <Version> of <Extension> by "
          "<Author> begins here.", filename);
        g_free(text);
        return;
    }
    
    /* Get the lowercase names for the author and extension */
    gchar *author_lc = g_utf8_strdown(author, -1);
    gchar *name_lc = g_utf8_strdown(name, -1);
    
    /* Create the directory for that author if it does not exist already */
    gchar *dir = get_extension_path(author, NULL);
    gchar *dir_lc = get_extension_path(author_lc, NULL);
    
    if(!g_file_test(dir, G_FILE_TEST_EXISTS)) {
        if(g_file_test(dir_lc, G_FILE_TEST_EXISTS)) {
            error_dialog(NULL, NULL, "A file called '%s' already exists. GNOME "
              "Inform 7 needs to use this name to link to the extension. Remove"
              " the file and try again.", dir_lc);
            g_free(name);   g_free(name_lc);
            g_free(author); g_free(author_lc);
            g_free(dir);    g_free(dir_lc);
            g_free(text);
            return;
        }
        if(g_mkdir_with_parents(dir, 0777) == -1) {
            error_dialog(NULL, NULL, "Error creating directory '%s'.", dir);
            g_free(name);   g_free(name_lc);
            g_free(author); g_free(author_lc);
            g_free(dir);    g_free(dir_lc);
            g_free(text);
            return;
        }
        if(symlink(dir, dir_lc)) {
            error_dialog(NULL, NULL, "Error linking '%s' to '%s'.", dir_lc,dir);
            g_free(name);   g_free(name_lc);
            g_free(author); g_free(author_lc);
            g_free(dir);    g_free(dir_lc);
            g_free(text);
            return;
        }
    }
    
    gchar *targetfile = g_build_filename(dir, name, NULL);
    gchar *targetfile_lc = g_build_filename(dir_lc, name_lc, NULL);
    g_free(dir);    g_free(dir_lc);
    g_free(author_lc);
    g_free(name_lc);

    /* Check if the extension is already installed */
    if(g_file_test(targetfile, G_FILE_TEST_EXISTS)) {
        GtkWidget *dialog = gtk_message_dialog_new(NULL, 0,
          GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
          "A version of the extension %s by %s is already installed. Do you "
          "want to overwrite the installed extension with this new one?",
          name, author);
        if(gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_YES) {
            gtk_widget_destroy(dialog);
            g_free(targetfile); g_free(targetfile_lc);
            g_free(name);
            g_free(author);
            g_free(text);
            return;
        }
        gtk_widget_destroy(dialog);
    }
    if(g_file_test(targetfile_lc, G_FILE_TEST_EXISTS)) {
        error_dialog(NULL, NULL, "A file called '%s' already exists. GNOME "
          "Inform 7 needs to use this name to link to the extension. Remove the"
          " file and try again.", targetfile_lc);
        g_free(targetfile); g_free(targetfile_lc);
        g_free(name);
        g_free(author);
        g_free(text);
        return;
    }
    
    /* Change newline separators to \n */
    if(strstr(text, "\r\n")) {
        gchar **lines = g_strsplit(text, "\r\n", 0);
        g_free(text);
        text = g_strjoinv("\n", lines);
        g_strfreev(lines);
    } else if(strstr(text, "\r")) {
        gchar **lines = g_strsplit(text, "\r", 0);
        g_free(text);
        text = g_strjoinv("\n", lines);
        g_strfreev(lines);
    }
    
    /* Copy the extension file to the user's extensions dir */
    if(!g_file_set_contents(targetfile, text, -1, &err)) {
        error_dialog(NULL, NULL, "Error copying file '%s' to '%s': ", filename,
          targetfile);
        g_free(text);
        g_free(targetfile); g_free(targetfile_lc);
        g_free(name);
        g_free(author);
        return;
    }
    
    /* Make a lowercase link */
    if(symlink(targetfile, targetfile_lc)) {
        error_dialog(NULL, NULL, "Error linking '%s' to '%s'.", targetfile_lc,
          targetfile_lc);
        g_free(text);
        g_free(targetfile); g_free(targetfile_lc);
        g_free(name);
        g_free(author);
        return;
    }
    
    g_free(text);
    g_free(targetfile); g_free(targetfile_lc);
    g_free(name);
    g_free(author);

    /* Index the new extensions, in the foreground */
    run_census(TRUE);
}


/* Delete extension and remove author dir if empty */
void delete_extension(gchar *author, gchar *extname) {
    /* Remove extension */
    gchar *filename = get_extension_path(author, extname);
    if(g_remove(filename) == -1) {
        GtkWidget *dialog = gtk_message_dialog_new(NULL,
          GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
          "There was an error removing %s.", filename);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        g_free(filename);
        return;
    }
    g_free(filename);
    
    /* Remove lowercase symlink to extension */
    gchar *extname_lc = g_utf8_strdown(extname, -1);
    filename = get_extension_path(author, extname_lc);
    g_free(extname_lc);
    if(g_remove(filename) == -1) {
        GtkWidget *dialog = gtk_message_dialog_new(NULL,
          GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
          "There was an error removing %s.", filename);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        g_free(filename);
        return;
    }
    g_free(filename);
    
    /* Remove author directory if empty */
    filename = get_extension_path(author, NULL); 
    if(g_rmdir(filename) == -1) {
        /* if there were other extensions, just return; but if it failed for any
        other reason, display an error */
        if(errno != ENOTEMPTY) {
            GtkWidget *dialog = gtk_message_dialog_new(NULL,
              GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
              "There was an error removing %s.", filename);
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
        }
        g_free(filename);
        return;
    }
    g_free(filename);
    
    /* Remove lowercase symlink to author directory */
    gchar *author_lc = g_utf8_strdown(author, -1);
    filename = get_extension_path(author_lc, NULL);
    g_free(author_lc);
    if(g_remove(filename) == -1) {
        GtkWidget *dialog = gtk_message_dialog_new(NULL,
          GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
          "There was an error removing %s.", filename);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
    g_free(filename);
}


/* Helper function to delete a file relative to the project path; does nothing
if file does not exist */
static void delete_from_project_dir(struct story *thestory, gchar *subdir,
gchar *filename) {
    gchar *pathname;
    
    if(subdir)
        pathname = g_build_filename(thestory->filename, subdir, filename, NULL);
    else
        pathname = g_build_filename(thestory->filename, filename, NULL);
    
    g_remove(pathname);
    g_free(pathname);
}

/* If the "delete build files" option is checked, delete all the build files
from the project directory */
void delete_build_files(struct story *thestory) {
    if(config_file_get_bool("Cleaning", "BuildFiles")) {
        delete_from_project_dir(thestory, NULL, "Metadata.iFiction");
        delete_from_project_dir(thestory, NULL, "Release.blurb");
        delete_from_project_dir(thestory, "Build", "auto.inf");
        delete_from_project_dir(thestory, "Build", "Debug log.txt");
        delete_from_project_dir(thestory, "Build", "Map.eps");
        delete_from_project_dir(thestory, "Build", "output.z5");
        delete_from_project_dir(thestory, "Build", "output.z8");
        delete_from_project_dir(thestory, "Build", "output.ulx");
        delete_from_project_dir(thestory, "Build", "Problems.html");
        delete_from_project_dir(thestory, "Build", "temporary file.inf");
        
        if(config_file_get_bool("Cleaning", "IndexFiles")) {
            delete_from_project_dir(thestory, "Index", "Actions.html");
            delete_from_project_dir(thestory, "Index", "Contents.html");
            delete_from_project_dir(thestory, "Index", "Headings.xml");
            delete_from_project_dir(thestory, "Index", "Kinds.html");
            delete_from_project_dir(thestory, "Index", "Phrasebook.html");
            delete_from_project_dir(thestory, "Index", "Rules.html");
            delete_from_project_dir(thestory, "Index", "Scenes.html");
            delete_from_project_dir(thestory, "Index", "World.html");
        }
    }
}
