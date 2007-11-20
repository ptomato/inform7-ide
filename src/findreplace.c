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
 
#include <gnome.h>
#include <string.h>
#include <gtksourceview/gtksourceiter.h>

#include "support.h"

#include "extension.h"
#include "findreplace.h"

/* Scroll the text view so that the cursor is within the inner 25%-75%, if
possible. */
static void scroll_text_view_to_cursor(GtkTextView *view) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(view);
    gtk_text_view_scroll_to_mark(view,
        gtk_text_buffer_get_insert(buffer),
        0.25, FALSE, 0.0, 0.0);
}


/* Searches for the next (or previous) occurrence of the string search_text
and puts it in the text iterators match_start and match_end */
static gboolean find_next(const gchar *search_text, GtkTextIter *search_from,
  GtkTextIter *match_start, GtkTextIter *match_end, gboolean ignore_case,
  gboolean reverse, int algorithm) {
    gboolean retval, do_it_again;
    /* It's probably better to implement it this way, i.e. non-recursively, so
      that we don't run out of stack space? */
      
    do {
        do_it_again = FALSE;
        if(reverse)
            retval = gtk_source_iter_backward_search(search_from, search_text,
              GTK_SOURCE_SEARCH_VISIBLE_ONLY | GTK_SOURCE_SEARCH_TEXT_ONLY |
              (ignore_case? GTK_SOURCE_SEARCH_CASE_INSENSITIVE : 0),
              match_start, match_end, NULL);
        else
            retval = gtk_source_iter_forward_search(search_from, search_text,
              GTK_SOURCE_SEARCH_VISIBLE_ONLY | GTK_SOURCE_SEARCH_TEXT_ONLY |
              (ignore_case? GTK_SOURCE_SEARCH_CASE_INSENSITIVE : 0),
              match_start, match_end, NULL);
        if(retval) {
            if(algorithm == FIND_STARTS_WITH) {
                if(!gtk_text_iter_starts_word(match_start))
                    do_it_again = TRUE;
            } else if(algorithm == FIND_FULL_WORD) {
                if(!gtk_text_iter_starts_word(match_start)
                  || !gtk_text_iter_ends_word(match_end))
                    do_it_again = TRUE;
            }      
        }
        if(do_it_again)
            *search_from = *match_end;
    } while(do_it_again);
    return retval;
}

static void find(GtkTextBuffer *buffer, const gchar *text, gboolean ignore_case,
gboolean wrap, gboolean reverse, int algorithm) {
    GtkTextIter cursor, match_start, match_end;
    GtkWidget *dialog;

    /* If there is a selection, start searching past it; otherwise start
    searching from the cursor. Likewise, if searching backwards, start before
    the selection. */
    gtk_text_buffer_get_iter_at_mark(buffer, &cursor, reverse?
      gtk_text_buffer_get_insert(buffer)
      : gtk_text_buffer_get_selection_bound(buffer));

    if(find_next(text, &cursor, &match_start, &match_end, ignore_case,
      reverse, algorithm)) {
        gtk_text_buffer_select_range(buffer, &match_start, &match_end);
        return;
    } else {
        /* Here it gets a little convoluted, because we don't want to keep
        wrapping around a million times. */
        if(wrap) {
            /* Wrap the search around to the beginning/end */
            if(reverse)
                gtk_text_buffer_get_end_iter(buffer, &cursor);
            else
                gtk_text_buffer_get_start_iter(buffer, &cursor);
            if(find_next(text, &cursor, &match_start, &match_end, ignore_case,
              reverse, algorithm)) {
                gtk_text_buffer_select_range(buffer, &match_start, &match_end);
                return;
            } else {
                /* Text does not occur in the whole document */
                dialog = gtk_message_dialog_new(
                  NULL, 0, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                  "Search text not found.");
                gtk_dialog_run(GTK_DIALOG(dialog));
                gtk_widget_destroy(dialog);
                return;
            }
        } else {
            /* Wrap is turned off, text was not found */
            dialog = gtk_message_dialog_new(
              NULL, 0, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
              "Search text not found.");
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            return;
        }
    }
}

