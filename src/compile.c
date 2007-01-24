/*  Copyright 2006 P.F. Chimento
 *  This file is part of GNOME Inform 7.
 * 
 *  GNOME Inform 7 is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  GNOME Inform 7 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNOME Inform 7; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#include <gnome.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifdef OSSP_UUID
#  include <ossp/uuid.h> /* For Debian-like systems */
#else
#  include <uuid.h>
#endif

#include "support.h"
#include "compile.h"
#include "story.h"
#include "appwindow.h"
#include "html.h"
#include "tabindex.h"
#include "file.h"
#include "error.h"
#include "configfile.h"
#include "tabgame.h"

#define BUFSIZE 1024
#define INFORM_VERSION "6.31" /* see check_external_binaries in configfile.c */
#define PROBLEMS_FILE "Build", "Problems.html"

/* Start the compiler running the census of extensions. If wait is TRUE, it will
not do it in the background. */
void run_census(gboolean wait) {
    /* Build the command line */
    gchar *working_dir = g_build_filename(g_get_home_dir(), ".wine", "drive_c",
      NULL);
    gchar **commandline; 

    commandline = g_new(gchar *, 6);
    commandline[0] = g_strdup("wine");
    commandline[1] = get_datafile_path("ni.exe");
    commandline[2] = g_strdup("-rules");
    commandline[3] = get_datafile_path("extensions");
    commandline[4] = g_strdup("-census");
    commandline[5] = NULL;
    
    if(wait)
        g_spawn_sync(working_dir, commandline, NULL, G_SPAWN_SEARCH_PATH
          | G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL,
          NULL, NULL, NULL, NULL, NULL, NULL);
    else
        g_spawn_async(working_dir, commandline, NULL, G_SPAWN_SEARCH_PATH
          | G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL,
          NULL, NULL, NULL, NULL);
    
    g_strfreev(commandline);
    g_free(working_dir);
}

/* Start the compiling process in the background and set up the callback that
is called when the Natural Inform stage is finished. */
void compile_project(struct story *thestory) {
    GError *err = NULL;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(lookup_widget(thestory->window, "compiler_output_l")));
    
    /* Show the errors tab */
    int right = choose_notebook(thestory->window, TAB_ERRORS);
    gtk_notebook_set_current_page(get_notebook(thestory->window, right),
      TAB_ERRORS);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(lookup_widget(thestory->window,
      right? "errors_notebook_r" : "errors_notebook_l")), TAB_ERRORS_PROGRESS);
    
    /* Clear the previous compile output */
    gtk_text_buffer_set_text(buffer, "", -1);
    html_load_blank(GTK_HTML(lookup_widget(thestory->window, "problems_l")));
    html_load_blank(GTK_HTML(lookup_widget(thestory->window, "problems_r")));
    
    /* Create the UUID file if needed */
    gchar *uuid_file = g_build_filename(thestory->filename, "uuid.txt", NULL);
    if(!g_file_test(uuid_file, G_FILE_TEST_EXISTS)) {
        gchar *uuid_string = NULL; /* a new buffer is allocated if NULL */
        uuid_t *uuid;
        
        if(!((uuid_create(&uuid) == UUID_RC_OK) &&
          (uuid_make(uuid, UUID_MAKE_V1) == UUID_RC_OK) &&
          (uuid_export(uuid, UUID_FMT_STR, (void **)&uuid_string, NULL)
            == UUID_RC_OK) &&
          (uuid_destroy(uuid) == UUID_RC_OK))) {
            error_dialog(GTK_WINDOW(thestory->window), NULL,
              "Error creating UUID.");
            g_free(uuid_file);
            return;
        }
        
        if(!g_file_set_contents(uuid_file, uuid_string, -1, &err)) {
            error_dialog(GTK_WINDOW(thestory->window), err,
              "Error creating UUID file: ");
            free(uuid_string);
            g_free(uuid_file);
            return;
        }
        free(uuid_string);
    }
    g_free(uuid_file);
 
    /* Build the command line */
    gchar *working_dir = g_build_filename(g_get_home_dir(), ".wine", "drive_c",
      NULL);
    gchar **commandline; 
    if(((struct story *)thestory)->release) {
        commandline = g_new(gchar *, 9);
        commandline[7] = g_strdup("-release");
        commandline[8] = NULL;
    } else {
        commandline = g_new(gchar *, 8);
        commandline[7] = NULL;
    }
    commandline[0] = g_strdup("wine");
    commandline[1] = get_datafile_path("ni.exe");
    commandline[2] = g_strdup("-rules");
    commandline[3] = get_datafile_path("extensions");
    commandline[4] = g_strdup("-package");
    commandline[5] = g_strdup(thestory->filename);
    commandline[6] = g_strconcat("-extension=", get_story_extension(thestory),
      NULL);

    display_status_message(thestory->window, "Running Natural Inform...");
    GPid pid = run_command(working_dir, commandline, buffer);
    /* set up a watch for the exit status */
    g_child_watch_add(pid, compile_stage2, (gpointer)thestory);
    
    g_strfreev(commandline);
    g_free(working_dir);
}

