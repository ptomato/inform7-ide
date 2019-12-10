/*  Copyright (C) 2011, 2012 P. F. Chimento
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

#include <string.h>
#include <glib.h>
#include "app-test.h"
#include "app.h"
#include "configfile.h"
#include "file.h"

void
test_app_create(void)
{
	g_autoptr(I7App) theapp = i7_app_new();
	g_assert(I7_IS_APP(theapp));
}

static void
check_file(GFile *file, const char *name)
{
	char *basename;

	g_assert(G_IS_FILE(file));
	basename = g_file_get_basename(file);
	g_assert_cmpstr(basename, ==, name);
	g_free(basename);
	g_object_unref(file);
}

void
test_app_files(void)
{
	GFile *file;
	g_autoptr(I7App) theapp = i7_app_new();

	file = i7_app_get_data_file(theapp, "Extensions");
	check_file(file, "Extensions");

	file = i7_app_get_data_file_va(theapp, "highlighting", "inform.lang", NULL);
	check_file(file, "inform.lang");

	file = i7_app_get_binary_file(theapp, "ni");
	check_file(file, "ni");

	/* TODO: How to test the functions that open an error dialog when they fail? */
}

void
test_app_extensions_install_remove(void)
{
	GFile *file = g_file_new_for_path("tests/Lickable Wallpaper.i7x");
	g_autoptr(I7App) theapp = i7_app_new();
	GError *err = NULL;
	char *contents;

	/* Directory of installed extensions */
	char *path = g_build_filename(g_get_home_dir(), "Inform", "Documentation", "Extensions.html", NULL);
	GFile *exts = g_file_new_for_path(path);
	g_free(path);

	/* Test installing */
	i7_app_install_extension(theapp, file);
	g_object_unref(file);
	GFile *installed_file = i7_app_get_extension_file(theapp, "Regera Dowdy", "Lickable Wallpaper");
	check_file(installed_file, "Lickable Wallpaper.i7x");
	g_file_load_contents(exts, NULL, &contents, NULL, NULL, &err);
	g_assert_no_error(err);

	/* Test removing */
	i7_app_delete_extension(theapp, "Regera Dowdy", "Lickable Wallpaper");
	installed_file = i7_app_get_extension_file(theapp, "Regera Dowdy", "Lickable Wallpaper");
	g_assert(g_file_query_exists(installed_file, NULL) == FALSE);
	g_object_unref(installed_file);

	/* Make sure it was listed in the directory before */
	g_assert(strstr(contents, "Regera Dowdy"));
	g_assert(strstr(contents, "Lickable Wallpaper"));
	g_free(contents);

	/* Make sure it is not listed in the directory anymore */
	g_file_load_contents(exts, NULL, &contents, NULL, NULL, &err);
	g_assert_no_error(err);
	g_assert(!strstr(contents, "Lickable Wallpaper"));
	g_free(contents);
}

void
test_app_extensions_get_builtin(void)
{
	g_autoptr(I7App) theapp = i7_app_new();
	gboolean builtin;
	char *version_string;

	/* Test that builtin extension is given as builtin */
	version_string = i7_app_get_extension_version(theapp, "Graham Nelson", "Standard Rules", &builtin);
	g_assert(version_string != NULL);
	g_free(version_string);
	g_assert(builtin);

	/* Test that non-builtin extension is not given as builtin */
	GFile *file = g_file_new_for_path("tests/Lickable Wallpaper.i7x");
	i7_app_install_extension(theapp, file);
	g_object_unref(file);
	version_string = i7_app_get_extension_version(theapp, "Regera Dowdy", "Lickable Wallpaper", &builtin);
	i7_app_delete_extension(theapp, "Regera Dowdy", "Lickable Wallpaper");
	g_assert(version_string != NULL);
	g_free(version_string);
	g_assert(!builtin);
}

void
test_app_extensions_get_version(void)
{
	g_autoptr(I7App) theapp = i7_app_new();
	char *version_string;

	/* Test that a nonexistent extension returns NULL */
	version_string = i7_app_get_extension_version(theapp, "Eduard Blutig", "Everlasting Gobstoppers", NULL);
	g_assert(version_string == NULL);

	/* Test that "Title by Author begins here" is given as empty string */
	GFile *file = g_file_new_for_path("tests/Lickable Wallpaper.i7x");
	i7_app_install_extension(theapp, file);
	g_object_unref(file);
	version_string = i7_app_get_extension_version(theapp, "Regera Dowdy", "Lickable Wallpaper", NULL);
	i7_app_delete_extension(theapp, "Regera Dowdy", "Lickable Wallpaper");
	g_assert(version_string != NULL);
	g_assert_cmpstr(version_string, ==, "");
	g_free(version_string);

	/* Test that "Version X of Title by Author begins here" is given as "X" */
	file = g_file_new_for_path("tests/Square Candies.i7x");
	i7_app_install_extension(theapp, file);
	g_object_unref(file);
	version_string = i7_app_get_extension_version(theapp, "Ogdred Weary", "Square Candies", NULL);
	i7_app_delete_extension(theapp, "Ogdred Weary", "Square Candies");
	g_assert(version_string != NULL);
	g_assert_cmpstr(version_string, ==, "12");
	g_free(version_string);
}

