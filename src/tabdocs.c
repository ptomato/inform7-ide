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

#include "support.h"

#include "appwindow.h"
#include "datafile.h"
#include "history.h"
#include "html.h"
#include "story.h"
#include "tabdocs.h"

void
on_docs_contents_l_clicked(GtkToolButton *toolbutton, gpointer data)
{
    gchar *file = get_datafile_path_va("Documentation", "index.html", NULL);
    html_load_file(GTK_HTML(lookup_widget(GTK_WIDGET(toolbutton), "docs_l")),
      file);
    history_push_docpage(get_story(GTK_WIDGET(toolbutton)), LEFT, file);
    g_free(file);

    gtk_notebook_set_current_page(get_notebook(GTK_WIDGET(toolbutton), LEFT), 
      TAB_DOCUMENTATION);
}

void
on_docs_contents_r_clicked             (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
    gchar *file = get_datafile_path_va("Documentation", "index.html", NULL);
    html_load_file(GTK_HTML(lookup_widget(GTK_WIDGET(toolbutton), "docs_r")),
      file);
    history_push_docpage(get_story(GTK_WIDGET(toolbutton)), RIGHT, file);
    g_free(file);

    gtk_notebook_set_current_page(get_notebook(GTK_WIDGET(toolbutton), RIGHT), 
      TAB_DOCUMENTATION);
}
