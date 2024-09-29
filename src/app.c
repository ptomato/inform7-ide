/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Philip Chimento <philip.chimento@gmail.com>
 */

#include "config.h"

#include <stdlib.h>
#include <stdarg.h>

#include <gio/gio.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>
#include <handy.h>

#include "app.h"
#include "app-retrospective.h"
#include "actions.h"
#include "configfile.h"
#include "error.h"
#include "file.h"
#include "welcomedialog.h"

#define EXTENSIONS_BASE_PATH "Inform", "Extensions"
#define EXTENSION_HOME_PATH "Inform", "Documentation", "Extensions.html"
#define EXTENSION_INDEX_PATH "Inform", "Documentation", "ExtIndex.html"
#define EXTENSION_DOCS_BASE_PATH "Inform", "Documentation", "Extensions"
#define EXTENSION_DOWNLOAD_TIMEOUT_S 15

/* The singleton application class. Contains the following global miscellaneous
 stuff:
 - actions that would be valid even if there were no document open (even though
   there is no menu at present when no document is open.)
 - the list of open documents.
 - information about the paths to project data and executable files.
 - the file monitor for the extension directory.
 - the tree of installed extensions.
 - the print settings and page setup objects.
 - the preferences dialog.
 - various compiled regices for use elsewhere in the program.
*/

struct _I7AppClass {
	GtkApplicationClass parent_class;
};

struct _I7App {
	GtkApplication parent_instance;

	/* Application directories */
	GFile *datadir;
	GFile *libexecdir;
	/* File monitor for extension directory */
	GFileMonitor *extension_dir_monitor;
	/* Tree of installed extensions (GNode<I7InstalledExtension>) */
	GNode *installed_extensions;
	/* Current print settings */
	GtkPrintSettings *print_settings;
	/* Color scheme manager */
	GtkSourceStyleSchemeManager *color_scheme_manager;
	/* Parsed contents of retrospective.txt (GListStore<I7Retrospective>) */
    GListStore *retrospectives;
	/* Preferences settings */
	GSettings *system_settings;
	GSettings *prefs_settings;
	GSettings *state_settings;
	GtkCssProvider *font_settings_provider;
} I7AppPrivate;

G_DEFINE_TYPE(I7App, i7_app, GTK_TYPE_APPLICATION);

static gboolean
free_installed_extension_node(GNode *node, void *unused)
{
	I7InstalledExtension *data = node->data;
	if (data) {
		g_clear_pointer(&data->title, g_free);
		g_clear_pointer(&data->version, g_free);
		g_clear_object(&data->file);
		g_free(data);
	}
	return false;  /* keep going */
}

static void
free_installed_extensions_tree(GNode *root)
{
	g_node_traverse(root, G_IN_ORDER, G_TRAVERSE_ALL, -1, free_installed_extension_node, NULL);
	g_node_destroy(root);
}

/* Helper function: call gtk_source_style_scheme_manager_append_search_path()
with a #GFile */
static void
scheme_manager_append_search_path_gfile(GtkSourceStyleSchemeManager *manager, GFile *file)
{
	char *path = g_file_get_path(file);
	gtk_source_style_scheme_manager_append_search_path(manager, path);
	g_free(path);
}

/* Helper function: create the application's color scheme manager (transfer full) */
static GtkSourceStyleSchemeManager *
create_color_scheme_manager(I7App *self)
{
	GtkSourceStyleSchemeManager *manager = gtk_source_style_scheme_manager_new();

	/* Add the built-in styles directory */
	GFile *styles_dir = i7_app_get_data_file(self, "styles");
	scheme_manager_append_search_path_gfile(manager, styles_dir);
	g_object_unref(styles_dir);

	/* Add the user styles directory */
	GFile *config_dir = i7_app_get_config_dir();
	styles_dir = g_file_get_child(config_dir, "styles");
	g_object_unref(config_dir);
	scheme_manager_append_search_path_gfile(manager, styles_dir);
	g_object_unref(styles_dir);

	return manager;
}

typedef void (*ActionCallback)(GSimpleAction *, GVariant *, void *);

static void
create_app_actions(I7App *self)
{
	const GActionEntry actions[] = {
		{ "new", (ActionCallback)action_new },
		{ "open", (ActionCallback)action_open },
		{ "open-recent", (ActionCallback)action_open_recent, "(ss)" },
		{ "install-extension", (ActionCallback)action_install_extension },
		{ "open-extension", (ActionCallback)action_open_extension, "(sb)" },
		{ "quit", (ActionCallback)action_quit },
		{ "preferences", (ActionCallback)action_preferences },
		{ "visit-inform7-com", (ActionCallback)action_visit_inform7_com },
        { "report-bug", (ActionCallback)action_report_bug },
		{ "about", (ActionCallback)action_about },
	};
	g_action_map_add_action_entries(G_ACTION_MAP(self), actions, G_N_ELEMENTS(actions), self);
}

static void
rebuild_recent_menu(GtkRecentManager *manager, I7App *self)
{
	GList *recent = gtk_recent_manager_get_items(manager);
	GMenu *recent_menu = gtk_application_get_menu_by_id(GTK_APPLICATION(self), "recent");
	g_menu_remove_all(recent_menu);

	for (GList *iter = recent; iter != NULL; iter = g_list_next(iter)) {
		g_autoptr(GtkRecentInfo) info = iter->data;
		if (gtk_recent_info_has_application(info, "Inform 7")) {
			const char *group = NULL;
			if (gtk_recent_info_has_group(info, "inform7_project"))
				group = "inform7_project";
			else if (gtk_recent_info_has_group(info, "inform7_extension"))
				group = "inform7_extension";
			else if (gtk_recent_info_has_group(info, "inform7_builtin"))
				group = "inform7_builtin";
			else
				continue;

            g_autofree char *escaped_uri = g_strescape(gtk_recent_info_get_uri(info), NULL);
			g_autofree char *action = g_strdup_printf("app.open-recent((\"%s\",'%s'))", escaped_uri, group);
			g_autoptr(GMenuItem) item = g_menu_item_new(gtk_recent_info_get_display_name(info), action);
			g_menu_append_item(recent_menu, item);
		}
	}
    g_list_free(recent);
}

