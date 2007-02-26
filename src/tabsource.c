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
#include <gtksourceview/gtksourceview.h>
#include <gtksourceview/gtksourcebuffer.h>
#include <gtksourceview/gtksourcelanguage.h>
#include <gtksourceview/gtksourcelanguagesmanager.h>

#include "tabsource.h"
#include "appwindow.h"
#include "colorscheme.h"
#include "story.h"
#include "support.h"
#include "prefs.h"
#include "configfile.h"
#include "inspector.h"

/* Create our default source view */
GtkWidget*
source_create (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2)
{
    GtkWidget *source = gtk_source_view_new();
    gtk_widget_set_name(source, widget_name);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(source), GTK_WRAP_WORD);
    update_font(source);
    update_tabs(GTK_SOURCE_VIEW(source));
    return source;
}

GtkTextTag *create_string_markup_tag() {
    gint colors = config_file_get_int("Colors", "ChangeColors");
    gint styling = config_file_get_int("Fonts", "FontStyling");
    GdkColor clr = get_scheme_color((colors == CHANGE_COLORS_NEVER)?
      CLR_TEXT : ((colors == CHANGE_COLORS_OCCASIONALLY)?
      CLR_STRING : CLR_STRING_MARKUP));
    
    GtkTextTag *markup = gtk_text_tag_new("string-markup");
    g_object_set(G_OBJECT(markup),
      "foreground-set", TRUE,
      "foreground-gdk", &clr,
      "style-set", TRUE,
      "style", (styling == FONT_STYLING_OFTEN)? PANGO_STYLE_ITALIC :
      PANGO_STYLE_NORMAL,
      "weight-set", TRUE,
      "weight", PANGO_WEIGHT_NORMAL,
      NULL);
    return markup;
}

/* Create our default source buffer with Natural Inform highlighting */
GtkSourceBuffer *create_natural_inform_source_buffer() {
    GtkSourceBuffer *buffer = gtk_source_buffer_new(NULL);

    /* Set up the Natural Inform highlighting */
    GtkSourceLanguage *language;
    GtkSourceLanguagesManager *lmanager;
    GList ldirs;
    
    gchar *specfile = get_datafile_path("naturalinform.lang");
    ldirs.data = g_path_get_dirname(specfile);
    g_free(specfile);

    ldirs.prev = NULL;
    ldirs.next = NULL;
    lmanager = GTK_SOURCE_LANGUAGES_MANAGER(g_object_new(
      GTK_TYPE_SOURCE_LANGUAGES_MANAGER, "lang_files_dirs", &ldirs, NULL));
    language = gtk_source_languages_manager_get_language_from_mime_type(
      lmanager, "text/x-natural-inform");
    if(language != NULL) {
        set_highlight_styles(language);
		gtk_source_buffer_set_highlight(buffer, TRUE);
		gtk_source_buffer_set_language(buffer, language);
    }
    g_object_unref((gpointer)lmanager);
    
    /* Create the "string markup" tag and add it to the buffer's tag table */
    gtk_text_tag_table_add(
      gtk_text_buffer_get_tag_table(GTK_TEXT_BUFFER(buffer)), 
      create_string_markup_tag());
    
    /* Turn off highlighting matching brackets */
    gtk_source_buffer_set_check_brackets(buffer, FALSE);
    
    /* Connect signals for intelligent syntax following */
    g_signal_connect_after(buffer, "delete-range", 
      (GCallback)after_source_buffer_delete_range, NULL);
    g_signal_connect_after(buffer, "insert-text",
      (GCallback)after_source_buffer_insert_text, NULL);
    
    return buffer;
}

/* Paste code at the current insert position in buffer, deleting any selection*/
void paste_code(GtkSourceBuffer *buffer, gchar *code) {
    gtk_text_buffer_delete_selection(GTK_TEXT_BUFFER(buffer), TRUE, TRUE);
    /* Delete selection does nothing if there is no selection */
    gtk_text_buffer_insert_at_cursor(GTK_TEXT_BUFFER(buffer), code, -1);
}

/* Scroll the source views to the specified line of the source */
void jump_to_line(GtkWidget *widget, gint line) {
    struct story *thestory = get_story(widget);
    GtkTextIter cursor, line_end;
    
    int right = choose_notebook(widget, TAB_SOURCE);
    gtk_notebook_set_current_page(get_notebook(widget, right), TAB_SOURCE);
    GtkWidget *view = lookup_widget(thestory->window, 
      right? "source_r" : "source_l");
    
    gtk_text_buffer_get_iter_at_line(GTK_TEXT_BUFFER(thestory->buffer), &cursor,
      line - 1); /* line is counted from 0 */
    line_end = cursor;
    if(!gtk_text_iter_ends_line(&line_end))
        /* if already at end, this will push it to the end of the NEXT line */
        gtk_text_iter_forward_to_line_end(&line_end);
    gtk_text_buffer_select_range(GTK_TEXT_BUFFER(thestory->buffer), &cursor,
      &line_end);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(view),
      gtk_text_buffer_get_insert(GTK_TEXT_BUFFER(thestory->buffer)),
      0.25, FALSE, 0.0, 0.0);
    gtk_widget_grab_focus(view);
}

