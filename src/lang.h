/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Zachary Amsden
 */

#ifndef LANG_H
#define LANG_H

#include "config.h"

#include <glib.h>
#include <gtksourceview/gtksource.h>

void set_buffer_language(GtkSourceBuffer *buffer, gchar *lang);

#endif
