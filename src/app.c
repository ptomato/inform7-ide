/*  Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012 P. F. Chimento
 *  This file is part of GNOME Inform 7.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <errno.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include "app.h"
#include "app-private.h"
#include "actions.h"
#include "builder.h"
#include "colorscheme.h"
#include "configfile.h"
#include "error.h"
#include "file.h"
#include "lang.h"
#include "prefs.h"

#define EXTENSIONS_BASE_PATH "Inform", "Extensions"

/* The singleton application class; should be derived from GtkApplication when
 porting to GTK 3. Contains the following global miscellaneous stuff:
 - an action group containing actions that would be valid even if there were no
   document open (even though there is no menu at present when no document is
   open.)
 - the list of open documents.
 - information about the paths to project data and executable files.
 - the file monitor for the extension directory.
 - the tree of installed extensions.
 - the print settings and page setup objects.
 - the preferences dialog.
 - various compiled regices for use elsewhere in the program.
*/

G_DEFINE_TYPE(I7App, i7_app, G_TYPE_OBJECT);

typedef struct {
	gchar *regex;
	gboolean caseless;
} I7AppRegexInfo;

static void
i7_app_init(I7App *self)
{
	I7_APP_USE_PRIVATE(self, priv);
	GError *error = NULL;

	/* Retrieve data directories if set externally */
	const gchar *env = g_getenv("GNOME_INFORM_DATA_DIR");
	if(env) {
		priv->datadir = g_file_new_for_path(env);
	} else {
		char *path = g_build_filename(PACKAGE_DATA_DIR, "gnome-inform7", NULL);
		priv->datadir = g_file_new_for_path(path);
		g_free(path);
	}

	env = g_getenv("GNOME_INFORM_LIBEXEC_DIR");
	priv->libexecdir = g_file_new_for_path(env? env : PACKAGE_LIBEXEC_DIR);

	GFile *builderfile = i7_app_get_data_file(self, "ui/gnome-inform7.ui");
	GtkBuilder *builder = create_new_builder(builderfile, self);
	g_object_unref(builderfile);

	/* Make the action group and ref it so that it won't be owned by whatever
	UI manager it's inserted into */
	priv->app_action_group = GTK_ACTION_GROUP(load_object(builder, "app_actions"));
	g_object_ref(priv->app_action_group);

	/* Add a filter to the Open Recent menu (can be removed once Glade supports
	building GtkRecentFilters) */
	GtkAction *recent = GTK_ACTION(load_object(builder, "open_recent"));
	GtkRecentFilter *filter = gtk_recent_filter_new();
	gtk_recent_filter_add_group(filter, "inform7_project");
	gtk_recent_filter_add_group(filter, "inform7_extension");
	gtk_recent_chooser_set_filter(GTK_RECENT_CHOOSER(recent), filter);

	priv->document_list = NULL;
	priv->installed_extensions = GTK_TREE_STORE(load_object(builder, "installed_extensions_store"));
	g_object_ref(priv->installed_extensions);
	/* Set print settings to NULL, since they are not remembered across
	application runs (yet) */
	priv->print_settings = NULL;
	priv->page_setup = gtk_page_setup_new();

	/* Create the Gnome Inform7 dir if it doesn't already exist */
	GFile *extensions_file = i7_app_get_extension_file(self, NULL, NULL);
	if(!make_directory_unless_exists(extensions_file, NULL, &error)) {
		IO_ERROR_DIALOG(NULL, extensions_file, error, _("creating the Inform directory"));
	}
	g_object_unref(extensions_file);

	/* Set up monitor for extensions directory */
	i7_app_run_census(self, FALSE);
	priv->extension_dir_monitor = NULL;
	i7_app_monitor_extensions_directory(self);

	/* Compile the regices */
	I7AppRegexInfo regex_info[] = {
		{ "^(?P<level>volume|book|part|chapter|section)\\s+(?P<secnum>.*?)(\\s+-\\s+(?P<sectitle>.*))?$", TRUE },
		{ "\\[=0x([0-9A-F]{4})=\\]", FALSE },
		{ "^\\s*(?:version\\s.+\\sof\\s+)?(?:the\\s+)?" /* Version X of [the] */
		  "(?P<title>.+)\\s+(?:\\(for\\s.+\\sonly\\)\\s+)?" /* <title> [(for X only)] */
		  "by\\s+(?P<author>.+)\\s+" /* by <author> */
		  "begins?\\s+here\\.?\\s*$", /* begins here[.] */
		  TRUE },
		{ "R?ex(?P<number>[0-9]+).html$", FALSE }
	};
	int i;
	for(i = 0; i < I7_APP_NUM_REGICES; i++) {
		self->regices[i] = g_regex_new(regex_info[i].regex, G_REGEX_OPTIMIZE | (regex_info[i].caseless? G_REGEX_CASELESS : 0), 0, &error);
		if(!self->regices[i])
			ERROR(_("Could not compile regex"), error);
	}

	self->prefs = create_prefs_window(builder);

	/* Check the application settings and set all the controls to their current
	values according to GConf. If the keys aren't set in GConf, set them to
	their default values. */
	init_config_file(builder);

	g_object_unref(builder);
}