/* Display any errors, continue the compiling process by running Inform 6, and
set up the callback for when that exits */
void compile_stage2(GPid pid, gint status, gpointer thestory) {
    /* Get the ni.exe exit code */
    int exit_code = WIFEXITED(status)? WEXITSTATUS(status) : -1;
    
    /* Get the text buffer to put our output in; it's the same buffer for both
    left and right */
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(lookup_widget(((struct story *)thestory)->window,
      "compiler_output_l")));
      
    /* Make the necessary changes to all the tabs */
    gchar *loadfile;
    gchar *trash = NULL;
    switch(exit_code) {
        case 0:
        case 1:
            loadfile = g_build_filename(((struct story *)thestory)->filename,
              PROBLEMS_FILE, NULL);
            break;
        case 2:
            trash = g_build_filename("doc", "sections", "Error2.html", NULL);
            loadfile = get_datafile_path(trash);
            break;
        case 10:
            trash = g_build_filename("doc", "sections", "Error10.html", NULL);
            loadfile = get_datafile_path(trash);
            break;
        case 11:
            trash = g_build_filename("doc", "sections", "Error11.html", NULL);
            loadfile = get_datafile_path(trash);
            break;
        default:
            trash = g_build_filename("doc", "sections", "Error0.html", NULL);
            loadfile = get_datafile_path(trash);
            break;
    }
    g_free(trash);
    
    html_load_file(GTK_HTML(lookup_widget(
      ((struct story *)thestory)->window, "problems_l")),
      loadfile);
    html_load_file(GTK_HTML(lookup_widget(
      ((struct story *)thestory)->window, "problems_r")),
      loadfile);
    g_free(loadfile);
      
    /* Show the debug log and Inform 6 code if necessary */
    if(config_file_get_bool("Debugging", "ShowLog")) {
        gchar *buffer;
        gchar *filename = g_build_filename(((struct story *)thestory)->filename,
          "Build", "Debug log.txt", NULL);
        /* Ignore errors, just don't show it if it's not there */
        if(g_file_get_contents(filename, &buffer, NULL, NULL))
            gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(
              gtk_bin_get_child(GTK_BIN(gtk_notebook_get_nth_page(GTK_NOTEBOOK(
              lookup_widget(((struct story *)thestory)->window,
              "errors_notebook_l")), TAB_ERRORS_DEBUGGING))))), buffer, -1);
        g_free(buffer);
        g_free(filename);
        
        filename = g_build_filename(((struct story *)thestory)->filename,
          "Build", "auto.inf", NULL);
        if(g_file_get_contents(filename, &buffer, NULL, NULL))
            gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(
              gtk_bin_get_child(GTK_BIN(gtk_notebook_get_nth_page(GTK_NOTEBOOK(
              lookup_widget(((struct story *)thestory)->window,
              "errors_notebook_l")), TAB_ERRORS_INFORM6))))), buffer, -1);
        g_free(buffer);
        g_free(filename);
    }
      
    /* Show the problems tab */
    int right = choose_notebook(((struct story *)thestory)->window, TAB_ERRORS);
    gtk_notebook_set_current_page(get_notebook(
      ((struct story *)thestory)->window, right), TAB_ERRORS);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(lookup_widget(
      ((struct story *)thestory)->window,
      right? "errors_notebook_r" : "errors_notebook_l")), TAB_ERRORS_PROBLEMS);
    
    if(exit_code == 0) {
        /* Refresh the index and documentation tabs */
        reload_index_tabs((struct story *)thestory);
        html_refresh(GTK_HTML(lookup_widget(((struct story *)thestory)->window,
          "docs_l")));
        html_refresh(GTK_HTML(lookup_widget(((struct story *)thestory)->window,
          "docs_r")));
    
        /* Now, start Inform6 */
        /* Build the command line */
        gchar *working_dir = g_build_filename(
          ((struct story *)thestory)->filename, "Build", NULL);
        gchar **commandline = g_new(gchar *, 6);
        gchar *libdir = get_datafile_path("lib/Natural");
        commandline[0] = g_strdup("inform-" INFORM_VERSION "-inform7");
        commandline[1] = g_strconcat("-w",
          ((struct story *)thestory)->release? "~S~D" : "SD",
          (((struct story *)thestory)->story_format == FORMAT_GLULX)? "G" :
          ((((struct story *)thestory)->story_format == FORMAT_Z8)? "v8" : "v5"),
          NULL);
        commandline[2] = g_strconcat("+include_path=../Source,", libdir, ",./",
          NULL);
        commandline[3] = g_strdup("auto.inf");
        commandline[4] = g_strconcat("output.",
          get_story_extension((struct story *)thestory), NULL);
        commandline[5] = NULL;
        
        display_status_message(((struct story *)thestory)->window,
          "Running Inform 6...");
        GPid child_pid = run_command(working_dir, commandline, buffer);
        /* set up a watch for the exit status */
        g_child_watch_add(child_pid, compile_stage3, thestory);
        
        g_strfreev(commandline);
        g_free(working_dir);
        g_free(libdir);
    } else
        display_status_message(((struct story *)thestory)->window,
          "Compiling failed.");
}

