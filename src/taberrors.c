/*  Copyright 2007 P.F. Chimento
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
#include <gtksourceview/gtksourcebuffer.h>
#include <gtksourceview/gtksourcelanguage.h>
#include <gtksourceview/gtksourcelanguagesmanager.h>

#include "support.h"

#include "colorscheme.h"
#include "datafile.h"

/* Add the debugging tabs to this main window */
void add_debug_tabs(GtkWidget *window) {
    gtk_widget_show(lookup_widget(window, "debugging_page_l"));
    gtk_widget_show(lookup_widget(window, "debugging_page_r"));
    gtk_widget_show(lookup_widget(window, "inform6_page_l"));
    gtk_widget_show(lookup_widget(window, "inform6_page_r"));
}

/* Remove the debugging tabs from this window */
void remove_debug_tabs(GtkWidget *window) {
    gtk_widget_hide(lookup_widget(window, "debugging_page_l"));
    gtk_widget_hide(lookup_widget(window, "debugging_page_r"));
    gtk_widget_hide(lookup_widget(window, "inform6_page_l"));
    gtk_widget_hide(lookup_widget(window, "inform6_page_r"));
}

/* Set up the Inform 6 highlighting */
GtkSourceBuffer *create_inform6_source_buffer() {
    GtkSourceBuffer *i6buffer = gtk_source_buffer_new(NULL);
    GtkSourceLanguage *language;
    GtkSourceLanguagesManager *lmanager;
    GList ldirs;
    
    gchar *specfile = get_datafile_path("inform.lang");

    ldirs.data = g_path_get_dirname(specfile);
    ldirs.prev = NULL;
    ldirs.next = NULL;
    lmanager = GTK_SOURCE_LANGUAGES_MANAGER(g_object_new(
      GTK_TYPE_SOURCE_LANGUAGES_MANAGER, "lang_files_dirs", &ldirs, NULL));
    language = gtk_source_languages_manager_get_language_from_mime_type(
      lmanager, "text/x-inform");
    if(language != NULL) {
        set_highlight_styles(language);
		gtk_source_buffer_set_highlight(i6buffer, TRUE);
		gtk_source_buffer_set_language(i6buffer, language);
    }
    
    g_object_unref((gpointer)lmanager);
    return i6buffer;
}
