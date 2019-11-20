/* Copyright (C) 2006-2009, 2010, 2012 P. F. Chimento
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

#include "config.h"

#include <string.h>

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "error.h"
#include "spawn.h"

#define BUFSIZE 1024

typedef struct {
	GtkTextBuffer *output;
	IOHookFunc *callback;
	gpointer *data;
} IOHookData;

/* The callback for writing data from the IO channel to the buffer */
static gboolean
write_channel_to_buffer(GIOChannel *ioc, GIOCondition cond,
						GtkTextBuffer *buffer)
{
	/* data for us to read? */
	if(cond & (G_IO_IN | G_IO_PRI)) {
		GIOStatus result;
		gchar scratch[BUFSIZE];
		gsize chars_read = 0;

		memset(scratch, 0, BUFSIZE); /* clear the buffer */
		result = g_io_channel_read_chars(ioc, scratch, BUFSIZE, &chars_read,
		  NULL);

		if (chars_read <= 0 || result != G_IO_STATUS_NORMAL)
			return FALSE;

		GtkTextIter iter;
		gtk_text_buffer_get_end_iter(buffer, &iter);
		gtk_text_buffer_insert(buffer, &iter, scratch, chars_read);
	}

	if (cond & (G_IO_ERR | G_IO_HUP | G_IO_NVAL))
		return FALSE;
	return TRUE;
}

/* The callback for writing data from the IO channel to the buffer */
static gboolean
write_channel_hook(GIOChannel *ioc, GIOCondition cond, IOHookData *data)
{
	GtkTextBuffer *textbuffer = data->output;
	IOHookFunc *callback = data->callback;
	gpointer funcdata = data->data;
	/* data for us to read? */
	if(cond & (G_IO_IN | G_IO_PRI)) {
		GIOStatus result;
		gchar scratch[BUFSIZE];
		gsize chars_read = 0;

		memset(scratch, 0, BUFSIZE); /* clear the buffer */
		result = g_io_channel_read_chars(ioc, scratch, BUFSIZE, &chars_read,
		  NULL);

		if (chars_read <= 0 || result != G_IO_STATUS_NORMAL)
			return FALSE;

		GtkTextIter iter;
		gtk_text_buffer_get_end_iter(textbuffer, &iter);
		gtk_text_buffer_insert(textbuffer, &iter, scratch, chars_read);

		(*callback)(funcdata, scratch);
	}

	if (cond & (G_IO_ERR | G_IO_HUP | G_IO_NVAL))
		return FALSE;
	return TRUE;
}

/*
 * The following functions are adapted from Tim-Philipp Mueller's example
 * From http://scentric.net/tmp/spawn-async-with-pipes-gtk.c
 *
 * Copyright 2004 Tim-Philip Mueller and subject to GPLv2
 */

/* Set up an IO channel from a file descriptor to a GtkTextBuffer */
static void
set_up_io_channel(gint fd, GtkTextBuffer *output)
{
	GIOChannel *ioc = g_io_channel_unix_new(fd);
	g_io_channel_set_encoding(ioc, NULL, NULL); /* enc. NULL = binary data? */
	g_io_channel_set_buffered(ioc, FALSE);
	g_io_channel_set_close_on_unref(ioc, TRUE);
	g_io_add_watch_full(ioc, G_PRIORITY_HIGH,
	  G_IO_IN|G_IO_PRI|G_IO_ERR|G_IO_HUP|G_IO_NVAL,
	  (GIOFunc)write_channel_to_buffer, (gpointer)output, NULL);
	g_io_channel_unref(ioc);
}

/* Echo the command invocation to the output buffer */
static void
echo_invocation_to_output(gchar **argv, GtkTextBuffer *output)
{
	gchar *args = g_strjoinv(" ", argv + 1);
	GtkTextIter end;
	gtk_text_buffer_get_end_iter(output, &end);
	gchar *invocation = g_strdup_printf("\n%s \\\n\t%s\n", argv[0], args);
	gtk_text_buffer_insert(output, &end, invocation, -1);
	g_free(args);
	g_free(invocation);
}

/**
 * run_command:
 * @wd_file: a #GFile pointing to the working directory for the command.
 * @argv: an array of strings with the command line arguments.
 * @output: a #GtkTextBuffer in which to place the command's output.
 *
 * Runs a command (in @argv[0]) asynchronously with working directory @wd_file,
 * and pipes the output to @output.
 *
 * Returns: a #GPid for the process.
 */
