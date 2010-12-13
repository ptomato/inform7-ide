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

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <libchimara/chimara-glk.h>
#include <libchimara/chimara-if.h>
#include "story.h"
#include "story-private.h"
#include "error.h"
#include "node.h"
#include "skein.h"

void
i7_story_run_compiler_output(I7Story *story)
{
	I7_STORY_USE_PRIVATE(story, priv);
	GError *err = NULL;

	/* Rewind the Skein to the beginning */
	i7_skein_reset(priv->skein, TRUE);

    I7StoryPanel side = i7_story_choose_panel(story, I7_PANE_GAME);
	ChimaraIF *glk = CHIMARA_IF(story->panel[side]->tabs[I7_PANE_GAME]);
	    
    /* Load and start the interpreter */
	if(!chimara_if_run_game(glk, priv->compiler_output, &err)) {
		error_dialog(GTK_WINDOW(story), err, _("Could not load interpreter: "));
    }
	
	/* Display and set the focus to the interpreter */
	i7_story_show_pane(story, I7_PANE_GAME);
    gtk_widget_grab_focus(GTK_WIDGET(glk));
}

/* Compile finished action: run the output and feed it the command "test me" */
void
i7_story_test_compiler_output(I7Story *story)
{
	I7_STORY_USE_PRIVATE(story, priv);
	GError *err = NULL;

	I7StoryPanel side = i7_story_choose_panel(story, I7_PANE_GAME);
	ChimaraIF *glk = CHIMARA_IF(story->panel[side]->tabs[I7_PANE_GAME]);

	/* Load and start the interpreter */
	if(!chimara_if_run_game(glk, priv->compiler_output, &err)) {
		error_dialog(GTK_WINDOW(story), err, _("Could not load interpreter: "));
    }

	/* Tell the interpreter to "test me" */
    i7_skein_reset(priv->skein, TRUE);
    chimara_glk_feed_line_input(CHIMARA_GLK(glk), "test me");

	/* Display and set the focus to the interpreter */
	i7_story_show_pane(story, I7_PANE_GAME);
    gtk_widget_grab_focus(GTK_WIDGET(glk));
}

/* Finish setting up the interpreter when forced input is done processing */
static void
on_waiting(ChimaraGlk *glk)
{
	if(!chimara_glk_is_line_input_pending(glk)) {
		chimara_glk_set_interactive(glk, TRUE);
	
		/* Set focus to the interpreter */
		gtk_widget_grab_focus(GTK_WIDGET(glk));

		/* Disconnect this signal handler - have to do it this way, because
		 a gulong can't be packed into a pointer */
		gulong handler = g_signal_handler_find(glk, G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA, 
		    0, 0, NULL, on_waiting, NULL);
		g_signal_handler_disconnect(glk, handler);
	}
}

/* Force input (listed in @commands) to the interpreter. */
static void
play_commands(I7Story *story, GSList *commands, gboolean start_interpreter)
{
	I7_STORY_USE_PRIVATE(story, priv);
	GError *err = NULL;
	
	I7StoryPanel side = i7_story_choose_panel(story, I7_PANE_GAME);
	ChimaraIF *glk = CHIMARA_IF(story->panel[side]->tabs[I7_PANE_GAME]);

    /* Set non-interactive if there are commands, because we don't want to
	 scroll through screens full of -- more -- prompts */
    if(commands)
		chimara_glk_set_interactive(CHIMARA_GLK(glk), FALSE);

	/* Load and start the interpreter */
	if(start_interpreter) {
		if(!chimara_if_run_game(glk, priv->compiler_output, &err)) {
			error_dialog(GTK_WINDOW(story), err, _("Could not load interpreter: "));
		}
		i7_skein_reset(priv->skein, TRUE);
	}
	
	/* Display the interpreter */
	i7_story_show_pane(story, I7_PANE_GAME);

	/* Feed the commands up to the "played" pointer in the skein into the
    interpreter */
	GSList *iter;
	for(iter = commands; iter; iter = g_slist_next(iter))
		chimara_glk_feed_line_input(CHIMARA_GLK(glk), (gchar *)iter->data);

	/* Finish the rest when the input is done being processed */
	g_signal_connect(glk, "waiting", G_CALLBACK(on_waiting), NULL);
}

void
i7_story_run_compiler_output_and_replay(I7Story *story)
{
	I7_STORY_USE_PRIVATE(story, priv);

	/* Get a list of the commands that need to be fed in */
    GSList *commands = i7_skein_get_commands(priv->skein);
	play_commands(story, commands, TRUE);
	g_slist_foreach(commands, (GFunc)g_free, NULL);
	g_slist_free(commands);
}

void
i7_story_run_compiler_output_and_play_to_node(I7Story *story, I7Node *node)
{
	I7_STORY_USE_PRIVATE(story, priv);

	/* Get a list of the commands that need to be fed in */
    GSList *commands = i7_skein_get_commands_to_node(priv->skein, i7_skein_get_root_node(priv->skein), node);
	play_commands(story, commands, TRUE);
	g_slist_foreach(commands, (GFunc)g_free, NULL);
	g_slist_free(commands);
}