void
test_app_extensions_case_insensitive(void)
{
	GFile *file = g_file_new_for_path("tests/Lickable Wallpaper.i7x");
	g_autoptr(I7App) theapp = i7_app_new();

	/* Install test extension */
	i7_app_install_extension(theapp, file);
	g_object_unref(file);

	GFile *extensions_file = i7_app_get_extension_file(theapp, NULL, NULL);
	GFile *child1 = g_file_get_child(extensions_file, "Regera Dowdy");
	GFile *child2 = g_file_get_child(extensions_file, "regera dowdy");
	GFile *child3 = g_file_get_child(extensions_file, "Ogdred Weary");
	GFile *test1 = g_file_get_child(child1, "Lickable Wallpaper.i7x");
	GFile *test2 = g_file_get_child(child1, "lickable wallpaper.i7x");
	GFile *test3 = g_file_get_child(child2, "Lickable Wallpaper.i7x");
	GFile *test4 = g_file_get_child(child2, "lickable wallpaper.i7x");
	GFile *test5 = g_file_get_child(child1, "Square Candies.i7x");
	GFile *test6 = g_file_get_child(child2, "Square Candies.i7x");
	GFile *test7 = g_file_get_child(child3, "Square Candies.i7x");
	g_object_unref(extensions_file);
	g_object_unref(child1);
	g_object_unref(child2);
	g_object_unref(child3);

	GFile *result1 = get_case_insensitive_extension(test1);
	GFile *result2 = get_case_insensitive_extension(test2);
	GFile *result3 = get_case_insensitive_extension(test3);
	GFile *result4 = get_case_insensitive_extension(test4);
	GFile *result5 = get_case_insensitive_extension(test5);
	GFile *result6 = get_case_insensitive_extension(test6);
	GFile *result7 = get_case_insensitive_extension(test7);

	g_assert(result1);
	g_assert(result2);
	g_assert(result3);
	g_assert(result4);
	g_assert(result1 == test1);
	g_assert(g_file_equal(result1, result2));
	g_assert(g_file_equal(result1, result3));
	g_assert(g_file_equal(result1, result4));
	g_assert(result5 == NULL);
	g_assert(result6 == NULL);
	g_assert(result7 == NULL);

	g_object_unref(test1);
	g_object_unref(test2);
	g_object_unref(test3);
	g_object_unref(test4);
	g_object_unref(test5);
	g_object_unref(test6);
	g_object_unref(test7);
	g_object_unref(result1);
	g_object_unref(result2);
	g_object_unref(result3);
	g_object_unref(result4);

	/* Remove test extension */
	i7_app_delete_extension(theapp, "Regera Dowdy", "Lickable Wallpaper");
}

static void
find_test_color_scheme(GtkSourceStyleScheme *scheme, gboolean *found)
{
	if(strcmp(gtk_source_style_scheme_get_id(scheme), "test_color_scheme") == 0)
		*found = TRUE;
}

void
test_app_colorscheme_install_remove(void)
{
	g_autoptr(I7App) theapp = i7_app_new();
	GFile *file = g_file_new_for_path("tests/test_color_scheme.xml");

	/* Test installing */
	const char *id = i7_app_install_color_scheme(theapp, file);
	g_object_unref(file);
	g_assert_cmpstr(id, ==, "test_color_scheme");

	/* Check if the file is really installed */
	char *installed_path = g_build_filename(g_get_user_config_dir(), "inform7", "styles", "test_color_scheme.xml", NULL);
	GFile *installed_file = g_file_new_for_path(installed_path);
	g_free(installed_path);
	g_assert(g_file_query_exists(installed_file, NULL));

	/* Test that the scheme is detected as a user-installed scheme */
	g_assert(i7_app_color_scheme_is_user_scheme(theapp, id));

	/* Test whether the scheme is in the list of installed schemes */
	gboolean found = FALSE;
	i7_app_foreach_color_scheme(theapp, (GFunc)find_test_color_scheme, &found);
	g_assert(found);

	/* Test uninstalling */
	g_assert(i7_app_uninstall_color_scheme(theapp, id));

	/* Check if file is really uninstalled */
	g_assert(g_file_query_exists(installed_file, NULL) == FALSE);

	g_object_unref(installed_file);
}

void
test_app_colorscheme_get_current(void)
{
	g_autoptr(I7App) theapp = i7_app_new();

	GtkSourceStyleScheme *scheme = i7_app_get_current_color_scheme(theapp);
	GSettings *prefs = i7_app_get_prefs(theapp);
	char *id = g_settings_get_string(prefs, PREFS_STYLE_SCHEME);
	g_assert_cmpstr(gtk_source_style_scheme_get_id(scheme), ==, id);
	g_free(id);
}