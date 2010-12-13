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

#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <webkit/webkit.h>

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef E2FS_UUID
#  include <uuid/uuid.h> /* Use e2fsprogs uuid */
#else
#  ifdef HAVE_OSSP_UUID_H
#    include <ossp/uuid.h> /* Otherwise, it is OSSP uuid */
#  else
#    include <uuid.h> /* May be in uuid.h */
#  endif
#endif

#include "story.h"
#include "story-private.h"
#include "configfile.h"
#include "error.h"
#include "html.h"
#include "spawn.h"

#define PROBLEMS_FILE "Build", "Problems.html"

typedef struct _CompilerData {
	I7Story *story;
	gboolean build_for_release;
	gboolean refresh_only;
	gchar *input_file;
	gchar *output_file;
	gchar *directory;
} CompilerData;

/* Declare these functions static so they can stay in this order */
static void prepare_ni_compiler(CompilerData *data);
static void start_ni_compiler(CompilerData *data);
static void finish_ni_compiler(GPid pid, gint status, CompilerData *data);
static void prepare_i6_compiler(CompilerData *data);
static void start_i6_compiler(CompilerData *data);
static void finish_i6_compiler(GPid pid, gint status, CompilerData *data);
static void prepare_cblorb_compiler(CompilerData *data);
static void start_cblorb_compiler(CompilerData *data);
static void finish_cblorb_compiler(GPid pid, gint status, CompilerData *data);
static void finish_compiling(gboolean success, CompilerData *data);

/* Set the function that will be called when compiling has finished. */
void 
i7_story_set_compile_finished_action(I7Story *story, CompileActionFunc callback, gpointer data)
{
	I7_STORY_USE_PRIVATE(story, priv);
	priv->compile_finished_callback = callback;
	priv->compile_finished_callback_data = data;
}

/* Start the compiling process. Called from the main thread. */
void
i7_story_compile(I7Story *story, gboolean release, gboolean refresh)
{
	I7_STORY_USE_PRIVATE(story, priv);
	
	i7_document_save(I7_DOCUMENT(story));
	i7_story_stop_running_game(story);
	g_free(priv->copyblorbto);
	priv->copyblorbto = NULL;
	g_free(priv->compiler_output);
	priv->compiler_output = NULL;
	
	/* Set up the compiler */
	CompilerData *data = g_slice_new0(CompilerData);
	data->story = story;
	data->build_for_release = i7_story_get_create_blorb(story) && release;
	data->refresh_only = refresh;

	gchar *filename;
	if(data->build_for_release) {
		if(i7_story_get_story_format(story) == I7_STORY_FORMAT_GLULX)
			filename = g_strdup("output.gblorb");
		else
			filename = g_strdup("output.zblorb");
	} else {
		filename = g_strconcat("output.", i7_story_get_extension(story), NULL);
	}
	data->input_file = i7_document_get_path(I7_DOCUMENT(story));
	data->output_file = g_build_filename(data->input_file, "Build", filename, NULL);
	g_free(filename);
	data->directory = g_build_filename(data->input_file, "Build", NULL);

	prepare_ni_compiler(data);
	start_ni_compiler(data);
}