static int replace_all(GtkTextBuffer *buffer, const gchar *search_text,
const gchar *replace_text, gboolean ignore_case, int algorithm) {
    GtkTextIter cursor, match_start, match_end;
    
    /* Replace All counts as one action for Undo */
    gtk_text_buffer_begin_user_action(buffer);

    gtk_text_buffer_get_start_iter(buffer, &cursor);
    int replace_count = 0;

    while(find_next(search_text, &cursor, &match_start, &match_end, ignore_case,
      FALSE, algorithm)) {
        gtk_text_buffer_delete(buffer, &match_start, &match_end);
        gtk_text_buffer_insert(buffer, &match_start, replace_text, -1);
        gtk_text_buffer_get_start_iter(buffer, &cursor);
        replace_count++;
    }

    gtk_text_buffer_end_user_action(buffer);
    
    return replace_count;
}

static void replace(GtkTextBuffer *buffer, const gchar *search_text,
const gchar *replace_text, gboolean ignore_case, gboolean wrap,
gboolean reverse, int algorithm, gboolean find_next) {
    GtkTextIter insert, selection_bound;

    gtk_text_buffer_get_selection_bounds(buffer, &insert, &selection_bound);
    gchar *selected = gtk_text_buffer_get_text(buffer, &insert,
      &selection_bound, FALSE);

    if(ignore_case? strcasecmp(selected, search_text)
      : strcmp(selected, search_text)) {
        /* if the text is NOT selected, "find" again to select the text */
        find(buffer, search_text, ignore_case, wrap, reverse, algorithm);
        g_free(selected);
        return; /* do nothing, wait for the user to click replace again */
    }

    /* Replacing counts as one action for Undo */
    gtk_text_buffer_begin_user_action(buffer);
    gtk_text_buffer_delete(buffer, &insert, &selection_bound);
    gtk_text_buffer_insert(buffer, &insert, replace_text, -1);
    gtk_text_buffer_end_user_action(buffer);

    /* Find the next occurrence of the text */
    if(find_next)
        find(buffer, search_text, ignore_case, wrap, reverse, algorithm);
    
    g_free(selected);
}

/*
 * CALLBACKS
 */

void
after_find_dialog_realize              (GtkWidget       *widget,
                                        gpointer         user_data)
{
    /* Set the find algorithm to "contains" */
    gtk_combo_box_set_active(
      GTK_COMBO_BOX(lookup_widget(widget, "find_algorithm")), FIND_CONTAINS);
}


void
on_find_text_changed                   (GtkEditable     *editable,
                                        gpointer         user_data)
{
    /* Do not free or modify the strings from gtk_entry_get_text */
    const gchar *text = gtk_entry_get_text(GTK_ENTRY(editable));
    gboolean text_not_empty = !(text == NULL || strlen(text) == 0);
    gtk_widget_set_sensitive(
      lookup_widget(GTK_WIDGET(editable), "find_next"), text_not_empty);
    gtk_widget_set_sensitive(
      lookup_widget(GTK_WIDGET(editable), "find_previous"), text_not_empty);
    gtk_widget_set_sensitive(
      lookup_widget(GTK_WIDGET(editable), "find_replace_find"), text_not_empty);
    gtk_widget_set_sensitive(
      lookup_widget(GTK_WIDGET(editable), "find_replace_all"), text_not_empty);
}


void
on_find_close_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
    /* Close the dialog */
    gtk_widget_destroy(gtk_widget_get_toplevel(GTK_WIDGET(button)));
}


/* Callback for when "Find Next" is clicked from a Find dialog that was
  started from an Inform 7 project */