static void
i7_app_init(I7App *self)
{
	GError *error = NULL;

	self->system_settings = g_settings_new(SCHEMA_SYSTEM);
	self->prefs_settings = g_settings_new(SCHEMA_PREFERENCES);
	self->state_settings = g_settings_new(SCHEMA_STATE);

	self->font_settings_provider = gtk_css_provider_new();
	gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(self->font_settings_provider),
		GTK_STYLE_PROVIDER_PRIORITY_USER);
	g_autoptr(GtkCssProvider) css = gtk_css_provider_new();
	gtk_css_provider_load_from_resource(css, "/com/inform7/IDE/ui/application.css");
	gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(css),
		GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

	/* Retrieve data directories if set externally */
	const gchar *env = g_getenv("INFORM7_IDE_DATA_DIR");
	if(env) {
		self->datadir = g_file_new_for_path(env);
	} else {
		char *path = g_build_filename(PACKAGE_DATA_DIR, "inform7-ide", NULL);
		self->datadir = g_file_new_for_path(path);
		g_free(path);
	}

	env = g_getenv("INFORM7_IDE_LIBEXEC_DIR");
	self->libexecdir = g_file_new_for_path(env? env : PACKAGE_LIBEXEC_DIR);

	create_app_actions(self);

	GtkRecentManager *default_recent_manager = gtk_recent_manager_get_default();
	g_signal_connect(default_recent_manager, "changed", G_CALLBACK(rebuild_recent_menu), self);

	self->installed_extensions = g_node_new(NULL);
	/* Set print settings to NULL, since they are not remembered across
	application runs (yet) */
	self->print_settings = NULL;

	/* Create the Inform dir if it doesn't already exist */
	g_autoptr(GFile) extensions_file = i7_app_get_extension_file(NULL, NULL);
	if(!make_directory_unless_exists(extensions_file, NULL, &error)) {
		IO_ERROR_DIALOG(NULL, extensions_file, error, _("creating the Inform directory"));
	}

	/* Set up signals for GSettings keys. */
	init_config_file(self->prefs_settings);
	g_signal_connect_swapped(self->system_settings, "changed::document-font-name", G_CALLBACK(i7_app_update_css), self);
	g_signal_connect_swapped(self->system_settings, "changed::monospace-font-name", G_CALLBACK(i7_app_update_css), self);

	/* Create the color scheme manager (must be run after priv->datadir is set) */
	self->color_scheme_manager = create_color_scheme_manager(self);

	/* Parse the retrospective.txt file */
	parse_retrospective_txt(&self->retrospectives);
}

static void
i7_app_finalize(GObject *object)
{
	I7App *self = I7_APP(object);

	g_object_unref(self->datadir);
	g_object_unref(self->libexecdir);
	i7_app_stop_monitoring_extensions_directory(self);
	g_clear_pointer(&self->installed_extensions, free_installed_extensions_tree);
	g_object_unref(self->color_scheme_manager);
	g_clear_object(&self->retrospectives);
	g_object_unref(self->system_settings);
	g_object_unref(self->state_settings);
	g_object_unref(self->prefs_settings);
	g_clear_object(&self->font_settings_provider);

	G_OBJECT_CLASS(i7_app_parent_class)->finalize(object);
}

static void
i7_app_startup(GApplication *app)
{
	I7App *self = I7_APP(app);

	G_APPLICATION_CLASS(i7_app_parent_class)->startup(app);

	hdy_init();

	/* Set up monitor for extensions directory */
	i7_app_run_census(self, FALSE);
	i7_app_monitor_extensions_directory(self);

	/* Set initial font sizes */
	i7_app_update_css(self);
}

static void
i7_app_activate(GApplication *app)
{
	/* If no windows were opened from command line arguments */
	if (gtk_application_get_windows(GTK_APPLICATION(app)) == NULL) {
		/* Create the splash window */
		GtkWidget *welcomedialog = create_welcome_dialog(GTK_APPLICATION(app));
		gtk_widget_show_all(welcomedialog);
	}
}

/* Detect the type of document represented by @filename and open it. If that
 document is already open, then bring its window to the front. */
static void
i7_app_open(GApplication *app, GFile **files, int n_files, const char *hint)
{
	I7App *self = I7_APP(app);

	for (int index = 0; index < n_files; index++) {
		I7Document *dupl = i7_app_get_already_open(self, files[index]);
		if (dupl) {
			gtk_window_present(GTK_WINDOW(dupl));
			continue;
		}

		if (g_file_query_exists(files[index], NULL)) {
			if (g_file_query_file_type(files[index], G_FILE_QUERY_INFO_NONE, NULL) == G_FILE_TYPE_DIRECTORY) {
				i7_story_new_from_file(self, files[index]);
				/* TODO make sure story.ni exists */
			} /* else */
				/* TODO Use is_valid_extension to check if they're extensions and then open them */
		}
	}
}

static void
i7_app_class_init(I7AppClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = i7_app_finalize;

	GApplicationClass *application_class = G_APPLICATION_CLASS(klass);
	application_class->startup = i7_app_startup;
	application_class->activate = i7_app_activate;
	application_class->open = i7_app_open;
}

I7App *
i7_app_new(void)
{
	return I7_APP(g_object_new(I7_TYPE_APP,
		"application-id", "com.inform7.IDE",
		"flags", G_APPLICATION_HANDLES_OPEN,
		NULL));
}

/* Custom search function for invocation of g_slist_find_custom() in
i7_app_get_already_open() below */
static int
document_compare_file(GtkWindow *window, GFile *file)
{
	if (!I7_IS_DOCUMENT(window))
		return 1;

	g_autoptr(GFile) document_file = i7_document_get_file(I7_DOCUMENT(window));
	return g_file_equal(file, document_file)? 0 : 1;
}

/**
 * i7_app_get_already_open:
 * @self: the application
 * @file: a #GFile
 *
 * Check to see if @file is already open in this instance of the application.
 *
 * Returns: the corresponding #I7Document object if file is open, or %NULL
 * otherwise.
 */
