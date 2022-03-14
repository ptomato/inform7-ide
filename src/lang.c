/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2008 Zachary Amsden
 */

#include "config.h"

#include <stdlib.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>

#include "app.h"
#include "error.h"
#include "lang.h"

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
	I7App *theapp = I7_APP(g_application_get_default());
	GFile *languages_dir = i7_app_get_data_file(theapp, "highlighting");
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
