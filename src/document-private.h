/*  Copyright (C) 2008, 2009, 2010, 2012 P. F. Chimento
 *  This file is part of GNOME Inform 7.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _DOCUMENT_PRIVATE_H_
#define _DOCUMENT_PRIVATE_H_

#include <glib.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksourcebuffer.h>
#include <gio/gio.h>
#include "document.h"

typedef struct {
	/* Action Groups */
	GtkActionGroup *document_action_group;
	GtkActionGroup *selection_action_group;
	GtkActionGroup *copy_action_group;
	GtkAccelGroup *accels;
	/* The file this document refers to */
	GFile *file;
	/* Whether it was modified since the last save*/
	gboolean modified;
	/* File monitor */
	GFileMonitor *monitor;
	/* The program code */
	GtkSourceBuffer *buffer;
	GtkTextTag *invisible_tag;
	/* The tree of section headings */
	I7Heading heading_depth;
	GtkTreeStore *headings;
	GtkTreeModel *filter;
	GtkTreePath *current_heading;
	/* The view with a search match currently being highlighted */
	GtkWidget *highlighted_view;

	/* Download counts */
	unsigned downloads_completed;
	unsigned downloads_total;
	unsigned downloads_failed;
	GCancellable *cancel_download;
} I7DocumentPrivate;

#define I7_DOCUMENT_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), I7_TYPE_DOCUMENT, I7DocumentPrivate))
#define I7_DOCUMENT_USE_PRIVATE(o,n) I7DocumentPrivate *n = I7_DOCUMENT_PRIVATE(o)

#endif /* _DOCUMENT_PRIVATE_H_ */
