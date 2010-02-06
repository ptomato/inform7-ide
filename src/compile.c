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
 
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#ifdef OSSP_UUID
#  include <ossp/uuid.h> /* For systems with OSSP uuid */
#else
#  ifdef E2FS_UUID
#    include <uuid/uuid.h> /* If no OSSP uuid, use e2fsprogs uuid */
#  else
#    include <uuid.h> /* Otherwise, it is OSSP uuid for Fedora 5 and earlier */
#    define OSSP_UUID
#  endif
#endif

#include "support.h"

#include "appwindow.h"
#include "compile.h"
#include "configfile.h"
#include "datafile.h"
#include "error.h"
#include "html.h"
#include "spawn.h"
#include "story.h"
#include "tabgame.h"
#include "tabindex.h"

#define PROBLEMS_FILE "Build", "Problems.html"

/* Declare these functions static so they can stay in this order */
static void prepare_ni_compiler(Story *thestory);
static void start_ni_compiler(Story *thestory);
static void finish_ni_compiler(GPid pid, gint status, Story *thestory);
static void prepare_i6_compiler(Story *thestory);
static void start_i6_compiler(Story *thestory);
static void finish_i6_compiler(GPid pid, gint status, Story *thestory);
static void prepare_cblorb_compiler(Story *thestory);
static void start_cblorb_compiler(Story *thestory);
static void finish_cblorb_compiler(GPid pid, gint status, Story *thestory);
static void finish_refresh_index(Story *thestory);
static void finish_save_debug_build(Story *thestory);
static void finish_run(Story *thestory);
static void finish_release(Story *thestory);
    
/* Start the compiler running the census of extensions. If wait is TRUE, it will
not do it in the background. */
void
run_census(gboolean wait) 
{
    /* Build the command line */
    gchar **commandline = g_new(gchar *, 5);
    commandline[0] = get_binary_path("ni");
    commandline[1] = g_strdup("-rules");
    commandline[2] = get_datafile_path("Extensions");
    commandline[3] = g_strdup("-census");
    commandline[4] = NULL;
    
    if(wait)
        g_spawn_sync(g_get_home_dir(), commandline, NULL, G_SPAWN_SEARCH_PATH
          | G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL,
          NULL, NULL, NULL, NULL, NULL, NULL);
    else
        g_spawn_async(g_get_home_dir(), commandline, NULL, G_SPAWN_SEARCH_PATH
          | G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL,
          NULL, NULL, NULL, NULL);
    
    g_strfreev(commandline);
}

/* Start the compiling process */
void
compile_project(Story *thestory) 
{
    prepare_ni_compiler(thestory);
    start_ni_compiler(thestory);
}

/* Set everything up for using the NI compiler */
static void 
prepare_ni_compiler(Story *thestory) 
{
    GError *err = NULL;
    
    /* Output buffer for messages */
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(lookup_widget(thestory->window, "compiler_output_l")));
    
    /* Show the Errors/Progress tab */
    gtk_notebook_set_current_page(GTK_NOTEBOOK(lookup_widget(thestory->window,
      "errors_notebook_r")), TAB_ERRORS_PROGRESS);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(lookup_widget(thestory->window,
      "errors_notebook_l")), TAB_ERRORS_PROGRESS);
    int right = choose_notebook(thestory->window, TAB_ERRORS);
    gtk_notebook_set_current_page(get_notebook(thestory->window, right),
      TAB_ERRORS);
    
    /* Clear the previous compile output */
    gtk_text_buffer_set_text(buffer, "", -1);
    html_load_blank(GTK_HTML(lookup_widget(thestory->window, "problems_l")));
    html_load_blank(GTK_HTML(lookup_widget(thestory->window, "problems_r")));
    
    /* Create the UUID file if needed */
    gchar *uuid_file = g_build_filename(thestory->filename, "uuid.txt", NULL);
    if(!g_file_test(uuid_file, G_FILE_TEST_EXISTS)) {
#ifdef OSSP_UUID /* code for OSSP uuid */
        gchar *uuid_string = NULL; /* a new buffer is allocated if NULL */
        uuid_t *uuid;
        
        if(!((uuid_create(&uuid) == UUID_RC_OK) &&
          (uuid_make(uuid, UUID_MAKE_V1) == UUID_RC_OK) &&
          (uuid_export(uuid, UUID_FMT_STR, (void **)&uuid_string, NULL)
            == UUID_RC_OK) &&
          (uuid_destroy(uuid) == UUID_RC_OK))) {
            error_dialog(GTK_WINDOW(thestory->window), NULL,
              _("Error creating UUID."));
            g_free(uuid_file);
            return;
        }
#else /* Code for e2fsprogs UUID */
        uuid_t uuid;
        gchar uuid_string[37];
        
        uuid_generate_time(uuid);
        uuid_unparse(uuid, uuid_string);
#endif /* UUID conditional */
        if(!g_file_set_contents(uuid_file, uuid_string, -1, &err)) {
            error_dialog(GTK_WINDOW(thestory->window), err,
              _("Error creating UUID file: "));
            g_free(uuid_file);
            return;
        }
#ifdef OSSP_UUID
        free(uuid_string);
#endif /* OSSP_UUID */
    }
    g_free(uuid_file);
    
    /* Display status message */
    display_status_message(thestory->window, _("Running Natural Inform..."));
}

