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
 
#ifndef TAB_GAME_H
#define TAB_GAME_H

#include <gnome.h>
#include <sys/types.h>

#include "story.h"

GtkWidget *game_create(gchar *widget_name, gchar *string1, gchar *string2,
                       gint int1, gint int2);
void on_game_viewport_l_size_allocate(GtkWidget *widget,
                                      GtkAllocation *allocation, gpointer data);
void on_game_viewport_r_size_allocate(GtkWidget *widget,
                                      GtkAllocation *allocation, gpointer data);
void run_project(Story *thestory);
void stop_project(Story *thestory);
void resize_game_window(Story *thestory, int right, guint w, guint h);
gboolean game_is_running(Story *thestory);

#endif
