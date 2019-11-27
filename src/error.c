/* Copyright (C) 2006-2009, 2010, 2011, 2012, 2015 P. F. Chimento
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

#include <stdarg.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "error.h"
#include "file.h"

/* Create and display an error dialog box, with parent window parent, and
message format string msg. If err is not NULL, tack the error message on to the
end of the format string. */
void
error_dialog(GtkWindow *parent, GError *err, const gchar *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	gchar buffer[1024];
	g_vsnprintf(buffer, 1024, msg, ap);
	va_end(ap);

	gchar *message;
	if(err) {
		message = g_strconcat(buffer, err->message, NULL);
		g_error_free(err);
	} else
		message = g_strdup(buffer);

	GtkWidget *dialog = gtk_message_dialog_new(parent,
		  parent? GTK_DIALOG_DESTROY_WITH_PARENT : 0,
		  GTK_MESSAGE_ERROR,
		  GTK_BUTTONS_OK,
		  "%s", message);

	/* WTF doesn't gtk_dialog_run() do this anymore? */
	gtk_widget_show(dialog);
	gtk_window_present(GTK_WINDOW(dialog));

	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	g_free(message);
}

/**
 * extended_error_dialog:
 * @parent: (allow-none): The parent window to make this dialog transient for.
 * @what_failed: a message describing what failed.
 * @why_failed: a message describing why the failure occurred.
 * @suggestions: a message suggesting actions the user can take to remedy the
 * failure.
 *
 * Displays an error dialog with extra information. Based on code from
 * Conglomerate. The messages can use Pango markup.
 */
void
extended_error_dialog(GtkWindow *parent, const char *what_failed, const char *why_failed, const char *suggestions)
{
	GtkWidget *dialog, *vbox, *content_hbox, *action_area, *ok, *image, *label;
	char *message;

	g_return_if_fail(what_failed);
	g_return_if_fail(why_failed);
	g_return_if_fail(suggestions);

	dialog = g_object_new(GTK_TYPE_DIALOG,
		"border-width", 6,
		"resizable", FALSE,
		"has-separator", FALSE,
		"transient-for", parent,
		NULL);

	vbox = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 2);

	content_hbox = gtk_hbox_new(FALSE, 12);
	gtk_container_set_border_width(GTK_CONTAINER(content_hbox), 6);

	image = gtk_image_new_from_icon_name("dialog-error", GTK_ICON_SIZE_DIALOG);
	gtk_box_pack_start(GTK_BOX(content_hbox), image, FALSE, TRUE, 0);
	gtk_misc_set_alignment(GTK_MISC(image), 0.5, 0);

	message = g_strdup_printf("<span weight=\"bold\" size=\"larger\">%s</span>\n\n%s\n\n%s", what_failed, why_failed, suggestions);
	label = g_object_new(GTK_TYPE_LABEL,
		"label", message,
		"use-markup", TRUE,
		"justify", GTK_JUSTIFY_LEFT,
		"wrap", TRUE,
		"xalign", 0.5,
		"yalign", 0.0,
		NULL);
	g_free(message);

	gtk_box_pack_start(GTK_BOX(content_hbox), label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), content_hbox, TRUE, TRUE, 0);

	action_area = gtk_dialog_get_action_area(GTK_DIALOG(dialog));
	gtk_container_set_border_width(GTK_CONTAINER(action_area), 5);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(action_area), GTK_BUTTONBOX_END);
	gtk_box_set_spacing(GTK_BOX(action_area), 10);

	ok = gtk_button_new_with_label(_("_OK"));
	gtk_dialog_add_action_widget(GTK_DIALOG(dialog), ok, GTK_RESPONSE_OK);
	gtk_widget_set_can_default(ok, TRUE);

	gtk_widget_show_all(dialog);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

/*
 * what_failed_on_file_operation_failure:
 * @file: the file reference that failed.
 * @transient: %TRUE if this failure is likely to be permanent, i.e. a malformed
 * path. %FALSE if the action might succeed on subsequent attempts.
 * @what: indicates whether it was an open, save, or other operation that
 * failed.
 *
 * Helper function that picks a suitable error message describing what failed.
 * Based on code from Conglomerate.
 *
 * Returns: (transfer full): a newly-allocated string.
 */