static void
i7_app_finalize(GObject *self)
{
	I7_APP_USE_PRIVATE(self, priv);
	g_object_unref(priv->datadir);
	g_object_unref(priv->libexecdir);
	i7_app_stop_monitoring_extensions_directory(I7_APP(self));
	if(I7_APP(self)->prefs)
		g_slice_free(I7PrefsWidgets, I7_APP(self)->prefs);
	g_object_unref(priv->installed_extensions);
	g_object_unref(priv->app_action_group);

	int i;
	for(i = 0; i < I7_APP_NUM_REGICES; i++)
		g_regex_unref(I7_APP(self)->regices[i]);

	G_OBJECT_CLASS(i7_app_parent_class)->finalize(self);
}

static void
i7_app_class_init(I7AppClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(I7AppPrivate));

	object_class->finalize = i7_app_finalize;
}

/* Function to get the singleton application object. */
I7App *
i7_app_get(void)
{
	static I7App *theapp = NULL;

	if(G_UNLIKELY(theapp == NULL)) {
		theapp = I7_APP(g_object_new(I7_TYPE_APP, NULL));

		/* Do any setup activities for the application that require calling
		 i7_app_get() */
		populate_schemes_list(theapp->prefs->schemes_list);
		/* Set up Natural Inform highlighting on the example buffer */
		GtkSourceBuffer *buffer = GTK_SOURCE_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(theapp->prefs->source_example)));
		set_buffer_language(buffer, "inform7");
		set_highlight_styles(buffer);
	}

	return theapp;
}

/* Detect the type of document represented by @filename and open it. If that
 document is already open, then bring its window to the front. */
void
i7_app_open(I7App *app, GFile *file)
{
	I7Document *dupl = i7_app_get_already_open(app, file);
	if(dupl) {
		gtk_window_present(GTK_WINDOW(dupl));
		return;
	}

	if(g_file_query_exists(file, NULL)) {
		if(g_file_query_file_type(file, G_FILE_QUERY_INFO_NONE, NULL) == G_FILE_TYPE_DIRECTORY) {
			i7_story_new_from_file(app, file);
			/* TODO make sure story.ni exists */
		} /* else */
			/* TODO Use is_valid_extension to check if they're extensions and then open them */
	}
}

/* Insert the application's private action group into a GtkUIManager */
void
i7_app_insert_action_groups(I7App *app, GtkUIManager *manager)
{
	I7_APP_USE_PRIVATE(app, priv);
	gtk_ui_manager_insert_action_group(manager, priv->app_action_group, 0);
}

/* Add @document to the list of open documents */
void
i7_app_register_document(I7App *app, I7Document *document)
{
	I7_APP_USE_PRIVATE(app, priv);
	priv->document_list = g_slist_prepend(priv->document_list, document);
}

/* Remove @document from the list of open documents */
void
i7_app_remove_document(I7App *app, I7Document *document)
{
	I7_APP_PRIVATE(app)->document_list = g_slist_remove(I7_APP_PRIVATE(app)->document_list, document);
}

/* Custom search function for invocation of g_slist_find_custom() in
i7_app_get_already_open() below */
static gboolean
document_compare_file(const I7Document *document, GFile *file)
{
	GFile *document_file = i7_document_get_file(document);

	gboolean equal = g_file_equal(file, document_file);

	g_object_unref(document_file);
	return equal? 0 : 1;
}

/**
 * i7_app_get_already_open:
 * @app: the application
 * @file: a #GFile
 *
 * Check to see if @file is already open in this instance of the application.
 *
 * Returns: the corresponding #I7Document object if file is open, or %NULL
 * otherwise.
 */
I7Document *
i7_app_get_already_open(I7App *app, const GFile *file)
{
	I7_APP_USE_PRIVATE(app, priv);
	GSList *node = g_slist_find_custom(priv->document_list, file, (GCompareFunc)document_compare_file);
	if(node)
		return node->data;
	return NULL;
}

/* Return the number of documents currently open. */
gint
i7_app_get_num_open_documents(I7App *app)
{
	I7_APP_USE_PRIVATE(app, priv);
	return g_slist_length(priv->document_list);
}

/* Close all story windows, no cancelling allowed */
void
i7_app_close_all_documents(I7App *app)
{
	I7_APP_USE_PRIVATE(app, priv);
	g_slist_foreach(priv->document_list, (GFunc)i7_document_close, NULL);
	if(i7_app_get_num_open_documents(app) == 0)
		gtk_main_quit();
}

/* Carry out @func for each document window. To do something to each story or
 extension window only, call this function and check for I7_IS_STORY() in your
 callback function. */
void
i7_app_foreach_document(I7App *app, I7DocumentForeachFunc func, gpointer data)
{
	I7_APP_USE_PRIVATE(app, priv);
	g_slist_foreach(priv->document_list, (GFunc)func, data);
}

/* Callback for file monitor on extensions directory; run the census if a file
 was created or deleted */
static void
extension_dir_changed(GFileMonitor *monitor, GFile *file, GFile *other_file, GFileMonitorEvent event_type, I7App *app)
{
	if(event_type == G_FILE_MONITOR_EVENT_CREATED || event_type == G_FILE_MONITOR_EVENT_DELETED)
		i7_app_run_census(app, FALSE);
}

