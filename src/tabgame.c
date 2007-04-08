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
#include <ctype.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <vte/vte.h>

#include "tabgame.h"
#include "story.h"
#include "appwindow.h"
#include "support.h"
#include "skein.h"
#include "prefs.h"

/* Callback for when the child process is finished */
static void on_interpreter_exit(VteTerminal *terminal, gpointer thestory) {
    gchar *msg = g_strdup("[The game has finished.]\n\r");
    vte_terminal_feed(terminal, msg, strlen(msg));
    g_free(msg);
    
    if(((struct story *)thestory)->handler_child_exit)
        g_signal_handler_disconnect((gpointer)terminal,
          ((struct story *)thestory)->handler_child_exit);
    if(((struct story *)thestory)->handler_commit)
        g_signal_handler_disconnect((gpointer)terminal,
          ((struct story *)thestory)->handler_commit);
              
    ((struct story *)thestory)->interp_running = FALSE;
    ((struct story *)thestory)->interp_process = 0;
    ((struct story *)thestory)->handler_child_exit = 0;
    ((struct story *)thestory)->handler_commit = 0;

    gtk_widget_set_sensitive(lookup_widget(((struct story *)thestory)->window,
      "stop"), FALSE);
    gtk_widget_set_sensitive(lookup_widget(((struct story *)thestory)->window,
      "stop_toolbutton"), FALSE);
}

/* Always returns TRUE, workaround for a bug in vte */
static gboolean return_true(VteTerminal*foo, glong bar, glong baz, gpointer qux)
{
    return TRUE;
}

/* When the terminal widget intercepts an Enter (0x0D) it grabs the text in
between the ">" prompt and the end of the current line. (Anything typed at a
prompt other than ">" or "> " is ignored; this is a bug.) */
static void catch_input(VteTerminal *terminal, gchar *input, guint length,
gpointer thestory) {
    if(length == 1 && input[0] == '\x0D') {
        glong column;
        glong row;
        vte_terminal_get_cursor_position(terminal, &column, &row);
        char *command = vte_terminal_get_text_range(terminal, row, 0,
          row, column, return_true, NULL, NULL);
        /* We have to use a dummy function that always returns true, because
        there is a bug in vte that doesn't disable the callback if it is NULL */
        if(command[0] == '>') {
            struct node *newnode = g_malloc(sizeof(struct node));
            /* Remove the initial prompt and trailing newline */
            if(strlen(command + 1) && command[1] == ' ')
                newnode->command = g_strndup(command + 2, strlen(command) - 3);
            else
                newnode->command = g_strndup(command + 1, strlen(command) - 2);
            ((struct story *)thestory)->theskein = add_node(
                ((struct story *)thestory)->theskein,
                &(((struct story *)thestory)->skein_ptr), newnode);
        }
        free(command);
    }
}

/* Create the VteTerminal widget and set some preferences */
GtkWidget*
game_create (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2)
{
    VteTerminal *term = VTE_TERMINAL(vte_terminal_new());
    vte_terminal_set_audible_bell(term, FALSE);
    vte_terminal_set_allow_bold(term, TRUE);
    vte_terminal_set_scroll_on_keystroke(term, TRUE);
    vte_terminal_set_font_from_string(term, "DejaVu Sans Mono 10");
    update_font_size(GTK_WIDGET(term));
    return GTK_WIDGET(term);
}

/* Run the story in the VteTerminal widget */
void run_project(struct story *thestory) {
    int right = choose_notebook(thestory->window, TAB_GAME);
    gtk_notebook_set_current_page(get_notebook(thestory->window, right),
      TAB_GAME);
    VteTerminal *term = VTE_TERMINAL(lookup_widget(thestory->window, right?
      "game_r" : "game_l"));
    
    
    vte_terminal_reset(term, TRUE, TRUE);
    
    /* Build the command line */
    gchar **args;
    if(thestory->story_format == FORMAT_GLULX) {
        args = g_new(gchar *,4);
        args[0] = get_datafile_path_va("Interpreters", "dumb-glulxe", NULL);
        args[1] = g_strdup("-w57");
        args[2] = g_strdup("output.ulx");
        args[3] = NULL;
    } else {
        args = g_new(gchar *,5);
        args[0] = g_strdup("frotz");
        args[1] = g_strdup("-w");
        args[2] = g_strdup("57");
        args[3] = g_strconcat("output.", get_story_extension(thestory), NULL);
        args[4] = NULL;
    }
    gchar *dir = g_build_filename(thestory->filename, "Build", NULL);

    /* Save the PID so we can kill it later, and save the signal handlers so we
    can disconnect them */
    thestory->interp_running = TRUE;
    thestory->interp_process = vte_terminal_fork_command(term, args[0], args,
      NULL, dir, FALSE, FALSE, FALSE);
    thestory->handler_child_exit = g_signal_connect((gpointer)term,
      "child-exited", G_CALLBACK(on_interpreter_exit), 
      (gpointer)thestory);
    thestory->handler_commit = g_signal_connect((gpointer)term, "commit",
      G_CALLBACK(catch_input), (gpointer)thestory);
    
    /* Now the "Stop" option works */
    gtk_widget_set_sensitive(lookup_widget(thestory->window, "stop"), TRUE);
    gtk_widget_set_sensitive(lookup_widget(thestory->window, "stop_toolbutton"),
      TRUE);
    /* Set the focus to the terminal widget */
    gtk_widget_grab_focus(
      lookup_widget(thestory->window, right? "game_r" : "game_l"));
    
    g_strfreev(args);
    g_free(dir);
    
    /* Feed the commands up to the current pointer in the skein into the
    terminal */
    GSList *commands = get_nodes_to_here(thestory->theskein,
      thestory->skein_ptr);
    GSList *iter = g_slist_next(commands);
    while(iter != NULL) {
        vte_terminal_feed_child(term, ((struct node *)(iter->data))->command,
          strlen(((struct node *)(iter->data))->command));
        vte_terminal_feed_child(term, "\n", 1);
        iter = g_slist_next(iter);
    }
    g_slist_free(commands);
}

/* Kill the interpreter PID if it is running */
void stop_project(struct story *thestory) {
    if(thestory->interp_running)
        kill(thestory->interp_process, SIGTERM);
}
