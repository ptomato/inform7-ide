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
	/* New project settings */
} I7AppPrivate;

#define I7_APP_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), I7_TYPE_APP, I7AppPrivate))
#define I7_APP_USE_PRIVATE(o,n) I7AppPrivate *n = I7_APP_PRIVATE(o)

#endif /* _APP_PRIVATE_H_ */