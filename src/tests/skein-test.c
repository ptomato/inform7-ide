/*  Copyright (C) 2012 P. F. Chimento
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
#include "skein.h"
#include "node.h"

void
test_skein_import(void)
{
	GError *err = NULL;
	I7Skein *skein = i7_skein_new();
	I7Node *node = i7_skein_get_root_node(skein);
	GFile *commands_file = g_file_new_for_path(TEST_DATA_DIR "commands.rec");
	char * const commands[] = {
		"- start -",
		"look at wallpaper",
		"stand on couch",
		"jump",
		"asplode spacecraft",
		"no",
		"kenneth, what's the frequency",
		NULL
	};
	char * const *ptr;

	g_assert(i7_skein_import(skein, commands_file, &err));
	g_assert(err == NULL);

	for(ptr = commands; *ptr; ptr++) {
		g_assert(node);

		char *command = i7_node_get_command(node);
		g_assert_cmpstr(command, ==, *ptr);
		g_free(command);

		GNode *child_gnode = node->gnode->children;
		node = child_gnode? child_gnode->data : NULL;
	}

	g_object_unref(commands_file);
	g_object_unref(skein);
}