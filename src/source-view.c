/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Philip Chimento <philip.chimento@gmail.com>
 */

#include "config.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <gspell/gspell.h>
#include <gtk/gtk.h>

#include "builder.h"
#include "configfile.h"
#include "elastic.h"
#include "source-view.h"

/* TYPE SYSTEM */

static GtkFrameClass *parent_class = NULL;
G_DEFINE_TYPE(I7SourceView, i7_source_view, GTK_TYPE_FRAME);

static void
i7_source_view_init(I7SourceView *self)
{
	/* Build the interface */
	g_autoptr(GtkBuilder) builder = gtk_builder_new_from_resource("/com/inform7/IDE/ui/source.ui");
	gtk_builder_connect_signals(builder, self);

	/* Make our base-class frame invisible */
	gtk_frame_set_label(GTK_FRAME(self), NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(self), GTK_SHADOW_NONE);

	/* Reparent the widget into our new frame */
	self->notebook = GTK_WIDGET(load_object(builder, "source_notebook"));
	gtk_container_add(GTK_CONTAINER(self), self->notebook);

	/* Save public pointers to specific widgets */
	self->source = GTK_WIDGET(load_object(builder, "source"));
	self->headings = GTK_WIDGET(load_object(builder, "headings"));
	self->heading_depth = GTK_WIDGET(load_object(builder, "heading_depth"));
	self->message = GTK_WIDGET(load_object(builder, "message"));
	self->previous = GTK_WIDGET(load_object(builder, "previous"));
	self->next = GTK_WIDGET(load_object(builder, "next"));

	gtk_range_set_value(GTK_RANGE(self->heading_depth), I7_DEPTH_PARTS_AND_HIGHER);

	GtkStyleContext *style = gtk_widget_get_style_context(GTK_WIDGET(self->source));
	gtk_style_context_add_class(style, "font-family-setting");
	gtk_style_context_add_class(style, "font-size-setting");

	/* Turn to the default page */
	gtk_notebook_set_current_page(GTK_NOTEBOOK(self->notebook), I7_SOURCE_VIEW_TAB_SOURCE);
}

static void
i7_source_view_finalize(GObject *object)
{
	G_OBJECT_CLASS(parent_class)->finalize(object);
}

static void
i7_source_view_class_init(I7SourceViewClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = i7_source_view_finalize;

	parent_class = g_type_class_peek_parent(klass);
}

/* SIGNAL HANDLERS */

static void
on_config_elastic_tabstops_changed(GSettings *prefs, const char *key, I7SourceView *self)
{
	if (g_settings_get_boolean(prefs, key)) {
		add_elastic_tabstops_to_view(GTK_TEXT_VIEW(self->source));
		elastic_recalculate_view(GTK_TEXT_VIEW(self->source));
	} else {
		remove_elastic_tabstops_from_view(GTK_TEXT_VIEW(self->source));
	}
}

gchar *
on_heading_depth_format_value(GtkScale *scale, gdouble value)
{
	if(value < I7_DEPTH_VOLUMES_AND_BOOKS)
		return g_strdup(_("Volumes only"));
	if(value < I7_DEPTH_PARTS_AND_HIGHER)
		return g_strdup(_("Volumes and Books"));
	if(value < I7_DEPTH_CHAPTERS_AND_HIGHER)
		return g_strdup(_("Parts and higher"));
	if(value < I7_DEPTH_ALL_HEADINGS)
		return g_strdup(_("Chapters and higher"));
	return g_strdup(_("All headings"));
}

/* PUBLIC FUNCTIONS */

GtkWidget *
i7_source_view_new()
{
	return GTK_WIDGET(g_object_new(I7_TYPE_SOURCE_VIEW, NULL));
}

void
i7_source_view_bind_settings(I7SourceView *self, GSettings *prefs)
{
	g_signal_connect(prefs, "changed::" PREFS_ELASTIC_TABSTOPS, G_CALLBACK(on_config_elastic_tabstops_changed), self);
}

void
i7_source_view_set_contents_display(I7SourceView *self, I7ContentsDisplay display)
{
	GtkWidget *headingswin = gtk_widget_get_parent(self->headings);
	GtkWidget *messagewin = gtk_widget_get_parent(self->message);
	switch(display) {
		case I7_CONTENTS_NORMAL:
			gtk_widget_show(headingswin);
			gtk_widget_hide(messagewin);
			break;
		case I7_CONTENTS_TOO_SHALLOW:
			gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(self->message)),
				_("No headings are visible at this level. Drag the slider below"
				" to the right to make the headings in the source text visible."), -1);
			gtk_widget_hide(headingswin);
			gtk_widget_show(messagewin);
			break;
		case I7_CONTENTS_NO_HEADINGS:
			gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(self->message)),
				_("Larger Inform projects are usually divided up with headings "
				"like 'Chapter 2 - Into the Forest'. This page automatically "
				"displays those headings as a Table of Contents, but since "
				"there are no headings in this project yet, there is nothing to"
				" see."), -1);
			gtk_widget_hide(headingswin);
			gtk_widget_show(messagewin);
	}
}

void
i7_source_view_jump_to_line(I7SourceView *self, guint line)
{
	GtkTextView *view = GTK_TEXT_VIEW(self->source);
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(view);
	GtkTextIter cursor, line_end;

	gtk_text_buffer_get_iter_at_line(buffer, &cursor, line - 1);
	/* line is counted from 0 */
	line_end = cursor;
	if(!gtk_text_iter_ends_line(&line_end))
		/* if already at end, this will push it to the end of the NEXT line */
		gtk_text_iter_forward_to_line_end(&line_end);
	gtk_text_buffer_select_range(buffer, &cursor, &line_end);
	gtk_text_view_scroll_to_mark(view, gtk_text_buffer_get_insert(buffer), 0.25, FALSE, 0.0, 0.0);
	gtk_widget_grab_focus(GTK_WIDGET(view));
}

void
i7_source_view_set_spellcheck(I7SourceView *self, gboolean spellcheck)
{
	GspellTextView *spell_view = gspell_text_view_get_from_gtk_text_view(GTK_TEXT_VIEW(self->source));
	gspell_text_view_set_inline_spell_checking(spell_view, spellcheck);
}
