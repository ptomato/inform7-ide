/* Copyright (C) 2011, 2015 P. F. Chimento
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

typedef enum  {
	STYLE_UNPLAYED,
	STYLE_UNCHANGED,
	STYLE_CHANGED,
	STYLE_NO_EXPECTED,
	STYLE_NO_MATCH,
	STYLE_NEAR_MATCH,
	STYLE_EXACT_MATCH,
	STYLE_COMMAND,
	STYLE_HIGHLIGHT,
	STYLE_ACTIVE,
	STYLE_LAST
} I7TranscriptStyle;

typedef enum {
	CANT_COMPARE = -1,
	NO_MATCH,
	NEAR_MATCH,
	EXACT_MATCH
} I7TranscriptMatchType; /* copy of I7NodeMatchType */

typedef struct {
	double r;
	double g;
	double b;
} Color;

static Color colors[] = {
	{ 0.8, 0.8, 0.8 }, /* UNPLAYED */
	{ 0.6, 1.0, 0.6 }, /* UNCHANGED */
	{ 1.0, 0.6, 0.6 }, /* CHANGED */
	{ 0.7, 0.7, 0.7 }, /* NO_EXPECTED */
	{ 1.0, 0.5, 0.5 }, /* NO_MATCH */
	{ 1.0, 1.0, 0.5 }, /* NEAR_MATCH */
	{ 0.5, 1.0, 0.5 }, /* EXACT_MATCH */
	{ 0.6, 0.8, 1.0 }, /* COMMAND */
	{ 0.4, 0.4, 1.0 }, /* HIGHLIGHT */
	{ 1.0, 1.0, 0.7 }  /* ACTIVE */
};

/* The private properties are the non-persistent renderer state; any call to 
i7_cell_renderer_transcript_render() must yield a cell of the same size for 
the same values of these properties */
typedef struct _I7CellRendererTranscriptPrivate I7CellRendererTranscriptPrivate;
struct _I7CellRendererTranscriptPrivate
{
	/* The width to make the whole entry */
	unsigned default_width;
	/* The padding in between the box borders and the text */
	unsigned text_padding;
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
			priv->default_width = g_value_get_uint(value);
			g_object_notify(object, "default-width");
			break;
		case PROP_TEXT_PADDING:
			priv->text_padding = g_value_get_uint(value);
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
			g_value_set_uint(value, priv->default_width);
			break;
		case PROP_TEXT_PADDING:
			g_value_set_uint(value, priv->text_padding);
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

static void
i7_cell_renderer_transcript_get_size(GtkCellRenderer *renderer, GtkWidget *widget, const GdkRectangle *cell_area, int *x_offset, int *y_offset, int *width, int *height)
{
	I7CellRendererTranscript *self = I7_CELL_RENDERER_TRANSCRIPT(renderer);
	I7CellRendererTranscriptPrivate *priv = i7_cell_renderer_transcript_get_instance_private(self);

	PangoRectangle command_rect, transcript_rect, expected_rect;
	PangoLayout *layout;
	unsigned xpad, ypad, transcript_width, calc_width, calc_height;
	
	g_object_get(self, "xpad", &xpad, "ypad", &ypad, NULL);
	transcript_width = (priv->default_width / 2) - xpad;

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
	calc_width = priv->default_width;
	calc_height = (unsigned)(command_rect.height + MAX(transcript_rect.height, expected_rect.height)) + ypad * 2 + priv->text_padding * 4;

	/* Set the passed-in parameters; if the available cell area is larger than
	 the required width and height, just use that instead */
	if(cell_area) {
		if(width)
			*width = MAX(cell_area->width, (int)calc_width);
		if(height)
			*height = MAX(cell_area->height, (int)calc_height);
	} else {
		if(width)
			*width = (int)calc_width;
		if(height)
			*height = (int)calc_height;
	}
	if(x_offset)
		*x_offset = 0;
	if(y_offset)
		*y_offset = 0;
}

/* Internal function for convenience in setting Cairo drawing style */
static void 
set_rgb_style(cairo_t *cr, I7TranscriptStyle style) {
	cairo_set_source_rgb(cr, colors[style].r, colors[style].g, colors[style].b);
}

/* TODO remove redundant code */
static void 
i7_cell_renderer_transcript_render(GtkCellRenderer *renderer, cairo_t *cr, GtkWidget *widget, const GdkRectangle *background_area, const GdkRectangle *cell_area, GtkCellRendererState flags)
{
	I7CellRendererTranscript *self = I7_CELL_RENDERER_TRANSCRIPT(renderer);
	I7CellRendererTranscriptPrivate *priv = i7_cell_renderer_transcript_get_instance_private(self);

	int x, y, width, height;
	unsigned xpad, ypad, transcript_width;
	GtkStateType state;
	PangoRectangle command_rect;
	PangoLayout *layout;
	GtkStyle *style = gtk_widget_get_style(widget);

	/* Get the size we calculated earlier and then take the padding into account */
	g_object_get(self, "xpad", &xpad, "ypad", &ypad, NULL);
	gtk_cell_renderer_get_size(renderer, widget, cell_area, &x, &y, &width, &height);
	x += cell_area->x + (int)xpad;
	y += cell_area->y + (int)ypad;
	width -= (int)xpad * 2;
	height -= (int)ypad * 2;
	
	/* Decide what state to draw the widget components in */ 
	switch(flags) {
		case GTK_CELL_RENDERER_PRELIT:
			state = GTK_STATE_PRELIGHT;
			break;
		case GTK_CELL_RENDERER_INSENSITIVE:
			state = GTK_STATE_INSENSITIVE;
			break;
		default:
			state = GTK_STATE_NORMAL;
	}

	/* Draw the command */
	layout = gtk_widget_create_pango_layout(widget, priv->command);
	pango_layout_get_pixel_extents(layout, NULL, &command_rect);

	set_rgb_style(cr, STYLE_COMMAND);
	cairo_rectangle(cr, (double)x, (double)y, 
	    (double)width, (double)(command_rect.height + priv->text_padding * 2));
	cairo_fill(cr);
	gtk_paint_layout(style, cr, state, TRUE, widget, NULL,
	    	x + priv->text_padding, y + priv->text_padding, 
	    	layout);
	g_object_unref(layout);

	/* Draw the transcript text */
	transcript_width = priv->default_width / 2 - xpad;
	if(priv->changed)
		set_rgb_style(cr, STYLE_CHANGED);
	else
		set_rgb_style(cr, STYLE_UNCHANGED);

	cairo_rectangle(cr, 
	    (double)x, 
	    (double)(y + command_rect.height + priv->text_padding * 2), 
	    (double)(width / 2), 
	    (double)(height - command_rect.height - priv->text_padding * 2));
	cairo_fill(cr);
	layout = gtk_widget_create_pango_layout(widget, NULL);
	pango_layout_set_markup(layout, priv->transcript_text, -1);
	pango_layout_set_width(layout, (int)(transcript_width - priv->text_padding * 2) * PANGO_SCALE);
	pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);
	gtk_paint_layout(style, cr, state, TRUE, widget, NULL,
	    x + (int)priv->text_padding, 
	    y + command_rect.height + (int)priv->text_padding * 3, 
		layout);
	g_object_unref(layout);
	