/* Display the NI compiler's status in the app status bar */
static void 
display_ni_status(GtkProgressBar *progressbar, gchar *text) 
{
    gint percent;
    gchar *message;
    
    if(sscanf(text, " ++ %d%% (%a[^)]", &percent, &message) == 2) {
        gtk_progress_bar_set_fraction(progressbar, percent / 100.0);
        gtk_progress_bar_set_text(progressbar, message);
        free(message);
    }
}

/* Start the NI compiler and set up the callback for when it is finished */
static void 
start_ni_compiler(Story *thestory) 
{
    /* Output buffer for messages */
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(lookup_widget(thestory->window, "compiler_output_l")));
    
    /* Build the command line */
    gchar **commandline; 
    if(thestory->action == COMPILE_RELEASE) {
        commandline = g_new(gchar *, 8);
        commandline[6] = g_strdup("-release");
        commandline[7] = NULL;
    } else if(thestory->nobble_rng) {
		commandline = g_new(gchar *, 8);
		commandline[6] = g_strdup("-rng");
		commandline[7] = NULL;
	} else {
        commandline = g_new(gchar *, 7);
        commandline[6] = NULL;
    }
    commandline[0] = get_binary_path("ni");
    commandline[1] = g_strdup("-rules");
    commandline[2] = get_datafile_path("Extensions");
    commandline[3] = g_strconcat("-extension=", get_story_extension(thestory), 
								 NULL);
    commandline[4] = g_strdup("-package");
    commandline[5] = g_strdup(thestory->filename);

    /* Run the command and pipe its output to the text buffer. Also pipe stderr
    through a function that analyzes the progress messages and puts them in the
    progress bar. */
    GtkProgressBar *progress = gnome_appbar_get_progress(
      GNOME_APPBAR(lookup_widget(thestory->window, "main_appbar")));
    GPid pid = run_command_hook(thestory->filename, commandline, buffer,
                                (IOHookFunc *)display_ni_status, progress, 
								FALSE, TRUE);
    /* set up a watch for the exit status */
    g_child_watch_add(pid, (GChildWatchFunc)finish_ni_compiler, thestory);
    
    g_strfreev(commandline);
}

