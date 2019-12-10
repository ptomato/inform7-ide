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

static void
queue_up_expected_messages(void)
{
	g_test_expect_message("Gtk", G_LOG_LEVEL_CRITICAL, "*assertion*GTK_IS_TEXT_MARK*failed*");
	g_test_expect_message("Gtk", G_LOG_LEVEL_CRITICAL, "*assertion*GTK_IS_TEXT_MARK*failed*");
	g_test_expect_message("Gtk", G_LOG_LEVEL_CRITICAL, "*assertion*GTK_IS_TEXT_MARK*failed*");
}

void
test_story_materials_file(void)
{
	g_autoptr(I7App) theapp = i7_app_new();
	while(gtk_events_pending())
		gtk_main_iteration();

	queue_up_expected_messages();

	GFile *story_file = g_file_new_for_path("The Arrow of Time.inform");
	I7Story *story = i7_story_new(theapp, story_file,
		"The Arrow of Time", "Eduard Blutig");

	GFile *materials_file = i7_story_get_materials_file(story);
	/* gtk_object_destroy(GTK_OBJECT(story)); FIXME crashes */
	char *basename = g_file_get_basename(materials_file);

	g_assert_cmpstr(basename, ==, "The Arrow of Time.materials");

	g_free(basename);

	g_assert(files_are_siblings(story_file, materials_file));

	g_object_unref(story_file);
	g_object_unref(materials_file);
}

void
test_story_old_materials_file(void)
{
	g_autoptr(I7App) theapp = i7_app_new();
	while(gtk_events_pending())
		gtk_main_iteration();

	queue_up_expected_messages();

	GFile *story_file = g_file_new_for_path("The Arrow of Time.inform");
	I7Story *story = i7_story_new(theapp, story_file,
		"The Arrow of Time", "Eduard Blutig");

	GFile *materials_file = i7_story_get_old_materials_file(story);
	char *basename = g_file_get_basename(materials_file);

	g_assert_cmpstr(basename, ==, "The Arrow of Time Materials");

	g_free(basename);

	g_assert(files_are_siblings(story_file, materials_file));

	g_object_unref(story_file);
	g_object_unref(materials_file);
	/* gtk_object_destroy(GTK_OBJECT(story)); FIXME crashes */
}

void
test_story_renames_materials_file(void)
{
	g_autoptr(I7App) theapp = i7_app_new();
	while(gtk_events_pending())
		gtk_main_iteration();

	queue_up_expected_messages();

	GFile *materials_file = g_file_new_for_path(TEST_DATA_DIR "Hereafter.materials");
	GFile *old_materials_file = g_file_new_for_path(TEST_DATA_DIR "Hereafter Materials");
	/* There may have been folders left over from an old failed test run */
	g_assert(!g_file_query_exists(materials_file, NULL) || g_file_delete(materials_file, NULL, NULL));
	g_assert(!g_file_query_exists(old_materials_file, NULL) || g_file_delete(old_materials_file, NULL, NULL));

	g_assert(g_file_make_directory(old_materials_file, NULL, NULL));

	GFile *story_file = g_file_new_for_path(TEST_DATA_DIR "Hereafter.inform");
	I7Story *story = i7_story_new_from_file(theapp, story_file);
	g_object_unref(story_file);
	gtk_object_destroy(GTK_OBJECT(story));

	g_assert(g_file_query_exists(materials_file, NULL));
	g_assert(!g_file_query_exists(old_materials_file, NULL));

	g_assert(g_file_delete(materials_file, NULL, NULL));

	g_object_unref(materials_file);
	g_object_unref(old_materials_file);
}
