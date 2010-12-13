#ifndef _DOCUMENT_PRIVATE_H_
#define _DOCUMENT_PRIVATE_H_

#include <glib.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksourcebuffer.h>
#include "document.h"

typedef struct {
	/* Action Groups */
	GtkActionGroup *document_action_group;
	GtkActionGroup *selection_action_group;
	GtkActionGroup *copy_action_group;
	GtkAccelGroup *accels;
	/* This document's filename */
    gchar *filename;
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
} I7DocumentPrivate;

#define I7_DOCUMENT_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), I7_TYPE_DOCUMENT, I7DocumentPrivate))
#define I7_DOCUMENT_USE_PRIVATE(o,n) I7DocumentPrivate *n = I7_DOCUMENT_PRIVATE(o)

#endif /* _DOCUMENT_PRIVATE_H_ */