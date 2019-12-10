/* Copyright (C) 2010, 2011, 2013, 2019 P. F. Chimento
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

#include "config.h"

#include <gtk/gtk.h>

#include "configfile.h"
#include "story.h"

gboolean
on_notes_window_delete_event(GtkWidget *window, GdkEvent *event, I7Story *story)
{
	I7App *theapp = I7_APP(g_application_get_default());
	GSettings *state = i7_app_get_state(theapp);

	gtk_widget_hide(window);
	g_settings_set_boolean(state, PREFS_STATE_SHOW_NOTEPAD, FALSE);
	GAction *view_notepad = g_action_map_lookup_action(G_ACTION_MAP(story), "view-notepad");
	g_simple_action_set_state(G_SIMPLE_ACTION(view_notepad), g_variant_new_boolean(FALSE));
	return TRUE; /* Block event */
}
