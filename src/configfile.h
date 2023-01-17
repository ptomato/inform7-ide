/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2006-2011, 2013, 2019 Philip Chimento <philip.chimento@gmail.com>
 */

#ifndef CONFIG_FILE_H
#define CONFIG_FILE_H

#include "config.h"

#include <glib.h>
#include <pango/pango.h>

typedef enum {
	FONT_STANDARD = 0,
	/* deprecated: FONT_MONOSPACE */
	FONT_CUSTOM = 2
} I7PrefsFont;

typedef enum {
	FONT_SIZE_SMALLEST,
	FONT_SIZE_SMALLER,
	FONT_SIZE_SMALL,
	FONT_SIZE_MEDIUM,
	FONT_SIZE_LARGE,
	FONT_SIZE_LARGER,
	FONT_SIZE_LARGEST,
} I7PrefsFontSize;

typedef enum {
  INTERPRETER_GLULXE = 0,
  INTERPRETER_GIT = 1
} I7PrefsInterpreter;

/* Pango point sizes of text size options */
#define DEFAULT_SIZE_STANDARD 10
#define RELATIVE_SIZE_SMALLEST 0.5
#define RELATIVE_SIZE_SMALLER 0.7
#define RELATIVE_SIZE_SMALL 0.8
#define RELATIVE_SIZE_MEDIUM 1.0
#define RELATIVE_SIZE_LARGE 1.2
#define RELATIVE_SIZE_LARGER 1.4
#define RELATIVE_SIZE_LARGEST 1.8

/* Other various settings */
#define DEFAULT_TAB_WIDTH 8
#define DEFAULT_STYLE_SCHEME "light"

/* Schemas */
#define SCHEMA_SYSTEM "org.gnome.desktop.interface"
#define SCHEMA_PREFERENCES "com.inform7.IDE.preferences"
#define SCHEMA_SKEIN "com.inform7.IDE.preferences.skein"
#define SCHEMA_STATE "com.inform7.IDE.state"

/* Keys */
#define PREFS_AUTHOR_NAME          "author-name"
#define PREFS_FONT_SET             "font-set"
#define PREFS_CUSTOM_FONT          "custom-font"
#define PREFS_DOCS_FONT_SIZE       "docs-font-size"
#define PREFS_STYLE_SCHEME         "style-scheme"
#define PREFS_TAB_WIDTH            "tab-width"
#define PREFS_ELASTIC_TABSTOPS     "elastic-tabstops"
#define PREFS_TABSTOPS_PADDING     "elastic-tabstops-padding"
#define PREFS_SYNTAX_HIGHLIGHTING  "syntax-highlighting"
#define PREFS_AUTO_INDENT          "auto-indent"
#define PREFS_AUTO_NUMBER          "auto-number"
#define PREFS_INTERPRETER          "interpreter"
#define PREFS_CLEAN_BUILD_FILES    "clean-build-files"
#define PREFS_CLEAN_INDEX_FILES    "clean-index-files"
#define PREFS_SHOW_DEBUG_LOG       "show-debug-log"

#define PREFS_STATE_SPELL_CHECK       "spell-check"
#define PREFS_STATE_SHOW_TOOLBAR      "show-toolbar"
#define PREFS_STATE_SHOW_STATUSBAR    "show-statusbar"
#define PREFS_STATE_SHOW_NOTEPAD      "show-notepad"
#define PREFS_STATE_WINDOW_SIZE       "app-window-size"
#define PREFS_STATE_DIVIDER_POS       "divider-position"
#define PREFS_STATE_EXT_WINDOW_SIZE   "ext-window-size"
#define PREFS_STATE_NOTEPAD_POS       "notepad-position"
#define PREFS_STATE_NOTEPAD_SIZE      "notepad-size"

#define PREFS_SKEIN_HORIZONTAL_SPACING  "horizontal-spacing"
#define PREFS_SKEIN_VERTICAL_SPACING    "vertical-spacing"

#define PREFS_SYSTEM_UI_FONT        "font-name"
#define PREFS_SYSTEM_DOCUMENT_FONT  "document-font-name"

void init_config_file(GSettings *prefs);

#endif