/* Set up a file monitor for the user's extensions directory */
void
i7_app_monitor_extensions_directory(I7App *app)
{
	I7_APP_USE_PRIVATE(app, priv);
	GError *error = NULL;
	if(!I7_APP_PRIVATE(app)->extension_dir_monitor) {
		GFile *extdir = i7_app_get_extension_file(app, NULL, NULL);
		priv->extension_dir_monitor = g_file_monitor_directory(extdir, G_FILE_MONITOR_NONE, NULL, &error);
		g_object_unref(extdir);
	}

	g_signal_connect(G_OBJECT(priv->extension_dir_monitor), "changed", G_CALLBACK(extension_dir_changed), app);
}

/* Turn off the file monitor on the user's extensions directory */
void
i7_app_stop_monitoring_extensions_directory(I7App *app)
{
	I7_APP_USE_PRIVATE(app, priv);
	g_file_monitor_cancel(priv->extension_dir_monitor);
	g_object_unref(priv->extension_dir_monitor);
	priv->extension_dir_monitor = NULL;
}

/* Reads an extension file to check whether it is a valid Inform 7 extension,
and returns the extension name and author stored in the locations pointed to by
'name' and 'author'. If the function returns TRUE, they must be freed. */
static gboolean
is_valid_extension(I7App *app, const gchar *text, gchar **thename, gchar **theauthor)
{
	g_return_val_if_fail(text != NULL, FALSE);

	GMatchInfo *match = NULL;

	if(!g_regex_match(app->regices[I7_APP_REGEX_EXTENSION], text, 0, &match)) {
		g_match_info_free(match);
		return FALSE;
	}
	if((*thename = g_match_info_fetch_named(match, "title")) == NULL) {
		g_match_info_free(match);
		return FALSE;
	}
	if((*theauthor = g_match_info_fetch_named(match, "author")) == NULL) {
		g_match_info_free(match);
		g_free(*thename);
		return FALSE;
	}

	return TRUE;
}

/**
 * i7_app_install_extension:
 * @app: the application
 * @file: a #GFile
 *
 * Install the extension @file into the user's extensions directory.
 */
void
i7_app_install_extension(I7App *app, GFile *file)
{
	g_return_if_fail(file);
	GError *err = NULL;

	/* Read the first line of the file */
	GFileInputStream *file_stream = g_file_read(file, NULL, &err);
	if(!file_stream)
		return;
	GDataInputStream *stream = g_data_input_stream_new(G_INPUT_STREAM(file_stream));
	char *text = g_data_input_stream_read_line_utf8(stream, NULL, NULL, &err);
	if(!text)
		return;

	/* Make sure the file is actually an Inform 7 extension */
	gchar *name = NULL;
	gchar *author = NULL;
	if(!is_valid_extension(app, text, &name, &author)) {
		char *display_name = file_get_display_name(file);
		error_dialog(NULL, NULL, _("The file '%s' does not seem to be an "
		  "extension. Extensions should be saved as UTF-8 text format files, "
		  "and should start with a line of one of these forms:\n\n<Extension> "
		  "by <Author> begins here.\nVersion <Version> of <Extension> by "
		  "<Author> begins here."), display_name);
		g_free(display_name);
		g_free(text);
		return;
	}
	g_free(text);

	/* Turn off the file monitor */
	i7_app_stop_monitoring_extensions_directory(app);

	/* Create the directory for that author if it does not exist already */
	GFile *dir = i7_app_get_extension_file(app, author, NULL);

	if(!make_directory_unless_exists(dir, NULL, &err)) {
		error_dialog_file_operation(NULL, dir, err, I7_FILE_ERROR_OTHER, _("creating a directory"));
		g_free(name);
		g_free(author);
		g_object_unref(dir);
		i7_app_monitor_extensions_directory(app);
		return;
	}

	char *canonical_name = g_strconcat(name, ".i7x", NULL);
	GFile *target = g_file_get_child(dir, name);
	GFile *canonical_target = g_file_get_child(dir, canonical_name);
	g_free(canonical_name);
	g_object_unref(dir);

	/* Check if the extension is already installed */
	if(g_file_query_exists(target, NULL) || g_file_query_exists(canonical_target, NULL))
	{
		GtkWidget *dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
		  _("A version of the extension %s by %s is already installed. Do you "
		  "want to overwrite the installed extension with this new one?"), name, author);
		if(gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_YES) {
			gtk_widget_destroy(dialog);
			g_object_unref(target);
			g_object_unref(canonical_target);
			g_free(name);
			g_free(author);
			i7_app_monitor_extensions_directory(app);
			return;
		}
		gtk_widget_destroy(dialog);
	}

	g_free(name);
	g_free(author);

	/* Copy the extension file to the user's extensions dir */
	if(!g_file_copy(file, canonical_target, G_FILE_COPY_OVERWRITE | G_FILE_COPY_TARGET_DEFAULT_PERMS, NULL, NULL, NULL, &err)) {
		error_dialog_file_operation(NULL, canonical_target, err, I7_FILE_ERROR_OTHER, _("copying a file"));
		g_object_unref(target);
		g_object_unref(canonical_target);
		i7_app_monitor_extensions_directory(app);
		return;
	}
	g_object_unref(canonical_target);

	/* If a version without *.i7x is still residing in that directory, delete
	it now */
	if(g_file_query_exists(target, NULL)) {
		if(!g_file_delete(target, NULL, &err) == -1)
			error_dialog_file_operation(NULL, target, err, I7_FILE_ERROR_OTHER, _("removing an old extension file"));
	}

	g_object_unref(target);
	i7_app_monitor_extensions_directory(app);

	/* Index the new extensions, in the foreground */
	i7_app_run_census(app, TRUE);
}

