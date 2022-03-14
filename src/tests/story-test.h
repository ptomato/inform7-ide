/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2014 Philip Chimento <philip.chimento@gmail.com>
 */

#ifndef STORY_TEST_H
#define STORY_TEST_H

#include <glib.h>
#include "story.h"

G_BEGIN_DECLS

/* Utility tests, used only here so far */
void test_files_are_siblings(void);
void test_files_are_not_siblings(void);

void test_story_materials_file(void);
void test_story_old_materials_file(void);
void test_story_renames_materials_file(void);

G_END_DECLS

#endif /* STORY_TEST_H */
