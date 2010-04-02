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
 
#ifndef _SPAWN_H
#define _SPAWN_H

#include <gnome.h>

typedef void IOHookFunc(gpointer, gchar *);

GPid run_command(const gchar *wd, gchar **argv, GtkTextBuffer *output);
GPid run_command_hook(const gchar *wd, gchar **argv, GtkTextBuffer *output,
                      IOHookFunc *callback, gpointer data, gboolean get_out, 
                      gboolean get_err);

#endif /* _SPAWN_H */