/*
 * remove_i7x_from_file:
 * @file: a #GFile reference.
 *
 * Removes the extension (in this case, it should be .i7x, but it doesn't really
 * matter) from the file and creates a new file reference.
 *
 * Returns: (transfer full): a new #GFile, unref when done; or %NULL if @file
 * didn't have an extension.
 */
static GFile *
remove_i7x_from_file(GFile *file)
{
	GFile *parent = g_file_get_parent(file);
	char *basename = g_file_get_basename(file);
	char *dot = strrchr(basename, '.');
	GFile *retval;

	if(dot) {
		*dot = '\0';
		retval = g_file_get_child(parent, basename);
	} else {
		retval = NULL;
	}

	g_object_unref(parent);
	g_free(basename);
	return retval;
}

/* Delete extension and remove author dir if empty */
void
i7_app_delete_extension(I7App *app, gchar *author, gchar *extname)
{
	GFile *file, *file_lc, *file_noext, *file_lc_noext, *author_dir, *author_dir_lc;
	char *extname_lc;
	GError *err = NULL;

	i7_app_stop_monitoring_extensions_directory(app);

	/* Get references to the various possible versions of this filename */
	file = i7_app_get_extension_file(app, author, extname);
	file_noext = remove_i7x_from_file(file);

	/* Remove extension, try versions with and without .i7x */
	if(!g_file_delete(file, NULL, &err) && !g_file_delete(file_noext, NULL, &err)) {
		error_dialog_file_operation(NULL, file, err, I7_FILE_ERROR_OTHER, _("deleting the file with or without extension"));
		g_object_unref(file);
		g_object_unref(file_noext);
		goto finally;
	}
	g_object_unref(file);
	g_object_unref(file_noext);

	/* Remove lowercase symlink to extension (holdover from previous versions
	of Inform) */
	extname_lc = g_utf8_strdown(extname, -1);
	file_lc = i7_app_get_extension_file(app, author, extname_lc);
	file_lc_noext = remove_i7x_from_file(file_lc);
	g_object_unref(file_lc);

	/* Only do this if the symlink actually exists */
	if(file_exists_and_is_symlink(file_lc_noext)) {
		if(!g_file_delete(file_lc_noext, NULL, &err)) {
			error_dialog_file_operation(NULL, file, err, I7_FILE_ERROR_OTHER, _("deleting an old symlink"));
			g_object_unref(file_lc_noext);
			goto finally;
		}
	}
	g_object_unref(file_lc_noext);

	/* Remove author directory if empty */
	author_dir = i7_app_get_extension_file(app, author, NULL);
	if(!g_file_delete(author_dir, NULL, &err)) {
		/* if the directory isn't empty, continue; but if it failed for any
		other reason, display an error */
		if(!g_error_matches(err, G_IO_ERROR, G_IO_ERROR_NOT_EMPTY)) {
			error_dialog_file_operation(NULL, author_dir, err, I7_FILE_ERROR_OTHER, _("deleting an empty directory"));
			g_object_unref(author_dir);
			goto finally;
		}
	}
	g_object_unref(author_dir);

	/* Remove lowercase symlink to author directory (holdover from previous
	versions of Inform) */
	gchar *author_lc = g_utf8_strdown(author, -1);
	author_dir_lc = i7_app_get_extension_file(app, author_lc, NULL);
	g_free(author_lc);
	/* Only do this if the symlink actually exists */
	if(file_exists_and_is_symlink(author_dir_lc)) {
		if(!g_file_delete(author_dir_lc, NULL, &err)) {
			error_dialog_file_operation(NULL, author_dir_lc, err, I7_FILE_ERROR_OTHER, _("deleting an old symlink"));
		}
	}
	g_object_unref(author_dir_lc);

finally:
	i7_app_monitor_extensions_directory(app);

	/* Index the new extensions, in the foreground */
	i7_app_run_census(app, TRUE);
}

/* Return the full path to the built-in Inform extension represented by @author
 and @extname, if not NULL; return the author directory if @extname is NULL;
 return the built-in extensions path if both are NULL. */
static GFile *
get_builtin_extension_file(I7App *app, const gchar *author,
	const gchar *extname)
{
	if(!author)
		return i7_app_get_data_file_va(app, "Extensions", NULL);
	if(!extname)
		return i7_app_get_data_file_va(app, "Extensions", author, NULL);
	return i7_app_get_data_file_va(app, "Extensions", author, extname, NULL);
}

