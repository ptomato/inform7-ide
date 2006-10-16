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

/* Load all the correct files in the index tabs, if they exist */
void reload_index_tabs(GtkWidget *thiswidget) {
    gchar *filename;
    struct story *thestory = get_story(thiswidget);
    
    filename = g_strconcat(thestory->filename, "/Index/Actions.html", NULL);
    if(g_file_test(filename, G_FILE_TEST_EXISTS)) {
        html_load_file(GTK_HTML(lookup_widget(thiswidget, "actions_l")),
          filename);
        html_load_file(GTK_HTML(lookup_widget(thiswidget, "actions_r")),
          filename);
    } else {
        html_load_blank(GTK_HTML(lookup_widget(thiswidget, "actions_l")));
        html_load_blank(GTK_HTML(lookup_widget(thiswidget, "actions_r")));
    }
    g_free(filename);
    
    filename = g_strconcat(thestory->filename, "/Index/Contents.html", NULL);
    if(g_file_test(filename, G_FILE_TEST_EXISTS)) {
        html_load_file(GTK_HTML(lookup_widget(thiswidget, "contents_l")),
          filename);
        html_load_file(GTK_HTML(lookup_widget(thiswidget, "contents_r")),
          filename);
    } else {
        html_load_blank(GTK_HTML(lookup_widget(thiswidget, "contents_l")));
        html_load_blank(GTK_HTML(lookup_widget(thiswidget, "contents_r")));
    }
    g_free(filename);
    
    filename = g_strconcat(thestory->filename, "/Index/Kinds.html", NULL);
    if(g_file_test(filename, G_FILE_TEST_EXISTS)) {
        html_load_file(GTK_HTML(lookup_widget(thiswidget, "kinds_l")),
          filename);
        html_load_file(GTK_HTML(lookup_widget(thiswidget, "kinds_r")),
          filename);
    } else {
        html_load_blank(GTK_HTML(lookup_widget(thiswidget, "kinds_l")));
        html_load_blank(GTK_HTML(lookup_widget(thiswidget, "kinds_r")));
    }
    g_free(filename);
    
    filename = g_strconcat(thestory->filename, "/Index/Phrasebook.html", NULL);
    if(g_file_test(filename, G_FILE_TEST_EXISTS)) {
        html_load_file(GTK_HTML(lookup_widget(thiswidget, "phrasebook_l")),
          filename);
        html_load_file(GTK_HTML(lookup_widget(thiswidget, "phrasebook_r")),
          filename);
    } else {
        html_load_blank(GTK_HTML(lookup_widget(thiswidget, "phrasebook_l")));
        html_load_blank(GTK_HTML(lookup_widget(thiswidget, "phrasebook_r")));
    }
    g_free(filename);
    
    filename = g_strconcat(thestory->filename, "/Index/Rules.html", NULL);
    if(g_file_test(filename, G_FILE_TEST_EXISTS)) {
        html_load_file(GTK_HTML(lookup_widget(thiswidget, "rules_l")),
          filename);
        html_load_file(GTK_HTML(lookup_widget(thiswidget, "rules_r")),
          filename);
    } else {
        html_load_blank(GTK_HTML(lookup_widget(thiswidget, "rules_l")));
        html_load_blank(GTK_HTML(lookup_widget(thiswidget, "rules_r")));
    }
    g_free(filename);
    
    filename = g_strconcat(thestory->filename, "/Index/Scenes.html", NULL);
    if(g_file_test(filename, G_FILE_TEST_EXISTS)) {
        html_load_file(GTK_HTML(lookup_widget(thiswidget, "scenes_l")),
          filename);
        html_load_file(GTK_HTML(lookup_widget(thiswidget, "scenes_r")),
          filename);
    } else {
        html_load_blank(GTK_HTML(lookup_widget(thiswidget, "scenes_l")));
        html_load_blank(GTK_HTML(lookup_widget(thiswidget, "scenes_r")));
    }
    g_free(filename);
    
    filename = g_strconcat(thestory->filename, "/Index/World.html", NULL);
    if(g_file_test(filename, G_FILE_TEST_EXISTS)) {
        html_load_file(GTK_HTML(lookup_widget(thiswidget, "world_l")),
          filename);
        html_load_file(GTK_HTML(lookup_widget(thiswidget, "world_r")),
          filename);
    } else {
        html_load_blank(GTK_HTML(lookup_widget(thiswidget, "world_l")));
        html_load_blank(GTK_HTML(lookup_widget(thiswidget, "world_r")));
    }
    g_free(filename);
}
