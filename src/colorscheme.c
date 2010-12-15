/* Copyright (C) 2006-2009, 2010 P. F. Chimento
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

static GtkSourceStyleSchemeManager *scheme_manager = NULL;

/* Scheme manager functions adapted from gedit-style-scheme-manager.c */

#define USER_STYLES_DIR ".gnome2/inform7/styles"

static gchar *
get_user_styles_dir(void)
{
	const gchar *home = g_get_home_dir();
	if(home)
		return g_build_filename(home, USER_STYLES_DIR, NULL);
	return NULL;
}

static GtkSourceStyleSchemeManager *
get_style_scheme_manager(void)
{
	if(!scheme_manager) {
		scheme_manager = gtk_source_style_scheme_manager_new();
		/* Add the built-in styles directory */
		gchar *dir = i7_app_get_datafile_path_va(i7_app_get(), "styles", NULL);
		gtk_source_style_scheme_manager_append_search_path(scheme_manager, dir);
		g_free(dir);
		/* Add the user styles directory */
		dir = get_user_styles_dir();
		if(dir) {
			gtk_source_style_scheme_manager_append_search_path(scheme_manager, dir);
			g_free(dir);
		}
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
	const gchar *home = g_get_home_dir();
	if(!home)
		return FALSE;

	gchar *dir = g_build_filename(home, USER_STYLES_DIR, NULL);
	gboolean retval = g_str_has_prefix(filename, dir);
	g_free(dir);

	return retval;
}

/**
 * file_copy:
 * @name: a pointer to a %NULL-terminated string, that names
 * the file to be copied, in the GLib file name encoding
 * @dest_name: a pointer to a %NULL-terminated string, that is the
 * name for the destination file, in the GLib file name encoding
 * @error: return location for a #GError, or %NULL
 *
 * Copies file @name to @dest_name.
 *
 * If the call was successful, it returns %TRUE. If the call was not
 * successful, it returns %FALSE and sets @error. The error domain
 * is #G_FILE_ERROR. Possible error
 * codes are those in the #GFileError enumeration.
 *
 * Return value: %TRUE on success, %FALSE otherwise.
 */
static gboolean
file_copy(const gchar *name, const gchar *dest_name, GError **error)
{
	g_return_val_if_fail(name, FALSE);
	g_return_val_if_fail(dest_name, FALSE);
	g_return_val_if_fail(error == NULL || *error == NULL, FALSE);

	/* Note: we allow to copy a file to itself since this is not a problem
	 * in our use case */

	/* Ensure the destination directory exists */
	gchar *dest_dir = g_path_get_dirname(dest_name);

	errno = 0;
	if(g_mkdir_with_parents(dest_dir, 0755) != 0) {
		gint save_errno = errno;
		gchar *display_filename = g_filename_display_name(dest_dir);
		g_set_error(error, G_FILE_ERROR, g_file_error_from_errno(save_errno),
			_("Directory '%s' could not be created: g_mkdir_with_parents() failed: %s"),
			display_filename, g_strerror(save_errno));
		g_free(dest_dir);
		g_free(display_filename);

		return FALSE;
	}
	g_free(dest_dir);

	gchar *contents;
	gsize length;
	if(!g_file_get_contents(name, &contents, &length, error))
		return FALSE;
	if(!g_file_set_contents(dest_name, contents, length, error))
		return FALSE;
	g_free(contents);

	return TRUE;
}

/**
 * install_scheme:
 * @fname: the file name of the style scheme to be installed
 *
 * Install a new user scheme.
 * This function copies @fname in #USER_STYLES_DIR and ask the style manager to
 * recompute the list of available style schemes. It then checks if a style
 * scheme with the right file name exists.
 *
 * If the call was succesful, it returns the id of the installed scheme
 * otherwise %NULL.
 *
 * Return value: the id of the installed scheme, %NULL otherwise.
 */
const gchar *
install_scheme(const gchar *fname)
{
	gchar *new_file_name = NULL;
	GError *error = NULL;
	gboolean copied = FALSE;

	g_return_val_if_fail(fname != NULL, NULL);

	GtkSourceStyleSchemeManager *manager = get_style_scheme_manager();
	gchar *dirname = g_path_get_dirname(fname);
	gchar *styles_dir = get_user_styles_dir();

	if(strcmp(dirname, styles_dir) != 0) {
		gchar *basename = g_path_get_basename(fname);
		new_file_name = g_build_filename(styles_dir, basename, NULL);
		g_free(basename);

		/* Copy the style scheme file into USER_STYLES_DIR */
		if(!file_copy(fname, new_file_name, &error)) {
			g_free(new_file_name);
			WARN(_("Cannot install style scheme"), error);
			return NULL;
		}
		copied = TRUE;
	} else
		new_file_name = g_strdup(fname);

	g_free(dirname);
	g_free(styles_dir);

	/* Reload the available style schemes */
	gtk_source_style_scheme_manager_force_rescan(manager);

	/* Check the new style scheme has been actually installed */
	const gchar * const *ids = gtk_source_style_scheme_manager_get_scheme_ids(manager);

	while(*ids != NULL) {
		GtkSourceStyleScheme *scheme = gtk_source_style_scheme_manager_get_scheme(manager, *ids);
		const gchar *filename = gtk_source_style_scheme_get_filename(scheme);

		if(filename && (strcmp(filename, new_file_name) == 0))	{
			/* The style scheme has been correctly installed */
			g_free (new_file_name);
			return gtk_source_style_scheme_get_id(scheme);
		}
		++ids;
	}

	/* The style scheme has not been correctly installed */
	if(copied)
		g_unlink(new_file_name);

	g_free(new_file_name);

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
	g_return_val_if_fail (id != NULL, FALSE);

	GtkSourceStyleSchemeManager *manager = get_style_scheme_manager();
	GtkSourceStyleScheme *scheme = gtk_source_style_scheme_manager_get_scheme(manager, id);
	if(!scheme)
		return FALSE;

	const gchar *filename = gtk_source_style_scheme_get_filename(scheme);
	if(!filename)
		return FALSE;

	if(g_unlink(filename) == -1)
		return FALSE;

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
