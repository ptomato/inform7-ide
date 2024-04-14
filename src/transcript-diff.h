/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Philip Chimento <philip.chimento@gmail.com>
 */

#include "config.h"

#include <glib.h>

gboolean word_diff(const char *expected, const char *actual, GList **expected_diffs, GList **actual_diffs);
char *make_pango_markup_string(const char *string, GList *diffs);
