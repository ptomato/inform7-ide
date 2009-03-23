/*
 * elastic.c
 * Based on gedit-elastictabstops-plugin.c, released 2007-09-16 (first release)
 * Copyright (C) 2007 Nick Gravgaard
 * Adapted 2009-03-22 by P. F. Chimento
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

#include <string.h>
#include <gtk/gtk.h>
#include "elastic.h"

#include "configfile.h"
#include "story.h"
#include "support.h"

typedef struct
{
	int text_width_pix;
	int *widest_width_pix;
	gboolean ends_in_tab;
} et_tabstop;

typedef struct
{
	int num_tabs;
} et_line;

typedef enum
{
	BACKWARDS,
	FORWARDS
} direction;

static int 
get_text_width(GtkTextView *view, GtkTextIter *start, GtkTextIter *end)
{
	GdkRectangle start_rect, end_rect;

	gtk_text_view_get_iter_location(view, start, &start_rect);
	gtk_text_view_get_iter_location(view, end, &end_rect);

	return end_rect.x - start_rect.x;
}

static int 
calc_tab_width(GtkTextView *view, int text_width_in_tab)
{
	int tab_width_minimum;
	PangoLayout *space = gtk_widget_create_pango_layout(GTK_WIDGET(view), " ");
    pango_layout_get_pixel_size(space, &tab_width_minimum, NULL);
    g_object_unref(space);
	tab_width_minimum *= config_file_get_int("EditorSettings", "TabWidth");
	
	if (text_width_in_tab < tab_width_minimum)
		text_width_in_tab = tab_width_minimum;
	return text_width_in_tab + 
		config_file_get_int("EditorSettings", "ElasticTabPadding");
}

static gboolean 
change_line(GtkTextIter *location, direction which_dir)
{
	if (which_dir == FORWARDS)
		return gtk_text_iter_forward_line(location);
	else
		return gtk_text_iter_backward_line(location);
}

/* returns the max number of tabs found, and sets location to the first line it finds with no tabs in */
static int 
get_block_boundary(GtkTextBuffer *textbuffer, GtkTextIter *location, 
				   direction which_dir)
{
	int max_tabs = 0;
	gboolean orig_line = TRUE;

	gtk_text_iter_set_line_offset(location, 0);
	do {
		int tabs_on_line = 0;
		GtkTextIter current_pos = *location;
		gunichar current_char = gtk_text_iter_get_char(&current_pos);
		gboolean current_char_ends_line = gtk_text_iter_ends_line(&current_pos);

		while (current_char != '\0' && current_char_ends_line == FALSE) {
			if (current_char == '\t') {
				tabs_on_line++;
				if (tabs_on_line > max_tabs)
					max_tabs = tabs_on_line;
			}
			gtk_text_iter_forward_char(&current_pos);
			current_char = gtk_text_iter_get_char(&current_pos);
			current_char_ends_line = gtk_text_iter_ends_line(&current_pos);
		}
		if (tabs_on_line == 0 && orig_line == FALSE)
			return max_tabs;
		orig_line = FALSE;
	} while (change_line(location, which_dir));
	return max_tabs;
}

/* returns the max number of tabs found between the start and end iters */
static int 
get_nof_tabs_between(GtkTextBuffer *textbuffer, GtkTextIter *start, 
					 GtkTextIter *end)
{
	GtkTextIter current_pos = *start;
	int max_tabs = 0;

	gtk_text_iter_set_line_offset(&current_pos, 0);
	do {
		int tabs_on_line = 0;
		gunichar current_char = gtk_text_iter_get_char(&current_pos);
		gboolean current_char_ends_line = gtk_text_iter_ends_line(&current_pos);

		while (current_char != '\0' && current_char_ends_line == FALSE) {
			if (current_char == '\t') {
				tabs_on_line++;
				if (tabs_on_line > max_tabs)
					max_tabs = tabs_on_line;
			}
			gtk_text_iter_forward_char(&current_pos);
			current_char = gtk_text_iter_get_char(&current_pos);
			current_char_ends_line = gtk_text_iter_ends_line(&current_pos);
		}
	} while (change_line(&current_pos, FORWARDS) && gtk_text_iter_compare(&current_pos, end) < 0);
	return max_tabs;
}

