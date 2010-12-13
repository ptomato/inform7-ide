/* This file is part of GNOME Inform 7.
 * Copyright (c) 2006-2009 P. F. Chimento <philip.chimento@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <stdlib.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include "app.h"
#include "configfile.h"
#include "error.h"
#include "welcomedialog.h"

int
main(int argc, char *argv[])
{
#ifdef ENABLE_NLS
    bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);
#endif

/*    g_mem_set_vtable(glib_mem_profiler_table);
    g_atexit(g_mem_profile);*/

    GError *error = NULL;

    /* Set up the command-line options */
    gchar **remaining_args = NULL;
    GOptionEntry option_entries[] = {
		{ G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY,
		  &remaining_args, "",
			/* TRANSLATORS: This string occurs in the --help message's usage
			 string, to indicate that the user can specify project files on the
			 command line in order to have them opened at startup */
			N_("[FILE1 FILE2 ...]") },
		{ NULL }
	};
    GOptionContext *option_context = g_option_context_new(
    /* TRANSLATORS: This is the usage string for the --help message */
      _("- Interactive fiction IDE"));
    g_option_context_add_main_entries(option_context, option_entries, GETTEXT_PACKAGE);
	g_option_context_add_group(option_context, gtk_get_option_group(TRUE));
	if(!g_option_context_parse(option_context, &argc, &argv, &error))
		ERROR(_("Failed to parse commandline options."), error);
	g_option_context_free(option_context);

	if(!g_thread_supported())
	    g_thread_init(NULL);
	gdk_threads_init();
	
	gtk_init(&argc, &argv);
	
	/* Initialize the Inform 7 application */
	/* TRANSLATORS: this is the human-readable application name */
	g_set_application_name(_("Inform 7"));
	I7App *theapp = i7_app_get();
	trigger_config_file();
	
	/* Open any project files specified on the command line */
    if(remaining_args != NULL) {
		gchar **file;
		for(file = remaining_args; *file; file++)
			i7_app_open(theapp, *file);
		g_strfreev (remaining_args);
	} 
    
    /* If no windows were opened from command line arguments */
    if(i7_app_get_num_open_documents(theapp) == 0) {
        /* Create the splash window */
        GtkWidget *welcomedialog = create_welcome_dialog();
        gtk_widget_show_all(welcomedialog);
    }

    gdk_threads_enter();
    gtk_main();
    gdk_threads_leave();

    g_object_unref(theapp);
    return 0;
}
