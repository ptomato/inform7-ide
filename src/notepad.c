/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Philip Chimento <philip.chimento@gmail.com>
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
