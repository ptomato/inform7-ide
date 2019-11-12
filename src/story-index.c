/* Copyright (C) 2006-2009, 2010, 2011, 2012, 2014, 2018 P. F. Chimento
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
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include "story.h"
#include "story-private.h"
#include "html.h"
#include "panel.h"

static void
load_index_file(I7Story *story, int counter)
{
	GFile *parent = i7_document_get_file(I7_DOCUMENT(story));
	GFile *child1 = g_file_get_child(parent, "Index");
	GFile *file = g_file_get_child(child1, i7_panel_index_names[counter]);
	g_object_unref(parent);
	g_object_unref(child1);

	if(g_file_query_exists(file, NULL)) {
		html_load_file(WEBKIT_WEB_VIEW(story->panel[LEFT]->index_tabs[counter]), file);
		html_load_file(WEBKIT_WEB_VIEW(story->panel[RIGHT]->index_tabs[counter]), file);
	} else {
		html_load_blank(WEBKIT_WEB_VIEW(story->panel[LEFT]->index_tabs[counter]));
		html_load_blank(WEBKIT_WEB_VIEW(story->panel[RIGHT]->index_tabs[counter]));
	}
	g_object_unref(file);
}

/* Idle function to check whether an index file exists and to load it, or a
blank page if it doesn't exist. */
static gboolean
check_and_load_idle(I7Story *story)
{
	static I7PaneIndexTab counter = 0;

	load_index_file(story, counter);
	counter++; /* next time, load the next tab */
	if(counter == I7_INDEX_NUM_TABS) {
		counter = 0; /* next time, load the first tab */
		i7_document_display_progress_percentage(I7_DOCUMENT(story), 0.0);
		i7_document_remove_status_message(I7_DOCUMENT(story), INDEX_TABS);
		return FALSE; /* quit the cycle */
	}

	/* Update the status bar */
	i7_document_display_progress_percentage(I7_DOCUMENT(story), (gdouble)counter / (gdouble)I7_INDEX_NUM_TABS);
	i7_document_display_status_message(I7_DOCUMENT(story), _("Reloading index..."), INDEX_TABS);

	return TRUE; /* make sure there is a next time */
}

/* Load all the correct files in the index tabs, if they exist */
void
i7_story_reload_index_tabs(I7Story *story, gboolean wait)
{
	if(wait)
		while(check_and_load_idle(story))
			;
	else
		g_idle_add((GSourceFunc)check_and_load_idle, story);
}
