#ifndef _BUILDER_H_
#define _BUILDER_H_

#include <glib.h>
#include <gtk/gtk.h>

GtkBuilder *create_new_builder(const gchar *filename, gpointer data);
GObject *load_object(GtkBuilder *builder, const gchar *name);
void add_actions(GtkBuilder *builder, GtkActionGroup **group, const gchar *group_name, const gchar **action_names);

/* Shortcuts for loading public widget and action pointers in _init() functions.
The object being init'ed must be called 'self'. */
#define LOAD_WIDGET(name) self->name = GTK_WIDGET(load_object(builder, G_STRINGIFY(name)))
#define LOAD_ACTION(group, name) self->name = gtk_action_group_get_action(group, G_STRINGIFY(name))

/* Arbitrarily putting this here; bracket a section of code to be timed in
 between START_TIMER and STOP_TIMER. */
#define START_TIMER \
	GTimeVal timer, timer2; \
	glong sec, usec; \
	g_get_current_time(&timer);
#define STOP_TIMER \
	g_get_current_time(&timer2); \
	sec = timer2.tv_sec - timer.tv_sec; \
	usec = timer2.tv_usec - timer.tv_usec; \
	if(usec < 0) { \
		usec += 1000000L; \
		sec--; \
	} \
	g_printerr("Time (%s): %ld.%06ld\n", G_STRFUNC, sec, usec);

#endif /* _BUILDER_H_ */