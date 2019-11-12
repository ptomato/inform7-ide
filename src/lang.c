/* Copyright (C) 2008 Zachary Amsden
 * Copyright (C) 2018 Philip Chimento
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

#include <stdlib.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>
#include "lang.h"
#include "app.h"
#include "error.h"

void
set_buffer_language(GtkSourceBuffer *buffer, gchar *lang)
{
	/* Set up the Natural Inform highlighting */
	GtkSourceLanguage *language;
	GtkSourceLanguageManager *lmanager;
	const gchar* const *paths;
	gchar **mypaths;
	int dirs, i;

	lmanager = gtk_source_language_manager_new();

	/* Get and count the default paths, then add our custom language
	definitions to the set. */
	paths = gtk_source_language_manager_get_search_path(lmanager);
	for(dirs = 0; paths[dirs]; dirs++);

	mypaths = g_new0(gchar *, dirs + 2);

	for(i = 0; i < dirs; i++)
		mypaths[i] = g_strdup(paths[i]);

	/* Get data dir */
	GFile *languages_dir = i7_app_get_data_file(i7_app_get(), "highlighting");
	mypaths[i++] = g_file_get_path(languages_dir);
	g_object_unref(languages_dir);
	mypaths[i] = NULL;
	gtk_source_language_manager_set_search_path(lmanager, mypaths);

	g_strfreev(mypaths);

	language = gtk_source_language_manager_get_language(lmanager, lang);
	if(language != NULL)
		gtk_source_buffer_set_language(buffer, language);
	else
		error_dialog(NULL, NULL, _("Cannot load %s source-language highlighting definition"), lang);
	g_object_unref(lmanager);
}
