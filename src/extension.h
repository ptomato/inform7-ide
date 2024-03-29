/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2006-2010, 2012 Philip Chimento <philip.chimento@gmail.com>
 */

#ifndef _EXTENSION_H_
#define _EXTENSION_H_

#include "config.h"

#include <glib.h>
#include <glib-object.h>

#include "app.h"
#include "document.h"
#include "source-view.h"

#define I7_TYPE_EXTENSION            (i7_extension_get_type())
#define I7_EXTENSION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), I7_TYPE_EXTENSION, I7Extension))
#define I7_EXTENSION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), I7_TYPE_EXTENSION, I7ExtensionClass))
#define I7_IS_EXTENSION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), I7_TYPE_EXTENSION))
#define I7_IS_EXTENSION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), I7_TYPE_EXTENSION))
#define I7_EXTENSION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), I7_TYPE_EXTENSION, I7ExtensionClass))

typedef struct {
	I7DocumentClass parent_class;
} I7ExtensionClass;

typedef struct {
	I7Document parent_instance;

	I7SourceView *sourceview;
} I7Extension;

GType i7_extension_get_type(void) G_GNUC_CONST;
I7Extension *i7_extension_new(I7App *app, GFile *file, const char *title, const char *author);
I7Extension *i7_extension_new_from_file(I7App *app, GFile *file, gboolean readonly);
gboolean i7_extension_open(I7Extension *self, GFile *file, gboolean readonly);
void i7_extension_set_read_only(I7Extension *self, gboolean readonly);

#endif /* _EXTENSION_H_ */
