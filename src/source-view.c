/* Copyright (C) 2008, 2009, 2010, 2011, 2015 P. F. Chimento
 * This file is part of GNOME Inform 7.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gtkspell/gtkspell.h>
#include "source-view.h"
#include "app.h"
#include "builder.h"
#include "elastic.h"
#include "error.h"

#define CONTENTS_FALLBACK_BG_COLOR "#FFFFBF"
#define CONTENTS_FALLBACK_FG_COLOR "black"
#define DEFAULT_ENGLISH_VARIANT "en_CA"  /* ha! */

/* TYPE SYSTEM */

typedef struct _I7SourceViewPrivate I7SourceViewPrivate;
struct _I7SourceViewPrivate
{
	GtkSpellChecker *spell;
};

#define I7_SOURCE_VIEW_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), I7_TYPE_SOURCE_VIEW, I7SourceViewPrivate))
#define I7_SOURCE_VIEW_USE_PRIVATE(o,n) I7SourceViewPrivate *n = I7_SOURCE_VIEW_PRIVATE(o)

static GtkFrameClass *parent_class = NULL;
G_DEFINE_TYPE(I7SourceView, i7_source_view, GTK_TYPE_FRAME);

static void
i7_source_view_init(I7SourceView *self)
{
	I7_SOURCE_VIEW_USE_PRIVATE(self, priv);

	/* Set private data */
	priv->spell = NULL;

	/* Build the interface */
	GFile *file = i7_app_get_data_file_va(i7_app_get(), "ui", "source.ui", NULL);
	GtkBuilder *builder = create_new_builder(file, self);
	g_object_unref(file);

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

	/* Change the color of the Contents page */
	GdkColor bg, fg;
	/* Look up the colors in the theme; if they aren't specified, use defaults */
	GtkStyle *style = gtk_rc_get_style(GTK_WIDGET(self->headings));
	if(!gtk_style_lookup_color(style, "text_color", &fg))
		gdk_color_parse(CONTENTS_FALLBACK_FG_COLOR, &fg);
	if(!gtk_style_lookup_color(style, "info_bg_color", &bg))
		gdk_color_parse(CONTENTS_FALLBACK_BG_COLOR, &bg);
	gtk_widget_modify_base(self->headings, GTK_STATE_NORMAL, &bg);
	gtk_widget_modify_base(self->message, GTK_STATE_NORMAL, &bg);

	gtk_range_set_value(GTK_RANGE(self->heading_depth), I7_DEPTH_PARTS_AND_HIGHER);

	/* Turn to the default page */
	gtk_notebook_set_current_page(GTK_NOTEBOOK(self->notebook), I7_SOURCE_VIEW_TAB_SOURCE);

	/* Builder object not needed anymore */
	g_object_unref(builder);
}

static void
i7_source_view_finalize(GObject *self)
{
	G_OBJECT_CLASS(parent_class)->finalize(self);
}

static void
i7_source_view_class_init(I7SourceViewClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = i7_source_view_finalize;

	parent_class = g_type_class_peek_parent(klass);

	g_type_class_add_private(klass, sizeof(I7SourceViewPrivate));
}

/* SIGNAL HANDLERS */

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

/* Helper function: run through the list of preferred system language in order
of preference until one is found that is a variant of English. If one is not
found, then return the default English variant. */
static const char *
get_nearest_system_language_to_english(void)
{
	const char * const *system_languages = g_get_language_names();
	const char * const *chosen_language;
	const char *default_language = DEFAULT_ENGLISH_VARIANT;

	for (chosen_language = system_languages; *chosen_language != NULL; chosen_language++) {
		if (g_str_has_prefix(*chosen_language, "en"))
			break;
	}
	if (*chosen_language == NULL)
		chosen_language = &default_language;
	return *chosen_language;
}

void
i7_source_view_set_spellcheck(I7SourceView *self, gboolean spellcheck)
{
	I7_SOURCE_VIEW_USE_PRIVATE(self, priv);

	if(spellcheck) {
		GError *error = NULL;
		const char *language = get_nearest_system_language_to_english();

		priv->spell = gtk_spell_checker_new();
		/* Fail relatively quietly if there's a problem */
		if(!gtk_spell_checker_set_language(priv->spell, language, &error)) {
			g_warning("Error initializing spell checking: %s. Is your spelling "
				"dictionary installed?", error->message);
			g_error_free(error);
		}
		if (!gtk_spell_checker_attach(priv->spell, GTK_TEXT_VIEW(self->source)))
			g_warning("Error initializing spell checking. Is your spelling "
				"dictionary installed?");
	} else {
		g_clear_pointer(&priv->spell, gtk_spell_checker_detach);
	}
}

void
i7_source_view_check_spelling(I7SourceView *self)
{
	I7_SOURCE_VIEW_USE_PRIVATE(self, priv);
	if(priv->spell)
		gtk_spell_checker_recheck_all(priv->spell);
}

void
i7_source_view_set_elastic_tabstops(I7SourceView *self, gboolean elastic)
{
	if(elastic) {
		add_elastic_tabstops_to_view(GTK_TEXT_VIEW(self->source));
		elastic_recalculate_view(GTK_TEXT_VIEW(self->source));
	} else
		remove_elastic_tabstops_from_view(GTK_TEXT_VIEW(self->source));
}
