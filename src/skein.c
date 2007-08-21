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

#include "skein.h"

/* The skein is now implemented as a simple command history. This is, of course,
not sufficient for a real skein implementation, but this interface can probably
stay the same no matter how the skein is implemented. */

static void free_node(gpointer thenode, gpointer data) {
    g_free(((struct node *)thenode)->command);
}

/* Create a new skein object */
skein create_skein() {
    GList *newskein = NULL;
    struct node *newnode = g_new0(struct node, 1);
    newnode->command = g_strdup("__START");
    newskein = add_node(newskein, &newskein, newnode);
    return (skein)newskein;
}

/* Get a pointer to the start node of the skein */
skein_pointer get_start_pointer(skein theskein) {
    return (skein_pointer)theskein;
}

/* Move the pointer back to the beginning */
/* In this implementation, since there are no branches, it just erases the whole
skein. */
skein reset_skein(skein theskein, skein_pointer *ptr) {
    destroy_skein(theskein);
    theskein = create_skein();
    *ptr = get_start_pointer(theskein);
    return theskein;
}

/* Add a node to the skein as a child of the specified node and move the pointer
to it */
skein add_node(skein theskein, skein_pointer *ptr, struct node *thenode) {
    theskein = g_list_insert_before(theskein, g_list_next(*ptr),
      (gpointer)thenode);
    *ptr = g_list_next(*ptr);
    return theskein;
}

/* Get a singly-linked list of the nodes comprising the path from the start node
to the specified one. The list must be freed with free_node_list afterward. */
GSList *get_nodes_to_here(skein theskein, skein_pointer ptr) {
    GSList *path = NULL;
    skein_pointer iter = ptr;
    struct node *newnode;
    while(iter) {
        newnode = g_new0(struct node, 1);
        newnode->command = g_strdup(((struct node *)(iter->data))->command);
        path = g_slist_prepend(path, (gpointer)newnode);
        iter = g_list_previous(iter);
    }
    return path;
}

/* destructor */
void destroy_skein(skein theskein) {
    g_list_foreach(theskein, (GFunc)free_node, NULL);
}

/* Free a list of nodes */
void free_node_list(GSList *nodes) {
    g_slist_foreach(nodes, (GFunc)free_node, NULL);
    g_slist_free(nodes);
}
