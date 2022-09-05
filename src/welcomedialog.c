/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2006-2010, 2012, 2014, 2019 Philip Chimento <philip.chimento@gmail.com>
 */

#include "config.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "app.h"
#include "builder.h"
#include "error.h"
#include "newdialog.h"
#include "story.h"
#include "welcomedialog.h"

void
on_welcome_new_button_clicked(GtkButton *button, I7App *app)
{
	GtkWidget *welcomedialog = gtk_widget_get_toplevel(GTK_WIDGET(button));
	GtkWidget *newdialog = create_new_dialog(I7_NEW_DIALOG_CHOOSE_TYPE);
	gtk_widget_destroy(welcomedialog);
	gtk_widget_show(newdialog);
}

void
on_welcome_open_button_clicked(GtkButton *button, I7App *app)
{
	GtkWidget *welcomedialog = gtk_widget_get_toplevel(GTK_WIDGET(button));
	gtk_widget_hide(welcomedialog);

	I7Story *story = i7_story_new_from_dialog(app);

	if(!story) {
		/* Take us back to the welcome dialog */
		gtk_widget_show(welcomedialog);
		return;
	}

	gtk_widget_destroy(welcomedialog);
}

void
on_welcome_reopen_button_clicked(GtkButton *button, I7App *app)
{
	GtkWidget *welcomedialog = gtk_widget_get_toplevel(GTK_WIDGET(button));
	GFile *file = i7_app_get_last_opened_project(app);
	g_assert(file); /* Button not sensitive if no last project */

	I7Story *story = i7_story_new_from_file(app, file);
	g_object_unref(file);

	if (story)
		gtk_widget_destroy(welcomedialog);
}

void
on_last_project_query_done(GFile *last_project, GAsyncResult *result, GtkWidget *reopen_button)
{
	/* ignore errors; just keep the button inactive */
	g_autoptr(GFileInfo) info = g_file_query_info_finish(last_project, result, NULL);
	g_object_unref(last_project);
	if (!info)
		return;
	gtk_widget_set_sensitive(reopen_button, TRUE);
}

GtkWidget *
create_welcome_dialog(GtkApplication *theapp)
{
	g_autoptr(GtkBuilder) builder = gtk_builder_new_from_resource("/com/inform7/IDE/ui/welcomedialog.ui");
	gtk_builder_connect_signals(builder, theapp);
	GtkWidget *retval = GTK_WIDGET(load_object(builder, "welcomedialog"));
	gtk_window_set_application(GTK_WINDOW(retval), theapp);

	/* If there is no "last project", make the reopen button inactive */
	GFile *last_project = i7_app_get_last_opened_project(I7_APP(theapp));
	if (last_project) {
		GtkWidget *reopen_button = GTK_WIDGET(load_object(builder, "welcome_reopen_button"));
		g_file_query_info_async(g_steal_pointer(&last_project), G_FILE_ATTRIBUTE_STANDARD_TYPE,
			G_FILE_QUERY_INFO_NONE, G_PRIORITY_DEFAULT, NULL,
			(GAsyncReadyCallback)on_last_project_query_done, reopen_button);
	}

	return retval;
}