/* Display any errors from the NI compiler and continue on */
static void 
finish_ni_compiler(GPid pid, gint status, Story *thestory) 
{
    /* Clear the progress indicator */
    clear_status(thestory->window);
        
    /* Get the ni.exe exit code */
    int exit_code = WIFEXITED(status)? WEXITSTATUS(status) : -1;
      
    /* Display the appropriate HTML error or success page */
    gchar *loadfile;
    gchar *trash = NULL;
    switch(exit_code) {
        case 0:
        case 1:
            loadfile = g_build_filename(thestory->filename, PROBLEMS_FILE,NULL);
            break;
        case 10:
            loadfile = get_datafile_path_va("Documentation", "Sections",
              "Error10.html", NULL);
            break;
        case 11:
            loadfile = get_datafile_path_va("Documentation", "Sections",
              "Error11.html", NULL);
            break;
        default:
            loadfile = get_datafile_path_va("Documentation", "Sections",
              "Error0.html", NULL);
            break;
    }
    g_free(trash);
    html_load_file(GTK_HTML(lookup_widget(thestory->window, "problems_l")),
      loadfile);
    html_load_file(GTK_HTML(lookup_widget(thestory->window, "problems_r")),
      loadfile);
    g_free(loadfile);
    
    if(config_file_get_bool("IDESettings", "DebugLogVisible")) {
        /* Update */
        while(gtk_events_pending())
            g_main_context_iteration(NULL, FALSE);
        
        /* Refresh the debug log */
        gchar *text;
        gchar *filename = g_build_filename(thestory->filename, "Build",
          "Debug log.txt", NULL);
        /* Ignore errors, just don't show it if it's not there */
        if(g_file_get_contents(filename, &text, NULL, NULL))
            gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(
              lookup_widget(thestory->window, "debugging_l"))), text, -1);
        g_free(text);
        g_free(filename);
        
        /* Refresh the I6 code */
        filename = g_build_filename(thestory->filename, "Build", "auto.inf",
          NULL);
        if(g_file_get_contents(filename, &text, NULL, NULL))
            gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(
              lookup_widget(thestory->window, "inform6_l"))), text, -1);
        g_free(text);
        g_free(filename);
    }
      
    /* Stop here and show the Errors/Problems tab if there was an error */
    if(exit_code != 0) {
        gtk_notebook_set_current_page(
          GTK_NOTEBOOK(lookup_widget(thestory->window, "errors_notebook_l")),
          TAB_ERRORS_PROBLEMS);
        gtk_notebook_set_current_page(
          GTK_NOTEBOOK(lookup_widget(thestory->window, "errors_notebook_r")),
          TAB_ERRORS_PROBLEMS);
        int right = choose_notebook(thestory->window, TAB_ERRORS);
        gtk_notebook_set_current_page(get_notebook(thestory->window, right),
          TAB_ERRORS);
        display_status_message(thestory->window, _("Compiling failed."));
        return;
    }

    /* Decide what to do next */
    switch(thestory->action) {
    case COMPILE_REFRESH_INDEX:
        finish_refresh_index(thestory);
        break;
    case COMPILE_SAVE_DEBUG_BUILD:
    case COMPILE_RUN:
	case COMPILE_TEST_ME:
    case COMPILE_RELEASE:
        prepare_i6_compiler(thestory);
        start_i6_compiler(thestory);
        break;
    default:
        ;
    }
}
    

/* Get ready to run the I6 compiler; right now this does almost nothing */
static void 
prepare_i6_compiler(Story *thestory) 
{
    display_status_message(thestory->window, _("Running Inform 6..."));
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
    case FORMAT_GLULX:
        version_switches = g_strdup("G");
        break;
    case FORMAT_Z8:
        version_switches = g_strdup("v8");
        break;
    case FORMAT_Z6:
        version_switches = g_strdup("v6");
        break;
    case FORMAT_Z5:
    default:
        version_switches = g_strdup("v5");
    }
    
    retval = g_strconcat("-wxE2", debug_switches, version_switches, NULL);
    g_free(debug_switches);
    g_free(version_switches);
    return retval;
}

static void 
display_i6_status(GtkProgressBar *progressbar, gchar *text) 
{
    if(strchr(text, '#'))
        gtk_progress_bar_pulse(progressbar);
}