I7Document *
i7_app_get_already_open(I7App *self, const GFile *file)
{
	GList *document_list = gtk_application_get_windows(GTK_APPLICATION(self));
	GList *node = g_list_find_custom(document_list, file, (GCompareFunc)document_compare_file);
	if(node)
		return node->data;
	return NULL;
}

/* Close all story windows, no cancelling allowed */
void
i7_app_close_all_documents(I7App *self)
{
	GList *document_list = gtk_application_get_windows(GTK_APPLICATION(self));
	g_list_foreach(document_list, (GFunc)i7_document_close, NULL);
}

/* Callback for file monitor on extensions directory; run the census if a file
 was created or deleted */
static void
extension_dir_changed(GFileMonitor *monitor, GFile *file, GFile *other_file, GFileMonitorEvent event_type, I7App *self)
{
	if(event_type == G_FILE_MONITOR_EVENT_CREATED || event_type == G_FILE_MONITOR_EVENT_DELETED)
		i7_app_run_census(self, FALSE);
}

/* Set up a file monitor for the user's extensions directory */
void
i7_app_monitor_extensions_directory(I7App *self)
{
	GError *error = NULL;
	if (!self->extension_dir_monitor) {
		g_autoptr(GFile) extdir = i7_app_get_extension_file(NULL, NULL);
		self->extension_dir_monitor = g_file_monitor_directory(extdir, G_FILE_MONITOR_NONE, NULL, &error);
	}

	g_signal_connect(G_OBJECT(self->extension_dir_monitor), "changed", G_CALLBACK(extension_dir_changed), self);
}

/* Turn off the file monitor on the user's extensions directory */
void
i7_app_stop_monitoring_extensions_directory(I7App *self)
{
	if (self->extension_dir_monitor) {
		g_file_monitor_cancel(self->extension_dir_monitor);
		g_clear_object(&self->extension_dir_monitor);
	}
}

/* Examines the @text (at least the first line) of an extension file to check
whether it is a valid Inform 7 extension, returns TRUE if the header is correct,
and returns the extension version, name, and author stored in the locations
pointed to by @version, @name, and @author, respectively. If the function
returns FALSE, nothing is stored in those locations; if it returns TRUE, then
the values returned in @name and @author must be freed, and the value in
@version must be freed if it is not NULL. It is okay to pass NULL for @version,
@name, and @author, in which case nothing will be stored there. */
static gboolean
is_valid_extension(const char *text, char **version, char **name, char **author)
{
	g_return_val_if_fail(text != NULL, FALSE);

	g_autoptr(GRegex) regex = g_regex_new(REGEX_EXTENSION, G_REGEX_OPTIMIZE | G_REGEX_CASELESS, 0, /* ignore error */ NULL);
	g_assert(regex && "Failed to compile extension regex");

	g_autoptr(GMatchInfo) match = NULL;
	if (!g_regex_match(regex, text, 0, &match))
		return FALSE;
	g_autofree char *matched_name = g_match_info_fetch_named(match, "title");
	g_autofree char *matched_author = g_match_info_fetch_named(match, "author");
	if (matched_name == NULL || matched_author == NULL)
		return FALSE;

	if(name != NULL)
		*name = g_steal_pointer(&matched_name);
	if(author != NULL)
		*author = g_steal_pointer(&matched_author);
	if(version != NULL)
		*version = g_match_info_fetch_named(match, "version");

	return TRUE;
}

/* Helper function: read first line of file and return it. Free return value
when done. */
static char *
read_first_line(GFile *file, GCancellable *cancellable, GError **error)
{
	GFileInputStream *istream = g_file_read(file, cancellable, error);
	if(istream == NULL)
		return NULL;
	GDataInputStream *dstream = g_data_input_stream_new(G_INPUT_STREAM(istream));
	g_object_unref(istream);
	char *retval = g_data_input_stream_read_line_utf8(dstream, NULL, cancellable, error);
	g_object_unref(dstream); /* closes stream */
	return retval; /* is NULL if error reading */
}

/**
 * i7_app_install_extension:
 * @self: the application
 * @file: a #GFile
 *
 * Install the extension @file into the user's extensions directory, possibly
 * asking for confirmation to overwrite; or display an error dialog if an error
 * occurred.
 *
 * Returns: %true if successful, %false if the extension was not installed.
 */
