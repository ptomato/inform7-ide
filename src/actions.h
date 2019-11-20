/*  Copyright (C) 2008, 2009, 2010 P. F. Chimento
 *  This file is part of GNOME Inform 7.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _ACTIONS_H_
#define _ACTIONS_H_

#include <glib.h>
#include <gtk/gtk.h>

void on_open_extension_readonly_activate(GtkMenuItem *menuitem, GFile *file);
void on_open_extension_activate(GtkMenuItem *menuitem, GFile *file);

#endif /* _ACTIONS_H_ */
