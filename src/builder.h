/*  Copyright (C) 2008, 2009, 2010, 2011 P. F. Chimento
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

#ifndef _BUILDER_H_
#define _BUILDER_H_

#include "config.h"

#include <glib.h>
#include <gtk/gtk.h>

GObject *load_object(GtkBuilder *builder, const gchar *name);

/* Shortcuts for loading public widget and action pointers in _init() functions.
The object being init'ed must be called 'self'. */
#define LOAD_WIDGET(name) self->name = GTK_WIDGET(load_object(builder, G_STRINGIFY(name)))
#define LOAD_ACTION(group, name) self->name = gtk_action_group_get_action(group, G_STRINGIFY(name))

/* Arbitrarily putting this here; bracket a section of code to be timed in
 between START_TIMER and STOP_TIMER. */
#define START_TIMER \
	GTimeVal timer, timer2; \
	glong sec, usec; \
	g_get_current_time(&timer);
#define STOP_TIMER \
	g_get_current_time(&timer2); \
	sec = timer2.tv_sec - timer.tv_sec; \
	usec = timer2.tv_usec - timer.tv_usec; \
	if(usec < 0) { \
		usec += 1000000L; \
		sec--; \
	} \
	g_printerr("Time (%s): %ld.%06ld\n", G_STRFUNC, sec, usec);

#endif /* _BUILDER_H_ */
