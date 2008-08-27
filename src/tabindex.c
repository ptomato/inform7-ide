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

#include "support.h"

#include "appwindow.h"
#include "html.h"
#include "story.h"

/* Load blank pages in all the index tabs */
/*void 
blank_index_tabs(GtkWidget *thiswidget) 
{
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
}*/

static void
load_index_file(Story *thestory, const gchar *file, const gchar *lwidget, 
                const gchar *rwidget)
{
    gchar *filename = g_build_filename(thestory->filename, "Index", file, NULL);
    if(g_file_test(filename, G_FILE_TEST_EXISTS)) {
        html_load_file(GTK_HTML(lookup_widget(thestory->window, lwidget)),
          filename);
        html_load_file(GTK_HTML(lookup_widget(thestory->window, rwidget)),
          filename);
    } else {
        html_load_blank(GTK_HTML(lookup_widget(thestory->window, lwidget)));
        html_load_blank(GTK_HTML(lookup_widget(thestory->window, rwidget)));
    }
    g_free(filename);
}

/* Idle function to check whether an index file exists and to load it, or a
blank page if it doesn't exist. */
static gboolean 
check_and_load_idle(Story *thestory) 
{
    static int counter = TAB_INDEX_FIRST;
    GtkWidget *widget = thestory->window;
    
    switch(counter) {
    case TAB_INDEX_ACTIONS:
        load_index_file(thestory, "Actions.html", "actions_l", "actions_r");
        break;
    case TAB_INDEX_CONTENTS:
        load_index_file(thestory, "Contents.html", "contents_l", 
          "contents_r");
        break;
    case TAB_INDEX_KINDS:
        load_index_file(thestory, "Kinds.html", "kinds_l", "kinds_r");
        break;
    case TAB_INDEX_PHRASEBOOK:
        load_index_file(thestory, "Phrasebook.html", "phrasebook_l", 
          "phrasebook_r");
        break;
    case TAB_INDEX_RULES:
        load_index_file(thestory, "Rules.html", "rules_l", "rules_r");
        break;
    case TAB_INDEX_SCENES:
        load_index_file(thestory, "Scenes.html", "scenes_l", "scenes_r");
        break;
    case TAB_INDEX_WORLD:
    default:
        load_index_file(thestory, "World.html", "world_l", "world_r");
        counter = TAB_INDEX_FIRST; /* next time, load the first tab */
        clear_status(widget);
        return FALSE; /* quit the cycle */
    }
        
    /* Update the status bar */
    display_status_percentage(widget,
      (gdouble)counter / (gdouble)TAB_INDEX_LAST);
    display_status_message(widget, _("Reloading index..."));
    
    counter++; /* next time, load the next tab */
    return TRUE; /* make sure there is a next time */
}

/* Load all the correct files in the index tabs, if they exist */
void 
reload_index_tabs(Story *thestory, gboolean wait) 
{
    if(wait)
        while(check_and_load_idle((gpointer)thestory))
            ;
    else
        g_idle_add((GSourceFunc)check_and_load_idle, (gpointer)thestory);
}
