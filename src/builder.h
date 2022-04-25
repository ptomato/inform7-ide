/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2008-2011, 2019 Philip Chimento <philip.chimento@gmail.com>
 */

#ifndef _BUILDER_H_
#define _BUILDER_H_

#include "config.h"

#include <glib.h>
#include <gtk/gtk.h>

GObject *load_object(GtkBuilder *builder, const gchar *name);

/* Shortcut for loading a public widget pointer in _init() functions.
The object being init'ed must be called 'self'. */
#define LOAD_WIDGET(name) self->name = GTK_WIDGET(load_object(builder, G_STRINGIFY(name)))

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
