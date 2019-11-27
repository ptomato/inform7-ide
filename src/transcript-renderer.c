/* Copyright (C) 2011, 2015, 2019 P. F. Chimento
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

#include "config.h"

#include <string.h>

#include <cairo.h>
#include <gdk/gdk.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <pango/pango.h>

#include "transcript-renderer.h"

/* Below this width, the outputs will likely wrap to the point of unreadability */
#define TRANSCRIPT_RENDERER_MIN_WIDTH 200

typedef enum {
	CANT_COMPARE = -1,
	NO_MATCH,
	NEAR_MATCH,
	EXACT_MATCH
} I7TranscriptMatchType; /* copy of I7NodeMatchType */

/* The private properties are the non-persistent renderer state; any call to 
i7_cell_renderer_transcript_render() must yield a cell of the same size for 
the same values of these properties */
typedef struct _I7CellRendererTranscriptPrivate I7CellRendererTranscriptPrivate;
struct _I7CellRendererTranscriptPrivate
{
	/* The width to make the whole entry */
	int default_width;
	/* The padding in between the box borders and the text */
	int text_padding;
	/* The text to render */
	char *command;
	char *transcript_text;
	char *expected_text;
	/* Match type of the transcript and expected text (I7NodeMatchType); */
	int match_type;
	/* Which borders to render */
	gboolean current;
	gboolean played;
	gboolean changed;
};

enum  {
	PROP_0,
	PROP_DEFAULT_WIDTH,
	PROP_TEXT_PADDING,
	PROP_COMMAND,
	PROP_TRANSCRIPT_TEXT,
	PROP_EXPECTED_TEXT,
	PROP_MATCH_TYPE,
	PROP_CURRENT,
	PROP_PLAYED,
	PROP_CHANGED
};

G_DEFINE_TYPE_WITH_PRIVATE(I7CellRendererTranscript, i7_cell_renderer_transcript, GTK_TYPE_CELL_RENDERER);

/* TYPE SYSTEM */

static void 
i7_cell_renderer_transcript_init(I7CellRendererTranscript *self) 
{
	I7CellRendererTranscriptPrivate *priv = i7_cell_renderer_transcript_get_instance_private(self);
	/* Default values of properties */
	priv->default_width = 400;
	priv->text_padding = 8;
	priv->command = NULL;
	priv->transcript_text = NULL;
	priv->expected_text = NULL;
	priv->match_type = CANT_COMPARE;
	priv->current = FALSE;
	priv->played = FALSE;
	priv->changed = FALSE;
}

static void 
i7_cell_renderer_transcript_set_property(GObject *object, unsigned prop_id, const GValue *value, GParamSpec *pspec)
{
	I7CellRendererTranscript *self = I7_CELL_RENDERER_TRANSCRIPT(object);
	I7CellRendererTranscriptPrivate *priv = i7_cell_renderer_transcript_get_instance_private(self);

	switch(prop_id) {
		case PROP_DEFAULT_WIDTH:
			priv->default_width = g_value_get_int(value);
			g_object_notify(object, "default-width");
			break;
		case PROP_TEXT_PADDING:
			priv->text_padding = g_value_get_int(value);
			g_object_notify(object, "text-padding");
			break;
		case PROP_COMMAND:
			g_free(priv->command);
			priv->command = g_strdup(g_value_get_string(value));
			g_object_notify(object, "command");
			break;
		case PROP_TRANSCRIPT_TEXT:
			g_free(priv->transcript_text);
			priv->transcript_text = g_strdup(g_value_get_string(value));
			g_object_notify(object, "transcript-text");
			break;
		case PROP_EXPECTED_TEXT:
			g_free(priv->expected_text);
			priv->expected_text = g_strdup(g_value_get_string(value));
			g_object_notify(object, "expected-text");
			break;
		case PROP_MATCH_TYPE:
			priv->match_type = g_value_get_int(value);
			g_object_notify(object, "match-type");
			break;
		case PROP_CURRENT:
			priv->current = g_value_get_boolean(value);
			g_object_notify(object, "current");
			break;
		case PROP_PLAYED:
			priv->played = g_value_get_boolean(value);
			g_object_notify(object, "played");
			break;
		case PROP_CHANGED:
			priv->changed = g_value_get_boolean(value);
			g_object_notify(object, "changed");
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(self, prop_id, pspec);
	}
}