void
i7_story_run_commands_from_node(I7Story *story, I7Node *node)
{	
	I7_STORY_USE_PRIVATE(story, priv);

	I7Node *played = i7_skein_get_played_node(priv->skein);
	g_assert(g_node_is_ancestor(played->gnode, node->gnode));

	/* Get a list of the commands that need to be fed in */
    GSList *commands = i7_skein_get_commands_to_node(priv->skein, played, node);
	play_commands(story, commands, FALSE);
	g_slist_foreach(commands, (GFunc)g_free, NULL);
	g_slist_free(commands);
}

void
i7_story_run_compiler_output_and_entire_skein(I7Story *story)
{
}

static void
panel_stop_running_game(I7Story *story, I7Panel *panel)
{
	ChimaraGlk *glk = CHIMARA_GLK(panel->tabs[I7_PANE_GAME]);
	if(chimara_glk_get_running(glk)) {
		chimara_glk_stop(glk);
		chimara_glk_wait(glk); /* Seems to be necessary? */
	}
}

void
i7_story_stop_running_game(I7Story *story)
{
	i7_story_foreach_panel(story, (I7PanelForeachFunc)panel_stop_running_game, NULL);
}

gboolean 
i7_story_get_game_running(I7Story *story)
{
	return chimara_glk_get_running(CHIMARA_GLK(story->panel[LEFT]->tabs[I7_PANE_GAME]))
		|| chimara_glk_get_running(CHIMARA_GLK(story->panel[RIGHT]->tabs[I7_PANE_GAME]));
}

static void
panel_set_use_git(I7Story *story, I7Panel *panel, gpointer data)
{
	ChimaraIFFormat interpreter = GPOINTER_TO_INT(data)? CHIMARA_IF_INTERPRETER_GIT : CHIMARA_IF_INTERPRETER_GLULXE;
	ChimaraIF *glk = CHIMARA_IF(panel->tabs[I7_PANE_GAME]);
	chimara_if_set_preferred_interpreter(glk, CHIMARA_IF_FORMAT_GLULX, interpreter);
}

void 
i7_story_set_use_git(I7Story *story, gboolean use_git)
{
	i7_story_foreach_panel(story, (I7PanelForeachFunc)panel_set_use_git, GINT_TO_POINTER(use_git));
}

/* Blorb resource load callback */
gchar *
load_blorb_resource(ChimaraResourceType usage, guint32 resnum, I7Story *story)
{
	I7_STORY_USE_PRIVATE(story, priv);
	g_return_val_if_fail(priv->manifest, NULL);
	
	/* Look up the filename in the manifest */
	gchar *resstring = g_strdup_printf("%d", resnum);
	const gchar *restype;
	switch(usage) {
		case CHIMARA_RESOURCE_SOUND:
			restype = "Sounds";
			break;
		case CHIMARA_RESOURCE_IMAGE:
		default:
			restype = "Graphics";
			break;
	}
	PlistObject *manifest_entry = plist_object_lookup(priv->manifest, restype, resstring, -1);
	g_return_val_if_fail(manifest_entry, NULL);
	g_free(resstring);

	/* Build the full path */
	gchar *filename = g_strdup(manifest_entry->string.val);
	gchar *materials = i7_story_get_materials_path(story);
	gchar *fullpath = g_build_filename(materials, filename, NULL);
	g_free(filename);
	g_free(materials);

	return fullpath;
}

/* SIGNAL HANDLERS */

/* Set the "stop" action to be sensitive when the game starts */
void
on_game_started(ChimaraGlk *game, I7Story *story)
{
	I7_STORY_USE_PRIVATE(story, priv);
	GtkAction *stop = gtk_action_group_get_action(priv->story_action_group, "stop");
	gtk_action_set_sensitive(stop, TRUE);
}

/* Set the "stop" action to be insensitive when the game finishes */
void
on_game_stopped(ChimaraGlk *game, I7Story *story)
{
	I7_STORY_USE_PRIVATE(story, priv);
	GtkAction *stop = gtk_action_group_get_action(priv->story_action_group, "stop");
	gtk_action_set_sensitive(stop, FALSE);
}

/* Grab commands entered by the user and store them in the skein */
void
on_game_command(ChimaraIF *game, gchar *input, gchar *response, I7Story *story)
{
	I7_STORY_USE_PRIVATE(story, priv);
	if(!input) {
		/* If no input, then this was the text printed before the first prompt.
		 It should become the transcript text of the root node. */
		I7Node *root = i7_skein_get_root_node(priv->skein);
		g_assert(i7_skein_get_current_node(priv->skein) == root);
		i7_node_set_transcript_text(root, response);
	} else {
		I7Node *node = i7_skein_new_command(priv->skein, input);
		i7_node_set_transcript_text(node, response);
	}
}
