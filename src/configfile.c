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
#include "error.h"
#include "prefs.h"

#define GCONF_BASE_PATH "/apps/gnome-inform7/"
/* The above are not directory separators, so they should be slashes! */

/* Make a full key name out of the relative path and key name. String must be
freed afterwards. */
static gchar *
make_keyname(const gchar *path, const gchar *key)
{
    return g_strconcat(GCONF_BASE_PATH, path, "/", key, NULL);
    /* The slash is not a directory separator */
}

/* The following three functions check whether a key exists and if not, set it
to its default value */
static void
config_file_default_string(const gchar *path, const gchar *key,
                           const gchar *defval)
{
    g_return_if_fail(path != NULL);
    g_return_if_fail(key != NULL);
    g_return_if_fail(defval != NULL);
    
    GError *err = NULL;
    GConfClient *client = gconf_client_get_default();
    
    gchar *keyname = make_keyname(path, key);
    gchar *value;

    if(!(value = gconf_client_get_string(client, keyname, &err))) {
        /* NULL is returned if the value is not set or an error occurs */
        if(err) /* err is not set if the key simply is unset */
            error_dialog(NULL, err, "Could not get GConf key '%s': ", keyname);
        else if(!gconf_client_set_string(client, keyname, defval, &err))
            error_dialog(NULL, err, "Could not set GConf key '%s' to '%s': ", 
                         keyname, defval);
    } else
        g_free(value);
    
    g_object_unref(client);
    g_free(keyname);
}

static void
config_file_default_int(const gchar *path, const gchar *key, const gint defval)
{
    g_return_if_fail(path != NULL);
    g_return_if_fail(key != NULL);
    
    GError *err = NULL;
    GConfClient *client = gconf_client_get_default();
    
    gchar *keyname = make_keyname(path, key);
    gint value;

    if(!(value = gconf_client_get_int(client, keyname, &err))) {
        /* NULL is returned if the value is not set or an error occurs */
        if(err) /* err is not set if the key simply is unset */
            error_dialog(NULL, err, "Could not get GConf key '%s': ", keyname);
        else if(!gconf_client_set_int(client, keyname, defval, &err))
            error_dialog(NULL, err, "Could not set GConf key '%s' to '%s': ", 
                         keyname, defval);
    }
    
    g_object_unref(client);
    g_free(keyname);
}

static void
config_file_default_bool(const gchar *path, const gchar *key,
                         const gboolean defval)
{
    g_return_if_fail(path != NULL);
    g_return_if_fail(key != NULL);
    
    GError *err = NULL;
    GConfClient *client = gconf_client_get_default();
    
    gchar *keyname = make_keyname(path, key);
    gboolean value;

    if(!(value = gconf_client_get_bool(client, keyname, &err))) {
        /* NULL is returned if the value is not set or an error occurs */
        if(err) /* err is not set if the key simply is unset */
            error_dialog(NULL, err, "Could not get GConf key '%s': ", keyname);
        else if(!gconf_client_set_bool(client, keyname, defval, &err))
            error_dialog(NULL, err, "Could not set GConf key '%s' to '%s': ", 
                         keyname, defval);
    }
    
    g_object_unref(client);
    g_free(keyname);
}

/* Check if the config keys exist and if not, set them to defaults. */
/* This could probably be done a lot better with a GConf schema, but,
unfortunately, there is no documentation on it! */
void
check_config_file()
{
    config_file_default_string("User", "Name", "");
    /* LastProject should stay empty, it's set elsewhere */
    config_file_default_bool("Settings", "SpellCheck", FALSE);
    config_file_default_bool("Settings", "InspectorVisible", FALSE);
    config_file_default_int("Settings", "InspectorPosX", 0);
    config_file_default_int("Settings", "InspectorPosY", 0);
    config_file_default_int("Settings", "AppWindowWidth", 800);
    config_file_default_int("Settings", "AppWindowHeight", 600);
    config_file_default_int("Settings", "SliderPosition", 400);
    config_file_default_int("Settings", "ExtWindowWidth", 400);
    config_file_default_int("Settings", "ExtWindowHeight", 600);
    config_file_default_int("Fonts", "FontSet", FONT_SET_STANDARD);
    config_file_default_string("Fonts", "CustomFont", "DejaVu Sans 11");
    config_file_default_int("Fonts", "FontStyling", FONT_STYLING_OFTEN);
    config_file_default_int("Fonts", "FontSize", FONT_SIZE_STANDARD);
    config_file_default_int("Colors", "ChangeColors", CHANGE_COLORS_OFTEN);
    config_file_default_int("Colors", "ColorSet", COLOR_SET_STANDARD);
    config_file_default_int("Tabs", "TabWidth", 0);
    config_file_default_bool("Inspectors", "ProjectFiles", FALSE);
    config_file_default_bool("Inspectors", "Notes", TRUE);
    config_file_default_bool("Inspectors", "Headings", TRUE);
    config_file_default_bool("Inspectors", "Skein", FALSE);
    config_file_default_bool("Inspectors", "Watchpoints", FALSE);
    config_file_default_bool("Inspectors", "Breakpoints", FALSE);
    config_file_default_bool("Inspectors", "Search", TRUE);
    config_file_default_bool("Syntax", "Highlighting", TRUE);
    config_file_default_bool("Syntax", "Indenting", TRUE);
    config_file_default_bool("Syntax", "Intelligence", TRUE);
    config_file_default_bool("Syntax", "IntelligentIndexInspector", TRUE);
    config_file_default_bool("Syntax", "AutoIndent", TRUE);
    config_file_default_bool("Syntax", "AutoNumberSections", FALSE);
    config_file_default_bool("Cleaning", "BuildFiles", TRUE);
    config_file_default_bool("Cleaning", "IndexFiles", FALSE);
    config_file_default_bool("Debugging", "ShowLog", FALSE);
    config_file_default_bool("Debugging", "RebuildCompiler", FALSE);
    config_file_default_int("Skein", "HorizontalSpacing", 40);
    config_file_default_int("Skein", "VerticalSpacing", 75);
}

