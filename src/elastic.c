/* Copyright (C) 2006-2010, 2011, 2013 P. F. Chimento
 * Portions copyright (C) 2007 Nick Gravgaard (based on
 * gedit-elastictabstops-plugin.c, released 2007-09-16)
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

#include <string.h>

#include <gtk/gtk.h>

#include "app.h"
#include "configfile.h"
#include "elastic.h"

/* calculate the width of the text between @start and @end */
static int
get_text_width(GtkTextView *view, GtkTextIter *start, GtkTextIter *end)
{
	GdkRectangle start_rect, end_rect;

	gtk_text_view_get_iter_location(view, start, &start_rect);
	gtk_text_view_get_iter_location(view, end, &end_rect);

	/* last iter terminates the cell, so take end_rect.x rather than
	 end_rect.x + end_rect.width */
	return end_rect.x - start_rect.x;
}

/* Predicate function for gtk_text_iter_forward_find_char() in stretch_tabstops() */
static gboolean
find_tab(gunichar ch)
{
	return ch == '\t';
}

/* Calculate the tabstop widths in one block ranging from @block_start to
 @block_end. Set the tabstop widths in @tag. */
static void
stretch_tabstops(GtkTextBuffer *textbuffer, GtkTextView *view, GtkTextTag *tag, GtkTextIter *block_start, GtkTextIter *block_end)
{
	I7App *theapp = I7_APP(g_application_get_default());
	GSettings *prefs = i7_app_get_prefs(theapp);
	GtkTextIter cell_start, current_pos, line_end;
	guint max_tabs = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(tag), "elastictabstops-numtabs"));
	int max_widths[max_tabs];
	guint current_tab_num;

	/* initialize tab widths to minimum */
	for(current_tab_num = 0; current_tab_num < max_tabs; current_tab_num++)
		max_widths[current_tab_num] = g_settings_get_uint(prefs, PREFS_TAB_WIDTH);

	/* get width of text in cells */
	g_assert(gtk_text_iter_starts_line(block_start));
	cell_start = current_pos = line_end = *block_start;
	gtk_text_iter_forward_to_line_end(&line_end);

	/* loop over each line */
	while (gtk_text_iter_in_range(&current_pos, block_start, block_end)) {
		/* loop over each cell */
		for (current_tab_num = 0; current_tab_num < max_tabs; current_tab_num++) {
			/* Check if the cell is empty */
			if (gtk_text_iter_get_char(&cell_start) == '\t') {
				current_pos = cell_start;
				/* Skip over the tab character */
				gtk_text_iter_forward_char(&cell_start);
				continue;
			}

			/* Move forward to the next cell */
			if (!gtk_text_iter_forward_find_char(&current_pos, (GtkTextCharPredicate)find_tab, NULL, &line_end))
				break;

			int text_width_in_tab = get_text_width(view, &cell_start, &current_pos);
			max_widths[current_tab_num] = MAX(text_width_in_tab, max_widths[current_tab_num]);

			cell_start = current_pos;
			/* Skip over the tab character itself */
			gtk_text_iter_forward_char(&cell_start);
		}

		gtk_text_iter_forward_line(&cell_start);
		current_pos = cell_start;
		gtk_text_iter_forward_to_line_end(&line_end);
	}

	/* set tabstops */
	int acc_tabstop = 0;
	PangoTabArray *tab_array = pango_tab_array_new(max_tabs, TRUE);
	for (current_tab_num = 0; current_tab_num < max_tabs; current_tab_num++) {
		acc_tabstop += max_widths[current_tab_num] + g_settings_get_uint(prefs, PREFS_TABSTOPS_PADDING);
		pango_tab_array_set_tab(tab_array, current_tab_num, PANGO_TAB_LEFT, acc_tabstop);
	}
	g_object_set(tag,
		"tabs", tab_array,
		"tabs-set", TRUE,
		NULL);
	pango_tab_array_free(tab_array);
}

/* returns the max number of tabs found, and sets location to the first line
 it finds with fewer tabs than the previous line */
static guint
forward_to_block_boundary(GtkTextBuffer *textbuffer, GtkTextIter *location)
{
	int max_tabs = 0, tabs_on_previous_line = 0;

	gtk_text_iter_set_line_offset(location, 0);
	do {
		int tabs_on_line = 0;
		GtkTextIter current_pos = *location, line_end = current_pos;

		/* if the first character is a tab, count that one */
		if(gtk_text_iter_get_char(&current_pos) == '\t')
			tabs_on_line = 1;

		gtk_text_iter_forward_to_line_end(&line_end);
		while (gtk_text_iter_forward_find_char(&current_pos, (GtkTextCharPredicate)find_tab, NULL, &line_end))
			tabs_on_line++;

		if (tabs_on_previous_line < max_tabs && tabs_on_line > tabs_on_previous_line)
			break;
		if (tabs_on_line > max_tabs)
			max_tabs = tabs_on_line;

		tabs_on_previous_line = tabs_on_line;
	} while (gtk_text_iter_forward_line(location));
	return max_tabs;
}

