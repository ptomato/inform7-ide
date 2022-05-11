/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2022 Philip Chimento <philip.chimento@gmail.com>
 */

#pragma once

#include <glib.h>

#include "app.h"

/* Record parsed out of retrospective.txt */
typedef struct {
	char *display_name;
	char *description;
} RetrospectiveData;

void parse_retrospective_txt(GHashTable **entries_out, char ***ids_out);
const RetrospectiveData *get_retrospective_data(I7App *app, const char *id);