bool
i7_app_install_extension(I7App *self, GFile *file)
{
	g_return_val_if_fail(file, false);
	GError *err = NULL;

	char *text = read_first_line(file, NULL, &err);
	if(text == NULL) {
		g_warning("Error reading extension: %s", err->message);
		g_error_free(err);
		return false;
	}

	/* Make sure the file is actually an Inform 7 extension */
	gchar *name = NULL;
	gchar *author = NULL;
	if (!is_valid_extension(text, NULL, &name, &author)) {
		char *display_name = file_get_display_name(file);
		error_dialog(NULL, NULL, _("The file '%s' does not seem to be an "
		  "extension. Extensions should be saved as UTF-8 text format files, "
		  "and should start with a line of one of these forms:\n\n<Extension> "
		  "by <Author> begins here.\nVersion <Version> of <Extension> by "
		  "<Author> begins here."), display_name);
		g_free(display_name);
		g_free(text);
		return false;
	}
	g_free(text);

	/* Turn off the file monitor */
	i7_app_stop_monitoring_extensions_directory(self);

	/* Create the directory for that author if it does not exist already */
	g_autoptr(GFile) dir = i7_app_get_extension_file(author, NULL);

	if(!make_directory_unless_exists(dir, NULL, &err)) {
		error_dialog_file_operation(NULL, dir, err, I7_FILE_ERROR_OTHER, _("creating a directory"));
		g_free(name);
		g_free(author);
		i7_app_monitor_extensions_directory(self);
		return false;
	}

	char *canonical_name = g_strconcat(name, ".i7x", NULL);
	GFile *target = g_file_get_child(dir, name);
	GFile *canonical_target = g_file_get_child(dir, canonical_name);
	g_free(canonical_name);

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
			i7_app_monitor_extensions_directory(self);
			return false;
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
		i7_app_monitor_extensions_directory(self);
		return false;
	}
	g_object_unref(canonical_target);

	/* If a version without *.i7x is still residing in that directory, delete
	it now */
	if(g_file_query_exists(target, NULL)) {
		if(!g_file_delete(target, NULL, &err))
			error_dialog_file_operation(NULL, target, err, I7_FILE_ERROR_OTHER, _("removing an old extension file"));
	}

	g_object_unref(target);
	i7_app_monitor_extensions_directory(self);

	/* Index the new extensions, in the foreground */
	i7_app_run_census(self, TRUE);

	return true;
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
i7_app_delete_extension(I7App *self, char *author, char *extname)
{
	GFile *file_lc, *file_noext, *file_lc_noext, *author_dir, *author_dir_lc;
	char *extname_lc;
	GError *err = NULL;

	i7_app_stop_monitoring_extensions_directory(self);

	/* Get references to the various possible versions of this filename */
	g_autoptr(GFile) file = i7_app_get_extension_file(author, extname);
	file_noext = remove_i7x_from_file(file);

	/* Remove extension, try versions with and without .i7x */
	if(!g_file_delete(file, NULL, &err) && !g_file_delete(file_noext, NULL, &err)) {
		error_dialog_file_operation(NULL, file, err, I7_FILE_ERROR_OTHER, _("deleting the file with or without extension"));
		g_object_unref(file_noext);
		goto finally;
	}
	g_object_unref(file_noext);

	/* Remove lowercase symlink to extension (holdover from previous versions
	of Inform) */
	extname_lc = g_utf8_strdown(extname, -1);
	file_lc = i7_app_get_extension_file(author, extname_lc);
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
	author_dir = i7_app_get_extension_file(author, NULL);
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
	author_dir_lc = i7_app_get_extension_file(author_lc, NULL);
	g_free(author_lc);
	/* Only do this if the symlink actually exists */
	if(file_exists_and_is_symlink(author_dir_lc)) {
		if(!g_file_delete(author_dir_lc, NULL, &err)) {
			error_dialog_file_operation(NULL, author_dir_lc, err, I7_FILE_ERROR_OTHER, _("deleting an old symlink"));
		}
	}
	g_object_unref(author_dir_lc);

finally:
	i7_app_monitor_extensions_directory(self);

	/* Index the new extensions, in the foreground */
	i7_app_run_census(self, TRUE);
}

/* Helper function: when a multiple extension download operation is cancelled,
cancel the single current download. */
static void
propagate_cancel(GCancellable *cancellable, GCancellable *inner_cancellable)
{
	g_cancellable_cancel(inner_cancellable);
}

/* Helper function: called when a download is taking too long. */
static gboolean
cancel_extension_download(GCancellable *inner_cancellable)
{
	g_cancellable_cancel(inner_cancellable);
	return G_SOURCE_REMOVE;
}

typedef struct {
	GCancellable *inner_cancellable;  /* owns ref */
	GFile *destination_file;  /* owns ref */
	unsigned cancel_handler;
	unsigned timeout_handler;
} DownloadExtensionClosure;

static void
download_extension_closure_free(DownloadExtensionClosure *data)
{
	g_clear_object(&data->inner_cancellable);
	g_clear_object(&data->destination_file);
	g_free(data);
}

static void on_download_extension_finished(GFile *remote_file, GAsyncResult *res, GTask *task);

/**
 * i7_app_download_extension_async:
 * @self: the app
 * @file: #GFile reference to a URI from which to download the extension
 * @cancellable: (allow-none): #GCancellable which will stop the operation, or
 * %NULL
 * @progress_callback: (nullable) (scope notified): function to call with
 *   progress information, or %NULL
 * @progress_callback_data: (closure progress_callback): user data to pass to
 *   @progress_callback, or %NULL
 * @finish_callback: (scope async): function to call when the operation finishes
 * @finish_callback_data: (closure finish_callback): user data to pass to
 *   @finish_callback, or %NULL
 *
 * Downloads an Inform 7 extension from @file and installs it.
 * The download will automatically be cancelled if it takes more than a
 * reasonable number of seconds (currently 15.)
 */
void
i7_app_download_extension_async(I7App *self, GFile *file, GCancellable *cancellable, GFileProgressCallback progress_callback, void *progress_callback_data, GAsyncReadyCallback finish_callback, void *finish_callback_data)
{
	GTask *task = g_task_new(self, cancellable, finish_callback, finish_callback_data);
	if (g_task_return_error_if_cancelled(task))
		return;

	DownloadExtensionClosure *task_data = g_new0(DownloadExtensionClosure, 1);
	g_task_set_task_data(task, task_data, (GDestroyNotify)download_extension_closure_free);

	/* Pick a location to download the file to */
	GFile *downloads_area = g_file_new_for_path(g_get_user_cache_dir());
	char *basename = g_file_get_basename(file);
	task_data->destination_file = g_file_get_child(downloads_area, basename);
	g_object_unref(downloads_area);
	g_free(basename);

	/* Break off the download after a suitable wait */
	task_data->inner_cancellable = g_cancellable_new();
	if(cancellable != NULL)
		task_data->cancel_handler = g_cancellable_connect(cancellable, G_CALLBACK(propagate_cancel), task_data->inner_cancellable, g_object_unref);
	task_data->timeout_handler = g_timeout_add_seconds(EXTENSION_DOWNLOAD_TIMEOUT_S, (GSourceFunc)cancel_extension_download, task_data->inner_cancellable);

	g_file_copy_async(file, task_data->destination_file, G_FILE_COPY_OVERWRITE, G_PRIORITY_DEFAULT, task_data->inner_cancellable,
		progress_callback, progress_callback_data,
		(GAsyncReadyCallback)on_download_extension_finished, task);
}

