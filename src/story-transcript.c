/* Copyright (C) 2011 P. F. Chimento
 * This file is part of GNOME Inform 7.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtk/gtk.h>
#include "panel.h"

void
on_transcript_size_allocate(GtkTreeView *view, GtkAllocation *allocation, I7Panel *panel)
{
	unsigned default_width;
	g_object_get(panel->transcript_cell, "default-width", &default_width, NULL);
	if(default_width != allocation->width) {
		g_object_set(panel->transcript_cell, "default-width", allocation->width, NULL);
		gtk_tree_view_column_queue_resize(panel->transcript_column);
	}
}

/* Pop up a context menu when right-clicking on the Transcript */
gboolean
on_transcript_button_press(GtkTreeView *view, GdkEventButton *event, I7Panel *panel)
{
	/* Only react to right-clicks */
	if(event->type != GDK_BUTTON_PRESS)
		return FALSE; /* didn't handle event */
	if(event->button != 3)
		return FALSE; /* propagate event */

	gtk_menu_popup(GTK_MENU(panel->transcript_menu), NULL, NULL, NULL, NULL, 3, event->time);
	return TRUE; /* handled event */
}