GPid
run_command(GFile *wd_file, char **argv, GtkTextBuffer *output)
{
	GError *err = NULL;
	GPid child_pid;
	gint stdout_fd, stderr_fd;
	char *wd;

	wd = g_file_get_path(wd_file);

	echo_invocation_to_output(argv, output);

	if (!g_spawn_async_with_pipes(
	  wd,           /* working directory */
	  argv,         /* command and arguments */
	  NULL,         /* do not change environment */
	  (GSpawnFlags) G_SPAWN_SEARCH_PATH   /* look for command in $PATH  */
	  | G_SPAWN_DO_NOT_REAP_CHILD,  /* we'll check the exit status ourself */
	  NULL,         /* child setup function */
	  NULL,         /* child setup func data argument */
	  &child_pid,   /* where to store the child's PID */
	  NULL,         /* default stdin = /dev/null */
	  output? &stdout_fd : NULL,   /* where to put stdout file descriptor */
	  output? &stderr_fd : NULL,   /* where to put stderr file descriptor */
	  &err)) {
		error_dialog(NULL, err, _("Could not spawn process: "));
		g_free(wd);
		return (GPid)0;
	}
	g_free(wd);

	/* Now use GIOChannels to monitor stdout and stderr */
	if(output != NULL) {
		set_up_io_channel(stdout_fd, output);
		set_up_io_channel(stderr_fd, output);
	}

	return child_pid;
}

/* Set up an IO channel from a file descriptor to a GtkTextBuffer */
static void
set_up_io_channel_hook(gint fd, GtkTextBuffer *output, IOHookFunc *callback,
					   gpointer data)
{
	GIOChannel *ioc = g_io_channel_unix_new(fd);
	g_io_channel_set_encoding(ioc, NULL, NULL); /* enc. NULL = binary data? */
	g_io_channel_set_buffered(ioc, FALSE);
	g_io_channel_set_close_on_unref(ioc, TRUE);

	IOHookData *hook_data = g_new(IOHookData, 1);
	hook_data->output = output;
	hook_data->callback = callback;
	hook_data->data = data;

	g_io_add_watch_full(ioc, G_PRIORITY_HIGH,
	  G_IO_IN|G_IO_PRI|G_IO_ERR|G_IO_HUP|G_IO_NVAL,
	  (GIOFunc)write_channel_hook, (gpointer)hook_data, g_free);
	g_io_channel_unref(ioc);
}

/**
 * run_command:
 * @wd_file: a #GFile pointing to the working directory for the command.
 * @argv: an array of strings with the command line arguments.
 * @output: a #GtkTextBuffer in which to place the command's output.
 * @callback: an #IOHookFunc to call with the command's output.
 * @data: arbitrary data to pass to @callback.
 * @get_out: whether to send the process's #stdout to @callback.
 * @get_err: whether to send the process's #stderr to @callback.
 *
 * Runs a command (in @argv[0]) asynchronously with working directory @wd_file,
 * and pipes the output to @output, and also to a hook function @callback.
 *
 * Returns: a #GPid for the process.
 */
GPid
run_command_hook(GFile *wd_file, char **argv, GtkTextBuffer *output,
				 IOHookFunc *callback, gpointer data, gboolean get_out,
				 gboolean get_err)
{
	GError *err = NULL;
	GPid child_pid;
	gint stdout_fd, stderr_fd;
	char *wd;

	wd = g_file_get_path(wd_file);

	echo_invocation_to_output(argv, output);

	if (!g_spawn_async_with_pipes(
	  wd,           /* working directory */
	  argv,         /* command and arguments */
	  NULL,         /* do not change environment */
	  (GSpawnFlags) G_SPAWN_SEARCH_PATH   /* look for command in $PATH  */
	  | G_SPAWN_DO_NOT_REAP_CHILD,  /* we'll check the exit status ourself */
	  NULL,         /* child setup function */
	  NULL,         /* child setup func data argument */
	  &child_pid,   /* where to store the child's PID */
	  NULL,         /* default stdin = /dev/null */
	  output? &stdout_fd : NULL,   /* where to put stdout file descriptor */
	  output? &stderr_fd : NULL,   /* where to put stderr file descriptor */
	  &err)) {
		error_dialog(NULL, err, _("Could not spawn process: "));
		g_free(wd);
		return (GPid)0;
	}
	g_free(wd);

	/* Now use GIOChannels to monitor stdout and stderr */
	if(output != NULL) {
		if(get_out)
			set_up_io_channel_hook(stdout_fd, output, callback, data);
		else
			set_up_io_channel(stdout_fd, output);
		if(get_err)
			set_up_io_channel_hook(stderr_fd, output, callback, data);
		else
			set_up_io_channel(stderr_fd, output);
	}

	return child_pid;
}
