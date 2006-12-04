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
 
/* To do
- Search documentation
- Make extensions windows in preferences work with drag'n'drop
- Make default author's name, if preference is blank, use system long user name
- Watch the source file for external changes
Low priority
- Find out how to highlight markup brackets in strings
- Search text at word boundaries

FOLLOWING RELEASES
- Embedded interpreters
- Skein, Transcript
- I6 projects

BUGS
- use libxml2 as it ought to be used
- get glulx compiling to work (new version of Inform 6??)
- If files are missing when loading, create default ones
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>

#if !GLIB_CHECK_VERSION(2,4,0)
# error You need at least Gtk/GLib 2.4.0 to run this code.
#endif

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#include "configfile.h"
#include "compile.h"

int
main (int argc, char *argv[])
{
    GtkWidget *welcome_dialog;

#ifdef ENABLE_NLS
    bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);
#endif

    gnome_program_init(PACKAGE, VERSION, LIBGNOMEUI_MODULE,
      argc, argv,
      GNOME_PARAM_APP_DATADIR, PACKAGE_DATA_DIR,
      NULL);
    
    /* Create the .gnome-inform7 dir if it doesn't already exist */
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
    
    /* Create the splash window */
    welcome_dialog = create_welcome_dialog();
    gtk_widget_show(welcome_dialog);

    gtk_main();
    return 0;
}
