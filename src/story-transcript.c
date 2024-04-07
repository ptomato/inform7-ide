/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2011, 2015, 2019, 2022 Philip Chimento <philip.chimento@gmail.com>
 */

#include "config.h"

#include <stdbool.h>

#include <gtk/gtk.h>

#include "node.h"
#include "panel.h"
#include "skein.h"
#include "story.h"

/* Helper function: the two Transcript panes might have different selections
 * showing, so pick the one most likely to be shown by scroll_to_index() and
 * return the selected row from that one.
 */
static GtkListBoxRow *
get_selected_transcript_row(I7Story *story)
{
	I7StoryPanel side = i7_story_choose_panel(story, I7_PANE_TRANSCRIPT);
	GtkListBox *transcript = GTK_LIST_BOX(story->panel[side]->tabs[I7_PANE_TRANSCRIPT]);
	return gtk_list_box_get_selected_row(transcript);
}

static void
perform_scroll_async(GtkListBox *transcript, GdkRectangle *ignored, GtkWidget *row)
{
	gtk_widget_grab_focus(row);
	g_signal_handlers_disconnect_by_func(transcript, perform_scroll_async, row);
}

/* Helper function: Picks a panel to show the Transcript pane in, & shows it.
 * Attempts to scroll to the row at @pos. This may happen async.
 */
static void
scroll_to_index(I7Story *story, unsigned pos)
{
	I7StoryPanel side = i7_story_choose_panel(story, I7_PANE_TRANSCRIPT);
	I7Panel *panel = story->panel[side];

	bool must_scroll_async = false;
	if (gtk_notebook_get_current_page(GTK_NOTEBOOK(panel->notebook)) != I7_PANE_TRANSCRIPT) {
		i7_story_show_pane(story, I7_PANE_TRANSCRIPT);
		must_scroll_async = true;
	}

	GtkListBox *transcript = GTK_LIST_BOX(panel->tabs[I7_PANE_TRANSCRIPT]);

	GtkListBoxRow *row = gtk_list_box_get_row_at_index(transcript, pos);
	gtk_list_box_select_row(transcript, row);
	gtk_widget_grab_focus(GTK_WIDGET(row));  /* scrolls if transcript is visible */

	if (must_scroll_async) {
		gtk_widget_queue_allocate(GTK_WIDGET(transcript));
		g_signal_connect_after(transcript, "size-allocate", G_CALLBACK(perform_scroll_async), row);
	}
}

typedef struct _ScrollAfterUpdateClosure {
	I7Story *story;
	GListModel *skein;
	I7Node *target_node;
} ScrollAfterUpdateClosure;

static gboolean
scroll_after_list_update(ScrollAfterUpdateClosure *data)
{
	/* Get the index of the item */
	bool found = false;
	unsigned pos = 0;
	unsigned n_items = g_list_model_get_n_items(data->skein);
	do {
		g_autoptr(I7Node) iter_node = g_list_model_get_item(data->skein, pos);
		if (iter_node == data->target_node)
			found = true;
	} while (!found && ++pos < n_items);

	/* Select and scroll to item */
	scroll_to_index(data->story, pos);

	g_free(data);
	return G_SOURCE_REMOVE;
}

/* Helper function: call this when intending to scroll to a particular row after
 * invalidating the Skein list model.
 */
