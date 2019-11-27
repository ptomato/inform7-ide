/* Copyright (C) 2011, 2015, 2019 P. F. Chimento
 * This file is part of GNOME Inform 7.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <gtk/gtk.h>

#include "node.h"
#include "panel.h"
#include "skein.h"
#include "story.h"

void
on_transcript_size_allocate(GtkTreeView *view, GtkAllocation *allocation, I7Panel *panel)
{
	unsigned default_width;
	g_object_get(panel->transcript_cell, "default-width", &default_width, NULL);
	if(allocation->width >= 0 && default_width != (unsigned)allocation->width) {
		g_object_set(panel->transcript_cell, "default-width", allocation->width, NULL);
		gtk_tree_view_column_queue_resize(panel->transcript_column);
	}
}

/* Pop up a context menu when right-clicking on the Transcript */
gboolean
on_transcript_button_press(GtkTreeView *view, GdkEventButton *event, I7Panel *panel)
{
	/* Only react to right-clicks */
	if(event->type != GDK_BUTTON_PRESS)
		return FALSE; /* didn't handle event */
	if(event->button != 3)
		return FALSE; /* propagate event */

	gtk_menu_popup_at_pointer(GTK_MENU(panel->transcript_menu), (GdkEvent *)event);

	/* Then send it through as if it was a left-button click, so the row gets selected */
	event->button = 1;
	return FALSE;
}

/* Get this panel's currently selected transcript node. Unref when done. */
static I7Node *
get_selected_node(I7Panel *panel)
{
	GtkTreeView *transcript = GTK_TREE_VIEW(panel->tabs[I7_PANE_TRANSCRIPT]);
	GtkTreeSelection *selection = gtk_tree_view_get_selection(transcript);
	GtkTreeModel *skein;
	GtkTreeIter iter;
	I7Node *node;

	if(!gtk_tree_selection_get_selected(selection, &skein, &iter))
		return NULL;
	gtk_tree_model_get(skein, &iter, I7_SKEIN_COLUMN_NODE_PTR, &node, -1);

	return node;
}

void
on_transcript_menu_bless(GtkMenuItem *item, I7Panel *panel)
{
	I7Node *node = get_selected_node(panel);
	i7_node_bless(node);
	g_object_unref(node);
}

void
on_transcript_menu_play_to_here(GtkMenuItem *item, I7Panel *panel)
{
	GtkTreeView *transcript = GTK_TREE_VIEW(panel->tabs[I7_PANE_TRANSCRIPT]);
	GtkTreeModel *skein = gtk_tree_view_get_model(transcript);
	I7Node *node = get_selected_node(panel);

	g_signal_emit_by_name(skein, "node-activate", node);
	g_object_unref(node);
}

void
on_transcript_menu_show_knot(GtkMenuItem *item, I7Panel *panel)
{
	GtkTreeView *transcript = GTK_TREE_VIEW(panel->tabs[I7_PANE_TRANSCRIPT]);
	GtkTreeModel *skein = gtk_tree_view_get_model(transcript);
	I7Node *node = get_selected_node(panel);

	g_signal_emit_by_name(skein, "show-node", I7_REASON_TRANSCRIPT, node);
	g_object_unref(node);
}

/*
 * display_and_return_transcript:
 * @story: the story
 * 
 * Internal function. Picks a panel to show the Transcript pane in, & shows it.
 * Returns: a pointer to the Transcript tree view.
 */
static GtkTreeView *
display_and_return_transcript(I7Story *story)
{
	I7StoryPanel side = i7_story_choose_panel(story, I7_PANE_TRANSCRIPT);
	I7Panel *panel = story->panel[side];
	i7_story_show_pane(story, I7_PANE_TRANSCRIPT);
	return GTK_TREE_VIEW(panel->tabs[I7_PANE_TRANSCRIPT]);
}

/* Hard to believe this function was only added in GTK 3.0 FIXME */
static gboolean
_gtk_tree_model_iter_previous(GtkTreeModel *model, GtkTreeIter *iter)
{
	GtkTreePath *path = gtk_tree_model_get_path(model, iter);
	gboolean retval = gtk_tree_path_prev(path);
	g_assert(gtk_tree_model_get_iter(model, iter, path));
	gtk_tree_path_free(path);
	return retval;
}

/*
 * i7_story_previous_changed:
 * @story: the story
 *
 * Moves the current selection in the Transcript panel to the previous node with
 * "changed" status, whose transcript text differs from the last run. If the
 * Transcript panel is not currently displayed, displays it. If there is no
 * current selection, displays the next changed node starting from the first
 * node.
 */
