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

#include "prefs.h"
#include "tabsource.h"

/* Custom function to create a GtkSourceView; if 'string1' is not NULL or empty,
then it becomes the contents of the buffer. If 'int1' is 0, then it is a regular
buffer, otherwise a Natural Inform source buffer. In the latter case, if 'int2'
is also nonzero, it will be a Natural Inform Extension source buffer. */
GtkWidget *
custom_gtk_source_view_create(gchar *widget_name, gchar *string1, 
                              gchar *string2, gint int1, gint int2)
{
    GtkWidget *source;
    
    if(string1 && strlen(string1)) {
        if(int1) {
            GtkSourceBuffer *buffer = 
                create_natural_inform_source_buffer(int2 != 0);
            source = gtk_source_view_new_with_buffer(buffer);
            gtk_text_buffer_set_text(GTK_TEXT_BUFFER(buffer), string1, -1);
        } else {
            source = gtk_source_view_new();
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(
              GTK_TEXT_VIEW(source));
            gtk_text_buffer_set_text(buffer, string1, -1);
        }
    } else
        source = gtk_source_view_new();
    
    gtk_widget_set_name(source, widget_name);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(source), GTK_WRAP_WORD);
    update_font(source);
    update_tabs(GTK_SOURCE_VIEW(source));
    return source;
}

/* Create a GtkSourceView and -Buffer and fill it with the example text */
/* Damn glade doesn't let you enter newlines or tabs */
GtkWidget *
source_example_create(gchar *widget_name, gchar *string1, gchar *string2,
                      gint int1, gint int2)
{
    return custom_gtk_source_view_create(widget_name, 
      "\nPart One - the Wharf\n\nThe Customs Wharf is a room. [change the "
      "description if the cask is open] \"Amid the bustle of the quayside, [if "
      "the case is open]many eyes stray to your broached cask. "
      "[otherwise]nobody takes much notice of a man heaving a cask about. "
      "[end if]Sleek gondolas jostle at the plank pier.\"",
      "", 1, 0);
}

/* Create another GtkSourceView with examples of tab stops */
GtkWidget *
tab_example_create(gchar *widget_name, gchar *string1, gchar *string2,
                   gint int1, gint int2)
{
    return custom_gtk_source_view_create(widget_name,
      _("\tTab\tTab\tTab\tTab\tTab\tTab\tTab"), "", 0, 0);
}
