/*  Copyright (C) 2014 P. F. Chimento
 *  This file is part of GNOME Inform 7.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