static char *
what_failed_on_file_operation_failure(GFile *file, gboolean transient, I7FileErrorWhat what)
{
	char *displayname, *what_failed, *path;
	GFile *parent;

	g_return_val_if_fail(file || G_IS_FILE(file), NULL);

	displayname = file_get_display_name(file);

	parent = g_file_get_parent(file);
	path = g_file_get_path(parent);
	g_object_unref(parent);

	if(what == I7_FILE_ERROR_OTHER) {
		/* Generic file error */
		what_failed = g_strdup_printf(_("Inform got an error while accessing \"%s\" from %s."), displayname, path);
	} else if(transient) {
		/* A "what failed" message when the failure is likely to be
		permanent; this URI won't be accessible */
		if(what == I7_FILE_ERROR_OPEN)
			what_failed = g_strdup_printf(_("Inform cannot read \"%s\" from %s."), displayname, path);
		else
			what_failed = g_strdup_printf(_("Inform cannot save \"%s\" to %s."), displayname, path);
	} else {
		/* A "what failed" message when the failure is likely to be
		transient; this URI might be accessible on subsequent attempts, or
		with some troubleshooting. */
		if(what == I7_FILE_ERROR_OPEN)
			what_failed = g_strdup_printf(_("Inform could not read \"%s\" from %s."), displayname, path);
		else
			what_failed = g_strdup_printf(_("Inform could not save \"%s\" to %s."), displayname, path);
	}

	g_free(path);
	g_free(displayname);

	return what_failed;
}

/*
 * format_file_size_for_display:
 * @size: a #gsize.
 *
 * Formats the file @size passed so that it is easy for
 * the user to read. Gives the size in bytes, kilobytes, megabytes, or
 * gigabytes, choosing whatever is appropriate. Based on code from Conglomerate.
 *
 * Returns: a newly allocated string with the size ready to be shown.
 */
#define KILOBYTE_FACTOR (1024.0)
#define MEGABYTE_FACTOR (1024.0 * KILOBYTE_FACTOR)
#define GIGABYTE_FACTOR (1024.0 * MEGABYTE_FACTOR)
static char *
format_file_size_for_display(goffset size)
{
	if(size < (goffset)KILOBYTE_FACTOR)
		return g_strdup_printf("%" G_GOFFSET_FORMAT " %s", size, ngettext("byte", "bytes", size));

	double displayed_size;

	if((double)size < MEGABYTE_FACTOR) {
		displayed_size = (double)size / KILOBYTE_FACTOR;
		return g_strdup_printf(_("%.1f KB"), displayed_size);
	} else if((double)size < GIGABYTE_FACTOR) {
		displayed_size = (double)size / MEGABYTE_FACTOR;
		return g_strdup_printf(_("%.1f MB"), displayed_size);
	}
	displayed_size = (double)size / GIGABYTE_FACTOR;
	return g_strdup_printf(_("%.1f GB"), displayed_size);
}

/**
 * error_dialog_file_operation:
 * @parent: (allow-none): the parent window to make the dialog transient for.
 * @file: the file reference of the file you tried to open.
 * @error: the #GError that resulted from the failed file operation. For
 * convenience, this function frees the error.
 * @what: indicates whether this was a save, open, or other kind of operation.
 * @message: (allow-none): if @what is %I7_FILE_ERROR_OTHER, then this format
 * string describes what the file operation attempted to do.
 * @...: format parameters for @message.
 *
 * Shows a slightly more informative error dialog that displays information
 * about a file operation that failed, extracting information from @error. Also
 * frees @error for convenience.
 *
 * Based on code from Conglomerate.
 */
