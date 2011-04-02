/*  Copyright (C) 2008, 2009, 2010 P. F. Chimento
 *  This file is part of GNOME Inform 7.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include "builder.h"
#include "error.h"

/* Miscellaneous GtkBuilder-related functions and macros; also, the arbitrary
 home of the START_TIMER and STOP_TIMER macros */

/* TODO: Move all the GtkBuilder interface definitions into one file and use
 gtk_builder_add_objects_from_file() to build the appropriate ones */

/* Create a new GtkBuilder from an interface definition file and connect its
 signals */
GtkBuilder *
create_new_builder(const gchar *filename, gpointer data)
{
	GError *error = NULL;
	GtkBuilder *builder;

	builder = gtk_builder_new();
	if(!gtk_builder_add_from_file(builder, filename, &error))
		ERROR(_("Error while building interface"), error);
	gtk_builder_connect_signals(builder, data);

	return builder;
}

/* Return a pointer to the object called @name constructed by @builder */
GObject *
load_object(GtkBuilder *builder, const gchar *name)
{
	GObject *retval;
	if(G_UNLIKELY((retval = gtk_builder_get_object(builder, name)) == NULL))
		g_error(_("Error while getting object '%s' during interface building"), name);
	return retval;
}

/* Add actions constructed by @builder to an empty action group also constructed
 by @builder. @group_name is the name of the action group, and @action_names is
 an array of strings: alternating action names and accelerators. @action_names
 must also be terminated by NULL. A pointer to the action group is placed into
 @group. */
void
add_actions(GtkBuilder *builder, GtkActionGroup **group, const gchar *group_name, const gchar **action_names)
{
	const gchar **ptr;
	*group = GTK_ACTION_GROUP(load_object(builder, group_name));
	for(ptr = action_names; *ptr; ptr += 2)
		gtk_action_group_add_action_with_accel(*group, GTK_ACTION(load_object(builder, ptr[0])), ptr[1]);
}
