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

#include <gnome.h>

#if !GLIB_CHECK_VERSION(2,8,0)
# error You need at least GLib 2.8.0 to run this code.
#endif

#if !GTK_CHECK_VERSION(2,8,0)
# error You need at least GTK+ 2.8.0 to run this code.
#endif

#include "interface.h"
#include "support.h"

#include "configfile.h"
#include "compile.h"
#include "datafile.h"
#include "inspector.h"
#include "file.h"
#include "windowlist.h"

#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif

int
main(int argc, char *argv[])
{
    GtkWidget *welcome_dialog;
    extern GtkWidget *inspector_window;
    extern GtkWidget *prefs_dialog;
    const char *datadir, *pixmapdir;

#ifdef ENABLE_NLS
    bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);
#endif

    /* Set up the command-line options */
    gchar **remaining_args = NULL;
    GOptionEntry option_entries[] = {
		{ G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY,
		  &remaining_args,
		  "Special option that collects any remaining arguments for us" },
		{ NULL }
	};
    GOptionContext *option_context = g_option_context_new(
    /* TRANSLATORS: This is the usage string for the --help message */
      _("[FILES...] - Interactive fiction IDE"));
    g_option_context_add_main_entries(option_context, option_entries, NULL);
    
    /* Retrieve data directory if set externally */
    datadir = getenv("GNOME_INFORM_DATA_DIR");
	pixmapdir = getenv("GNOME_INFORM_PIXMAP_DIR");

    gnome_program_init(PACKAGE, VERSION, LIBGNOMEUI_MODULE,
      argc, argv,
      GNOME_PARAM_GOPTION_CONTEXT, option_context,
      GNOME_PARAM_APP_DATADIR, datadir == NULL ? PACKAGE_DATA_DIR : datadir,
      GNOME_PARAM_NONE);
    
    /* Create the Gnome Inform7 dir if it doesn't already exist */
    gchar *extensions_dir = get_extension_path(NULL, NULL);
    g_mkdir_with_parents(extensions_dir, 0777);
    g_free(extensions_dir);
    
    /* Index the extensions in the background */
    run_census(FALSE);
    
    /* Create the global inspector window and preferences dialog, but keep them 
    hidden */
    inspector_window = create_inspector_window();
    prefs_dialog = create_prefs_dialog();

    /* Check the application settings and, if they don't exist, set them to the
    defaults */
    init_config_file();

    /* Do stuff with the remaining command line arguments (files) */
    if(remaining_args != NULL) {
	    gint i, num_args;

		num_args = g_strv_length(remaining_args);
		for (i = 0; i < num_args; ++i) {
			Story *thestory = open_project(remaining_args[i]);
            if(thestory != NULL)
                gtk_widget_show(thestory->window);
		}
		g_strfreev (remaining_args);
		remaining_args = NULL;
	} 
    
    /* If no windows were opened from command line arguments */
    if(get_num_app_windows() == 0) {
        /* Create the splash window */
        welcome_dialog = create_welcome_dialog();
        gtk_widget_show(welcome_dialog);
    }

    gtk_main();
    
    /* Save the position of the inspector window */
    save_inspector_window_position();
    
    return 0;
}
