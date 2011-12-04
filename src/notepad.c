/* Copyright (C) 2010, 2011 P. F. Chimento
 * This file is part of GNOME Inform 7.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtk/gtk.h>
#include "configfile.h"
#include "story.h"
#include "story-private.h"

gboolean
on_notes_window_delete_event(GtkWidget *window, GdkEvent *event, I7Story *story)
{
	I7_STORY_USE_PRIVATE(story, priv);
	I7App *theapp = i7_app_get();
	GSettings *state = i7_app_get_state(theapp);

	gtk_widget_hide(window);
	g_settings_set_boolean(state, PREFS_STATE_SHOW_NOTEPAD, FALSE);
	gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(gtk_action_group_get_action(priv->story_action_group, "view_notepad")), FALSE);
	return TRUE; /* Block event */
}
