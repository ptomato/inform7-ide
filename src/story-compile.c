/* Copyright (C) 2006-2015, 2018 P. F. Chimento
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

#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <webkit2/webkit2.h>

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

#define INFORM6_COMPILER_NAME "inform6"

#include "story.h"
#include "story-private.h"
#include "configfile.h"
#include "error.h"
#include "html.h"
#include "spawn.h"

typedef struct _CompilerData {
	I7Story *story;
	gboolean create_blorb;
	gboolean use_debug_flags;
	gboolean refresh_only;
	GFile *input_file;
	GFile *output_file;
	GFile *builddir_file;
	GFile *results_file;
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
	if(priv->copy_blorb_dest_file) {
		g_object_unref(priv->copy_blorb_dest_file);
		priv->copy_blorb_dest_file = NULL;
	}
	if(priv->compiler_output_file) {
		g_object_unref(priv->compiler_output_file);
		priv->compiler_output_file = NULL;
	}
	gtk_action_group_set_sensitive(priv->compile_action_group, FALSE);

	/* Set up the compiler */
	CompilerData *data = g_slice_new0(CompilerData);
	data->story = story;
	data->create_blorb = release && i7_story_get_create_blorb(story);
	data->use_debug_flags = !release;
	data->refresh_only = refresh;

	gchar *filename;
	if(data->create_blorb) {
		if(i7_story_get_story_format(story) == I7_STORY_FORMAT_GLULX)
			filename = g_strdup("output.gblorb");
		else
			filename = g_strdup("output.zblorb");
	} else {
		filename = g_strconcat("output.", i7_story_get_extension(story), NULL);
	}
	data->input_file = i7_document_get_file(I7_DOCUMENT(story));
	data->builddir_file = g_file_get_child(data->input_file, "Build");
	data->output_file = g_file_get_child(data->builddir_file, filename);
	g_free(filename);

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
	html_load_blank(WEBKIT_WEB_VIEW(data->story->panel[LEFT]->results_tabs[I7_RESULTS_TAB_REPORT]));
	html_load_blank(WEBKIT_WEB_VIEW(data->story->panel[RIGHT]->results_tabs[I7_RESULTS_TAB_REPORT]));

	/* Create the UUID file if needed */
	GFile *uuid_file = g_file_get_child(data->input_file, "uuid.txt");
	if(!g_file_query_exists(uuid_file, NULL)) {
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
			g_object_unref(uuid_file);
			return;
		}
#endif /* UUID conditional */
		if(!g_file_replace_contents(uuid_file, uuid_string, strlen(uuid_string), NULL, FALSE, G_FILE_CREATE_NONE, NULL, NULL, &err)) {
			IO_ERROR_DIALOG(GTK_WINDOW(data->story), uuid_file, err, _("creating UUID file"));
			g_object_unref(uuid_file);
			return;
		}
#ifndef E2FS_UUID /* Only OSSP UUID */
		free(uuid_string);