static void
on_download_extension_finished(GFile *remote_file, GAsyncResult *res, GTask *task)
{
	DownloadExtensionClosure *data = g_task_get_task_data(task);
	I7App *self = g_task_get_source_object(task);
	GCancellable *cancellable = g_task_get_cancellable(task);  /* unowned */

	g_source_remove(data->timeout_handler);
	if(cancellable != NULL)
		g_cancellable_disconnect(cancellable, data->cancel_handler);

	GError *err = NULL;
	if (!g_file_copy_finish(remote_file, res, &err)) {
		g_task_return_error(task, err);
		return;
	}

	bool success = i7_app_install_extension(self, data->destination_file);
	g_task_return_boolean(task, success);
}

/** 
 * i7_app_download_extension_finish:
 *
 * Note that the return value may be false even if @err is not set.
 * This happens when an error dialog has already been displayed to the user as
 * part of i7_app_install_extension().
 * TODO: This should be refactored in the future.
 */
bool
i7_app_download_extension_finish(I7App *self, GAsyncResult *res, GError **err)
{
	g_return_val_if_fail(g_task_is_valid(G_TASK(res), self), FALSE);
	return g_task_propagate_boolean(G_TASK(res), err);
}

/* Helper function: iterate over authors in installed extensions @store, to see
if @author is there. If so, return the node containing @author. Otherwise,
return a null pointer. */
GNode *
get_node_for_author(GNode *tree, const char *author)
{
	for (GNode *iter = g_node_first_child(tree); iter; iter = g_node_next_sibling(iter)) {
		I7InstalledExtensionAuthor *data = iter->data;
		g_autofree char *author_nocase = g_utf8_casefold(author, -1);
		g_autofree char *found_author_nocase = g_utf8_casefold(data->author_name, -1);
		if (strcmp(author_nocase, found_author_nocase) == 0)
			return iter;
	}
	return NULL;
}

/* Helper function: trim whitespace from string. Free return value when
 * done. Copied from deprecated pango_trim_string() function. */
static char *
trim(const char *string)
{
	while (*string && g_ascii_isspace(*string))
		string++;
	int len = strlen(string);
	while (len > 0 && g_ascii_isspace(string[len - 1]))
		len--;
	return g_strndup(string, len);
}

/* Helper function: remove "(for X only)" from extension title. Free return
value when done. */
static char *
remove_machine_spec_from_title(const char *title)
{
	if(!g_str_has_suffix(title, " only)"))
		return NULL;
	char *workstring = g_strdup(title);
	char *open_paren = strrchr(workstring, '(');
	if(open_paren == NULL) {
		g_free(workstring);
		return NULL;
	}
	*open_paren = '\0';
	char *retval = trim(workstring);
	g_free(workstring);
	return retval;
}

/* Helper function: iterate over extensions by author contained in @author_node
in installed extensions store, to see if @title is there. If so, return a node
containing @title Otherwise, return a null pointer. */
GNode *
get_node_for_extension_title(GNode *author_node, const char *title)
{
	for (GNode *iter = g_node_first_child(author_node); iter; iter = g_node_next_sibling(iter)) {
		I7InstalledExtension *data = iter->data;
		g_autofree char *title_nocase = g_utf8_casefold(title, -1);
		g_autofree char *found_title_nocase = g_utf8_casefold(data->title, -1);
		if (strcmp(title_nocase, found_title_nocase) == 0)
			return iter;

		if (g_str_has_prefix(title_nocase, found_title_nocase) || g_str_has_prefix(found_title_nocase, title_nocase)) {
			g_autofree char *canonical_title = remove_machine_spec_from_title(title_nocase);
			g_autofree char *canonical_found_title = remove_machine_spec_from_title(found_title_nocase);
			if (canonical_title != NULL || canonical_found_title != NULL) {
				if (canonical_title != NULL) {
					g_free(title_nocase);
					title_nocase = canonical_title;
				}
				if (canonical_found_title != NULL) {
					g_free(found_title_nocase);
					found_title_nocase = canonical_found_title;
				}
				if (strcmp(title_nocase, found_title_nocase) == 0)
					return iter;
			}
		}
	}
	return NULL;
}

/**
 * i7_app_get_extension_version:
 * @self: the app
 * @author: the author of the extension
 * @title: the title of the extension
 * @builtin: (allow-none): return location for a boolean
 *
 * Gets the version of the extension uniquely identified by "@title by @author".
 * The version may be the empty string if the extension does not identify itself
 * with a version.
 * If the extension is not installed, this function returns %NULL.
 *
 * Returns: (transfer full): a string representing the version of the extension,
 * or %NULL if the extension is not installed.
 */
char *
i7_app_get_extension_version(I7App *self, const char *author, const char *title, gboolean *builtin)
{
	GNode *author_node = get_node_for_author(self->installed_extensions, author);
	if (author_node == NULL)
		return NULL;
	GNode *ext_node = get_node_for_extension_title(author_node, title);
	if (!ext_node)
		return NULL;

	I7InstalledExtension *data = ext_node->data;
	if(builtin != NULL)
		*builtin = data->read_only;
	return g_strdup(data->version ? data->version : "");
}

/* Return the full path to the built-in Inform extension represented by @author
 and @extname, if not NULL; return the author directory if @extname is NULL;
 return the built-in extensions path if both are NULL. */
static GFile *
get_builtin_extension_file(I7App *self, const char *author,	const char *extname)
{
	if(!author)
		return i7_app_get_data_file_va(self, "Extensions", NULL);
	if(!extname)
		return i7_app_get_data_file_va(self, "Extensions", author, NULL);
	return i7_app_get_data_file_va(self, "Extensions", author, extname, NULL);
}

static GNode *add_author_to_tree(GFileInfo *info, GNode *tree);
static void add_extension_to_tree(bool builtin, GFile *parent, GFileInfo *info, GNode *author_node);

/**
 * iterate_and_add_installed_extensions:
 * @self: the app
 * @builtin: whether to iterate over the built-in extensions or the
 * user-installed ones
 *
 * Iterates over the installed extensions (the built-in ones if @builtin is
 * %true, or the user-installed ones if %false), adding each author directory
 * and each installed extension in each author directory to @store.
 */
