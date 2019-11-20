/* Copyright (C) 2006-2009, 2010, 2011, 2013 P. F. Chimento
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

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>

#include "app.h"
#include "configfile.h"
#include "document.h"
#include "panel.h"
#include "story.h"

/* Following two functions adapted from gtksourceview. We could use
gtk_source_view_set_auto_indent(), but that auto-indents leading spaces as well
as tabs, and we don't want that. */
static char *
compute_indentation(GtkSourceView *source, GtkTextIter *cur)
{
	GtkTextIter start, end;
	int line = gtk_text_iter_get_line(cur);

	gtk_text_buffer_get_iter_at_line(gtk_text_view_get_buffer(GTK_TEXT_VIEW(source)), &start, line);
	end = start;

	gunichar ch = gtk_text_iter_get_char(&end);

	while(ch == '\t' && gtk_text_iter_compare(&end, cur) < 0) {
		if(!gtk_text_iter_forward_char(&end))
			break;

		ch = gtk_text_iter_get_char(&end);
	}

	if(gtk_text_iter_equal(&start, &end))
		return NULL;

	return gtk_text_iter_get_slice(&start, &end);
}

gboolean
on_source_key_press_event(GtkSourceView *source, GdkEventKey *event, I7SourceView *view)
{
	I7App *theapp = i7_app_get();
	GSettings *prefs = i7_app_get_prefs(theapp);
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(source));

	int key = event->keyval;

	if((key == GDK_KEY_Return || key == GDK_KEY_KP_Enter) &&
		!(event->state & GDK_SHIFT_MASK) &&
		g_settings_get_boolean(prefs, PREFS_AUTO_INDENT)) {
		/* Auto-indent means that when you press ENTER at the end of a line, the new
		line is automatically indented at the same level as the previous line.
		SHIFT+ENTER avoids auto-indentation. */
		GtkTextMark *mark = gtk_text_buffer_get_insert(buffer);
		GtkTextIter cur;
		gtk_text_buffer_get_iter_at_mark(buffer, &cur, mark);

		char *indent = compute_indentation(source, &cur);

		if(indent != NULL) {
			/* Allow input methods to internally handle a key press event. If this
			function returns TRUE, then no further processing should be done for this
			keystroke. */
			if(gtk_text_view_im_context_filter_keypress(GTK_TEXT_VIEW(source), event)) {
				g_free(indent);
				return TRUE; /* stop event */
			}

			/* If an input method has inserted some text while handling the key press
			event, the cur iter may be invalid, so get the iter again */
			gtk_text_buffer_get_iter_at_mark(buffer, &cur, mark);

			/* Insert new line and auto-indent. */
			gtk_text_buffer_begin_user_action(buffer);
			gtk_text_buffer_insert(buffer, &cur, "\n", 1);
			gtk_text_buffer_insert(buffer, &cur, indent, strlen(indent));
			g_free(indent);
			gtk_text_buffer_end_user_action(buffer);
			gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(source), mark);
			return TRUE; /* stop event */
		}
	}
	return FALSE;  /* propagate event */
}

void
after_source_buffer_delete_range(GtkTextBuffer *buffer, GtkTextIter *start, GtkTextIter *end, I7Document *document)
{
	I7App *theapp = i7_app_get();
	GSettings *prefs = i7_app_get_prefs(theapp);

	if(g_settings_get_boolean(prefs, PREFS_INDENT_WRAPPED))
		i7_document_update_indent_tags(document, start, end);

	if(!g_settings_get_boolean(prefs, PREFS_INTELLIGENCE))
		return;
	/* Reindex the section headings anytime text is deleted, because running after
	the default signal handler means we have no access to the deleted text. */
	i7_document_reindex_headings(document);
	/* TODO: do this in idle time and remove the old idle function */
}

void
after_source_buffer_insert_text(GtkTextBuffer *buffer, GtkTextIter *location, gchar *text, gint len, I7Document *document)
{
	I7App *theapp = i7_app_get();
	GSettings *prefs = i7_app_get_prefs(theapp);

	if(g_settings_get_boolean(prefs, PREFS_INDENT_WRAPPED)) {
		GtkTextIter insert_start = *location;
		gtk_text_iter_backward_chars(&insert_start, len);
		i7_document_update_indent_tags(document, &insert_start, location);
	}

	/* Return after that if we are not doing intelligent symbol following */
	if(!g_settings_get_boolean(prefs, PREFS_INTELLIGENCE))
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
