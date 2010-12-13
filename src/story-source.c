/*  Copyright 2006 P.F. Chimento
 *  This file is part of GNOME Inform 7.
 * 
 *  GNOME Inform 7 is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  GNOME Inform 7 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNOME Inform 7; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#include <string.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksourceview.h>
#include <gtksourceview/gtksourcebuffer.h>
#include "story.h"
#include "story-private.h"
#include "app.h"
#include "configfile.h"
#include "document.h"
#include "panel.h"

void 
after_source_buffer_delete_range(GtkTextBuffer *buffer, GtkTextIter *start, GtkTextIter *end, I7Document *document) 
{
	if(!config_file_get_bool(PREFS_INTELLIGENCE))
		return;
	/* Reindex the section headings anytime text is deleted, because running after
	the default signal handler means we have no access to the deleted text. */
	i7_document_reindex_headings(document);
	/* TODO: do this in idle time and remove the old idle function */
}

void 
after_source_buffer_insert_text(GtkTextBuffer *buffer, GtkTextIter *location, gchar *text, gint len, I7Document *document)
{
	/* If the inserted text ended in a newline, then do auto-indenting */
	/* We could use gtk_source_view_set_auto_indent(), but that auto-indents
	  leading spaces as well as tabs, and we don't want that */
	if(g_str_has_suffix(text, "\n") && config_file_get_bool(PREFS_AUTO_INDENT)) {
		int tab_count = 0;
		GtkTextIter prev_line = *location;
		gtk_text_iter_backward_line(&prev_line);
		while(gtk_text_iter_get_char(&prev_line) == '\t') {
			gtk_text_iter_forward_char(&prev_line);
			tab_count++;
		}
		gchar *tabs = g_strnfill(tab_count, '\t');
		/* Preserve and restore iter position by creating a mark with right gravity (FALSE) */
		GtkTextMark *bookmark = gtk_text_buffer_create_mark(buffer, "bookmark", location, FALSE);
		gtk_text_buffer_insert_at_cursor(buffer, tabs, -1);
		gtk_text_buffer_get_iter_at_mark(buffer, location, bookmark);
		gtk_text_buffer_delete_mark(buffer, bookmark);
	}
	
	/* Return after that if we are not doing intelligent symbol following */    
	if(!config_file_get_bool(PREFS_INTELLIGENCE))
		return;
	
	/* For any text, a section heading might have been entered or changed, so
	reindex the section headings */
	i7_document_reindex_headings(document);
	/* TODO: do this in idle time and remove the old idle function */
	
	/* If the text ends with a space, check whether it is a section heading that
	needs auto-numbering */
#if 0
	if(config_file_get_bool(PREFS_AUTO_NUMBER_SECTIONS)) {
		if(g_str_has_suffix(text, " ")) {
			gint line = gtk_text_iter_get_line(location);
			GtkTextIter line_start;
			gtk_text_buffer_get_iter_at_line(buffer, &line_start, line);
			GtkTextIter prev_line = line_start;
			gtk_text_iter_backward_line(&prev_line);
			gchar *line_text = gtk_text_iter_get_text(&line_start, location);
			gchar *lcase = g_utf8_strdown(line_text, -1);
			
			if(gtk_text_iter_get_char(&prev_line) == '\n' /*blank line before*/
			  && !(strcmp(lcase, "volume ") && strcmp(lcase, "book ")
			  && strcmp(lcase, "part ") && strcmp(lcase, "chapter ")
			  && strcmp(lcase, "section "))) {
				/* Count all this as one action for undo */
				gtk_text_buffer_begin_user_action(buffer);
				gtk_text_buffer_insert(buffer, location, "0 - ", -1);
				renumber_sections(buffer);
				gtk_text_buffer_end_user_action(buffer);
			}
			
			g_free(line_text);
			g_free(lcase);

			/* For some reason renumber_sections moves the cursor to the start
			of the next line; this counteracts that */
			GtkTextIter cursor;
			GtkTextMark *mark = gtk_text_buffer_get_insert(buffer);
			gtk_text_buffer_get_iter_at_mark(buffer, &cursor, mark);
			if(gtk_text_iter_starts_line(&cursor)) {
				gtk_text_iter_backward_char(&cursor);
				gtk_text_buffer_place_cursor(buffer, &cursor);
			}
		}
	}
#endif
}

void
on_panel_paste_code(I7Panel *panel, gchar *code, I7Story *story)
{
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(i7_document_get_buffer(I7_DOCUMENT(story)));
	gtk_text_buffer_begin_user_action(buffer);
	gtk_text_buffer_delete_selection(buffer, TRUE, TRUE);
	/* Delete selection does nothing if there is no selection */
	gtk_text_buffer_insert_at_cursor(buffer, code, -1);
	gtk_text_buffer_end_user_action(buffer);
}

void
on_panel_jump_to_line(I7Panel *panel, guint line, I7Story *story)
{
	int side = i7_story_choose_panel(story, I7_PANE_SOURCE);
	gtk_notebook_set_current_page(GTK_NOTEBOOK(story->panel[side]->notebook), I7_PANE_SOURCE);
	i7_source_view_jump_to_line(story->panel[side]->sourceview, line);
}

/* Reindex the section headings */
gboolean 
reindex_headings(GtkTextBuffer *buffer, I7Document *document) 
{
	i7_document_reindex_headings(document);
    return FALSE; /* One-shot idle function */
}
