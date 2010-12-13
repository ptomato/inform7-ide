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

#include <gtk/gtk.h>
#include <gtksourceview/gtksourcebuffer.h>
#include <gtksourceview/gtksourcelanguage.h>
#include <gtksourceview/gtksourcelanguagemanager.h>
#include "story.h"
#include "colorscheme.h"
#include "document.h"
#include "lang.h"


/* Add the debugging tabs to this main window */
void 
i7_story_add_debug_tabs(I7Document *document) 
{
	if(!I7_IS_STORY(document))
		return;
    gtk_widget_show(GTK_WIDGET(I7_STORY(document)->panel[LEFT]->debugging_scrolledwindow));
    gtk_widget_show(GTK_WIDGET(I7_STORY(document)->panel[RIGHT]->debugging_scrolledwindow));
    gtk_widget_show(GTK_WIDGET(I7_STORY(document)->panel[LEFT]->inform6_scrolledwindow));
    gtk_widget_show(GTK_WIDGET(I7_STORY(document)->panel[RIGHT]->inform6_scrolledwindow));
}

/* Remove the debugging tabs from this window */
void 
i7_story_remove_debug_tabs(I7Document *document) 
{
	if(!I7_IS_STORY(document))
		return;
    gtk_widget_hide(GTK_WIDGET(I7_STORY(document)->panel[LEFT]->debugging_scrolledwindow));
    gtk_widget_hide(GTK_WIDGET(I7_STORY(document)->panel[RIGHT]->debugging_scrolledwindow));
    gtk_widget_hide(GTK_WIDGET(I7_STORY(document)->panel[LEFT]->inform6_scrolledwindow));
    gtk_widget_hide(GTK_WIDGET(I7_STORY(document)->panel[RIGHT]->inform6_scrolledwindow));
}

/* Set up the Inform 6 highlighting */
GtkSourceBuffer *
create_inform6_source_buffer() 
{
    GtkSourceBuffer *i6buffer = gtk_source_buffer_new(NULL);

    set_buffer_language(i6buffer, "inform");
    set_highlight_styles(i6buffer);
    gtk_source_buffer_set_highlight_syntax(i6buffer, TRUE);
    return i6buffer;
}
