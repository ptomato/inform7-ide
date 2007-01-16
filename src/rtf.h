/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#ifndef RTF_H
#define RTF_H

#include <gnome.h>

void    gtk_text_buffer_set_rtf_text   (GtkTextBuffer *buffer,
                                        const gchar *rtf_text);

gchar * gtk_text_buffer_get_rtf_text   (GtkTextBuffer *buffer,
                                        const GtkTextIter *start,
                                        const GtkTextIter *end);

#endif /* RTF_H */
