/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2022 Philip Chimento <philip.chimento@gmail.com>
 */

#include "config.h"

#include <stdbool.h>

#include <glib-object.h>
#include <goocanvas.h>
#include <gtk/gtk.h>

#include "node.h"
#include "skein.h"
#include "transcript-entry.h"

struct _I7TranscriptEntry {
	GtkGrid parent;

	I7Node *node;  /* owns a reference */
	I7Skein *skein;  /* owns a reference */
	GBinding *command_text_binding;  /* owned */

	/* Unowned widget pointers (owned by parent GtkGrid) */
	GtkLabel *command_label;
	GtkLabel *transcript_label;
	GtkLabel *expected_label;
	GtkRevealer *bless_revealer;

	unsigned long on_skein_notify_current_node_handler;
	unsigned long on_node_notify_transcript_text_handler;
	unsigned long on_node_notify_expected_text_handler;
};

G_DEFINE_TYPE(I7TranscriptEntry, i7_transcript_entry, GTK_TYPE_GRID)

enum  {
	PROP_0,
	PROP_NODE,
};

static void
update_text(I7TranscriptEntry *self)
{
	const char *transcript = i7_node_get_transcript_pango_string(self->node);
	gtk_label_set_markup(self->transcript_label, transcript);

	const char *expected = i7_node_get_expected_pango_string(self->node);
	gtk_label_set_markup(self->expected_label, expected);

	GtkStyleContext *style = gtk_widget_get_style_context(GTK_WIDGET(self->transcript_label));
	if (i7_node_get_changed(self->node)) {
		gtk_style_context_add_class(style, "changed");
		gtk_style_context_remove_class(style, "unchanged");
	} else {
		gtk_style_context_add_class(style, "unchanged");
		gtk_style_context_remove_class(style, "changed");
	}

	style = gtk_widget_get_style_context(GTK_WIDGET(self->expected_label));
	switch (i7_node_get_match_type(self->node)) {
		case I7_NODE_CANT_COMPARE:
			gtk_style_context_add_class(style, "no-expected");
			gtk_style_context_remove_class(style, "no-match");
			gtk_style_context_remove_class(style, "near-match");
			gtk_style_context_remove_class(style, "exact-match");
			gtk_revealer_set_reveal_child(self->bless_revealer, TRUE);
			break;
		case I7_NODE_NO_MATCH:
			gtk_style_context_add_class(style, "no-match");
			gtk_style_context_remove_class(style, "no-expected");
			gtk_style_context_remove_class(style, "near-match");
			gtk_style_context_remove_class(style, "exact-match");
			gtk_revealer_set_reveal_child(self->bless_revealer, TRUE);
			break;
		case I7_NODE_NEAR_MATCH:
			gtk_style_context_add_class(style, "near-match");
			gtk_style_context_remove_class(style, "no-expected");
			gtk_style_context_remove_class(style, "no-match");
			gtk_style_context_remove_class(style, "exact-match");
			gtk_revealer_set_reveal_child(self->bless_revealer, TRUE);
			break;
		case I7_NODE_EXACT_MATCH:
		default:
			gtk_style_context_add_class(style, "exact-match");
			gtk_style_context_remove_class(style, "no-expected");
			gtk_style_context_remove_class(style, "no-match");
			gtk_style_context_remove_class(style, "near-match");
			gtk_revealer_set_reveal_child(self->bless_revealer, FALSE);
			break;
	}
}

static void
set_played(I7TranscriptEntry *self)
{
	bool played = (i7_skein_get_current_node(self->skein) == self->node);

	/* Draw a border around the last played node */
	GtkStyleContext *style = gtk_widget_get_style_context(GTK_WIDGET(self));
	if (played)
		gtk_style_context_add_class(style, "last-played");
	else
		gtk_style_context_remove_class(style, "last-played");
}

