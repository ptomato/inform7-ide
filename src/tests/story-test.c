/*  Copyright (C) 2014, 2021 P. F. Chimento
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

#include "config.h"

#include <glib.h>

#include "story.h"

static gboolean
files_are_siblings(GFile *a, GFile *b)
{
	GFile *a_parent = g_file_get_parent(a);
	GFile *b_parent = g_file_get_parent(b);

	gboolean retval = g_file_equal(a_parent, b_parent);

	g_object_unref(a_parent);
	g_object_unref(b_parent);

	return retval;
}

void
test_files_are_siblings(void)
{
	GFile *a = g_file_new_for_path("c/a");
	GFile *b = g_file_new_for_path("c/b");

	g_assert(files_are_siblings(a, b));

	g_object_unref(a);
	g_object_unref(b);
}

void
test_files_are_not_siblings(void)
{
	GFile *a = g_file_new_for_path("c/a");
	GFile *b = g_file_new_for_path("d/b");

	g_assert(!files_are_siblings(a, b));

	g_object_unref(a);
	g_object_unref(b);
}

void
test_story_materials_file(void)
{
	g_autoptr(I7App) theapp = i7_app_new();

	const char *filename = g_test_get_filename(G_TEST_DIST, "tests", "The Arrow of Time.inform", NULL);
	g_autoptr(GFile) story_file = g_file_new_for_path(filename);
	I7Story *story = i7_story_new(theapp, story_file,
		"The Arrow of Time", "Eduard Blutig");

	GFile *materials_file = i7_story_get_materials_file(story);
	/* gtk_widget_destroy(GTK_WIDGET(story)); FIXME crashes */
	char *basename = g_file_get_basename(materials_file);

	g_assert_cmpstr(basename, ==, "The Arrow of Time.materials");

	g_free(basename);

	g_assert(files_are_siblings(story_file, materials_file));

	g_object_unref(materials_file);
}

void
test_story_old_materials_file(void)
{
	g_autoptr(I7App) theapp = i7_app_new();

	const char *filename = g_test_get_filename(G_TEST_DIST, "tests", "The Arrow of Time.inform", NULL);
	g_autoptr(GFile) story_file = g_file_new_for_path(filename);
	I7Story *story = i7_story_new(theapp, story_file,
		"The Arrow of Time", "Eduard Blutig");

	GFile *materials_file = i7_story_get_old_materials_file(story);
	char *basename = g_file_get_basename(materials_file);

	g_assert_cmpstr(basename, ==, "The Arrow of Time Materials");

	g_free(basename);

	g_assert(files_are_siblings(story_file, materials_file));

	g_object_unref(materials_file);
	/* gtk_widget_destroy(GTK_WIDGET(story)); FIXME crashes */
}

void
test_story_renames_materials_file(void)
{
	g_autoptr(I7App) theapp = i7_app_new();

	const char *filename = g_test_get_filename(G_TEST_DIST, "tests", "Hereafter.materials", NULL);
	g_autoptr(GFile) materials_file = g_file_new_for_path(filename);
	const char *old_filename = g_test_get_filename(G_TEST_DIST, "tests", "Hereafter Materials", NULL);
	g_autoptr(GFile) old_materials_file = g_file_new_for_path(old_filename);
	/* There may have been folders left over from an old failed test run */
	g_assert(!g_file_query_exists(materials_file, NULL) || g_file_delete(materials_file, NULL, NULL));
	g_assert(!g_file_query_exists(old_materials_file, NULL) || g_file_delete(old_materials_file, NULL, NULL));

	g_assert(g_file_make_directory(old_materials_file, NULL, NULL));

	const char *story_filename = g_test_get_filename(G_TEST_DIST, "tests", "Hereafter.inform", NULL);
	g_autoptr(GFile) story_file = g_file_new_for_path(story_filename);
	I7Story *story = i7_story_new_from_file(theapp, story_file);
	gtk_widget_destroy(GTK_WIDGET(story));

	g_assert(g_file_query_exists(materials_file, NULL));
	g_assert(!g_file_query_exists(old_materials_file, NULL));

	g_assert(g_file_delete(materials_file, NULL, NULL));
}
