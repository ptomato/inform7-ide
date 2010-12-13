#ifndef HYPERLINK_H
#define HYPERLINK_H

#include <glib.h>
#include <gtk/gtk.h>

#include "glk.h"
#include "window.h"
#include "event.h"

struct hyperlink {
	guint32 value;
	GtkTextTag *tag;
	gulong event_handler;
	winid_t window;
};
typedef struct hyperlink hyperlink_t;

G_GNUC_INTERNAL gboolean on_hyperlink_clicked(GtkTextTag *tag, GObject *object, GdkEvent *event, GtkTextIter *iter, hyperlink_t *link);

#endif