static void
set_node(I7TranscriptEntry *self, I7Node *node)
{
	self->node = g_object_ref(node);
	self->skein = g_object_ref(I7_SKEIN(goo_canvas_item_model_get_parent(GOO_CANVAS_ITEM_MODEL(node))));

	g_autofree char *command = i7_node_get_command(node);
	g_object_bind_property(node, "command", self->command_label, "label", G_BINDING_SYNC_CREATE);

	self->on_node_notify_transcript_text_handler =
		g_signal_connect_swapped(node, "notify::transcript-text", G_CALLBACK(update_text), self);
	self->on_node_notify_expected_text_handler =
		g_signal_connect_swapped(node, "notify::expected-text", G_CALLBACK(update_text), self);

	update_text(self);

	self->on_skein_notify_current_node_handler =
		g_signal_connect_swapped(self->skein, "notify::current-node", G_CALLBACK(set_played), self);

	set_played(self);
}

/* GTKBUILDER SIGNAL HANDLERS */

static void
on_transcript_entry_blessed(I7TranscriptEntry *self)
{
	i7_node_bless(self->node);
}

static void
on_transcript_entry_play_to_here(I7TranscriptEntry *self)
{
	g_signal_emit_by_name(self->skein, "node-activate", self->node);
}

static void
on_transcript_entry_show_knot(I7TranscriptEntry *self)
{
	g_signal_emit_by_name(self->skein, "show-node", I7_REASON_TRANSCRIPT, self->node);
}

/* TYPE SYSTEM */

static void
i7_transcript_entry_init(I7TranscriptEntry *self)
{
	gtk_widget_init_template(GTK_WIDGET(self));
}

static void
i7_transcript_entry_set_property(GObject *object, unsigned prop_id, const GValue *value, GParamSpec *pspec)
{
	I7TranscriptEntry *self = I7_TRANSCRIPT_ENTRY(object);

	switch(prop_id) {
		case PROP_NODE:
			set_node(self, g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(self, prop_id, pspec);
	}
}

static void
i7_transcript_entry_get_property(GObject *object, unsigned prop_id, GValue *value, GParamSpec *pspec)
{
	I7TranscriptEntry *self = I7_TRANSCRIPT_ENTRY(object);

	switch(prop_id) {
		case PROP_NODE:
			g_value_set_object(value, self->node);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(self, prop_id, pspec);
	}
}

static void
i7_transcript_entry_dispose(GObject *object)
{
	I7TranscriptEntry *self = I7_TRANSCRIPT_ENTRY(object);

	g_clear_signal_handler(&self->on_skein_notify_current_node_handler, self->skein);
	g_clear_signal_handler(&self->on_node_notify_transcript_text_handler, self->node);
	g_clear_signal_handler(&self->on_node_notify_expected_text_handler, self->node);

	g_clear_object(&self->command_text_binding);
	g_clear_object(&self->node);
	g_clear_object(&self->skein);

	G_OBJECT_CLASS(i7_transcript_entry_parent_class)->dispose(object);
}

static void
i7_transcript_entry_class_init(I7TranscriptEntryClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	gtk_widget_class_set_template_from_resource(widget_class, "/com/inform7/IDE/ui/transcript-entry.ui");
	gtk_widget_class_bind_template_child(widget_class, I7TranscriptEntry, command_label);
	gtk_widget_class_bind_template_child(widget_class, I7TranscriptEntry, transcript_label);
	gtk_widget_class_bind_template_child(widget_class, I7TranscriptEntry, expected_label);
	gtk_widget_class_bind_template_child(widget_class, I7TranscriptEntry, bless_revealer);
	gtk_widget_class_bind_template_callback(widget_class, on_transcript_entry_blessed);
	gtk_widget_class_bind_template_callback(widget_class, on_transcript_entry_play_to_here);
	gtk_widget_class_bind_template_callback(widget_class, on_transcript_entry_show_knot);

	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->get_property = i7_transcript_entry_get_property;
	object_class->set_property = i7_transcript_entry_set_property;
	object_class->dispose = i7_transcript_entry_dispose;

	g_object_class_install_property(object_class, PROP_NODE,
		g_param_spec_object("node", "Node", "Skein node", I7_TYPE_NODE,
			G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
}

GtkWidget *
i7_transcript_entry_new(I7Node *node)
{
	g_return_val_if_fail(node != NULL, NULL);
	return g_object_new(I7_TYPE_TRANSCRIPT_ENTRY,
		"node", node,
		NULL);
}
