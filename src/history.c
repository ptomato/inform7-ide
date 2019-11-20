/* Copyright (C) 2006-2009, 2010, 2014, 2019 P. F. Chimento
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

#include "config.h"

#include <glib.h>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

#include "panel.h"

/* Forward declaration of signal handlers from I7Panel */
void after_notebook_switch_page(GtkNotebook *notebook, GtkWidget *page, unsigned page_num, I7Panel *self);
void after_source_notebook_switch_page(GtkNotebook *notebook, GtkWidget *page, unsigned page_num, I7Panel *self);
void after_results_notebook_switch_page(GtkNotebook *notebook, GtkWidget *page, unsigned page_num, I7Panel *self);
void after_index_notebook_switch_page(GtkNotebook *notebook, GtkWidget *page, unsigned page_num, I7Panel *self);
void after_documentation_notify_uri(WebKitWebView *webview, GParamSpec *pspec, I7Panel *self);
void after_extensions_notify_uri(WebKitWebView *webview, GParamSpec *pspec, I7Panel *self);

void
i7_panel_history_free(I7PanelHistory *self)
{
	g_clear_pointer(&self->page, g_free);
	g_slice_free(I7PanelHistory, self);
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

/* Go to the location pointed to by the "current" history pointer */
void
history_goto_current(I7Panel *panel)
{
	I7PanelHistory *current = i7_panel_get_current_history_item(panel);

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

/* Set a pane as the current location, and push the previous location
into the back queue */
void
history_push_pane(I7Panel *panel, I7PanelPane pane)
{
	I7PanelHistory *newitem = g_slice_new0(I7PanelHistory);
	newitem->pane = pane;
	i7_panel_push_history_item(panel, newitem);
}

/* Set a combination of pane and tab as the current location, and
push the previous location into the back queue */
void
history_push_tab(I7Panel *panel, I7PanelPane pane, guint tab)
{
	I7PanelHistory *newitem = g_slice_new0(I7PanelHistory);
	newitem->pane = pane;
	newitem->tab = tab;
	i7_panel_push_history_item(panel, newitem);
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
	i7_panel_push_history_item(panel, newitem);
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
	i7_panel_push_history_item(panel, newitem);
}

/* Signal handlers, connected in I7Panel */

void
after_notebook_switch_page(GtkNotebook *notebook, GtkWidget *page, unsigned page_num, I7Panel *self)
{
	switch(page_num) {
		case I7_PANE_SOURCE:
			history_push_tab(self, page_num,
				gtk_notebook_get_current_page(GTK_NOTEBOOK(self->tabs[I7_PANE_SOURCE])));
			break;
		case I7_PANE_RESULTS:
			history_push_tab(self, page_num,
				gtk_notebook_get_current_page(GTK_NOTEBOOK(self->tabs[I7_PANE_RESULTS])));
			break;
		case I7_PANE_INDEX:
			history_push_tab(self, page_num,
				gtk_notebook_get_current_page(GTK_NOTEBOOK(self->tabs[I7_PANE_INDEX])));
			break;
		case I7_PANE_DOCUMENTATION:
			history_push_docpage(self, NULL);
			break;
		case I7_PANE_EXTENSIONS:
			history_push_extensions_page(self, NULL);
			break;
		default:
			history_push_pane(self, page_num);
	}
}

void
after_results_notebook_switch_page(GtkNotebook *notebook, GtkWidget *page, unsigned page_num, I7Panel *self)
{
	if(gtk_notebook_get_current_page(GTK_NOTEBOOK(self->notebook)) == I7_PANE_RESULTS)
		history_push_tab(self, I7_PANE_RESULTS, page_num);
}

void
after_index_notebook_switch_page(GtkNotebook *notebook, GtkWidget *page, unsigned page_num, I7Panel *self)
{
	if(gtk_notebook_get_current_page(GTK_NOTEBOOK(self->notebook)) == I7_PANE_INDEX)
		history_push_tab(self, I7_PANE_INDEX, page_num);
}