/* Run the I6 compiler */
static void 
start_i6_compiler(Story *thestory) 
{
    /* Get the text buffer to put our output in */
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(lookup_widget(thestory->window, "compiler_output_l")));
    
    /* Build the command line */
    gchar *working_dir = g_build_filename(thestory->filename, "Build", NULL);
    gchar **commandline = g_new(gchar *, 6);
    commandline[0] = get_binary_path("inform-6.31-biplatform");
    commandline[1] = get_i6_compiler_switches(
      thestory->action == COMPILE_RELEASE, thestory->story_format);
    commandline[2] = g_strdup("$huge");
    commandline[3] = g_strdup("auto.inf");
    commandline[4] = g_strconcat("output.", get_story_extension(thestory),NULL);
    commandline[5] = NULL;

    GtkProgressBar *progress = gnome_appbar_get_progress(
      GNOME_APPBAR(lookup_widget(thestory->window, "main_appbar")));
    GPid child_pid = run_command_hook(working_dir, commandline, buffer,
      (IOHookFunc *)display_i6_status, progress, TRUE, TRUE);
    /* set up a watch for the exit status */
    g_child_watch_add(child_pid, (GChildWatchFunc)finish_i6_compiler, thestory);
    
    g_strfreev(commandline);
    g_free(working_dir);
}

/* Display any errors from Inform 6 and decide what to do next */
static void 
finish_i6_compiler(GPid pid, gint status, Story *thestory) 
{
    /* Clear the status bar */
    clear_status(thestory->window);
    
    /* Get exit code from I6 process */
    int exit_code = WIFEXITED(status)? WEXITSTATUS(status) : -1;
    
    /* Display the exit status of the I6 compiler in the Progress tab */
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(lookup_widget(thestory->window, "compiler_output_l")));
      
    gchar *statusmsg = g_strdup_printf(_("\nCompiler finished with code %d\n"),
      exit_code);
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    gtk_text_buffer_insert(buffer, &iter, statusmsg, -1);
    g_free(statusmsg);
    
    GtkTextIter start, end;
    int line;
    gchar *loadfile = NULL;

    /* Display the appropriate HTML error pages */
    for(line = gtk_text_buffer_get_line_count(buffer); line >= 0; line--) {
        gchar *msg, *pos;
        gtk_text_buffer_get_iter_at_line(buffer, &start, line);
        end = start;
        gtk_text_iter_forward_to_line_end(&end);
        msg = gtk_text_iter_get_text(&start, &end);
        if((pos = strstr(msg, "rror:"))) { /* "Error:", "Fatal error:" */
            pos += 5; /* skip those five chars */
            g_strchug(pos); /* remove leading whitespace */
            
            char scratch[256];
            if(sscanf(pos, "The memory setting %[^)] has been exceeded.",
              scratch) == 1)
                loadfile = get_datafile_path_va("Documentation", "Sections", 
                  "ErrorI6MemorySetting.html", NULL);
            else if(sscanf(pos, "This program has overflowed the maximum "
              "readable-memory size of the %s format.", scratch) == 1)
                loadfile = get_datafile_path_va("Documentation", "Sections",
                  "ErrorI6Readable.html", NULL);
            else if(sscanf(pos, "The story file exceeds %s limit",
              scratch) == 1)
                loadfile = get_datafile_path_va("Documentation", "Sections",
                  "ErrorI6TooBig.html", NULL);
            else
                loadfile = get_datafile_path_va("Documentation", "Sections",
                  "ErrorI6.html", NULL);
           
            g_free(msg);
            break;
        }
        g_free(msg);
    }
    if(!loadfile && exit_code)
        loadfile = get_datafile_path_va("Documentation", "Sections", 
                                        "ErrorI6.html", NULL);
    if(loadfile) {
        html_load_file(GTK_HTML(lookup_widget(thestory->window, "problems_l")),
          loadfile);
        html_load_file(GTK_HTML(lookup_widget(thestory->window, "problems_r")),
          loadfile);
        g_free(loadfile);
    }
    
    /* Stop here and show the Errors/Problems tab if there was an error */
    if(exit_code != 0) {
        gtk_notebook_set_current_page(
          GTK_NOTEBOOK(lookup_widget(thestory->window, "errors_notebook_l")),
          TAB_ERRORS_PROBLEMS);
        gtk_notebook_set_current_page(
          GTK_NOTEBOOK(lookup_widget(thestory->window, "errors_notebook_r")),
          TAB_ERRORS_PROBLEMS);
        int right = choose_notebook(thestory->window, TAB_ERRORS);
        gtk_notebook_set_current_page(get_notebook(thestory->window, right),
          TAB_ERRORS);
        display_status_message(thestory->window, _("Compiling failed."));
        return;
    }
    
    /* Decide what to do next */
    switch(thestory->action) {
    case COMPILE_SAVE_DEBUG_BUILD:
        finish_save_debug_build(thestory);
        break;
    case COMPILE_RUN:
	case COMPILE_TEST_ME:
        finish_run(thestory);
        break;
    case COMPILE_RELEASE:
        if(thestory->make_blorb) {
            prepare_cblorb_compiler(thestory);
            start_cblorb_compiler(thestory);
        } else
            finish_release(thestory);
        break;
    default:
        ;
    }
}