/* Set everything up for using the NI compiler. Called from the main thread. */
static void 
prepare_ni_compiler(CompilerData *data) 
{
	I7_STORY_USE_PRIVATE(data->story, priv);
    GError *err = NULL;
    
    /* Clear the previous compile output */
    gtk_text_buffer_set_text(priv->progress, "", -1);
    html_load_blank(WEBKIT_WEB_VIEW(data->story->panel[LEFT]->errors_tabs[I7_ERRORS_TAB_PROBLEMS]));
    html_load_blank(WEBKIT_WEB_VIEW(data->story->panel[RIGHT]->errors_tabs[I7_ERRORS_TAB_PROBLEMS]));
    
    /* Create the UUID file if needed */
    gchar *uuid_file = g_build_filename(data->input_file, "uuid.txt", NULL);
    if(!g_file_test(uuid_file, G_FILE_TEST_EXISTS)) {
#ifdef E2FS_UUID /* code for e2fsprogs uuid */
        uuid_t uuid;
        gchar uuid_string[37];
        
        uuid_generate_time(uuid);
        uuid_unparse(uuid, uuid_string);
#else /* code for OSSP UUID */
		gchar *uuid_string = NULL; /* a new buffer is allocated if NULL */
        uuid_t *uuid;
        
        if(!((uuid_create(&uuid) == UUID_RC_OK) 
		    && (uuid_make(uuid, UUID_MAKE_V1) == UUID_RC_OK) 
		    && (uuid_export(uuid, UUID_FMT_STR, (void **)&uuid_string, NULL) == UUID_RC_OK) 
		    && (uuid_destroy(uuid) == UUID_RC_OK))) {
            error_dialog(GTK_WINDOW(data->story), NULL, _("Error creating UUID."));
            g_free(uuid_file);
            return;
        }
#endif /* UUID conditional */
        if(!g_file_set_contents(uuid_file, uuid_string, -1, &err)) {
            error_dialog(GTK_WINDOW(data->story), err, _("Error creating UUID file: "));
            g_free(uuid_file);
            return;
        }
#ifndef E2FS_UUID /* Only OSSP UUID */
        free(uuid_string);
#endif /* !OSSP_UUID */
    }
    g_free(uuid_file);
    
    /* Display status message */
	i7_document_display_status_message(I7_DOCUMENT(data->story), _("Compiling Inform 7 to Inform 6"), COMPILE_OPERATIONS);
}

/* Display the NI compiler's status in the app status bar. This function is
 called from a child process watch, so the GDK lock is not held and must be
 acquired for any GUI calls. */
static void 
display_ni_status(I7Document *document, gchar *text) 
{
    gint percent;
    gchar *message;
    
    if(sscanf(text, " ++ %d%% (%a[^)]", &percent, &message) == 2) {
		gdk_threads_enter();
		i7_document_display_progress_percentage(document, percent / 100.0);
        gdk_threads_leave();
        free(message);
    }
}

/* Start the NI compiler and set up the callback for when it is finished. Called
 from the main thread.*/
static void 
start_ni_compiler(CompilerData *data) 
{
	I7_STORY_USE_PRIVATE(data->story, priv);
	
    /* Build the command line */
	GSList *args = NULL;
	I7App *theapp = i7_app_get();
    args = g_slist_prepend(args, i7_app_get_binary_path(theapp, "ni"));
	args = g_slist_prepend(args, g_strdup("-rules"));
	args = g_slist_prepend(args, i7_app_get_datafile_path(theapp, "Extensions"));
	args = g_slist_prepend(args, g_strconcat("-extension=", i7_story_get_extension(data->story), NULL));
	args = g_slist_prepend(args, g_strdup("-package"));
	args = g_slist_prepend(args, g_strdup(data->input_file));
    if(data->build_for_release)
		args = g_slist_prepend(args, g_strdup("-release"));
	if(config_file_get_bool(PREFS_DEBUG_LOG_VISIBLE))
		args = g_slist_prepend(args, g_strdup("-log"));
	if(i7_story_get_nobble_rng(data->story))
		args = g_slist_prepend(args, g_strdup("-rng"));
	args = g_slist_reverse(args);
	gchar **commandline = g_new0(gchar *, g_slist_length(args) + 1);
	GSList *iter;
	gchar **arg;
	for(iter = args, arg = commandline; iter; iter = g_slist_next(iter))
		*arg++ = iter->data;
	*arg = NULL;
	g_slist_free(args);
	
    /* Run the command and pipe its output to the text buffer. Also pipe stderr
    through a function that analyzes the progress messages and puts them in the
    progress bar. */
    GPid pid = run_command_hook(data->directory, commandline, priv->progress,
                                (IOHookFunc *)display_ni_status, data->story, 
								FALSE, TRUE);
    /* set up a watch for the exit status */
    g_child_watch_add(pid, (GChildWatchFunc)finish_ni_compiler, data);
    
    g_strfreev(commandline);
}

/* Display any errors from the NI compiler and continue on. This function is
 called from a child process watch, so the GDK lock is not held and must be
 acquired for any GUI calls. */
