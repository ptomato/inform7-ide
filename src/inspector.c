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

#include "inspector.h"
#include "configfile.h"
#include "support.h"
#include "story.h"

/* The global pointer to the inspector window */
GtkWidget *inspector_window;

void
after_inspector_window_realize         (GtkWidget       *widget,
                                        gpointer         user_data)
{
    update_inspectors();
}

/* Show or hide the inspectors according to the user's preferences */
void update_inspectors() {
    /* Show the message that no inspectors are showing; then, if one is showing,
    hide it again */
    gtk_widget_show(lookup_widget(inspector_window, "no_inspector"));
    show_inspector(INSPECTOR_NOTES,
      config_file_get_bool("Inspectors", "Notes"));
    show_inspector(INSPECTOR_HEADINGS,
      config_file_get_bool("Inspectors", "Headings"));
    show_inspector(INSPECTOR_SKEIN,
      config_file_get_bool("Inspectors", "Skein"));
    show_inspector(INSPECTOR_SEARCH_FILES,
      config_file_get_bool("Inspectors", "Search"));
}

/* Show or hide the inspector in the inspector window */
void show_inspector(int which, gboolean show) {
    GtkWidget *inspector = NULL;
    switch(which) {
      case INSPECTOR_NOTES:
        inspector = lookup_widget(inspector_window, "notes_inspector");
        break;
      case INSPECTOR_HEADINGS:
        inspector = lookup_widget(inspector_window, "headings_inspector");
        break;
      case INSPECTOR_SKEIN:
        inspector = lookup_widget(inspector_window, "skein_inspector");
        break;
      case INSPECTOR_SEARCH_FILES:
        inspector = lookup_widget(inspector_window, "search_inspector");
        break;
      default:
        return;
    }
    if(show) {
        gtk_widget_show(inspector);
        gtk_widget_hide(lookup_widget(inspector_window, "no_inspector"));
    } else
        gtk_widget_hide(inspector);
}

/* Display the data from the story in the inspector. (Do not check whether we
   are already displaying the data from the same story, because this function is
   also called when we just want to refresh the data. */
void refresh_inspector(struct story *thestory) {
    GtkWidget *widget = lookup_widget(inspector_window, "notes");
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(widget), thestory->notes);
}
