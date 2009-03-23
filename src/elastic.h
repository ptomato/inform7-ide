#ifndef __ELASTIC_H__
#define __ELASTIC_H__

#include <gtk/gtk.h>
#include "story.h"

void elastic_remove(GtkSourceBuffer *textbuffer);
void elastic_refresh(GtkTextBuffer *textbuffer, GtkTextView *view);
gboolean elastic_insert_text(GtkTextBuffer *textbuffer, GtkTextIter *location,
							 gchar *text, gint len, GtkTextView *view);
gboolean elastic_delete_range(GtkTextBuffer *textbuffer, GtkTextIter *start, 
							  GtkTextIter *end, GtkTextView *view);
void elastic_setup(GtkTextBuffer *textbuffer, GtkTextView *view);

#endif /* __ELASTIC_H__ */

