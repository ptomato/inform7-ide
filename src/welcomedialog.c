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

#include "welcomedialog.h"
#include "interface.h"
#include "support.h"
#include "story.h"
#include "file.h"
#include "configfile.h"

void
after_welcome_dialog_realize           (GtkWidget       *widget,
                                        gpointer         user_data)
{
    /* Set the background pixmap for this window */
    GtkRcStyle *newstyle = gtk_widget_get_modifier_style(widget);
    newstyle->bg_pixmap_name[GTK_STATE_NORMAL] =
      gnome_program_locate_file(gnome_program_get(),
      GNOME_FILE_DOMAIN_APP_PIXMAP, "gnome-inform7/welcome-background.png",
      TRUE, NULL);
    gtk_widget_modify_style(widget, newstyle);
    
    /* If there is no "last project", make the reopen button inactive */
    gchar *trash = config_file_get_string( "Settings", "LastProject");
    if(!trash)
        gtk_widget_set_sensitive(lookup_widget(widget, "welcome_reopen_button"),
          FALSE);
    else
        g_free(trash);
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
    struct story *thestory;

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

    gtk_widget_destroy(gtk_widget_get_toplevel(GTK_WIDGET(button)));
    gtk_widget_show(thestory->window);
}

void
on_welcome_reopen_button_clicked       (GtkButton       *button,
                                        gpointer         user_data)
{
    gchar *projectname = config_file_get_string("Settings", "LastProject");
    struct story *thestory;
    thestory = open_project(projectname);
    gtk_widget_show(thestory->window);
    g_free(projectname);
    gtk_widget_destroy(gtk_widget_get_toplevel(GTK_WIDGET(button)));
}
