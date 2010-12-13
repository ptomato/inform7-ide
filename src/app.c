/*  Copyright 2007 P.F. Chimento
 *  This file is part of GNOME Inform 7.
 *
 *  GNOME Inform 7 is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  GNOME Inform 7 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNOME Inform 7; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
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
	priv->datadir = env? g_strdup(env) : g_build_filename(PACKAGE_DATA_DIR, "gnome-inform7", NULL);
	env = g_getenv("GNOME_INFORM_PIXMAP_DIR");
	priv->pixmapdir = env? g_strdup(env) : g_build_filename(PACKAGE_DATA_DIR, "pixmaps", "gnome-inform7", NULL);
	env = g_getenv("GNOME_INFORM_LIBEXEC_DIR");
	priv->libexecdir = env? g_strdup(env) : g_strdup(PACKAGE_LIBEXEC_DIR);
	
	gchar *builderfilename = i7_app_get_datafile_path(self, "ui/gnome-inform7.ui");
	GtkBuilder *builder = create_new_builder(builderfilename, self);
	g_free(builderfilename);

	/* Make the action groups. This for-loop is a temporary fix
	and can be removed once Glade supports adding actions and accelerators to an
	action group. */
	const gchar *actions[] = { 
		"file_menu", "",
		"edit_menu", "",
		"help_menu", "",
		"new", NULL, /* NULL means use the stock accelerator */
		"open", NULL,
		"open_recent", "",
		"install_extension", "",
		"open_extension", "",
		"preferences", "",
		"quit", NULL,
		"visit_inform7_com", "",
		"suggest_feature", "",
		"report_bug", "",
		"about", "",
		NULL
	};
	add_actions(builder, &(priv->app_action_group), "app_actions", actions);
	g_object_ref(priv->app_action_group);
	
	/* Add a filter to the Open Recent menu (can be removed once Glade supports
	building GtkRecentFilters */
	GtkAction *recent = GTK_ACTION(load_object(builder, "open_recent"));
    GtkRecentFilter *filter = gtk_recent_filter_new();
    gtk_recent_filter_add_application(filter, "GNOME Inform 7");
    gtk_recent_chooser_set_filter(GTK_RECENT_CHOOSER(recent), filter);
	
	priv->document_list = NULL;
	priv->installed_extensions = GTK_TREE_STORE(load_object(builder, "installed_extensions_store"));
	g_object_ref(priv->installed_extensions);
	/* Set print settings to NULL, since they are not remembered across
	application runs (yet) */
	priv->print_settings = NULL;
	priv->page_setup = NULL;
	
	/* Create the Gnome Inform7 dir if it doesn't already exist */
    gchar *extensions_dir = i7_app_get_extension_path(self, NULL, NULL);
    g_mkdir_with_parents(extensions_dir, 0777);
    g_free(extensions_dir);
		
	/* Add icons to theme */
	GtkIconTheme *theme = gtk_icon_theme_get_default();
	gtk_icon_theme_append_search_path(theme, priv->pixmapdir);
	
	/* Set up monitor for extensions directory */
	i7_app_run_census(self, FALSE);
	priv->extension_dir_monitor = NULL;
	i7_app_monitor_extensions_directory(self);
	
	/* Compile the regices */
	I7AppRegexInfo regex_info[] = {
		{ "^(?P<level>volume|book|part|chapter|section)\\s+(?P<secnum>.*?)(\\s+-\\s+(?P<sectitle>.*))?$", TRUE },
		{ "\\[=0x([0-9A-F]{4})=\\]", FALSE },
		{ "src=(['\"]?)inform:/(.*?\\.png)\\1(\\s|>)", FALSE },
		{ "^\\s*(version\\s.+\\sof\\s+)?(the\\s+)?(?P<title>.+)\\s+by\\s+(?P<author>.+)\\s+begins?\\s+here\\.?\\s*\\n", TRUE },
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
	g_free(priv->datadir);
	g_free(priv->pixmapdir);
	g_free(priv->libexecdir);
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
i7_app_open(I7App *app, const gchar *filename)
{
	gchar *fullpath = expand_initial_tilde(filename);
	I7Document *dupl = i7_app_get_already_open(app, fullpath);
	if(dupl) {
		gtk_window_present(GTK_WINDOW(dupl));
		g_free(fullpath);
		return;
	}
	
	if(g_file_test(fullpath, G_FILE_TEST_EXISTS)) {
		if(g_file_test(fullpath, G_FILE_TEST_IS_DIR))
			i7_story_new_from_file(app, fullpath);
			/* TODO make sure story.ni exists */
		/* else */
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
static gint
document_compare_name(const I7Document *document, const gchar *filename)
{
	gchar *name = i7_document_get_path(document);
	gint retval = strcmp(name, filename);
	g_free(name);
	return retval;
}

/* Check to see if @filename is already open in this instance of the
 application. Return the corresponding I7Document object if so, or NULL
 otherwise. */
I7Document *
i7_app_get_already_open(I7App *app, const gchar *filename)
{
	I7_APP_USE_PRIVATE(app, priv);
	GSList *node = g_slist_find_custom(priv->document_list, filename, (GCompareFunc)document_compare_name);
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
		gchar *extpath = i7_app_get_extension_path(app, NULL, NULL);
		GFile *extdir = g_file_new_for_path(extpath);
		g_free(extpath);
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

/* Install the extension at @filename into the user's extensions dir */
void 
i7_app_install_extension(I7App *app, const gchar *filename) 
{
	g_return_if_fail(filename != NULL);
	GError *err = NULL;
	
	gchar *text = read_source_file(filename);
	if(!text)
		return;
	
	/* Make sure the file is actually an Inform 7 extension */
	gchar *name = NULL;
	gchar *author = NULL;
	if(!is_valid_extension(app, text, &name, &author)) {
		error_dialog(NULL, NULL, _("The file '%s' does not seem to be an "
		  "extension. Extensions should be saved as UTF-8 text format files, "
		  "and should start with a line of one of these forms:\n\n<Extension> "
		  "by <Author> begins here.\nVersion <Version> of <Extension> by "
		  "<Author> begins here."), filename);
		g_free(text);
		return;
	}
	
	/* Turn off the file monitor */
	i7_app_stop_monitoring_extensions_directory(app);
	
	/* Create the directory for that author if it does not exist already */
	gchar *dir = i7_app_get_extension_path(app, author, NULL);
	
	if(!g_file_test(dir, G_FILE_TEST_EXISTS)) {
		if(g_mkdir_with_parents(dir, 0777) == -1) {
			error_dialog(NULL, NULL, _("Error creating directory '%s'."), dir);
			g_free(name);
			g_free(author);
			g_free(dir);
			g_free(text);
			i7_app_monitor_extensions_directory(app);
			return;
		}
	}
	
	gchar *targetname = g_build_filename(dir, name, NULL);
	gchar *canonicaltarget = g_strconcat(targetname, ".i7x", NULL);
	g_free(dir);

	/* Check if the extension is already installed */
	if(g_file_test(targetname, G_FILE_TEST_EXISTS) || g_file_test(canonicaltarget, G_FILE_TEST_EXISTS)) 
	{
		GtkWidget *dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
		  _("A version of the extension %s by %s is already installed. Do you "
		  "want to overwrite the installed extension with this new one?"), name, author);
		if(gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_YES) {
			gtk_widget_destroy(dialog);
			g_free(targetname);
			g_free(canonicaltarget);
			g_free(name);
			g_free(author);
			g_free(text);
			i7_app_monitor_extensions_directory(app);
			return;
		}
		gtk_widget_destroy(dialog);
	}
	
	/* Copy the extension file to the user's extensions dir */
	if(!g_file_set_contents(canonicaltarget, text, -1, &err)) {
		error_dialog(NULL, NULL, _("Error copying file '%s' to '%s': "), filename, canonicaltarget);
		g_free(text);
		g_free(targetname);
		g_free(canonicaltarget);
		g_free(name);
		g_free(author);
		i7_app_monitor_extensions_directory(app);
		return;
	}
	
	g_free(text);
	g_free(canonicaltarget);
	g_free(name);
	g_free(author);
	
	/* If a version without *.i7x is still residing in that directory, delete
	it now */
	if(g_file_test(targetname, G_FILE_TEST_EXISTS)) {
		if(g_remove(targetname) == -1)
			error_dialog(NULL, NULL, _("There was an error removing the old file '%s'."), targetname);
	}
	g_free(targetname);
	i7_app_monitor_extensions_directory(app);
	
	/* Index the new extensions, in the foreground */
	i7_app_run_census(app, TRUE);
}

/* Delete extension and remove author dir if empty */
void 
i7_app_delete_extension(I7App *app, gchar *author, gchar *extname) 
{
	i7_app_stop_monitoring_extensions_directory(app);
	
	/* Remove extension, try versions with and without .i7x */
	gchar *filename = i7_app_get_extension_path(app, author, extname);
	gchar *canonicalname = g_strconcat(filename, ".i7x", NULL);
	if(g_remove(filename) == -1 && g_remove(canonicalname) == -1) {
		error_dialog(NULL, NULL, _("There was an error removing %s."), filename);
		g_free(filename);
		i7_app_monitor_extensions_directory(app);
		return;
	}
	g_free(filename);
	
	/* Remove lowercase symlink to extension (holdover from previous versions
	of Inform) */
	gchar *extname_lc = g_utf8_strdown(extname, -1);
	filename = i7_app_get_extension_path(app, author, extname_lc);
	g_free(extname_lc);
	/* Only do this if the symlink actually exists */
	if(g_file_test(filename, G_FILE_TEST_IS_SYMLINK)) {
		if(g_remove(filename) == -1) {
			error_dialog(NULL, NULL, _("There was an error removing %s."), filename);
			g_free(filename);
			i7_app_monitor_extensions_directory(app);
			return;
		}
	}
	g_free(filename);
	
	/* Remove author directory if empty */
	filename = i7_app_get_extension_path(app, author, NULL); 
	if(g_rmdir(filename) == -1) {
		/* if there were other extensions, continue; but if it failed for any
		other reason, display an error */
		if(errno != ENOTEMPTY) {
			error_dialog(NULL, NULL, _("There was an error removing %s."), filename);
			g_free(filename);
			i7_app_monitor_extensions_directory(app);
			return;
		}
	}
	g_free(filename);
	
	/* Remove lowercase symlink to author directory (holdover from previous 
	versions of Inform) */
	gchar *author_lc = g_utf8_strdown(author, -1);
	filename = i7_app_get_extension_path(app, author_lc, NULL);
	g_free(author_lc);
	/* Only do this if the symlink actually exists */
	if(g_file_test(filename, G_FILE_TEST_IS_SYMLINK)) {
		if(g_remove(filename) == -1)
			error_dialog(NULL, NULL, _("There was an error removing %s."), filename);
	}
	g_free(filename);
	
	i7_app_monitor_extensions_directory(app);
	
	/* Index the new extensions, in the foreground */
	i7_app_run_census(app, TRUE);
}

/* Return the full path to the built-in Inform extension represented by @author
 and @extname, if not NULL; return the author directory if @extname is NULL;
 return the built-in extensions path if both are NULL. */
static gchar *
get_builtin_extension_path(I7App *app, const gchar *author, 
	const gchar *extname)
{
	if(!author)
        return i7_app_get_datafile_path_va(app, "Extensions", NULL);
    if(!extname)
        return i7_app_get_datafile_path_va(app, "Extensions", author, NULL);
    return i7_app_get_datafile_path_va(app, "Extensions", author, extname, NULL);
}

/* Look in the user's extensions directory and list all the extensions there in
the application's extensions tree */
static gboolean
update_installed_extensions_tree(I7App *app)
{
	GError *err = NULL;
	
	GtkTreeStore *store = I7_APP_PRIVATE(app)->installed_extensions;
	gtk_tree_store_clear(store);
	GtkTreeIter parent_iter, child_iter;
	
	gchar *extension_dir = i7_app_get_extension_path(app, NULL, NULL);
	GDir *extensions = g_dir_open(extension_dir, 0, &err);
	g_free(extension_dir);
	if(err) {
		error_dialog(NULL, err, _("Error opening extensions directory: "));
		return FALSE;
	}

	const gchar *dir_entry;
	while((dir_entry = g_dir_read_name(extensions)) != NULL) {
		if(!strcmp(dir_entry, "Reserved"))
			continue;
		gchar *author_dir = i7_app_get_extension_path(app, dir_entry, NULL);
		if(g_file_test(author_dir, G_FILE_TEST_IS_SYMLINK) || !g_file_test(author_dir, G_FILE_TEST_IS_DIR)) 
		{
			g_free(author_dir);
			continue;
		}
		/* Read each extension dir, but skip "Reserved", symlinks and nondirs*/
		gtk_tree_store_append(store, &parent_iter, NULL);
		gtk_tree_store_set(store, &parent_iter, 
			I7_APP_EXTENSION_TEXT, dir_entry, 
			I7_APP_EXTENSION_READ_ONLY, TRUE,
			I7_APP_EXTENSION_ICON, NULL, 
			I7_APP_EXTENSION_PATH, NULL, 
			-1);
		
		/* Descend into each author directory */
		GDir *author = g_dir_open(author_dir, 0, &err);
		g_free(author_dir);
		if(err) {
			error_dialog(NULL, err, _("Error opening extensions directory: "));
			return FALSE;
		}
		const gchar *author_entry;
		while((author_entry = g_dir_read_name(author)) != NULL) {
			gchar *extname = i7_app_get_extension_path(app, dir_entry, author_entry);
			if(g_file_test(extname, G_FILE_TEST_IS_SYMLINK)) {
				g_free(extname);
				continue;
			}
			/* Read each file, but skip symlinks */
			/* Remove .i7x from the filename, if it is there */
			gchar *displayname;
			if(g_str_has_suffix(author_entry, ".i7x"))
				displayname = g_strndup(author_entry, strlen(author_entry) - 4);
			else
				displayname = g_strdup(author_entry);
			gtk_tree_store_append(store, &child_iter, &parent_iter);
			gtk_tree_store_set(store, &child_iter, 
				I7_APP_EXTENSION_TEXT, displayname, 
				I7_APP_EXTENSION_READ_ONLY, FALSE, 
				I7_APP_EXTENSION_ICON, NULL, 
				I7_APP_EXTENSION_PATH, extname, 
				-1);
			g_free(extname);
			g_free(displayname);
		}
		g_dir_close(author);
	}
	g_dir_close(extensions);
	
	/* Do it again for the built-in extensions directory */
	extension_dir = get_builtin_extension_path(app, NULL, NULL);
	extensions = g_dir_open(extension_dir, 0, &err);
	g_free(extension_dir);
	if(err) {
		error_dialog(NULL, err, _("Error opening built-in extensions directory: "));
		return FALSE;
	}

	while((dir_entry = g_dir_read_name(extensions)) != NULL) {
		if(!strcmp(dir_entry, "Reserved"))
			continue;
		gchar *author_dir = get_builtin_extension_path(app, dir_entry, NULL);
		if(g_file_test(author_dir, G_FILE_TEST_IS_SYMLINK) || !g_file_test(author_dir, G_FILE_TEST_IS_DIR)) 
		{
			g_free(author_dir);
			continue;
		}
		/* Read each extension dir, but skip "Reserved", symlinks and nondirs.
		If the author directory was already indexed before, add the extension
		to it instead of making a new entry */
		
		gboolean found = FALSE;
		if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &parent_iter)) {
			do {
				gchar *author;
				gtk_tree_model_get(GTK_TREE_MODEL(store), &parent_iter, I7_APP_EXTENSION_TEXT, &author, -1);
				if(strcmp(dir_entry, author) == 0) {
					found = TRUE;
					break;
				}
			} while(gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &parent_iter));
		}
		if(!found) {
			gtk_tree_store_prepend(store, &parent_iter, NULL);
			gtk_tree_store_set(store, &parent_iter, 
				I7_APP_EXTENSION_TEXT, dir_entry, 
				I7_APP_EXTENSION_READ_ONLY, TRUE,
				I7_APP_EXTENSION_ICON, NULL, 
				I7_APP_EXTENSION_PATH, NULL, 
				-1);
		}
		
		/* Descend into each author directory */
		GDir *author = g_dir_open(author_dir, 0, &err);
		g_free(author_dir);
		if(err) {
			error_dialog(NULL, err, _("Error opening built-in extensions directory: "));
			return FALSE;
		}
		const gchar *author_entry;
		while((author_entry = g_dir_read_name(author)) != NULL) {
			gchar *extname = get_builtin_extension_path(app, dir_entry, author_entry);
			if(g_file_test(extname, G_FILE_TEST_IS_SYMLINK)) {
				g_free(extname);
				continue;
			}
			/* Read each file, but skip symlinks */
			/* Remove .i7x from the filename, if it is there */
			gchar *displayname;
			if(g_str_has_suffix(author_entry, ".i7x"))
				displayname = g_strndup(author_entry, strlen(author_entry) - 4);
			else
				displayname = g_strdup(author_entry);
			
			/* Only add it if it is not overridden by a user-installed extension */
			found = FALSE;
			if(gtk_tree_model_iter_children(GTK_TREE_MODEL(store), &child_iter, &parent_iter)) {
				do {
					gchar *name;
					gtk_tree_model_get(GTK_TREE_MODEL(store), &child_iter, 
						I7_APP_EXTENSION_TEXT, &name, 
						-1);
					if(strcmp(displayname, name) == 0) {
						found = TRUE;
						break;
					}
				} while(gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &child_iter));
			}
			if(!found) {
				gtk_tree_store_prepend(store, &child_iter, &parent_iter);
				gtk_tree_store_set(store, &child_iter, 
					I7_APP_EXTENSION_TEXT, displayname, 
					I7_APP_EXTENSION_READ_ONLY, TRUE, 
					I7_APP_EXTENSION_ICON, "inform7-builtin", 
					I7_APP_EXTENSION_PATH, extname, 
					-1);
			}
			
			g_free(extname);
			g_free(displayname);
		}
		g_dir_close(author);
	}
	g_dir_close(extensions);
	
	/* Rebuild the Open Extension menus */
	i7_app_update_extensions_menu(app);
	
	return FALSE; /* one-shot idle function */
}