void
error_dialog_file_operation(GtkWindow *parent, GFile *file, GError *error, I7FileErrorWhat what, const char *message, ...)
{
	char *what_failed, *why_failed, *path;
	GFile *parent_file;
	GFileInfo *info;

	g_return_if_fail(file);
	g_return_if_fail(error);

	if(what == I7_FILE_ERROR_OTHER) {
		char *formatted_message;
		va_list ap;
		va_start(ap, message);
		formatted_message = g_strdup_vprintf(message, ap);
		va_end(ap);

		what_failed = what_failed_on_file_operation_failure(file, TRUE, what);
		why_failed = g_strdup_printf(_("The error <b>\"%s\"</b> occurred during the "
			"following operation: %s."), error->message, formatted_message);
		extended_error_dialog(parent, what_failed, why_failed,
		    _("Try again. If it fails again, file a bug report on the Inform "
			"website."));
		g_free(what_failed);
		g_free(why_failed);
		g_free(formatted_message);
		return;
	}

	/* Get at the parent file reference in case it's needed: */
	parent_file = g_file_get_parent(file);

	switch(error->code) {
	case G_IO_ERROR_NOT_FOUND:
		what_failed = what_failed_on_file_operation_failure(file, TRUE, what);
		/* Either "file not found" (if opening only) or "path not found".
		Does the parent exist? */
		if(what == I7_FILE_ERROR_OPEN && g_file_query_exists(parent_file, NULL)) {
			/* OK; the path exists, but the file doesn't: */
			extended_error_dialog(parent, what_failed,
				_("There is no file with that name at that location."),
				_("Try checking that you spelled the file's name "
				"correctly. Remember that capitalization is significant "
				"(\"MyFile\" is not the same as \"MYFILE\" or \"myfile\")."));
		} else {
			/* The path doesn't exist: */
			extended_error_dialog(parent, what_failed,
				_("The location does not exist."),
				_("Try checking that you spelled the location correctly. "
				"Remember that capitalization is significant (\"MyDirectory\" "
				"is not the same as \"mydirectory\" or \"MYDIRECTORY\")."));
		}
		break;

	case G_IO_ERROR_NOT_SUPPORTED:
		what_failed = what_failed_on_file_operation_failure(file, FALSE, what);
		path = g_file_get_path(file);

		if(what == I7_FILE_ERROR_OPEN) {
			why_failed = g_strdup_printf(_("The location \"%s\" does not "
				"support the reading of files."), path);
			extended_error_dialog(parent, what_failed, why_failed,
				_("Try loading a file from a different location. If you think that "
				"you ought to be able to read this file, contact your system "
				"administrator."));
		} else {
			why_failed = g_strdup_printf(_("The location \"%s\" does not "
				"support the writing of files."), path);
			extended_error_dialog(parent, what_failed, why_failed,
				_("Try saving to a different location."));
		}

		g_free(path);
		g_free(why_failed);
		break;

	case G_IO_ERROR_NO_SPACE:
		what_failed = what_failed_on_file_operation_failure(file, FALSE, what);
		why_failed = NULL;

		info = g_file_query_info(file, G_FILE_ATTRIBUTE_STANDARD_SIZE, G_FILE_QUERY_INFO_NONE, NULL, NULL);
		if(info) {
			goffset file_size = g_file_info_get_size(info);
			char *file_size_string = format_file_size_for_display(file_size);
			g_object_unref(info);

			/* We call the "get space" function on the parent URI rather than
			the file URI since this function fails if the URI does not exist (it
			decides it's not a local URI as it can't stat the file) */
			GFileInfo *parent_info = g_file_query_info(parent_file, G_FILE_ATTRIBUTE_FILESYSTEM_FREE, G_FILE_QUERY_INFO_NONE, NULL, NULL);

			if(parent_info) {
				guint64 free_space = g_file_info_get_attribute_uint64(parent_info, G_FILE_ATTRIBUTE_FILESYSTEM_FREE);
				char *free_space_string = format_file_size_for_display(free_space);
				why_failed = g_strdup_printf(_("The size of the file would be "
					"%s, but you only have %s free on that device."),
				    file_size_string, free_space_string);
				g_free(free_space_string);
				g_object_unref(parent_info);
			} else {
				/* Can't get at the free space for the device or "volume": */
				why_failed = g_strdup_printf(_("The file is too big to fit in "
					"the remaining space on the device (file size would be %s)."),
				    file_size_string);
			}

			g_free(file_size_string);

		} else {
			/* We don't know the size of the file: */
			why_failed = g_strdup_printf(_("The file is too big to fit in the "
			"remaining space on the device."));
		}

		g_assert(why_failed);

		extended_error_dialog(parent, what_failed, why_failed,
			_("Try saving the file to a different location, or making more "
			"space by moving unwanted files from that device to the trash."));

		g_free(why_failed);
		break;

	case G_IO_ERROR_READ_ONLY:
		what_failed = what_failed_on_file_operation_failure(file, FALSE, what);
		path = g_file_get_path(file);
		why_failed = g_strdup_printf(_("The location \"%s\" does not "
			"support the writing of files."), path);
		g_free(path);
		extended_error_dialog(parent, what_failed, why_failed,
			_("Try saving to a different location."));
		g_free(why_failed);
		break;

	case G_IO_ERROR_INVALID_FILENAME:
		what_failed = what_failed_on_file_operation_failure(file, FALSE, what);
		extended_error_dialog(parent, what_failed,
		    _("The system does not recognize that as a valid location."),
		    _("Try checking that you spelled the location correctly. "
			"Remember that capitalization is significant (\"http\" is not the "
			"same as \"Http\" or \"HTTP\")."));
		break;

	case G_IO_ERROR_PERMISSION_DENIED:
		what_failed = what_failed_on_file_operation_failure(file, FALSE, what);
		if(what == I7_FILE_ERROR_OPEN) {
			extended_error_dialog(parent, what_failed,
				_("You do not have permission to read that file."),
				_("Try asking your system administrator to give you permission."));
		} else {
			extended_error_dialog(parent, what_failed,
				_("You do not have permission to write to that location."),
				_("Try saving to a different location, or ask your system "
				"administrator to give you permission."));
		}
		break;

	case G_IO_ERROR_TOO_MANY_OPEN_FILES:
		what_failed = what_failed_on_file_operation_failure(file, TRUE, what);
		extended_error_dialog(parent, what_failed,
		    _("The system is trying to operate on too many files at once."),
			_("Try again. If it fails again, try closing unneeded applications,"
			" or contact your system administrator."));
		break;

	case G_IO_ERROR_TIMED_OUT:
		what_failed = what_failed_on_file_operation_failure(file, TRUE, what);
		extended_error_dialog(parent, what_failed,
		    _("There were problems reading the contents of the file."),
			_("Try again. If it fails again, contact your system administrator."));
		break;

	case G_IO_ERROR_IS_DIRECTORY:
		what_failed = what_failed_on_file_operation_failure(file, FALSE, what);
		extended_error_dialog(parent, what_failed,
			_("It is a directory, rather than a file."),
			what == I7_FILE_ERROR_OPEN?
				_("Try using the Desktop Search to find your file.")
				: _("You must save to a file within a directory, rather than to"
				" the directory itself."));
		break;

	case G_IO_ERROR_HOST_NOT_FOUND:
		what_failed = what_failed_on_file_operation_failure(file, FALSE, what);
		extended_error_dialog(parent, what_failed,
			_("The server could not be contacted."),
			_("Try again. If it fails again, the server may be down."));
		break;

	case G_IO_ERROR_CONNECTION_REFUSED:
		what_failed = what_failed_on_file_operation_failure(file, TRUE, what);
		extended_error_dialog(parent, what_failed,
			_("The system could not login to the location."),
			_("Try again. If it fails again, contact your system administrator."));
		break;

	case G_IO_ERROR_BUSY:
		what_failed = what_failed_on_file_operation_failure(file, TRUE, what);
		extended_error_dialog(parent, what_failed,
			_("The location was too busy."),
			_("Try again. If it fails again, contact your system administrator."));
		break;

	case G_IO_ERROR_FILENAME_TOO_LONG:
		what_failed = what_failed_on_file_operation_failure(file, FALSE, what);
		extended_error_dialog(parent, what_failed,
		    _("The name is too long for the location to manage."),
			_("Try again with a shorter file name."));
		break;

	case G_IO_ERROR_FAILED_HANDLED:
		/* This means a helper program has already interacted with the user and
		we shouldn't display an error dialog. */
		what_failed = NULL;
		break;

	default:
		/* Unknown (or inapplicable) error */
		what_failed = what_failed_on_file_operation_failure(file, TRUE, what);
		extended_error_dialog(parent, what_failed,
		    _("An unexpected internal error occurred."),
		    _("Try again. If it fails again, file a bug report on the Inform "
			"website."));
		break;
	}

	g_free(what_failed);
	g_object_unref(parent_file);
	g_error_free(error);
}
