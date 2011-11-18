/* Copyright (C) 2006-2009, 2010, 2011 P. F. Chimento
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

#ifndef CONFIG_FILE_H
#define CONFIG_FILE_H

#include <glib.h>
#include <gtk/gtk.h>
#include <gio/gio.h>

/* The slashes below are not directory separators, so they should be slashes! */
#define PREFS_BASE_PATH    "/apps/gnome-inform7/"
#define PREFS_APP_PATH     PREFS_BASE_PATH "app/"
#define PREFS_IDE_PATH     PREFS_BASE_PATH "ide/"
#define PREFS_EDITOR_PATH  PREFS_BASE_PATH "editor/"
#define PREFS_SYNTAX_PATH  PREFS_BASE_PATH "syntax/"
#define PREFS_WINDOW_PATH  PREFS_BASE_PATH "window/"
#define PREFS_SKEIN_PATH   PREFS_BASE_PATH "skein/"

#define PREFS_AUTHOR_NAME  PREFS_APP_PATH "AuthorName"

#define PREFS_SPELL_CHECK_DEFAULT       PREFS_IDE_PATH "spell-check-default"
#define PREFS_CLEAN_BUILD_FILES         PREFS_IDE_PATH "clean-build-files"
#define PREFS_CLEAN_INDEX_FILES         PREFS_IDE_PATH "clean-index-files"
#define PREFS_DEBUG_LOG_VISIBLE         PREFS_IDE_PATH "debug-log-visible"
#define PREFS_TOOLBAR_VISIBLE           PREFS_IDE_PATH "toolbar-default"
#define PREFS_STATUSBAR_VISIBLE         PREFS_IDE_PATH "statusbar-default"
#define PREFS_NOTEPAD_VISIBLE           PREFS_IDE_PATH "notepad-default"
#define PREFS_USE_GIT                   PREFS_IDE_PATH "use-git"
#define PREFS_ELASTIC_TABSTOPS_DEFAULT  PREFS_IDE_PATH "elastic-tabs-default"

#define PREFS_FONT_SET                  PREFS_EDITOR_PATH "font-set"
#define PREFS_CUSTOM_FONT               PREFS_EDITOR_PATH "custom-font"
#define PREFS_FONT_SIZE                 PREFS_EDITOR_PATH "font-size"
#define PREFS_STYLE_SCHEME              PREFS_EDITOR_PATH "style-scheme"
#define PREFS_TAB_WIDTH                 PREFS_EDITOR_PATH "tab-width"
#define PREFS_ELASTIC_TABSTOPS_PADDING  PREFS_EDITOR_PATH "elastic-tab-padding"

#define PREFS_SYNTAX_HIGHLIGHTING   PREFS_SYNTAX_PATH "syntax-highlighting"
#define PREFS_AUTO_INDENT           PREFS_SYNTAX_PATH "auto-indent"
#define PREFS_INTELLIGENCE          PREFS_SYNTAX_PATH "intelligence"
#define PREFS_AUTO_NUMBER_SECTIONS  PREFS_SYNTAX_PATH "auto-number-sections"

#define PREFS_APP_WINDOW_WIDTH   PREFS_WINDOW_PATH "app-window-width"
#define PREFS_APP_WINDOW_HEIGHT  PREFS_WINDOW_PATH "app-window-height"
#define PREFS_SLIDER_POSITION    PREFS_WINDOW_PATH "slider-position"
#define PREFS_EXT_WINDOW_WIDTH   PREFS_WINDOW_PATH "ext-window-width"
#define PREFS_EXT_WINDOW_HEIGHT  PREFS_WINDOW_PATH "ext-window-height"
#define PREFS_NOTEPAD_X          PREFS_WINDOW_PATH "notepad-pos-x"
#define PREFS_NOTEPAD_Y          PREFS_WINDOW_PATH "notepad-pos-y"
#define PREFS_NOTEPAD_WIDTH      PREFS_WINDOW_PATH "notepad-width"
#define PREFS_NOTEPAD_HEIGHT     PREFS_WINDOW_PATH "notepad-height"

#define PREFS_HORIZONTAL_SPACING  PREFS_SKEIN_PATH "horizontal-spacing"
#define PREFS_VERTICAL_SPACING    PREFS_SKEIN_PATH "vertical-spacing"

#define DESKTOP_PREFS_STANDARD_FONT   "/org/gnome/desktop/interface/font-name"
#define DESKTOP_PREFS_MONOSPACE_FONT  "/org/gnome/desktop/interface/monospace-font-name"

/* Three options for editor font */
typedef enum {
	FONT_STANDARD = 0,
	FONT_MONOSPACE = 1,
	FONT_CUSTOM = 2
} I7PrefsFont;

/* Four different text size options, ratios have a 10x factor */
typedef enum {
	FONT_SIZE_STANDARD = 10,
	FONT_SIZE_MEDIUM = 12,
	FONT_SIZE_LARGE = 14,
	FONT_SIZE_HUGE = 18
} I7PrefsFontSize;

/* Pango point sizes of text size options */
#define DEFAULT_SIZE_STANDARD	12
#define RELATIVE_SIZE_STANDARD 1.0
#define RELATIVE_SIZE_MEDIUM 1.2
#define RELATIVE_SIZE_LARGE 1.4
#define RELATIVE_SIZE_HUGE 1.8

/* Other various settings */
#define DEFAULT_TAB_WIDTH			8
#define DEFAULT_ELASTIC_TAB_PADDING 16
#define DEFAULT_HORIZONTAL_SPACING 40
#define DEFAULT_VERTICAL_SPACING 75

void config_file_set_string(const gchar *key, const gchar *value);
gchar *config_file_get_string(const gchar *key);
void config_file_set_int(const gchar *key, const gint value);
gint config_file_get_int(const gchar *key);
void config_file_set_bool(const gchar *key, const gboolean value);
gboolean config_file_get_bool(const gchar *key);
void config_file_set_enum(const gchar *key, const gint value);
gint config_file_get_enum(const gchar *key);
void init_config_file(GtkBuilder *builder);
void trigger_config_file(void);
PangoFontDescription *get_desktop_standard_font(void);
PangoFontDescription *get_desktop_monospace_font(void);
gint get_font_size(PangoFontDescription *font);
PangoFontDescription *get_font_description(void);

#endif
