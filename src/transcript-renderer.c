/* Copyright (C) 2011 P. F. Chimento
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

#include <string.h>
#include <glib-object.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <pango/pango.h>
#include <cairo.h>
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
};

#define I7_CELL_RENDERER_TRANSCRIPT_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), I7_TYPE_CELL_RENDERER_TRANSCRIPT, I7CellRendererTranscriptPrivate))
#define I7_CELL_RENDERER_TRANSCRIPT_USE_PRIVATE I7CellRendererTranscriptPrivate *priv = I7_CELL_RENDERER_TRANSCRIPT_PRIVATE(self)

enum  {
	PROP_0,
	PROP_DEFAULT_WIDTH,
	PROP_TEXT_PADDING,
	PROP_COMMAND,
	PROP_TRANSCRIPT_TEXT,
	PROP_EXPECTED_TEXT
};

G_DEFINE_TYPE(I7CellRendererTranscript, i7_cell_renderer_transcript, GTK_TYPE_CELL_RENDERER);

/* TYPE SYSTEM */

static void 
i7_cell_renderer_transcript_init(I7CellRendererTranscript *self) 
{
	I7_CELL_RENDERER_TRANSCRIPT_USE_PRIVATE;
	/* Default values of properties */
	priv->default_width = 400;
	priv->text_padding = 6;
	priv->command = NULL;
	priv->transcript_text = NULL;
	priv->expected_text = NULL;
}

static void 
i7_cell_renderer_transcript_set_property(GObject *self, unsigned prop_id, const GValue *value, GParamSpec *pspec) 
{
	I7_CELL_RENDERER_TRANSCRIPT_USE_PRIVATE;
	switch(prop_id) {
		case PROP_DEFAULT_WIDTH:
			priv->default_width = g_value_get_uint(value);
			g_object_notify(self, "default-width");
			break;
		case PROP_TEXT_PADDING:
			priv->text_padding = g_value_get_uint(value);
			g_object_notify(self, "text-padding");
			break;
		case PROP_COMMAND:
			g_free(priv->command);
			priv->command = g_strdup(g_value_get_string(value));
			g_object_notify(self, "command");
			break;
		case PROP_TRANSCRIPT_TEXT:
			g_free(priv->transcript_text);
			priv->transcript_text = g_strdup(g_value_get_string(value));
			g_object_notify(self, "transcript-text");
			break;
		case PROP_EXPECTED_TEXT:
			g_free(priv->expected_text);
			priv->expected_text = g_strdup(g_value_get_string(value));
			g_object_notify(self, "expected-text");
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(self, prop_id, pspec);
	}
}

static void 
i7_cell_renderer_transcript_get_property(GObject *self, guint prop_id, GValue *value, GParamSpec *pspec) 
{
	I7_CELL_RENDERER_TRANSCRIPT_USE_PRIVATE;
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
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(self, prop_id, pspec);
	}
}

static void 
i7_cell_renderer_transcript_finalize(GObject* self) 
{
	I7_CELL_RENDERER_TRANSCRIPT_USE_PRIVATE;
	
	g_free(priv->command);
	g_free(priv->transcript_text);
	g_free(priv->expected_text);
	
	G_OBJECT_CLASS(i7_cell_renderer_transcript_parent_class)->finalize(self);
}

