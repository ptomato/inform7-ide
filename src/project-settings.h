/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Philip Chimento <philip.chimento@gmail.com>
 */

#pragma once

#include "config.h"

#include <glib.h>
#include <glib-object.h>
#include <handy.h>

#include "story.h"

#define I7_TYPE_PROJECT_SETTINGS i7_project_settings_get_type()
G_DECLARE_FINAL_TYPE(I7ProjectSettings, i7_project_settings, I7, PROJECT_SETTINGS, HdyPreferencesPage)

I7ProjectSettings *i7_project_settings_new(void);
void i7_project_settings_bind_properties(I7ProjectSettings *self, I7Story *story);
