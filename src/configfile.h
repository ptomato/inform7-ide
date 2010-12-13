/* This file is part of GNOME Inform 7.
 * Copyright (c) 2006-2009 P. F. Chimento <philip.chimento@gmail.com>
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
#include <gconf/gconf.h>

/* The slashes below are not directory separators, so they should be slashes! */
#define PREFS_BASE_PATH		"/apps/gnome-inform7"
#define PREFS_APP_PATH		PREFS_BASE_PATH "/AppSettings/"
#define PREFS_IDE_PATH		PREFS_BASE_PATH "/IDESettings/"
#define PREFS_EDITOR_PATH	PREFS_BASE_PATH "/EditorSettings/"
#define PREFS_SYNTAX_PATH	PREFS_BASE_PATH "/SyntaxSettings/"
#define PREFS_WINDOW_PATH	PREFS_BASE_PATH "/WindowSettings/"
#define PREFS_SKEIN_PATH	PREFS_BASE_PATH "/SkeinSettings/"

#define PREFS_AUTHOR_NAME	PREFS_APP_PATH "AuthorName"

#define PREFS_SPELL_CHECK_DEFAULT	PREFS_IDE_PATH "SpellCheckDefault"
#define PREFS_CLEAN_BUILD_FILES		PREFS_IDE_PATH "CleanBuildFiles"
#define PREFS_CLEAN_INDEX_FILES		PREFS_IDE_PATH "CleanIndexFiles"
#define PREFS_DEBUG_LOG_VISIBLE		PREFS_IDE_PATH "DebugLogVisible"
#define PREFS_TOOLBAR_VISIBLE       PREFS_IDE_PATH "ToolbarDefault"
#define PREFS_STATUSBAR_VISIBLE     PREFS_IDE_PATH "StatusbarDefault"
#define PREFS_NOTEPAD_VISIBLE       PREFS_IDE_PATH "NotepadDefault"
#define PREFS_USE_GIT               PREFS_IDE_PATH "UseGit"
#define PREFS_ELASTIC_TABS_DEFAULT  PREFS_IDE_PATH "ElasticTabsDefault"

#define PREFS_FONT_SET		        PREFS_EDITOR_PATH "FontSet"
#define PREFS_CUSTOM_FONT	        PREFS_EDITOR_PATH "CustomFont"
#define PREFS_FONT_SIZE		        PREFS_EDITOR_PATH "FontSize"
#define PREFS_STYLE_SCHEME	        PREFS_EDITOR_PATH "StyleScheme"
#define PREFS_TAB_WIDTH		        PREFS_EDITOR_PATH "TabWidth"
#define PREFS_ELASTIC_TABS_PADDING  PREFS_EDITOR_PATH "ElasticTabPadding"

#define PREFS_SYNTAX_HIGHLIGHTING	PREFS_SYNTAX_PATH "SyntaxHighlighting"
#define PREFS_AUTO_INDENT			PREFS_SYNTAX_PATH "AutoIndent"
#define PREFS_INTELLIGENCE			PREFS_SYNTAX_PATH "Intelligence"
#define PREFS_AUTO_NUMBER_SECTIONS	PREFS_SYNTAX_PATH "AutoNumberSections"

#define PREFS_APP_WINDOW_WIDTH	PREFS_WINDOW_PATH "AppWindowWidth"
#define PREFS_APP_WINDOW_HEIGHT	PREFS_WINDOW_PATH "AppWindowHeight"
#define PREFS_SLIDER_POSITION	PREFS_WINDOW_PATH "SliderPosition"
#define PREFS_EXT_WINDOW_WIDTH	PREFS_WINDOW_PATH "ExtWindowWidth"
#define PREFS_EXT_WINDOW_HEIGHT PREFS_WINDOW_PATH "ExtWindowHeight"
#define PREFS_NOTEPAD_X         PREFS_WINDOW_PATH "NotepadPosX"
#define PREFS_NOTEPAD_Y         PREFS_WINDOW_PATH "NotepadPosY"
#define PREFS_NOTEPAD_WIDTH     PREFS_WINDOW_PATH "NotepadWidth"
#define PREFS_NOTEPAD_HEIGHT    PREFS_WINDOW_PATH "NotepadHeight"

#define PREFS_HORIZONTAL_SPACING	PREFS_SKEIN_PATH "HorizontalSpacing"
#define PREFS_VERTICAL_SPACING		PREFS_SKEIN_PATH "VerticalSpacing"

#define DESKTOP_PREFS_STANDARD_FONT	"/desktop/gnome/interface/document_font_name"
#define DESKTOP_PREFS_MONOSPACE_FONT "/desktop/gnome/interface/monospace_font_name"

/* Three options for editor font */
typedef enum {
    FONT_STANDARD,
    FONT_MONOSPACE,
    FONT_CUSTOM
} I7PrefsFont;

/* Four different text size options */
typedef enum {
    FONT_SIZE_STANDARD,
    FONT_SIZE_MEDIUM,
    FONT_SIZE_LARGE,
    FONT_SIZE_HUGE
} I7PrefsFontSize;

/* Pango point sizes of text size options */
#define DEFAULT_SIZE_STANDARD	12
#define RELATIVE_SIZE_STANDARD	1.0
#define RELATIVE_SIZE_MEDIUM	1.2
#define RELATIVE_SIZE_LARGE		1.4
#define RELATIVE_SIZE_HUGE		1.8

/* Other various settings */
#define DEFAULT_TAB_WIDTH			8

/* Enum-to-string lookup tables */
extern GConfEnumStringPair font_styling_lookup_table[];
extern GConfEnumStringPair font_set_lookup_table[];
extern GConfEnumStringPair font_size_lookup_table[];
extern GConfEnumStringPair change_colors_lookup_table[];
extern GConfEnumStringPair color_set_lookup_table[];

void config_file_set_string(const gchar *key, const gchar *value);
gchar *config_file_get_string(const gchar *key);
void config_file_set_int(const gchar *key, const gint value);
gint config_file_get_int(const gchar *key);
void config_file_set_bool(const gchar *key, const gboolean value);
gboolean config_file_get_bool(const gchar *key);
void config_file_set_enum(const gchar *key, const gint value, GConfEnumStringPair lookup_table[]);
gint config_file_get_enum(const gchar *key, GConfEnumStringPair lookup_table[]);
void config_file_set_to_default(const gchar *key);
void init_config_file(GtkBuilder *builder);
void trigger_config_file(void);
gchar *get_font_family(void);
gint get_font_size(PangoFontDescription *font);
PangoFontDescription *get_font_description(void);

#endif
