/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Philip Chimento <philip.chimento@gmail.com>
 */

#include "config.h"

#include <stdlib.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>

#include "app.h"
#include "error.h"
#include "searchwindow.h"

/*
 * version:
 *
 * Print version information to the terminal.
 */
static void
version(void)
{
	g_print("%s %s\n", PACKAGE, PACKAGE_VERSION);
}

int
main(int argc, char *argv[])
{
#ifdef ENABLE_NLS
	bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);
#endif

	/* g_mem_set_vtable(glib_mem_profiler_table); */

	gtk_source_init();

	GError *error = NULL;

	/* Set up the command-line options */
	gchar **remaining_args = NULL;
	gboolean print_version = FALSE;
	GOptionEntry entries[] = {
		{
			.long_name = "version",
			.short_name = 'v',
			.arg = G_OPTION_ARG_NONE,
			.arg_data = &print_version,
			.description = N_("Print version information"),
		},
		{
			.long_name = G_OPTION_REMAINING,
			.arg = G_OPTION_ARG_FILENAME_ARRAY,
			.arg_data = &remaining_args,
			.description = "",
			/* TRANSLATORS: This string occurs in the --help message's usage
			 string, to indicate that the user can specify project files on the
			 command line in order to have them opened at startup */
			.arg_description = N_("[FILE1 FILE2 ...]"),
		},
		{ .long_name = NULL }
	};
	GOptionContext *context = g_option_context_new(
	/* TRANSLATORS: This is the usage string for the --help message */
	  _("- Interactive fiction IDE"));
	g_option_context_add_main_entries(context, entries, GETTEXT_PACKAGE);
	g_option_context_add_group(context, gtk_get_option_group(TRUE));
	if(!g_option_context_parse(context, &argc, &argv, &error))
		WARN(_("Failed to parse commandline options"), error);
	g_option_context_free(context);

	if(print_version) {
		version();
		return 0;
	}

	gtk_init(&argc, &argv);

	/* Initialize the Inform 7 application */
	/* TRANSLATORS: this is the human-readable application name */
	g_set_application_name(_("Inform 7"));
	I7App *theapp = i7_app_new();

	int returncode = g_application_run(G_APPLICATION(theapp), argc, argv);

	g_object_unref(theapp);
	i7_search_window_free_index();
	gtk_source_finalize();
	/* g_mem_profile();*/
	return returncode;
}
