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
#include <gtksourceview/gtksourcebuffer.h>
#include <libgnomevfs/gnome-vfs.h>

#include "interface.h"
#include "support.h"

#include "file.h"
#include "skein.h"
#include "story.h"
#include "tabgame.h"
#include "tabskein.h"
#include "tabsource.h"
#include "windowlist.h"

/* This is the list of all story structures that are currently allocated */
static GSList *storylist = NULL;

/* Create and initialize a new story structure, with main window and source
buffer and skein, etc. */
Story *new_story() {
    Story *newstory = g_new0(Story, 1);

    newstory->filename = NULL;
    newstory->monitor = NULL;
    newstory->window = create_app_window();
    newstory->buffer = create_natural_inform_source_buffer();
    newstory->notes = gtk_text_buffer_new(NULL);
    newstory->headings = gtk_tree_store_new(NUM_HEADING_COLUMNS,
      G_TYPE_STRING, /* Heading title */
      G_TYPE_INT /* Line number */);

    /* Connect the source buffer to both of our sourceviews */
    gtk_text_view_set_buffer(
      GTK_TEXT_VIEW(lookup_widget(newstory->window, "source_l")),
      GTK_TEXT_BUFFER(newstory->buffer));
    gtk_text_view_set_buffer(
      GTK_TEXT_VIEW(lookup_widget(newstory->window, "source_r")),
      GTK_TEXT_BUFFER(newstory->buffer));

    /* Do the default settings */
    newstory->story_format = FORMAT_Z5;
    newstory->make_blorb = FALSE;
    newstory->handler_finished = 0;
    newstory->handler_input = 0;
    newstory->action = COMPILE_NONE;
    
    /* Create an empty skein */
    int foo;
    newstory->theskein = skein_new();
#ifdef I_LIKE_SKEIN
    newstory->editingskein = FALSE;
    newstory->redrawingskein = FALSE;
    for(foo = 0; foo < 3; foo++) {
        newstory->skeingroup[foo] = NULL;
        newstory->drawflag[foo] = FALSE;
    }
    g_signal_connect(G_OBJECT(newstory->theskein), "tree-changed",
                     G_CALLBACK(skein_layout_and_redraw), (gpointer)newstory);
    g_signal_connect(G_OBJECT(newstory->theskein), "node-text-changed",
                     G_CALLBACK(skein_layout_and_redraw), (gpointer)newstory);
    g_signal_connect(G_OBJECT(newstory->theskein), "thread-changed",
                     G_CALLBACK(skein_schedule_redraw), (gpointer)newstory);
    g_signal_connect(G_OBJECT(newstory->theskein), "node-color-changed",
                     G_CALLBACK(skein_schedule_redraw), (gpointer)newstory);
    g_signal_connect(G_OBJECT(newstory->theskein), "lock-changed",
                     G_CALLBACK(skein_schedule_redraw), (gpointer)newstory);
    g_signal_connect(G_OBJECT(newstory->theskein), "transcript-thread-changed",
                     G_CALLBACK(skein_schedule_redraw), (gpointer)newstory);
    g_signal_connect(G_OBJECT(newstory->theskein), "show-node",
                     G_CALLBACK(show_node), (gpointer)newstory);
#endif /* I_LIKE_SKEIN */
    
    /* Initialize the navigation history */
    for(foo = 0; foo < 2; foo++) {
        newstory->back[foo] = g_queue_new();
        newstory->forward[foo] = g_queue_new();
        newstory->current[foo] = NULL;
        newstory->handler_notebook_change[foo] =
            g_signal_handler_find(lookup_widget(newstory->window, foo == 0?
                                                "notebook_l" : "notebook_r"),
                                  G_SIGNAL_MATCH_ID,
                                  g_signal_lookup("switch-page",
                                                  GTK_TYPE_NOTEBOOK),
                                  0, NULL, NULL, NULL);
        newstory->handler_errors_change[foo] =
            g_signal_handler_find(lookup_widget(newstory->window, foo == 0?
                                                "errors_notebook_l" :
                                                "errors_notebook_r"),
                                  G_SIGNAL_MATCH_ID,
                                  g_signal_lookup("switch-page",
                                                  GTK_TYPE_NOTEBOOK),
                                  0, NULL, NULL, NULL);
        newstory->handler_index_change[foo] =
            g_signal_handler_find(lookup_widget(newstory->window, foo == 0?
                                                "index_notebook_l" :
                                                "index_notebook_r"),
                                  G_SIGNAL_MATCH_ID,
                                  g_signal_lookup("switch-page",
                                                  GTK_TYPE_NOTEBOOK),
                                  0, NULL, NULL, NULL);
    }
    
    storylist = g_slist_append(storylist, (gpointer)newstory);
    update_window_list();
    
    return newstory;
}

