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

#include "windowlist.h"

/* Returns the number of toplevel windows which are GnomeApp instances; that is,
project windows and extension windows. */
int get_num_app_windows() {
    GList *toplevels = gtk_window_list_toplevels();
    GList *iter;
    int num_items = 0;
    
    for(iter = toplevels; iter != NULL; iter = g_list_next(iter))
        if(GNOME_IS_APP((GtkWidget *)(iter->data)))
            num_items++;
    return num_items;
}

/* Closes all windows, asking if we want to save the documents. */
void close_all_windows() {
    GList *toplevels = gtk_window_list_toplevels();
    GList *iter;
    
    for(iter = toplevels; iter != NULL; iter = g_list_next(iter))
        if(GNOME_IS_APP((GtkWidget *)(iter->data)))
            gtk_widget_destroy((GtkWidget *)(iter->data));
    
    g_list_free(toplevels);
}

/* Update the items in the Window menu so that all open appwindows are displayed
in it, and connect the callback on_window_list_activate to each menu item with
a pointer to the window as user_data */
void update_window_list() {
    GList *toplevels = gtk_window_list_toplevels();
    GList *tl_iter;
    
    GSList *windowlist = NULL;
    GSList *iter;
    GnomeUIInfo item = {
        GNOME_APP_UI_ITEM, NULL, N_("Switch to this window"),
        (gpointer)on_window_list_activate, NULL, NULL,
        GNOME_APP_PIXMAP_NONE, NULL, 0, (GdkModifierType)0, NULL
    };
    GnomeUIInfo end_marker = GNOMEUIINFO_END;
    
    for(tl_iter = toplevels; tl_iter != NULL; tl_iter = g_list_next(tl_iter))
        if(GNOME_IS_APP((GtkWidget *)(tl_iter->data)))
            windowlist = g_slist_prepend(windowlist, tl_iter->data);
        
    int num_items = g_slist_length(windowlist);
    GnomeUIInfo *newitems = g_new(GnomeUIInfo, num_items + 1);
    
    int count = 0;
    for(iter = windowlist; iter != NULL; count++, iter = g_slist_next(iter)) {
        item.label = g_strdup_printf("%d. %s", count + 1,
          gtk_window_get_title(GTK_WINDOW((GtkWidget *)(iter->data))));
        item.user_data = (gpointer)(iter->data);
        memcpy(newitems + count, &item, sizeof(GnomeUIInfo));
    }
    memcpy(newitems + count, &end_marker, sizeof(GnomeUIInfo));
    
    for(iter = windowlist; iter != NULL; iter = g_slist_next(iter)) {
        gnome_app_remove_menu_range(
          GNOME_APP((GtkWidget *)(iter->data)),
          "Window/<Separator>", 1, num_items + 1);
          /* +1 in case a story was just removed from the storylist; it
          apparently causes no harm to remove past the end */
        gnome_app_insert_menus(
          GNOME_APP((GtkWidget *)(iter->data)),
          "Window/<Separator>", newitems);
    }
    
    for(count = 0; count < num_items; count++) {
        g_free((gchar *)(newitems[count].label));
    }
    g_free(newitems);
    g_slist_free(windowlist);
    g_list_free(toplevels);
}

void
on_window_list_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    /* Switch to the selected window */
    gtk_window_present_with_time(GTK_WINDOW((GtkWidget *)user_data),
      gdk_event_get_time(NULL));
    /* What is the reason we have to add the time stamp?? */
}
