/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Philip Chimento <philip.chimento@gmail.com>
 */

#include "config.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

#include "html.h"
#include "panel.h"
#include "story.h"

typedef struct {
	I7Story *story;
	GQueue *q;  /* element type GFile */
	I7PaneIndexTab ix;
} IndexLoadClosure;

static void
on_index_file_load_finish(GFile *file, GAsyncResult *res, IndexLoadClosure *data)
{
	g_autofree char *contents = NULL;
	g_autoptr(GError) err = NULL;
	if (!g_file_load_contents_finish(file, res, &contents, /* length = */ NULL, /* etag = */ NULL, &err)) {
		if (!g_error_matches(err, G_IO_ERROR, G_IO_ERROR_NOT_FOUND))
			g_critical("Index page %u not loaded for other reason than not found: %s", data->ix, err->message);

		html_load_blank(WEBKIT_WEB_VIEW(data->story->panel[LEFT]->index_tabs[data->ix]));
		html_load_blank(WEBKIT_WEB_VIEW(data->story->panel[RIGHT]->index_tabs[data->ix]));
	} else {
		g_autofree char *base_url = g_file_get_uri(file);
		webkit_web_view_load_html(WEBKIT_WEB_VIEW(data->story->panel[LEFT]->index_tabs[data->ix]), contents, base_url);
		webkit_web_view_load_html(WEBKIT_WEB_VIEW(data->story->panel[RIGHT]->index_tabs[data->ix]), contents, base_url);
	}

	g_debug("Index load: loaded page %u", data->ix);

	g_autoptr(GFile) next_file = g_queue_pop_head(data->q);
	if (next_file) {
		data->ix++;
		g_file_load_contents_async(next_file, /* cancellable = */ NULL,
			(GAsyncReadyCallback)on_index_file_load_finish, data);
		return;
	}

	g_debug("Index load: finished loading pages");

	g_object_unref(data->story);
	g_queue_free(data->q);
	g_free(data);
}

/* Idle function to load the index pages. This is done in idle time because the
 * index isn't what a user sees immediately when the app starts. */
static gboolean
check_and_load_idle(I7Story *story)
{
	g_autoptr(GFile) parent = i7_document_get_file(I7_DOCUMENT(story));
	g_autoptr(GFile) index_dir = g_file_get_child(parent, "Index");

	IndexLoadClosure *data = g_new0(IndexLoadClosure, 1);
	data->story = g_object_ref(story);
	data->q = g_queue_new();
	data->ix = 0;

	for (I7PaneIndexTab counter = 0; counter < I7_INDEX_NUM_TABS; counter++) {
		GFile* index_file = g_file_get_child(index_dir, i7_panel_index_names[counter]);
		g_queue_push_tail(data->q, index_file);
	}

	g_autoptr(GFile) index_file = g_queue_pop_head(data->q);
	g_file_load_contents_async(index_file, /* cancellable = */ NULL,
		(GAsyncReadyCallback)on_index_file_load_finish, data);

	return G_SOURCE_REMOVE;
}

/* Load all the correct files in the index tabs, if they exist */
void
i7_story_reload_index_tabs(I7Story *story)
{
	g_idle_add((GSourceFunc)check_and_load_idle, story);
}
