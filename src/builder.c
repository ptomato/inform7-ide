#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include "builder.h"
#include "error.h"

/* Miscellaneous GtkBuilder-related functions and macros; also, the arbitrary
 home of the START_TIMER and STOP_TIMER macros */

/* SUCKY DEBIAN: When Debian updates to GTK 2.14, then move all the GtkBuilder
 interface definitions into one file and use gtk_builder_add_objects_from_file()
 to build the appropriate ones */

/* Create a new GtkBuilder from an interface definition file and connect its
 signals */
GtkBuilder *
create_new_builder(const gchar *filename, gpointer data)
{
	GError *error = NULL;
	GtkBuilder *builder;
	
	builder = gtk_builder_new();
	if(!gtk_builder_add_from_file(builder, filename, &error))
		ERROR(_("Error while building interface"), error);
	gtk_builder_connect_signals(builder, data);
		
	return builder;
}

/* Return a pointer to the object called @name constructed by @builder */
GObject *
load_object(GtkBuilder *builder, const gchar *name)
{
    GObject *retval;
    if(G_UNLIKELY((retval = gtk_builder_get_object(builder, name)) == NULL))
        g_error(_("Error while getting object '%s' during interface building"), name);
    return retval;
}

/* Add actions constructed by @builder to an empty action group also constructed
 by @builder. @group_name is the name of the action group, and @action_names is
 an array of strings: alternating action names and accelerators. @action_names
 must also be terminated by NULL. A pointer to the action group is placed into
 @group. */
void
add_actions(GtkBuilder *builder, GtkActionGroup **group, const gchar *group_name, const gchar **action_names)
{
	const gchar **ptr;
	*group = GTK_ACTION_GROUP(load_object(builder, group_name));
	for(ptr = action_names; *ptr; ptr += 2)
		gtk_action_group_add_action_with_accel(*group, GTK_ACTION(load_object(builder, ptr[0])), ptr[1]);
}
