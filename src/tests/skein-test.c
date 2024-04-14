/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Philip Chimento <philip.chimento@gmail.com>
 */

#include "config.h"

#include <glib.h>

#include "node.h"
#include "skein.h"

void
test_skein_import(void)
{
	GError *err = NULL;
	I7Skein *skein = i7_skein_new();
	I7Node *node = i7_skein_get_root_node(skein);
	const char *filename = g_test_get_filename(G_TEST_DIST, "tests", "commands.rec", NULL);
	g_autoptr(GFile) commands_file = g_file_new_for_path(filename);
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

	g_object_unref(skein);
}