/* Start the compiler running the census of extensions. If @wait is FALSE, do it
 in the background. */
void
i7_app_run_census(I7App *app, gboolean wait) 
{
	/* Build the command line */
	gchar **commandline = g_new(gchar *, 5);
	commandline[0] = i7_app_get_binary_path(app, "ni");
	commandline[1] = g_strdup("--rules");
	commandline[2] = get_builtin_extension_path(app, NULL, NULL);
	commandline[3] = g_strdup("--census");
	commandline[4] = NULL;
	
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

/* Returns the directory for installed extensions, with the author and name path
components tacked on if they are not NULL. Returns an allocated string which
must be freed. */
gchar *
i7_app_get_extension_path(I7App *app, const gchar *author, const gchar *extname) 
{
    if(!author)
        return g_build_filename(g_get_home_dir(), EXTENSIONS_BASE_PATH, NULL);
    if(!extname)
        return g_build_filename(g_get_home_dir(), EXTENSIONS_BASE_PATH, author, NULL);
    return g_build_filename(g_get_home_dir(), EXTENSIONS_BASE_PATH, author,	extname, NULL);
}

/* Returns the path to filename in the application data directory. Free string
 afterwards. */
gchar *
i7_app_get_datafile_path(I7App *app, const gchar *filename) 
{
    gchar *path = g_build_filename(I7_APP_PRIVATE(app)->datadir, filename, NULL);
    if(g_file_test(path, G_FILE_TEST_EXISTS))
        return path;
    error_dialog(NULL, NULL, _("An application file, %s, was not found. "
		"Please reinstall Inform 7."), filename);
    return NULL;
}

/* Concatenates the path elements and returns the path to the filename in the
application data directory. Must end with NULL. Free string afterwards. */
gchar *
i7_app_get_datafile_path_va(I7App *app, const gchar *path1, ...) 
{
    va_list ap;
    
    int num_args = 0;
    va_start(ap, path1);
    do
        num_args++;
    while(va_arg(ap, gchar *) != NULL);
    va_end(ap);
    
    gchar **args = g_new(gchar *, num_args + 2);
    args[0] = g_strdup(I7_APP_PRIVATE(app)->datadir);
    args[1] = g_strdup(path1);
    int i;
    va_start(ap, path1);
    for(i = 2; i < num_args + 2; i++)
        args[i] = g_strdup(va_arg(ap, gchar *));
    va_end(ap);
    
    gchar *path = g_build_filenamev(args);
    if(g_file_test(path, G_FILE_TEST_EXISTS)) {
        g_strfreev(args);
        return path;
    }
    error_dialog(NULL, NULL, _("An application file, %s, was not found. "
		"Please reinstall Inform 7."), args[num_args]); /* argument before NULL */
    g_strfreev(args);
    return NULL;
}

/* Returns TRUE if filename exists in the data directory, otherwise FALSE.
Used when we do not necessarily want to return an error if it does not. */
gboolean 
i7_app_check_datafile(I7App *app, const gchar *filename) 
{
    gchar *path = g_build_filename(I7_APP_PRIVATE(app)->datadir, filename, NULL);
	gboolean retval = g_file_test(path, G_FILE_TEST_EXISTS);
	g_free(path);
	return retval;
}

/* Varargs variant of check_datafile. Must end with NULL. */
gboolean
i7_app_check_datafile_va(I7App *app, const gchar *path1, ...) 
{
    va_list ap;
    
    int num_args = 0;
    va_start(ap, path1);
    do
        num_args++;
    while(va_arg(ap, gchar *) != NULL);
    va_end(ap);
    
    gchar **args = g_new(gchar *, num_args + 2);
    args[0] = g_strdup(I7_APP_PRIVATE(app)->datadir);
    args[1] = g_strdup(path1);
    int i;
    va_start(ap, path1);
    for(i = 2; i < num_args + 2; i++)
        args[i] = g_strdup(va_arg(ap, gchar *));
    va_end(ap);
    
    gchar *path = g_build_filenamev(args);
	gboolean retval = g_file_test(path, G_FILE_TEST_EXISTS);
    g_strfreev(args);
    return retval;
}

/* Returns the path to filename in the application pixmap directory. Free string
 afterwards. */
gchar *
i7_app_get_pixmap_path(I7App *app, const gchar *filename) 
{
	gchar *path = g_build_filename(I7_APP_PRIVATE(app)->pixmapdir, filename, NULL);
	if(g_file_test(path, G_FILE_TEST_EXISTS))
        return path;
    error_dialog(NULL, NULL, _("An application file, %s, was not found. "
		"Please reinstall Inform 7."), filename);
    return NULL;
}

/* Returns the path to filename in the application libexec directory. Free 
 string afterwards. */
gchar *
i7_app_get_binary_path(I7App *app, const gchar *filename)
{
	gchar *path = g_build_filename(I7_APP_PRIVATE(app)->libexecdir, filename, NULL);
	if(g_file_test(path, G_FILE_TEST_EXISTS))
		return path;
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
			gchar *extname, *path;
			gboolean readonly;
			gtk_tree_model_get(model, &title,
				I7_APP_EXTENSION_TEXT, &extname,
				I7_APP_EXTENSION_READ_ONLY, &readonly,
				I7_APP_EXTENSION_PATH, &path, 
				-1);
			GtkWidget *extitem;
			if(readonly) {
				extitem = gtk_image_menu_item_new_with_label(extname);
				GtkWidget *image = gtk_image_new_from_icon_name("inform7-builtin", GTK_ICON_SIZE_MENU);
				gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(extitem), image);
				g_signal_connect(extitem, "activate", G_CALLBACK(on_open_extension_readonly_activate), path);
			} else {
				extitem = gtk_menu_item_new_with_label(extname);
				g_signal_connect(extitem, "activate", G_CALLBACK(on_open_extension_activate), path);
			}
			gtk_widget_show(extitem);
			gtk_menu_shell_append(GTK_MENU_SHELL(extmenu), extitem);
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
