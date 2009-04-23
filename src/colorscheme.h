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
 
#ifndef COLOR_SCHEME_H
#define COLOR_SCHEME_H

#include <gnome.h>
#include <gtksourceview/gtksourcebuffer.h>
#include <gtksourceview/gtksourcelanguage.h>

/* All the different colors we use in the application */
enum {
    CLR_BACKGROUND = 0,
    CLR_TEXT,
    CLR_STRING,
    CLR_STRING_MARKUP,
    CLR_KEYWORD,
    CLR_COMMENT,
    CLR_ERROR,
    CLR_HIGHLIGHT,
    CLR_LOCKED,
    CLR_UNLOCKED,
    CLR_SKEIN_INPUT,
    CLR_I6_CODE,
    CLR_TRANS_CHANGED,
    CLR_TRANS_UNCHANGED,
    CLR_TRANS_INPUT,
    CLR_TRANS_UNSET,
    CLR_LAST = CLR_TRANS_UNSET
};

GtkSourceStyleScheme *get_style_scheme(void);
void set_highlight_styles(GtkSourceBuffer *buffer);
GdkColor get_scheme_color(int color);

#endif