static void 
i7_cell_renderer_transcript_get_property(GObject *object, unsigned prop_id, GValue *value, GParamSpec *pspec)
{
	I7CellRendererTranscript *self = I7_CELL_RENDERER_TRANSCRIPT(object);
	I7CellRendererTranscriptPrivate *priv = i7_cell_renderer_transcript_get_instance_private(self);

	switch(prop_id) {
		case PROP_DEFAULT_WIDTH:
			g_value_set_int(value, priv->default_width);
			break;
		case PROP_TEXT_PADDING:
			g_value_set_int(value, priv->text_padding);
			break;
		case PROP_COMMAND:
			g_value_set_string(value, priv->command);
			break;
		case PROP_TRANSCRIPT_TEXT:
			g_value_set_string(value, priv->transcript_text);
			break;
		case PROP_EXPECTED_TEXT:
			g_value_set_string(value, priv->expected_text);
			break;
		case PROP_MATCH_TYPE:
			g_value_set_int(value, priv->match_type);
			break;
		case PROP_CURRENT:
			g_value_set_boolean(value, priv->current);
			break;
		case PROP_PLAYED:
			g_value_set_boolean(value, priv->played);
			break;
		case PROP_CHANGED:
			g_value_set_boolean(value, priv->changed);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(self, prop_id, pspec);
	}
}

static void 
i7_cell_renderer_transcript_finalize(GObject* object) 
{
	I7CellRendererTranscript *self = I7_CELL_RENDERER_TRANSCRIPT(object);
	I7CellRendererTranscriptPrivate *priv = i7_cell_renderer_transcript_get_instance_private(self);

	g_free(priv->command);
	g_free(priv->transcript_text);
	g_free(priv->expected_text);

	G_OBJECT_CLASS(i7_cell_renderer_transcript_parent_class)->finalize(object);
}

static GtkSizeRequestMode
i7_cell_renderer_transcript_get_request_mode(GtkCellRenderer *renderer)
{
	return GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}

static void
i7_cell_renderer_transcript_get_preferred_width(GtkCellRenderer *renderer, GtkWidget *widget, int *min_width, int *natural_width)
{
	I7CellRendererTranscript *self = I7_CELL_RENDERER_TRANSCRIPT(renderer);
	I7CellRendererTranscriptPrivate *priv = i7_cell_renderer_transcript_get_instance_private(self);

	if (min_width)
		*min_width = TRANSCRIPT_RENDERER_MIN_WIDTH;
	if (natural_width)
		*natural_width = priv->default_width;
}

static void
i7_cell_renderer_transcript_get_preferred_height_for_width(GtkCellRenderer *renderer, GtkWidget *widget, int width, int *min_height, int *natural_height)
{
	I7CellRendererTranscript *self = I7_CELL_RENDERER_TRANSCRIPT(renderer);
	I7CellRendererTranscriptPrivate *priv = i7_cell_renderer_transcript_get_instance_private(self);

	PangoRectangle command_rect, transcript_rect, expected_rect;
	PangoLayout *layout;

	int xpad, ypad;
	gtk_cell_renderer_get_padding(renderer, &xpad, &ypad);

	int transcript_width = (width / 2) - xpad;

	/* Get size of command */
	layout = gtk_widget_create_pango_layout(widget, priv->command);
	pango_layout_get_pixel_extents(layout, NULL, &command_rect);
	g_object_unref(layout);

	/* Get size of transcript text */
	layout = gtk_widget_create_pango_layout(widget, priv->transcript_text);
	pango_layout_set_width(layout, (int)(transcript_width - priv->text_padding * 2) * PANGO_SCALE);
	pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);
	pango_layout_get_pixel_extents(layout, NULL, &transcript_rect);
	g_object_unref(layout);

	/* Get size of expected text */
	layout = gtk_widget_create_pango_layout(widget, priv->expected_text);
	pango_layout_set_width(layout, (int)(transcript_width - priv->text_padding * 2) * PANGO_SCALE);
	pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);
	pango_layout_get_pixel_extents(layout, NULL, &expected_rect);
	g_object_unref (layout);

	/* Calculate the required width and height for the cell */
	int calc_height = command_rect.height + MAX(transcript_rect.height, expected_rect.height) + ypad * 2 + priv->text_padding * 4;

	if (min_height)
		*min_height = calc_height;
	if (natural_height)
		*natural_height = calc_height;
}

