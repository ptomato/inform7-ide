/*  Copyright (C) 2008, 2009, 2010, 2011, 2015 P. F. Chimento
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

#include "config.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "builder.h"
#include "error.h"

/* Miscellaneous GtkBuilder-related functions and macros; also, the arbitrary
 home of the START_TIMER and STOP_TIMER macros */

/* TODO: Move all the GtkBuilder interface definitions into one file and use
 gtk_builder_add_objects_from_file() to build the appropriate ones */

/* Return a pointer to the object called @name constructed by @builder */
GObject *
load_object(GtkBuilder *builder, const gchar *name)
{
	GObject *retval;
	if(G_UNLIKELY((retval = gtk_builder_get_object(builder, name)) == NULL))
		g_error("Error while getting object '%s' during interface building", name);
	return retval;
}
