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

#endif /* _BUILDER_H_ */