/**
 * i7_app_foreach_installed_extension:
 * @app: the app
 * @builtin: whether to iterate over the built-in extensions or the
 * user-installed ones
 * @author_func: (allow-none) (scope call): a function to call for each author
 * directory found, which may return a result
 * @author_func_data: (allow-none): data to pass to @author_func
 * @extension_func: (allow-none) (scope call): a function to call for each
 * extension file found in each author directory
 * @extension_func_data: (allow-none): data to pass to @extension_func
 * @free_author_result: (allow-none): function to free the return value of
 * @author_func
 *
 * Iterates over the installed extensions (the built-in ones if @builtin is
 * %TRUE, or the user-installed ones if %FALSE), calling functions for each
 * author directory and each installed extension in each author directory.
 *
 * @author_func may return a result, which is passed to the @extension_func when
 * called for extensions in that author directory. After the author directory
 * has been traversed, @free_author_result is called on the return value of
 * @author_func, if both of them are not %NULL.
 */
void
i7_app_foreach_installed_extension(I7App *app, gboolean builtin, I7AppAuthorFunc author_func, gpointer author_func_data, I7AppExtensionFunc extension_func, gpointer extension_func_data, GDestroyNotify free_author_result)
{
	GError *err = NULL;
	GFile *root_file;
	GFileEnumerator *root_dir;
	GFileInfo *author_info;
	gpointer author_result;

	if(builtin)
		root_file = get_builtin_extension_file(app, NULL, NULL);
	else
		root_file = i7_app_get_extension_file(app, NULL, NULL);

	root_dir = g_file_enumerate_children(root_file, "standard::*", G_FILE_QUERY_INFO_NONE, NULL, &err);
	if(!root_dir) {
		error_dialog_file_operation(NULL, root_file, err, I7_FILE_ERROR_OTHER, _("opening extensions directory"));
		g_object_unref(root_file);
		return;
	}

	while((author_info = g_file_enumerator_next_file(root_dir, NULL, NULL)) != NULL) {
		const char *author_name = g_file_info_get_name(author_info);
		GFile *author_file;
		GFileEnumerator *author_dir;
		GFileInfo *extension_info;

		/* Read each extension dir, but skip "Reserved" and nondirs */
		if(strcmp(author_name, "Reserved") == 0)
			continue;
		if(g_file_info_get_file_type(author_info) != G_FILE_TYPE_DIRECTORY)
			continue;

		if(author_func)
			author_result = author_func(author_info, author_func_data);
		else
			author_result = NULL;

		/* Descend into each author directory */
		author_file = g_file_get_child(root_file, author_name);
		author_dir = g_file_enumerate_children(author_file, "standard::*", G_FILE_QUERY_INFO_NONE, NULL, &err);
		if(!author_dir) {
			error_dialog_file_operation(NULL, author_file, err, I7_FILE_ERROR_OTHER, _("opening extensions directory"));
			g_object_unref(author_file);
			return;
		}

		while((extension_info = g_file_enumerator_next_file(author_dir, NULL, NULL)) != NULL) {

			/* Read each file, but skip symlinks */
			if(g_file_info_get_is_symlink(extension_info))
				continue;

			if(extension_func)
				extension_func(author_file, extension_info, author_result, extension_func_data);

			g_object_unref(extension_info);
		}

		if(free_author_result && author_result)
			free_author_result(author_result);

		/* Finished enumerating author directory */
		if(err) {
			error_dialog_file_operation(NULL, author_file, err, I7_FILE_ERROR_OTHER, _("reading extensions directory"));
		}
		g_object_unref(author_file);
		g_file_enumerator_close(author_dir, NULL, NULL); /* ignore error */
		g_object_unref(author_dir);
		g_object_unref(author_info);
	}

	/* Finished enumerating user extensions directory */
	if(err) {
		error_dialog_file_operation(NULL, root_file, err, I7_FILE_ERROR_OTHER, _("reading extensions directory"));
	}
	g_object_unref(root_file);
	g_file_enumerator_close(root_dir, NULL, NULL); /* ignore error */
	g_object_unref(root_dir);
}

/* Helper function: Add author to tree store callback */
static GtkTreeIter *
add_author_to_tree_store(GFileInfo *info, GtkTreeStore *store)
{
	GtkTreeIter parent_iter;
	const char *author_display_name = g_file_info_get_display_name(info);
	gboolean found = FALSE;

	/* If the author directory was already indexed before, add the extension
	to it instead of making a new entry */
	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &parent_iter)) {
		do {
			char *author;
			gtk_tree_model_get(GTK_TREE_MODEL(store), &parent_iter, I7_APP_EXTENSION_TEXT, &author, -1);
			if(strcmp(author_display_name, author) == 0)
				found = TRUE;
			g_free(author);
			if(found)
				break;
		} while(gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &parent_iter));
	}

	if(!found) {
		gtk_tree_store_append(store, &parent_iter, NULL);
		gtk_tree_store_set(store, &parent_iter,
			I7_APP_EXTENSION_TEXT, author_display_name,
			I7_APP_EXTENSION_READ_ONLY, TRUE,
			I7_APP_EXTENSION_ICON, NULL,
			I7_APP_EXTENSION_FILE, NULL,
			-1);
	}

	return gtk_tree_iter_copy(&parent_iter);
}