void
on_find_next_clicked                   (GtkButton       *button,
                                        Story           *thestory)
{
    GtkWidget *thiswidget = GTK_WIDGET(button);
    find(GTK_TEXT_BUFFER(thestory->buffer),
      gtk_entry_get_text(GTK_ENTRY(lookup_widget(thiswidget, "find_text"))),
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(thiswidget,
      "find_ignore_case"))),
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(thiswidget,
      "find_wrap"))),
      FALSE /* not reverse */,
      gtk_combo_box_get_active(GTK_COMBO_BOX(lookup_widget(thiswidget,
      "find_algorithm"))));
      /* Do not free or modify the strings from gtk_entry_get_text */
    scroll_text_view_to_cursor(
      GTK_TEXT_VIEW(lookup_widget(thestory->window, "source_l")));
    scroll_text_view_to_cursor(
      GTK_TEXT_VIEW(lookup_widget(thestory->window, "source_r")));
}

/* Callback for when "Find Next" is clicked from a Find dialog that was
  started from an extension */
void
on_xfind_next_clicked                   (GtkButton       *button,
                                         Extension       *ext)
{
    GtkWidget *thiswidget = GTK_WIDGET(button);
    find(GTK_TEXT_BUFFER(ext->buffer),
      gtk_entry_get_text(GTK_ENTRY(lookup_widget(thiswidget, "find_text"))),
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(thiswidget,
      "find_ignore_case"))),
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(thiswidget,
      "find_wrap"))),
      FALSE /* not reverse */,
      gtk_combo_box_get_active(GTK_COMBO_BOX(lookup_widget(thiswidget,
      "find_algorithm"))));
      /* Do not free or modify the strings from gtk_entry_get_text */
    scroll_text_view_to_cursor(
      GTK_TEXT_VIEW(lookup_widget(ext->window, "ext_code")));
}


/* Callback for when "Find Previous" is clicked from a Find dialog that was
  started from an Inform 7 project */
void
on_find_previous_clicked               (GtkButton       *button,
                                        Story           *thestory)
{
    GtkWidget *thiswidget = GTK_WIDGET(button);
    find(GTK_TEXT_BUFFER(thestory->buffer),
      gtk_entry_get_text(GTK_ENTRY(lookup_widget(thiswidget, "find_text"))),
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(thiswidget,
      "find_ignore_case"))),
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(thiswidget,
      "find_wrap"))),
      TRUE /* reverse */,
      gtk_combo_box_get_active(GTK_COMBO_BOX(lookup_widget(thiswidget,
      "find_algorithm"))));
      /* Do not free or modify the strings from gtk_entry_get_text */
    scroll_text_view_to_cursor(
      GTK_TEXT_VIEW(lookup_widget(thestory->window, "source_l")));
    scroll_text_view_to_cursor(
      GTK_TEXT_VIEW(lookup_widget(thestory->window, "source_r")));
}

/* Callback for when "Find Previous" is clicked from a Find dialog that was
  started from an extension */
void
on_xfind_previous_clicked              (GtkButton       *button,
                                        Extension       *ext)
{
    GtkWidget *thiswidget = GTK_WIDGET(button);
    find(GTK_TEXT_BUFFER(ext->buffer),
      gtk_entry_get_text(GTK_ENTRY(lookup_widget(thiswidget, "find_text"))),
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(thiswidget,
      "find_ignore_case"))),
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(thiswidget,
      "find_wrap"))),
      TRUE /* reverse */,
      gtk_combo_box_get_active(GTK_COMBO_BOX(lookup_widget(thiswidget,
      "find_algorithm"))));
      /* Do not free or modify the strings from gtk_entry_get_text */
    scroll_text_view_to_cursor(
      GTK_TEXT_VIEW(lookup_widget(ext->window, "ext_code")));
}

/* Callback for when "Replace" is clicked from a Find dialog that was
  started from an I7 project */
