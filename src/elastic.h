/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Philip Chimento <philip.chimento@gmail.com>
 */

#pragma once

#include "config.h"

#include <gtk/gtk.h>

gboolean elastic_recalculate_view(GtkTextView *view);
void add_elastic_tabstops_to_view(GtkTextView *view);
void remove_elastic_tabstops_from_view(GtkTextView *view);