/* Divide the range from @start to @end into blocks of elastic tabstops, and
 calculate the tab widths */
static void
divide_into_blocks(GtkTextView *view, GtkTextBuffer *buffer, GtkTextIter *start, GtkTextIter *end)
{
	g_assert(gtk_text_iter_starts_line(start));

	GtkTextIter block_start, block_end = *start;

	while (gtk_text_iter_in_range(&block_end, start, end)) {
		block_start = block_end;
		guint num_tabs = forward_to_block_boundary(buffer, &block_end);

		GtkTextTag *tag = gtk_text_buffer_create_tag(buffer, NULL, NULL);
		/* Mark this tag so we can identify it as ours */
		g_object_set_data(G_OBJECT(tag), "elastictabstops", tag);
		/* Cache some data on it */
		g_object_set_data(G_OBJECT(tag), "elastictabstops-numtabs", GUINT_TO_POINTER(num_tabs));
		gtk_text_buffer_apply_tag(buffer, tag, &block_start, &block_end);

		/* Calculate the widths of the tabs and apply them */
		stretch_tabstops(buffer, view, tag, &block_start, &block_end);
	}
}

/* foreach function for remove_all_elastic_tabstops_tags() */
static void
find_elastic_tabstops_tags(GtkTextTag *tag, GSList **list)
{
	if (g_object_get_data(G_OBJECT(tag), "elastictabstops"))
		*list = g_slist_prepend(*list, tag);
}

/* foreach function for remove_all_elastic_tabstops_tags() */
static void
remove_elastic_tabstops_tag(GtkTextTag *tag, GtkTextTagTable *table)
{
	gtk_text_tag_table_remove(table, tag);
}

/* remove all our elastic tabstops tags from the buffer */
static void
remove_all_elastic_tabstops_tags(GtkTextBuffer *textbuffer)
{
	GtkTextTagTable *table = gtk_text_buffer_get_tag_table(textbuffer);
	GSList *ourtags = NULL;
	gtk_text_tag_table_foreach(table, (GtkTextTagTableForeach)find_elastic_tabstops_tags, &ourtags);
	g_slist_foreach(ourtags, (GFunc)remove_elastic_tabstops_tag, table);
	g_slist_free(ourtags);
}

/* recalculate the elastic tab stops in the entire document; meant to be called
 either by itself or as a high-priority idle function with g_idle_add_full().
 The priority has to be high so that it runs before the GUI update, otherwise
 you have text shooting all over the place. */
gboolean
elastic_recalculate_view(GtkTextView *view)
{
	GtkTextBuffer *textbuffer = gtk_text_view_get_buffer(view);
	GtkTextIter start, end;

	remove_all_elastic_tabstops_tags(textbuffer);
	gtk_text_buffer_get_bounds(textbuffer, &start, &end);
	divide_into_blocks(view, textbuffer, &start, &end);

	return FALSE; /* one-shot idle function */
}

static void
insert_text_cb(GtkTextBuffer *textbuffer, GtkTextIter *location, gchar *text, gint len, GtkTextView *view)
{
	/* no need to recalculate if we are typing at the end of a line and not
	 entering a newline or tab */
	if ((strchr(text, '\n') || strchr(text, '\t'))
		|| !gtk_text_iter_ends_line(location))
	{
		/* We first remove any pending recalculate function, but perhaps this
		 might also remove idle functions spawned by other plugins? */
		g_idle_remove_by_data(view);
		g_idle_add_full(G_PRIORITY_HIGH_IDLE, (GSourceFunc)elastic_recalculate_view, view, NULL);
	}
}

static void
delete_range_cb(GtkTextBuffer *textbuffer, GtkTextIter *start, GtkTextIter *end, GtkTextView *view)
{
	g_idle_remove_by_data(view); /* is this OK? (see insert_text_cb()) */
	g_idle_add_full(G_PRIORITY_HIGH_IDLE, (GSourceFunc)elastic_recalculate_view, view, NULL);
}

void
add_elastic_tabstops_to_view(GtkTextView *view)
{
	GtkTextIter start, end;
	GtkTextBuffer *textbuffer = gtk_text_view_get_buffer(view);

	gtk_text_buffer_get_bounds(textbuffer, &start, &end);
	divide_into_blocks(view, textbuffer, &start, &end);

	g_signal_connect_after(textbuffer, "insert-text", G_CALLBACK(insert_text_cb), view);
	g_signal_connect_after(textbuffer, "delete-range", G_CALLBACK(delete_range_cb), view);
}

void
remove_elastic_tabstops_from_view(GtkTextView *view)
{
	GtkTextBuffer *textbuffer = gtk_text_view_get_buffer(view);

	g_signal_handlers_disconnect_by_func(textbuffer, insert_text_cb, view);
	g_signal_handlers_disconnect_by_func(textbuffer, delete_range_cb, view);

	remove_all_elastic_tabstops_tags(textbuffer);
}