/* Helper function: add extension to tree store as a non-built-in extension */
static void
add_extension_to_tree_store(GFile *parent, GFileInfo *info, GtkTreeIter *parent_iter, GtkTreeStore *store)
{
	const char *extension_name = g_file_info_get_name(info);
	GFile *extension_file = g_file_get_child(parent, extension_name);
	const char *display_name = g_file_info_get_display_name(info);
	GtkTreeIter child_iter;

	/* Remove .i7x from the filename, if it is there */
	char *i7_display_name;
	if(g_str_has_suffix(display_name, ".i7x"))
		i7_display_name = g_strndup(display_name, strlen(display_name) - 4);
	else
		i7_display_name = g_strdup(display_name);

	gtk_tree_store_append(store, &child_iter, parent_iter);
	gtk_tree_store_set(store, &child_iter,
		I7_APP_EXTENSION_TEXT, i7_display_name, /* copies */
		I7_APP_EXTENSION_READ_ONLY, FALSE,
		I7_APP_EXTENSION_ICON, NULL,
		I7_APP_EXTENSION_FILE, extension_file, /* references */
		-1);

	g_object_unref(extension_file);
	g_free(i7_display_name);
}

/* Helper function: add extension to tree store as a built-in extension. Makes
 * sure that user-installed extensions override the built-in ones. */
static void
add_builtin_extension_to_tree_store(GFile *parent, GFileInfo *info, GtkTreeIter *parent_iter, GtkTreeStore *store)
{
	const char *extension_name = g_file_info_get_name(info);
	const char *display_name = g_file_info_get_display_name(info);
	GFile *extension_file = g_file_get_child(parent, extension_name);
	GtkTreeIter child_iter;
	gboolean found;

	/* Remove .i7x from the filename, if it is there */
	char *i7_display_name;
	if(g_str_has_suffix(display_name, ".i7x"))
		i7_display_name = g_strndup(display_name, strlen(display_name) - 4);
	else
		i7_display_name = g_strdup(display_name);

	/* Only add it if it is not overridden by a user-installed extension */
	found = FALSE;
	if(gtk_tree_model_iter_children(GTK_TREE_MODEL(store), &child_iter, parent_iter)) {
		do {
			char *name;
			gtk_tree_model_get(GTK_TREE_MODEL(store), &child_iter,
				I7_APP_EXTENSION_TEXT, &name,
				-1);
			if(strcmp(i7_display_name, name) == 0)
				found = TRUE;
			g_free(name);
			if(found)
				break;
		} while(gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &child_iter));
	}

	if(!found) {
		gtk_tree_store_append(store, &child_iter, parent_iter);
		gtk_tree_store_set(store, &child_iter,
			I7_APP_EXTENSION_TEXT, i7_display_name,
			I7_APP_EXTENSION_READ_ONLY, TRUE,
			I7_APP_EXTENSION_ICON, "inform7-builtin",
			I7_APP_EXTENSION_FILE, extension_file,
			-1);
	}

	g_object_unref(extension_file);
	g_free(i7_display_name);
}

/* Helper function: look in the user's extensions directory and the built-in one
 and list all the extensions there in the application's extensions tree */
static gboolean
update_installed_extensions_tree(I7App *app)
{
	GtkTreeStore *store = I7_APP_PRIVATE(app)->installed_extensions;
	gtk_tree_store_clear(store);

	i7_app_foreach_installed_extension(app, FALSE,
	    (I7AppAuthorFunc)add_author_to_tree_store, store,
	    (I7AppExtensionFunc)add_extension_to_tree_store, store,
	    (GDestroyNotify)gtk_tree_iter_free);
	i7_app_foreach_installed_extension(app, TRUE,
	    (I7AppAuthorFunc)add_author_to_tree_store, store,
	    (I7AppExtensionFunc)add_builtin_extension_to_tree_store, store,
	    (GDestroyNotify)gtk_tree_iter_free);

	/* Rebuild the Open Extension menus */
	i7_app_update_extensions_menu(app);

	return FALSE; /* one-shot idle function */
}

/* Start the compiler running the census of extensions. If @wait is FALSE, do it
 in the background. */
void
i7_app_run_census(I7App *app, gboolean wait)
{
	GFile *ni_binary = i7_app_get_binary_file(app, "ni");
	GFile *builtin_extensions = get_builtin_extension_file(app, NULL, NULL);

	/* Build the command line */
	gchar **commandline = g_new(gchar *, 5);
	commandline[0] = g_file_get_path(ni_binary);
	commandline[1] = g_strdup("--rules");
	commandline[2] = g_file_get_path(builtin_extensions);
	commandline[3] = g_strdup("--census");
	commandline[4] = NULL;

	g_object_unref(ni_binary);
	g_object_unref(builtin_extensions);

	/* Run the census */
	if(wait) {
		g_spawn_sync(g_get_home_dir(), commandline, NULL, G_SPAWN_SEARCH_PATH
			| G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL,
			NULL, NULL, NULL, NULL, NULL, NULL);
		update_installed_extensions_tree(app);
	} else {
		g_spawn_async(g_get_home_dir(), commandline, NULL, G_SPAWN_SEARCH_PATH
			| G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL,
			NULL, NULL, NULL, NULL);
		g_idle_add((GSourceFunc)update_installed_extensions_tree, app);
	}

	g_strfreev(commandline);
}