/* The next six functions are wrappers for GConf setting and getting functions,
that give us a nice error dialog if they fail. */

void
config_file_set_string(const gchar *path, const gchar *key, const gchar *value)
{
    g_return_if_fail(path != NULL);
    g_return_if_fail(key != NULL);
    g_return_if_fail(value != NULL);
    
    GError *err = NULL;
    GConfClient *client = gconf_client_get_default();
    
    gchar *keyname = make_keyname(path, key);
    
    if(!gconf_client_set_string(client, keyname, value, &err))
        error_dialog(NULL, err, "Could not set GConf key '%s' to '%s': ",
          keyname, value);

    g_object_unref(client);
    g_free(keyname);
}

void
config_file_set_int(const gchar *path, const gchar *key, const gint value)
{
    g_return_if_fail(path != NULL);
    g_return_if_fail(key != NULL);
    
    GError *err = NULL;
    GConfClient *client = gconf_client_get_default();
    
    gchar *keyname = make_keyname(path, key);
    
    if(!gconf_client_set_int(client, keyname, value, &err))
        error_dialog(NULL, err, "Could not set GConf key '%s' to '%s': ",
          keyname, value);
        
    g_object_unref(client);
    g_free(keyname);
}

void
config_file_set_bool(const gchar *path, const gchar *key, const gboolean value)
{
    g_return_if_fail(path != NULL);
    g_return_if_fail(key != NULL);
    
    GError *err = NULL;
    GConfClient *client = gconf_client_get_default();
    
    gchar *keyname = make_keyname(path, key);
    
    if(!gconf_client_set_bool(client, keyname, value, &err)) 
        error_dialog(NULL, err, "Could not set GConf key '%s' to '%s': ",
          keyname, value);
    
    g_object_unref(client);
    g_free(keyname);
}

/* The string must be freed afterward. Returns NULL if key is unset*/
gchar *
config_file_get_string(const gchar *path, const gchar *key)
{
    g_return_val_if_fail(path != NULL, NULL);
    g_return_val_if_fail(key != NULL, NULL);
    
    GError *err = NULL;
    GConfClient *client = gconf_client_get_default();
    
    gchar *keyname = make_keyname(path, key);
    gchar *value;
    
    if(!(value = gconf_client_get_string(client, keyname, &err))) {
        /* NULL is returned if the value is not set or an error occurs */
        if(err) /* err is not set if the key simply is unset */
            error_dialog(NULL, err, "Could not get GConf key '%s': ", keyname);
        else 
            value = NULL;
    }
    
    g_object_unref(client);
    g_free(keyname);
    return value;
}

gint
config_file_get_int(const gchar *path, const gchar *key)
{
    g_return_val_if_fail(path != NULL, 0);
    g_return_val_if_fail(key != NULL, 0);
    
    GError *err = NULL;
    GConfClient *client = gconf_client_get_default();
    
    gchar *keyname = make_keyname(path, key);
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

gboolean
config_file_get_bool(const gchar *path, const gchar *key)
{
    g_return_val_if_fail(path != NULL, FALSE);
    g_return_val_if_fail(key != NULL, FALSE);
    
    GError *err = NULL;
    GConfClient *client = gconf_client_get_default();
    
    gchar *keyname = make_keyname(path, key);
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