static void 
finish_ni_compiler(GPid pid, gint status, CompilerData *data) 
{
	I7_STORY_USE_PRIVATE(data->story, priv);
	
    /* Clear the progress indicator */
	gdk_threads_enter();
	i7_document_remove_status_message(I7_DOCUMENT(data->story), COMPILE_OPERATIONS);
    i7_document_clear_progress(I7_DOCUMENT(data->story));
	gdk_threads_leave();
        
    /* Get the ni.exe exit code */
    int exit_code = WIFEXITED(status)? WEXITSTATUS(status) : -1;
      
    /* Display the appropriate HTML error or success page */
    gchar *problems_url;
	if(exit_code <= 1) {
		/* In the case of success or a "normal" failure, or a negative error
		 code should one occur, display the compiler's generated Problems.html*/
		problems_url = g_build_filename(data->input_file, PROBLEMS_FILE, NULL);
	} else {
		I7App *theapp = i7_app_get();
		gchar *file = g_strdup_printf("Error%i.html", exit_code);
		if(i7_app_check_datafile_va(theapp, "Documentation", "Sections", file, NULL))
			problems_url = i7_app_get_datafile_path_va(theapp, "Documentation", "Sections", file, NULL);
		else
			problems_url = i7_app_get_datafile_path_va(theapp, "Documentation", "Sections", "Error0.html", NULL);
		g_free(file);
	}

    html_load_file(WEBKIT_WEB_VIEW(data->story->panel[LEFT]->errors_tabs[I7_ERRORS_TAB_PROBLEMS]), problems_url);
	html_load_file(WEBKIT_WEB_VIEW(data->story->panel[RIGHT]->errors_tabs[I7_ERRORS_TAB_PROBLEMS]), problems_url);
    g_free(problems_url);
    
    if(config_file_get_bool(PREFS_DEBUG_LOG_VISIBLE)) {
        /* Update */
		gdk_threads_enter();
        while(gtk_events_pending())
            gtk_main_iteration();
		gdk_threads_leave();
        
        /* Refresh the debug log */
        gchar *text;
        gchar *filename = g_build_filename(data->input_file, "Build", "Debug log.txt", NULL);
        /* Ignore errors, just don't show it if it's not there */
        if(g_file_get_contents(filename, &text, NULL, NULL)) {
			gdk_threads_enter();
            gtk_text_buffer_set_text(priv->debug_log, text, -1);
			gdk_threads_leave();
		}
        g_free(text);
        g_free(filename);
        
        /* Refresh the I6 code */
        filename = g_build_filename(data->input_file, "Build", "auto.inf", NULL);
        if(g_file_get_contents(filename, &text, NULL, NULL)) {
			gdk_threads_enter();
            gtk_text_buffer_set_text(GTK_TEXT_BUFFER(priv->i6_source), text, -1);
			gdk_threads_leave();
		}
        g_free(text);
        g_free(filename);
    }
      
    /* Stop here and show the Errors/Problems tab if there was an error */
    if(exit_code != 0) {
		finish_compiling(FALSE, data);
        return;
    }

	/* Read in the Blorb manifest */
	gchar *path = i7_document_get_path(I7_DOCUMENT(data->story));
	gchar *manifest_filename = g_build_filename(path, "manifest.plist", NULL);
	g_free(path);
	PlistObject *manifest = plist_read(manifest_filename, NULL);
	g_free(manifest_filename);
	/* If that failed, then silently keep the old manifest */
	if(manifest) {
		plist_object_free(priv->manifest);
		priv->manifest = manifest;
	}
	
    /* Decide what to do next */
	if(data->refresh_only) {
		I7Story *story = data->story;
		finish_compiling(TRUE, data);
		/* Hold the GDK lock for the callback */
		gdk_threads_enter();
		(priv->compile_finished_callback)(story, priv->compile_finished_callback_data);
		gdk_threads_leave();
		return;
	}

    prepare_i6_compiler(data);
    start_i6_compiler(data);
}
    

/* Get ready to run the I6 compiler; right now this does almost nothing. This 
 function is called from a child process watch, so the GDK lock is not held and 
 must be acquired for any GUI calls. */
static void 
prepare_i6_compiler(CompilerData *data) 
{
	gdk_threads_enter();
    i7_document_display_status_message(I7_DOCUMENT(data->story), _("Running Inform 6..."), COMPILE_OPERATIONS);
	gdk_threads_leave();
}

