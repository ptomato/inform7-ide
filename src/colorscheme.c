/* Copyright (C) 2006-2009, 2010, 2011, 2012 P. F. Chimento
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
#include <gtksourceview/gtksourcelanguage.h>
#include <gtksourceview/gtksourcestyle.h>
#include <gtksourceview/gtksourcestyleschememanager.h>

#include "colorscheme.h"
#include "app.h"
#include "configfile.h"
#include "error.h"
#include "file.h"

static GtkSourceStyleSchemeManager *scheme_manager = NULL;

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

/* Helper function: call gtk_source_style_scheme_manager_append_search_path()
with a #GFile */
static void
scheme_manager_append_search_path_gfile(GtkSourceStyleSchemeManager *manager, GFile *file)
{
	char *path = g_file_get_path(file);
	gtk_source_style_scheme_manager_append_search_path(scheme_manager, path);
	g_free(path);
}

/* Helper function: get the static style scheme manager, or create it if it
doesn't exist yet */
static GtkSourceStyleSchemeManager *
get_style_scheme_manager(void)
{
	if(G_UNLIKELY(!scheme_manager)) {
		scheme_manager = gtk_source_style_scheme_manager_new();

		/* Add the built-in styles directory */
		GFile *styles_dir = i7_app_get_data_file(i7_app_get(), "styles");
		scheme_manager_append_search_path_gfile(scheme_manager, styles_dir);
		g_object_unref(styles_dir);

		/* Add the user styles directory */
		styles_dir = get_user_styles_dir();
		scheme_manager_append_search_path_gfile(scheme_manager, styles_dir);
		g_object_unref(styles_dir);
	}

	return scheme_manager;
}

static gint
schemes_compare(GtkSourceStyleScheme *a, GtkSourceStyleScheme *b)
{
	const gchar *name_a = gtk_source_style_scheme_get_name(a);
	const gchar *name_b = gtk_source_style_scheme_get_name(b);
	return g_utf8_collate(name_a, name_b);
}

GSList *
get_style_schemes_sorted()
{
	GSList *schemes = NULL;
	GtkSourceStyleSchemeManager *manager = get_style_scheme_manager();
	const gchar * const *scheme_ids = gtk_source_style_scheme_manager_get_scheme_ids(manager);

	while (*scheme_ids != NULL) {
		GtkSourceStyleScheme *scheme = gtk_source_style_scheme_manager_get_scheme(manager, *scheme_ids);
		schemes = g_slist_prepend(schemes, scheme);
		++scheme_ids;
	}

	if(schemes != NULL)
		schemes = g_slist_sort(schemes, (GCompareFunc)schemes_compare);

	return schemes;
}

gboolean
is_user_scheme(const gchar *scheme_id)
{
	GtkSourceStyleSchemeManager *manager = get_style_scheme_manager();
	GtkSourceStyleScheme *scheme = gtk_source_style_scheme_manager_get_scheme(manager, scheme_id);
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
 * install_scheme:
 * @file: a #GFile reference to the style scheme to be installed
 *
 * Install a new user scheme.
 * This function copies @fname into #USER_STYLES_DIR and asks the style manager
 * to recompute the list of available style schemes. It then checks if a style
 * scheme with the right file name exists.
 *
 * If the call was succesful, it returns the id of the installed scheme
 * otherwise %NULL.
 *
 * Return value: the id of the installed scheme, %NULL on error.
 */
const gchar *
install_scheme(GFile *file)
{
	GFile *new_file = NULL;
	GError *error = NULL;
	gboolean copied = FALSE;

	g_return_val_if_fail(file != NULL, NULL);

	GtkSourceStyleSchemeManager *manager = get_style_scheme_manager();
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
	gtk_source_style_scheme_manager_force_rescan(manager);

	/* Check the new style scheme has been actually installed */
	const gchar * const *ids = gtk_source_style_scheme_manager_get_scheme_ids(manager);

	while(*ids != NULL) {
		GtkSourceStyleScheme *scheme = gtk_source_style_scheme_manager_get_scheme(manager, *ids);
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
 * uninstall_scheme:
 * @id: the id of the style scheme to be uninstalled
 *
 * Uninstall a user scheme.
 *
 * If the call was succesful, it returns %TRUE
 * otherwise %FALSE.
 *
 * Return value: %TRUE on success, %FALSE otherwise.
 */
gboolean
uninstall_scheme(const gchar *id)
{
	GError *error = NULL;

	g_return_val_if_fail (id != NULL, FALSE);

	GtkSourceStyleSchemeManager *manager = get_style_scheme_manager();
	GtkSourceStyleScheme *scheme = gtk_source_style_scheme_manager_get_scheme(manager, id);
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
	gtk_source_style_scheme_manager_force_rescan(manager);

	return TRUE;
}

/* Get the appropriate color scheme for the current settings. Return value must
not be unref'd. */
GtkSourceStyleScheme *
get_style_scheme(void)
{
	GtkSourceStyleSchemeManager *manager = get_style_scheme_manager();
	gchar *scheme_name = config_file_get_string(PREFS_STYLE_SCHEME);
	GtkSourceStyleScheme *scheme = gtk_source_style_scheme_manager_get_scheme(manager, scheme_name);
	g_free(scheme_name);
	return scheme;
}

/* Set up the style colors for the Natural Inform highlighting */
void
set_highlight_styles(GtkSourceBuffer *buffer)
{
	GtkSourceStyleScheme *scheme = get_style_scheme();
	gtk_source_buffer_set_style_scheme(buffer, scheme);
}

#if 0
/* Return the GdkColor in the current scheme */
GdkColor
get_scheme_color(int color)
{
	GdkColor *scheme;
	switch(config_file_get_enum("EditorSettings", "ColorSet",
	  color_set_lookup_table)) {
		case COLOR_SET_SUBDUED:
			scheme = scheme_subdued;
			break;
		case COLOR_SET_PSYCHEDELIC:
			scheme = scheme_psychedelic;
			break;
		case COLOR_SET_STANDARD:
		default:
			scheme = scheme_standard;
	}
	return scheme[color];
}
#endif
