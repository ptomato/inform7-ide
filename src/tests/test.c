/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2011, 2012, 2014, 2022 Philip Chimento <philip.chimento@gmail.com>
 */

#include "config.h"

#include <glib.h>
#include <gtk/gtk.h>

#include "app.h"
#include "app-test.h"
#include "difftest.h"
#include "skein-test.h"
#include "story-test.h"

int
main(int argc, char **argv)
{
	gtk_test_init(&argc, &argv);
	
	g_test_add_func("/app/create", test_app_create);
	g_test_add_func("/app/files", test_app_files);
	g_test_add_func("/app/extensions/install-remove", test_app_extensions_install_remove);
	g_test_add_func("/app/extensions/get-builtin", test_app_extensions_get_builtin);
	g_test_add_func("/app/extensions/get-version", test_app_extensions_get_version);
	g_test_add_func("/app/extensions/case-insensitive", test_app_extensions_case_insensitive);
	g_test_add_func("/app/colorscheme/install-remove", test_app_colorscheme_install_remove);
	g_test_add_func("/app/colorscheme/get-current", test_app_colorscheme_get_current);

	g_test_add_func("/diffs/same", test_diffs_same);
	g_test_add_func("/diffs/whitespace", test_diffs_whitespace);
	g_test_add_func("/diffs/different", test_diffs_different);

	g_test_add_func("/skein/import", test_skein_import);

	g_test_add_func("/story/util/files-are-siblings", test_files_are_siblings);
	g_test_add_func("/story/util/files-are-not-siblings", test_files_are_not_siblings);
	g_test_add_func("/story/materials-file", test_story_materials_file);
	g_test_add_func("/story/old-materials-file", test_story_old_materials_file);
	g_test_add_func("/story/renames-materials-file", test_story_renames_materials_file);

	int retval = g_test_run();

	return retval;
}
	