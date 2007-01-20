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
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>
#include <gtksourceview/gtksourcebuffer.h>

#include "story.h"
#include "support.h"
#include "callbacks.h"
#include "tabsettings.h"
#include "appwindow.h"
#include "extension.h"
#include "configfile.h"
#include "error.h"
#include "compile.h"
#include "file.h"
#include "rtf.h"

/* If the document is not saved, ask the user whether he/she wants to save it.
Returns TRUE if we can proceed, FALSE if the user cancelled. */
gboolean verify_save(GtkWidget *thiswidget) {
    struct story *thestory = get_story(thiswidget);
    
    if(gtk_text_buffer_get_modified(GTK_TEXT_BUFFER(thestory->buffer))) {
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

/* DEBUGGING FUNCTION */
/* static void
print_element_names(xmlNode * a_node)
{
    xmlNode *cur_node = NULL;

    for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
        fprintf(stderr, "name: %s children: {", cur_node->name);
        print_element_names(cur_node->children);
        fprintf(stderr, "}");
    }
}*/

/* iterate through the XML document to find the node with name nodename */
xmlNode *find_node(xmlDoc *doc, xmlChar *nodename) {
    xmlNode *node = xmlDocGetRootElement(doc);
    xmlChar *content, *name;
    node = node->children;
    while (node != NULL) {
        name = xmlStrdup(node->name);
        content = xmlNodeGetContent(node);
        if(!xmlStrcmp(name, (xmlChar *)"dict") &&
          xmlStrstr(content, nodename)) {
            node = node->children;
        } else if(!xmlStrcmp(name, (xmlChar *)"key") &&
          !xmlStrcmp(content, nodename)) {
           /* fprintf(stderr, "%s - %s\n", name, content);*/
            xmlFree(name);
            xmlFree(content);
            return node;   
        } else {
            node = node->next;
        }
        xmlFree(name);
        xmlFree(content);
    }
    return NULL;
}

/* Read a project directory, loading all the appropriate files into a new
story struct and returning that */
struct story *open_project(gchar *directory) {
    gchar *source_dir = g_strconcat(directory, "/Source", NULL);    
    GError *err = NULL;
    gchar *filename, *text;

    struct story *thestory = new_story();
    set_story_filename(thestory, directory);

    /* Read the source */
    filename = g_strconcat(source_dir, "/story.ni", NULL);
    if(!g_file_get_contents(filename, &text, NULL, &err)) {
        error_dialog(NULL, err, "Could not open the project's source file, "
          "'%s'.\n\nMake sure that this file has not been deleted or renamed. ",
          filename);
        g_free(filename);
        delete_story(thestory);
        return NULL;
    }
    g_free(filename);
    
    /* Write the source to the source buffer, clearing the undo history */
    gtk_source_buffer_begin_not_undoable_action(thestory->buffer);
    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(thestory->buffer), text, -1);
    gtk_source_buffer_end_not_undoable_action(thestory->buffer);
    g_free(text);
    
    /* Read the skein: TODO */

    /* Read the notes */
    filename = g_strconcat(directory, "/notes.rtf", NULL);
    if(!g_file_get_contents(filename, &text, NULL, &err)) {
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
    filename = g_strconcat(directory, "/Settings.plist", NULL);

    xmlDoc *doc = xmlReadFile(filename, NULL, 0);
    if (doc == NULL) {
        error_dialog(NULL, NULL, "Failed to parse '%s'. Using default settings",
          filename);
        g_free(filename);
        return NULL;
    }
    g_free(filename);
    
    /* To do: This part sucks, because there is almost no documentation for
    libxml2. There must be some better way to do this. */
    xmlNode *node = find_node(doc, (xmlChar *)"IFSettingCreateBlorb");
    if(node) {
        node = node->next;
        thestory->make_blorb = !xmlStrcmp(node->name,
          (xmlChar *)"true") ? TRUE : FALSE;
    } /* else default setting */

    node = find_node(doc, (xmlChar *)"IFSettingZCodeVersion");
    if(node) {
        node = node->next;
        xmlChar *content = xmlNodeGetContent(node);
        thestory->story_format = atoi((char *)content);
        xmlFree(content);
    } /* else default setting */
    xmlFreeDoc(doc);

    /* Load index tabs if they exist and update settings */
    reload_index_tabs(thestory->window);
    update_settings(thestory);

    GtkTextIter start;
    gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(thestory->buffer), &start);
    gtk_text_buffer_place_cursor(GTK_TEXT_BUFFER(thestory->buffer), &start);
    gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(thestory->buffer), FALSE);

    /* Update the list of recently used files */
    GtkRecentManager *manager = gtk_recent_manager_get_default();
    gchar *file_uri;
    if((file_uri = g_filename_to_uri(directory, NULL, &err)) == NULL) {
        /* fail discreetly */
        g_warning("Cannot convert project filename to URI: %s", err->message);
        g_error_free(err);
    } else {
        GtkRecentData *recent_data = g_new0(GtkRecentData, 1);
        recent_data->mime_type = g_strdup("text/x-natural-inform");
        recent_data->app_name = g_strdup("GNOME Inform 7");
        recent_data->app_exec = g_strdup("gnome-inform7");
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
    
    return thestory;
}

/* Save the project being edited in the topwindow of thiswidget */
void save_project(GtkWidget *thiswidget, gchar *directory) {
    gchar *build_dir = g_strconcat(directory, "/Build", NULL);
    gchar *index_dir = g_strconcat(directory, "/Index", NULL);
    gchar *source_dir = g_strconcat(directory, "/Source", NULL);
    GError *err = NULL;
    gchar *filename, *text;
    struct story *thestory = get_story(thiswidget);

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
    gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(thestory->buffer), &start);
    gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(thestory->buffer), &end);
    /* Get text in UTF-8 */
    text = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(thestory->buffer), &start,
      &end, FALSE);
    
    /* Write text to file */
    filename = g_strconcat(source_dir, "/story.ni", NULL);
    if(!g_file_set_contents(filename, text, -1, &err)) {
        error_dialog(GTK_WINDOW(gtk_widget_get_toplevel(thiswidget)), err,
          "Error saving file '%s': ", filename);
        g_free(source_dir);
        g_free(filename);
        g_free(text);
        return;
    }
    g_free(source_dir);
    g_free(filename);
    g_free(text);

    /* Save the skein */
    /* TODO */

    /* Save the notes */
    gtk_text_buffer_get_bounds(thestory->notes, &start, &end);
    text = gtk_text_buffer_get_rtf_text(thestory->notes, &start, &end);
    filename = g_strconcat(directory, "/notes.rtf", NULL);
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
    filename = g_strconcat(directory, "/Settings.plist", NULL);
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
and returns the extension name and author. Does dangerous things with the
strings. 'name' and 'author' must both be allocated to at least maxsize. This
function was adapted from the Windows source. */
static gboolean is_valid_extension(const gchar *text, gchar *name,
gchar *author, guint maxsize) {
    g_return_val_if_fail(text != NULL, FALSE);
    g_return_val_if_fail(name != NULL, FALSE);
    g_return_val_if_fail(author != NULL, FALSE);
    
    gchar *firstline = g_strdup(text);
    *(strchr(firstline, '\n')) = '\0';
    if(maxsize < strlen(firstline)) {
        g_free(firstline);
        error_dialog(NULL, NULL, "There was a programming error in 'file.c' at "
        "line 299. Please notify the author of GNOME Inform 7.");
        return FALSE;
    }
    
    gchar **tokens = g_strsplit_set(g_strstrip(firstline), " \t", 0);
    g_free(firstline);
    gchar **ptr = tokens;
    
    if(ptr == NULL)
        return FALSE;
    /* Skip "Version XXXX of" */
    if(!strcmp(ptr[0], "Version")) {
        if(ptr[1] == NULL || ptr[2] == NULL || !strcmp(ptr[2], "of")) {
            g_strfreev(tokens);
            return FALSE;
        }
        ptr += 3;
    }
    
    /* Skip "The" */
    if(ptr[0] != NULL && (!strcmp(ptr[0], "The") || !strcmp(ptr[0], "the")))
        ptr++;
    
    strcpy(name, "");
    while(ptr[0] != NULL && strcmp(ptr[0], "by")) {
        if(strlen(name))
            g_strlcat(name, " ", maxsize);
        g_strlcat(name, ptr[0], maxsize);
        ptr++;
    }
    ptr++; /* Skip "by" */
    strcpy(author, "");
    while(ptr[0] != NULL && strcmp(ptr[0], "begins")) {
        /* Author's name ends before "begins here", or at end of line */
        if(strlen(author))
            g_strlcat(author, " ", maxsize);
        g_strlcat(author, ptr[0], maxsize);
        ptr++;
    }
    g_strfreev(tokens);
    
    if(!strlen(name))
        return FALSE;
    if(!strlen(author))
        return FALSE;
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
    GtkRecentManager *manager = gtk_recent_manager_get_default();
    gchar *file_uri;
    if((file_uri = g_filename_to_uri(filename, NULL, &err)) == NULL) {
        /* fail discreetly */
        g_warning("Cannot convert extension filename to URI: %s", err->message);
        g_error_free(err);
    } else {
        GtkRecentData *recent_data = g_new0(GtkRecentData, 1);
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
    }
    
    return ext;
}

