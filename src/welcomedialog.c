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

#include "interface.h"
#include "support.h"

#include "configfile.h"
#include "datafile.h"
#include "file.h"
#include "story.h"
#include "welcomedialog.h"

/* Returns the label widget from a GtkButton or NULL if not found */
static GtkWidget *gtk_button_get_label_widget(GtkWidget *button) {
    GtkBin *alignment = GTK_BIN(gtk_bin_get_child(GTK_BIN(button)));
    GtkContainer *hbox = GTK_CONTAINER(gtk_bin_get_child(alignment));
    GList *boxchildren = gtk_container_get_children(hbox);
    GList *iter = boxchildren;
    GtkWidget *label = NULL;
    for( ; iter != NULL; iter = g_list_next(iter))
        if(GTK_IS_LABEL(iter->data)) {
            label = iter->data;
            break;
        }
    g_list_free(boxchildren);
    return label;
}

void
after_welcome_dialog_realize           (GtkWidget       *widget,
                                        gpointer         user_data)
{
    /* Set the background pixmap for this window */
    GtkRcStyle *newstyle = gtk_widget_get_modifier_style(widget);
    gchar *filename = g_build_filename("gnome-inform7",
                                       "welcome-background.png", NULL);
    newstyle->bg_pixmap_name[GTK_STATE_NORMAL] =
      gnome_program_locate_file(gnome_program_get(),
      GNOME_FILE_DOMAIN_APP_PIXMAP, filename,
      TRUE, NULL);
    g_free(filename);
    gtk_widget_modify_style(widget, newstyle);
    
    /* Set the font size to 14 pixels for the widgets in this window */
    PangoFontDescription *font = pango_font_description_new();
    pango_font_description_set_absolute_size(font, 14 * PANGO_SCALE);
    gtk_widget_modify_font(lookup_widget(widget, "welcome_label"), font);
    gtk_widget_modify_font(
      gtk_button_get_label_widget(lookup_widget(widget, "welcome_new_button")),
      font);
    gtk_widget_modify_font(
      gtk_button_get_label_widget(
      lookup_widget(widget, "welcome_reopen_button")),
      font);
    gtk_widget_modify_font(
      gtk_button_get_label_widget(lookup_widget(widget, "welcome_open_button")),
      font);
    pango_font_description_free(font);
    
    /* If there is no "last project", make the reopen button inactive */
#ifndef SUCKY_GNOME
    GtkRecentManager *manager = gtk_recent_manager_get_default();
    GList *recent = gtk_recent_manager_get_items(manager);
    GList *iter;
    for(iter = recent; iter != NULL; iter = g_list_next(iter)) {
        if(gtk_recent_info_has_application((GtkRecentInfo *)(iter->data),
          "GNOME Inform 7") 
          && gtk_recent_info_has_group((GtkRecentInfo *)(iter->data),
          "inform7_project")) {
            gtk_widget_set_sensitive(
              lookup_widget(widget, "welcome_reopen_button"), TRUE);
            break;
        }
    }
    /* free the list */
    for(iter = recent; iter != NULL; iter = g_list_next(iter)) {
        gtk_recent_info_unref((GtkRecentInfo *)(iter->data));
    }
    g_list_free(recent);
#else
    gchar *filename = config_file_get_string("Settings", "LastProject");
    if(filename)
        gtk_widget_set_sensitive(
              lookup_widget(widget, "welcome_reopen_button"), TRUE);
    g_free(filename);
#endif /* SUCKY_GNOME */
}

void
on_welcome_new_button_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
    GtkWidget *new_dialog = create_new_dialog();
    gtk_widget_show(new_dialog);
    gtk_widget_destroy(gtk_widget_get_toplevel(GTK_WIDGET(button)));
}

void
on_welcome_open_button_clicked         (GtkButton       *button,
                                        gpointer         user_data)
{
    Story *thestory;

    gtk_widget_hide(gtk_widget_get_toplevel(GTK_WIDGET(button)));

    /* Create a file chooser for *.inform */
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Open Project",
      GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(button))),
      GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, "*.inform");
    gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog), filter);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(
          GTK_FILE_CHOOSER(dialog));
        thestory = open_project(filename);
        g_free(filename);
        gtk_widget_destroy(dialog);
    } else {
        gtk_widget_show(gtk_widget_get_toplevel(GTK_WIDGET(button)));
        gtk_widget_destroy(dialog);
        return;
    }

    if(thestory == NULL) {
        /* Take us back to the welcome dialog */
        gtk_widget_show(gtk_widget_get_toplevel(GTK_WIDGET(button)));
        return;
    }
    gtk_widget_show(thestory->window);
    gtk_widget_destroy(gtk_widget_get_toplevel(GTK_WIDGET(button)));
}

void
on_welcome_reopen_button_clicked       (GtkButton       *button,
                                        gpointer         user_data)
{
#ifndef SUCKY_GNOME
    GtkRecentManager *manager = gtk_recent_manager_get_default();
    GList *recent = gtk_recent_manager_get_items(manager);
    GList *iter;
    time_t timestamp, latest = 0;
    gchar *projectname = NULL;
    GError *err;
    
    for(iter = recent; iter != NULL; iter = g_list_next(iter)) {
        GtkRecentInfo *info = gtk_recent_info_ref(
          (GtkRecentInfo *)(iter->data));
        if(gtk_recent_info_has_application(info, "GNOME Inform 7")
          && gtk_recent_info_get_application_info(info, "GNOME Inform 7", NULL,
          NULL, &timestamp)) {
            if(gtk_recent_info_has_group(info, "inform7_project")) {
                if(latest == 0 || difftime(timestamp, latest) > 0) {
                    latest = timestamp;
                    if(projectname)
                        g_free(projectname);
                    if((projectname = g_filename_from_uri(
                      gtk_recent_info_get_uri(info), NULL, &err)) == NULL) {
                        g_warning("Cannot get filename from URI: %s",
                          err->message);
                        g_error_free(err);
                    }
                }
            }
        }
        gtk_recent_info_unref(info);
    }
    /* free the list */
    for(iter = recent; iter != NULL; iter = g_list_next(iter)) {
        gtk_recent_info_unref((GtkRecentInfo *)(iter->data));
    }
    g_list_free(recent);

    g_return_if_fail(projectname); /* Button not sensitive if no last project */
    
    gchar *trash = g_path_get_dirname(projectname); /* Remove "story.ni" */
    gchar *projectdir = g_path_get_dirname(trash); /* Remove "Source" */
    g_free(trash);
    g_free(projectname);
    /* Do not free the string from gtk_recent_info_get_uri */
#else
    gchar *projectdir = config_file_get_string("Settings", "LastProject");
    g_return_if_fail(projectdir);
#endif /* SUCKY_GNOME */
    
    /* Hide the welcome dialog when opening the new story */
    gtk_widget_hide(gtk_widget_get_toplevel(GTK_WIDGET(button)));
    
    Story *thestory;
    thestory = open_project(projectdir);
    g_free(projectdir);
    
    if(thestory == NULL) {
        /* Take us back to the welcome dialog */
        gtk_widget_show(gtk_widget_get_toplevel(GTK_WIDGET(button)));
        return;
    }
    gtk_widget_show(thestory->window);
    gtk_widget_destroy(gtk_widget_get_toplevel(GTK_WIDGET(button)));
}