void
i7_story_previous_changed(I7Story *story)
{
	GtkTreeView *transcript = display_and_return_transcript(story);
	GtkTreeSelection *selection = gtk_tree_view_get_selection(transcript);
	GtkTreeModel *skein;
	GtkTreeIter iter;

	if(!gtk_tree_selection_get_selected(selection, &skein, &iter)) {
		/* Start at the top if no selected item */
		i7_story_next_changed(story);
		return;
	}

	/* Find the previous item */
	gboolean found = FALSE;
	while(!found && _gtk_tree_model_iter_previous(skein, &iter)) {
		I7Node *node = NULL;
		gtk_tree_model_get(skein, &iter, I7_SKEIN_COLUMN_NODE_PTR, &node, -1);
		if(i7_node_get_changed(node))
			found = TRUE;
		g_object_unref(node);
	}

	if(!found) {
		/* No previous item */
		gdk_window_beep(gtk_widget_get_window(GTK_WIDGET(story)));
		return;
	}

	/* Move to the previous item */
	gtk_tree_selection_select_iter(selection, &iter);
	GtkTreePath *path = gtk_tree_model_get_path(skein, &iter);
	gtk_tree_view_scroll_to_cell(transcript, path, NULL, FALSE, 0.0, 0.0);

	gtk_tree_path_free(path);
}

/*
 * i7_story_next_changed:
 * @story: the story
 *
 * Moves the current selection in the Transcript panel to the next node with
 * "changed" status, whose transcript text differs from the last run. If the
 * Transcript panel is not currently displayed, displays it. If there is no
 * current selection, starts at the first node.
 */
void
i7_story_next_changed(I7Story *story)
{
	GtkTreeView *transcript = display_and_return_transcript(story);
	GtkTreeSelection *selection = gtk_tree_view_get_selection(transcript);
	GtkTreeModel *skein;
	GtkTreeIter iter;
	gboolean found = FALSE;

	if(!gtk_tree_selection_get_selected(selection, &skein, &iter)) {
		/* Start at the top if no selected item */
		g_assert(gtk_tree_model_get_iter_first(skein, &iter));

		/* If the top node is changed, just use it */
		found = i7_node_get_changed(i7_skein_get_root_node(I7_SKEIN(skein)));
	}

	/* Find the next item */
	while(!found && gtk_tree_model_iter_next(skein, &iter)) {
		I7Node *node = NULL;
		gtk_tree_model_get(skein, &iter, I7_SKEIN_COLUMN_NODE_PTR, &node, -1);
		if(i7_node_get_changed(node))
			found = TRUE;
		g_object_unref(node);
	}

	if(!found) {
		/* No next item */
		gdk_window_beep(gtk_widget_get_window(GTK_WIDGET(story)));
		return;
	}

	/* Move to the next item */
	gtk_tree_selection_select_iter(selection, &iter);
	GtkTreePath *path = gtk_tree_model_get_path(skein, &iter);
	gtk_tree_view_scroll_to_cell(transcript, path, NULL, FALSE, 0.0, 0.0);

	gtk_tree_path_free(path);
}

/*
 * i7_story_previous_difference:
 * @story: the story
 * 
 * Moves the current selection in the Transcript panel to the previous blessed
 * node that is different from its expected text. If the Transcript panel is not
 * currently displayed, displays it. If there is no current selection, displays
 * the next blessed node starting from the first node.
 */
void
i7_story_previous_difference(I7Story *story)
{
	GtkTreeView *transcript = display_and_return_transcript(story);
	GtkTreeSelection *selection = gtk_tree_view_get_selection(transcript);
	GtkTreeModel *skein;
	GtkTreeIter iter;

	if(!gtk_tree_selection_get_selected(selection, &skein, &iter)) {
		/* Start at the top if no selected item */
		i7_story_next_difference(story);
		return;
	}

	/* Find the previous item */
	gboolean found = FALSE;
	while(!found && _gtk_tree_model_iter_previous(skein, &iter)) {
		I7Node *node = NULL;
		gtk_tree_model_get(skein, &iter, I7_SKEIN_COLUMN_NODE_PTR, &node, -1);
		if(i7_node_get_blessed(node) && i7_node_get_different(node))
			found = TRUE;
		g_object_unref(node);
	}
	
	if(!found) {
		/* No previous item */
		gdk_window_beep(gtk_widget_get_window(GTK_WIDGET(story)));
		return;
	}
	
	/* Move to the previous item */
	gtk_tree_selection_select_iter(selection, &iter);
	GtkTreePath *path = gtk_tree_model_get_path(skein, &iter);
	gtk_tree_view_scroll_to_cell(transcript, path, NULL, FALSE, 0.0, 0.0);

	gtk_tree_path_free(path);
}

/*
 * i7_story_next_difference:
 * @story: the story
 * 
 * Moves the current selection in the Transcript panel to the next blessed node
 * that is different from its expected text. If the Transcript panel is not
 * currently displayed, displays it. If there is no current selection, starts at
 * the first node.
 */