/* Get ready to run the CBlorb compiler */
static void 
prepare_cblorb_compiler(Story *thestory) 
{
    display_status_message(thestory->window, _("Running cBlorb..."));
}

static void
parse_cblorb_output(Story *thestory, gchar *text)
{
	gchar *ptr = strstr(text, "Copy blorb to: [[");
	if(ptr) {
		thestory->copyblorbto = g_strdup(ptr + 17);
		*(strstr(thestory->copyblorbto, "]]")) = '\0';
	}
}

/* Run the CBlorb compiler */
static void 
start_cblorb_compiler(Story *thestory) 
{
    /* Get buffer for messages */
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(lookup_widget(thestory->window, "compiler_output_l")));
    
    /* Build the command line */
	gchar *outfile = g_strconcat("output.", 
		thestory->story_format == FORMAT_GLULX? "g" : "z", "blorb", NULL);
    gchar *working_dir = g_strdup(thestory->filename);
    gchar **commandline = g_new(gchar *, 5);
    commandline[0] = get_binary_path("cBlorb");
	commandline[1] = g_strdup("-unix");
    commandline[2] = g_strdup("Release.blurb");
	commandline[3] = g_build_filename("Build", outfile, NULL);
    commandline[4] = NULL;
	g_free(outfile);

	if(thestory->copyblorbto) {
		g_free(thestory->copyblorbto);
		thestory->copyblorbto = NULL;
	}
	
    GPid child_pid = run_command_hook(working_dir, commandline, buffer,
									  (IOHookFunc *)parse_cblorb_output,
									  thestory, TRUE, FALSE);
    /* set up a watch for the exit status */
    g_child_watch_add(child_pid, (GChildWatchFunc)finish_cblorb_compiler,
                      thestory);
    
    g_strfreev(commandline);
    g_free(working_dir);
    return;
}
    
/* Display any errors from cBlorb */
static void 
finish_cblorb_compiler(GPid pid, gint status, Story *thestory) 
{
    /* Get exit code from CBlorb */
    int exit_code = WIFEXITED(status)? WEXITSTATUS(status) : -1;
    
    /* Display the appropriate HTML page */
    gchar *trash = get_datafile_path_va("Documentation", "Sections",
      (exit_code == 0)? "GoodCblorb.html" : "ErrorCblorb.html", NULL);
    html_load_file(GTK_HTML(lookup_widget(thestory->window, "problems_l")),
      trash);
    html_load_file(GTK_HTML(lookup_widget(thestory->window, "problems_r")),
      trash);
    g_free(trash);

    /* Stop here and show the Errors/Problems tab if there was an error */
    if(exit_code != 0) {
        int right = choose_notebook(thestory->window, TAB_ERRORS);
        gtk_notebook_set_current_page(get_notebook(thestory->window, right),
          TAB_ERRORS);
        gtk_notebook_set_current_page(
          GTK_NOTEBOOK(lookup_widget(thestory->window,
          right? "errors_notebook_r" : "errors_notebook_l")),
          TAB_ERRORS_PROBLEMS);
        display_status_message(thestory->window, _("Compiling failed."));
        return;
    }
    
    /* Decide what to do next */
    switch(thestory->action) {
    case COMPILE_RELEASE:
        finish_release(thestory);
        break;
    default:
        ;
    }
}

