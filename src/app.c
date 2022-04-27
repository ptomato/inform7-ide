/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2007-2015, 2019, 2021, 2022 Philip Chimento <philip.chimento@gmail.com>
 */

#include "config.h"

#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>

#include <gio/gio.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>

#include "app.h"
#include "actions.h"
#include "builder.h"
#include "configfile.h"
#include "error.h"
#include "file.h"
#include "lang.h"
#include "prefs.h"
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

typedef struct {
	/* Application directories */
	GFile *datadir;
	GFile *libexecdir;
	/* File monitor for extension directory */
	GFileMonitor *extension_dir_monitor;
	/* Tree of installed extensions */
	GtkTreeStore *installed_extensions;
	/* Current print settings */
	GtkPrintSettings *print_settings;
	/* Color scheme manager */
	GtkSourceStyleSchemeManager *color_scheme_manager;
	/* Preferences settings */
	GSettings *system_settings;
	GSettings *prefs_settings;
	GSettings *state_settings;
	GtkCssProvider *font_settings_provider;
} I7AppPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(I7App, i7_app, GTK_TYPE_APPLICATION);

typedef struct {
	gchar *regex;
	gboolean caseless;
} I7AppRegexInfo;

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
	GFile *config_dir = i7_app_get_config_dir(self);
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
		GtkRecentInfo *info = gtk_recent_info_ref(iter->data);
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

			g_autofree char *action = g_strdup_printf("app.open-recent(('%s','%s'))", gtk_recent_info_get_uri(info), group);
			GMenuItem *item = g_menu_item_new(gtk_recent_info_get_display_name(info), action);
			g_menu_append_item(recent_menu, item);
		}
		gtk_recent_info_unref(info);
	}
}

static void
i7_app_init(I7App *self)
{
	I7AppPrivate *priv = i7_app_get_instance_private(self);
	GError *error = NULL;

	priv->system_settings = g_settings_new(SCHEMA_SYSTEM);
	priv->prefs_settings = g_settings_new(SCHEMA_PREFERENCES);
	priv->state_settings = g_settings_new(SCHEMA_STATE);

	priv->font_settings_provider = gtk_css_provider_new();
	gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(priv->font_settings_provider),
		GTK_STYLE_PROVIDER_PRIORITY_USER);
	g_autoptr(GtkCssProvider) css = gtk_css_provider_new();
	gtk_css_provider_load_from_resource(css, "/com/inform7/IDE/ui/application.css");
	gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(css),
		GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

	/* Retrieve data directories if set externally */
	const gchar *env = g_getenv("INFORM7_IDE_DATA_DIR");
	if(env) {
		priv->datadir = g_file_new_for_path(env);
	} else {
		char *path = g_build_filename(PACKAGE_DATA_DIR, "inform7-ide", NULL);
		priv->datadir = g_file_new_for_path(path);
		g_free(path);
	}

	env = g_getenv("INFORM7_IDE_LIBEXEC_DIR");
	priv->libexecdir = g_file_new_for_path(env? env : PACKAGE_LIBEXEC_DIR);

	g_autoptr(GtkBuilder) builder = gtk_builder_new_from_resource("/com/inform7/IDE/ui/app.ui");
	gtk_builder_connect_signals(builder, self);

	create_app_actions(self);

	GtkRecentManager *default_recent_manager = gtk_recent_manager_get_default();
	g_signal_connect(default_recent_manager, "changed", G_CALLBACK(rebuild_recent_menu), self);

	priv->installed_extensions = GTK_TREE_STORE(load_object(builder, "installed_extensions_store"));
	g_object_ref(priv->installed_extensions);
	/* Set print settings to NULL, since they are not remembered across
	application runs (yet) */
	priv->print_settings = NULL;

	/* Create the Inform dir if it doesn't already exist */
	GFile *extensions_file = i7_app_get_extension_file(self, NULL, NULL);
	if(!make_directory_unless_exists(extensions_file, NULL, &error)) {
		IO_ERROR_DIALOG(NULL, extensions_file, error, _("creating the Inform directory"));
	}
	g_object_unref(extensions_file);

	/* Compile the regices */
	I7AppRegexInfo regex_info[] = {
		{ "^(?P<level>volume|book|part|chapter|section)\\s+(?P<secnum>.*?)(\\s+-\\s+(?P<sectitle>.*))?$", TRUE },
		{ "\\[=0x([0-9A-F]{4})=\\]", FALSE },
		{ "^\\s*(?:version\\s(?P<version>.+)\\sof\\s+)?(?:the\\s+)?" /* Version X of [the] */
		  "(?P<title>.+?)\\s+(?:\\(for\\s.+\\sonly\\)\\s+)?" /* <title> [(for X only)] */
		  "by\\s+(?P<author>.+)\\s+" /* by <author> */
		  "begins?\\s+here\\.?\\s*$", /* begins here[.] */
		  TRUE },
	};
	int i;
	for(i = 0; i < I7_APP_NUM_REGICES; i++) {
		self->regices[i] = g_regex_new(regex_info[i].regex, G_REGEX_OPTIMIZE | (regex_info[i].caseless? G_REGEX_CASELESS : 0), 0, &error);
		if(!self->regices[i])
			ERROR(_("Could not compile regex"), error);
	}

	self->prefs = create_prefs_window(priv->prefs_settings, builder);

	/* Set up signals for GSettings keys. */
	init_config_file(priv->prefs_settings);
	g_signal_connect_swapped(priv->system_settings, "changed::document-font-name", G_CALLBACK(i7_app_update_css), self);
	g_signal_connect_swapped(priv->system_settings, "changed::monospace-font-name", G_CALLBACK(i7_app_update_css), self);

	/* Create the color scheme manager (must be run after priv->datadir is set) */
	priv->color_scheme_manager = create_color_scheme_manager(self);
}

