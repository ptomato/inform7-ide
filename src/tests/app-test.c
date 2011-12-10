/*  Copyright (C) 2011 P. F. Chimento
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

#include "app-test.h"
#include <app.h>

void
test_app_create(void)
{
	I7App *theapp = i7_app_get();

	g_assert(I7_IS_APP(theapp));
	g_assert_cmpint(i7_app_get_num_open_documents(theapp), ==, 0);
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
	I7App *theapp = i7_app_get();

	file = i7_app_check_data_file(theapp, "Extensions");
	check_file(file, "Extensions");

	file = i7_app_check_data_file_va(theapp, "ui", "gnome-inform7.ui", NULL);
	check_file(file, "gnome-inform7.ui");
	
	file = i7_app_get_data_file(theapp, "Extensions");
	check_file(file, "Extensions");

	file = i7_app_get_data_file_va(theapp, "ui", "gnome-inform7.ui", NULL);
	check_file(file, "gnome-inform7.ui");

	file = i7_app_get_binary_file(theapp, "ni");
	check_file(file, "ni");

	g_assert(i7_app_check_data_file(theapp, "nonexistent") == NULL);
	g_assert(i7_app_check_data_file_va(theapp, "nonexistent", "nonexistent", NULL) == NULL);

	/* TODO: How to test the functions that open an error dialog when they fail? */
}
