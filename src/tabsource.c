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

/* Create our default source view */
GtkWidget*
source_create (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2)
{
    GtkWidget *source = gtk_source_view_new();
    gtk_widget_set_name(source, widget_name);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(source), GTK_WRAP_WORD);
    update_font(source);
    return source;
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
    gtk_source_buffer_set_check_brackets(buffer, FALSE);
    
    g_free(specfile);
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
    GtkTextIter cursor;
    
    int right = choose_notebook(widget, TAB_SOURCE);
    gtk_notebook_set_current_page(get_notebook(widget, right), TAB_SOURCE);
    
    gtk_text_buffer_get_iter_at_line(GTK_TEXT_BUFFER(thestory->buffer), &cursor,
      line - 1); /* line is counted from 0 */
    gtk_text_buffer_place_cursor(GTK_TEXT_BUFFER(thestory->buffer), &cursor);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(lookup_widget(thestory->window,
      right? "source_r" : "source_l")),
      gtk_text_buffer_get_insert(GTK_TEXT_BUFFER(thestory->buffer)),
      0.25, FALSE, 0.0, 0.0);
    gtk_widget_grab_focus(lookup_widget(thestory->window,
      right? "source_r" : "source_l"));
}
