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

#include <sys/types.h>
#include <pwd.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include "file.h"
#include "configfile.h"
#include "error.h"
#include "story.h"

/* HELPER FUNCTIONS */

/* From gnome-vfs-utils.c */
gchar *
expand_initial_tilde(const gchar *path)
{
	char *slash_after_user_name, *user_name;
	struct passwd *passwd_file_entry;

	g_return_val_if_fail(path != NULL, NULL);

	if(path[0] != '~')
		return g_strdup(path);

	if(path[1] == '/' || path[1] == '\0')
		return g_strconcat(g_get_home_dir(), &path[1], NULL);

	slash_after_user_name = strchr(&path[1], '/');
	if(slash_after_user_name == NULL)
		user_name = g_strdup(&path[1]);
	else
		user_name = g_strndup(&path[1], slash_after_user_name - &path[1]);

	passwd_file_entry = getpwnam(user_name);
	g_free(user_name);

	if(passwd_file_entry == NULL || passwd_file_entry->pw_dir == NULL)
		return g_strdup(path);

	return g_strconcat(passwd_file_entry->pw_dir, slash_after_user_name, NULL);
}

/* Read a source file into a string. Allocates a new string */
gchar *
read_source_file(const gchar *filename)
{
	GError *error = NULL;
	gchar *text;

	gsize num_bytes;
	if(!g_file_get_contents(filename, &text, &num_bytes, &error)) {
		error_dialog(NULL, error, _("Could not open the file '%s'.\n\n"
			"Make sure that this file has not been deleted or renamed."), filename);
		return NULL;
	}

	/* Make sure the file wasn't binary or something */
	if(!g_utf8_validate(text, num_bytes, NULL)) {
		error_dialog(NULL, NULL,
		_("The file '%s' could not be read because it contained invalid UTF-8 text."), filename);
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

gchar *
get_filename_from_save_dialog(const gchar *default_filename)
{
	/* Create a file chooser */
	GtkWidget *dialog = gtk_file_chooser_dialog_new(_("Save File"), NULL, GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
		NULL);

	if(default_filename) {
		gchar *path = g_path_get_dirname(default_filename);
		gchar *file = g_path_get_basename(default_filename);
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), path);
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), file);
		g_free(path);
		g_free(file);
	} else {
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), _("Untitled document"));
	}

	GtkFileFilter *filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filter, "*.inform");
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog), filter);

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		gchar *path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		gchar *filename = g_str_has_suffix(path, ".inform")? g_strdup(path) : g_strconcat(path, ".inform", NULL);
		g_free(path);
		gtk_widget_destroy(dialog);

		/* For "Select folder" mode, we must do our own confirmation */
		/* Adapted from gtkfilechooserdefault.c */
		/* Sourcefile is a workaround: if you type a new folder into the file
		selection dialog, GTK will create that folder automatically and it
		will then already exist */
		gchar *sourcefile = g_build_filename(filename, "Source", NULL);
		if(g_file_test(filename, G_FILE_TEST_EXISTS)
			&& g_file_test(sourcefile, G_FILE_TEST_EXISTS))
		{
			dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
				_("A project named \"%s\" already exists. Do you want to replace it?"), filename);
			gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
				_("Replacing it will overwrite its contents."));
			gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
			GtkWidget *button = gtk_button_new_with_mnemonic(_("_Replace"));
			GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
			gtk_button_set_image(GTK_BUTTON(button), gtk_image_new_from_stock(GTK_STOCK_SAVE_AS, GTK_ICON_SIZE_BUTTON));
			gtk_widget_show(button);
			gtk_dialog_add_action_widget(GTK_DIALOG(dialog), button, GTK_RESPONSE_ACCEPT);
			gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog),
				GTK_RESPONSE_ACCEPT, GTK_RESPONSE_CANCEL,
				-1);
			gtk_dialog_set_default_response (GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);

			int response = gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
			if(response != GTK_RESPONSE_ACCEPT) {
				g_free(filename);
				g_free(sourcefile);
				return get_filename_from_save_dialog(default_filename);
			}
		}
		g_free(sourcefile);
		return filename;
	} else {
		gtk_widget_destroy(dialog);
		return NULL;
	}
}

/* Helper function to delete a file relative to the project path; does nothing
if file does not exist */
static void
delete_from_project_dir(I7Story *story, gchar *storyname, gchar *subdir, gchar *filename)
{
	gchar *pathname;

	if(subdir)
		pathname = g_build_filename(storyname, subdir, filename, NULL);
	else
		pathname = g_build_filename(storyname, filename, NULL);

	g_remove(pathname);
	g_free(pathname);
	i7_document_display_progress_busy(I7_DOCUMENT(story));
}

