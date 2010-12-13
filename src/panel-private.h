/***************************************************************************
 *            pane-private.h
 *
 *  Thu Sep 25 00:38:21 2008
 *  Copyright  2008  P. F. Chimento
 *  <philip.chimento@gmail.com>
 ****************************************************************************/

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */
 
#ifndef PANEL_PRIVATE_H
#define PANEL_PRIVATE_H

#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <webkit/webkit.h>
#include <JavaScriptCore/JavaScript.h>

#include "panel.h"

typedef struct {
	I7PanelPane pane;
	gint tab;
	gchar *page;
} I7PanelHistory;

typedef struct {
	/* JavaScript Project Class Type */
	JSClassRef js_class;
	/* Action Groups */
	GtkUIManager *ui_manager;
	GtkActionGroup *common_action_group;
	GtkActionGroup *skein_action_group;
	GtkActionGroup *transcript_action_group;
	GtkActionGroup *documentation_action_group;
	/* History list */
	GQueue *history; /* "front" is more recent, "back" is older */
	guint current;
	/* Webview settings */
	WebKitWebSettings *websettings;
} I7PanelPrivate;

#define I7_PANEL_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), I7_TYPE_PANEL, I7PanelPrivate))
#define I7_PANEL_USE_PRIVATE(o,n) I7PanelPrivate *n = I7_PANEL_PRIVATE(o)

/* Semi-private signal handlers */
void after_notebook_switch_page(GtkNotebook *notebook, GtkNotebookPage *page, guint page_num, I7Panel *panel);
void after_source_notebook_switch_page(GtkNotebook *notebook, GtkNotebookPage *page, guint page_num, I7Panel *panel);
void after_errors_notebook_switch_page(GtkNotebook *notebook, GtkNotebookPage *page, guint page_num, I7Panel *panel);
void after_index_notebook_switch_page(GtkNotebook *notebook, GtkNotebookPage *page, guint page_num, I7Panel *panel);
gint after_documentation_navigation_requested(WebKitWebView *webview, WebKitWebFrame *frame, WebKitNetworkRequest *request, I7Panel *panel);

#endif /* PANEL_PRIVATE_H */

 
