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
 
#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>
#include <glib.h>
#include <gtk/gtk.h>

#define WARN(msg,err) g_warning("%s: %s: %s", __func__, (msg), (err)->message)
#define WARN_S(msg,str,err) g_warning("%s: (%s) %s: %s", __func__, (str), \
	(msg), (err)->message)
#define ERROR(msg,err) g_error("%s: %s: %s", __func__, (msg), (err)->message)
#define ERROR_S(msg,str,err) g_error("%s: (%s) %s: %s", __func__, (str), \
	(msg), (err)->message)

void error_dialog(GtkWindow *parent, GError *err, const gchar *msg, ...);

#endif