static void 
finish_common(Story *thestory) 
{
    /* Display status message */
    display_status_message(thestory->window, _("Compiling succeeded."));
                                 
    /* Switch the Errors tab to the Problems page */
    gtk_notebook_set_current_page(GTK_NOTEBOOK(lookup_widget(thestory->window,
      "errors_notebook_r")), TAB_ERRORS_PROBLEMS);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(lookup_widget(thestory->window,
      "errors_notebook_l")), TAB_ERRORS_PROBLEMS);
  
    /* Update */
    while(gtk_events_pending())
	  g_main_context_iteration(NULL, FALSE);
}

/* Finish up the user's Refresh Index command */
static void 
finish_refresh_index(Story *thestory) 
{
    finish_common(thestory);
    
    /* Refresh the index and documentation tabs */
    reload_index_tabs(thestory, TRUE);
    html_refresh(GTK_HTML(lookup_widget(thestory->window, "docs_l")));
    html_refresh(GTK_HTML(lookup_widget(thestory->window, "docs_r")));
    
    /* Display the index */
    gtk_notebook_set_current_page(get_notebook(thestory->window,
      choose_notebook(thestory->window, TAB_INDEX)), TAB_INDEX);
}

/* Finish up the user's Save Debug Build command */
static void 
finish_save_debug_build(Story *thestory) 
{
    finish_common(thestory);
    
    /* Switch to the Errors tab */
    int right = choose_notebook(thestory->window, TAB_ERRORS);
    gtk_notebook_set_current_page(get_notebook(thestory->window, right),
      TAB_ERRORS);
    
    /* Make a file filter */
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, _("Game files (.z?,.ulx)"));
    gtk_file_filter_add_pattern(filter, "*.ulx");
    gtk_file_filter_add_pattern(filter, "*.z?");
    
    /* Get the appropriate file name extension */        
    gchar *ext = g_strdup(get_story_extension(thestory));
    /* Append it to the file name */
    gchar *name = g_path_get_basename(thestory->filename);
    gchar *pos = strchr(name, '.');
    *pos = '\0';
    gchar *filename = g_strconcat(name, ".", ext, NULL);    
    g_free(name);
    
    /* Create a file chooser */
    GtkWidget *dialog = gtk_file_chooser_dialog_new(_("Save debug build"),
      GTK_WINDOW(thestory->window), GTK_FILE_CHOOSER_ACTION_SAVE,
      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
      NULL);
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog),
      TRUE);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), filename);
    g_free(filename);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    
    /* Copy the finished file to the chosen location */
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gchar *oldfile_base = g_strconcat("output.", ext, NULL);
        gchar *oldfile = g_build_filename(thestory->filename, "Build",
          oldfile_base, NULL);
        g_free(oldfile_base);
        
        if(g_rename(oldfile, filename)) {
            error_dialog(NULL, NULL, _("Error copying file '%s' to '%s': "),
              oldfile, filename);
            g_free(filename);
            g_free(oldfile);
            g_free(ext);
            gtk_widget_destroy(dialog);
            return;
        }

        g_free(filename);
        g_free(oldfile);
    }
    
    g_free(ext);
    gtk_widget_destroy(dialog);
    
    /* Refresh the index and documentation tabs */
    reload_index_tabs(thestory, FALSE);
    html_refresh(GTK_HTML(lookup_widget(thestory->window, "docs_l")));
    html_refresh(GTK_HTML(lookup_widget(thestory->window, "docs_r")));
}

/* Finish up the user's Go or Replay command */
static void 
finish_run(Story *thestory) 
{
    finish_common(thestory);
    
    /* Run the project */
    run_project(thestory);
    
    /* Refresh the index and documentation tabs */
    reload_index_tabs(thestory, FALSE);
    html_refresh(GTK_HTML(lookup_widget(thestory->window, "docs_l")));
    html_refresh(GTK_HTML(lookup_widget(thestory->window, "docs_r")));
}


