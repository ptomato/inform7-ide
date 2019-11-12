/* Copyright (C) 2006-2009, 2010, 2014, 2018 P. F. Chimento
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

#include <glib.h>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include "panel.h"
#include "panel-private.h"

static void
history_free(I7PanelHistory *history)
{
	if(history->page)
		g_free(history->page);
	g_slice_free(I7PanelHistory, history);
}

void
history_free_queue(I7Panel *panel)
{
	I7_PANEL_USE_PRIVATE(panel, priv);

	g_queue_foreach(priv->history, (GFunc)history_free, NULL);
	g_queue_free(priv->history);
	priv->history = NULL;
	priv->current = 0;
}

static void
history_block_handlers(I7Panel *panel)
{
	g_signal_handlers_block_by_func(panel->notebook, after_notebook_switch_page, panel);
	g_signal_handlers_block_by_func(panel->tabs[I7_PANE_SOURCE], after_source_notebook_switch_page, panel);
	g_signal_handlers_block_by_func(panel->tabs[I7_PANE_RESULTS], after_results_notebook_switch_page, panel);
	g_signal_handlers_block_by_func(panel->tabs[I7_PANE_INDEX], after_index_notebook_switch_page, panel);
	g_signal_handlers_block_by_func(panel->tabs[I7_PANE_DOCUMENTATION], after_documentation_notify_uri, panel);
	g_signal_handlers_block_by_func(panel->tabs[I7_PANE_EXTENSIONS], after_extensions_notify_uri, panel);
}

static void
history_unblock_handlers(I7Panel *panel)
{
	g_signal_handlers_unblock_by_func(panel->notebook, after_notebook_switch_page, panel);
	g_signal_handlers_unblock_by_func(panel->tabs[I7_PANE_SOURCE], after_source_notebook_switch_page, panel);
	g_signal_handlers_unblock_by_func(panel->tabs[I7_PANE_RESULTS], after_results_notebook_switch_page, panel);
	g_signal_handlers_unblock_by_func(panel->tabs[I7_PANE_INDEX], after_index_notebook_switch_page, panel);
	g_signal_handlers_unblock_by_func(panel->tabs[I7_PANE_DOCUMENTATION], after_documentation_notify_uri, panel);
	g_signal_handlers_unblock_by_func(panel->tabs[I7_PANE_EXTENSIONS], after_extensions_notify_uri, panel);
}

/* Empty the list of pages to go forward to */
static void
history_empty_forward_queue(I7Panel *panel)
{
	I7_PANEL_USE_PRIVATE(panel, priv);

	/* Delete all the members of the queue before "current" */
	guint count;
	for(count = 0; count < priv->current; count++)
		history_free(g_queue_pop_head(priv->history));
	priv->current = 0;
	gtk_action_set_sensitive(gtk_action_group_get_action(priv->common_action_group, "forward"), FALSE);
}

/* Go to the location pointed to by the "current" history pointer */
void
history_goto_current(I7Panel *panel)
{
	I7_PANEL_USE_PRIVATE(panel, priv);
	I7PanelHistory *current = g_queue_peek_nth(priv->history, priv->current);

	history_block_handlers(panel);
	switch(current->pane) {
		case I7_PANE_SOURCE:
		case I7_PANE_RESULTS:
		case I7_PANE_INDEX:
			gtk_notebook_set_current_page(GTK_NOTEBOOK(panel->tabs[current->pane]), current->tab);
			break;
		case I7_PANE_DOCUMENTATION:
			webkit_web_view_load_uri(WEBKIT_WEB_VIEW(panel->tabs[I7_PANE_DOCUMENTATION]), current->page);
			/* Deprecated in 1.1.1 */
			break;
		case I7_PANE_EXTENSIONS:
			webkit_web_view_load_uri(WEBKIT_WEB_VIEW(panel->tabs[I7_PANE_EXTENSIONS]), current->page);
		default:
			;
	}
	gtk_notebook_set_current_page(GTK_NOTEBOOK(panel->notebook), current->pane);
	history_unblock_handlers(panel);
}

/* Empty the forward queue and push a new item to the front of the history */
static void
history_push_item(I7Panel *panel, I7PanelHistory *item)
{
	I7_PANEL_USE_PRIVATE(panel, priv);

	if(priv->current == g_queue_get_length(priv->history) - 1)
		gtk_action_set_sensitive(gtk_action_group_get_action(priv->common_action_group, "back"), TRUE);

	history_empty_forward_queue(panel);
	g_queue_push_head(priv->history, item);
	priv->current = 0;
}

/* Set a pane as the current location, and push the previous location
into the back queue */
void
history_push_pane(I7Panel *panel, I7PanelPane pane)
{
	I7PanelHistory *newitem = g_slice_new0(I7PanelHistory);
	newitem->pane = pane;
	history_push_item(panel, newitem);
}

/* Set a combination of pane and tab as the current location, and
push the previous location into the back queue */
void
history_push_tab(I7Panel *panel, I7PanelPane pane, guint tab)
{
	I7PanelHistory *newitem = g_slice_new0(I7PanelHistory);
	newitem->pane = pane;
	newitem->tab = tab;
	history_push_item(panel, newitem);
}

/* Set a documentation page as the current location, and push
the previous location into the back queue. If uri is NULL then it
gets the URI of the currently displayed page. */
void
history_push_docpage(I7Panel *panel, const gchar *uri)
{
	I7PanelHistory *newitem = g_slice_new0(I7PanelHistory);
	newitem->pane = I7_PANE_DOCUMENTATION;
	newitem->page = g_strdup(uri? uri : webkit_web_view_get_uri(WEBKIT_WEB_VIEW(panel->tabs[I7_PANE_DOCUMENTATION])));
	history_push_item(panel, newitem);
}

/* Set an extensions documentation page as the current location, and push the
previous location into the back queue. If @uri is NULL then it retrieves the
value for @uri from the currently displayed page. */
void
history_push_extensions_page(I7Panel *panel, const char *uri)
{
	I7PanelHistory *newitem = g_slice_new0(I7PanelHistory);
	newitem->pane = I7_PANE_EXTENSIONS;
	newitem->page = g_strdup((uri != NULL)? uri : webkit_web_view_get_uri(WEBKIT_WEB_VIEW(panel->tabs[I7_PANE_EXTENSIONS])));
	history_push_item(panel, newitem);
}