static void 
i7_cell_renderer_transcript_render(GtkCellRenderer *renderer, cairo_t *cr, GtkWidget *widget, const GdkRectangle *background_area, const GdkRectangle *cell_area, GtkCellRendererState flags)
{
	I7CellRendererTranscript *self = I7_CELL_RENDERER_TRANSCRIPT(renderer);
	I7CellRendererTranscriptPrivate *priv = i7_cell_renderer_transcript_get_instance_private(self);

	PangoRectangle command_rect;
	PangoLayout *layout;
	GtkStyleContext *style = gtk_widget_get_style_context(widget);
	gtk_style_context_save(style);
	gtk_style_context_add_class(style, "transcript");

	/* Get the size and take the padding into account */
	int xpad, ypad;
	gtk_cell_renderer_get_padding(renderer, &xpad, &ypad);
	double x = cell_area->x + xpad;
	double y = cell_area->y + ypad;
	double width = cell_area->width - 2 * xpad;
	double height = cell_area->height - 2 * ypad;

	/* Draw the command */
	gtk_style_context_save(style);
	gtk_style_context_add_class(style, "command");

	layout = gtk_widget_create_pango_layout(widget, priv->command);
	pango_layout_get_pixel_extents(layout, NULL, &command_rect);

	double command_height = command_rect.height + priv->text_padding * 2;
	gtk_render_background(style, cr, x, y, width, command_height);
	gtk_render_frame(style, cr, x, y, width, command_height);
	gtk_render_layout(style, cr, x + priv->text_padding, y + priv->text_padding, layout);
	g_object_unref(layout);

	gtk_style_context_restore(style);

	/* Draw the transcript text */
	double transcript_width = width / 2;

	gtk_style_context_save(style);
	gtk_style_context_add_class(style, "actual");
	if(priv->changed)
		gtk_style_context_add_class(style, "changed");
	else
		gtk_style_context_add_class(style, "unchanged");

	double hline_y = y + command_height;
	gtk_render_background(style, cr, x, hline_y, transcript_width, height - command_height);
	gtk_render_frame(style, cr, x, hline_y, transcript_width, height - command_height);
	layout = gtk_widget_create_pango_layout(widget, NULL);
	pango_layout_set_markup(layout, priv->transcript_text, -1);
	pango_layout_set_width(layout, (transcript_width - priv->text_padding * 2) * PANGO_SCALE);
	pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);
	gtk_render_layout(style, cr, x + priv->text_padding, hline_y + priv->text_padding, layout);
	g_object_unref(layout);

	gtk_style_context_restore(style);  /* !actual */

	/* Draw the expected text */
	gtk_style_context_save(style);
	gtk_style_context_add_class(style, "expected");
	switch(priv->match_type) {
		case CANT_COMPARE:
			gtk_style_context_add_class(style, "no-expected");
			break;
		case NO_MATCH:
			gtk_style_context_add_class(style, "no-match");
			break;
		case NEAR_MATCH:
			gtk_style_context_add_class(style, "near-match");
			break;
		case EXACT_MATCH:
		default:
			gtk_style_context_add_class(style, "exact-match");
			break;
	}

	gtk_render_background(style, cr, x + transcript_width, hline_y,
		transcript_width, height - command_height);
	gtk_render_frame(style, cr, x + transcript_width, hline_y, transcript_width,
		height - command_height);
	layout = gtk_widget_create_pango_layout(widget, NULL);
	pango_layout_set_markup(layout, priv->expected_text, -1);
	pango_layout_set_width(layout, (transcript_width - priv->text_padding * 2) * PANGO_SCALE);
	pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);
	gtk_render_layout(style, cr, x + transcript_width + priv->text_padding,
		hline_y + priv->text_padding, layout);
	g_object_unref(layout);

	gtk_style_context_restore(style);  /* !expected */

	/* Draw a border around the highlighted node */
	if(priv->current) {
		gtk_style_context_save(style);
		gtk_style_context_add_class(style, "highlight");
		gtk_render_frame(style, cr, x, y, width, height);
		gtk_style_context_restore(style);
	}

	/* Draw a border around the active node */
	if(priv->played) {
		gtk_style_context_save(style);
		gtk_style_context_add_class(style, "active");
		gtk_render_frame(style, cr, x, y, width, height);
		gtk_style_context_restore(style);
	}

	gtk_style_context_restore(style);  /* !transcript */
}