static void 
stretch_tabstops(GtkTextBuffer *textbuffer, GtkTextView *view, 
				 int block_start_linenum, int block_nof_lines, int max_tabs)
{
	int l, t;
	et_line lines[block_nof_lines];
	et_tabstop grid[block_nof_lines][max_tabs];

	memset(lines, 0, sizeof(lines));
	memset(grid, 0, sizeof(grid));

	// get width of text in cells
	for (l = 0; l < block_nof_lines; l++) // for each line
	{
		GtkTextIter current_pos, cell_start;
		int text_width_in_tab = 0;
		int current_line_num = block_start_linenum + l;
		int current_tab_num = 0;
		gboolean cell_empty = TRUE;

		gtk_text_buffer_get_iter_at_line(textbuffer, &current_pos, current_line_num);
		cell_start = current_pos;
		gunichar current_char = gtk_text_iter_get_char(&current_pos);
		gboolean current_char_ends_line = gtk_text_iter_ends_line(&current_pos);
		/* maybe change this to search forwards for tabs/newlines using
		gtk_text_iter_forward_find_char
		see http://www.bravegnu.org/gtktext/x370.html */

		while (current_char != '\0') {
			if (current_char_ends_line == TRUE) {
				grid[l][current_tab_num].ends_in_tab = FALSE;
				text_width_in_tab = 0;
				break;
			}
			else if (current_char == '\t') {
				if (cell_empty == FALSE)
					text_width_in_tab = 
						get_text_width(view, &cell_start, &current_pos);
				grid[l][current_tab_num].ends_in_tab = TRUE;
				grid[l][current_tab_num].text_width_pix = 
					calc_tab_width(view, text_width_in_tab);
				current_tab_num++;
				lines[l].num_tabs++;
				text_width_in_tab = 0;
				cell_empty = TRUE;
			} else {
				if (cell_empty == TRUE) {
					cell_start = current_pos;
					cell_empty = FALSE;
				}
			}
			gtk_text_iter_forward_char(&current_pos);
			current_char = gtk_text_iter_get_char(&current_pos);
			current_char_ends_line = gtk_text_iter_ends_line(&current_pos);
		}
	}

	/* find columns blocks and stretch to fit the widest cell */
	for (t = 0; t < max_tabs; t++) { /* for each column */
		gboolean starting_new_block = TRUE;
		int first_line_in_block = 0;
		int max_width = 0;
		for (l = 0; l < block_nof_lines; l++) { /* for each line */
			if (starting_new_block == TRUE) {
				starting_new_block = FALSE;
				first_line_in_block = l;
				max_width = 0;
			}
			if (grid[l][t].ends_in_tab == TRUE) {
				grid[l][t].widest_width_pix = &(grid[first_line_in_block][t].text_width_pix); 
				/* point widestWidthPix at first */
				if (grid[l][t].text_width_pix > max_width) {
					max_width = grid[l][t].text_width_pix;
					grid[first_line_in_block][t].text_width_pix = max_width;
				}
			} else { /* end column block */
				starting_new_block = TRUE;
			}
		}
	}

	/* set tabstops */
	for (l = 0; l < block_nof_lines; l++) { /* for each line */
		GtkTextIter line_start, line_end;
		int current_line_num = block_start_linenum + l;
		int acc_tabstop = 0;
		
		PangoTabArray *tab_array = pango_tab_array_new(lines[l].num_tabs, TRUE);

		for (t = 0; t < lines[l].num_tabs; t++) {
			if (grid[l][t].widest_width_pix != NULL) {
				acc_tabstop += *(grid[l][t].widest_width_pix);
				pango_tab_array_set_tab(tab_array, t, PANGO_TAB_LEFT, acc_tabstop);
			}
		}

		GtkTextTag *tag = gtk_text_buffer_create_tag(textbuffer, NULL, "tabs",
													 tab_array, NULL);
	
		/* Apply word_wrap tag to whole buffer */
		/* gtk_text_buffer_get_bounds(textbuffer, &start, &end);*/
	
		gtk_text_buffer_get_iter_at_line(textbuffer, &line_start, 
										 current_line_num);
	
		line_end = line_start;
	
		if (gtk_text_iter_ends_line(&line_end) == FALSE)
			gtk_text_iter_forward_to_line_end(&line_end);
	
		/* gtk_text_buffer_remove_all_tags(textbuffer, &line_start, &line_end); // is this necessary? */
		gtk_text_buffer_apply_tag(textbuffer, tag, &line_start, &line_end);
	
		pango_tab_array_free(tab_array);
	}
}

