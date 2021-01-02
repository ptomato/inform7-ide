/* Copyright (C) 2006-2009, 2010, 2011, 2012, 2013, 2014, 2015 P. F. Chimento
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

#include "config.h"

#include <sys/types.h>
#include <pwd.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>

#include "configfile.h"
#include "error.h"
#include "file.h"
#include "story.h"
#include "document.h"

/* HELPER FUNCTIONS */

/* Read a source file into a string. Allocates a new string */
gchar *
read_source_file(GFile *file)
{
	GError *error = NULL;
	gchar *text;

	gsize num_bytes;
	if(!g_file_load_contents(file, NULL, &text, &num_bytes, NULL, &error)) {
		error_dialog_file_operation(NULL, file, error, I7_FILE_ERROR_OPEN, NULL);
		return NULL;
	}

	/* Make sure the file wasn't binary or something */
	if(!g_utf8_validate(text, num_bytes, NULL)) {
		char *filename = g_file_get_path(file);
		error_dialog(NULL, NULL, _("The file '%s' could not be read because it contained invalid UTF-8 text."), filename);
		g_free(filename);
		g_free(text);
		return NULL;
	}

	/* Change newline separators to \n */
	if(strstr(text, "\r\n")) {
		gchar **lines = g_strsplit(text, "\r\n", 0);
		g_free(text);
		text = g_strjoinv("\n", lines);
		g_strfreev(lines);
	}
	text = g_strdelimit(text, "\r", '\n');

	return text;
}

/*
 * FUNCTIONS FOR SAVING AND LOADING STUFF
 */

/* Helper function to delete a file relative to the project path; does nothing
if file does not exist */
static void
delete_from_project_dir(I7Story *story, GFile *root_file, const char *subdir, const char *filename)
{
	GFile *file_to_delete;

	if(subdir) {
		GFile *child = g_file_get_child(root_file, subdir);
		file_to_delete = g_file_get_child(child, filename);
		g_object_unref(child);
	} else
		file_to_delete = g_file_get_child(root_file, filename);

	g_file_delete(file_to_delete, NULL, NULL); /* ignore errors */

	g_object_unref(file_to_delete);
	i7_document_display_progress_busy(I7_DOCUMENT(story));
}

/* If the "delete build files" option is checked, delete all the build files
from the project directory */
void
delete_build_files(I7Story *story)
{
	I7App *theapp = I7_APP(g_application_get_default());
	GSettings *prefs = i7_app_get_prefs(theapp);

	if(g_settings_get_boolean(prefs, PREFS_CLEAN_BUILD_FILES)) {
		i7_document_display_status_message(I7_DOCUMENT(story), _("Cleaning out build files..."), FILE_OPERATIONS);

		GFile *storyname = i7_document_get_file(I7_DOCUMENT(story));

		delete_from_project_dir(story, storyname, NULL, "Metadata.iFiction");
		delete_from_project_dir(story, storyname, NULL, "Release.blurb");
		delete_from_project_dir(story, storyname, "Build", "auto.inf");
		delete_from_project_dir(story, storyname, "Build", "Debug log.txt");
		delete_from_project_dir(story, storyname, "Build", "Map.eps");
		/* output.z5 and .z6 are not created, but may be present in old projects */
		delete_from_project_dir(story, storyname, "Build", "output.z5");
		delete_from_project_dir(story, storyname, "Build", "output.z6");
		delete_from_project_dir(story, storyname, "Build", "output.z8");
		delete_from_project_dir(story, storyname, "Build", "output.ulx");
		delete_from_project_dir(story, storyname, "Build", "Problems.html");
		delete_from_project_dir(story, storyname, "Build", "gameinfo.dbg");
		delete_from_project_dir(story, storyname, "Build", "temporary file.inf");
		delete_from_project_dir(story, storyname, "Build", "temporary file 2.inf");
		delete_from_project_dir(story, storyname, "Build", "StatusCblorb.html");

		if(g_settings_get_boolean(prefs, PREFS_CLEAN_INDEX_FILES)) {
			delete_from_project_dir(story, storyname, "Index", "Actions.html");
			delete_from_project_dir(story, storyname, "Index", "Contents.html");
			delete_from_project_dir(story, storyname, "Index", "Headings.xml");
			delete_from_project_dir(story, storyname, "Index", "Kinds.html");
			delete_from_project_dir(story, storyname, "Index", "Phrasebook.html");
			delete_from_project_dir(story, storyname, "Index", "Rules.html");
			delete_from_project_dir(story, storyname, "Index", "Scenes.html");
			delete_from_project_dir(story, storyname, "Index", "World.html");

			/* Delete the "Details" subdirectory */

			GFile *temp = g_file_get_child(storyname, "Index");
			GFile *details_file = g_file_get_child(temp, "Details");
			g_object_unref(temp);
			g_object_unref(storyname);

			/* Remove each file in the directory */
			GFileEnumerator *details_dir = g_file_enumerate_children(details_file, G_FILE_ATTRIBUTE_STANDARD_NAME, G_FILE_QUERY_INFO_NONE, NULL, NULL);
			if(details_dir) {
				GFileInfo *info;
				while((info = g_file_enumerator_next_file(details_dir, NULL, NULL)) != NULL) {
					const char *child_name = g_file_info_get_name(info);
					GFile *child = g_file_get_child(details_file, child_name);
					g_file_delete(child, NULL, NULL);
					g_object_unref(child);
					g_object_unref(info);
					i7_document_display_progress_busy(I7_DOCUMENT(story));
				}
				g_file_enumerator_close(details_dir, NULL, NULL);
				g_object_unref(details_dir);
			}

			/* Remove the directory */
			g_file_delete(details_file, NULL, NULL);
			g_object_unref(details_file);
		}
	}
	i7_document_remove_status_message(I7_DOCUMENT(story), FILE_OPERATIONS);
	i7_document_display_progress_percentage(I7_DOCUMENT(story), 0.0);
}

