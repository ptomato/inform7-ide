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
 
#include <gnome.h>
#include <gconf/gconf-client.h>

#include "configfile.h"
#include "prefs.h"
#include "error.h"

#define GCONF_BASE_PATH "/apps/gnome-inform7"
#define EXTENSIONS_BASE_PATH "/.wine/drive_c/Inform/Extensions"

/* Returns the directory for installed extensions, with the author and name path
components tacked on if they are not NULL. Returns an allocated string which
must be freed. */
gchar *get_extension_path(const gchar *author, const gchar *extname) {
    if(!author)
        return g_strconcat(g_get_home_dir(), EXTENSIONS_BASE_PATH, NULL);
    if(!extname)
        return g_strconcat(g_get_home_dir(), EXTENSIONS_BASE_PATH, "/", author,
          NULL);
    return g_strconcat(g_get_home_dir(), EXTENSIONS_BASE_PATH, "/", author, "/",
      extname, NULL);
}

/* Check if the config keys exist and if not, set them to defaults. */
/* This could probably be done a lot better with a GConf schema, but,
unfortunately, there is no documentation on it! */
void check_config_file() {
    GError *err = NULL;
    GConfClient *client = gconf_client_get_default();
    
    if(!gconf_client_dir_exists(client, GCONF_BASE_PATH "/User", &err)) {
        config_file_set_string("User", "Name", "");
    }
    if(!gconf_client_dir_exists(client, GCONF_BASE_PATH "/Settings", &err)) {
        /* do nothing; LastProject should stay empty, it is set elsewhere */
    }
    if(!gconf_client_dir_exists(client, GCONF_BASE_PATH "/Fonts", &err)) {
        config_file_set_int("Fonts", "FontSet", FONT_SET_STANDARD);
        config_file_set_string("Fonts", "CustomFont", "Bitstream Vera Sans 11");
        config_file_set_int("Fonts", "FontStyling", FONT_STYLING_OFTEN);
        config_file_set_int("Fonts", "FontSize", FONT_SIZE_STANDARD);
    }
    if(!gconf_client_dir_exists(client, GCONF_BASE_PATH "/Colors", &err)) {
        config_file_set_int("Colors", "ChangeColors", CHANGE_COLORS_OFTEN);
        config_file_set_int("Colors", "ColorSet", COLOR_SET_STANDARD);
    }
    if(!gconf_client_dir_exists(client, GCONF_BASE_PATH "/Tabs", &err)) {
        config_file_set_int("Tabs", "TabWidth", 0);
    }
    if(!gconf_client_dir_exists(client, GCONF_BASE_PATH "/Inspectors", &err)) {
        config_file_set_bool("Inspectors", "ProjectFiles", TRUE);
        config_file_set_bool("Inspectors", "Notes", TRUE);
        config_file_set_bool("Inspectors", "Headings", TRUE);
        config_file_set_bool("Inspectors", "Skein", TRUE);
        config_file_set_bool("Inspectors", "Watchpoints", TRUE);
        config_file_set_bool("Inspectors", "Breakpoints", TRUE);
        config_file_set_bool("Inspectors", "Search", TRUE);
    }
    if(!gconf_client_dir_exists(client, GCONF_BASE_PATH "/Syntax", &err)) {
        config_file_set_bool("Syntax", "Highlighting", TRUE);
        config_file_set_bool("Syntax", "Indenting", TRUE);
        config_file_set_bool("Syntax", "Intelligence", TRUE);
        config_file_set_bool("Syntax", "IntelligentIndexInspector", TRUE);
        config_file_set_bool("Syntax", "AutoIndent", TRUE);
        config_file_set_bool("Syntax", "AutoNumberSections", FALSE);
    }
    if(!gconf_client_dir_exists(client, GCONF_BASE_PATH "/Cleaning", &err)) {
        config_file_set_bool("Cleaning", "BuildFiles", TRUE);
        config_file_set_bool("Cleaning", "IndexFiles", FALSE);
    }
    if(!gconf_client_dir_exists(client, GCONF_BASE_PATH "/Debugging", &err)) {
        config_file_set_bool("Debugging", "ShowLog", FALSE);
        config_file_set_bool("Debugging", "RebuildCompiler", FALSE);
    }
}

