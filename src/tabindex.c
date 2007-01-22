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

#include "html.h"
#include "story.h"
#include "support.h"

/* Load blank pages in all the index tabs */
void blank_index_tabs(GtkWidget *thiswidget) {
    html_load_blank(GTK_HTML(lookup_widget(thiswidget, "actions_l")));
    html_load_blank(GTK_HTML(lookup_widget(thiswidget, "actions_r")));
    html_load_blank(GTK_HTML(lookup_widget(thiswidget, "contents_l")));
    html_load_blank(GTK_HTML(lookup_widget(thiswidget, "contents_r")));
    html_load_blank(GTK_HTML(lookup_widget(thiswidget, "kinds_l")));
    html_load_blank(GTK_HTML(lookup_widget(thiswidget, "kinds_r")));
    html_load_blank(GTK_HTML(lookup_widget(thiswidget, "phrasebook_l")));
    html_load_blank(GTK_HTML(lookup_widget(thiswidget, "phrasebook_r")));
    html_load_blank(GTK_HTML(lookup_widget(thiswidget, "rules_l")));
    html_load_blank(GTK_HTML(lookup_widget(thiswidget, "rules_r")));
    html_load_blank(GTK_HTML(lookup_widget(thiswidget, "scenes_l")));
    html_load_blank(GTK_HTML(lookup_widget(thiswidget, "scenes_r")));
    html_load_blank(GTK_HTML(lookup_widget(thiswidget, "world_l")));
    html_load_blank(GTK_HTML(lookup_widget(thiswidget, "world_r")));
}

/* Helper function to check whether an index file exists and to load it, or a
blank page if it doesn't exist. basename MUST end in 'l'. Not for use outside
the function 'reload_index_tabs'. */
static void check_and_load(GtkWidget *thiswidget, struct story *thestory,
const gchar *basename, const gchar *file) {
    gchar *filename = g_build_filename(thestory->filename, "Index", file, NULL);
    gchar *wname = g_strdup(basename);
    
    if(g_file_test(filename, G_FILE_TEST_EXISTS)) {
        html_load_file(GTK_HTML(lookup_widget(thiswidget, wname)), filename);
        *(strrchr(wname, 'l')) = 'r';
        html_load_file(GTK_HTML(lookup_widget(thiswidget, wname)), filename);
    } else {
        html_load_blank(GTK_HTML(lookup_widget(thiswidget, wname)));
        *(strrchr(wname, 'l')) = 'r';
        html_load_blank(GTK_HTML(lookup_widget(thiswidget, wname)));
    }
    
    g_free(wname);
    g_free(filename);
}

/* Load all the correct files in the index tabs, if they exist */
void reload_index_tabs(GtkWidget *thiswidget) {
    struct story *thestory = get_story(thiswidget);
    
    check_and_load(thiswidget, thestory, "actions_l", "Actions.html");
    check_and_load(thiswidget, thestory, "contents_l", "Contents.html");
    check_and_load(thiswidget, thestory, "kinds_l", "Kinds.html");
    check_and_load(thiswidget, thestory, "phrasebook_l", "Phrasebook.html");
    check_and_load(thiswidget, thestory, "rules_l", "Rules.html");
    check_and_load(thiswidget, thestory, "scenes_l", "Scenes.html");
    check_and_load(thiswidget, thestory, "world_l", "World.html");
}