/*
 * get_match_within_directory:
 * @d: a #GFileEnumerator for the directory to search
 * @name: a string to search case-insensitively for
 *
 * Helper function: return the first match within directory @d which matches
 * @name case-insensitively.
 *
 * Returns: (transfer full): a #GFile
 */
static GFile *
get_match_within_directory(GFileEnumerator *d, const char *name)
{
	GFileInfo *info;
	while((info = g_file_enumerator_next_file(d, NULL, NULL)) != NULL) {
		const char *basename = g_file_info_get_name(info);
		if(strcasecmp(name, basename) == 0) {
			GFile *retval = g_file_get_child(g_file_enumerator_get_container(d), basename);
			g_object_unref(info);
			return retval;
		}
		g_object_unref(info);
	}
	return NULL;
}

/**
 * get_case_insensitive_extension:
 * @file: a #GFile
 *
 * Helper function: find the on-disk filename of @file, whose last two
 * components may have incorrect casing. Adapted from Sec. 2/cifn of Inform
 * source, which was written by Adam Thornton.
 *
 * Returns: (transfer full): a #GFile containing the on-disk filename, or %NULL
 * on failure.
 */
GFile *
get_case_insensitive_extension(GFile *file)
{
	GFile *retval;

	g_return_val_if_fail(file, NULL);

	/* For efficiency's sake, though it's logically equivalent, we try... */
	if(g_file_query_exists(file, NULL))
		return g_object_ref(file);

	/* Parse the path to break it into top directory, extension directory, and
	 leafname */
	GFile *extdir_file = g_file_get_parent(file);
	GFile *topdir_file = g_file_get_parent(extdir_file);
	char *ciextdir_basename = g_file_get_basename(extdir_file);
	char *cileaf_basename = g_file_get_basename(file);

	/* topdir is assumed case-correct */
	if(!g_file_query_exists(extdir_file, NULL)) {
		g_object_unref(extdir_file);
		GFileEnumerator *topdir = g_file_enumerate_children(topdir_file, G_FILE_ATTRIBUTE_STANDARD_NAME, G_FILE_QUERY_INFO_NONE, NULL, NULL);
		if(!topdir)
			goto fail;

		/* Try to find a unique insensitively matching directory name in topdir */
		extdir_file = get_match_within_directory(topdir, ciextdir_basename);

		g_file_enumerator_close(topdir, NULL, NULL);
		g_object_unref(topdir);

		if(!extdir_file)
			goto fail;
	}

	retval = g_file_get_child(extdir_file, cileaf_basename);
	if(g_file_query_exists(retval, NULL))
		goto finally;
	g_object_unref(retval);

	/* Try to find a unique insensitively matching entry in extdir */
	GFileEnumerator *extdir = g_file_enumerate_children(extdir_file, G_FILE_ATTRIBUTE_STANDARD_NAME, G_FILE_QUERY_INFO_NONE, NULL, NULL);
	if(!extdir)
		goto fail1;

	retval = get_match_within_directory(extdir, cileaf_basename);

	g_file_enumerator_close(extdir, NULL, NULL);
	g_object_unref(extdir);

	if(!retval)
		goto fail1;

finally:
	g_object_unref(extdir_file);
	g_object_unref(topdir_file);
	g_free(ciextdir_basename);
	g_free(cileaf_basename);
	return retval;
fail1:
	g_object_unref(extdir_file);
fail:
	g_object_unref(topdir_file);
	g_free(ciextdir_basename);
	g_free(cileaf_basename);
	return NULL;
}

