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
 
#ifndef STORY_H
#define STORY_H

#include <glib.h>
#include <gtksourceview/gtksourcebuffer.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomecanvas/libgnomecanvas.h>

#include "skein.h"

enum {
    FORMAT_Z5 = 5,
    FORMAT_Z6 = 6,
    FORMAT_Z8 = 8,
    FORMAT_GLULX = 256
};

enum {
    HEADING_TITLE,
    HEADING_LINE,
    NUM_HEADING_COLUMNS
};

/* Compiling actions */
enum {
    COMPILE_NONE,
    COMPILE_REFRESH_INDEX,
    COMPILE_SAVE_DEBUG_BUILD,
    COMPILE_RUN,
	COMPILE_TEST_ME,
	COMPILE_SAVE_IFICTION,
    COMPILE_RELEASE
};

struct history {
    int tab;
    int subtab;
    char *page;
    char *anchor;
};

typedef struct history History;

enum {
    SKEIN_LEFT = 0,
    SKEIN_RIGHT,
    SKEIN_INSPECTOR,
    NUM_SKEINS
};

typedef struct {
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
    /* The tree of section headings */
    GtkTreeStore *headings;
    /* Various settings */
    int story_format;
    gboolean make_blorb;
	gboolean nobble_rng;
    
    /* Which interpreter is running it */
    gulong handler_finished;
    gulong handler_input;
    
    /* Compile actions */
    int action;
	gchar *copyblorbto;
    
    /* Skein */
    Skein *theskein;
    GnomeCanvasGroup *skeingroup[NUM_SKEINS];
    gboolean drawflag[NUM_SKEINS];
    int drawcounter;
    gboolean editingskein;
    gboolean redrawingskein;
    gint old_horizontal_spacing;
    gint old_vertical_spacing;
        
    /* History navigation */
    GQueue *back[2];
    GQueue *forward[2];
    History *current[2];
    gulong handler_notebook_change[2];
    gulong handler_errors_change[2];
    gulong handler_index_change[2];
} Story;

Story *new_story();
void delete_story(Story *oldstory);
Story *get_story(GtkWidget *widget);
void set_story_filename(Story *thestory, gchar *filename);
void for_each_story_window(void (*func)(GtkWidget *));
void for_each_story_window_idle(GSourceFunc func);
void for_each_story_buffer(void (*func)(GtkSourceBuffer *));
void for_each_story(void (*func)(Story *));
const gchar *get_story_extension(Story *thestory);

#endif