	/* Draw the expected text */
	switch(priv->match_type) {
		case CANT_COMPARE:
			set_rgb_style(cr, STYLE_NO_EXPECTED);
			break;
		case NO_MATCH:
			set_rgb_style(cr, STYLE_NO_MATCH);
			break;
		case NEAR_MATCH:
			set_rgb_style(cr, STYLE_NEAR_MATCH);
			break;
		case EXACT_MATCH:
		default:
			set_rgb_style(cr, STYLE_EXACT_MATCH);
			break;
	}

	cairo_rectangle(cr, 
	    (double)(x + width / 2), 
	    (double)(y + command_rect.height + priv->text_padding * 2), 
	    (double)(width / 2), 
	    (double)(height - command_rect.height - priv->text_padding * 2));
	cairo_fill(cr);
	layout = gtk_widget_create_pango_layout(widget, NULL);
	pango_layout_set_markup(layout, priv->expected_text, -1);
	pango_layout_set_width(layout, (int)(transcript_width - priv->text_padding * 2) * PANGO_SCALE);
	pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);
	gtk_paint_layout(style, cr, state, TRUE, widget, NULL,
	    x + width / 2 + (int)priv->text_padding, 
	    y + command_rect.height + (int)priv->text_padding * 3, 
		layout);
	g_object_unref(layout);

	/* Draw some lines */
	gtk_paint_hline(style, cr, state, widget, NULL,
	    x, x + width, 
	    y + command_rect.height + priv->text_padding * 2);
	gtk_paint_vline(style, cr, state, widget, NULL,
	    y + command_rect.height + priv->text_padding * 2, y + height, 
	    x + width / 2);
	
	/* Draw a border around the highlighted node */
	if(priv->current) {
		cairo_set_line_width(cr, 4.0);
		set_rgb_style(cr, STYLE_HIGHLIGHT);
		cairo_rectangle(cr, (double)x + 2.0, (double)y + 2.0, 
			(double)width - 4.0, (double)height - 4.0);
		cairo_stroke(cr);
	}

	/* Draw a border around the active node */
	if(priv->played) {
		cairo_set_line_width(cr, 2.0);
		set_rgb_style(cr, STYLE_ACTIVE);
		cairo_rectangle(cr, (double)x + 1.0, (double)y + 1.0, 
			(double)width - 2.0, (double)height - 2.0);
		cairo_stroke(cr);
	}
}

static void 
i7_cell_renderer_transcript_class_init(I7CellRendererTranscriptClass *klass) 
{
	GtkCellRendererClass *renderer_class = GTK_CELL_RENDERER_CLASS(klass);
	renderer_class->get_size = i7_cell_renderer_transcript_get_size;
	renderer_class->render = i7_cell_renderer_transcript_render;
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->get_property = i7_cell_renderer_transcript_get_property;
	object_class->set_property = i7_cell_renderer_transcript_set_property;
	object_class->finalize = i7_cell_renderer_transcript_finalize;
	
	/* Install properties */
	GParamFlags flags = G_PARAM_LAX_VALIDATION | G_PARAM_STATIC_STRINGS;
	g_object_class_install_property(object_class, PROP_DEFAULT_WIDTH, 
		g_param_spec_uint("default-width", "Default width",
			"The width to make the whole renderer",
			0, G_MAXUINT, 400, G_PARAM_READWRITE | flags));
	g_object_class_install_property(object_class, PROP_TEXT_PADDING, 
		g_param_spec_uint("text-padding", "Text padding",
			"Padding between the edges of the rectangles and the text",
			0, G_MAXUINT, 6, G_PARAM_READWRITE | flags));
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
