/*  Copyright (C) 2008, 2009, 2010, 2021 P. F. Chimento
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

#include "config.h"

#include <string.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

#include "document.h"
#include "searchwindow.h"

/* THE "SEARCH ENGINE" */

gboolean
find_no_wrap(const GtkTextIter *startpos, const char *text, gboolean forward, GtkTextSearchFlags flags,
    I7SearchType search_type, GtkTextIter *match_start, GtkTextIter *match_end)
{
	if(search_type == I7_SEARCH_CONTAINS)
		return forward?
			gtk_text_iter_forward_search(startpos, text, flags, match_start, match_end, NULL)
			: gtk_text_iter_backward_search(startpos, text, flags, match_start, match_end, NULL);

	GtkTextIter start, end, searchfrom = *startpos;
	while(forward?
		gtk_text_iter_forward_search(&searchfrom, text, flags, &start, &end, NULL)
		: gtk_text_iter_backward_search(&searchfrom, text, flags, &start, &end, NULL))
	{
		if(search_type == I7_SEARCH_FULL_WORD && gtk_text_iter_starts_word(&start) && gtk_text_iter_ends_word(&end)) {
			*match_start = start;
			*match_end = end;
			return TRUE;
		} else if(search_type == I7_SEARCH_STARTS_WORD && gtk_text_iter_starts_word(&start)) {
			*match_start = start;
			*match_end = end;
			return TRUE;
		}
		searchfrom = forward? end : start;
	}
	return FALSE;
}

static gboolean
find(GtkTextBuffer *buffer, const gchar *text, gboolean forward, gboolean ignore_case, gboolean restrict_search, I7SearchType search_type, GtkTextIter *match_start, GtkTextIter *match_end)
{
	GtkTextIter iter;
	GtkTextSearchFlags flags = GTK_TEXT_SEARCH_TEXT_ONLY
		| (ignore_case? GTK_TEXT_SEARCH_CASE_INSENSITIVE : 0)
		| (restrict_search? GTK_TEXT_SEARCH_VISIBLE_ONLY : 0);

	/* Start the search at the end or beginning of the selection */
	if(forward)
		gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_selection_bound(buffer));
	else
		gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));

	if(!find_no_wrap(&iter, text, forward, flags, search_type, match_start, match_end))
	{
		/* Wrap around to the beginning or end */
		if(forward)
			gtk_text_buffer_get_start_iter(buffer, &iter);
		else
			gtk_text_buffer_get_end_iter(buffer, &iter);
		if(!find_no_wrap(&iter, text, forward, flags, search_type, match_start, match_end))
			return FALSE;
	}
	return TRUE;
}

/* CALLBACKS */

void
on_findbar_entry_changed(GtkEditable *editable, I7Document *self)
{
	i7_document_unhighlight_quicksearch(self);
	gchar *search_text = gtk_editable_get_chars(editable, 0, -1);
	i7_document_set_quicksearch_not_found(self, !i7_document_highlight_quicksearch(self, search_text, TRUE));
	g_free(search_text);
}

void
on_find_entry_changed(GtkEditable *editable, I7Document *self)
{
	const gchar *text = gtk_entry_get_text(GTK_ENTRY(editable));
	gboolean text_not_empty = !(text == NULL || strlen(text) == 0);
	gtk_widget_set_sensitive(self->find_button, text_not_empty);
	gtk_widget_set_sensitive(self->replace_button, text_not_empty);
	gtk_widget_set_sensitive(self->replace_all_button, text_not_empty);
}

void
on_find_button_clicked(GtkButton *button, I7Document *self)
{
	const char *text = gtk_entry_get_text(GTK_ENTRY(self->find_entry));
	gboolean ignore_case = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->ignore_case));
	gboolean forward = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->reverse));
	gboolean restrict_search = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->restrict_search));
	I7SearchType search_type = gtk_combo_box_get_active(GTK_COMBO_BOX(self->search_type));
	i7_document_find(self, text, forward, ignore_case, restrict_search, search_type);
}

void
on_replace_button_clicked(GtkButton *button, I7Document *self)
{
	const char *search_text = gtk_entry_get_text(GTK_ENTRY(self->find_entry));
	const char *replace_text = gtk_entry_get_text(GTK_ENTRY(self->replace_entry));
	gboolean ignore_case = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->ignore_case));
	gboolean forward = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->reverse));
	gboolean restrict_search = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->restrict_search));
	I7SearchType search_type = gtk_combo_box_get_active(GTK_COMBO_BOX(self->search_type));
	GtkTextIter start, end;
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(i7_document_get_buffer(self));

	gtk_text_buffer_get_selection_bounds(buffer, &start, &end);
	gchar *selected = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

	/* if the text is already selected, then replace it, otherwise "find" again
	 to select the text */
	if(!(ignore_case? strcasecmp(selected, search_text) : strcmp(selected, search_text))) {
		/* Replacing counts as one action for Undo */
		gtk_text_buffer_begin_user_action(buffer);
		gtk_text_buffer_delete(buffer, &start, &end);
		gtk_text_buffer_insert(buffer, &start, replace_text, -1);
		gtk_text_buffer_end_user_action(buffer);
	}
	g_free(selected);

	/* Find the next occurrence of the text */
	i7_document_find(self, search_text, forward, ignore_case, restrict_search, search_type);
}

