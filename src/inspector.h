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
 
#ifndef INSPECTOR_H
#define INSPECTOR_H

#include <gnome.h>

#include "story.h"

/* Identifier for the reindex_headings() idle function */
#define IDLE_REINDEX_HEADINGS (42)

/* The names of the inspectors */
enum {
    INSPECTOR_FIRST = 0,
    INSPECTOR_NOTES = INSPECTOR_FIRST,
    INSPECTOR_HEADINGS,
    INSPECTOR_SKEIN,
    INSPECTOR_SEARCH_FILES,
    INSPECTOR_LAST = INSPECTOR_SEARCH_FILES
};    

void after_inspector_window_realize(GtkWidget *widget, gpointer data);
void on_search_inspector_text_changed(GtkEditable *editable, gpointer data);
void on_search_inspector_search_clicked(GtkButton *button, gpointer data);
gboolean on_inspector_window_delete(GtkWidget *widget, GdkEvent *event,
                                    gpointer data);
void on_headings_row_activated(GtkTreeView *treeview, GtkTreePath *path,
                               GtkTreeViewColumn *column, gpointer data);
void update_inspectors();
void refresh_inspector(Story *thestory);
void save_inspector_window_position();
gboolean reindex_headings(gpointer data);

#endif /* INSPECTOR_H */