static void 
elastictabstops_onmodify(GtkTextBuffer *textbuffer, GtkTextIter *start, 
						 GtkTextIter *end, GtkTextView *view)
{
	GtkTextIter block_start_iter = *start;
	GtkTextIter block_end_iter = *end;

	int max_tabs_between = 
		get_nof_tabs_between(textbuffer, &block_start_iter, &block_end_iter);
	int max_tabs_backwards = 
		get_block_boundary(textbuffer, &block_start_iter, BACKWARDS);
	int max_tabs_forwards = 
		get_block_boundary(textbuffer, &block_end_iter, FORWARDS);
	int max_tabs = 
		MAX(MAX(max_tabs_between, max_tabs_backwards), max_tabs_forwards);

	int block_start_linenum = gtk_text_iter_get_line(&block_start_iter);
	int block_end_linenum = gtk_text_iter_get_line(&block_end_iter);
	int block_nof_lines = (block_end_linenum - block_start_linenum) + 1;

	stretch_tabstops(textbuffer, view, block_start_linenum, block_nof_lines, max_tabs);
}

/* Appends the text tag to the list pointed to by listptr, if the tag specifies
 a tab array, but no other formatting */
static void
append_tab_tag_to_list(GtkTextTag *tag, GSList **listptr)
{
	/* See if this tag specifies a tab array or is nameless */
	gboolean set;
	gchar *name;
	g_object_get(tag, "tabs-set", &set, "name", &name, NULL);
	if(!set || name != NULL)
		return;
	
	gchar *props[] = {
		"background-full-height-set", "background-set", 
		"background-stipple-set", "editable-set", "family-set", 
		"foreground-set", "foreground-stipple-set", "indent-set", 
		"invisible-set", "justification-set", "language-set", "left-margin-set",
		"paragraph-background-set", "pixels-above-lines-set",
		"pixels-below-lines-set", "pixels-inside-wrap-set", "right-margin-set",
		"rise-set", "scale-set", "size-set", "stretch-set", "strikethrough-set",
		"style-set", "underline-set", "variant-set", "weight-set", 
		"wrap-mode-set"
	};
	int foo;
	for(foo = 0; foo < G_N_ELEMENTS(props); foo++) {
		g_object_get(tag, props[foo], &set, NULL);
		if(set)
			return;
	}
	
	*listptr = g_slist_prepend(*listptr, tag);
}

void
elastic_remove(GtkSourceBuffer *buffer)
{
	GtkTextBuffer *textbuffer = GTK_TEXT_BUFFER(buffer);
	GtkTextTagTable *tagtable = gtk_text_buffer_get_tag_table(textbuffer);
	GSList *tabtags = NULL;
	gtk_text_tag_table_foreach(tagtable, 
							   (GtkTextTagTableForeach)append_tab_tag_to_list, 
							   &tabtags);
	
	GtkTextIter start, end;
	gtk_text_buffer_get_bounds(textbuffer, &start, &end);
	
	GSList *iter;
	for(iter = tabtags; iter; iter = g_slist_next(iter))
		gtk_text_buffer_remove_tag(textbuffer, iter->data, &start, &end);
	
	g_slist_free(tabtags);
}

void 
elastic_refresh(GtkTextBuffer *textbuffer, GtkTextView *view)
{
	if(!config_file_get_bool("EditorSettings", "ElasticTabstops"))
		return;
	GtkTextIter start, end;
	gtk_text_buffer_get_bounds(textbuffer, &start, &end);
	elastictabstops_onmodify(textbuffer, &start, &end, view);
}

gboolean 
elastic_insert_text(GtkTextBuffer *textbuffer, GtkTextIter *location,
					gchar *text, gint len, GtkTextView *view)
{
	if(!config_file_get_bool("EditorSettings", "ElasticTabstops"))
		return FALSE;
	GtkTextIter start, end;
	start = end = *location;
	gtk_text_iter_backward_chars(&start, len);
	elastictabstops_onmodify(textbuffer, &start, &end, view);
	return FALSE;
}

gboolean 
elastic_delete_range(GtkTextBuffer *textbuffer, GtkTextIter *start, 
					 GtkTextIter *end, GtkTextView *view)
{
	if(!config_file_get_bool("EditorSettings", "ElasticTabstops"))
		return FALSE;
	elastictabstops_onmodify(textbuffer, start, end, view);
	return FALSE;
}

void elastic_setup(GtkTextBuffer *textbuffer, GtkTextView *view)
{
	g_signal_connect(textbuffer, "insert-text", G_CALLBACK(elastic_insert_text),
					 view);
	g_signal_connect(textbuffer, "delete-range",
					 G_CALLBACK(elastic_delete_range), view);
}