/**
 * i7_app_get_extension_file:
 * @app: the application
 * @author: (allow-none): the extension author
 * @extname: (allow-none): the extensions name, with or without .i7x
 *
 * Returns the directory for user-installed extensions, with the @author and
 * @extname path components tacked on if they are not %NULL. Does not check
 * whether the file exists.
 *
 * Returns: (transfer full): a new #GFile.
 */
GFile *
i7_app_get_extension_file(I7App *app, const gchar *author, const gchar *extname)
{
	char *path;

	if(!author) {
		path = g_build_filename(g_get_home_dir(), EXTENSIONS_BASE_PATH, NULL);
	} else if(!extname) {
		path = g_build_filename(g_get_home_dir(), EXTENSIONS_BASE_PATH, author, NULL);
	} else {
		char *extname_ext;
		if(g_str_has_suffix(extname, ".i7x"))
			extname_ext = g_strdup(extname);
		else
			extname_ext = g_strconcat(extname, ".i7x", NULL);

		path = g_build_filename(g_get_home_dir(), EXTENSIONS_BASE_PATH, author,	extname_ext, NULL);
		g_free(extname_ext);
	}

	GFile *retval = g_file_new_for_path(path);
	g_free(path);
	return retval;
}

/**
 * i7_app_get_data_file:
 * @app: the app
 * @filename: the basename of the data file
 *
 * Locates @filename in the application data directory. If it is not found,
 * displays an error dialog asking the user to reinstall the application.
 *
 * Returns: (transfer full): a new #GFile pointing to @filename, or %NULL if not
 * found.
 */
GFile *
i7_app_get_data_file(I7App *app, const char *filename)
{
	GFile *retval = g_file_get_child(I7_APP_PRIVATE(app)->datadir, filename);

	if(g_file_query_exists(retval, NULL))
		return retval;

	g_object_unref(retval);
	error_dialog(NULL, NULL, _("An application file, %s, was not found. "
		"Please reinstall Inform 7."), filename);
	return NULL;
}

/**
 * i7_app_get_data_file_va:
 * @app: the app
 * @path1: first component of the path
 * @...: subsequent path components, ending with %NULL.
 *
 * Locates a file in a subdirectory of the application data directory. If it is
 * not found, displays an error dialog asking the user to reinstall the
 * application.
 *
 * Returns: (transfer full): a new #GFile pointing to @filename, or %NULL if not
 * found.
 */
GFile *
i7_app_get_data_file_va(I7App *app, const char *path1, ...)
{
	va_list ap;
	GFile *retval, *previous;
	char *arg, *lastarg = NULL;

	retval = previous = g_file_get_child(I7_APP_PRIVATE(app)->datadir, path1);

	va_start(ap, path1);
	while((arg = va_arg(ap, char *)) != NULL) {
		retval = g_file_get_child(previous, arg);
		g_object_unref(previous);
		previous = retval;
		lastarg = arg;
	}
	va_end(ap);

	if(g_file_query_exists(retval, NULL))
		return retval;

	g_object_unref(retval);
	error_dialog(NULL, NULL, _("An application file, %s, was not found. "
		"Please reinstall Inform 7."), lastarg); /* argument before NULL */
	return NULL;
}

/**
 * i7_app_check_data_file:
 * @app: the app
 * @filename: the basename of the data file
 *
 * Locates @filename in the application data directory. Used when we do not
 * necessarily want to display an error if it does not exist.
 *
 * Returns: (transfer full): a new #GFile pointing to @filename, or %NULL if not
 * found.
 */
GFile *
i7_app_check_data_file(I7App *app, const char *filename)
{
	GFile *retval = g_file_get_child(I7_APP_PRIVATE(app)->datadir, filename);

	if(!g_file_query_exists(retval, NULL)) {
		g_object_unref(retval);
		return NULL;
	}

	return retval;
}

/**
 * i7_app_check_data_file_va:
 * @app: the app
 * @path1: first component of the path
 * @...: subsequent path components, ending with %NULL.
 *
 * Locates a file in a subdirectory of the application data directory. Used when
 * we do not necessarily want to display an error if it does not exist.
 *
 * Returns: (transfer full): a new #GFile pointing to @filename, or %NULL if not
 * found.
 */
GFile *
i7_app_check_data_file_va(I7App *app, const char *path1, ...)
{
	va_list ap;
	GFile *retval, *previous;
	char *arg;

	retval = previous = g_file_get_child(I7_APP_PRIVATE(app)->datadir, path1);

	va_start(ap, path1);
	while((arg = va_arg(ap, char *)) != NULL) {
		retval = g_file_get_child(previous, arg);
		g_object_unref(previous);
		previous = retval;
	}
	va_end(ap);

	if(!g_file_query_exists(retval, NULL)) {
		g_object_unref(retval);
		return NULL;
	}

	return retval;
}

/**
 * i7_app_get_binary_file:
 * @app: the app
 * @filename: the basename of the file
 *
 * Locates @filename in the application libexec directory. If it is not found,
 * displays an error dialog asking the user to reinstall the application.
 *
 * Returns: (transfer full): a new #GFile pointing to @filename, or %NULL if not
 * found.
 */
GFile *
i7_app_get_binary_file(I7App *app, const char *filename)
{
	GFile *retval = g_file_get_child(I7_APP_PRIVATE(app)->libexecdir, filename);

	if(g_file_query_exists(retval, NULL))
		return retval;

	g_object_unref(retval);
	error_dialog(NULL, NULL, _("An application file, %s, was not found. "
		"Please reinstall Inform 7."), filename);
	return NULL;
}