void
on_find_replace_find_clicked           (GtkButton       *button,
                                        Story           *thestory)
{
    GtkWidget *thiswidget = GTK_WIDGET(button);
    replace(GTK_TEXT_BUFFER(thestory->buffer),
      gtk_entry_get_text(GTK_ENTRY(lookup_widget(thiswidget,
      "find_text"))),
      gtk_entry_get_text(GTK_ENTRY(lookup_widget(thiswidget,
      "find_replace_text"))),
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(thiswidget,
      "find_ignore_case"))),
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(thiswidget,
      "find_wrap"))),
      FALSE /* not reverse */,
      gtk_combo_box_get_active(GTK_COMBO_BOX(lookup_widget(thiswidget,
      "find_algorithm"))),
      TRUE /* go to next occurrence */);
    scroll_text_view_to_cursor(
      GTK_TEXT_VIEW(lookup_widget(thestory->window, "source_l")));
    scroll_text_view_to_cursor(
      GTK_TEXT_VIEW(lookup_widget(thestory->window, "source_r")));
}

/* Callback for when "Replace" is clicked from a Find dialog that was
  started from an extension */
void
on_xfind_replace_find_clicked          (GtkButton       *button,
                                        Extension       *ext)
{
    GtkWidget *thiswidget = GTK_WIDGET(button);
    replace(GTK_TEXT_BUFFER(ext->buffer),
      gtk_entry_get_text(GTK_ENTRY(lookup_widget(thiswidget,
      "find_text"))),
      gtk_entry_get_text(GTK_ENTRY(lookup_widget(thiswidget,
      "find_replace_text"))),
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(thiswidget,
      "find_ignore_case"))),
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(thiswidget,
      "find_wrap"))),
      FALSE /* not reverse */,
      gtk_combo_box_get_active(GTK_COMBO_BOX(lookup_widget(thiswidget,
      "find_algorithm"))),
      TRUE /* go to next occurrence */);
    scroll_text_view_to_cursor(
      GTK_TEXT_VIEW(lookup_widget(ext->window, "ext_code")));
}

/* Callback for when "Replace All" is clicked from a Find dialog that was
started from an I7 project */
void
on_find_replace_all_clicked        (GtkButton       *button,
                                    Story           *thestory)
{
    GtkWidget *thiswidget = GTK_WIDGET(button);
    int replace_count = replace_all(
      GTK_TEXT_BUFFER(thestory->buffer),
      gtk_entry_get_text(GTK_ENTRY(lookup_widget(thiswidget, 
        "find_text"))),
      gtk_entry_get_text(GTK_ENTRY(lookup_widget(thiswidget,
        "find_replace_text"))),
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(thiswidget,
        "find_ignore_case"))),
      gtk_combo_box_get_active(GTK_COMBO_BOX(lookup_widget(thiswidget,
        "find_algorithm"))));
      /* Do not free or modify the strings from gtk_entry_get_text */

    gtk_widget_destroy(gtk_widget_get_toplevel(thiswidget));

    GtkWidget *dialog = gtk_message_dialog_new(
      GTK_WINDOW(thestory->window),
      GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
      "%d occurences replaced.", replace_count);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

/* Callback for when "Replace All" is clicked from a Find dialog that was
started from an extension */
void
on_xfind_replace_all_clicked        (GtkButton       *button,
                                     Extension       *ext)
{
    GtkWidget *thiswidget = GTK_WIDGET(button);
    int replace_count = replace_all(
      GTK_TEXT_BUFFER(ext->buffer),
      gtk_entry_get_text(GTK_ENTRY(lookup_widget(thiswidget,
        "find_text"))),
      gtk_entry_get_text(GTK_ENTRY(lookup_widget(thiswidget,
        "find_replace_text"))),
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(thiswidget,
        "find_ignore_case"))),
      gtk_combo_box_get_active(GTK_COMBO_BOX(lookup_widget(thiswidget,
        "find_algorithm"))));
      /* Do not free or modify the strings from gtk_entry_get_text */

    gtk_widget_destroy(gtk_widget_get_toplevel(thiswidget));

    GtkWidget *dialog = gtk_message_dialog_new(
      GTK_WINDOW(ext->window),
      GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
      "%d occurences replaced.", replace_count);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}