/* Display any errors from Inform 6, start cBlorb if we are building for
release, or run the story file if we are running. */
void compile_stage3(GPid pid, gint status, gpointer thestory) {
    int exit_code = WIFEXITED(status)? WEXITSTATUS(status) : -1;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(lookup_widget(((struct story *)thestory)->window,
      "compiler_output_l")));
      
    gchar *statusmsg = g_strdup_printf("\nCompiler finished with code %d\n",
      exit_code);
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    gtk_text_buffer_insert(buffer, &iter, statusmsg, -1);
    g_free(statusmsg);
    
    GtkTextIter start, end;
    int line;
    gchar *loadfile = NULL;

    for(line = gtk_text_buffer_get_line_count(buffer) - 1; line >= 0; line--) {
        gchar *msg, *pos;
        gtk_text_buffer_get_iter_at_line(buffer, &start, line);
        end = start;
        gtk_text_iter_forward_to_line_end(&end);
        msg = gtk_text_iter_get_text(&start, &end);
        if((pos = strstr(msg, "rror:"))) { /* "Error:", "Fatal error:" */
            pos += 5; /* skip those five chars */
            g_strchug(pos); /* remove leading whitespace */
            
            char scratch[256];
            gchar *trash;
            
            if(sscanf(pos, "The memory setting %[^)] has been exceeded.",
              scratch) == 1) {
                trash = g_build_filename("doc", "sections", 
                  "ErrorI6MemorySetting.html", NULL);
                loadfile = get_datafile_path(trash);
            } else if(sscanf(pos, "This program has overflowed the maximum "
              "readable-memory size of the %s format.", scratch) == 1) {
                trash = g_build_filename("doc", "sections",
                "ErrorI6Readable.html", NULL);
                loadfile = get_datafile_path(trash);
            } else if(sscanf(pos, "The story file exceeds %s limit",
              scratch) == 1) {
                trash = g_build_filename("doc", "sections",
                  "ErrorI6TooBig.html", NULL);
                loadfile = get_datafile_path(trash);
            } else {
                trash = g_build_filename("doc", "sections", "ErrorI6.html",
                  NULL);
                loadfile = get_datafile_path(trash);
            }
            
            g_free(msg);
            g_free(trash);
            
            break;
        }
        g_free(msg);
    }
    
    if(loadfile) {
        html_load_file(GTK_HTML(lookup_widget(
          ((struct story *)thestory)->window, "problems_l")),
          loadfile);
        html_load_file(GTK_HTML(lookup_widget(
          ((struct story *)thestory)->window, "problems_r")),
          loadfile);
        g_free(loadfile);
          
        /* Show the problems tab */
        int right = choose_notebook(((struct story *)thestory)->window,
          TAB_ERRORS);
        gtk_notebook_set_current_page(get_notebook(
          ((struct story *)thestory)->window, right), TAB_ERRORS);
        gtk_notebook_set_current_page(GTK_NOTEBOOK(lookup_widget(
          ((struct story *)thestory)->window,
          right? "errors_notebook_r" : "errors_notebook_l")),
          TAB_ERRORS_PROBLEMS);
    }
    
    if(exit_code != 0) {
        display_status_message(((struct story *)thestory)->window,
          "Compiling failed.");
        return;
    }
    
    if(((struct story *)thestory)->release) {
        /* Skip this if we are not making a Blorb file */
        if(!(((struct story *)thestory)->make_blorb)) {
            finish_release((struct story *)thestory, exit_code == 0);
            return;
        }
        
        /* first we need to edit the blurb file, because there are backward
        slashes in the directory names. This can be removed when we have a
        native version of the compiler. */
        gchar *blurbfile = g_build_filename(
          ((struct story *)thestory)->filename, "Release.blurb", NULL);
        GError *err = NULL;
        gchar *blurbtext;
        if(!g_file_get_contents(blurbfile, &blurbtext, NULL, &err)) {
            error_dialog(GTK_WINDOW(((struct story *)thestory)->window), err,
              "Cannot open Release.blurb file: ");
            g_free(blurbfile);
            return;
        }
        /* Replace all backslashes with forward slashes. This causes a bug: now
        cblorb will not work with files with backslashes in their names. */
        gchar *pos;
        while((pos = strchr(blurbtext, '\\')))
            *pos = '/';
        /* Write the contents back to the file */
        if(!g_file_set_contents(blurbfile, blurbtext, -1, &err)) {
            error_dialog(GTK_WINDOW(((struct story *)thestory)->window), err,
              "Cannot write to Release.blurb file: ");
            g_free(blurbfile);
            g_free(blurbtext);
            return;
        }
        g_free(blurbfile);
        g_free(blurbtext);
        
        /* Now run cblorb */
        gchar *working_dir = g_strdup(((struct story *)thestory)->filename);
        gchar **commandline = g_new(gchar *, 4);
        commandline[0] = g_strdup("cblorb");
        commandline[1] = g_strdup("Release.blurb");
        commandline[2] = g_build_filename("Build", "output.zblorb", NULL);
        commandline[3] = NULL;
        
        display_status_message(((struct story *)thestory)->window,
          "Running cBlorb...");
        GPid child_pid = run_command(working_dir, commandline, buffer);
        /* set up a watch for the exit status */
        g_child_watch_add(child_pid, compile_stage4, thestory);
        
        g_strfreev(commandline);
        g_free(working_dir);
    } else
        display_status_message(((struct story *)thestory)->window,
          "Compiling succeeded.");
        
    if(((struct story *)thestory)->run) {
        /* Run the project if we need to */
        run_project((struct story *)thestory);
    } else {
        /* Display the index */
        gtk_notebook_set_current_page(
          get_notebook(((struct story *)thestory)->window,
          choose_notebook(((struct story *)thestory)->window, TAB_INDEX)),
          TAB_INDEX);
    }
}