static void
iterate_and_add_installed_extensions(I7App *self, bool builtin)
{
	GError *err = NULL;
	GFile *root_file;
	GFileEnumerator *root_dir;
	GFileInfo *author_info;

	if(builtin)
		root_file = get_builtin_extension_file(self, NULL, NULL);
	else
		root_file = i7_app_get_extension_file(NULL, NULL);

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

		GNode *author_result = add_author_to_tree(author_info, self->installed_extensions);

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

			add_extension_to_tree(builtin, author_file, extension_info, author_result);

			g_object_unref(extension_info);
		}

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
static GNode *
add_author_to_tree(GFileInfo *info, GNode *tree)
{
	const char *author_display_name = g_file_info_get_display_name(info);

	/* If the author directory was already indexed before, add the extension
	to it instead of making a new entry */
	GNode *author_node = get_node_for_author(tree, author_display_name);
	if (!author_node) {
		I7InstalledExtension *data = g_new0(I7InstalledExtension, 1);
		data->title = g_strdup(author_display_name);  /* Field 'author_name' is aliased */
		data->read_only = true;
		author_node = g_node_insert_data(tree, -1, data);
	}

	return author_node;
}

/* Helper function: add extension to tree store. Makes sure that user-installed
 * extensions override the built-in ones. */
static void
add_extension_to_tree(bool builtin, GFile *parent, GFileInfo *info, GNode *author_node)
{
	GError *error = NULL;
	const char *extension_name = g_file_info_get_name(info);
	GFile *extension_file = g_file_get_child(parent, extension_name);
	char *version, *title;

	char *firstline = read_first_line(extension_file, NULL, &error);
	if(firstline == NULL) {
		g_warning("Error reading extension file %s, skipping: %s", extension_name, error->message);
		g_error_free(error);
		goto finally;
	}
	if (!is_valid_extension(firstline, &version, &title, NULL)) {
		g_free(firstline);
		g_warning("Invalid extension file %s, skipping.", extension_name);
		goto finally;
	}
	g_free(firstline);

	/* Only add a built-in extension if it is not overridden by a user-installed
	 * extension */
	if (!builtin || !get_node_for_extension_title(author_node, title)) {
		I7InstalledExtension *data = g_new0(I7InstalledExtension, 1);
		data->title = g_strdup(title);
		data->version = g_strdup(version);
		data->read_only = builtin;
		data->file = g_object_ref(extension_file);
		g_node_insert_data(author_node, -1, data);
	}

	g_free(title);
	g_free(version);
finally:
	g_object_unref(extension_file);
}

/* Helper function: look in the user's extensions directory and the built-in one
 and list all the extensions there in the application's extensions tree */
static gboolean
update_installed_extensions_tree(I7App *self)
{
	free_installed_extensions_tree(self->installed_extensions);
	self->installed_extensions = g_node_new(NULL);

	iterate_and_add_installed_extensions(self, false);
	iterate_and_add_installed_extensions(self, true);

	/* Rebuild the Open Extension menus */
	i7_app_update_extensions_menu(self);

	return FALSE; /* one-shot idle function */
}

/* Start the compiler running the census of extensions. If @wait is FALSE, do it
 in the background. */
void
i7_app_run_census(I7App *self, gboolean wait)
{
	GFile *ni_binary = i7_app_get_binary_file(self, "inform7");
	GFile *builtin_extensions = i7_app_get_internal_dir(self);

	/* Build the command line */
	g_auto(GStrv) commandline = g_new(char *, 6);
	commandline[0] = g_file_get_path(ni_binary);
	commandline[1] = g_strdup("-internal");
	commandline[2] = g_file_get_path(builtin_extensions);
	commandline[3] = g_strdup("-census");
	commandline[4] = g_strdup("-silence");
	commandline[5] = NULL;

	g_object_unref(ni_binary);
	g_object_unref(builtin_extensions);

	/* Run the census */
	if(wait) {
		g_spawn_sync(g_get_home_dir(), commandline, NULL, G_SPAWN_SEARCH_PATH,
			NULL, NULL, NULL, NULL, NULL, NULL);
		update_installed_extensions_tree(self);
	} else {
		g_spawn_async(g_get_home_dir(), commandline, NULL, G_SPAWN_SEARCH_PATH,
			NULL, NULL, NULL, NULL);
		g_idle_add((GSourceFunc)update_installed_extensions_tree, self);
	}
}

/**
 * i7_app_get_extension_file:
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
i7_app_get_extension_file(const char *author, const char *extname)
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
 * i7_app_get_extension_home_page:
 *
 * Returns the home page for installed extensions (by default,
 * $HOME/Inform/Documentation/Extensions.html.)
 *
 * Returns: (transfer full): a new #GFile.
 */
GFile *
i7_app_get_extension_home_page(void)
{
	char *path = g_build_filename(g_get_home_dir(), EXTENSION_HOME_PATH, NULL);
	GFile *retval = g_file_new_for_path(path);
	g_free(path);
	return retval;
}

/**
 * i7_app_get_internal_dir:
 * @self: the app
 *
 * Gets a reference to the application data directory, or in other words the
 * directory which the Inform 7 compiler considers to be the "internal"
 * directory.
 *
 * Returns: (transfer full): a new #GFile.
 */
GFile *
i7_app_get_internal_dir(I7App *self)
{
	return g_object_ref(self->datadir);
}

static void *
missing_data_file(const char *filename)
{
	error_dialog(NULL, NULL, _("An application file, %s, was not found. "
		"Please reinstall Inform 7."), filename);
	return NULL;
}

/**
 * i7_app_get_retrospective_internal_dir:
 * @self: the app
 * @build: the build number of the retrospective, e.g. "6M62"
 *
 * Gets a reference to the directory which version @build of the Inform 7
 * compiler considers to be the "internal" directory.
 * If it is not found, displays an error dialog asking the user to reinstall the
 * application.
 *
 * Returns: (transfer full): a new #GFile.
 */
