#ifndef _ACTIONS_H_
#define _ACTIONS_H_

#include <glib.h>
#include <gtk/gtk.h>

void on_open_extension_readonly_activate(GtkMenuItem *menuitem, gchar *path);
void on_open_extension_activate(GtkMenuItem *menuitem, gchar *path);

#endif /* _ACTIONS_H_ */
