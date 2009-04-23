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
#include <ctype.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>

#include "support.h"

#include "appwindow.h"
#include "configfile.h"
#include "datafile.h"
#include "error.h"
#include "prefs.h"
#include "skein.h"
#include "story.h"
#include "tabgame.h"

#include "gtkterp/gtkterp.h"

/* Callback for when the interpreter is finished */
static void 
on_interpreter_exit(GtkTerp *terp, gint exit_code, Story *thestory)
{
    gtk_terp_unload_game(terp);
    
    if(thestory->handler_finished)
        g_signal_handler_disconnect((gpointer)terp, thestory->handler_finished);
    if(thestory->handler_input)
        g_signal_handler_disconnect((gpointer)terp, thestory->handler_input);
    
    thestory->handler_finished = 0;
    thestory->handler_input = 0;
    
    gtk_widget_set_sensitive(lookup_widget(thestory->window, "stop"), FALSE);
    gtk_widget_set_sensitive(lookup_widget(thestory->window, "stop_toolbutton"),
      FALSE);
}

/* Grab commands entered by the user and store them in the skein */
static void 
catch_input(GtkTerp *terp, const gchar *command, Story *thestory)
{
    skein_new_line(thestory->theskein, command);
}

/* Create the GtkTerp widget and set some preferences */
GtkWidget*
game_create(gchar *widget_name, gchar *string1, gchar *string2, gint int1, 
            gint int2)
{
    GtkTerp *terp = GTK_TERP(gtk_terp_new());
    gtk_terp_set_interactive(terp, TRUE);
    gtk_terp_set_protected(terp, FALSE);
    return GTK_WIDGET(terp);
}

/* Resize the interpreter widget when its parent scroll window is allocated */
void
on_game_viewport_l_size_allocate(GtkWidget *widget, GtkAllocation *allocation,
                                 gpointer data)
{
    resize_game_window(get_story(widget), LEFT, allocation->width,
      allocation->height);
}

void
on_game_viewport_r_size_allocate(GtkWidget *widget, GtkAllocation *allocation,
                                 gpointer data)
{
    resize_game_window(get_story(widget), RIGHT, allocation->width,
      allocation->height);
}

/* Run the story in the GtkTerp widget */
void 
run_project(Story *thestory) 
{
    int right = choose_notebook(thestory->window, TAB_GAME);
    GtkTerp *terp = GTK_TERP(lookup_widget(thestory->window, right?
      "game_r" : "game_l"));
    
    /* Load and start the interpreter */
    gchar *file = g_strconcat("output.", get_story_extension(thestory), NULL);
    gchar *path = g_build_filename(thestory->filename, "Build", file, NULL);
    g_free(file);
    GError *err = NULL;
    if(!gtk_terp_load_game(terp, path, &err)) {
        error_dialog(GTK_WINDOW(thestory->window), err,
          _("Could not load interpreter: "));
        g_free(path);
        return;
    }
    g_free(path);
    
    /* Get a list of the commands that need to be fed in */
    GSList *commands = skein_get_commands(thestory->theskein);
    
    /* Set non-interactive if there are commands, because if we don't, the first
    screen might freeze on a "-- more --" prompt and ignore the first automatic
    input */
    if(commands)
        gtk_terp_set_interactive(terp, FALSE);
    
    if(!gtk_terp_start_game(terp, (thestory->story_format == FORMAT_GLULX)?
       (config_file_get_bool("IDESettings", "UseGit")? 
	   GTK_TERP_GIT : GTK_TERP_GLULXE) : GTK_TERP_FROTZ, &err)) {
        error_dialog(GTK_WINDOW(thestory->window), err,
          _("Could not start interpreter: "));
        gtk_terp_unload_game(terp);
        return;
    }

    /* Connect signals and save the signal handlers to disconnect later */
    thestory->handler_finished = g_signal_connect((gpointer)terp,
      "stopped", G_CALLBACK(on_interpreter_exit), (gpointer)thestory);
    thestory->handler_input = g_signal_connect((gpointer)terp,
      "command-received", G_CALLBACK(catch_input), (gpointer)thestory);
    
    /* Now the "Stop" option works */
    gtk_widget_set_sensitive(lookup_widget(thestory->window, "stop"), TRUE);
    gtk_widget_set_sensitive(lookup_widget(thestory->window, "stop_toolbutton"),
      TRUE);
    /* Display and set the focus to the terminal widget */
    gtk_notebook_set_current_page(get_notebook(thestory->window, right),
      TAB_GAME);
    gtk_widget_grab_focus(GTK_WIDGET(terp));
    
    /* Wait for the interpreter to get ready */
    while(!gtk_terp_get_running(terp))
        gtk_main_iteration();
    
    /* Feed the commands up to the current pointer in the skein into the
    terminal */
    skein_reset(thestory->theskein, TRUE);      
    while(commands != NULL) {
        gtk_terp_feed_command(terp, (gchar *)(commands->data));
        g_free(commands->data);
        commands = g_slist_delete_link(commands, commands);
    }
    gtk_terp_set_interactive(terp, TRUE);
}

/* Kill the interpreter if it is running */
void
stop_project(Story *thestory)
{
    GtkTerp *terp = GTK_TERP(lookup_widget(thestory->window, "game_l"));
    if(gtk_terp_get_game_loaded(terp))
        gtk_terp_stop_game(terp);
    terp = GTK_TERP(lookup_widget(thestory->window, "game_r"));
    if(gtk_terp_get_game_loaded(terp))
        gtk_terp_stop_game(terp);
}

/* Resize the interpreter */
void 
resize_game_window(Story *thestory, int right, guint w, guint h)
{
    GtkTerp *terp = GTK_TERP(lookup_widget(thestory->window,
                                           right? "game_r" : "game_l"));
    if(gtk_terp_get_running(terp))
        gtk_terp_set_minimum_size(terp, w, h);
}
/* Tell whether the interpreter is running on either side */
gboolean
game_is_running(Story *thestory)
{
    return
      gtk_terp_get_running(GTK_TERP(lookup_widget(thestory->window,"game_r")))||
      gtk_terp_get_running(GTK_TERP(lookup_widget(thestory->window,"game_l")));
}
