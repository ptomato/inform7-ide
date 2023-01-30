/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Philip Chimento <philip.chimento@gmail.com>
 */

#include "config.h"

#include <stdbool.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "toast.h"

#define TIMEOUT_MS 2000

struct _I7Toast {
	GtkRevealer parent;

	/* template child */
	GtkLabel *message_label;

	/* private */
	unsigned timeout_id;
};

G_DEFINE_TYPE(I7Toast, i7_toast, GTK_TYPE_REVEALER);

/* CALLBACKS */

static gboolean
on_timeout(I7Toast *self)
{
	if (self->timeout_id) {
		g_source_remove(self->timeout_id);
		self->timeout_id = 0;
	}
	gtk_revealer_set_reveal_child(GTK_REVEALER(self), FALSE);
	return G_SOURCE_REMOVE;
}

void
on_toast_child_revealed_notify(GtkRevealer *self)
{
	if (!gtk_revealer_get_child_revealed(self))
		gtk_widget_hide(GTK_WIDGET(self));
}

/* TYPE SYSTEM */

static void
i7_toast_init(I7Toast *self)
{
	gtk_widget_init_template(GTK_WIDGET(self));
}

static void
i7_toast_class_init(I7ToastClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	gtk_widget_class_set_template_from_resource(widget_class, "/com/inform7/IDE/ui/toast.ui");
	gtk_widget_class_bind_template_child(widget_class, I7Toast, message_label);
	gtk_widget_class_bind_template_callback(widget_class, on_toast_child_revealed_notify);
}

/* PUBLIC API */

I7Toast *
i7_toast_new(void)
{
	return I7_TOAST(g_object_new(I7_TYPE_TOAST,
		"transition-type", GTK_REVEALER_TRANSITION_TYPE_CROSSFADE,
		"transition-duration", 200 /* ms */,
		NULL));
}

void
i7_toast_show_message(I7Toast *self, const char *message)
{
	gtk_label_set_label(GTK_LABEL(self->message_label), message);
	gtk_widget_show(GTK_WIDGET(self));
	gtk_revealer_set_reveal_child(GTK_REVEALER(self), TRUE);

	if (self->timeout_id == 0)
		self->timeout_id = gdk_threads_add_timeout(TIMEOUT_MS, (GSourceFunc)on_timeout, self);
}
