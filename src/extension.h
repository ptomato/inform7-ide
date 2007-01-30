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
 
#ifndef EXTENSION_H
#define EXTENSION_H

#include <gnome.h>
#include <gtksourceview/gtksourcebuffer.h>
#include <libgnomevfs/gnome-vfs.h>

struct extension {
    /* The toplevel window in which this extension is being edited */
    GtkWidget *window;
    /* This extension's filename */
    gchar *filename;
    /* File monitor */
    GnomeVFSMonitorHandle *monitor;
    /* The program code */
    GtkSourceBuffer *buffer;
};

struct extension *new_ext();
void delete_ext(struct extension *oldext);
struct extension *get_ext(GtkWidget *widget);
void set_ext_filename(struct extension *ext, gchar *filename);
void for_each_extension_window(void (*func)(GtkWidget *));
void for_each_extension_buffer(void (*func)(GtkSourceBuffer *));
struct extension *get_extension_if_open(gchar *filename);

#endif
