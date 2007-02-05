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
 
#ifndef STORY_H
#define STORY_H

#include <glib.h>
#include <gtksourceview/gtksourcebuffer.h>
#include <libgnomevfs/gnome-vfs.h>

#include "skein.h"

enum {
    FORMAT_Z5 = 5,
    FORMAT_Z8 = 8,
    FORMAT_GLULX = 256
};

struct story {
    /* The toplevel window in which this story is being edited */
    GtkWidget *window;
    /* This story's filename */
    gchar *filename;
    /* File monitor */
    GnomeVFSMonitorHandle *monitor;
    /* The program code */
    GtkSourceBuffer *buffer;
    /* The user's notes */
    GtkTextBuffer *notes;
    /* Various settings */
    int story_format;
    gboolean make_blorb;
    
    /* Which interpreter is running it */
    gboolean interp_running;
    pid_t interp_process;
    gulong handler_child_exit;
    gulong handler_commit;
    
    /* Compile actions */
    gboolean release;
    gboolean run;
    
    /* Skein */
    skein theskein;
    skein_pointer skein_ptr;
};

struct story *new_story();
void delete_story(struct story *oldstory);
struct story *get_story(GtkWidget *widget);
void set_story_filename(struct story *thestory, gchar *filename);
void for_each_story_window(void (*func)(GtkWidget *));
void for_each_story_window_idle(GSourceFunc func);
void for_each_story_buffer(void (*func)(GtkSourceBuffer *));
gchar *get_story_extension(struct story *thestory);

#endif
