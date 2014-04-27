/*  Copyright (C) 2011, 2012, 2014 P. F. Chimento
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
#include <gtk/gtk.h>
#include "app.h"
#include "app-test.h"
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

	g_test_add_func("/skein/import", test_skein_import);

	g_test_add_func("/story/util/files-are-siblings", test_files_are_siblings);
	g_test_add_func("/story/util/files-are-not-siblings", test_files_are_not_siblings);
	g_test_add_func("/story/materials-file", test_story_materials_file);
	g_test_add_func("/story/old-materials-file", test_story_old_materials_file);
	g_test_add_func("/story/renames-materials-file", test_story_renames_materials_file);

	int retval = g_test_run();

	return retval;
}
	