/* Determine i6 compiler switches, given the compiler action and the virtual
machine format. Return string must be freed. */
static gchar *
get_i6_compiler_switches(gboolean release, int format) 
{
    gchar *debug_switches, *version_switches, *retval;
    
    /* Switch off strict warnings and debug if the game is for release */
    if(release)
        debug_switches = g_strdup("~S~D");
    else
        debug_switches = g_strdup("kSD");
    /* Pick the appropriate virtual machine version */
    switch(format) {
    case I7_STORY_FORMAT_GLULX:
        version_switches = g_strdup("G");
        break;
    case I7_STORY_FORMAT_Z5:
        version_switches = g_strdup("v5");
        break;
    case I7_STORY_FORMAT_Z6:
        version_switches = g_strdup("v6");
        break;
    case I7_STORY_FORMAT_Z8:
    default:
        version_switches = g_strdup("v8");
    }
    
    retval = g_strconcat("-wxE2", debug_switches, version_switches, NULL);
    g_free(debug_switches);
    g_free(version_switches);
    return retval;
}

/* Pulse the progress bar every time the I6 compiler outputs a '#' (which
 happens whenever it has processed 100 source lines.) This function is
 called from a child process watch, so the GDK lock is not held and must be
 acquired for any GUI calls. */
static void 
display_i6_status(I7Document *document, gchar *text) 
{
    if(strchr(text, '#')) {
		gdk_threads_enter();
        i7_document_display_progress_busy(document);
		gdk_threads_leave();
	}
}

/* Run the I6 compiler. This function is called from a child process watch, so 
 the GDK lock is not held and must be acquired for any GUI calls. */
static void 
start_i6_compiler(CompilerData *data) 
{   
	I7_STORY_USE_PRIVATE(data->story, priv);
	
    /* Build the command line */
    gchar **commandline = g_new(gchar *, 6);
    commandline[0] = i7_app_get_binary_path(i7_app_get(), "inform-6.31-biplatform");
    commandline[1] = get_i6_compiler_switches(data->build_for_release, i7_story_get_story_format(data->story));
    commandline[2] = g_strdup("$huge");
    commandline[3] = g_strdup("auto.inf");
	gchar *i6out = g_strconcat("output.", i7_story_get_extension(data->story), NULL);
    commandline[4] = g_build_filename(data->input_file, "Build", i6out, NULL);
	g_free(i6out);
    commandline[5] = NULL;

    GPid child_pid = run_command_hook(data->directory, commandline, 
	    priv->progress, (IOHookFunc *)display_i6_status, data->story, TRUE, TRUE);
    /* set up a watch for the exit status */
    g_child_watch_add(child_pid, (GChildWatchFunc)finish_i6_compiler, data);
    
    g_strfreev(commandline);
}

/* Display any errors from Inform 6 and decide what to do next. This function is
 called from a child process watch, so the GDK lock is not held and must be
 acquired for any GUI calls. */