/* Free all the resources from the story */
void delete_story(Story *oldstory) {
    storylist = g_slist_remove(storylist, (gconstpointer)oldstory);

    stop_project(oldstory);
    
    if(oldstory->filename != NULL) {
        delete_build_files(oldstory);
        free(oldstory->filename);
    }
    gtk_widget_destroy(oldstory->window);
    skein_free(oldstory->theskein);
    if(oldstory->monitor)
        gnome_vfs_monitor_cancel(oldstory->monitor);
    
    int side;
    for(side = 0; side < 2; side++) {
        History *foo;
        while((foo = g_queue_pop_head(oldstory->back[side])) != NULL) {
            if(foo->page)
                g_free(foo->page);
            if(foo->anchor)
                g_free(foo->anchor);
            g_free(foo);
        }
        g_queue_free(oldstory->back[side]);
        while((foo = g_queue_pop_head(oldstory->forward[side])) != NULL) {
            if(foo->page)
                g_free(foo->page);
            if(foo->anchor)
                g_free(foo->anchor);
            g_free(foo);
        }
        g_queue_free(oldstory->forward[side]);
        if(oldstory->current[side]->page)
            g_free(oldstory->current[side]->page);
        if(oldstory->current[side]->anchor)
            g_free(oldstory->current[side]->anchor);
        g_free(oldstory->current[side]);
    }
    
    g_free(oldstory);
    
    update_window_list();
}

/* Returns the story struct associated with the main window that is the toplevel
of widget */
Story *get_story(GtkWidget *widget) {
    GSList *iter = storylist;
    /* the following is because the menu items do not share the same toplevel
     as the other widgets? */
    GtkWidget *topwindow = lookup_widget(widget, "app_window");

    while(iter != NULL) {
        if(((Story *)iter->data)->window == topwindow)
            return (Story *)iter->data;
        iter = g_slist_next(iter);
    }
    return NULL;
}

/* Format and set the filename of a story struct */
void set_story_filename(Story *thestory, gchar *filename) {
    if(thestory->filename)
        g_free(thestory->filename);
    thestory->filename = g_strdup(filename);
    gchar *title = g_path_get_basename(filename);
    gtk_window_set_title(GTK_WINDOW(thestory->window), title);
    g_free(title);
    update_window_list();
}

/* Carry out func for each story window */
void for_each_story_window(void (*func)(GtkWidget *)) {
    GSList *iter;
    for(iter = storylist; iter != NULL; iter = g_slist_next(iter))
        func(((Story *)iter->data)->window);
}

/* Carry out func for each story window in idle time */
void for_each_story_window_idle(GSourceFunc func) {
    GSList *iter;
    for(iter = storylist; iter != NULL; iter = g_slist_next(iter))
        g_idle_add(func, (gpointer)(((Story *)iter->data)->window));
}

/* Carry out func for each story buffer */
void for_each_story_buffer(void (*func)(GtkSourceBuffer *)) {
    GSList *iter;
    for(iter = storylist; iter != NULL; iter = g_slist_next(iter))
        func(((Story *)iter->data)->buffer);
}

/* Return the extension of the output file of this story */
gchar *get_story_extension(Story *thestory) {
    switch(thestory->story_format) {
        case FORMAT_Z5:
            return "z5";
        case FORMAT_Z6:
            return "z6";
        case FORMAT_Z8:
            return "z8";
        case FORMAT_GLULX:
            return "ulx";
    }
    return "error";
}
