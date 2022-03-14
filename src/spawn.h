/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2006-2010, 2012 Philip Chimento <philip.chimento@gmail.com>
 */

#ifndef _SPAWN_H
#define _SPAWN_H

#include "config.h"

#include <gtk/gtk.h>

typedef void IOHookFunc(gpointer, gchar *);

GPid run_command(GFile *wd_file, char **argv, GtkTextBuffer *output);
GPid run_command_hook(GFile *wd_file, char **argv, GtkTextBuffer *output,
					  IOHookFunc *callback, gpointer data, gboolean get_out,
					  gboolean get_err);

#endif /* _SPAWN_H */