/**
 * make_directory_unless_exists:
 * @file: a #GFile pointing to the directory to create.
 * @cancellable: a #GCancellable, or %NULL.
 * @error: return location for an error, or %NULL.
 *
 * Does the same thing as g_file_make_directory_with_parents(), except that it
 * doesn't fail if the directory already exists.
 *
 * Returns: %TRUE on success, %FALSE if @error was set.
 */
gboolean
make_directory_unless_exists(GFile *file, GCancellable *cancellable, GError **error)
{
	gboolean retval;

	retval = g_file_make_directory_with_parents(file, cancellable, error);
	if(!retval && g_error_matches(*error, G_IO_ERROR, G_IO_ERROR_EXISTS)) {
		retval = TRUE;
		g_clear_error(error);
	}
	return retval;
}

/**
 * file_exists_and_is_dir:
 * @file: a #GFile.
 *
 * Figures out whether @file points to a real on-disk directory. Ignores errors
 * and cannot be canceled. (This is a convenience function.)
 *
 * Returns: %TRUE if @file exists and is a directory, %FALSE if not.
 */
gboolean
file_exists_and_is_dir(GFile *file)
{
	return g_file_query_exists(file, NULL) && g_file_query_file_type(file, G_FILE_QUERY_INFO_NONE, NULL) == G_FILE_TYPE_DIRECTORY;
}

/**
 * file_exists_and_is_symlink:
 * @file: a #GFile.
 *
 * Figures out whether @file points to a real on-disk symbolic link. Ignores
 * errors and cannot be canceled. (This is a convenience function.)
 *
 * Returns: %TRUE if @file exists and is a symlink, %FALSE if not.
 */
gboolean
file_exists_and_is_symlink(GFile *file)
{
	return g_file_query_exists(file, NULL) && g_file_query_file_type(file, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL) == G_FILE_TYPE_SYMBOLIC_LINK;
}

/**
 * file_get_display_name:
 * @file: a #GFile.
 *
 * Gets @file's display name, or provides a fallback value if not available.
 * Ignores errors and cannot be canceled. (This is a convenience function.)
 *
 * Returns: (transfer full): a string containing @file's display name in UTF-8
 * or at least Latin-1 if an error occurred. Free when done.
 */
char *
file_get_display_name(GFile *file)
{
	char *retval;
	GFileInfo *info = g_file_query_info(file, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME, G_FILE_QUERY_INFO_NONE, NULL, NULL);

	/* If everything was OK */
	if(info) {
		retval = g_strdup(g_file_info_get_display_name(info)); /* cannot return NULL (?) */
		g_object_unref(info);
		return retval;
	}

	/* If not, provide a fallback */
	char *path = g_file_get_path(file);
	retval = g_filename_display_basename(path);
	g_free(path);
	return retval;
}

