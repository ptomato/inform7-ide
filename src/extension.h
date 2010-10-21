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
 
#ifndef EXTENSION_H
#define EXTENSION_H

#include <gnome.h>
#include <gtksourceview/gtksourcebuffer.h>
#include <libgnomevfs/gnome-vfs.h>

typedef struct {
    /* The toplevel window in which this extension is being edited */
    GtkWidget *window;
    /* This extension's filename */
    gchar *filename;
    /* File monitor */
    GnomeVFSMonitorHandle *monitor;
    /* The program code */
    GtkSourceBuffer *buffer;
} Extension;

Extension *new_ext();
void delete_ext(Extension *oldext);
Extension *get_ext(GtkWidget *widget);
void set_ext_filename(Extension *ext, gchar *filename);
void for_each_extension_window(void (*func)(GtkWidget *));
void for_each_extension_window_idle(GSourceFunc func);
void for_each_extension_buffer(void (*func)(GtkSourceBuffer *));
void for_each_extension(void (*func)(Extension *));
Extension *get_extension_if_open(gchar *filename);

#endif
