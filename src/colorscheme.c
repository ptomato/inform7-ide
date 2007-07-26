/*  Copyright 2006 P.F. Chimento
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
 
#include <gnome.h>
#include <gtksourceview/gtksourcelanguage.h>

#include "colorscheme.h"
#include "configfile.h"
#include "prefs.h"

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

/* Set up the style colors for the Natural Inform highlighting */
void set_highlight_styles(GtkSourceLanguage *language) {
    GtkSourceTagStyle *style;
    
    int styling = config_file_get_int("Fonts", "FontStyling");
    int colors = config_file_get_int("Colors", "ChangeColors");
    
    style = gtk_source_language_get_tag_style(language, "Comment");
    style->foreground = get_scheme_color((colors == CHANGE_COLORS_NEVER)?
      CLR_TEXT : CLR_COMMENT);
    style->bold = FALSE;
    style->italic = (styling == FONT_STYLING_OFTEN);
    gtk_source_language_set_tag_style(language, "Comment", style);
    gtk_source_language_set_tag_style(language, "Documentation", style);
    gtk_source_tag_style_free(style);

    style = gtk_source_language_get_tag_style(language, "I6 Code");
    style->foreground = get_scheme_color((colors == CHANGE_COLORS_NEVER)?
      CLR_TEXT : CLR_I6_CODE);
    style->bold = FALSE;
    style->italic = (styling != FONT_STYLING_NONE);
    gtk_source_language_set_tag_style(language, "I6 Code", style);
    gtk_source_tag_style_free(style);
    
    style = gtk_source_language_get_tag_style(language, "String");
    style->foreground = get_scheme_color((colors == CHANGE_COLORS_NEVER)?
      CLR_TEXT : CLR_STRING);
    style->bold = (styling != FONT_STYLING_NONE);
    style->italic = FALSE;
    gtk_source_language_set_tag_style(language, "String", style);
    gtk_source_tag_style_free(style);

    /* Markup 
    style = gtk_source_language_get_tag_style(language, "Markup");
    style->foreground = get_scheme_color((colors == CHANGE_COLORS_NEVER)?
      CLR_TEXT : ((colors == CHANGE_COLORS_OCCASIONALLY)?
      CLR_STRING : CLR_MARKUP));
    style->bold = FALSE;
    style->italic = (styling == FONT_STYLING_OFTEN);
    gtk_source_language_set_tag_style(language, "Markup", style);
    gtk_source_tag_style_free(style);
    */

    /* For Inform 6 only - use the markup color, for lack of a better one */
    style = gtk_source_language_get_tag_style(language, "Number");
    style->foreground = get_scheme_color((colors == CHANGE_COLORS_NEVER)?
      CLR_TEXT : CLR_STRING_MARKUP);
    style->bold = FALSE;
    style->italic = (styling == FONT_STYLING_OFTEN);
    gtk_source_language_set_tag_style(language, "Character Constant", style);
    gtk_source_language_set_tag_style(language, "Decimal", style);
    gtk_source_language_set_tag_style(language, "Hexadecimal", style);
    gtk_source_language_set_tag_style(language, "Binary", style);
    gtk_source_tag_style_free(style);

    style = gtk_source_language_get_tag_style(language, "Keyword");
    style->foreground = get_scheme_color((colors == CHANGE_COLORS_OFTEN)?
      CLR_KEYWORD : CLR_TEXT);
    style->bold = (styling == FONT_STYLING_OFTEN);
    style->italic = FALSE;
    gtk_source_language_set_tag_style(language, "Keyword", style);
    gtk_source_tag_style_free(style);

    style = gtk_source_tag_style_new();
    style->mask = 0;
    style->bold = (styling != FONT_STYLING_NONE);
    style->italic = (styling != FONT_STYLING_NONE);
    gtk_source_language_set_tag_style(language, "Heading", style);
    gtk_source_tag_style_free(style);
}

/* Return the GdkColor in the current scheme */
GdkColor get_scheme_color(int color) {
    GdkColor *scheme;
    int set = config_file_get_int("Colors", "ColorSet");
    switch(set) {
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
