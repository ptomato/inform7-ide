/* Copyright (C) 2006-2009, 2010, 2011, 2012, 2013, 2019 P. F. Chimento
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

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "app.h"
#include "builder.h"
#include "configfile.h"
#include "error.h"
#include "extension.h"
#include "newdialog.h"
#include "story.h"
#include "welcomedialog.h"

typedef enum _I7NewProjectType I7NewProjectType;
enum _I7NewProjectType {
	I7_NEW_PROJECT_NOTHING = 0,
	I7_NEW_PROJECT_INFORM7_STORY,
	I7_NEW_PROJECT_INFORM7_EXTENSION,
	I7_NEW_PROJECT_INFORM6_EMPTY,
	I7_NEW_PROJECT_INFORM6_ONE_ROOM
};

enum {
	COLUMN_TYPE,
	COLUMN_TYPE_NAME,
	COLUMN_DISPLAY_TEXT
};

typedef struct {
	I7NewProjectType type;
	GFile *directory;
	gchar *name;
	gchar *author;

	GtkWidget *assistant;
	GtkWidget *label;
	GtkWidget *author_box;
	GtkWidget *chooser;
} I7NewProjectOptions;

/* Change the description text when the selection changes */
static void
on_project_type_selection_changed(GtkTreeSelection *selection, I7NewProjectOptions *options)
{
	GtkTreeIter iter;
	GtkTreeModel *model;

	GtkWidget *page = gtk_assistant_get_nth_page(GTK_ASSISTANT(options->assistant), 0);

	if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
		gchar *description;
		I7NewProjectType type;
		gtk_tree_model_get(model, &iter,
			COLUMN_TYPE, &type,
			COLUMN_DISPLAY_TEXT, &description,
			-1);
		gtk_label_set_text(GTK_LABEL(options->label), description);
		gtk_assistant_set_page_complete(GTK_ASSISTANT(options->assistant), page, TRUE);
		g_free(description);
		options->type = type;
	} else
		gtk_assistant_set_page_complete(GTK_ASSISTANT(options->assistant), page, FALSE);
}

static void
new_project_options_free(I7NewProjectOptions *options)
{
	if(options) {
		if(options->directory)
			g_object_unref(options->directory);
		if(options->name)
			g_free(options->name);
		if(options->author)
			g_free(options->author);
		g_slice_free(I7NewProjectOptions, options);
	}
}

void
on_newdialog_cancel(GtkAssistant *assistant, I7NewProjectOptions *options)
{
	gtk_widget_destroy(GTK_WIDGET(assistant));
	new_project_options_free(options);
	/* If we aren't editing a story, go back to the welcome dialog */
	GtkApplication *theapp = GTK_APPLICATION(g_application_get_default());
	if (gtk_application_get_windows(theapp) == NULL) {
		GtkWidget *welcome_dialog = create_welcome_dialog(theapp);
		gtk_widget_show(welcome_dialog);
	}
}

gboolean
on_newdialog_delete_event(GtkWidget *widget, GdkEvent *event, I7NewProjectOptions *options)
{
	on_newdialog_cancel(GTK_ASSISTANT(widget), options);
	return TRUE;
}

/* Enable the OK button if data in all fields are filled in */
static void
check_page_finished(I7NewProjectOptions *options)
{
	GtkWidget *page = gtk_assistant_get_nth_page(GTK_ASSISTANT(options->assistant), 1);
	if(options->directory
		&& options->name && strlen(options->name)
		&& options->author && strlen(options->author))
	{
		char *filename = g_strconcat(options->name, ".inform", NULL);
		GFile *file = g_file_get_child(options->directory, filename);
		g_free(filename);

		if(!g_file_query_exists(file, NULL)) {
			gtk_assistant_set_page_complete(GTK_ASSISTANT(options->assistant), page, TRUE);
			g_object_unref(file);
			return;
		}

		g_object_unref(file);
		char *dirpath = g_file_get_path(options->directory);
		error_dialog(NULL, NULL, _("The story \"%s\" already exists in the "
			"directory %s. Please choose another title or directory."),
			options->name, dirpath);
		g_free(dirpath);
	}
	gtk_assistant_set_page_complete(GTK_ASSISTANT(options->assistant), page, FALSE);
}

