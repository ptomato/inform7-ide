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
 
#ifndef _WIDGETS_H
#define _WIDGETS_H

#include <gnome.h>

GtkWidget *custom_gtk_source_view_create(gchar *widget_name, gchar *string1,
                                         gchar *string2, gint int1, gint int2);
GtkWidget *source_example_create(gchar *widget_name, gchar *string1,
                                 gchar *string2, gint int1, gint int2);
GtkWidget *tab_example_create(gchar *widget_name, gchar *string1, 
                              gchar *string2, gint int1, gint int2);

#endif /* _WIDGETS_H */
