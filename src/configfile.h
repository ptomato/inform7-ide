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

#define STANDARD_FONT_FALLBACK "Sans 11"
#define MONOSPACE_FONT_FALLBACK "Monospace 11"

/* Three options for editor font */
typedef enum {
	FONT_STANDARD = 0,
	FONT_MONOSPACE = 1,
	FONT_CUSTOM = 2
} I7PrefsFont;

/* Four different text size options */
typedef enum {
	FONT_SIZE_STANDARD = 0,
	FONT_SIZE_MEDIUM = 1,
	FONT_SIZE_LARGE = 2,
	FONT_SIZE_HUGE = 3
} I7PrefsFontSize;

typedef enum {
  INTERPRETER_GLULXE = 0,
  INTERPRETER_GIT = 1
} I7PrefsInterpreter;

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

/* Generates a series of method definitions you can
  use of the form:
    * type get_name()
    * void set_name(type value)
 */
#define getter_and_setter(type, name) \
  type config_get_##name(); \
  void config_set_##name(const type v);

getter_and_setter(gboolean, spell_check_default);
getter_and_setter(gboolean, clean_build_files);
getter_and_setter(gboolean, clean_index_files);
getter_and_setter(gboolean, debug_log_visible);
getter_and_setter(gboolean, toolbar_visible);
getter_and_setter(gboolean, statusbar_visible);
getter_and_setter(gboolean, notepad_visible);
getter_and_setter(int, interpreter);
getter_and_setter(gboolean, elastic_tabs_default);

getter_and_setter(int, horizontal_spacing);
getter_and_setter(int, vertical_spacing);

getter_and_setter(int, app_window_width);
getter_and_setter(int, app_window_height);
getter_and_setter(int, ext_window_width);
getter_and_setter(int, ext_window_height);
getter_and_setter(int, notepad_x);
getter_and_setter(int, notepad_y);
getter_and_setter(int, notepad_width);
getter_and_setter(int, notepad_height);
getter_and_setter(int, slider_position);

getter_and_setter(gboolean, syntax_highlighting);
getter_and_setter(gboolean, auto_indent);
getter_and_setter(gboolean, intelligence);
getter_and_setter(gboolean, auto_number_sections);

getter_and_setter(int, font_set);
getter_and_setter(int, font_size);
getter_and_setter(char *, custom_font);
getter_and_setter(char *, style_scheme);
getter_and_setter(int, tab_width);
getter_and_setter(int, elastic_tabstops_padding);

getter_and_setter(char *, author_name);


void init_config_file(GtkBuilder *builder);
void trigger_config_file(void);
PangoFontDescription *get_desktop_standard_font(void);
PangoFontDescription *get_desktop_monospace_font(void);
gint get_font_size(PangoFontDescription *font);
PangoFontDescription *get_font_description(void);

#endif
