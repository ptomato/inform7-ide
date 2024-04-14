/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Philip Chimento <philip.chimento@gmail.com>
 */

#include "config.h"

#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>

#include "lang.h"
#include "story.h"

/* Add the debugging tabs to this main window */
void
i7_story_add_debug_tabs(I7Story *story)
{
	gtk_widget_show(GTK_WIDGET(story->panel[LEFT]->debugging_scrolledwindow));
	gtk_widget_show(GTK_WIDGET(story->panel[RIGHT]->debugging_scrolledwindow));
	gtk_widget_show(GTK_WIDGET(story->panel[LEFT]->inform6_scrolledwindow));
	gtk_widget_show(GTK_WIDGET(story->panel[RIGHT]->inform6_scrolledwindow));
}

/* Remove the debugging tabs from this window */
void
i7_story_remove_debug_tabs(I7Story *story)
{
	gtk_widget_hide(GTK_WIDGET(story->panel[LEFT]->debugging_scrolledwindow));
	gtk_widget_hide(GTK_WIDGET(story->panel[RIGHT]->debugging_scrolledwindow));
	gtk_widget_hide(GTK_WIDGET(story->panel[LEFT]->inform6_scrolledwindow));
	gtk_widget_hide(GTK_WIDGET(story->panel[RIGHT]->inform6_scrolledwindow));
}

/* Set up the Inform 6 highlighting */
GtkSourceBuffer *
create_inform6_source_buffer()
{
	GtkSourceBuffer *i6buffer = gtk_source_buffer_new(NULL);

	set_buffer_language(i6buffer, "inform");
	I7App *theapp = I7_APP(g_application_get_default());
	gtk_source_buffer_set_style_scheme(i6buffer, i7_app_get_current_color_scheme(theapp));
	gtk_source_buffer_set_highlight_syntax(i6buffer, TRUE);
	return i6buffer;
}