/* Finish up the user's Release command by choosing a location to store the
project */
static void 
finish_release(Story *thestory) 
{
    finish_common(thestory);
    
    /* Switch to the Errors tab */
    int right = choose_notebook(thestory->window, TAB_ERRORS);
    gtk_notebook_set_current_page(get_notebook(thestory->window, right),
      TAB_ERRORS);

	/* Get the appropriate file name extension */
	gchar *ext = g_strdup(thestory->make_blorb?
	  (thestory->story_format == FORMAT_GLULX? 
	   "gblorb" : "zblorb") : get_story_extension(thestory));
	
	gchar *filename = NULL;
	if(thestory->copyblorbto == NULL) {
		/* ask the user for a release file name if cBlorb didn't provide one */
		
		/* Append the extension to the file name */
		gchar *name = g_path_get_basename(thestory->filename);
		*(strchr(name, '.')) = '\0';
		gchar *curfilename = g_strconcat(name, ".", ext, NULL);    
		g_free(name);
		
		/* Create a file chooser */
		GtkFileFilter *filter = gtk_file_filter_new();
		if(thestory->story_format == FORMAT_GLULX) {
			gtk_file_filter_set_name(filter, _("Glulx games (.ulx,.gblorb)"));
			gtk_file_filter_add_pattern(filter, "*.ulx");
			gtk_file_filter_add_pattern(filter, "*.gblorb");
		} else {
			gtk_file_filter_set_name(filter, _("Z-code games (.z?,.zblorb)"));
			gtk_file_filter_add_pattern(filter, "*.z?");
			gtk_file_filter_add_pattern(filter, "*.zblorb");
		}
		GtkWidget *dialog = gtk_file_chooser_dialog_new(
		  _("Save the game for release"),
		  GTK_WINDOW(thestory->window), GTK_FILE_CHOOSER_ACTION_SAVE,
		  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		  GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
		  NULL);
		gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog),
		  TRUE);
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), curfilename);
		g_free(curfilename);
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    
		if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
			filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		
		gtk_widget_destroy(dialog);
	} else {
		filename = g_strdup(thestory->copyblorbto);
	}
	
	if(filename) {
    	/* Copy the finished file to the release location */
    
        /* Determine the name of the compiler output file */
        gchar *oldfile, *temp;
		temp = g_strconcat("output.", ext, NULL);
        oldfile = g_build_filename(thestory->filename, "Build", temp, NULL);
		g_free(temp);
		
		/* try to copy the file */
        if(g_rename(oldfile, filename)) {
            if(errno == EXDEV) { 
                /* Can't rename across devices, so physically copy the file */
                gchar *contents;
                gsize length;
                GError *err = NULL;
                if(!g_file_get_contents(oldfile, &contents, &length, &err)
                   || !g_file_set_contents(filename, contents, length, &err)) {
                    error_dialog(NULL, err, 
                      /* TRANSLATORS: Error copying OLDFILE to NEWFILE */
                      _("Error copying file '%s' to '%s': "),
                      oldfile, filename);
                    g_free(filename);
                    g_free(oldfile);
                    g_free(ext);
                    return;
                }
            } else {
                error_dialog(NULL, NULL, 
                  /* TRANSLATORS: Error copying OLDFILE to NEWFILE: ERROR_MESSAGE */
                  _("Error copying file '%s' to '%s': %s"),
                  oldfile, filename, g_strerror(errno));
                g_free(filename);
                g_free(oldfile);
                g_free(ext);
                return;
            }
        }
        g_free(oldfile);
    }
	
	g_free(filename);
	g_free(ext);
	
    /* Refresh the index and documentation tabs */
    reload_index_tabs(thestory, FALSE);
    html_refresh(GTK_HTML(lookup_widget(thestory->window, "docs_l")));
    html_refresh(GTK_HTML(lookup_widget(thestory->window, "docs_r")));
}