static void 
finish_i6_compiler(GPid pid, gint status, CompilerData *data) 
{
    I7_STORY_USE_PRIVATE(data->story, priv);
	
    /* Clear the progress indicator */
	gdk_threads_enter();
	i7_document_remove_status_message(I7_DOCUMENT(data->story), COMPILE_OPERATIONS);
    i7_document_clear_progress(I7_DOCUMENT(data->story));
	gdk_threads_leave();
    
    /* Get exit code from I6 process */
    int exit_code = WIFEXITED(status)? WEXITSTATUS(status) : -1;
    
    /* Display the exit status of the I6 compiler in the Progress tab */
    gchar *statusmsg = g_strdup_printf(_("\nCompiler finished with code %d\n"),
      exit_code);
    GtkTextIter iter;
	gdk_threads_enter();
    gtk_text_buffer_get_end_iter(priv->progress, &iter);
    gtk_text_buffer_insert(priv->progress, &iter, statusmsg, -1);
	gdk_threads_leave();
    g_free(statusmsg);
    
    GtkTextIter start, end;
    int line;
    gchar *loadfile = NULL;

    /* Display the appropriate HTML error pages */
	gdk_threads_enter();
    for(line = gtk_text_buffer_get_line_count(priv->progress); line >= 0; line--) {
        gchar *msg;
        gtk_text_buffer_get_iter_at_line(priv->progress, &start, line);
        end = start;
        gtk_text_iter_forward_to_line_end(&end);
        msg = gtk_text_iter_get_text(&start, &end);
        if(strstr(msg, "rror:")) { /* "Error:", "Fatal error:" */
            if(strstr(msg, "The memory setting ") && strstr(msg, " has been exceeded."))
                loadfile = i7_app_get_datafile_path_va(i7_app_get(), 
				    "Documentation", "Sections", "ErrorI6MemorySetting.html", NULL);
            else if(strstr(msg, "This program has overflowed the maximum readable-memory size of the "))
                loadfile = i7_app_get_datafile_path_va(i7_app_get(),
				    "Documentation", "Sections", "ErrorI6Readable.html", NULL);
            else if(strstr(msg, "The story file exceeds "))
                loadfile = i7_app_get_datafile_path_va(i7_app_get(),
				    "Documentation", "Sections", "ErrorI6TooBig.html", NULL);
            else
                loadfile = i7_app_get_datafile_path_va(i7_app_get(),
				    "Documentation", "Sections", "ErrorI6.html", NULL);
            g_free(msg);
            break;
        }
        g_free(msg);
    }
	gdk_threads_leave();
    if(!loadfile && exit_code != 0)
        loadfile = i7_app_get_datafile_path_va(i7_app_get(), 
		    "Documentation", "Sections", "ErrorI6.html", NULL);
    if(loadfile) {
		gdk_threads_enter();
        html_load_file(WEBKIT_WEB_VIEW(data->story->panel[LEFT]->errors_tabs[I7_ERRORS_TAB_PROBLEMS]), loadfile);
        html_load_file(WEBKIT_WEB_VIEW(data->story->panel[RIGHT]->errors_tabs[I7_ERRORS_TAB_PROBLEMS]), loadfile);
		gdk_threads_leave();
        g_free(loadfile);
    }
    
    /* Stop here and show the Errors/Problems tab if there was an error */
    if(exit_code != 0) {
		finish_compiling(FALSE, data);
		return;
    }
    
    /* Decide what to do next */
	if(!data->build_for_release) {
		I7Story *story = data->story;
		finish_compiling(TRUE, data);
		/* Hold the GDK lock for the callback */
		gdk_threads_enter();
		(priv->compile_finished_callback)(story, priv->compile_finished_callback_data);
		gdk_threads_leave();
		return;
	}

	prepare_cblorb_compiler(data);
	start_cblorb_compiler(data);
}

/* Get ready to run the CBlorb compiler. This function is called from a child 
 process watch, so the GDK lock is not held and must be acquired for any GUI 
 calls. */
static void 
prepare_cblorb_compiler(CompilerData *data) 
{
	gdk_threads_enter();
	i7_document_display_status_message(I7_DOCUMENT(data->story), _("Running cBlorb..."), COMPILE_OPERATIONS);
	gdk_threads_leave();
}

static void
parse_cblorb_output(I7Story *story, gchar *text)
{
	I7_STORY_USE_PRIVATE(story, priv);
	gchar *ptr = strstr(text, "Copy blorb to: [[");
	if(ptr) {
		g_free(priv->copyblorbto);
		priv->copyblorbto = g_strdup(ptr + 17);
		*(strstr(priv->copyblorbto, "]]")) = '\0';
	}
}

/* Run the CBlorb compiler. This function is called from a child process watch, 
 so the GDK lock is not held and must be acquired for any GUI calls. */
static void 
start_cblorb_compiler(CompilerData *data) 
{
	I7_STORY_USE_PRIVATE(data->story, priv);
	
    /* Build the command line */
    gchar **commandline = g_new(gchar *, 5);
    commandline[0] = i7_app_get_binary_path(i7_app_get(), "cBlorb");
	commandline[1] = g_strdup("-unix");
    commandline[2] = g_strdup("Release.blurb");
	commandline[3] = g_strdup(data->output_file);
    commandline[4] = NULL;
	
    GPid child_pid = run_command_hook(data->input_file, commandline, 
	    priv->progress, (IOHookFunc *)parse_cblorb_output, data->story, TRUE, FALSE);
    /* set up a watch for the exit status */
    g_child_watch_add(child_pid, (GChildWatchFunc)finish_cblorb_compiler, data);
    
    g_strfreev(commandline);
}
    
