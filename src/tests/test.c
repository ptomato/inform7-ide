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

#include <glib.h>
#include <gtk/gtk.h>
#include "app-test.h"

int
main(int argc, char **argv)
{
	gtk_test_init(&argc, &argv);
	
	g_test_add_func("/app/create", test_app_create);
	g_test_add_func("/app/files", test_app_files);
	
	I7App *theapp = i7_app_get();
	int retval = g_test_run();
	g_object_unref(theapp);

	return retval;
}
	