void
on_replace_all_button_clicked(GtkButton *button, I7Document *self)
{
	const char *search_text = gtk_entry_get_text(GTK_ENTRY(self->find_entry));
	const char *replace_text = gtk_entry_get_text(GTK_ENTRY(self->replace_entry));
	gboolean ignore_case = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->ignore_case));
	gboolean restrict_search = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->restrict_search));
	I7SearchType search_type = gtk_combo_box_get_active(GTK_COMBO_BOX(self->search_type));
	GtkTextIter cursor, start, end;
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(i7_document_get_buffer(self));
	GtkTextSearchFlags flags = GTK_TEXT_SEARCH_TEXT_ONLY
		| (ignore_case? GTK_TEXT_SEARCH_CASE_INSENSITIVE : 0)
		| (restrict_search? GTK_TEXT_SEARCH_VISIBLE_ONLY : 0);

	/* Replace All counts as one action for Undo */
	gtk_text_buffer_begin_user_action(buffer);

	gtk_text_buffer_get_start_iter(buffer, &cursor);
	int replace_count = 0;

	while(find_no_wrap(&cursor, search_text, TRUE, flags, search_type, &start, &end)) {
		/* delete preserves start and end iterators */
		gtk_text_buffer_delete(buffer, &start, &end);
		/* Save end position */
		GtkTextMark *tempmark = gtk_text_buffer_create_mark(buffer, NULL, &end, FALSE);
		gtk_text_buffer_insert(buffer, &start, replace_text, -1);
		/* Continue from end position, so as to avoid a loop if replace text
		 contains search text */
		gtk_text_buffer_get_iter_at_mark(buffer, &cursor, tempmark);
		gtk_text_buffer_delete_mark(buffer, tempmark);
		replace_count++;
	}

	gtk_text_buffer_end_user_action(buffer);

	gchar *message = g_strdup_printf(_("%d occurrences replaced"), replace_count);
	i7_document_flash_status_message(self, message, SEARCH_OPERATIONS);
	g_free(message);

	/* Close the dialog */
	gtk_widget_hide(self->find_dialog);
}

void
on_search_files_entry_changed(GtkEditable *editable, I7Document *self)
{
	const gchar *text = gtk_entry_get_text(GTK_ENTRY(editable));
	gboolean text_not_empty = !(text == NULL || strlen(text) == 0);
	gtk_widget_set_sensitive(self->search_files_find, text_not_empty);
}

void
on_search_files_find_clicked(GtkButton *button, I7Document *self)
{
	const char *text = gtk_entry_get_text(GTK_ENTRY(self->search_files_entry));
	gboolean ignore_case = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->search_files_ignore_case));
	I7SearchType search_type = gtk_combo_box_get_active(GTK_COMBO_BOX(self->search_files_type));

	/* Close the dialog */
	gtk_widget_hide(self->search_files_dialog);

	GtkWidget *search_window = i7_search_window_new(self, text, ignore_case, search_type);
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->search_files_project)))
		i7_search_window_search_project(I7_SEARCH_WINDOW(search_window));
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->search_files_extensions)))
		i7_search_window_search_extensions(I7_SEARCH_WINDOW(search_window));
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->search_files_documentation)))
		i7_search_window_search_documentation(I7_SEARCH_WINDOW(search_window));
	i7_search_window_done_searching(I7_SEARCH_WINDOW(search_window));
}

/* PUBLIC FUNCTIONS */

gboolean
i7_document_highlight_quicksearch(I7Document *self, const char *text, gboolean forward)
{
	return I7_DOCUMENT_GET_CLASS(self)->highlight_search(self, text, forward);
}

void
i7_document_unhighlight_quicksearch(I7Document *self)
{
	GtkWidget *focus = i7_document_get_highlighted_view(self);
	if(!focus)
		return;

	if(GTK_IS_TEXT_VIEW(focus)) {
		/* Remove selection */
		GtkTextIter start;
		GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(focus));
		if(gtk_text_buffer_get_selection_bounds(buffer, &start, NULL))
			gtk_text_buffer_place_cursor(buffer, &start);
	} else if(WEBKIT_IS_WEB_VIEW(focus)) {
		WebKitFindController *controller = webkit_web_view_get_find_controller(WEBKIT_WEB_VIEW(focus));
		webkit_find_controller_search_finish(controller);
	}

	i7_document_set_highlighted_view(self, NULL);
}

void
i7_document_set_quicksearch_not_found(I7Document *self, gboolean not_found)
{
	GtkStyleContext *style = gtk_widget_get_style_context(self->findbar_entry);
	if(not_found) {
		gtk_style_context_add_class(style, "error");
		i7_document_flash_status_message(self, _("Phrase not found"), SEARCH_OPERATIONS);
	} else {
		gtk_style_context_remove_class(style, "error");
	}
}

void
i7_document_find(I7Document *self, const char *text, gboolean forward, gboolean ignore_case, gboolean restrict_search, I7SearchType search_type)
{
	GtkTextIter start, end;
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(i7_document_get_buffer(self));

	if(!find(buffer, text, forward, ignore_case, restrict_search, search_type, &start, &end))
	{
		i7_document_flash_status_message(self, _("Phrase not found"), SEARCH_OPERATIONS);
		return;
	}

	/* We may have searched the invisible regions, so if the found text is
	 invisible, go back to showing the entire source. */
	if(i7_document_iter_is_invisible(self, &start) || i7_document_iter_is_invisible(self, &end))
		i7_document_show_entire_source(self);

	gtk_text_buffer_select_range(buffer, &start, &end);
	i7_document_scroll_to_selection(self);
}