#endif /* !OSSP_UUID */
	}
	g_object_unref(uuid_file);

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

	if(sscanf(text, " ++ %d%% (%m[^)]", &percent, &message) == 2) {
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
	GPtrArray *args = g_ptr_array_new_full(7, g_free); /* usual number of args */
	I7App *theapp = i7_app_get();
	GFile *ni_compiler = i7_app_get_binary_file(theapp, "ni");
	GFile *internal_dir = i7_app_get_internal_dir(theapp);
	g_ptr_array_add(args, g_file_get_path(ni_compiler));
	g_ptr_array_add(args, g_strdup("-internal"));
	g_ptr_array_add(args, g_file_get_path(internal_dir));
	g_ptr_array_add(args, g_strconcat("-format=", i7_story_get_extension(data->story), NULL));
	g_ptr_array_add(args, g_strdup("-project"));
	g_ptr_array_add(args, g_file_get_path(data->input_file));
	if(!data->use_debug_flags)
		g_ptr_array_add(args, g_strdup("-release")); /* Omit "not for relase" material */
	if(i7_story_get_nobble_rng(data->story))
		g_ptr_array_add(args, g_strdup("-rng"));
	g_ptr_array_add(args, NULL);

	g_object_unref(ni_compiler);
	g_object_unref(internal_dir);

	char **commandline = (char **)g_ptr_array_free(args, FALSE);

	/* Run the command and pipe its output to the text buffer. Also pipe stderr
	through a function that analyzes the progress messages and puts them in the
	progress bar. */
	GPid pid = run_command_hook(data->builddir_file, commandline, priv->progress,
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
	I7App *theapp = i7_app_get();
	GSettings *prefs = i7_app_get_prefs(theapp);

	/* Clear the progress indicator */
	gdk_threads_enter();
	i7_document_remove_status_message(I7_DOCUMENT(data->story), COMPILE_OPERATIONS);
	i7_document_clear_progress(I7_DOCUMENT(data->story));
	gdk_threads_leave();

	/* Get the ni.exe exit code */
	int exit_code = WIFEXITED(status)? WEXITSTATUS(status) : -1;

	/* Display the appropriate HTML error or success page */
	GFile *problems_file = NULL;
	if(exit_code <= 1) {
		/* In the case of success or a "normal" failure, or a negative error
		 code should one occur, display the compiler's generated Problems.html*/
		problems_file = g_file_get_child(data->builddir_file, "Problems.html");
	} else {
		gchar *file = g_strdup_printf("Error%i.html", exit_code);
		problems_file = i7_app_check_data_file_va(theapp, "Resources", "en", file, NULL);
		g_free(file);
		if(!problems_file)
			problems_file = i7_app_get_data_file_va(theapp, "Resources", "en", "Error0.html", NULL);
	}

	g_clear_object(&data->results_file);
	data->results_file = problems_file; /* assumes reference */

	if(g_settings_get_boolean(prefs, PREFS_SHOW_DEBUG_LOG)) {
		/* Update */
		gdk_threads_enter();
		while(gtk_events_pending())
			gtk_main_iteration();
		gdk_threads_leave();

		/* Refresh the debug log */
		gchar *text;
		GFile *debug_file = g_file_get_child(data->builddir_file, "Debug log.txt");
		/* Ignore errors, just don't show it if it's not there */
		if(g_file_load_contents(debug_file, NULL, &text, NULL, NULL, NULL)) {
			gdk_threads_enter();
			gtk_text_buffer_set_text(priv->debug_log, text, -1);
			gdk_threads_leave();
			g_free(text);
		}
		g_object_unref(debug_file);

		/* Refresh the I6 code */
		GFile *i6_file = g_file_get_child(data->builddir_file, "auto.inf");
		if(g_file_load_contents(i6_file, NULL, &text, NULL, NULL, NULL)) {
			gdk_threads_enter();
			gtk_text_buffer_set_text(GTK_TEXT_BUFFER(priv->i6_source), text, -1);
			gdk_threads_leave();
			g_free(text);
		}
		g_object_unref(i6_file);
	}

	/* Stop here and show the Results/Report tab if there was an error */
	if(exit_code != 0) {
		finish_compiling(FALSE, data);
		return;
	}

	/* Reload the Index in the background */
	i7_story_reload_index_tabs(data->story, FALSE);

	/* Read in the Blorb manifest */
	GFile *file = i7_document_get_file(I7_DOCUMENT(data->story));
	GFile *manifest_file = g_file_get_child(file, "manifest.plist");
	g_object_unref(file);
	PlistObject *manifest = plist_read_file(manifest_file, NULL, NULL);
	g_object_unref(manifest_file);
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
get_i6_compiler_switches(gboolean use_debug_flags, int format)
{
	gchar *debug_switches, *version_switches, *retval;

	/* Switch off strict warnings and debug if the game is for release */
	if(use_debug_flags)
		debug_switches = g_strdup("kSD");
	else
		debug_switches = g_strdup("~S~D");
	/* Pick the appropriate virtual machine version */
	switch(format) {
	case I7_STORY_FORMAT_GLULX:
		version_switches = g_strdup("G");
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

	GFile *i6_compiler = i7_app_get_binary_file(i7_app_get(), INFORM6_COMPILER_NAME);
	char *i6out = g_strconcat("output.", i7_story_get_extension(data->story), NULL);
	GFile *i6_output = g_file_get_child(data->builddir_file, i6out);
	g_free(i6out);

	/* Build the command line */
	gchar **commandline = g_new(gchar *, 6);
	commandline[0] = g_file_get_path(i6_compiler);
	commandline[1] = get_i6_compiler_switches(data->use_debug_flags, i7_story_get_story_format(data->story));
	commandline[2] = g_strdup("$huge");
	commandline[3] = g_strdup("auto.inf");
	commandline[4] = g_file_get_path(i6_output);
	commandline[5] = NULL;

	g_object_unref(i6_compiler);
	g_object_unref(i6_output);

	GPid child_pid = run_command_hook(data->builddir_file, commandline,
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
	GFile *loadfile = NULL;

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
				loadfile = i7_app_get_data_file_va(i7_app_get(), "Resources", "en", "ErrorI6MemorySetting.html", NULL);
			else if(strstr(msg, "This program has overflowed the maximum readable-memory size of the "))
				loadfile = i7_app_get_data_file_va(i7_app_get(), "Resources", "en", "ErrorI6Readable.html", NULL);
			else if(strstr(msg, "The story file exceeds "))
				loadfile = i7_app_get_data_file_va(i7_app_get(), "Resources", "en", "ErrorI6TooBig.html", NULL);
			else
				loadfile = i7_app_get_data_file_va(i7_app_get(), "Resources", "en", "ErrorI6.html", NULL);
			g_free(msg);
			break;
		}
		g_free(msg);
	}
	gdk_threads_leave();
	if(!loadfile && exit_code != 0)
		loadfile = i7_app_get_data_file_va(i7_app_get(), "Resources", "en", "ErrorI6.html", NULL);
	if(loadfile) {
		g_clear_object(&data->results_file);
		data->results_file = loadfile; /* assumes reference */
	}

	/* Stop here and show the Results/Report tab if there was an error */
	if(exit_code != 0) {
		finish_compiling(FALSE, data);
		return;
	}

	/* Decide what to do next */
	if(!data->create_blorb) {
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
		char *copy_blorb_path = g_strdup(ptr + 17);
		*(strstr(copy_blorb_path, "]]")) = '\0';

		if(priv->copy_blorb_dest_file)
			g_object_unref(priv->copy_blorb_dest_file);
		priv->copy_blorb_dest_file = g_file_new_for_path(copy_blorb_path);
		g_free(copy_blorb_path);
	}
}

/* Run the CBlorb compiler. This function is called from a child process watch,
 so the GDK lock is not held and must be acquired for any GUI calls. */
static void
start_cblorb_compiler(CompilerData *data)
{
	I7_STORY_USE_PRIVATE(data->story, priv);

	GFile *cblorb = i7_app_get_binary_file(i7_app_get(), "cBlorb");

	/* Build the command line */
	gchar **commandline = g_new(gchar *, 5);
	commandline[0] = g_file_get_path(cblorb);
	commandline[1] = g_strdup("-unix");
	commandline[2] = g_strdup("Release.blurb");
	commandline[3] = g_file_get_path(data->output_file);
	commandline[4] = NULL;

	g_object_unref(cblorb);

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
	g_clear_object(&data->results_file);
	data->results_file = g_file_get_child(data->builddir_file, "StatusCblorb.html");

	/* Stop here and show the Results/Report tab if there was an error */
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

/* FIXME: This is necessary because WebKit caches the Problems.html page, even
though Page Caching is turned off! So unless it is reloaded right after it is
loaded, the compiler's newly written version is not picked up. Shame there is no
webkit_web_view_load_uri_bypass_cache(). Presumably this is fixed in WebKit2. */
// static void
// on_load_status_finished_reload(WebKitWebView *html, GParamSpec *pspec)
// {
// 	WebKitLoadStatus status = webkit_web_view_get_load_status(html);
// 	if(status != WEBKIT_LOAD_FINISHED && status != WEBKIT_LOAD_FAILED)
// 		return;
// 	webkit_web_view_reload_bypass_cache(html);
// 	g_signal_handlers_disconnect_by_func(html, on_load_status_finished_reload, NULL);
// }

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

	/* Switch the Results tab to the Report page */
	html_load_file(WEBKIT_WEB_VIEW(data->story->panel[LEFT]->results_tabs[I7_RESULTS_TAB_REPORT]), data->results_file);
	html_load_file(WEBKIT_WEB_VIEW(data->story->panel[RIGHT]->results_tabs[I7_RESULTS_TAB_REPORT]), data->results_file);
	// g_signal_connect(data->story->panel[LEFT]->results_tabs[I7_RESULTS_TAB_REPORT], "notify::load-status", G_CALLBACK(on_load_status_finished_reload), NULL);
	// g_signal_connect(data->story->panel[RIGHT]->results_tabs[I7_RESULTS_TAB_REPORT], "notify::load-status", G_CALLBACK(on_load_status_finished_reload), NULL);
	i7_story_show_tab(data->story, I7_PANE_RESULTS, I7_RESULTS_TAB_REPORT);

	gtk_action_group_set_sensitive(priv->compile_action_group, TRUE);
	gdk_threads_leave();

	/* Store the compiler output filename (the I7Story now owns the reference) */
	priv->compiler_output_file = data->output_file;

	/* Free the compiler data object */
	g_object_unref(data->input_file);
	g_object_unref(data->builddir_file);
	g_clear_object(&data->results_file);
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
	GFile *project_file = i7_document_get_file(I7_DOCUMENT(story));
	if(project_file == NULL) {
		g_warning("Tried to save iFiction record of story without associated file");
		return; /* This shouldn't happen because the file is saved before compilation */
	}
	GFile *ifiction_file = g_file_get_child(project_file, "Metadata.iFiction");

	/* Prompt user to save iFiction file if it exists */
	if(g_file_query_exists(ifiction_file, NULL))
	{
		/* Make a file filter */
		GtkFileFilter *filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, _("iFiction records (.iFiction)"));
		gtk_file_filter_add_pattern(filter, "*.iFiction");

		/* Make up a default file name */
		gchar *name = i7_document_get_display_name(I7_DOCUMENT(story));
		/* project_file is not NULL so neither is name */
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

		GFile *parent_file = g_file_get_parent(project_file);
		/* Ignore error */
		gtk_file_chooser_set_current_folder_file(GTK_FILE_CHOOSER(dialog), parent_file, NULL);
		g_object_unref(parent_file);

		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

		/* Copy the finished file to the chosen location */
		if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
			GFile *dest_file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));
			GError *error = NULL;

			if(!g_file_copy(ifiction_file, dest_file, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, &error)) {
				IO_ERROR_DIALOG(GTK_WINDOW(story), dest_file, error, _("copying iFiction record"));
			}

			g_object_unref(dest_file);
		}
		gtk_widget_destroy(dialog);
	}
	else
		error_dialog(GTK_WINDOW(story), NULL,
			_("The compiler failed to create an iFiction record; check the "
			"results page to see why."));

	g_object_unref(ifiction_file);
	g_object_unref(project_file);
}