/* Display any errors from cBlorb and then pass it along to finish_release */
void compile_stage4(GPid pid, gint status, gpointer thestory) {
    int exit_code = WIFEXITED(status)? WEXITSTATUS(status) : -1;
    
    gchar *trash;
    trash = get_datafile_path(g_build_filename("doc", "sections",
      (exit_code == 0)? "GoodCblorb.html" : "ErrorCblorb.html", NULL));
    html_load_file(GTK_HTML(lookup_widget(
      ((struct story *)thestory)->window, "problems_l")), trash);
    html_load_file(GTK_HTML(lookup_widget(
      ((struct story *)thestory)->window, "problems_r")), trash);
      
    /* Show the problems tab */
    int right = choose_notebook(((struct story *)thestory)->window,
      TAB_ERRORS);
    gtk_notebook_set_current_page(get_notebook(
      ((struct story *)thestory)->window, right), TAB_ERRORS);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(lookup_widget(
      ((struct story *)thestory)->window,
      right? "errors_notebook_r" : "errors_notebook_l")),
      TAB_ERRORS_PROBLEMS);

    display_status_message(((struct story *)thestory)->window,
      (exit_code == 0)? "Compiling succeeded." : "Compiling failed.");
      
    /* Do the rest of the releasing, and pass along the error code. We don't
    bother checking for the 'run' flag as it's not an option in the IDE to
    build for release and run at the same time. */
    finish_release((struct story *)thestory, exit_code == 0);
}

/*
 * The following three functions are thanks to Tim-Philipp Mueller's example
 * From http://scentric.net/tmp/spawn-async-with-pipes-gtk.c 
 */

/* Runs a command (in argv[0]) with working directory wd, and pipes the output
to a GtkTextBuffer */
GPid run_command(const gchar *wd, gchar **argv, GtkTextBuffer *output) {
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
        error_dialog(NULL, err, "Could not spawn process: ");
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
void set_up_io_channel(gint fd, GtkTextBuffer *output) {
    GIOChannel *ioc = g_io_channel_unix_new(fd);
    g_io_channel_set_encoding(ioc, NULL, NULL); /* enc. NULL = binary data? */
    g_io_channel_set_buffered(ioc, FALSE);
    g_io_channel_set_close_on_unref(ioc, TRUE);
    g_io_add_watch(ioc, G_IO_IN|G_IO_PRI|G_IO_ERR|G_IO_HUP|G_IO_NVAL, 
      write_channel_to_buffer, (gpointer)output);
	g_io_channel_unref (ioc);
}

/* The callback for writing data from the IO channel to the buffer */
gboolean write_channel_to_buffer(GIOChannel *ioc, GIOCondition cond,
gpointer buffer) {
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
        gtk_text_buffer_get_end_iter((GtkTextBuffer *)buffer, &iter);
        gtk_text_buffer_insert((GtkTextBuffer *)buffer, &iter, scratch,
          chars_read);
    }
    
    if (cond & (G_IO_ERR | G_IO_HUP | G_IO_NVAL))
        return FALSE;
    return TRUE;
}
