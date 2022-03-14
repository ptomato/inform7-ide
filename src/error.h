/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2008-2011 Philip Chimento <philip.chimento@gmail.com>
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