static void 
i7_cell_renderer_transcript_get_size(GtkCellRenderer *self, GtkWidget *widget, GdkRectangle *cell_area, int *x_offset, int *y_offset, int *width, int *height) 
{
	I7_CELL_RENDERER_TRANSCRIPT_USE_PRIVATE;

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
	calc_height = (unsigned)(command_rect.height + MAX(transcript_rect.height, expected_rect.height)) + ypad * 2 + priv->text_padding * 2;

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

/* TODO prettify and recomment */
static void 
i7_cell_renderer_transcript_render(GtkCellRenderer *self, GdkWindow *window, GtkWidget *widget, GdkRectangle *background_area, GdkRectangle *cell_area, GdkRectangle *expose_area, GtkCellRendererState flags) 
{
	I7_CELL_RENDERER_TRANSCRIPT_USE_PRIVATE;
	
	int x, y, width, height;
	unsigned xpad, ypad, transcript_width;
	GtkStateType state;
	cairo_t *cr;
	PangoRectangle command_rect;
	PangoLayout *layout;
	GtkStyle *style = gtk_widget_get_style(widget);

	/* Get the size we calculated earlier and then take the padding into account */
	g_object_get(self, "xpad", &xpad, "ypad", &ypad, NULL);
	gtk_cell_renderer_get_size(self, widget, cell_area, &x, &y, &width, &height);
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

	/* Get a cairo context to draw the rectangles on directly; use GTK themed
	 drawing to draw everything else */
	cr = gdk_cairo_create(GDK_DRAWABLE(window));

	/* Draw the command */
	layout = gtk_widget_create_pango_layout(widget, priv->command);
	pango_layout_get_pixel_extents(layout, NULL, &command_rect);

	set_rgb_style(cr, STYLE_COMMAND);
	cairo_rectangle(cr, (double)x, (double)y, 
	    (double)width, (double)command_rect.height);
	cairo_fill(cr);
	gtk_paint_layout(style, window, state, TRUE, cell_area, widget, NULL, 
	    	x, y, 
	    	layout);
	g_object_unref(layout);

	/* Draw the transcript text */
	transcript_width = priv->default_width / 2 - xpad;
	if(priv->expected_text && strcmp(priv->transcript_text, priv->expected_text) != 0)
		set_rgb_style(cr, STYLE_CHANGED);
	else
		set_rgb_style(cr, STYLE_UNCHANGED);

	cairo_rectangle(cr, (double)x, (double)(y + command_rect.height), 
	    (double)(width / 2), (double)(height - command_rect.height));
	cairo_fill(cr);
	layout = gtk_widget_create_pango_layout(widget, priv->transcript_text);
	pango_layout_set_width(layout, (int)(transcript_width - priv->text_padding * 2) * PANGO_SCALE);
	pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);
	gtk_paint_layout(style, window, state, TRUE, cell_area, widget, NULL, 
	    x + (int)priv->text_padding, y + command_rect.height + (int)priv->text_padding, 
		layout);
	g_object_unref(layout);
	
	/* Draw the expected text */
	if(priv->expected_text == NULL)
		set_rgb_style(cr, STYLE_NO_EXPECTED);
	else if(strcmp(priv->transcript_text, priv->expected_text) != 0)
		set_rgb_style(cr, STYLE_NO_MATCH);
	else
		set_rgb_style(cr, STYLE_EXACT_MATCH);

	cairo_rectangle(cr, (double)(x + width / 2), (double)(y + command_rect.height), 
	    (double)(width / 2), (double)(height - command_rect.height));
	cairo_fill(cr);
	layout = gtk_widget_create_pango_layout(widget, priv->expected_text);
	pango_layout_set_width(layout, (int)(transcript_width - priv->text_padding * 2) * PANGO_SCALE);
	pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);
	gtk_paint_layout(style, window, state, TRUE, cell_area, widget, NULL,
	    x + width / 2 + (int)priv->text_padding, y + command_rect.height + (int)priv->text_padding, 
		layout);
	g_object_unref(layout);

	/* Draw some lines */
	gtk_paint_hline(style, window, state, cell_area, widget, NULL, 
	    x, x + width, y + command_rect.height);
	gtk_paint_vline(style, window, state, cell_area, widget, NULL, 
	    y + command_rect.height, y + height, x + width / 2);
	
	cairo_destroy(cr);
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
		g_param_spec_uint("default-width", _("Default width"), 
			_("The width to make the whole renderer"), 
			0, G_MAXUINT, 400, G_PARAM_READWRITE | flags));
	g_object_class_install_property(object_class, PROP_TEXT_PADDING, 
		g_param_spec_uint("text-padding", _("Text padding"), 
			_("Padding between the edges of the rectangles and the text"), 
			0, G_MAXUINT, 6, G_PARAM_READWRITE | flags));
	g_object_class_install_property(object_class, PROP_COMMAND, 
		g_param_spec_string("command", _("Command"), 
			_("Command from the Skein"), 
			NULL, G_PARAM_READWRITE | flags));
	g_object_class_install_property(object_class, PROP_TRANSCRIPT_TEXT, 
		g_param_spec_string("transcript-text", _("Transcript text"), 
			_("Transcript text from the Skein"), 
			NULL, G_PARAM_READWRITE | flags));
	g_object_class_install_property(object_class, PROP_EXPECTED_TEXT, 
		g_param_spec_string("expected-text", _("Expected text"), 
			_("Expected text from the Skein"), 
			NULL, G_PARAM_READWRITE | flags));
	
	/* Add private data */
	g_type_class_add_private (klass, sizeof (I7CellRendererTranscriptPrivate));
}

I7CellRendererTranscript *
i7_cell_renderer_transcript_new(void) 
{
	return g_object_new(I7_TYPE_CELL_RENDERER_TRANSCRIPT, NULL);
}