/**
 * file_set_custom_icon:
 * @file: a #GFile.
 * @icon_name: icon name to set (corresponds to a MIME type?)
 *
 * Sets a custom icon on @file; useful for folders, which do not get custom
 * icons by default.
 * Ignores errors and cannot be canceled, but prints a g_warning() on failure.
 */
void
file_set_custom_icon(GFile *file, const char *icon_name)
{
	GError *error = NULL;
	if(!g_file_set_attribute_string(file, "metadata::custom-icon-name", icon_name, G_FILE_QUERY_INFO_NONE, NULL, &error)) {
		char *path = g_file_get_path(file);
		g_warning("Error setting custom icon on file %s: %s", path, error->message);
		g_free(path);
		g_error_free(error);
	}
}

/**
 * show_uri_in_browser:
 * @uri: a string with a URI to display.
 * @parent: a parent window for a possible error dialog, or %NULL.
 * @display_name: a string to display to the user in an error message instead of
 * the @uri, or %NULL.
 *
 * Tries to open @file externally and shows an error dialog if unsuccessful.
 * This function is identical to show_uri_externally() except for intent, and
 * therefore the message displayed on failure.
 *
 * If @display_name is %NULL, then @uri will be used instead.
 *
 * Returns: %TRUE if the operation succeeded, %FALSE otherwise.
 */
gboolean
show_uri_in_browser(const char *uri, GtkWindow *parent, const char *display_name)
{
	g_return_val_if_fail(uri != NULL, FALSE);
	g_return_val_if_fail(parent == NULL || GTK_IS_WINDOW(parent), FALSE);

	GError *error = NULL;

	gboolean success = gtk_show_uri_on_window(parent, uri, GDK_CURRENT_TIME, &error);
	if (!success) {
		error_dialog(parent, error,
			/* TRANSLATORS: %s can be a URL, a filename, or a noun like "the
			Inform website" */
			_("We couldn't show %s in your browser. The error was: "),
			display_name != NULL ? display_name : uri);
	}
	return success;
}

/**
 * show_uri_externally:
 * @uri: a string with a URI to display.
 * @parent: a parent window for a possible error dialog, or %NULL.
 * @display_name: a string to display to the user in an error message instead of
 * the @uri, or %NULL.
 *
 * Tries to open @file externally and shows an error dialog if unsuccessful.
 * This function is identical to show_uri_in_browser() except for intent, and
 * therefore the message displayed on failure.
 *
 * If @display_name is %NULL, then @uri will be used instead.
 *
 * Returns: %TRUE if the operation succeeded, %FALSE otherwise.
 */
gboolean
show_uri_externally(const char *uri, GtkWindow *parent, const char *display_name)
{
	g_return_val_if_fail(uri != NULL, FALSE);
	g_return_val_if_fail(parent == NULL || GTK_IS_WINDOW(parent), FALSE);

	GError *error = NULL;

	gboolean success = gtk_show_uri_on_window(parent, uri, GDK_CURRENT_TIME, &error);
	if (!success) {
		error_dialog(parent, error,
			/* TRANSLATORS: %s can be a URL, a filename, or a noun like "the
			Materials folder" */
			_("We couldn't open a program to show %s. The error was: "),
			display_name != NULL ? display_name : uri);
	}
	return success;
}

/**
 * show_file_in_browser:
 * @file: a #GFile to display.
 * @parent: a parent window for a possible error dialog, or %NULL.
 *
 */
gboolean
show_file_in_browser(GFile *file, GtkWindow *parent)
{
	g_return_val_if_fail(G_IS_FILE(file), FALSE);
	g_return_val_if_fail(parent == NULL || GTK_IS_WINDOW(parent), FALSE);

	GError *error = NULL;
	char *uri = g_file_get_uri(file);
	gboolean success = gtk_show_uri_on_window(parent, uri, GDK_CURRENT_TIME, &error);
	if(!success) {
		char *display_name = file_get_display_name(file);
		error_dialog(parent, error,
			_("We couldn't show %s in your browser. The error was: "),
			display_name);
		g_free(display_name);
	}
	g_free(uri);
	return success;
}
