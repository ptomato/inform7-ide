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

#include <gnome.h>

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

/* Runs a command (in argv[0]) with working directory wd, and pipes the output
to a GtkTextBuffer */
GPid 
run_command(const gchar *wd, gchar **argv, GtkTextBuffer *output)
{
    GError *err = NULL;
    GPid child_pid;
    gint stdout_fd, stderr_fd;
    
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
        return (GPid)0;
    }
    
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

/* Runs a command (in argv[0]) with working directory wd, pipes the output
to a GtkTextBuffer, and also to a hook function */
GPid
run_command_hook(const gchar *wd, gchar **argv, GtkTextBuffer *output, 
                 IOHookFunc *callback, gpointer data, gboolean get_out,
                 gboolean get_err) 
{
    GError *err = NULL;
    GPid child_pid;
    gint stdout_fd, stderr_fd;
    
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
        return (GPid)0;
    }
    
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
