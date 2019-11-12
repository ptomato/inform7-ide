/* Copyright (C) 2006-2009, 2010, 2011, 2012, 2013, 2018 P. F. Chimento
 * This file is part of GNOME Inform 7.
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

#include <errno.h>
#include <string.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>

#include "app.h"
#include "app-private.h"
#include "configfile.h"
#include "error.h"
#include "file.h"

/* Scheme manager functions adapted from gedit-style-scheme-manager.c */

/* Helper function: Gets the directory for user-installed style files */
static GFile *
get_user_styles_dir(void)
{
	const char *config = g_get_user_config_dir();
	char *path = g_build_filename(config, "inform7", "styles", NULL);
	GFile *retval = g_file_new_for_path(path);
	g_free(path);
	return retval;
}

static gint
schemes_compare(GtkSourceStyleScheme *a, GtkSourceStyleScheme *b)
{
	const gchar *name_a = gtk_source_style_scheme_get_name(a);
	const gchar *name_b = gtk_source_style_scheme_get_name(b);
	return g_utf8_collate(name_a, name_b);
}

/**
 * i7_app_foreach_color_scheme:
 * @self: the app
 * @func: an #GFunc callback
 * @data: user data to pass to @func
 *
 * Iterates through the installed color schemes (system-wide and user-installed)
 * sorted alphabetically by name, and calls @func on each one.
 */
void
i7_app_foreach_color_scheme(I7App *self, GFunc func, gpointer data)
{
	I7_APP_USE_PRIVATE(self, priv);

	GSList *schemes = NULL;
	const char * const *scheme_ids = gtk_source_style_scheme_manager_get_scheme_ids(priv->color_scheme_manager);

	while (*scheme_ids != NULL) {
		GtkSourceStyleScheme *scheme = gtk_source_style_scheme_manager_get_scheme(priv->color_scheme_manager, *scheme_ids);
		schemes = g_slist_prepend(schemes, scheme);
		++scheme_ids;
	}

	if(schemes != NULL)
		schemes = g_slist_sort(schemes, (GCompareFunc)schemes_compare);

	g_slist_foreach(schemes, func, data);
	g_slist_free(schemes);
}

/**
 * i7_app_color_scheme_is_user_scheme:
 * @self: the app
 * @id: the string ID of a color scheme
 *
 * Determines whether a particular color scheme is an application-specific
 * scheme installed by the user.
 *
 * Returns: %TRUE if @id is user-installed, %FALSE if not.
 */
gboolean
i7_app_color_scheme_is_user_scheme(I7App *self, const char *id)
{
	I7_APP_USE_PRIVATE(self, priv);

	GtkSourceStyleScheme *scheme = gtk_source_style_scheme_manager_get_scheme(priv->color_scheme_manager, id);
	if(!scheme)
		return FALSE;
	const gchar *filename = gtk_source_style_scheme_get_filename(scheme);
	if(!filename)
		return FALSE;

	GFile *scheme_file = g_file_new_for_path(filename);
	GFile *user_styles_dir = get_user_styles_dir();
	gboolean retval = g_file_has_parent(scheme_file, user_styles_dir);
	g_object_unref(scheme_file);
	g_object_unref(user_styles_dir);

	return retval;
}

/**
 * i7_app_install_color_scheme:
 * @self: the app
 * @file: a #GFile reference to the color scheme to be installed
 *
 * Install a new user color scheme.
 *
 * This function copies @file into the user color scheme directory and asks the
 * style manager to recompute the list of available style schemes. It then
 * checks if a style scheme with the right file name exists.
 *
 * Return value: (allow-none): the id of the installed scheme, %NULL on error.
 */
