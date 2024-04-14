/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Philip Chimento <philip.chimento@gmail.com>
 */

#include "config.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "builder.h"

/* Miscellaneous GtkBuilder-related functions and macros */

/* Return a pointer to the object called @name constructed by @builder */
GObject *
load_object(GtkBuilder *builder, const gchar *name)
{
	GObject *retval;
	if(G_UNLIKELY((retval = gtk_builder_get_object(builder, name)) == NULL))
		g_error("Error while getting object '%s' during interface building", name);
	return retval;
}
