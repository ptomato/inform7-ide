/* Copyright 2007 P. F. Chimento
 * This file is part of GNOME Inform 7.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#include <gnome.h>

#include "interface.h"
#include "support.h"

#include "appmenu.h"
#include "apptoolbar.h"
#include "findreplace.h"
#include "searchwindow.h"

void
on_go_toolbutton_clicked(GtkToolButton *toolbutton, gpointer data)
{
    on_go_activate(GTK_MENU_ITEM(lookup_widget(GTK_WIDGET(toolbutton), "go")), 
      data);
}

void
on_replay_toolbutton_clicked(GtkToolButton *toolbutton, gpointer data)
{
    on_replay_activate(GTK_MENU_ITEM(lookup_widget(GTK_WIDGET(toolbutton),
      "replay")), data);
}

void
on_stop_toolbutton_clicked(GtkToolButton *toolbutton, gpointer data)
{
    on_stop_activate(GTK_MENU_ITEM(lookup_widget(GTK_WIDGET(toolbutton),
      "stop")), data);
}

void
on_release_toolbutton_clicked(GtkToolButton *toolbutton, gpointer data)
{
    on_release_activate(GTK_MENU_ITEM(lookup_widget(GTK_WIDGET(toolbutton),
      "release")), data);
}

void
on_docs_search_activate(GtkEntry *entry, gpointer user_data)
{
    const gchar *search_text = gtk_entry_get_text(entry);
    GList *list = search_doc(search_text, TRUE /*ignore case*/, FIND_CONTAINS);
    GtkWidget *search_window = new_search_window(
      gtk_widget_get_toplevel(GTK_WIDGET(entry)), search_text, list);
    gtk_widget_show(search_window);
}

gboolean
on_docs_search_focus(GtkWidget *widget, GdkEventFocus *event, gpointer data)
{
    gtk_editable_delete_text(GTK_EDITABLE(widget), 0, -1);
    return FALSE;
}

void
on_help_toolbutton_clicked(GtkToolButton *toolbutton, gpointer data)
{
    on_inform_help_activate(GTK_MENU_ITEM(lookup_widget(GTK_WIDGET(toolbutton),
      "inform_help")), data);
}
