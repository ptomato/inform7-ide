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

#include "support.h"

#include "appwindow.h"
#include "colorscheme.h"
#include "configfile.h"
#include "datafile.h"
#include "inspector.h"
#include "prefs.h"
#include "story.h"
#include "lang.h"
#include "tabsource.h"

static void 
after_source_buffer_delete_range(GtkTextBuffer *buffer, GtkTextIter *start, 
                                 GtkTextIter *end, gpointer data) 
{
    if(!config_file_get_bool("SyntaxSettings", "Intelligence"))
        return;
    /* Reindex the section headings anytime text is deleted, because running after
    the default signal handler means we have no access to the deleted text. */
    if(config_file_get_bool("SyntaxSettings", "IntelligentHeadingsInspector")) {
        g_idle_remove_by_data(GINT_TO_POINTER(IDLE_REINDEX_HEADINGS));
        g_idle_add((GSourceFunc)reindex_headings,
          GINT_TO_POINTER(IDLE_REINDEX_HEADINGS));
    }
}

static void 
after_source_buffer_insert_text(GtkTextBuffer *buffer, GtkTextIter *location, 
                                gchar *text, gint len, gpointer data)
{
    /* If the inserted text ended in a newline, then do auto-indenting */
    /* We could use gtk_source_view_set_auto_indent(), but that auto-indents
      leading spaces as well as tabs, and we don't want that */
    if(g_str_has_suffix(text, "\n") &&
      config_file_get_bool("SyntaxSettings", "AutoIndent")) {
        int tab_count = 0;
        GtkTextIter prev_line = *location;
        gtk_text_iter_backward_line(&prev_line);
        while(gtk_text_iter_get_char(&prev_line) == '\t') {
            gtk_text_iter_forward_char(&prev_line);
            tab_count++;
        }
        gchar *tabs = g_strnfill(tab_count, '\t');
        /* Preserve and restore iter position by creating a mark with right
          gravity (FALSE) */
        GtkTextMark *bookmark = gtk_text_buffer_create_mark(buffer, "bookmark",
                                                            location, FALSE);
        gtk_text_buffer_insert_at_cursor(buffer, tabs, -1);
        gtk_text_buffer_get_iter_at_mark(buffer, location, bookmark);
        gtk_text_buffer_delete_mark(buffer, bookmark);
    }
    
    /* Return after that if we are not doing intelligent symbol following */    
    if(!config_file_get_bool("SyntaxSettings", "Intelligence"))
        return;
    
    /* For any text, a section heading might have been entered or changed, so
    reindex the section headings */
    if(config_file_get_bool("SyntaxSettings", "IntelligentHeadingsInspector")) {
        g_idle_remove_by_data(GINT_TO_POINTER(IDLE_REINDEX_HEADINGS));
        g_idle_add((GSourceFunc)reindex_headings,
          GINT_TO_POINTER(IDLE_REINDEX_HEADINGS));
    }
    
    /* If the text ends with a space, check whether it is a section heading that
    needs auto-numbering */
    if(config_file_get_bool("SyntaxSettings", "AutoNumberSections")) {
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
}

/* Create our default source buffer with Natural Inform highlighting. If
extension is true, create one with the slightly different Natural Inform
Extension highlighting. */
GtkSourceBuffer *
create_natural_inform_source_buffer(gboolean extension) 
{
    GtkSourceBuffer *buffer = gtk_source_buffer_new(NULL);

    /* Set up the Natural Inform highlighting */
    set_buffer_language(buffer, extension? "inform7x" : "inform7");
    set_highlight_styles(buffer);
    gtk_source_buffer_set_highlight_syntax(buffer, TRUE);
    
    /* Turn off highlighting matching brackets */
    gtk_source_buffer_set_highlight_matching_brackets(buffer, FALSE);
    
    /* Connect signals for intelligent syntax following */
    g_signal_connect_after(buffer, "delete-range", 
      (GCallback)after_source_buffer_delete_range, NULL);
    g_signal_connect_after(buffer, "insert-text",
      (GCallback)after_source_buffer_insert_text, NULL);
	
    return buffer;
}

static void
on_headings_menu_item_activate(GtkMenuItem *menuitem, gpointer lineno)
{
    /* Only jump to the line if the menu item doesn't have a submenu; otherwise,
    it will jump when the user hovers over the item, opening the submenu */
    if(!gtk_menu_item_get_submenu(menuitem))
        jump_to_line(GTK_WIDGET(menuitem), GPOINTER_TO_UINT(lineno));
}

void
on_source_headings_show_menu(GtkMenuToolButton *menutoolbutton, gpointer data)
{
    Story *thestory = get_story(GTK_WIDGET(menutoolbutton));
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(thestory->buffer);
    GtkTextIter pos, end;
    gtk_text_buffer_get_start_iter(buffer, &pos);

    /* Destroy the previous menu */
    gtk_menu_tool_button_set_menu(menutoolbutton, NULL);
    
    GtkWidget *volume = NULL, *book = NULL, *part = NULL, *chapter = NULL,
      *section = NULL;
    GtkWidget *volume_item = NULL, *book_item = NULL, *part_item = NULL,
      *chapter_item = NULL, *section_item = NULL;
   
    while(gtk_text_iter_get_char(&pos) != 0) {
        if(gtk_text_iter_get_char(&pos) == '\n') {
            gtk_text_iter_forward_line(&pos);
            end = pos;
            gtk_text_iter_forward_line(&end);
            if(gtk_text_iter_get_char(&end) == '\n') {
                /* Preceded and followed by a blank line */
                /* Get the entire line and its line number, chop the \n */
                gchar *text = g_strchomp(gtk_text_iter_get_text(&pos, &end));
                gchar *lcase = g_utf8_strdown(text, -1);
                gint lineno = gtk_text_iter_get_line(&pos) + 1;
                /* Line numbers counted from 0 */
                
                if(g_str_has_prefix(lcase, "volume ")) {
                    if(!volume) {
                        volume = gtk_menu_new();
                        volume_item = gtk_menu_item_new_with_label("");
                        gtk_label_set_markup(GTK_LABEL(gtk_bin_get_child(
                          GTK_BIN(volume_item))), _("<b>Volume...</b>"));
                        gtk_widget_set_sensitive(volume_item, FALSE);
                        gtk_widget_show(volume_item);
                        gtk_menu_shell_append(GTK_MENU_SHELL(volume),
                          volume_item);
                        gtk_widget_show(volume);
                    }
                    volume_item = gtk_menu_item_new_with_label(text + 7);
                    gtk_widget_show(volume_item);
                    gtk_menu_shell_append(GTK_MENU_SHELL(volume), volume_item);
                    g_signal_connect(volume_item, "activate",
                      G_CALLBACK(on_headings_menu_item_activate),
                      GUINT_TO_POINTER(lineno));
                    book = part = chapter = section = NULL;
                } else if(g_str_has_prefix(lcase, "book ")) {
                    if(!book) {
                        book = gtk_menu_new();
                        book_item = gtk_menu_item_new_with_label("");
                        gtk_label_set_markup(GTK_LABEL(gtk_bin_get_child(
                          GTK_BIN(book_item))), _("<b>Book...</b>"));
                        gtk_widget_set_sensitive(book_item, FALSE);
                        gtk_widget_show(book_item);
                        gtk_menu_shell_append(GTK_MENU_SHELL(book), book_item);
                        gtk_widget_show(book);
                        if(volume)
                            gtk_menu_item_set_submenu(
                              GTK_MENU_ITEM(volume_item), book);
                    }
                    book_item = gtk_menu_item_new_with_label(text + 5);
                    gtk_widget_show(book_item);
                    gtk_menu_shell_append(GTK_MENU_SHELL(book), book_item);
                    g_signal_connect(book_item, "activate",
                      G_CALLBACK(on_headings_menu_item_activate),
                      GUINT_TO_POINTER(lineno));
                    part = chapter = section = NULL;
                } else if(g_str_has_prefix(lcase, "part ")) {
                    if(!part) {
                        part = gtk_menu_new();
                        part_item = gtk_menu_item_new_with_label("");
                        gtk_label_set_markup(GTK_LABEL(gtk_bin_get_child(
                          GTK_BIN(part_item))), _("<b>Part...</b>"));
                        gtk_widget_set_sensitive(part_item, FALSE);
                        gtk_widget_show(part_item);
                        gtk_menu_shell_append(GTK_MENU_SHELL(part), part_item);
                        gtk_widget_show(part);
                        if(book)
                            gtk_menu_item_set_submenu(GTK_MENU_ITEM(book_item),
                              part);
                        else if(volume)
                            gtk_menu_item_set_submenu(
                              GTK_MENU_ITEM(volume_item), part);
                    }
                    part_item = gtk_menu_item_new_with_label(text + 5);
                    gtk_widget_show(part_item);
                    gtk_menu_shell_append(GTK_MENU_SHELL(part), part_item);
                    g_signal_connect(part_item, "activate",
                      G_CALLBACK(on_headings_menu_item_activate),
                      GUINT_TO_POINTER(lineno));
                    chapter = section = NULL;
                } else if(g_str_has_prefix(lcase, "chapter ")) {
                    if(!chapter) {
                        chapter = gtk_menu_new();
                        chapter_item = gtk_menu_item_new_with_label("");
                        gtk_label_set_markup(GTK_LABEL(gtk_bin_get_child(
                          GTK_BIN(chapter_item))), _("<b>Chapter...</b>"));
                        gtk_widget_set_sensitive(chapter_item, FALSE);
                        gtk_widget_show(chapter_item);
                        gtk_menu_shell_append(GTK_MENU_SHELL(chapter),
                          chapter_item);
                        gtk_widget_show(chapter);
                        if(part)
                            gtk_menu_item_set_submenu(GTK_MENU_ITEM(part_item),
                              chapter);
                        else if(book)
                            gtk_menu_item_set_submenu(GTK_MENU_ITEM(book_item),
                              chapter);
                        else if(volume)
                            gtk_menu_item_set_submenu(
                              GTK_MENU_ITEM(volume_item), chapter);
                    }
                    chapter_item = gtk_menu_item_new_with_label(text + 8);
                    gtk_widget_show(chapter_item);
                    gtk_menu_shell_append(GTK_MENU_SHELL(chapter),chapter_item);
                    g_signal_connect(chapter_item, "activate",
                      G_CALLBACK(on_headings_menu_item_activate),
                      GUINT_TO_POINTER(lineno));
                    section = NULL;
                } else if(g_str_has_prefix(lcase, "section ")) {
                    if(!section) {
                        section = gtk_menu_new();
                        section_item = gtk_menu_item_new_with_label("");
                        gtk_label_set_markup(GTK_LABEL(gtk_bin_get_child(
                          GTK_BIN(section_item))), _("<b>Section...</b>"));
                        gtk_widget_set_sensitive(section_item, FALSE);
                        gtk_widget_show(section_item);
                        gtk_menu_shell_append(GTK_MENU_SHELL(section),
                          section_item);
                        gtk_widget_show(section);
                        if(chapter)
                            gtk_menu_item_set_submenu(
                              GTK_MENU_ITEM(chapter_item), section);
                        else if(part)
                            gtk_menu_item_set_submenu(GTK_MENU_ITEM(part_item),
                              section);
                        else if(book)
                            gtk_menu_item_set_submenu(GTK_MENU_ITEM(book_item),
                              section);
                        else if(volume)
                            gtk_menu_item_set_submenu(
                              GTK_MENU_ITEM(volume_item), section);
                    }
                    section_item = gtk_menu_item_new_with_label(text + 8);
                    gtk_widget_show(section_item);
                    gtk_menu_shell_append(GTK_MENU_SHELL(section),section_item);
                    g_signal_connect(section_item, "activate",
                      G_CALLBACK(on_headings_menu_item_activate),
                      GUINT_TO_POINTER(lineno));
                }
                g_free(text);
                g_free(lcase);
            }
	    gtk_text_iter_backward_line(&pos);
        }
       	gtk_text_iter_forward_line(&pos);
    }
    
    /* Set the appropriate menu as the drop-down menu of the button, or an empty
    menu if no headings */
    gtk_menu_tool_button_set_menu(menutoolbutton, volume? volume : book? book :
      part? part : chapter? chapter : section? section : gtk_menu_new());
}

/* Paste code at the current insert position in buffer, deleting any selection*/
void 
paste_code(GtkSourceBuffer *buffer, gchar *code) 
{
    gtk_text_buffer_delete_selection(GTK_TEXT_BUFFER(buffer), TRUE, TRUE);
    /* Delete selection does nothing if there is no selection */
    gtk_text_buffer_insert_at_cursor(GTK_TEXT_BUFFER(buffer), code, -1);
}

/* Scroll the source views to the specified line of the source */
void 
jump_to_line(GtkWidget *widget, guint line)
{
    Story *thestory = get_story(widget);
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

/* Look for all the section headings and renumber them */
void 
renumber_sections(GtkTextBuffer *buffer) 
{
    GtkTextIter pos, end;
    int volume = 1, book = 1, part = 1, chapter = 1, section = 1;
    
    gtk_text_buffer_get_start_iter(buffer, &pos);
    
    while(gtk_text_iter_get_char(&pos) != 0) {
        if(gtk_text_iter_get_char(&pos) == '\n') {
            gtk_text_iter_forward_line(&pos);
            end = pos;
            gboolean not_last = gtk_text_iter_forward_line(&end);
            if(!not_last || gtk_text_iter_get_char(&end) == '\n') {
                /* Preceded and followed by a blank line,
                or only preceded by one and current line is last line */
                /* Get the entire line and its line number, chop the \n */
                gchar *text = gtk_text_iter_get_text(&pos, &end);
                gchar *lcase = g_utf8_strdown(text, -1);
                gchar *title = strchr(text, '-');
                if(title) {
                    if(g_str_has_suffix(title, "\n"))
                        *(strrchr(title, '\n')) = '\0'; /* remove trailing \n */
                }
                gchar *newtitle;
                
                if(g_str_has_prefix(lcase, "volume")) {
                    newtitle = g_strdup_printf("Volume %d %s\n", volume++,
                      title);
                    gtk_text_buffer_delete(buffer, &pos, &end);
                    gtk_text_buffer_insert(buffer, &pos, newtitle, -1);
                    g_free(newtitle);
                    book = part = chapter = section = 1;
                } else if(g_str_has_prefix(lcase, "book")) {
                    newtitle = g_strdup_printf("Book %d %s\n", book++, title);
                    gtk_text_buffer_delete(buffer, &pos, &end);
                    gtk_text_buffer_insert(buffer, &pos, newtitle, -1);
                    g_free(newtitle);
                    part = chapter = section = 1;
                } else if(g_str_has_prefix(lcase, "part")) {
                    newtitle = g_strdup_printf("Part %d %s\n", part++, title);
                    gtk_text_buffer_delete(buffer, &pos, &end);
                    gtk_text_buffer_insert(buffer, &pos, newtitle, -1);
                    g_free(newtitle);
                    chapter = section = 1;
                } else if(g_str_has_prefix(lcase, "chapter")) {
                    newtitle = g_strdup_printf("Chapter %d %s\n", chapter++,
                      title);
                    gtk_text_buffer_delete(buffer, &pos, &end);
                    gtk_text_buffer_insert(buffer, &pos, newtitle, -1);
                    g_free(newtitle);
                    section = 1;
                } else if(g_str_has_prefix(lcase, "section")) {
                    newtitle = g_strdup_printf("Section %d %s\n", section++,
                      title);
                    gtk_text_buffer_delete(buffer, &pos, &end);
                    gtk_text_buffer_insert(buffer, &pos, newtitle, -1);
                    g_free(newtitle);
                }
                g_free(text);
                g_free(lcase);
            }
        } else {
        	gtk_text_iter_forward_line(&pos);
	}
    }
}

/* Shift the selected lines in the buffer one tab to the right */
void
shift_selection_right(GtkTextBuffer *buffer)
{
    /* Adapted from gtksourceview.c */
    GtkTextIter start, end;
    gtk_text_buffer_get_selection_bounds(buffer, &start, &end);
    gint start_line = gtk_text_iter_get_line(&start);
    gint end_line = gtk_text_iter_get_line(&end);
    gint i;

    /* if the end of the selection is before the first character on a line,
    don't indent it */
    if((gtk_text_iter_get_visible_line_offset(&end) == 0)
      && (end_line > start_line))
        end_line--;

    gtk_text_buffer_begin_user_action(buffer);
    for(i = start_line; i <= end_line; i++) {
        GtkTextIter iter;
        gtk_text_buffer_get_iter_at_line(buffer, &iter, i);

        /* don't add indentation on empty lines */
        if(gtk_text_iter_ends_line(&iter))
            continue;

        gtk_text_buffer_insert(buffer, &iter, "\t", -1);
	}
	gtk_text_buffer_end_user_action(buffer);
}

/* Shift the selected lines in the buffer one tab to the left */
void
shift_selection_left(GtkTextBuffer *buffer)
{
    /* Adapted from gtksourceview.c */
    GtkTextIter start, end;
    gtk_text_buffer_get_selection_bounds(buffer, &start, &end);
    gint start_line = gtk_text_iter_get_line(&start);
    gint end_line = gtk_text_iter_get_line(&end);
    gint i;

    /* if the end of the selection is before the first character on a line,
    don't indent it */
	if((gtk_text_iter_get_visible_line_offset(&end) == 0)
      && (end_line > start_line))
        end_line--;

    gtk_text_buffer_begin_user_action(buffer);
    for(i = start_line; i <= end_line; i++) {
        GtkTextIter iter, iter2;

        gtk_text_buffer_get_iter_at_line(buffer, &iter, i);

        if(gtk_text_iter_get_char(&iter) == '\t') {
            iter2 = iter;
            gtk_text_iter_forward_char(&iter2);
            gtk_text_buffer_delete(buffer, &iter, &iter2);
        }
    }
    gtk_text_buffer_end_user_action(buffer);
}
