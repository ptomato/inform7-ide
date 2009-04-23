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
 
#include <gnome.h>
#include <gtksourceview/gtksourcelanguage.h>
#include <gtksourceview/gtksourcestyle.h>
#include <gtksourceview/gtksourcestyleschememanager.h>

#include "colorscheme.h"
#include "configfile.h"
#include "prefs.h"
#include "datafile.h"

/* (0..255)*256 = (0..255)*65536/256 = convert color component from 8 bits to
16 bits */
#define RGB(r,g,b) {0, (r)*256, (g)*256, (b)*256}

static GdkColor scheme_subdued[] = {
    RGB(0xFF, 0xFF, 0xFF), /* CLR_BACKGROUND */
    RGB(0x00, 0x00, 0x00), /* CLR_TEXT */
    RGB(0x00, 0x26, 0x4D), /* CLR_STRING */
    RGB(0x26, 0x26, 0x80), /* CLR_STRING_MARKUP */
    RGB(0x7D, 0x2E, 0x2E), /* CLR_KEYWORD */
    RGB(0x12, 0x33, 0x12), /* CLR_COMMENT */
    RGB(0xFF, 0x00, 0x00), /* CLR_ERROR */
    RGB(0x24, 0xD4, 0xD4), /* CLR_HIGHLIGHT */
    RGB(0x00, 0x00, 0x00), /* CLR_LOCKED */
    RGB(0x68, 0x65, 0xFF), /* CLR_UNLOCKED */
    RGB(0x68, 0x65, 0xFF), /* CLR_SKEIN_INPUT */
    RGB(0x60, 0x60, 0x60), /* CLR_I6_CODE */
    RGB(0xFF, 0xF0, 0xF0), /* CLR_TRANS_CHANGED */
    RGB(0xF0, 0xFF, 0xF0), /* CLR_TRANS_UNCHANGED */
    RGB(0xE0, 0xF0, 0xFF), /* CLR_TRANS_INPUT */
    RGB(0xF0, 0xF0, 0xF0), /* CLR_TRANS_UNSET */
};

static GdkColor scheme_standard[] = {
    RGB(0xFF, 0xFF, 0xFF), /* CLR_BACKGROUND */
    RGB(0x00, 0x00, 0x00), /* CLR_TEXT */
    RGB(0x00, 0x4D, 0x99), /* CLR_STRING */
    RGB(0x3E, 0x9E, 0xFF), /* CLR_STRING_MARKUP */
    RGB(0x88, 0x14, 0x14), /* CLR_KEYWORD */
    RGB(0x24, 0x6E, 0x24), /* CLR_COMMENT */
    RGB(0xFF, 0x00, 0x00), /* CLR_ERROR */
    RGB(0x24, 0xD4, 0xD4), /* CLR_HIGHLIGHT */
    RGB(0x00, 0x00, 0x00), /* CLR_LOCKED */
    RGB(0x68, 0x65, 0xFF), /* CLR_UNLOCKED */
    RGB(0x68, 0x65, 0xFF), /* CLR_SKEIN_INPUT */
    RGB(0x60, 0x60, 0x60), /* CLR_I6_CODE */
    RGB(0xFF, 0xF0, 0xF0), /* CLR_TRANS_CHANGED */
    RGB(0xF0, 0xFF, 0xF0), /* CLR_TRANS_UNCHANGED */
    RGB(0xE0, 0xF0, 0xFF), /* CLR_TRANS_INPUT */
    RGB(0xF0, 0xF0, 0xF0), /* CLR_TRANS_UNSET */
};   

