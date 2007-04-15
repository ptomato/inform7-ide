/*  Copyright 2006 P.F. Chimento
 *  This file is part of GNOME Inform 7.
 * 
 *  GNOME Inform 7 is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  GNOME Inform 7 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNOME Inform 7; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>

#if !GLIB_CHECK_VERSION(2,8,0)
# error You need at least GLib 2.8.0 to run this code.
#endif

#if !GTK_CHECK_VERSION(2,10,0)
# error You need at least GTK+ 2.10.0 to run this code.
#endif

#include "interface.h"
#include "support.h"

#include "configfile.h"
#include "compile.h"
#include "datafile.h"
#include "inspector.h"
#include "file.h"
#include "windowlist.h"

int
main (int argc, char *argv[])
{
    GtkWidget *welcome_dialog;
    extern GtkWidget *inspector_window;

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
      "[FILES...] - Interactive fiction IDE");
    g_option_context_add_main_entries(option_context, option_entries, NULL);
    
    gnome_program_init(PACKAGE, VERSION, LIBGNOMEUI_MODULE,
      argc, argv,
      GNOME_PARAM_GOPTION_CONTEXT, option_context,
      GNOME_PARAM_APP_DATADIR, PACKAGE_DATA_DIR,
      GNOME_PARAM_NONE);
    
    /* Create the Gnome Inform7 dir if it doesn't already exist */
    gchar *extensions_dir = get_extension_path(NULL, NULL);
    g_mkdir_with_parents(extensions_dir, 0777);
    g_free(extensions_dir);
    
    /* Check the application settings and, if they don't exist, set them to the
    defaults */
    check_config_file();
    
    /* Check if the external binaries are in the path */
    if(!check_external_binaries())
        return -1;
    
    /* Index the extensions in the background */
    run_census(FALSE);
    
    /* Create the global inspector window, but keep it hidden */
    inspector_window = create_inspector_window();
    
    /* Do stuff with the remaining command line arguments (files) */
    if(remaining_args != NULL) {
	    gint i, num_args;

		num_args = g_strv_length(remaining_args);
		for (i = 0; i < num_args; ++i) {
			struct story *thestory = open_project(remaining_args[i]);
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
