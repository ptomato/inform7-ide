/*  Copyright 2007 P.F. Chimento
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

#ifndef _TABSKEIN_H
#define _TABSKEIN_H

#include <gnome.h>
#include <libgnomecanvas/libgnomecanvas.h>

#include "skein.h"
#include "story.h"

gboolean draw_node(GNode *node, Story *thestory);
void skein_layout_and_redraw(Skein *skein, Story *thestory);
void skein_schedule_redraw(Skein *skein, Story *thestory);
gboolean skein_redraw(Story *thestory);
void show_node(Skein *skein, guint why, GNode *node, Story *thestory);
void play_to_node(Skein *skein, GNode *node, Story *thestory);
void on_skein_layout_clicked(GtkToolButton *toolbutton, gpointer user_data);
void on_skein_trim_clicked(GtkToolButton *toolbutton, gpointer user_data);
void on_skein_play_all_clicked(GtkToolButton *toolbutton, gpointer user_data);
void on_skein_spacing_use_defaults_clicked(GtkButton *button, 
                                           gpointer user_data);
void on_skein_spacing_cancel_clicked(GtkButton *button, gpointer user_data);
void on_skein_spacing_ok_clicked(GtkButton *button, gpointer user_data);
void on_skein_trim_cancel_clicked(GtkButton *button, gpointer user_data);
void on_skein_trim_ok_clicked(GtkButton *button, gpointer user_data);

#endif /* _TABSKEIN_H */

