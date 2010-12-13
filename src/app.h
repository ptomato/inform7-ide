/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * new-i7
 * 
 * new-i7 is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * new-i7 is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _APP_H_
#define _APP_H_

#include <stdarg.h>
#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>
#include "document.h"
#include "prefs.h"

#define I7_TYPE_APP            (i7_app_get_type())
#define I7_APP(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), I7_TYPE_APP, I7App))
#define I7_APP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), I7_TYPE_APP, I7AppClass))
#define I7_IS_APP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), I7_TYPE_APP))
#define I7_IS_APP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), I7_TYPE_APP))
#define I7_APP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), I7_TYPE_APP, I7AppClass))

enum _I7AppRegices {
	I7_APP_REGEX_HEADINGS,
	I7_APP_REGEX_UNICODE_ESCAPE,
	I7_APP_REGEX_IMAGES_REPLACE,
	I7_APP_REGEX_EXTENSION,
	I7_APP_REGEX_EXTENSION_FILE_NAME,
	I7_APP_NUM_REGICES
};

enum _I7AppExtensionsColumns {
	I7_APP_EXTENSION_TEXT,
	I7_APP_EXTENSION_READ_ONLY,
	I7_APP_EXTENSION_ICON,
	I7_APP_EXTENSION_PATH,
	I7_APP_NUM_EXTENSION_COLUMNS
};

typedef void (*I7DocumentForeachFunc)(I7Document *, gpointer);

typedef struct {
	GObjectClass parent_class;
} I7AppClass;

typedef struct {
	GObject parent_instance;

	/* Public preferences dialog */
	I7PrefsWidgets *prefs;
	/* Already-compiled regices */
	GRegex *regices[I7_APP_NUM_REGICES];
} I7App;

GType i7_app_get_type(void) G_GNUC_CONST;
I7App *i7_app_get(void);
void i7_app_open(I7App *app, const gchar *filename);
void i7_app_insert_action_groups(I7App *app, GtkUIManager *manager);

void i7_app_register_document(I7App *app, I7Document *document);
void i7_app_remove_document(I7App *app, I7Document *document);
I7Document *i7_app_get_already_open(I7App *app, const gchar *filename);
gint i7_app_get_num_open_documents(I7App *app);
void i7_app_close_all_documents(I7App *app);
void i7_app_foreach_document(I7App *app, I7DocumentForeachFunc func, gpointer data);

void i7_app_monitor_extensions_directory(I7App *app);
void i7_app_stop_monitoring_extensions_directory(I7App *app);
void i7_app_install_extension(I7App *app, const gchar *filename);
void i7_app_delete_extension(I7App *app, gchar *author, gchar *extname);
void i7_app_run_census(I7App *app, gboolean wait);

gchar *i7_app_get_extension_path(I7App *app, const gchar *author, const gchar *extname);
gchar *i7_app_get_datafile_path(I7App *app, const gchar *filename);
gchar *i7_app_get_datafile_path_va(I7App *app, const gchar *path1, ...);
gboolean i7_app_check_datafile(I7App *app, const gchar *filename);
gboolean i7_app_check_datafile_va(I7App *app, const gchar *path1, ...); 
gchar *i7_app_get_pixmap_path(I7App *app, const gchar *filename);
gchar *i7_app_get_binary_path(I7App *app, const gchar *filename);

GtkTreeStore *i7_app_get_installed_extensions_tree(I7App *app);
void i7_app_update_extensions_menu(I7App *app);

GtkPrintSettings *i7_app_get_print_settings(I7App *app);
void i7_app_set_print_settings(I7App *app, GtkPrintSettings *settings);
GtkPageSetup *i7_app_get_page_setup(I7App *app);
void i7_app_set_page_setup(I7App *app, GtkPageSetup *setup);

void i7_app_present_prefs_window(I7App *app);

void i7_app_set_busy(I7App *app, gboolean busy);

#endif /* _APP_H_ */