/* Display any errors from cBlorb. This function is called from a child process
 watch, so the GDK lock is not held and must be acquired for any GUI calls. */
static void 
finish_cblorb_compiler(GPid pid, gint status, CompilerData *data) 
{
	I7_STORY_USE_PRIVATE(data->story, priv);
	
    /* Clear the progress indicator */
	gdk_threads_enter();
	i7_document_remove_status_message(I7_DOCUMENT(data->story), COMPILE_OPERATIONS);
	gdk_threads_leave();
	
    /* Get exit code from CBlorb */
    int exit_code = WIFEXITED(status)? WEXITSTATUS(status) : -1;
    
    /* Display the appropriate HTML page */
    gchar *file = g_build_filename(data->input_file, "Build", "StatusCblorb.html", NULL);
	gdk_threads_enter();
    html_load_file(WEBKIT_WEB_VIEW(data->story->panel[LEFT]->errors_tabs[I7_ERRORS_TAB_PROBLEMS]), file);
    html_load_file(WEBKIT_WEB_VIEW(data->story->panel[RIGHT]->errors_tabs[I7_ERRORS_TAB_PROBLEMS]), file);
	gdk_threads_leave();
    g_free(file);

    /* Stop here and show the Errors/Problems tab if there was an error */
    if(exit_code != 0) {
        finish_compiling(FALSE, data);
        return;
    }
    
    /* Decide what to do next */
	I7Story *story = data->story;
	finish_compiling(TRUE, data);

	/* Hold the GDK lock for the callback */
	gdk_threads_enter();
	(priv->compile_finished_callback)(story, priv->compile_finished_callback_data);
	gdk_threads_leave();
}

/* Clean up the compiling stuff and notify the user that compiling has finished.
 All compiler tool chains must call this function at the end!! This function is
 called from a child process watch, so the GDK lock is not held and must be
 acquired for any GUI calls. */
static void 
finish_compiling(gboolean success, CompilerData *data) 
{
	I7_STORY_USE_PRIVATE(data->story, priv);
	
    /* Display status message */
	gdk_threads_enter();
	i7_document_remove_status_message(I7_DOCUMENT(data->story), COMPILE_OPERATIONS);
    i7_document_flash_status_message(I7_DOCUMENT(data->story), 
	    success? _("Compiling succeeded.") : _("Compiling failed."), 
	    COMPILE_OPERATIONS);
                                 
    /* Switch the Errors tab to the Problems page */
    i7_story_show_tab(data->story, I7_PANE_ERRORS, I7_ERRORS_TAB_PROBLEMS);
	gdk_threads_leave();

	/* Store the compiler output filename */
	priv->compiler_output = data->output_file;
	
	/* Free the compiler data object */
	g_free(data->input_file);
	g_free(data->directory);
	g_slice_free(CompilerData, data);
	
    /* Update */
	gdk_threads_enter();
    while(gtk_events_pending())
		gtk_main_iteration();
	gdk_threads_leave();
}

/* Finish up the user's Export iFiction Record command. This is a callback and
 the GDK lock is held when entering this function. */
