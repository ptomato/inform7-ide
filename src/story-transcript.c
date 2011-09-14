/* Copyright (C) 2011 P. F. Chimento
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

#include <gtk/gtk.h>
#include "node.h"
#include "panel.h"
#include "skein.h"

void
on_transcript_size_allocate(GtkTreeView *view, GtkAllocation *allocation, I7Panel *panel)
{
	unsigned default_width;
	g_object_get(panel->transcript_cell, "default-width", &default_width, NULL);
	if(default_width != allocation->width) {
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

	gtk_menu_popup(GTK_MENU(panel->transcript_menu), NULL, NULL, NULL, NULL, 3, event->time);

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

	gtk_tree_selection_get_selected(selection, &skein, &iter);
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