/* Check to see if all the external binaries are in the path and the wine
directory is set up correctly */
gboolean check_external_binaries() {
    gchar *external_binaries[] = {
        "wine", "frotz"
    };
    gchar *names[] = {
        "Wine", "the Frotz interpreter"
    };
    
    int foo;
    for(foo = 0; foo < 2; foo++)
        if(!g_find_program_in_path(external_binaries[foo])) {
            error_dialog(NULL, NULL, "The '%s' binary was not found in your "
              "path. You must have %s installed in order to use GnomeInform 7.",
              external_binaries[foo], names[foo]);
            return FALSE;
        }

    gchar *wine_home = g_strconcat(g_get_home_dir(), "/.wine/drive_c/windows",
      NULL);

    if(!g_file_test(wine_home, G_FILE_TEST_EXISTS)) {
        error_dialog(NULL, NULL, "You did not configure Wine the way I "
          "expected. Please link your virtual C: drive to '~/.wine/drive_c'. "
          "Running the command 'wineprefixcreate' will probably do this.");
        g_free(wine_home);
        return FALSE;
    }
    g_free(wine_home);
    return TRUE;
}

/* The next six functions are wrappers for GConf setting and getting functions,
that give us a nice error dialog if they fail. */

void config_file_set_string(gchar *path, gchar *key, const gchar *value) {
    g_return_if_fail(path != NULL);
    g_return_if_fail(key != NULL);
    g_return_if_fail(value != NULL);
    
    GError *err = NULL;
    GConfClient *client = gconf_client_get_default();
    
    gchar *keyname = g_strconcat(GCONF_BASE_PATH "/", path, "/", key, NULL);
    
    if(!gconf_client_set_string(client, keyname, value, &err))
        error_dialog(NULL, err, "Could not set GConf key '%s' to '%s': ",
          keyname, value);

    g_object_unref(client);
    g_free(keyname);
}

void config_file_set_int(gchar *path, gchar *key, const gint value) {
    g_return_if_fail(path != NULL);
    g_return_if_fail(key != NULL);
    
    GError *err = NULL;
    GConfClient *client = gconf_client_get_default();
    
    gchar *keyname = g_strconcat(GCONF_BASE_PATH "/", path, "/", key, NULL);
    
    if(!gconf_client_set_int(client, keyname, value, &err))
        error_dialog(NULL, err, "Could not set GConf key '%s' to '%s': ",
          keyname, value);
        
    g_object_unref(client);
    g_free(keyname);
}

void config_file_set_bool(gchar *path, gchar *key, const gboolean value) {
    g_return_if_fail(path != NULL);
    g_return_if_fail(key != NULL);
    
    GError *err = NULL;
    GConfClient *client = gconf_client_get_default();
    
    gchar *keyname = g_strconcat(GCONF_BASE_PATH "/", path, "/", key, NULL);
    
    if(!gconf_client_set_bool(client, keyname, value, &err)) 
        error_dialog(NULL, err, "Could not set GConf key '%s' to '%s': ",
          keyname, value);
    
    g_object_unref(client);
    g_free(keyname);
}

/* The string must be freed afterward. */
gchar *config_file_get_string(gchar *path, gchar *key) {
    g_return_val_if_fail(path != NULL, NULL);
    g_return_val_if_fail(key != NULL, NULL);
    
    GError *err = NULL;
    GConfClient *client = gconf_client_get_default();
    
    gchar *keyname = g_strconcat(GCONF_BASE_PATH "/", path, "/", key, NULL);
    gchar *value;
    
    if(!(value = gconf_client_get_string(client, keyname, &err))) {
        /* NULL is returned if the value is not set or an error occurs */
        if(err) /* err is not set if the key simply is unset */
            error_dialog(NULL, err, "Could not get GConf key '%s': ", keyname);
    }
    
    g_object_unref(client);
    g_free(keyname);
    return value;
}

gint config_file_get_int(gchar *path, gchar *key) {
    g_return_val_if_fail(path != NULL, 0);
    g_return_val_if_fail(key != NULL, 0);
    
    GError *err = NULL;
    GConfClient *client = gconf_client_get_default();
    
    gchar *keyname = g_strconcat(GCONF_BASE_PATH "/", path, "/", key, NULL);
    gint value;
    
    if(!(value = gconf_client_get_int(client, keyname, &err))) {
        /* NULL is returned if the value is not set or an error occurs */
        if(err) /* err is not set if the key simply is unset */
            error_dialog(NULL, err, "Could not get GConf key '%s': ", keyname);
    }
    
    g_object_unref(client);
    g_free(keyname);
    return value;
}

gboolean config_file_get_bool(gchar *path, gchar *key) {
    g_return_val_if_fail(path != NULL, FALSE);
    g_return_val_if_fail(key != NULL, FALSE);
    
    GError *err = NULL;
    GConfClient *client = gconf_client_get_default();
    
    gchar *keyname = g_strconcat(GCONF_BASE_PATH "/", path, "/", key, NULL);
    gboolean value;
    
    if(!(value = gconf_client_get_bool(client, keyname, &err))) {
        /* NULL is returned if the value is not set or an error occurs */
        if(err) /* err is not set if the key simply is unset */
            error_dialog(NULL, err, "Could not get GConf key '%s': ", keyname);
    }
    
    g_object_unref(client);
    g_free(keyname);
    return value;
}