const char *
i7_app_install_color_scheme(I7App *self, GFile *file)
{
	I7_APP_USE_PRIVATE(self, priv);

	GFile *new_file = NULL;
	GError *error = NULL;
	gboolean copied = FALSE;

	g_return_val_if_fail(file != NULL, NULL);

	GFile *styles_dir = get_user_styles_dir();

	if(!g_file_has_parent(file, styles_dir)) {

		/* Make sure USER_STYLES_DIR exists */
		if(!make_directory_unless_exists(styles_dir, NULL, &error)) {
			g_object_unref(styles_dir);
			WARN(_("Cannot create user styles directory"), error);
			g_error_free(error);
			return NULL;
		}

		char *basename = g_file_get_basename(file);
		new_file = g_file_get_child(styles_dir, basename);
		g_free(basename);

		/* Copy the style scheme file into USER_STYLES_DIR */
		if(!g_file_copy(file, new_file, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, &error)) {
			g_object_unref(new_file);
			g_object_unref(styles_dir);
			WARN(_("Cannot install style scheme"), error);
			g_error_free(error);
			return NULL;
		}
		copied = TRUE;
	} else
		new_file = g_object_ref(file);

	g_object_unref(styles_dir);

	/* Reload the available style schemes */
	gtk_source_style_scheme_manager_force_rescan(priv->color_scheme_manager);

	/* Check the new style scheme has been actually installed */
	const char * const *ids = gtk_source_style_scheme_manager_get_scheme_ids(priv->color_scheme_manager);

	while(*ids != NULL) {
		GtkSourceStyleScheme *scheme = gtk_source_style_scheme_manager_get_scheme(priv->color_scheme_manager, *ids);
		const gchar *filename = gtk_source_style_scheme_get_filename(scheme);
		char *new_path = g_file_get_path(new_file);

		if(filename && (strcmp(filename, new_path) == 0))	{
			/* The style scheme has been correctly installed */
			g_object_unref(new_file);
			g_free(new_path);
			return gtk_source_style_scheme_get_id(scheme);
		}
		++ids;
	}

	/* The style scheme has not been correctly installed */
	if(copied)
		g_file_delete(new_file, NULL, NULL); /* ignore error */

	g_object_unref(new_file);

	return NULL;
}

/**
 * i7_app_uninstall_color_scheme:
 * @self: the app
 * @id: the id of the color scheme to be uninstalled
 *
 * Uninstalls a user color scheme.
 *
 * Return value: %TRUE on success, %FALSE otherwise.
 */
gboolean
i7_app_uninstall_color_scheme(I7App *self, const char *id)
{
	I7_APP_USE_PRIVATE(self, priv);

	GError *error = NULL;

	g_return_val_if_fail (id != NULL, FALSE);

	GtkSourceStyleScheme *scheme = gtk_source_style_scheme_manager_get_scheme(priv->color_scheme_manager, id);
	if(!scheme)
		return FALSE;

	const gchar *filename = gtk_source_style_scheme_get_filename(scheme);
	if(!filename)
		return FALSE;

	GFile *file = g_file_new_for_path(filename);
	if(!g_file_delete(file, NULL, &error)) {
		WARN(_("Cannot uninstall style scheme"), error);
		g_error_free(error);
		g_object_unref(file);
		return FALSE;
	}
	g_object_unref(file);

	/* Reload the available style schemes */
	gtk_source_style_scheme_manager_force_rescan(priv->color_scheme_manager);

	return TRUE;
}

/**
 * i7_app_get_current_color_scheme:
 * @self: the app
 *
 * Get the appropriate color scheme for the current settings.
 *
 * Returns: (transfer none): the appropriate #GtkSourceStyleScheme.
 */
GtkSourceStyleScheme *
i7_app_get_current_color_scheme(I7App *self)
{
	I7_APP_USE_PRIVATE(self, priv);
	GSettings *prefs = i7_app_get_prefs(self);
	gchar *scheme_name = g_settings_get_string(prefs, PREFS_STYLE_SCHEME);
	GtkSourceStyleScheme *scheme = gtk_source_style_scheme_manager_get_scheme(priv->color_scheme_manager, scheme_name);
	g_free(scheme_name);
	return scheme;
}
