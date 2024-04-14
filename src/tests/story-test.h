/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Philip Chimento <philip.chimento@gmail.com>
 */

#pragma once

#include <glib.h>

G_BEGIN_DECLS

/* Utility tests, used only here so far */
void test_files_are_siblings(void);
void test_files_are_not_siblings(void);

void test_story_materials_file(void);
void test_story_old_materials_file(void);
void test_story_renames_materials_file(void);

G_END_DECLS
