/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Philip Chimento <philip.chimento@gmail.com>
 */

#pragma once

#include "config.h"

#include <stdbool.h>

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#define I7_TYPE_TOAST i7_toast_get_type()
G_DECLARE_FINAL_TYPE(I7Toast, i7_toast, I7, TOAST, GtkRevealer)

I7Toast *i7_toast_new(void);

void i7_toast_show_message(I7Toast *self, const char *message);
