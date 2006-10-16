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
 
#ifndef HTML_H
#define HTML_H

#include <gnome.h>
#include <gtkhtml/gtkhtml.h>
#include <gtkhtml/gtkhtml-stream.h>

struct source_entry {
    GtkHTML *html;
    gchar *source;
};

void source_table_add(GtkHTML *html, gchar *source);
void source_table_remove(GtkHTML *html);
void source_table_update(GtkHTML *html, gchar *source);
gchar *source_table_get(GtkHTML *html);

GtkWidget *create_html(gchar *widget_name, gchar *string1, gchar *string2,
  gint int1, gint int2);
void html_load_file(GtkHTML *html, const gchar *filename);
void html_load_blank(GtkHTML *html);
void html_refresh(GtkHTML *html);

void on_url_requested(GtkHTML *html, const gchar *url, GtkHTMLStream *handle,
  gpointer data);
void on_link_clicked(GtkHTML *html, const gchar *requested_url, gpointer data);
void on_html_destroy(GtkObject *widget, gpointer data);
gchar *get_base_name(const gchar *url);
gchar *javascript_find_paste_code(const gchar *source,
  const gchar *function_call);

#endif
