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
 
#ifndef TAB_SOURCE_H
#define TAB_SOURCE_H

#include <gnome.h>
#include <gtksourceview/gtksourcebuffer.h>

GtkWidget* source_create(gchar *widget_name, gchar *string1, gchar *string2,
                         gint int1, gint int2);
void on_source_headings_show_menu(GtkMenuToolButton *menutoolbutton,
                                  gpointer data);
GtkSourceBuffer *create_natural_inform_source_buffer(gboolean extension);
void paste_code (GtkSourceBuffer *buffer, gchar *code);
void jump_to_line(GtkWidget *widget, guint line);
void renumber_sections(GtkTextBuffer *buffer);
void shift_selection_right(GtkTextBuffer *buffer);
void shift_selection_left(GtkTextBuffer *buffer);
#endif