void
i7_story_next_difference(I7Story *story)
{
	GtkTreeView *transcript = display_and_return_transcript(story);
	GtkTreeSelection *selection = gtk_tree_view_get_selection(transcript);
	GtkTreeModel *skein;
	GtkTreeIter iter;
	gboolean found = FALSE;

	if(!gtk_tree_selection_get_selected(selection, &skein, &iter)) {
		/* Start at the top if no selected item */
		g_assert(gtk_tree_model_get_iter_first(skein, &iter));

		/* If the top node is a difference, just use it */
		I7Node *root = i7_skein_get_root_node(I7_SKEIN(skein));
		found = (i7_node_get_blessed(root) && i7_node_get_different(root));
	}

	/* Find the next item */
	while(!found && gtk_tree_model_iter_next(skein, &iter)) {
		I7Node *node = NULL;
		gtk_tree_model_get(skein, &iter, I7_SKEIN_COLUMN_NODE_PTR, &node, -1);
		if(i7_node_get_blessed(node) && i7_node_get_different(node))
			found = TRUE;
		g_object_unref(node);
	}
	
	if(!found) {
		/* No next item */
		gdk_window_beep(gtk_widget_get_window(GTK_WIDGET(story)));
		return;
	}
	
	/* Move to the next item */
	gtk_tree_selection_select_iter(selection, &iter);
	GtkTreePath *path = gtk_tree_model_get_path(skein, &iter);
	gtk_tree_view_scroll_to_cell(transcript, path, NULL, FALSE, 0.0, 0.0);

	gtk_tree_path_free(path);
}

/*
 * i7_story_next_difference_skein:
 * @story: the story
 *
 * Finds the next knot with a difference between the transcript and expected
 * text that is below the currently selected knot in the Transcript pane, and
 * scrolls all Transcript and Skein views to it. If no suitable knot below the
 * one that is selected, moves to the next branch to the right in the Skein. If
 * no suitable knot there, starts from the root of the Skein.
 */
void
i7_story_next_difference_skein(I7Story *story)
{
	GtkTreeView *transcript = display_and_return_transcript(story);
	GtkTreeSelection *selection = gtk_tree_view_get_selection(transcript);
	GtkTreeModel *skein;
	GtkTreeIter iter;
	I7Node *current_node;

	if(!gtk_tree_selection_get_selected(selection, &skein, &iter)) {
		/* Start at the top if no selected item */
		g_assert(gtk_tree_model_get_iter_first(skein, &iter));
	}

	gtk_tree_model_get(skein, &iter, I7_SKEIN_COLUMN_NODE_PTR, &current_node, -1);

	/* Find the next item */
	I7Node *next_node = i7_node_get_next_difference(current_node);
	if(!next_node)
		next_node = i7_node_get_next_difference(i7_skein_get_root_node(I7_SKEIN(skein)));
	if(!next_node) {
		/* No next item */
		gdk_window_beep(gtk_widget_get_window(GTK_WIDGET(story)));
		return;
	}
	g_object_unref(current_node);

	/* display item in skein and transcript */
	i7_story_show_node_in_transcript(story, next_node);
	g_signal_emit_by_name(skein, "show-node", I7_REASON_TRANSCRIPT, next_node);
}

/*
 * i7_story_show_node_in_transcript:
 * @story: the story
 * @node: the node to show
 *
 * Sets the current thread of the Transcript to @node, then selects and scrolls
 * to @node in both Transcript panes. This invalidates any outstanding iterators
 * on the Skein tree model.
 */
void
i7_story_show_node_in_transcript(I7Story *story, I7Node *node)
{
	GtkTreeModel *skein = GTK_TREE_MODEL(i7_story_get_skein(story));
	GtkTreeIter iter;

	/* display transcript to item, invalidates iters */
	i7_skein_set_current_node(I7_SKEIN(skein), node);

	/* Get an iterator to the item */
	gboolean found = FALSE;
	g_assert(gtk_tree_model_get_iter_first(skein, &iter));
	do {
		I7Node *iter_node = NULL;
		gtk_tree_model_get(skein, &iter, I7_SKEIN_COLUMN_NODE_PTR, &iter_node, -1);
		if(iter_node == node)
			found = TRUE;
		g_object_unref(iter_node);
	} while(!found && gtk_tree_model_iter_next(skein, &iter));
	/* Get a path to the iterator */
	GtkTreePath *path = gtk_tree_model_get_path(skein, &iter);

	I7StoryPanel side;
	for(side = LEFT; side < I7_STORY_NUM_PANELS; side++) {
	    GtkTreeView *transcript = GTK_TREE_VIEW(story->panel[side]->tabs[I7_PANE_TRANSCRIPT]);

		/* Scroll to item */
		gtk_tree_view_scroll_to_cell(transcript, path, NULL, FALSE, 0.0, 0.0);

		/* Select item */
	    GtkTreeSelection *selection = gtk_tree_view_get_selection(transcript);
		gtk_tree_selection_select_path(selection, path);
    }

	gtk_tree_path_free(path);
}
