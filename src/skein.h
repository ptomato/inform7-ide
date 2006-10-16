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
 
#ifndef SKEIN_H
#define SKEIN_H

#include <gnome.h>

typedef GList * skein;
typedef GList * skein_pointer;

struct node {
    gchar *command;
};

skein create_skein();
skein_pointer get_start_pointer(skein theskein);
skein reset_skein(skein theskein, skein_pointer *ptr);
skein add_node(skein theskein, skein_pointer *ptr, struct node *thenode);
GSList *get_nodes_to_here(skein theskein, skein_pointer ptr);
void destroy_skein(skein theskein);
void free_node_list(GSList *nodes);

#endif
