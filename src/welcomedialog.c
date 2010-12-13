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
 
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include "welcomedialog.h"
#include "app.h"
#include "builder.h"
#include "error.h"
#include "newdialog.h"
#include "story.h"

void
on_welcome_new_button_clicked(GtkButton *button, I7App *app)
{
	GtkWidget *welcomedialog = gtk_widget_get_toplevel(GTK_WIDGET(button));
	GtkWidget *newdialog = create_new_dialog();
	gtk_widget_destroy(welcomedialog);
    gtk_widget_show(newdialog);
}

void
on_welcome_open_button_clicked(GtkButton *button, I7App *app)
{
	GtkWidget *welcomedialog = gtk_widget_get_toplevel(GTK_WIDGET(button));
	gtk_widget_hide(welcomedialog);

	I7Story *story = i7_story_new_from_dialog(app);

    if(!story) {
        /* Take us back to the welcome dialog */
        gtk_widget_show(welcomedialog);
        return;
    }

	gtk_widget_destroy(welcomedialog);
}

void
on_welcome_reopen_button_clicked(GtkButton *button, I7App *app)
{
    GtkWidget *welcomedialog = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkRecentManager *manager = gtk_recent_manager_get_default();
    GList *recent = gtk_recent_manager_get_items(manager);
    GList *iter;
    time_t timestamp, latest = 0;
    GList *lastproject = NULL;
    
    for(iter = recent; iter != NULL; iter = g_list_next(iter)) {
        GtkRecentInfo *info = gtk_recent_info_ref(
          (GtkRecentInfo *)iter->data);
        if(gtk_recent_info_has_application(info, "GNOME Inform 7")
          && gtk_recent_info_get_application_info(info, "GNOME Inform 7", NULL,
          NULL, &timestamp)
          && gtk_recent_info_has_group(info, "inform7_project")
          && (latest == 0 || difftime(timestamp, latest) > 0)) 
        {
            latest = timestamp;
            lastproject = iter;
        }
        gtk_recent_info_unref(info);
    }

	g_assert(lastproject); /* Button not sensitive if no last project */
	gchar *uri = g_strdup(gtk_recent_info_get_uri((GtkRecentInfo *)lastproject->data));
	/* Do not free the string from gtk_recent_info_get_uri */
	
    /* free the list */
    g_list_foreach(recent, (GFunc)gtk_recent_info_unref, NULL);
    g_list_free(recent);

	I7Story *story = i7_story_new_from_uri(app, uri);
	g_free(uri);
	
	if(story)
		gtk_widget_destroy(welcomedialog);
}

GtkWidget *
create_welcome_dialog(void)
{
	I7App *theapp = i7_app_get();
	gchar *filename = i7_app_get_datafile_path(theapp, "ui/welcomedialog.ui");
	GtkBuilder *builder = create_new_builder(filename, theapp);
	g_free(filename);
	GtkWidget *retval = GTK_WIDGET(load_object(builder, "welcomedialog"));
	
	/* Set the background pixmap for this window */
	GtkRcStyle *newstyle = gtk_widget_get_modifier_style(retval);
    filename = i7_app_get_datafile_path_va(theapp, "Documentation", "Welcome Background.png", NULL);
    newstyle->bg_pixmap_name[GTK_STATE_NORMAL] = filename; /* take ownership */
    gtk_widget_modify_style(retval, newstyle);
	
    /* Set the font size to 12 pixels for the widgets in this window */
	PangoFontDescription *font = pango_font_description_new();
	pango_font_description_set_absolute_size(font, 12.0 * PANGO_SCALE);
	gtk_widget_modify_font(GTK_WIDGET(load_object(builder, "welcome_label")), font);
	pango_font_description_free(font);
	
	/* If there is no "last project", make the reopen button inactive */
	GtkRecentManager *manager = gtk_recent_manager_get_default();
	GList *recent = gtk_recent_manager_get_items(manager);
	GList *iter;
	for(iter = recent; iter != NULL; iter = g_list_next(iter)) {
		if(gtk_recent_info_has_application((GtkRecentInfo *)(iter->data), "GNOME Inform 7") 
			&& gtk_recent_info_has_group((GtkRecentInfo *)(iter->data), "inform7_project")) 
		{
			gtk_widget_set_sensitive(GTK_WIDGET(load_object(builder, "welcome_reopen_button")), TRUE);
			break;
		}
	}
	/* free the list */
	g_list_foreach(recent, (GFunc)gtk_recent_info_unref, NULL);
	g_list_free(recent);

	g_object_unref(builder);
	
	return retval;
}