/* Write the extension being edited in the topwindow of thiswidget to disk. */
void save_extension(GtkWidget *thiswidget) {
    GError *err = NULL;
    gchar *text;
    struct extension *ext = get_ext(thiswidget);

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
    gchar *name = g_malloc(1024);
    gchar *author = g_malloc(1024);
    if(!is_valid_extension(text, name, author, 1024)) {
        error_dialog(NULL, NULL, "The file '%s' does not seem to be an "
          "extension. Extensions should be saved as UTF-8 text format files, "
          "and should start with a line of one of these forms:\n\n<Extension> "
          "by <Author> begins here.\nVersion <Version> of <Extension> by "
          "<Author> begins here.", filename);
        g_free(text);
        g_free(name);
        g_free(author);
        return;
    }

    /* Create the directory for that author if it does not exist already */
    gchar *dir = get_extension_path(author, NULL);
    if(g_mkdir_with_parents(dir, 0777) == -1) {
        error_dialog(NULL, NULL, "Error creating directory '%s'.", dir);
        g_free(name);
        g_free(author);
        g_free(dir);
        g_free(text);
        return;
    }
    gchar *targetfile = g_strconcat(dir, "/", name, NULL);
    g_free(dir);

    /* Check if the extension is already installed */
    if(g_file_test(targetfile, G_FILE_TEST_EXISTS)) {
        GtkWidget *dialog = gtk_message_dialog_new(NULL, 0,
          GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
          "A version of the extension %s by %s is already installed. Do you "
          "want to overwrite the installed extension with this new one?",
          name, author);
        if(gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_YES) {
            gtk_widget_destroy(dialog);
            g_free(targetfile);
            g_free(name);
            g_free(author);
            g_free(text);
            return;
        }
        gtk_widget_destroy(dialog);
    }
    
    /* Copy the extension file to the user's extensions dir */
    if(!g_file_set_contents(targetfile, text, -1, &err)) {
        error_dialog(NULL, NULL, "Error copying file '%s' to '%s': ", filename,
          targetfile);
        g_free(text);
        g_free(targetfile);
        g_free(name);
        g_free(author);
        return;
    }
    
    g_free(text);
    g_free(targetfile);
    g_free(name);
    g_free(author);

    /* Index the new extensions, in the foreground */
    run_census(TRUE);
}