void
i7_story_save_ifiction(I7Story *story)
{
	/* Work out where the file should be */
	gchar *path = i7_document_get_path(I7_DOCUMENT(story));
	gchar *ifiction_path = g_build_filename(path, "Metadata.iFiction", NULL);

	/* Prompt user to save iFiction file if it exists */
	if(g_file_test(ifiction_path, G_FILE_TEST_EXISTS))
	{
		/* Make a file filter */
		GtkFileFilter *filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, _("iFiction records (.iFiction)"));
		gtk_file_filter_add_pattern(filter, "*.iFiction");

		/* Make up a default file name */        
		gchar *name = i7_document_get_display_name(I7_DOCUMENT(story));
		*(strrchr(name, '.')) = '\0';
		gchar *filename = g_strconcat(name, ".iFiction", NULL);    
		g_free(name);

		/* Create a file chooser */
		GtkWidget *dialog = gtk_file_chooser_dialog_new(_("Save iFiction record"),
		    GTK_WINDOW(story), GTK_FILE_CHOOSER_ACTION_SAVE,
		    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		    GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
		    NULL);
		gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), filename);
		g_free(filename);
		gchar *directory = g_path_get_dirname(path);
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), directory);
		g_free(directory);
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

		/* Copy the finished file to the chosen location */
		if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		    filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

			/* Poor man's copy */
			gchar *text;
			GError *error = NULL;
		    if(!g_file_get_contents(ifiction_path, &text, NULL, &error) 
			  || !g_file_set_contents(filename, text, -1, &error)) {
		        error_dialog(GTK_WINDOW(story), error, 
				  _("Error copying iFiction record to '%s': "), filename);
		    }
		    g_free(filename);
		}
		gtk_widget_destroy(dialog);
	}
	else
		error_dialog(GTK_WINDOW(story), NULL, 
		    _("The compiler failed to create an iFiction record; check the "
			"errors page to see why."));

	g_free(path);
	g_free(ifiction_path);
}

/* Finish up the user's Release command by choosing a location to store the
project. This is a callback and the GDK lock is held when entering this 
 function. */
void 
i7_story_save_compiler_output(I7Story *story, const gchar *dialog_title) 
{
	I7_STORY_USE_PRIVATE(story, priv);
	
	gchar *filename = NULL;
	if(priv->copyblorbto == NULL) {
		/* ask the user for a release file name if cBlorb didn't provide one */
		
		/* Create a file chooser */
		GtkFileFilter *filter = gtk_file_filter_new();
		if(i7_story_get_story_format(story) == I7_STORY_FORMAT_GLULX) {
			gtk_file_filter_set_name(filter, _("Glulx games (.ulx,.gblorb)"));
			gtk_file_filter_add_pattern(filter, "*.ulx");
			gtk_file_filter_add_pattern(filter, "*.gblorb");
		} else {
			gtk_file_filter_set_name(filter, _("Z-code games (.z?,.zblorb)"));
			gtk_file_filter_add_pattern(filter, "*.z?");
			gtk_file_filter_add_pattern(filter, "*.zblorb");
		}
		GtkWidget *dialog = gtk_file_chooser_dialog_new(dialog_title,
		    GTK_WINDOW(story), GTK_FILE_CHOOSER_ACTION_SAVE,
		    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		    GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
		    NULL);
		gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
		gchar *curfilename = g_path_get_basename(priv->compiler_output);
		gchar *title = i7_document_get_display_name(I7_DOCUMENT(story));
		*(strrchr(title, '.')) = '\0';
		gchar *suggestname = g_strconcat(title, strrchr(curfilename, '.'), NULL);
		g_free(title);
		g_free(curfilename);
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), suggestname);
		g_free(suggestname);
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    
		if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
			filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		
		gtk_widget_destroy(dialog);
	} else {
		filename = g_strdup(priv->copyblorbto);
	}
	
	if(filename) {
    	/* Copy the finished file to the release location */
		
		/* try to copy the file */
        if(g_rename(priv->compiler_output, filename)) {
            if(errno == EXDEV) { 
                /* Can't rename across devices, so physically copy the file */
                gchar *contents;
                gsize length;
                GError *err = NULL;
                if(!g_file_get_contents(priv->compiler_output, &contents, &length, &err)
                   || !g_file_set_contents(filename, contents, length, &err)) {
                    error_dialog(GTK_WINDOW(story), err, 
                      /* TRANSLATORS: Error copying OLDFILE to NEWFILE */
                      _("Error copying file '%s' to '%s': "), priv->compiler_output, filename);
                    goto finally;
                }
            } else {
                error_dialog(GTK_WINDOW(story), NULL, 
                  /* TRANSLATORS: Error copying OLDFILE to NEWFILE: ERROR_MESSAGE */
                  _("Error copying file '%s' to '%s': %s"), priv->compiler_output, filename, g_strerror(errno));
                goto finally;
            }
        }
    }
	
finally:
	g_free(filename);
}