static void
i7_app_finalize(GObject *object)
{
	I7App *self = I7_APP(object);
	I7AppPrivate *priv = i7_app_get_instance_private(self);
	g_object_unref(priv->datadir);
	g_object_unref(priv->libexecdir);
	i7_app_stop_monitoring_extensions_directory(self);
	if(self->prefs)
		g_slice_free(I7PrefsWidgets, self->prefs);
	g_object_unref(priv->installed_extensions);
	g_object_unref(priv->color_scheme_manager);
	g_object_unref(priv->system_settings);
	g_object_unref(priv->state_settings);
	g_object_unref(priv->prefs_settings);
	g_clear_object(&priv->font_settings_provider);

	int i;
	for(i = 0; i < I7_APP_NUM_REGICES; i++)
		g_regex_unref(self->regices[i]);

	G_OBJECT_CLASS(i7_app_parent_class)->finalize(object);
}

static void
i7_app_startup(GApplication *app)
{
	I7App *self = I7_APP(app);
	I7AppPrivate *priv = i7_app_get_instance_private(self);

	G_APPLICATION_CLASS(i7_app_parent_class)->startup(app);

	/* Set up monitor for extensions directory */
	i7_app_run_census(self, FALSE);
	priv->extension_dir_monitor = NULL;
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
	I7App *theapp = I7_APP(g_object_new(I7_TYPE_APP,
		"application-id", "com.inform7.IDE",
		"flags", G_APPLICATION_HANDLES_OPEN,
		NULL));

	/* Do any setup activities for the application that require calling
	 g_application_get_default(), e.g. for access to GSettings */
	populate_schemes_list(theapp->prefs->schemes_list);
	/* Set up Natural Inform highlighting on the example buffer */
	GtkSourceBuffer *buffer = GTK_SOURCE_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(theapp->prefs->source_example)));
	set_buffer_language(buffer, "inform7");
	gtk_source_buffer_set_style_scheme(buffer, i7_app_get_current_color_scheme(theapp));

	return theapp;
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
	I7AppPrivate *priv = i7_app_get_instance_private(self);
	GError *error = NULL;
	if(!priv->extension_dir_monitor) {
		GFile *extdir = i7_app_get_extension_file(self, NULL, NULL);
		priv->extension_dir_monitor = g_file_monitor_directory(extdir, G_FILE_MONITOR_NONE, NULL, &error);
		g_object_unref(extdir);
	}

	g_signal_connect(G_OBJECT(priv->extension_dir_monitor), "changed", G_CALLBACK(extension_dir_changed), self);
}