static GdkColor scheme_psychedelic[] = {
    RGB(0xFF, 0xFF, 0xFF), /* CLR_BACKGROUND */
    RGB(0x00, 0x00, 0x00), /* CLR_TEXT */
    RGB(0x00, 0x00, 0xFF), /* CLR_STRING */
    RGB(0xFF, 0x00, 0xFF), /* CLR_STRING_MARKUP */
    RGB(0xFF, 0x80, 0x80), /* CLR_KEYWORD */
    RGB(0x00, 0xCC, 0x00), /* CLR_COMMENT */
    RGB(0xFF, 0x00, 0x00), /* CLR_ERROR */
    RGB(0x24, 0xD4, 0xD4), /* CLR_HIGHLIGHT */
    RGB(0x00, 0x00, 0x00), /* CLR_LOCKED */
    RGB(0x68, 0x65, 0xFF), /* CLR_UNLOCKED */
    RGB(0x68, 0x65, 0xFF), /* CLR_SKEIN_INPUT */
    RGB(0x60, 0x60, 0x60), /* CLR_I6_CODE */
    RGB(0xFF, 0xF0, 0xF0), /* CLR_TRANS_CHANGED */
    RGB(0xF0, 0xFF, 0xF0), /* CLR_TRANS_UNCHANGED */
    RGB(0xE0, 0xF0, 0xFF), /* CLR_TRANS_INPUT */
    RGB(0xF0, 0xF0, 0xF0), /* CLR_TRANS_UNSET */
};   

static GtkSourceStyleSchemeManager *scheme_manager = NULL;

static GtkSourceStyleSchemeManager *
get_style_scheme_manager(void)
{
    if (!scheme_manager) {
    	GList ldirs;
        ldirs.data = get_datafile_path("styles"); 
        ldirs.prev = NULL;
        ldirs.next = NULL;
        scheme_manager = GTK_SOURCE_STYLE_SCHEME_MANAGER(g_object_new(
          GTK_TYPE_SOURCE_STYLE_SCHEME_MANAGER, "search-path", &ldirs, NULL));
    }
    return scheme_manager;
}

/* Get the appropriate color scheme for the current settings. Return value must
not be unref'd. */
GtkSourceStyleScheme *
get_style_scheme(void)
{
    GtkSourceStyleSchemeManager *manager = get_style_scheme_manager();
    int set = config_file_get_enum("EditorSettings", "ColorSet", 
      color_set_lookup_table);
    int styling = config_file_get_enum("EditorSettings", "FontStyling",
      font_styling_lookup_table);
    int colors = config_file_get_enum("EditorSettings", "ChangeColors",
      change_colors_lookup_table);
    
    gchar *schemename = NULL;
    if(colors == CHANGE_COLORS_NEVER)
        schemename = g_strconcat("nocolor-", 
          styling == FONT_STYLING_NONE ? "nostyling" :
          styling == FONT_STYLING_SUBTLE ? "somestyling" : "styling", NULL);
    else
        schemename = g_strconcat(set == COLOR_SET_PSYCHEDELIC ? "psychedelic" :
          set == COLOR_SET_SUBDUED ? "subdued" :  "standard", "-",
          colors == CHANGE_COLORS_OCCASIONALLY ? "somecolor" : "color", "-",
          styling == FONT_STYLING_NONE ? "nostyling" :
          styling == FONT_STYLING_SUBTLE ? "somestyling" : "styling", NULL);
    
  	GtkSourceStyleScheme *scheme =
  	  gtk_source_style_scheme_manager_get_scheme(manager, schemename);
  	g_free(schemename);
  	return scheme;
}

/* Set up the style colors for the Natural Inform highlighting */
void 
set_highlight_styles(GtkSourceBuffer *buffer) 
{
    GtkSourceStyleScheme *scheme = get_style_scheme();
    gtk_source_buffer_set_style_scheme(buffer, scheme);
}

/* Return the GdkColor in the current scheme */
GdkColor 
get_scheme_color(int color) 
{
    GdkColor *scheme;
    switch(config_file_get_enum("EditorSettings", "ColorSet",
      color_set_lookup_table)) {
        case COLOR_SET_SUBDUED:
            scheme = scheme_subdued;
            break;
        case COLOR_SET_PSYCHEDELIC:
            scheme = scheme_psychedelic;
            break;
        case COLOR_SET_STANDARD:
        default:
            scheme = scheme_standard;
    }
    return scheme[color];
}
