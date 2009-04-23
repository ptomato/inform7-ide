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

#ifndef __ELASTIC_H__
#define __ELASTIC_H__

#include <gtk/gtk.h>
#include "story.h"

void elastic_remove(GtkSourceBuffer *textbuffer);
void elastic_refresh(GtkTextBuffer *textbuffer, GtkTextView *view);
gboolean elastic_insert_text(GtkTextBuffer *textbuffer, GtkTextIter *location,
							 gchar *text, gint len, GtkTextView *view);
gboolean elastic_delete_range(GtkTextBuffer *textbuffer, GtkTextIter *start, 
							  GtkTextIter *end, GtkTextView *view);
void elastic_setup(GtkTextBuffer *textbuffer, GtkTextView *view);

#endif /* __ELASTIC_H__ */