/* Finish up the user's Release command by choosing a location to store the
project. This is a callback and the GDK lock is held when entering this
 function. */
void
i7_story_save_compiler_output(I7Story *story, const gchar *dialog_title)
{
	I7_STORY_USE_PRIVATE(story, priv);

	GFile *file = NULL;
	if(priv->copy_blorb_dest_file == NULL) {
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
		char *curfilename = g_file_get_basename(priv->compiler_output_file);
		gchar *title = i7_document_get_display_name(I7_DOCUMENT(story));
		char *extension = strrchr(curfilename, '.'); /* not allocated */
		char *suggestname;
		if(title != NULL) {
			*(strrchr(title, '.')) = '\0';
			suggestname = g_strconcat(title, extension, NULL);
			g_free(title);
		} else {
			suggestname = g_strconcat("Untitled", extension, NULL);
		}
		g_free(curfilename);
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), suggestname);
		g_free(suggestname);
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

		if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
			file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));

		gtk_widget_destroy(dialog);
	} else {
		file = g_object_ref(priv->copy_blorb_dest_file);
	}

	if(file) {
		/* Move the finished file to the release location */
		GError *err = NULL;
		if(!g_file_move(priv->compiler_output_file, file, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, &err)) {
			IO_ERROR_DIALOG(GTK_WINDOW(story), file, err, _("copying compiler output"));
		}
		g_object_unref(file);
	}
}