static void
wait_for_items_changed(GListModel *skein, unsigned pos, unsigned removed, unsigned added, ScrollAfterUpdateClosure *data)
{
	g_idle_add((GSourceFunc)scroll_after_list_update, data);
	g_signal_handlers_disconnect_by_func(skein, wait_for_items_changed, data);
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
	GtkListBoxRow *selection = get_selected_transcript_row(story);
	GListModel *skein = G_LIST_MODEL(i7_story_get_skein(story));

	if (!selection) {
		/* Start at the top if no selected item */
		i7_story_next_changed(story);
		return;
	}

	/* Find the previous item */
	gboolean found = FALSE;
	unsigned pos = gtk_list_box_row_get_index(selection);
	while (!found && pos-- != 0) {
		g_autoptr(I7Node) node = g_list_model_get_item(skein, pos);
		if(i7_node_get_changed(node))
			found = TRUE;
	}

	if(!found) {
		/* No previous item */
		gdk_window_beep(gtk_widget_get_window(GTK_WIDGET(story)));
		return;
	}

	/* Move to the previous item */
	scroll_to_index(story, pos);
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
	GtkListBoxRow *selection = get_selected_transcript_row(story);
	GListModel *skein = G_LIST_MODEL(i7_story_get_skein(story));
	gboolean found = FALSE;

	unsigned pos;
	if (selection) {
		pos = gtk_list_box_row_get_index(selection);
	} else {
		/* Start at the top if no selected item */
		pos = 0;

		/* If the top node is changed, just use it */
		found = i7_node_get_changed(i7_skein_get_root_node(I7_SKEIN(skein)));
	}

	/* Find the next item */
	unsigned n_items = g_list_model_get_n_items(skein);
	while (!found && ++pos < n_items) {
		g_autoptr(I7Node) node = g_list_model_get_item(skein, pos);
		if(i7_node_get_changed(node))
			found = TRUE;
	}

	if(!found) {
		/* No next item */
		gdk_window_beep(gtk_widget_get_window(GTK_WIDGET(story)));
		return;
	}

	/* Move to the next item */
	scroll_to_index(story, pos);
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
	GtkListBoxRow *selection = get_selected_transcript_row(story);
	GListModel *skein = G_LIST_MODEL(i7_story_get_skein(story));

	if (!selection) {
		/* Start at the top if no selected item */
		i7_story_next_difference(story);
		return;
	}

	/* Find the previous item */
	gboolean found = FALSE;
	unsigned pos = gtk_list_box_row_get_index(selection);
	while (!found && pos-- != 0) {
		g_autoptr(I7Node) node = g_list_model_get_item(skein, pos);
		if(i7_node_get_blessed(node) && i7_node_get_different(node))
			found = TRUE;
	}

	if(!found) {
		/* No previous item */
		gdk_window_beep(gtk_widget_get_window(GTK_WIDGET(story)));
		return;
	}

	/* Move to the previous item */
	scroll_to_index(story, pos);
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
	GtkListBoxRow *selection = get_selected_transcript_row(story);
	GListModel *skein = G_LIST_MODEL(i7_story_get_skein(story));
	gboolean found = FALSE;

	unsigned pos;
	if (selection) {
		pos = gtk_list_box_row_get_index(selection);
	} else {
		/* Start at the top if no selected item */
		pos = 0;

		/* If the top node is a difference, just use it */
		I7Node *root = i7_skein_get_root_node(I7_SKEIN(skein));
		found = (i7_node_get_blessed(root) && i7_node_get_different(root));
	}

	/* Find the next item */
	unsigned n_items = g_list_model_get_n_items(skein);
	while (!found && ++pos < n_items) {
		g_autoptr(I7Node) node = g_list_model_get_item(skein, pos);
		if(i7_node_get_blessed(node) && i7_node_get_different(node))
			found = TRUE;
	}

	if(!found) {
		/* No next item */
		gdk_window_beep(gtk_widget_get_window(GTK_WIDGET(story)));
		return;
	}

	/* Move to the next item */
	scroll_to_index(story, pos);
}

/*
 * i7_story_next_difference_skein:
 * @story: the story
 *
 * Finds the next knot with a difference between the transcript and expected
 * text that is below the currently selected knot in the Transcript pane, and
 * scrolls the Transcript and Skein views to it. If no suitable knot below the
 * one that is selected, moves to the next branch to the right in the Skein. If
 * no suitable knot there, starts from the root of the Skein.
 */
void
i7_story_next_difference_skein(I7Story *story)
{
	GtkListBoxRow *selection = get_selected_transcript_row(story);
	GListModel *skein = G_LIST_MODEL(i7_story_get_skein(story));

	/* Start at the top if no selected item */
	unsigned pos = selection? gtk_list_box_row_get_index(selection) : 0;

	g_autoptr(I7Node) current_node = g_list_model_get_item(skein, pos);

	/* Find the next item */
	I7Node *next_node = i7_node_get_next_difference(current_node);
	if(!next_node)
		next_node = i7_node_get_next_difference(i7_skein_get_root_node(I7_SKEIN(skein)));
	if(!next_node) {
		/* No next item */
		gdk_window_beep(gtk_widget_get_window(GTK_WIDGET(story)));
		return;
	}

	/* display item in skein and transcript */
	i7_story_show_node_in_transcript(story, next_node);
	g_signal_emit_by_name(skein, "show-node", I7_REASON_TRANSCRIPT, next_node);
}

/*
 * i7_story_show_node_in_transcript:
 * @story: the story
 * @node: the node to show
 *
 * Sets the current thread of the Transcript to @node, displays one Transcript
 * pane as appropriate, selects @node, and scrolls to it. This may recalculate
 * the Skein list model.
 */
void
i7_story_show_node_in_transcript(I7Story *story, I7Node *node)
{
	GListModel *skein = G_LIST_MODEL(i7_story_get_skein(story));

	if (!i7_skein_is_node_in_current_thread(I7_SKEIN(skein), node)) {
		ScrollAfterUpdateClosure *data = g_new0(ScrollAfterUpdateClosure, 1);
		data->story = story;
		data->skein = skein;
		data->target_node = node;
		g_signal_connect(skein, "items-changed", G_CALLBACK(wait_for_items_changed), data);
		i7_skein_set_current_node(I7_SKEIN(skein), node);
		return;
	}

	/* Get the index of the item */
	gboolean found = FALSE;
	unsigned pos = 0;
	unsigned n_items = g_list_model_get_n_items(skein);
	do {
		g_autoptr(I7Node) iter_node = g_list_model_get_item(skein, pos);
		if(iter_node == node)
			found = TRUE;
	} while (!found && ++pos < n_items);

	/* Select and scroll to item */
	scroll_to_index(story, pos);
}
