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
#include <libgnomevfs/gnome-vfs.h>

#include "interface.h"
#include "support.h"

#include "extension.h"
#include "tabsource.h"
#include "windowlist.h"

static GSList *extlist = NULL;

/* Create a new representation of an extension file and add it to the list */
Extension *new_ext() {
    Extension *newext = g_new0(Extension, 1);

    newext->filename = NULL;
    newext->monitor = NULL;
    newext->window = create_ext_window();
    newext->buffer = create_natural_inform_source_buffer(TRUE);

    /* Connect the source buffer to our sourceview */
    gtk_text_view_set_buffer(
      GTK_TEXT_VIEW(lookup_widget(newext->window, "ext_code")),
      GTK_TEXT_BUFFER(newext->buffer));

    extlist = g_slist_append(extlist, (gpointer)newext);
    update_window_list();
    
    return newext;
}

/* Remove an extension from the list and free it */
void delete_ext(Extension *oldext) {
    extlist = g_slist_remove(extlist, (gconstpointer)oldext);

    if(oldext->filename != NULL)
        free(oldext->filename);
    if(oldext->monitor)
        gnome_vfs_monitor_cancel(oldext->monitor);
    gtk_widget_destroy(oldext->window);
    g_free(oldext);
    
    update_window_list();
}

/* Get the extension whose topwindow is the ancestor of widget */
Extension *get_ext(GtkWidget *widget) {
    GSList *iter = extlist;
    /* the following is because the menu items do not share the same toplevel
     as the other widgets? */
    GtkWidget *topwindow = lookup_widget(widget, "ext_window");

    while(iter != NULL) {
        if(((Extension *)iter->data)->window == topwindow)
            return (Extension *)iter->data;
        iter = g_slist_next(iter);
    }
    return NULL;
}

/* Set the filename for this representation */
void set_ext_filename(Extension *ext, gchar *filename) {
    if(ext->filename)
        g_free(ext->filename);
    ext->filename = g_strdup(filename);
    gchar *title = g_path_get_basename(filename);
    gtk_window_set_title(GTK_WINDOW(ext->window), title);
    g_free(title);
    update_window_list();
}

/* Carry out func on each topwindow */
void for_each_extension_window(void (*func)(GtkWidget *)) {
    GSList *iter;
    for(iter = extlist; iter != NULL; iter = g_slist_next(iter))
        func(((Extension *)iter->data)->window);
}

/* Carry out func on each topwindow in idle time */
void for_each_extension_window_idle(GSourceFunc func) {
    GSList *iter;
    for(iter = extlist; iter != NULL; iter = g_slist_next(iter))
        g_idle_add(func,(gpointer)(((Extension *)iter->data)->window));
}

/* Carry out func on each text buffer */
void for_each_extension_buffer(void (*func)(GtkSourceBuffer *)) {
    GSList *iter;
    for(iter = extlist; iter != NULL; iter = g_slist_next(iter))
        func(((Extension *)iter->data)->buffer);
}


/* Check whether extension 'filename' is open and return a pointer to its
extension structure, otherwise return NULL */
Extension *get_extension_if_open(gchar *filename) {
    GSList *iter;
    for(iter = extlist; iter != NULL; iter = g_slist_next(iter))
        if(!strcmp(filename, ((Extension *)iter->data)->filename))
            return (Extension *)iter->data;
    return NULL;
}
