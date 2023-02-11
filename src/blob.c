/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Philip Chimento <philip.chimento@gmail.com>
 */

#include "config.h"

#include <stdbool.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "blob.h"

struct _I7Blob {
	GtkStack parent;

	/* template children */
	GtkLabel *build_number;
	GtkButton *cancel;
	GtkProgressBar *progress;
	GtkLabel *status;
	GtkButton *stop;

	/* private */
	GCancellable *cancel_target;  /* owns ref */
	bool has_status : 1;
};

G_DEFINE_TYPE(I7Blob, i7_blob, GTK_TYPE_STACK);

#define PAGE_MAIN "main"
#define PAGE_STATUS "status"
#define PAGE_PROGRESS "progress"

/* SIGNAL HANDLERS */

static void
on_cancel_clicked(GtkButton *button, I7Blob *self)
{
	gtk_widget_set_sensitive(GTK_WIDGET(self->cancel), FALSE);
	g_cancellable_cancel(self->cancel_target);
}

/* TYPE SYSTEM */

static void
i7_blob_init(I7Blob *self)
{
	gtk_widget_init_template(GTK_WIDGET(self));

	g_autofree char *build_label = g_strdup_printf(_("Build %s"), INFORM7_VERSION);
	gtk_label_set_label(self->build_number, build_label);
}

static void
i7_blob_dispose(GObject *object)
{
	I7Blob *self = I7_BLOB(object);

	g_clear_object(&self->cancel_target);

	G_OBJECT_CLASS(i7_blob_parent_class)->dispose(object);
}

static void
i7_blob_class_init(I7BlobClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->dispose = i7_blob_dispose;

	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	gtk_widget_class_set_template_from_resource(widget_class, "/com/inform7/IDE/ui/blob.ui");
	gtk_widget_class_bind_template_child(widget_class, I7Blob, build_number);
	gtk_widget_class_bind_template_child(widget_class, I7Blob, cancel);
	gtk_widget_class_bind_template_child(widget_class, I7Blob, progress);
	gtk_widget_class_bind_template_child(widget_class, I7Blob, status);
	gtk_widget_class_bind_template_child(widget_class, I7Blob, stop);
	gtk_widget_class_bind_template_callback(widget_class, on_cancel_clicked);
}

/* PUBLIC API */

I7Blob *
i7_blob_new(void)
{
	return I7_BLOB(g_object_new(I7_TYPE_BLOB, NULL));
}

void
i7_blob_set_status(I7Blob *self, const char *status, bool show_stop_button)
{
	gtk_label_set_label(self->status, status);
	gtk_stack_set_visible_child_name(GTK_STACK(self), PAGE_STATUS);
	self->has_status = true;
	if (show_stop_button)
		gtk_widget_show(GTK_WIDGET(self->stop));
	else
		gtk_widget_hide(GTK_WIDGET(self->stop));
}

void
i7_blob_set_progress(I7Blob *self, double fraction, GCancellable *cancel_target)
{
	gtk_progress_bar_set_fraction(self->progress, fraction);
	gtk_stack_set_visible_child_name(GTK_STACK(self), PAGE_PROGRESS);

	if (cancel_target != NULL) {
		self->cancel_target = g_object_ref(cancel_target);
		gtk_widget_show(GTK_WIDGET(self->cancel));
		gtk_widget_set_sensitive(GTK_WIDGET(self->cancel), TRUE);
	} else {
		g_clear_object(&self->cancel_target);
		gtk_widget_hide(GTK_WIDGET(self->cancel));
	}
}

void
i7_blob_pulse_progress(I7Blob *self)
{
	gtk_progress_bar_pulse(self->progress);
	gtk_stack_set_visible_child_name(GTK_STACK(self), PAGE_PROGRESS);
}

void
i7_blob_clear_progress(I7Blob *self)
{
	gtk_progress_bar_set_fraction(self->progress, 0.0);
	gtk_stack_set_visible_child_name(GTK_STACK(self), self->has_status ? PAGE_STATUS : PAGE_MAIN);
}