GFile *
i7_app_get_retrospective_internal_dir(I7App *self, const char *build)
{
    g_autoptr(GFile) dir1 = g_file_get_child(self->datadir, "retrospective");
    g_autoptr(GFile) dir2 = g_file_get_child(dir1, build);
    g_autoptr(GFile) retval = g_file_get_child(dir2, g_str_equal(build, "6L02") ? "Extensions" : "Internal");

    if (!g_file_query_exists(retval, NULL)) {
        g_autofree char *display_name = g_build_filename("retrospective", build, NULL);
        return missing_data_file(display_name);
    }

    return g_steal_pointer(&retval);
}

/**
 * i7_app_get_data_file:
 * @self: the app
 * @filename: the basename of the data file
 *
 * Locates @filename in the application data directory. If it is not found,
 * displays an error dialog asking the user to reinstall the application.
 *
 * Returns: (transfer full): a new #GFile pointing to @filename, or %NULL if not
 * found.
 */
GFile *
i7_app_get_data_file(I7App *self, const char *filename)
{
	GFile *retval = g_file_get_child(self->datadir, filename);

	if(g_file_query_exists(retval, NULL))
		return retval;

	g_object_unref(retval);
	return missing_data_file(filename);
}

/**
 * i7_app_get_data_file_va:
 * @self: the app
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
i7_app_get_data_file_va(I7App *self, const char *path1, ...)
{
	va_list ap;
	GFile *retval, *previous;
	char *arg, *lastarg = NULL;

	retval = previous = g_file_get_child(self->datadir, path1);

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
	return missing_data_file(lastarg);  /* argument before NULL */
}

/**
 * i7_app_get_binary_file:
 * @self: the app
 * @filename: the basename of the file
 *
 * Locates @filename in the application libexec directory. If it is not found,
 * displays an error dialog asking the user to reinstall the application.
 *
 * Returns: (transfer full): a new #GFile pointing to @filename, or %NULL if not
 * found.
 */
GFile *
i7_app_get_binary_file(I7App *self, const char *filename)
{
	GFile *retval = g_file_get_child(self->libexecdir, filename);

	if(g_file_query_exists(retval, NULL))
		return retval;

	g_object_unref(retval);
	return missing_data_file(filename);
}

/**
 * i7_app_get_retrospective_binary_file:
 * @self: the app
 * @build: the build number of the retrospective, e.g. "6M62"
 * @filename: the basename of the file, e.g. "cBlorb"
 *
 * Locates @filename in the retrospective section of the application libexec
 * directory.
 * If it is not found, displays an error dialog asking the user to reinstall the
 * application.
 *
 * Returns: (transfer full): a new #GFile pointing to @filename, or %NULL if not
 * found.
 */
GFile *
i7_app_get_retrospective_binary_file(I7App *self, const char *build, const char *filename)
{
	g_autoptr(GFile) dir1 = g_file_get_child(self->libexecdir, "retrospective");
	g_autoptr(GFile) dir2 = g_file_get_child(dir1, build);
	g_autoptr(GFile) retval = g_file_get_child(dir2, filename);

	if (!g_file_query_exists(retval, NULL)) {
		g_autofree char *display_name = g_build_filename("retrospective", build, filename, NULL);
		return missing_data_file(display_name);
	}

	return g_steal_pointer(&retval);
}

/**
 * i7_app_get_config_dir:
 *
 * Gets the location of the directory for user-specific configuration files.
 *
 * Returns: (transfer full): a #GFile pointing to the config dir
 */
GFile *
i7_app_get_config_dir(void)
{
	const char *config = g_get_user_config_dir();
	char *path = g_build_filename(config, "inform7", NULL);
	GFile *retval = g_file_new_for_path(path);
	g_free(path);
	return retval;
}

/* Getter function for installed extensions tree (transfer none). */
GNode *
i7_app_get_installed_extensions_tree(I7App *self)
{
	return self->installed_extensions;
}

/* Regenerate the installed extensions submenu */
void
i7_app_update_extensions_menu(I7App *self)
{
	GMenu *extensions_menu = gtk_application_get_menu_by_id(GTK_APPLICATION(self), "extensions");
	g_menu_remove_all(extensions_menu);

	g_autoptr(GIcon) builtin_emblem = g_themed_icon_new("com.inform7.IDE.builtin");

	for (GNode *author_iter = g_node_first_child(self->installed_extensions);
		author_iter != NULL;
		author_iter = g_node_next_sibling(author_iter)) {
		I7InstalledExtensionAuthor *author_data = author_iter->data;

		GNode *iter = g_node_first_child(author_iter);
		if (iter == NULL)
			continue;

		g_autoptr(GMenu) extmenu = g_menu_new();

		for (; iter != NULL; iter = g_node_next_sibling(iter)) {
			I7InstalledExtension *data = iter->data;
			g_autofree char *uri = g_file_get_uri(data->file);
			g_autoptr(GMenuItem) extitem = g_menu_item_new(data->title, NULL);
			if (data->read_only) {
				g_menu_item_set_icon(extitem, builtin_emblem);
				g_menu_item_set_action_and_target(extitem, "app.open-extension", "(sb)", uri, TRUE);
			} else {
				g_menu_item_set_action_and_target(extitem, "app.open-extension", "(sb)", uri, FALSE);
			}
			g_menu_append_item(extmenu, extitem);
		}

		g_menu_append_submenu(extensions_menu, author_data->author_name, G_MENU_MODEL(extmenu));
	}
}

/* Getter function for the global print settings object */
GtkPrintSettings *
i7_app_get_print_settings(I7App *self)
{
	return self->print_settings;
}

/* Setter function for the global print settings object */
void
i7_app_set_print_settings(I7App *self, GtkPrintSettings *settings)
{
	g_clear_object(&self->print_settings);
	self->print_settings = settings;
}

/*
 * i7_app_update_css:
 * @self: the app
 *
 * Update the global font settings CSS provider, so that any widgets with the
 * style classes "font-family-setting" or "font-size-setting" get their font
 * style updated.
 */