/* Finish off the release process by choosing a location to store the project */
void finish_release(struct story *thestory, gboolean everything_ok) {
    if(!everything_ok)
        return;
    
    GError *err = NULL;
    
    /* make up a release file name */
    gchar *ext = g_strdup(thestory->make_blorb?
      "zblorb" : get_story_extension(thestory));
    gchar *name = g_path_get_basename(thestory->filename);
    gchar *pos = strchr(name, '.');
    *pos = '\0';
    gchar *filename = g_strconcat(name, ".", ext, NULL);    
    g_free(name);
    g_free(ext);
    
    /* Create a file chooser */
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Save the game for release",
      GTK_WINDOW(thestory->window), GTK_FILE_CHOOSER_ACTION_SAVE,
      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
      NULL);
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog),
      TRUE);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), filename);
    g_free(filename);
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Game Files (.z?,.zblorb)");
    gtk_file_filter_add_pattern(filter, "*.z?");
    gtk_file_filter_add_pattern(filter, "*.zblorb");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    
    /* Copy the finished file to the release location */
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gchar *oldfile = g_strconcat(thestory->filename, "/Build/output.",
          get_story_extension(thestory), NULL);
        gsize bytes_read;
        gchar *text;
        
        if(!g_file_get_contents(oldfile, &text, &bytes_read, &err)) {
            error_dialog(NULL, err, "Error reading file '%s': ", oldfile);
            return;
        }
        if(!g_file_set_contents(filename, text, bytes_read, &err)) {
            error_dialog(NULL, err, "Error reading file '%s': ", filename);
            g_free(text);
            return;
        }
        g_free(filename);
        g_free(oldfile);
        g_free(text);
    }
    gtk_widget_destroy(dialog);
}

/* Helper function to delete a file relative to the project path */
static void delete_from_project_dir(struct story *thestory, gchar *filename) {
    gchar *pathname = g_strconcat(thestory->filename, filename, NULL);
    g_remove(pathname);
    g_free(pathname);
}

/* If the "delete build files" option is checked, delete all the build files
from the project directory */
void delete_build_files(struct story *thestory) {
    if(config_file_get_bool("Cleaning", "BuildFiles")) {
        delete_from_project_dir(thestory, "/Metadata.iFiction");
        delete_from_project_dir(thestory, "/Release.blurb");
        delete_from_project_dir(thestory, "/Build/auto.inf");
        delete_from_project_dir(thestory, "/Build/Debug log.txt");
        delete_from_project_dir(thestory, "/Build/Map.eps");
        delete_from_project_dir(thestory, "/Build/output.z5");
        delete_from_project_dir(thestory, "/Build/output.z8");
        delete_from_project_dir(thestory, "/Build/output.ulx");
        delete_from_project_dir(thestory, "/Build/Problems.html");
        delete_from_project_dir(thestory, "/Build/temporary file.inf");
        
        if(config_file_get_bool("Cleaning", "IndexFiles")) {
            delete_from_project_dir(thestory, "/Index/Actions.html");
            delete_from_project_dir(thestory, "/Index/Contents.html");
            delete_from_project_dir(thestory, "/Index/Headings.xml");
            delete_from_project_dir(thestory, "/Index/Kinds.html");
            delete_from_project_dir(thestory, "/Index/Phrasebook.html");
            delete_from_project_dir(thestory, "/Index/Rules.html");
            delete_from_project_dir(thestory, "/Index/Scenes.html");
            delete_from_project_dir(thestory, "/Index/World.html");
        }
    }
}