void
on_new_directory_selection_changed(GtkFileChooser *chooser, I7NewProjectOptions *options)
{
	if(options->directory)
		g_object_unref(options->directory);
	options->directory = gtk_file_chooser_get_file(chooser);
	check_page_finished(options);
}

/* Get the project name from the text entry and format it */
void
on_new_name_changed(GtkEditable *editable, I7NewProjectOptions *options)
{
	if(options->name)
		g_free(options->name);
	options->name = g_strstrip(gtk_editable_get_chars(editable, 0, -1));
	/* Check that the project name is valid */
	const char *invalid = "\\/:*?\"<>|";
	if(strpbrk(options->name, invalid)) {
		error_dialog(GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(editable))),
			NULL, _("The project name cannot contain any of the following: %s"),
			invalid);
		g_free(options->name);
		options->name = NULL;
	}
	check_page_finished(options);
}

/* Get the author's name from the text entry and format it */
void
on_new_author_changed(GtkEditable *editable, I7NewProjectOptions *options)
{
	if(options->author)
		g_free(options->author);
	options->author = g_strstrip(gtk_editable_get_chars(editable, 0, -1));
	check_page_finished(options);
}

/* For new Inform projects and extensions; get the author's name from the config
file and put it in the "new_author" text entry automatically */
void
on_newdialog_prepare(GtkAssistant *assistant, GtkWidget *page, I7NewProjectOptions *options)
{
	char *text, *dirpath;
	I7App *theapp = I7_APP(g_application_get_default());
	GSettings *prefs = i7_app_get_prefs(theapp);

	switch(gtk_assistant_get_current_page(assistant)) {
		case 1:
			text = g_strstrip(g_settings_get_string(prefs, "author-name"));
			gtk_entry_set_text(GTK_ENTRY(options->author_box), (text && strlen(text))? text : g_get_real_name());
			if(text)
				g_free(text);
			on_new_directory_selection_changed(GTK_FILE_CHOOSER(options->chooser), options);
			break;
		case 2:
			dirpath = g_file_get_path(options->directory);
			text = g_strdup_printf(_("<big><b>%s</b>\nby %s</big>\n\n"
				"Project Type: %s\nDirectory to create project in: %s\n"),
				options->name, options->author,
				(options->type == I7_NEW_PROJECT_INFORM7_EXTENSION)?
				_("Inform 7 Extension") : _("Inform 7 Story"),
				dirpath);
			g_free(dirpath);
			gtk_label_set_markup(GTK_LABEL(page), text);
			g_free(text);
			gtk_assistant_set_page_complete(assistant, page, TRUE);
			break;
	}
	GdkPixbuf *icon = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(),
		"inform7", 128, 0, NULL); /* just set NULL on failure */
	gtk_assistant_set_page_side_image(assistant, page, icon);
	if(icon)
		g_object_unref(icon);
}

void
on_newdialog_close(GtkAssistant *assistant, I7NewProjectOptions *options)
{
	char *filename;
	GFile *file;
	I7App *theapp = I7_APP(g_application_get_default());
	GSettings *prefs = i7_app_get_prefs(theapp);

	/* Save the author name to the config file */
	g_settings_set_string(prefs, PREFS_AUTHOR_NAME, options->author);

	switch(options->type) {
		case I7_NEW_PROJECT_INFORM7_STORY:
			filename = g_strconcat(options->name, ".inform", NULL);
			file = g_file_get_child(options->directory, filename);
			i7_story_new(theapp, file, options->name, options->author);
			break;
		case I7_NEW_PROJECT_INFORM7_EXTENSION:
			filename = g_strconcat(options->name, ".i7x", NULL);
			file = g_file_get_child(options->directory, filename);
			i7_extension_new(theapp, file, options->name, options->author);
			break;
		default:
			on_newdialog_cancel(assistant, options);
			return;
	}
	g_free(filename);
	g_object_unref(file);

	new_project_options_free(options);
	gtk_widget_destroy(GTK_WIDGET(assistant));

	g_application_release(g_application_get_default());
}