/* Getter function for installed extensions tree */
GtkTreeStore *
i7_app_get_installed_extensions_tree(I7App *app)
{
	return I7_APP_PRIVATE(app)->installed_extensions;
}

/* Regenerate the installed extensions submenu attached to @parent_item */
static void
rebuild_extensions_menu(GtkWidget *parent_item, I7App *app)
{
	GtkTreeModel *model = GTK_TREE_MODEL(I7_APP_PRIVATE(app)->installed_extensions);
	GtkTreeIter author, title;
	GtkWidget *authormenu = gtk_menu_new();

	gtk_tree_model_get_iter_first(model, &author);
	do {
		gchar *authorname;
		gtk_tree_model_get(model, &author, I7_APP_EXTENSION_TEXT, &authorname, -1);
		GtkWidget *authoritem = gtk_menu_item_new_with_label(authorname);
		gtk_widget_show(authoritem);
		gtk_menu_shell_append(GTK_MENU_SHELL(authormenu), authoritem);

		g_assert(gtk_tree_model_iter_children(model, &title, &author));
		GtkWidget *extmenu = gtk_menu_new();
		do {
			char *extname;
			GFile *extension_file;
			gboolean readonly;
			gtk_tree_model_get(model, &title,
				I7_APP_EXTENSION_TEXT, &extname,
				I7_APP_EXTENSION_READ_ONLY, &readonly,
				I7_APP_EXTENSION_FILE, &extension_file,
				-1);
			GtkWidget *extitem;
			if(readonly) {
				extitem = gtk_image_menu_item_new_with_label(extname);
				GtkWidget *image = gtk_image_new_from_icon_name("inform7-builtin", GTK_ICON_SIZE_MENU);
				gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(extitem), image);
				g_signal_connect(extitem, "activate", G_CALLBACK(on_open_extension_readonly_activate), extension_file);
			} else {
				extitem = gtk_menu_item_new_with_label(extname);
				g_signal_connect(extitem, "activate", G_CALLBACK(on_open_extension_activate), extension_file);
			}
			gtk_widget_show(extitem);
			gtk_menu_shell_append(GTK_MENU_SHELL(extmenu), extitem);

			g_free(extname);
			g_object_unref(extension_file);

		} while(gtk_tree_model_iter_next(model, &title));
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(authoritem), extmenu);
	} while(gtk_tree_model_iter_next(model, &author));
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(parent_item), authormenu);
}

/* Rebuild all the Open Installed Extensions submenus in existence, by calling
 rebuild_extensions_menu() on all proxies of the "open_extension" action */
void
i7_app_update_extensions_menu(I7App *app)
{
	GSList *proxies = gtk_action_get_proxies(gtk_action_group_get_action(I7_APP_PRIVATE(app)->app_action_group, "open_extension"));
	/* do not free list */
	g_slist_foreach(proxies, (GFunc)rebuild_extensions_menu, app);
}

/* Getter function for the global print settings object */
GtkPrintSettings *
i7_app_get_print_settings(I7App *app)
{
	return I7_APP_PRIVATE(app)->print_settings;
}

/* Setter function for the global print settings object */
void
i7_app_set_print_settings(I7App *app, GtkPrintSettings *settings)
{
	I7AppPrivate *priv = I7_APP_PRIVATE(app);
	if(priv->print_settings)
		g_object_unref(priv->print_settings);
	priv->print_settings = settings;
}

/* Getter function for the global page setup object */
GtkPageSetup *
i7_app_get_page_setup(I7App *app)
{
	return I7_APP_PRIVATE(app)->page_setup;
}

/* Setter function for the global page setup object */
void
i7_app_set_page_setup(I7App *app, GtkPageSetup *setup)
{
	I7AppPrivate *priv = I7_APP_PRIVATE(app);
	if(priv->page_setup)
		g_object_unref(priv->page_setup);
	priv->page_setup = setup;
}

/* Bring the preferences dialog to the front */
void
i7_app_present_prefs_window(I7App *app)
{
	gtk_window_present(GTK_WINDOW(app->prefs->window));
}

/* Helper function: change the cursor of @toplevel's GdkWindow to @cursor.
 Called by g_list_foreach() in i7_app_set_busy() below. */
static void
set_cursor(GtkWindow *toplevel, GdkCursor *cursor)
{
	GdkWindow *window = gtk_widget_get_window(GTK_WIDGET(toplevel));
	if(window)
		gdk_window_set_cursor(window, cursor);
}

/* Change the cursor in all application windows to GDK_WATCH if @busy is TRUE,
 or to the default cursor if @busy is FALSE. */
void
i7_app_set_busy(I7App *app, gboolean busy)
{
	GList *windows = gtk_window_list_toplevels();
	if(busy) {
		GdkCursor *cursor = gdk_cursor_new(GDK_WATCH);
		g_list_foreach(windows, (GFunc)set_cursor, cursor);
		gdk_cursor_unref(cursor);
	} else
		g_list_foreach(windows, (GFunc)set_cursor, NULL);
	gdk_flush();
	g_list_free(windows);
}
