/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Philip Chimento <philip.chimento@gmail.com>
 */

#pragma once

#include "config.h"

#include <stdbool.h>

#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>

#include "document.h"

#define I7_TYPE_APP            (i7_app_get_type())
G_DECLARE_FINAL_TYPE(I7App, i7_app, I7, APP, GtkApplication)

#define REGEX_EXTENSION \
	"^\\s*(?:version\\s(?P<version>.+)\\sof\\s+)?(?:the\\s+)?" /* Version X of [the] */ \
	"(?P<title>.+?)\\s+(?:\\(for\\s.+\\sonly\\)\\s+)?" /* <title> [(for X only)] */ \
	"by\\s+(?P<author>.+)\\s+" /* by <author> */ \
	"begins?\\s+here\\.?\\s*$" /* begins here[.] */

enum _I7AppExtensionsColumns {
	I7_APP_EXTENSION_TEXT,  /* title rows are children of author rows */
	I7_APP_EXTENSION_VERSION,
	I7_APP_EXTENSION_READ_ONLY,
	I7_APP_EXTENSION_ICON,
	I7_APP_EXTENSION_FILE,
	I7_APP_NUM_EXTENSION_COLUMNS
};

typedef void (*I7DocumentForeachFunc)(I7Document *, gpointer);

/**
 * I7AppAuthorFunc:
 * @info: the #GFileInfo for the author directory.
 * @data: user data to pass to the callback.
 *
 * Callback for enumerating installed extensions, called for each author
 * directory. May return a result, which is passed to #I7AppExtensionFunc for
 * each extension file found in that author directory.
 */
typedef void * (*I7AppAuthorFunc)(GFileInfo *info, void *data);
/**
 * I7AppExtensionFunc:
 * @parent: the #GFile for the extension file's parent.
 * @info: the #GFileInfo for the extension file.
 * @author_result: the return value of the #I7AppAuthorFunc for the parent.
 * @data: user data to pass to the callback.
 *
 * Callback for enumerating installed extensions, called for each author
 * directory.
 */
typedef void (*I7AppExtensionFunc)(GFile *parent, GFileInfo *info, void *author_result, void *data);

GType i7_app_get_type(void) G_GNUC_CONST;
I7App *i7_app_new(void);

I7Document *i7_app_get_already_open(I7App *self, const GFile *file);
void i7_app_close_all_documents(I7App *self);

void i7_app_monitor_extensions_directory(I7App *self);
void i7_app_stop_monitoring_extensions_directory(I7App *self);
void i7_app_install_extension(I7App *self, GFile *file);
void i7_app_delete_extension(I7App *self, char *author, char *extname);
void i7_app_download_extension_async(I7App *self, GFile *file, GCancellable *cancellable, GFileProgressCallback progress_callback, void *progress_callback_data, GAsyncReadyCallback finish_callback, void *finish_callback_data);
bool i7_app_download_extension_finish(I7App *self, GAsyncResult *res, GError **err);
char *i7_app_get_extension_version(I7App *self, const char *author, const char *title, gboolean *builtin);
void i7_app_run_census(I7App *self, gboolean wait);

GFile *i7_app_get_extension_file(const char *author, const char *extname);
GFile *i7_app_get_extension_home_page(void);
GFile *i7_app_get_internal_dir(I7App *self);
GFile *i7_app_get_retrospective_internal_dir(I7App *self, const char *build);
GFile *i7_app_get_data_file(I7App *self, const char *filename);
GFile *i7_app_get_data_file_va(I7App *self, const char *path1, ...) G_GNUC_NULL_TERMINATED;
GFile *i7_app_get_binary_file(I7App *self, const char *filename);
GFile *i7_app_get_retrospective_binary_file(I7App *self, const char *build, const char *filename);
GFile *i7_app_get_config_dir(void);

GtkTreeStore *i7_app_get_installed_extensions_tree(I7App *self);
void i7_app_update_extensions_menu(I7App *self);
const char *i7_app_lookup_action_tooltip(I7App *self, const char *action_name, GVariant *target_value);

GtkPrintSettings *i7_app_get_print_settings(I7App *self);
void i7_app_set_print_settings(I7App *self, GtkPrintSettings *settings);

GtkSourceStyleSchemeManager *i7_app_get_color_scheme_manager(I7App *self);

void i7_app_update_css(I7App *self);

char *i7_app_get_document_font_string(I7App *self);
PangoFontDescription *i7_app_get_document_font_description(I7App *self);
char *i7_app_get_ui_font(I7App *self);
double i7_app_get_docs_font_scale(I7App *self);

GFile *i7_app_get_last_opened_project(void);

/* Color scheme functions, in app-colorscheme.c */

void i7_app_foreach_color_scheme(I7App *self, GFunc callback, gpointer data);
gboolean i7_app_color_scheme_is_user_scheme(I7App *self, const char *id);
const char *i7_app_install_color_scheme(I7App *self, GFile *file);
gboolean i7_app_uninstall_color_scheme(I7App *self, const char *id);
GtkSourceStyleScheme *i7_app_get_current_color_scheme(I7App *self);

/* GSettings accessors */
GSettings *i7_app_get_system_settings(I7App *self);
GSettings *i7_app_get_state(I7App *self);
GSettings *i7_app_get_prefs(I7App *self);

/* Retrospective functions, in app-retrospective.c */

bool i7_app_is_valid_retrospective_id(I7App *self, const char *id);
GListStore *i7_app_get_retrospectives(I7App *self);

char **i7_app_get_inform_command_line(I7App *self, const char *version_id, int /* I7StoryFormat */ format, bool debug, bool reproducible, bool basic_inform, GFile *project_file);
char **i7_app_get_inblorb_command_line(I7App *self, const char *version_id, GFile *blorb_file);
