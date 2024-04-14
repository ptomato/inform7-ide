/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Zachary Amsden
 */

#pragma once

#include "config.h"

#include <glib.h>
#include <gtksourceview/gtksource.h>

void set_buffer_language(GtkSourceBuffer *buffer, gchar *lang);
