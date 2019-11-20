/* Copyright (C) 2008, 2009, 2010, 2011 P. F. Chimento
 * This file is part of GNOME Inform 7.
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

#include "config.h"

#include <stdarg.h>

#include <glib.h>
#include <gtk/gtk.h>

#define WARN(msg,err) g_warning("%s: %s: %s", __func__, (msg), (err)->message)
#define WARN_S(msg,str,err) g_warning("%s: (%s) %s: %s", __func__, (str), \
	(msg), (err)->message)
#define ERROR(msg,err) g_error("%s: %s: %s", __func__, (msg), (err)->message)
#define ERROR_S(msg,str,err) g_error("%s: (%s) %s: %s", __func__, (str), \
	(msg), (err)->message)

#define IO_ERROR_DIALOG(parent,file,err,msg) \
	error_dialog_file_operation(parent, file, err, I7_FILE_ERROR_OTHER, \
	"%s (at %s:%d)", msg, __FILE__, __LINE__)

typedef enum {
	I7_FILE_ERROR_OPEN,
	I7_FILE_ERROR_SAVE,
	I7_FILE_ERROR_OTHER
} I7FileErrorWhat;

void error_dialog(GtkWindow *parent, GError *err, const gchar *msg, ...) G_GNUC_PRINTF(3, 4);
void error_dialog_file_operation(GtkWindow *parent, GFile *file, GError *error, I7FileErrorWhat what, const char *msg, ...) G_GNUC_PRINTF(5, 6);

#endif