void after_source_buffer_delete_range(GtkTextBuffer *buffer, GtkTextIter *start,
  GtkTextIter *end, gpointer data) {
    if(!config_file_get_bool("Syntax", "Intelligence"))
        return;
    /* Do the extra highlighting anytime text is deleted, because running after
    the default signal handler means we have no access to the deleted text. */
    if(config_file_get_bool("Syntax", "Highlighting"))
        g_idle_add((GSourceFunc)do_extra_highlighting, (gpointer)buffer);
    /* Reindex the section headings now for the same reason */
    if(config_file_get_bool("Syntax", "IntelligentIndexInspector"))
        g_idle_add((GSourceFunc)reindex_headings, NULL);
}

void after_source_buffer_insert_text(GtkTextBuffer *buffer,
  GtkTextIter *location, gchar *text, gint len, gpointer data) {
    /* If the inserted text ended in a newline, then do auto-indenting */
    /* We could use gtk_source_view_set_auto_indent(), but that auto-indents
      leading spaces as well as tabs, and we don't want that */
    if(g_str_has_suffix(text, "\n") &&
      config_file_get_bool("Syntax", "AutoIndent")) {
        int tab_count = 0;
        GtkTextIter prev_line = *location;
        gtk_text_iter_backward_line(&prev_line);
        while(gtk_text_iter_get_char(&prev_line) == '\t') {
            gtk_text_iter_forward_char(&prev_line);
            tab_count++;
        }
        gchar *tabs = g_strnfill(tab_count, '\t');
        gtk_text_buffer_insert_at_cursor(buffer, tabs, -1);
    }
    
    /* Return after that if we are not doing intelligent symbol following */    
    if(!config_file_get_bool("Syntax", "Intelligence"))
        return;
    
    /* if the inserted text contains a [, ] or ", then the highlighting for the
    markup brackets may have changed, so redo the extra highlighting */
    if(config_file_get_bool("Syntax", "Highlighting") &&
      (strchr(text, '[') || strchr(text, ']') || strchr(text, '\"'))) {
        g_idle_add((GSourceFunc)do_extra_highlighting, (gpointer)buffer);
    }
    
    /* For any text, a section heading might have been entered or changed, so
    reindex the section headings */
    if(config_file_get_bool("Syntax", "IntelligentIndexInspector")) {
        g_idle_add((GSourceFunc)reindex_headings, NULL);
    }
}


gboolean do_extra_highlighting(gpointer data) {
    GtkTextIter iter;
    gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(data), &iter);
    GtkTextIter start, end, stringstart;
    
    /* Remove the string markup tags */
    gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(data), &start);
    gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(data), &end);
    gtk_text_buffer_remove_tag_by_name(GTK_TEXT_BUFFER(data), "string-markup",
      &start, &end);
    
    /* Quit here if we are not using syntax highlighting or intelligent symbol
    following (this is not redundant, because this function can be called from
    other places than the signal handlers */
    if(!config_file_get_bool("Syntax", "Intelligence")
      || !config_file_get_bool("Syntax", "Highlighting"))
        return FALSE;
    
    /* Search the text for brackets within quotes */
    start = end = stringstart = iter;
    gunichar pos;
    while((pos = gtk_text_iter_get_char(&iter)) != 0) {
        if(pos == '\"') {
            stringstart = iter;
            gtk_text_iter_forward_char(&iter);
            while((pos = gtk_text_iter_get_char(&iter)) != '\"' && pos != '\0'){
                if(pos == '[') {
                    start = iter;
                    gtk_text_iter_forward_char(&iter);
                } else if(pos == ']') {
                    /* don't highlight if the ] was unmatched */
                    if(gtk_text_iter_compare(&start, &stringstart) > 0
                      && gtk_text_iter_compare(&start, &end) >= 0) {
                        gtk_text_iter_forward_char(&iter);
                        end = iter;
                        gtk_text_buffer_apply_tag_by_name(GTK_TEXT_BUFFER(data),
                          "string-markup", &start, &end);
                    }
                } else
                    gtk_text_iter_forward_char(&iter);
            }
            /* If there was an unmatched [ in the string */
            if(gtk_text_iter_compare(&start, &end) > 0) {
                end = iter;
                gtk_text_buffer_apply_tag_by_name(GTK_TEXT_BUFFER(data),
                  "string-markup", &start, &end);
            }
        }
        gtk_text_iter_forward_char(&iter);
    }
    
    return FALSE; /* one-shot idle function */
}
