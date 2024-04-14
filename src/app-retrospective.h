/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Philip Chimento <philip.chimento@gmail.com>
 */

#pragma once

#include <gio/gio.h>
#include <glib.h>
#include <glib-object.h>

#define I7_TYPE_RETROSPECTIVE i7_retrospective_get_type()
G_DECLARE_FINAL_TYPE(I7Retrospective, i7_retrospective, I7, RETROSPECTIVE, GObject)

const char *i7_retrospective_get_id(const I7Retrospective *self);
const char *i7_retrospective_get_display_name(const I7Retrospective *self);
const char *i7_retrospective_get_description(const I7Retrospective *self);

/* private */
void parse_retrospective_txt(GListStore **store_out);