static void 
i7_cell_renderer_transcript_class_init(I7CellRendererTranscriptClass *klass) 
{
	GtkCellRendererClass *renderer_class = GTK_CELL_RENDERER_CLASS(klass);
	renderer_class->get_request_mode = i7_cell_renderer_transcript_get_request_mode;
	renderer_class->get_preferred_width = i7_cell_renderer_transcript_get_preferred_width;
	renderer_class->get_preferred_height_for_width = i7_cell_renderer_transcript_get_preferred_height_for_width;
	renderer_class->render = i7_cell_renderer_transcript_render;
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->get_property = i7_cell_renderer_transcript_get_property;
	object_class->set_property = i7_cell_renderer_transcript_set_property;
	object_class->finalize = i7_cell_renderer_transcript_finalize;
	
	/* Install properties */
	GParamFlags flags = G_PARAM_LAX_VALIDATION | G_PARAM_STATIC_STRINGS;
	g_object_class_install_property(object_class, PROP_DEFAULT_WIDTH, 
		g_param_spec_int("default-width", "Default width",
			"The width to make the whole renderer",
			0, G_MAXINT, 400, G_PARAM_READWRITE | flags));
	g_object_class_install_property(object_class, PROP_TEXT_PADDING, 
		g_param_spec_int("text-padding", "Text padding",
			"Padding between the edges of the rectangles and the text",
			0, G_MAXINT, 6, G_PARAM_READWRITE | flags));
	g_object_class_install_property(object_class, PROP_COMMAND, 
		g_param_spec_string("command", "Command",
			"Command from the Skein",
			NULL, G_PARAM_READWRITE | flags));
	g_object_class_install_property(object_class, PROP_TRANSCRIPT_TEXT, 
		g_param_spec_string("transcript-text", "Transcript text",
			"Transcript text from the Skein",
			NULL, G_PARAM_READWRITE | flags));
	g_object_class_install_property(object_class, PROP_EXPECTED_TEXT, 
		g_param_spec_string("expected-text", "Expected text",
			"Expected text from the Skein",
			NULL, G_PARAM_READWRITE | flags));
	g_object_class_install_property(object_class, PROP_MATCH_TYPE,
	    g_param_spec_int("match-type", "Match type",
		    "-1 = no comparison, 0 = no match, 1 = near match, 2 = exact match",
		    -1, 2, -1, G_PARAM_READWRITE | flags));
	g_object_class_install_property(object_class, PROP_CURRENT,
	    g_param_spec_boolean("current", "Current",
		    "Whether to render the node as the currently highlighted node",
		    FALSE, G_PARAM_READWRITE | flags));
	g_object_class_install_property(object_class, PROP_PLAYED,
	    g_param_spec_boolean("played", "Played",
		    "Whether to render the node as the latest played node",
		    FALSE, G_PARAM_READWRITE | flags));
	g_object_class_install_property(object_class, PROP_CHANGED,
	    g_param_spec_boolean("changed", "Changed",
		    "Whether to render the node as having been changed since it was last played",
		    FALSE, G_PARAM_READWRITE | flags));
}

I7CellRendererTranscript *
i7_cell_renderer_transcript_new(void) 
{
	return g_object_new(I7_TYPE_CELL_RENDERER_TRANSCRIPT, NULL);
}