void
i7_app_update_css(I7App *self)
{
	g_autoptr(PangoFontDescription) desc = i7_app_get_document_font_description(self);
	const char *font_family = pango_font_description_get_family(desc);
	int font_size = pango_font_description_get_size(desc) / PANGO_SCALE;

	g_autofree char *css = g_strdup_printf(""
	    ".font-family-setting {"
	    "    font-family: '%s';"
	    "}"
	    ".font-size-setting {"
	    "    font-size: %dpt;"
	    "}", font_family, font_size);

	g_autoptr(GError) error = NULL;
	if (!gtk_css_provider_load_from_data(self->font_settings_provider, css, -1, &error))
		g_warning("Invalid CSS: %s", error->message);
}

/**
 * i7_app_get_last_opened_project:
 *
 * Looks for the story (not extension file) that was last opened.
 *
 * Returns: (allow-none) (transfer full): a #GFile pointing to the story, or
 * %NULL if there is no story in the history (e.g., if the application is newly
 * installed, or the recent documents history has been cleared.)
 */
GFile *
i7_app_get_last_opened_project(void)
{
	GtkRecentManager *manager = gtk_recent_manager_get_default();
	GList *recent = gtk_recent_manager_get_items(manager);
	GList *iter;
	time_t timestamp, latest = 0;
	GList *lastproject = NULL;
	GFile *retval = NULL;

	for(iter = recent; iter != NULL; iter = g_list_next(iter)) {
		GtkRecentInfo *info = gtk_recent_info_ref((GtkRecentInfo *)iter->data);
		if(gtk_recent_info_has_application(info, "Inform 7")
			&& gtk_recent_info_get_application_info(info, "Inform 7", NULL, NULL, &timestamp)
			&& gtk_recent_info_has_group(info, "inform7_project")
			&& (latest == 0 || difftime(timestamp, latest) > 0))
		{
			latest = timestamp;
			lastproject = iter;
		}
		gtk_recent_info_unref(info);
	}

	if(lastproject) {
		retval = g_file_new_for_uri(gtk_recent_info_get_uri((GtkRecentInfo *)lastproject->data));
		/* Do not free the string from gtk_recent_info_get_uri */
	}

	/* free the list */
	g_list_foreach(recent, (GFunc)gtk_recent_info_unref, NULL);
	g_list_free(recent);

	return retval;
}

/*
 * i7_app_get_system_settings:
 * @self: the application singleton
 *
 * Gets the #GSettings object for the system settings.
 *
 * Returns: (transfer none): #GSettings
 */
GSettings *
i7_app_get_system_settings(I7App *self)
{
	return self->system_settings;
}

/*
 * i7_app_get_prefs:
 * @self: the application singleton
 *
 * Gets the #GSettings object for the application preferences.
 *
 * Returns: #GSettings
 */
GSettings *
i7_app_get_prefs(I7App *self)
{
	return self->prefs_settings;
}

/*
 * i7_app_get_state:
 * @self: the application singleton
 *
 * Gets the #GSettings object for the application state that is saved between
 * runs.
 *
 * Returns: #GSettings
 */
GSettings *
i7_app_get_state(I7App *self)
{
	return self->state_settings;
}

/* Private method. For access to self->color_scheme_manager in app-colorscheme.c
 * so that files can stay separated by topic. */
GtkSourceStyleSchemeManager *
i7_app_get_color_scheme_manager(I7App *self)
{
	return self->color_scheme_manager;
}

/*
 * i7_app_get_document_font_string:
 * @self: the application singleton
 *
 * Returns: (transfer full): a string representing the font setting that can be
 *   understood by PangoFontDescription.
 */
char *
i7_app_get_document_font_string(I7App *self)
{
	if (g_settings_get_enum(self->prefs_settings, PREFS_FONT_SET) == FONT_CUSTOM)
		return g_settings_get_string(self->prefs_settings, PREFS_CUSTOM_FONT);
	return g_settings_get_string(self->system_settings, PREFS_SYSTEM_DOCUMENT_FONT);
}

/*
 * i7_app_get_document_font_description:
 * @self: the application singleton
 *
 * Returns: (transfer full): a PangoFontDescription representing the font
 *   setting.
 */
PangoFontDescription *
i7_app_get_document_font_description(I7App *self)
{
	g_autofree char *font = i7_app_get_document_font_string(self);
	return pango_font_description_from_string(font);
}

/*
 * i7_app_get_ui_font:
 * @self: the application singleton
 *
 * Returns: (transfer full): a string representing the font family used in the
 * UI, to set in WebViews used as part of the UI.
 */
char *
i7_app_get_ui_font(I7App *self)
{
	return g_settings_get_string(self->system_settings, PREFS_SYSTEM_UI_FONT);
}

/*
 * i7_app_get_docs_font_scale:
 * @self: the application singleton
 *
 * Returns: a relative font size (in ems) for the docs font size setting
 */
double
i7_app_get_docs_font_scale(I7App *self)
{
	switch (g_settings_get_enum(self->prefs_settings, PREFS_DOCS_FONT_SIZE)) {
		case FONT_SIZE_SMALLEST:
			return RELATIVE_SIZE_SMALLEST;
		case FONT_SIZE_SMALLER:
			return RELATIVE_SIZE_SMALLER;
		case FONT_SIZE_SMALL:
			return RELATIVE_SIZE_SMALL;
		case FONT_SIZE_MEDIUM:
			return RELATIVE_SIZE_MEDIUM;
		case FONT_SIZE_LARGE:
			return RELATIVE_SIZE_LARGE;
		case FONT_SIZE_LARGER:
			return RELATIVE_SIZE_LARGER;
		case FONT_SIZE_LARGEST:
			return RELATIVE_SIZE_LARGEST;
		default:
			return RELATIVE_SIZE_MEDIUM;
	}
}

bool
i7_app_is_valid_retrospective_id(I7App *self, const char *id)
{
	unsigned ix = 0;
	I7Retrospective *record;
	while ((record = g_list_model_get_item(G_LIST_MODEL(self->retrospectives), ix++)) != NULL) {
		const char *candidate = i7_retrospective_get_id(record);
		if (strcmp(candidate, id) == 0)
			return TRUE;
	}
	return FALSE;
}

GListStore *
i7_app_get_retrospectives(I7App *self)
{
	return self->retrospectives;
}