/* Turn off the file monitor on the user's extensions directory */
void
i7_app_stop_monitoring_extensions_directory(I7App *self)
{
	I7AppPrivate *priv = i7_app_get_instance_private(self);
	if (priv->extension_dir_monitor) {
		g_file_monitor_cancel(priv->extension_dir_monitor);
		g_object_unref(priv->extension_dir_monitor);
		priv->extension_dir_monitor = NULL;
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
is_valid_extension(I7App *self, const char *text, char **version, char **name, char **author)
{
	g_return_val_if_fail(text != NULL, FALSE);

	GMatchInfo *match = NULL;
	char *matched_name, *matched_author;

	if(!g_regex_match(self->regices[I7_APP_REGEX_EXTENSION], text, 0, &match)) {
		g_match_info_free(match);
		return FALSE;
	}
	matched_name = g_match_info_fetch_named(match, "title");
	matched_author = g_match_info_fetch_named(match, "author");
	if(matched_name == NULL || matched_author == NULL) {
		g_free(matched_name);
		g_free(matched_author);
		g_match_info_free(match);
		return FALSE;
	}

	if(name != NULL)
		*name = matched_name;
	if(author != NULL)
		*author = matched_author;
	if(version != NULL)
		*version = g_match_info_fetch_named(match, "version");

	g_match_info_free(match);
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
 * Install the extension @file into the user's extensions directory.
 */
void
i7_app_install_extension(I7App *self, GFile *file)
{
	g_return_if_fail(file);
	GError *err = NULL;

	char *text = read_first_line(file, NULL, &err);
	if(text == NULL) {
		g_warning("Error reading extension: %s", err->message);
		g_error_free(err);
		return;
	}

	/* Make sure the file is actually an Inform 7 extension */
	gchar *name = NULL;
	gchar *author = NULL;
	if(!is_valid_extension(self, text, NULL, &name, &author)) {
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
	i7_app_stop_monitoring_extensions_directory(self);

	/* Create the directory for that author if it does not exist already */
	GFile *dir = i7_app_get_extension_file(self, author, NULL);

	if(!make_directory_unless_exists(dir, NULL, &err)) {
		error_dialog_file_operation(NULL, dir, err, I7_FILE_ERROR_OTHER, _("creating a directory"));
		g_free(name);
		g_free(author);
		g_object_unref(dir);
		i7_app_monitor_extensions_directory(self);
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
			i7_app_monitor_extensions_directory(self);
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
		i7_app_monitor_extensions_directory(self);
		return;
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
	GFile *file, *file_lc, *file_noext, *file_lc_noext, *author_dir, *author_dir_lc;
	char *extname_lc;
	GError *err = NULL;

	i7_app_stop_monitoring_extensions_directory(self);

	/* Get references to the various possible versions of this filename */
	file = i7_app_get_extension_file(self, author, extname);
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
	file_lc = i7_app_get_extension_file(self, author, extname_lc);
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
	author_dir = i7_app_get_extension_file(self, author, NULL);
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
	author_dir_lc = i7_app_get_extension_file(self, author_lc, NULL);
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

/**
 * i7_app_download_extension:
 * @self: the app
 * @file: #GFile reference to a URI from which to download the extension
 * @cancellable: (allow-none): #GCancellable which will stop the operation, or
 * %NULL
 * @progress_callback: (allow-none) (scope call): function to call with progress
 * information, or %NULL
 * @progress_callback_data: (closure): user data to pass to @progress_callback,
 * or %NULL
 * @error: return location for a #GError, or %NULL
 *
 * Downloads an Inform 7 extension from @file and installs it.
 * The download will automatically be cancelled if it takes more than a
 * reasonable number of seconds (currently 15.)
 *
 * Returns: %TRUE if the download succeeded, %FALSE if not, in which case
 * @error is set.
 */
gboolean
i7_app_download_extension(I7App *self, GFile *file, GCancellable *cancellable, GFileProgressCallback progress_callback, gpointer progress_callback_data, GError **error)
{
	if(g_cancellable_set_error_if_cancelled(cancellable, error))
		return FALSE;

	/* Pick a location to download the file to */
	GFile *downloads_area = g_file_new_for_path(g_get_user_cache_dir());
	char *basename = g_file_get_basename(file);
	GFile *destination_file = g_file_get_child(downloads_area, basename);
	g_object_unref(downloads_area);
	g_free(basename);

	/* Break off the download after a suitable wait */
	GCancellable *inner_cancellable = g_cancellable_new();
	unsigned cancel_handler, timeout_handler;
	if(cancellable != NULL)
		cancel_handler = g_cancellable_connect(cancellable, G_CALLBACK(propagate_cancel), inner_cancellable, g_object_unref);
	timeout_handler = g_timeout_add_seconds(EXTENSION_DOWNLOAD_TIMEOUT_S, (GSourceFunc)cancel_extension_download, inner_cancellable);

	gboolean success = g_file_copy(file, destination_file, G_FILE_COPY_OVERWRITE, inner_cancellable, progress_callback, progress_callback_data, error);

	g_source_remove(timeout_handler);
	if(cancellable != NULL)
		g_cancellable_disconnect(cancellable, cancel_handler);  /* unrefs inner_cancellable */

	if(!success)
		return FALSE;

	i7_app_install_extension(self, destination_file);
	return TRUE;
}

/* Helper function: iterate over authors in installed extensions @store, to see
if @author is there. If so, return a tree iter pointing to @author in @iter, and
return TRUE from function. Otherwise, return FALSE, and @iter is invalidated. */
gboolean
get_iter_for_author(GtkTreeModel *store, const char *author, GtkTreeIter *iter)
{
	gboolean found = FALSE;
	if(gtk_tree_model_get_iter_first(store, iter)) {
		do {
			char *found_author, *author_nocase, *found_author_nocase;
			gtk_tree_model_get(store, iter, I7_APP_EXTENSION_TEXT, &found_author, -1);
			author_nocase = g_utf8_casefold(author, -1);
			found_author_nocase = g_utf8_casefold(found_author, -1);
			g_free(found_author);
			if(strcmp(author_nocase, found_author_nocase) == 0)
				found = TRUE;
			g_free(author_nocase);
			g_free(found_author_nocase);
			if(found)
				break;
		} while(gtk_tree_model_iter_next(store, iter));
	}
	return found;
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

/* Helper function: iterate over extensions by author pointed to by @parent_iter
in installed extensions @store, to see if @title is there. If so, return a tree
iter pointing to @title in @iter, and return TRUE from function. Otherwise,
return FALSE, and @iter is invalidated. */
gboolean
get_iter_for_extension_title(GtkTreeModel *store, const char *title, GtkTreeIter *parent_iter, GtkTreeIter *iter)
{
	gboolean found = FALSE;
	if(gtk_tree_model_iter_children(store, iter, parent_iter)) {
		do {
			char *found_title, *title_nocase, *found_title_nocase;
			gtk_tree_model_get(store, iter, I7_APP_EXTENSION_TEXT, &found_title, -1);
			title_nocase = g_utf8_casefold(title, -1);
			found_title_nocase = g_utf8_casefold(found_title, -1);
			g_free(found_title);
			if(strcmp(title_nocase, found_title_nocase) == 0) {
				found = TRUE;
			} else if(g_str_has_prefix(title_nocase, found_title_nocase) || g_str_has_prefix(found_title_nocase, title_nocase)) {
				char *canonical_title = remove_machine_spec_from_title(title_nocase);
				char *canonical_found_title = remove_machine_spec_from_title(found_title_nocase);
				if(canonical_title != NULL || canonical_found_title != NULL) {
					if(canonical_title != NULL) {
						g_free(title_nocase);
						title_nocase = canonical_title;
					}
					if(canonical_found_title != NULL) {
						g_free(found_title_nocase);
						found_title_nocase = canonical_found_title;
					}
					if(strcmp(title_nocase, found_title_nocase) == 0)
						found = TRUE;
				}
			}
			g_free(title_nocase);
			g_free(found_title_nocase);
			if(found)
				break;
		} while(gtk_tree_model_iter_next(store, iter));
	}
	return found;
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
	I7AppPrivate *priv = i7_app_get_instance_private(self);
	GtkTreeModel *store = GTK_TREE_MODEL(priv->installed_extensions);
	GtkTreeIter parent_iter, child_iter;
	char *version;
	gboolean readonly;

	if(!get_iter_for_author(store, author, &parent_iter))
		return NULL;
	if(!get_iter_for_extension_title(store, title, &parent_iter, &child_iter))
		return NULL;

	gtk_tree_model_get(store, &child_iter,
	    I7_APP_EXTENSION_VERSION, &version,
	    I7_APP_EXTENSION_READ_ONLY, &readonly,
	    -1);
	if(builtin != NULL)
		*builtin = readonly;
	if(version == NULL)
		return g_strdup("");
	return version;
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

/**
 * i7_app_foreach_installed_extension:
 * @self: the app
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
i7_app_foreach_installed_extension(I7App *self, gboolean builtin, I7AppAuthorFunc author_func, void *author_func_data, I7AppExtensionFunc extension_func, void *extension_func_data, GDestroyNotify free_author_result)
{
	GError *err = NULL;
	GFile *root_file;
	GFileEnumerator *root_dir;
	GFileInfo *author_info;
	gpointer author_result;

	if(builtin)
		root_file = get_builtin_extension_file(self, NULL, NULL);
	else
		root_file = i7_app_get_extension_file(self, NULL, NULL);

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
			author_result = author_func(self, author_info, author_func_data);
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
				extension_func(self, author_file, extension_info, author_result, extension_func_data);

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
add_author_to_tree_store(I7App *app, GFileInfo *info, GtkTreeStore *store)
{
	GtkTreeIter parent_iter;
	const char *author_display_name = g_file_info_get_display_name(info);

	/* If the author directory was already indexed before, add the extension
	to it instead of making a new entry */
	if(!get_iter_for_author(GTK_TREE_MODEL(store), author_display_name, &parent_iter)) {
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
add_extension_to_tree_store(I7App *app, GFile *parent, GFileInfo *info, GtkTreeIter *parent_iter, GtkTreeStore *store)
{
	GError *error = NULL;
	const char *extension_name = g_file_info_get_name(info);
	GFile *extension_file = g_file_get_child(parent, extension_name);
	GtkTreeIter child_iter;
	char *version, *title;

	char *firstline = read_first_line(extension_file, NULL, &error);
	if(firstline == NULL) {
		g_warning("Error reading extension file %s, skipping: %s", extension_name, error->message);
		g_error_free(error);
		goto finally;
	}
	if(!is_valid_extension(app, firstline, &version, &title, NULL)) {
		g_free(firstline);
		g_warning("Invalid extension file %s, skipping.", extension_name);
		goto finally;
	}
	g_free(firstline);

	gtk_tree_store_append(store, &child_iter, parent_iter);
	gtk_tree_store_set(store, &child_iter,
		I7_APP_EXTENSION_TEXT, title, /* copies */
		I7_APP_EXTENSION_VERSION, version, /* copies */
		I7_APP_EXTENSION_READ_ONLY, FALSE,
		I7_APP_EXTENSION_ICON, NULL,
		I7_APP_EXTENSION_FILE, extension_file, /* references */
		-1);

	g_free(title);
	g_free(version);
finally:
	g_object_unref(extension_file);
}

/* Helper function: add extension to tree store as a built-in extension. Makes
 * sure that user-installed extensions override the built-in ones. */
static void
add_builtin_extension_to_tree_store(I7App *app, GFile *parent, GFileInfo *info, GtkTreeIter *parent_iter, GtkTreeStore *store)
{
	GError *error = NULL;
	const char *extension_name = g_file_info_get_name(info);
	GFile *extension_file = g_file_get_child(parent, extension_name);
	GtkTreeIter child_iter;
	char *version, *title;

	char *firstline = read_first_line(extension_file, NULL, &error);
	if(firstline == NULL) {
		g_warning("Error reading extension file %s, skipping: %s", extension_name, error->message);
		g_error_free(error);
		goto finally;
	}
	if(!is_valid_extension(app, firstline, &version, &title, NULL)) {
		g_free(firstline);
		g_warning("Invalid extension file %s, skipping.", extension_name);
		goto finally;
	}
	g_free(firstline);

	/* Only add it if it is not overridden by a user-installed extension */
	if(!get_iter_for_extension_title(GTK_TREE_MODEL(store), title, parent_iter, &child_iter)) {
		gtk_tree_store_append(store, &child_iter, parent_iter);
		gtk_tree_store_set(store, &child_iter,
			I7_APP_EXTENSION_TEXT, title,
			I7_APP_EXTENSION_VERSION, version,
			I7_APP_EXTENSION_READ_ONLY, TRUE,
			I7_APP_EXTENSION_ICON, "com.inform7.IDE.builtin",
			I7_APP_EXTENSION_FILE, extension_file,
			-1);
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
	I7AppPrivate *priv = i7_app_get_instance_private(self);
	GtkTreeStore *store = priv->installed_extensions;
	gtk_tree_store_clear(store);

	i7_app_foreach_installed_extension(self, FALSE,
	    (I7AppAuthorFunc)add_author_to_tree_store, store,
	    (I7AppExtensionFunc)add_extension_to_tree_store, store,
	    (GDestroyNotify)gtk_tree_iter_free);
	i7_app_foreach_installed_extension(self, TRUE,
	    (I7AppAuthorFunc)add_author_to_tree_store, store,
	    (I7AppExtensionFunc)add_builtin_extension_to_tree_store, store,
	    (GDestroyNotify)gtk_tree_iter_free);

	/* Rebuild the Open Extension menus */
	i7_app_update_extensions_menu(self);

	return FALSE; /* one-shot idle function */
}

/* Start the compiler running the census of extensions. If @wait is FALSE, do it
 in the background. */
void
i7_app_run_census(I7App *self, gboolean wait)
{
	GFile *ni_binary = i7_app_get_binary_file(self, "ni");
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
 * @self: the application
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
i7_app_get_extension_file(I7App *self, const char *author, const char *extname)
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
 * i7_app_get_extension_docpage:
 * @self: the application
 * @author: (allow-none): the extension author
 * @extname: (allow-none): the extension name, without .i7x
 *
 * Returns the documentation page for the extension @extname by @author.
 * If both are %NULL, returns the directory where extension documentation is
 * stored.
 * Does not check whether the file exists.
 *
 * Returns: (transfer full): a new #GFile.
 */
GFile *
i7_app_get_extension_docpage(I7App *self, const char *author, const char *extname)
{
	char *path;

	if(author == NULL)
		path = g_build_filename(g_get_home_dir(), EXTENSION_DOCS_BASE_PATH, NULL);
	else if(extname == NULL)
		path = g_build_filename(g_get_home_dir(), EXTENSION_DOCS_BASE_PATH, author, NULL);
	else
		path = g_build_filename(g_get_home_dir(), EXTENSION_DOCS_BASE_PATH, author, extname, NULL);

	GFile *retval = g_file_new_for_path(path);
	g_free(path);
	return retval;
}

/**
 * i7_app_get_extension_home_page:
 * @se;f: the application
 *
 * Returns the home page for installed extensions (by default,
 * $HOME/Inform/Documentation/Extensions.html.)
 *
 * Returns: (transfer full): a new #GFile.
 */
GFile *
i7_app_get_extension_home_page(I7App *self)
{
	char *path = g_build_filename(g_get_home_dir(), EXTENSION_HOME_PATH, NULL);
	GFile *retval = g_file_new_for_path(path);
	g_free(path);
	return retval;
}

/**
 * i7_app_get_extension_index_page:
 * @self: the application
 *
 * Returns the definitions index page for installed extensions (by default,
 * $HOME/Inform/Documentation/ExtIndex.html.)
 *
 * Returns: (transfer full): a new #GFile.
 */
GFile *
i7_app_get_extension_index_page(I7App *self)
{
	char *path = g_build_filename(g_get_home_dir(), EXTENSION_INDEX_PATH, NULL);
	GFile *retval = g_file_new_for_path(path);
	g_free(path);
	return retval;
}

/**
 * i7_app_get_internal_dir:
 * @self: the app
 *
 * Gets a reference to the application data directory, or in other words the
 * directory which the NI compiler considers to be the "internal" directory.
 *
 * Returns: (transfer full): a new #GFile.
 */
GFile *
i7_app_get_internal_dir(I7App *self)
{
	I7AppPrivate *priv = i7_app_get_instance_private(self);
	return g_object_ref(priv->datadir);
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
	I7AppPrivate *priv = i7_app_get_instance_private(self);
	GFile *retval = g_file_get_child(priv->datadir, filename);

	if(g_file_query_exists(retval, NULL))
		return retval;

	g_object_unref(retval);
	error_dialog(NULL, NULL, _("An application file, %s, was not found. "
		"Please reinstall Inform 7."), filename);
	return NULL;
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
	I7AppPrivate *priv = i7_app_get_instance_private(self);

	retval = previous = g_file_get_child(priv->datadir, path1);

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
	I7AppPrivate *priv = i7_app_get_instance_private(self);
	GFile *retval = g_file_get_child(priv->libexecdir, filename);

	if(g_file_query_exists(retval, NULL))
		return retval;

	g_object_unref(retval);
	error_dialog(NULL, NULL, _("An application file, %s, was not found. "
		"Please reinstall Inform 7."), filename);
	return NULL;
}

/**
 * i7_app_get_config_dir:
 * @self: the app
 *
 * Gets the location of the directory for user-specific configuration files.
 *
 * Returns: (transfer full): a #GFile pointing to the config dir
 */
GFile *
i7_app_get_config_dir(I7App *self)
{
	const char *config = g_get_user_config_dir();
	char *path = g_build_filename(config, "inform7", NULL);
	GFile *retval = g_file_new_for_path(path);
	g_free(path);
	return retval;
}

/* Getter function for installed extensions tree (transfer none). */
GtkTreeStore *
i7_app_get_installed_extensions_tree(I7App *self)
{
	I7AppPrivate *priv = i7_app_get_instance_private(self);
	return priv->installed_extensions;
}

/* Regenerate the installed extensions submenu */
void
i7_app_update_extensions_menu(I7App *self)
{
	I7AppPrivate *priv = i7_app_get_instance_private(self);
	GtkTreeModel *model = GTK_TREE_MODEL(priv->installed_extensions);
	GtkTreeIter author, title;
	GMenu *extensions_menu = gtk_application_get_menu_by_id(GTK_APPLICATION(self), "extensions");
	g_menu_remove_all(extensions_menu);

	g_autoptr(GIcon) builtin_emblem = g_themed_icon_new("com.inform7.IDE.builtin");

	gtk_tree_model_get_iter_first(model, &author);
	do {
		gchar *authorname;
		gtk_tree_model_get(model, &author, I7_APP_EXTENSION_TEXT, &authorname, -1);

		if(gtk_tree_model_iter_children(model, &title, &author))
		{
			GMenu *extmenu = g_menu_new();
			do {
				char *extname;
				GFile *extension_file;
				gboolean readonly;
				gtk_tree_model_get(model, &title,
					I7_APP_EXTENSION_TEXT, &extname,
					I7_APP_EXTENSION_READ_ONLY, &readonly,
					I7_APP_EXTENSION_FILE, &extension_file,
					-1);
				g_autofree char *uri = g_file_get_uri(extension_file);
				GMenuItem *extitem = g_menu_item_new(extname, NULL);
				if(readonly) {
					g_menu_item_set_icon(extitem, builtin_emblem);
					g_menu_item_set_action_and_target(extitem, "app.open-extension", "(sb)", uri, TRUE);
				} else {
					g_menu_item_set_action_and_target(extitem, "app.open-extension", "(sb)", uri, FALSE);
				}
				g_menu_append_item(extmenu, extitem);

				g_free(extname);
				g_object_unref(extension_file);

			} while(gtk_tree_model_iter_next(model, &title));
			g_menu_append_submenu(extensions_menu, authorname, G_MENU_MODEL(extmenu));
		}
	} while(gtk_tree_model_iter_next(model, &author));
}

/* Getter function for the global print settings object */
GtkPrintSettings *
i7_app_get_print_settings(I7App *self)
{
	I7AppPrivate *priv = i7_app_get_instance_private(self);
	return priv->print_settings;
}

/* Setter function for the global print settings object */
void
i7_app_set_print_settings(I7App *self, GtkPrintSettings *settings)
{
	I7AppPrivate *priv = i7_app_get_instance_private(self);
	if(priv->print_settings)
		g_object_unref(priv->print_settings);
	priv->print_settings = settings;
}

/* Bring the preferences dialog to the front */
void
i7_app_present_prefs_window(I7App *self)
{
	gtk_window_present(GTK_WINDOW(self->prefs->window));
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
	I7AppPrivate *priv = i7_app_get_instance_private(self);

	g_autofree char *font_family = i7_app_get_font_family(self);
	double font_size = i7_app_get_font_scale(self);
	g_autofree char *css = g_strdup_printf(""
	    ".font-family-setting {"
	    "    font-family: '%s';"
	    "}"
	    ".font-size-setting {"
	    "    font-size: %.1fem;"
	    "}", font_family, font_size);

	g_autoptr(GError) error = NULL;
	if (!gtk_css_provider_load_from_data(priv->font_settings_provider, css, -1, &error))
		g_warning("Invalid CSS: %s", error->message);
}

/**
 * i7_app_get_last_opened_project:
 * @self: the app
 *
 * Looks for the story (not extension file) that was last opened.
 *
 * Returns: (allow-none) (transfer full): a #GFile pointing to the story, or
 * %NULL if there is no story in the history (e.g., if the application is newly
 * installed, or the recent documents history has been cleared.)
 */
GFile *
i7_app_get_last_opened_project(I7App *self)
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
	I7AppPrivate *priv = i7_app_get_instance_private(self);
	return priv->system_settings;
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
	I7AppPrivate *priv = i7_app_get_instance_private(self);
	return priv->prefs_settings;
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
	I7AppPrivate *priv = i7_app_get_instance_private(self);
	return priv->state_settings;
}

/* Private method. For access to priv->color_scheme_manager in app-colorscheme.c
 * so that files can stay separated by topic. */
GtkSourceStyleSchemeManager *
i7_app_get_color_scheme_manager(I7App *self)
{
	I7AppPrivate *priv = i7_app_get_instance_private(self);
	return priv->color_scheme_manager;
}

/* modifies string in place */
static void
remove_font_size(char *font) {
	char *ptr = strrchr(font, ' ');
	if (ptr)
		*ptr = '\0';
}

/*
 * i7_app_get_font_family:
 * @self: the application singleton
 *
 * Returns: (transfer full): a string representing the font setting suitable to
 *   use in CSS.
 */
char *
i7_app_get_font_family(I7App *self)
{
	I7AppPrivate *priv = i7_app_get_instance_private(self);

	char *font;
	switch(g_settings_get_enum(priv->prefs_settings, PREFS_FONT_SET)) {
		case FONT_MONOSPACE:
			font = g_settings_get_string(priv->system_settings, PREFS_SYSTEM_MONOSPACE_FONT);
			break;
		case FONT_CUSTOM:
			font = g_settings_get_string(priv->prefs_settings, PREFS_CUSTOM_FONT);
			break;
		default:
			font = g_settings_get_string(priv->system_settings, PREFS_SYSTEM_DOCUMENT_FONT);
	}
	remove_font_size(font);
	return font;
}

/*
 * i7_app_get_font_scale:
 * @self: the application singleton
 *
 * Returns: a relative font size (in ems) for the font size setting
 */
double
i7_app_get_font_scale(I7App *self)
{
	I7AppPrivate *priv = i7_app_get_instance_private(self);

	switch(g_settings_get_enum(priv->prefs_settings, PREFS_FONT_SIZE)) {
		case FONT_SIZE_MEDIUM:
			return RELATIVE_SIZE_MEDIUM;
		case FONT_SIZE_LARGE:
			return RELATIVE_SIZE_LARGE;
		case FONT_SIZE_HUGE:
			return RELATIVE_SIZE_HUGE;
		default:
			return RELATIVE_SIZE_STANDARD;
	}
}

/*
 * i7_app_get_font_size:
 * @self: the application singleton
 *
 * Returns: an absolute font size for the font size setting in webviews and the
 *   skein
 */
double
i7_app_get_font_size(I7App *self)
{
	I7AppPrivate *priv = i7_app_get_instance_private(self);

	g_autofree char* font = g_settings_get_string(priv->system_settings, PREFS_SYSTEM_DOCUMENT_FONT);
	const char* ptr = strrchr(font, ' ');

	int base = 0;
	if (ptr)
		base = atoi(ptr);
	if (base < 1)
		base = DEFAULT_SIZE_STANDARD;

	return base * i7_app_get_font_scale(self);
}
