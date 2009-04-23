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
 
#ifndef FIND_REPLACE_H
#define FIND_REPLACE_H

#include <gnome.h>

#include "extension.h"
#include "story.h"

enum {
    FIND_CONTAINS,
    FIND_STARTS_WITH,
    FIND_FULL_WORD
};

void after_find_dialog_realize(GtkWidget *widget, gpointer data);
void on_find_text_changed(GtkEditable *editable, gpointer data);
void on_find_next_clicked(GtkButton *button, Story *thestory);
void on_xfind_next_clicked(GtkButton *button, Extension *ext);
void on_find_previous_clicked(GtkButton *button, Story *thestory);
void on_xfind_previous_clicked(GtkButton *button, Extension *ext);
void on_find_replace_find_clicked(GtkButton *button, Story *thestory);
void on_xfind_replace_find_clicked(GtkButton *button, Extension *ext);
void on_find_replace_clicked(GtkButton *button, gpointer data);
void on_xfind_replace_clicked(GtkButton *button, gpointer data);
void on_find_replace_all_clicked(GtkButton *button, Story *thestory);
void on_xfind_replace_all_clicked(GtkButton *button, Extension *ext);
  
#endif