/* If the "delete build files" option is checked, delete all the build files
from the project directory */
void
delete_build_files(I7Story *story)
{
	if(config_file_get_bool(PREFS_CLEAN_BUILD_FILES)) {
		i7_document_display_status_message(I7_DOCUMENT(story), _("Cleaning out build files..."), FILE_OPERATIONS);

		gchar *storyname = i7_document_get_path(I7_DOCUMENT(story));

		delete_from_project_dir(story, storyname, NULL, "Metadata.iFiction");
		delete_from_project_dir(story, storyname, NULL, "Release.blurb");
		delete_from_project_dir(story, storyname, "Build", "auto.inf");
		delete_from_project_dir(story, storyname, "Build", "Debug log.txt");
		delete_from_project_dir(story, storyname, "Build", "Map.eps");
		delete_from_project_dir(story, storyname, "Build", "output.z5");
		delete_from_project_dir(story, storyname, "Build", "output.z6");
		delete_from_project_dir(story, storyname, "Build", "output.z8");
		delete_from_project_dir(story, storyname, "Build", "output.ulx");
		delete_from_project_dir(story, storyname, "Build", "Problems.html");
		delete_from_project_dir(story, storyname, "Build", "gameinfo.dbg");
		delete_from_project_dir(story, storyname, "Build", "temporary file.inf");
		delete_from_project_dir(story, storyname, "Build", "temporary file 2.inf");
		delete_from_project_dir(story, storyname, "Build", "StatusCblorb.html");

		if(config_file_get_bool(PREFS_CLEAN_INDEX_FILES)) {
			delete_from_project_dir(story, storyname, "Index", "Actions.html");
			delete_from_project_dir(story, storyname, "Index", "Contents.html");
			delete_from_project_dir(story, storyname, "Index", "Headings.xml");
			delete_from_project_dir(story, storyname, "Index", "Kinds.html");
			delete_from_project_dir(story, storyname, "Index", "Phrasebook.html");
			delete_from_project_dir(story, storyname, "Index", "Rules.html");
			delete_from_project_dir(story, storyname, "Index", "Scenes.html");
			delete_from_project_dir(story, storyname, "Index", "World.html");
			/* Delete the "Details" subdirectory */
			gchar *details_dir = g_build_filename(storyname, "Index", "Details", NULL);
			GDir *details = g_dir_open(details_dir, 0, NULL);
			if(details) {
				const gchar *dir_entry;
				while((dir_entry = g_dir_read_name(details)) != NULL) {
					gchar *filename = g_build_filename(storyname, "Index", "Details", dir_entry, NULL);
					g_remove(filename);
					i7_document_display_progress_busy(I7_DOCUMENT(story));
				}
				g_dir_close(details);
				g_remove(details_dir);
			}
			g_free(details_dir);
		}

		g_free(storyname);
	}
	i7_document_remove_status_message(I7_DOCUMENT(story), FILE_OPERATIONS);
	i7_document_display_progress_percentage(I7_DOCUMENT(story), 0.0);
}

/* Helper function: return the first match within directory @d */
static gchar *
get_match_within_directory(GDir *d, const gchar *name)
{
	const gchar *dirp;
	while((dirp = g_dir_read_name(d)) != NULL) {
		if(strcasecmp(name, dirp) == 0)
			return g_strdup(dirp);
	}
	return NULL;
}

/* Helper function: find the on-disk filename of @path, whose last two
 components may have incorrect casing. Adapted from Sec. 2/cifn of Inform
 source, which was written by Adam Thornton. Returns a newly-allocated string
 containing the on-disk filename, or NULL on failure. */
gchar *
get_case_insensitive_extension(const gchar *path)
{
	gchar *cistring, *retval;

	/* For efficiency's sake, though it's logically equivalent, we try... */
	if(g_file_test(path, G_FILE_TEST_EXISTS))
		return g_strdup(path);

	/* Find the length of the path, giving an error if it is empty or NULL */
	size_t length = 0;
	if(path)
		length = strlen(path);
	if(length < 1)
		return NULL;

	/* Parse the path to break it into topdirpath, extension directory and
	 leafname */
	gchar *p = strrchr(path, G_DIR_SEPARATOR);
	size_t extindex = (size_t)(p - path);
	size_t namelen = length - extindex - 1;
	gchar *ciextname = g_strndup(path + extindex + 1, namelen);
	gchar *workstring = g_strndup(path, extindex - 1);

	p = strrchr(workstring, G_DIR_SEPARATOR);
	size_t extdirindex = (size_t)(p - workstring);
	gchar *topdirpath = g_strndup(path, extdirindex);

	size_t dirlen = extindex - extdirindex - 1;
	gchar *ciextdirpath = g_strndup(path + extdirindex + 1, dirlen);

	GDir *topdir = g_dir_open(topdirpath, 0, NULL);
	/* pathname is assumed case-correct */
	if(!topdir)
		goto fail; /* ... so that failure is fatal */

	g_free(workstring);
	workstring = g_build_filename(topdirpath, ciextdirpath, NULL);
	GDir *extdir = g_dir_open(workstring, 0, NULL);
	if(!extdir) {
		/* Try to find a unique insensitively matching directory name in topdir */
		g_free(workstring);
		if((workstring = get_match_within_directory(topdir, ciextdirpath))) {
			cistring = g_build_filename(topdirpath, workstring, NULL);
			extdir = g_dir_open(cistring, 0, NULL);
			if(!extdir)
				goto fail1;
		} else
			goto fail1;
	} else
		cistring = g_strdup(workstring);

	retval = g_build_filename(cistring, ciextname, NULL);
	if(g_file_test(retval, G_FILE_TEST_EXISTS))
		goto success;
	g_free(retval);

	/* Try to find a unique insensitively matching entry in extdir */
	g_free(workstring);
	if((workstring = get_match_within_directory(extdir, ciextname))) {
		retval = g_build_filename(cistring, workstring, NULL);
		if(g_file_test(retval, G_FILE_TEST_EXISTS))
			goto success;
		g_free(retval);
	}

	g_dir_close(extdir);
fail1:
	g_dir_close(topdir);
fail:
	g_free(topdirpath);
	g_free(ciextdirpath);
	g_free(ciextname);
	g_free(workstring);
	return NULL;
success:
	g_dir_close(topdir);
	g_dir_close(extdir);
	g_free(topdirpath);
	g_free(ciextdirpath);
	g_free(ciextname);
	g_free(workstring);
	return retval;
}