static gboolean
project_type_selection_function(GtkTreeSelection *selection, GtkTreeModel *model, GtkTreePath *path, gboolean path_currently_selected, gpointer data)
{
	/* Can't select any of the top-level categories */
	if(gtk_tree_path_get_depth(path) == 1)
		return path_currently_selected;
	return TRUE;
}

GtkWidget *
create_new_dialog(void)
{
	/* Create data object to be passed to the callbacks */
	I7NewProjectOptions *options = g_slice_new0(I7NewProjectOptions);
	options->type = I7_NEW_PROJECT_INFORM7_STORY;

	g_autoptr(GtkBuilder) builder = gtk_builder_new_from_resource("/com/inform7/IDE/ui/newdialog.ui");
	gtk_builder_connect_signals(builder, options);
	options->assistant = GTK_WIDGET(load_object(builder, "newdialog"));
	options->label = GTK_WIDGET(load_object(builder, "project_type_description"));
	options->author_box = GTK_WIDGET(load_object(builder, "new_author"));
	options->chooser = GTK_WIDGET(load_object(builder, "new_directory"));

	/* Create a tree store with one column, with a string in it, and have two
	columns of "hidden" data: an integer index and a description string */

	GtkTreeStore *store = GTK_TREE_STORE(load_object(builder, "project_type_model"));
	GtkTreeIter iter_parent, iter_child;
	gtk_tree_store_append(store, &iter_parent, NULL); /* Parent iterator */
	gtk_tree_store_set(store, &iter_parent,
		COLUMN_TYPE, I7_NEW_PROJECT_NOTHING,
		COLUMN_TYPE_NAME, _("<b>Inform 7</b>"),
		COLUMN_DISPLAY_TEXT, _("The Inform 7 application is a natural-language "
		"based system for creating and editing works of interactive fiction. "
		"Choose one of the options below to get started."),
		-1);
	gtk_tree_store_append(store, &iter_child, &iter_parent); /*Child iterator*/
	gtk_tree_store_set(store, &iter_child,
		COLUMN_TYPE, I7_NEW_PROJECT_INFORM7_STORY,
		COLUMN_TYPE_NAME, _("New story"),
		COLUMN_DISPLAY_TEXT, _("Creates a blank Inform 7 project, in which you "
		"can write a work of interactive fiction."),
		-1);
	gtk_tree_store_append(store, &iter_child, &iter_parent);
	gtk_tree_store_set(store, &iter_child,
		COLUMN_TYPE, I7_NEW_PROJECT_INFORM7_EXTENSION,
		COLUMN_TYPE_NAME, _("New set of extension rules"),
		COLUMN_DISPLAY_TEXT, _("For experienced users of Inform 7. An extension"
		" is a set of rules which can be used in more than one work of "
		"interactive fiction, either for your own use or to be contributed to "
		"the community."),
		-1);

	/* Glade is rubbish at creating GtkTreeSelection */
	GtkTreeView *tree = GTK_TREE_VIEW(load_object(builder, "project_type"));
	GtkTreeSelection *select = gtk_tree_view_get_selection(tree);
	gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);
	gtk_tree_selection_set_select_function(select, project_type_selection_function, NULL, NULL);
	g_signal_connect(select, "changed", G_CALLBACK(on_project_type_selection_changed), options);

	/* Select "Inform 7 project" by default. */
	GtkTreePath *default_path = gtk_tree_path_new_from_indices(0, 0, -1);
	gtk_tree_view_expand_to_path(tree, default_path);
	gtk_tree_selection_select_path(select, default_path);
	gtk_tree_path_free(default_path);

	/* This is for all intents and purposes an application window */
	g_application_hold(g_application_get_default());

	return options->assistant;
}
