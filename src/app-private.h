/*  Copyright (C) 2008, 2009, 2010, 2011 P. F. Chimento
 *  This file is part of GNOME Inform 7.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _APP_PRIVATE_H_
#define _APP_PRIVATE_H_

#include <glib.h>
#include <gtk/gtk.h>
#include "app.h"

typedef struct {
	/* Action Groups */
	GtkActionGroup *app_action_group;
	/* List of open documents */
	GSList *document_list;
	/* Application directories */
	gchar *datadir;
	gchar *pixmapdir;
	gchar *libexecdir;
	/* File monitor for extension directory */
	GFileMonitor *extension_dir_monitor;
	/* Tree of installed extensions */
	GtkTreeStore *installed_extensions;
	/* Current print settings */
	GtkPrintSettings *print_settings;
	GtkPageSetup *page_setup;
	/* Preferences settings */
	GSettings *prefs_settings;
	GSettings *state_settings;
	GSettings *desktop_settings;
} I7AppPrivate;

#define I7_APP_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), I7_TYPE_APP, I7AppPrivate))
#define I7_APP_USE_PRIVATE(o,n) I7AppPrivate *n = I7_APP_PRIVATE(o)

#endif /* _APP_PRIVATE_H_ */
