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
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include "transcript-renderer.h"

/*#include <glib.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <gdk/gdk.h>
#include <pango/pango.h>
#include <cairo.h>*/

/* TODO get rid of this */
static int _vala_strcmp0 (const char * str1, const char * str2) {
	if (str1 == NULL) {
		return -(str1 != str2);
	}
	if (str2 == NULL) {
		return str1 != str2;
	}
	return strcmp (str1, str2);
}

typedef enum  {
	I7_TRANSCRIPT_STYLE_UNPLAYED,
	I7_TRANSCRIPT_STYLE_UNCHANGED,
	I7_TRANSCRIPT_STYLE_CHANGED,
	I7_TRANSCRIPT_STYLE_NO_EXPECTED,
	I7_TRANSCRIPT_STYLE_NO_MATCH,
	I7_TRANSCRIPT_STYLE_NEAR_MATCH,
	I7_TRANSCRIPT_STYLE_EXACT_MATCH,
	I7_TRANSCRIPT_STYLE_COMMAND,
	I7_TRANSCRIPT_STYLE_HIGHLIGHT,
	I7_TRANSCRIPT_STYLE_ACTIVE,
	I7_TRANSCRIPT_STYLE_LAST
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

/* TODO prettify and recomment */
static void 
i7_cell_renderer_transcript_get_size(GtkCellRenderer *self, GtkWidget *widget, GdkRectangle *cell_area, int *x_offset, int *y_offset, int *width, int *height) 
{
	I7_CELL_RENDERER_TRANSCRIPT_USE_PRIVATE;
	
	PangoRectangle command_rect = {0};
	PangoRectangle transcript_rect = {0};
	PangoRectangle expected_rect = {0};
	PangoLayout* layout;
	guint _tmp0_;
	guint transcript_width;
	guint calc_width;
	guint _tmp3_;
	guint calc_height;
	g_return_if_fail (widget != NULL);
	layout = gtk_widget_create_pango_layout (widget, priv->command);
	transcript_width = (priv->default_width / 2) - (g_object_get ((GtkCellRenderer*) self, "xpad", &_tmp0_, NULL), _tmp0_);
	pango_layout_get_pixel_extents (layout, NULL, &command_rect);
	layout = gtk_widget_create_pango_layout (widget, priv->transcript_text);
	pango_layout_set_width (layout, ((gint) (transcript_width - (priv->text_padding * 2))) * PANGO_SCALE);
	pango_layout_set_wrap (layout, PANGO_WRAP_WORD_CHAR);
	pango_layout_get_pixel_extents (layout, NULL, &transcript_rect);
	g_object_unref(layout);
	layout = gtk_widget_create_pango_layout(widget, priv->expected_text);
	pango_layout_set_width (layout, ((gint) (transcript_width - (priv->text_padding * 2))) * PANGO_SCALE);
	pango_layout_set_wrap (layout, PANGO_WRAP_WORD_CHAR);
	pango_layout_get_pixel_extents (layout, NULL, &expected_rect);
	calc_width = priv->default_width;
	calc_height = ((command_rect.height + MAX ((guint) transcript_rect.height, (guint) expected_rect.height)) + ((g_object_get ((GtkCellRenderer*) self, "ypad", &_tmp3_, NULL), _tmp3_) * 2)) + (priv->text_padding * 2);
	if (cell_area != NULL) {
		if ((width) != NULL) {
			*width = MAX ((*cell_area).width, (gint) calc_width);
		}
		if ((height) != NULL) {
			*height = MAX ((*cell_area).height, (gint) calc_height);
		}
	} else {
		if ((width) != NULL) {
			*width = (gint) calc_width;
		}
		if ((height) != NULL) {
			*height = (gint) calc_height;
		}
	}
	if ((x_offset) != NULL) {
		*x_offset = 0;
	}
	if ((y_offset) != NULL) {
		*y_offset = 0;
	}
	g_object_unref (layout);
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
	gint x = 0;
	gint y = 0;
	gint width = 0;
	gint height = 0;
	guint _tmp0_;
	guint _tmp1_;
	guint _tmp2_;
	guint _tmp3_;
	GtkStateType state = 0;
	cairo_t* cr;
	PangoRectangle command_rect = {0};
	PangoLayout* layout;
	guint _tmp4_;
	guint transcript_width;
	gboolean _tmp5_ = FALSE;
	g_return_if_fail (window != NULL);
	g_return_if_fail (widget != NULL);
	gtk_cell_renderer_get_size ((GtkCellRenderer*) self, widget, cell_area, &x, &y, &width, &height);
	x = x + ((*cell_area).x + ((gint) (g_object_get ((GtkCellRenderer*) self, "xpad", &_tmp0_, NULL), _tmp0_)));
	y = y + ((*cell_area).y + ((gint) (g_object_get ((GtkCellRenderer*) self, "ypad", &_tmp1_, NULL), _tmp1_)));
	width = width - (((gint) (g_object_get ((GtkCellRenderer*) self, "xpad", &_tmp2_, NULL), _tmp2_)) * 2);
	height = height - (((gint) (g_object_get ((GtkCellRenderer*) self, "ypad", &_tmp3_, NULL), _tmp3_)) * 2);
	
	/* Decide what state to draw the widget components in */ 
	switch (flags) {
		case GTK_CELL_RENDERER_PRELIT:
		{
			state = GTK_STATE_PRELIGHT;
			break;
		}
		case GTK_CELL_RENDERER_INSENSITIVE:
		{
			state = GTK_STATE_INSENSITIVE;
			break;
		}
		default:
		{
			state = GTK_STATE_NORMAL;
			break;
		}
	}
	cr = gdk_cairo_create ((GdkDrawable*) window);
	layout = gtk_widget_create_pango_layout (widget, priv->command);
	pango_layout_get_pixel_extents (layout, NULL, &command_rect);
	set_rgb_style (cr, I7_TRANSCRIPT_STYLE_COMMAND);
	cairo_rectangle (cr, (double) x, (double) y, (double) width, (double) command_rect.height);
	cairo_fill (cr);
	gtk_paint_layout (gtk_widget_get_style (widget), window, state, TRUE, cell_area, widget, NULL, x, y, layout);
	transcript_width = (priv->default_width / 2) - (g_object_get ((GtkCellRenderer*) self, "xpad", &_tmp4_, NULL), _tmp4_);
	if (priv->expected_text != NULL) {
		_tmp5_ = _vala_strcmp0 (priv->transcript_text, priv->expected_text) != 0;
	} else {
		_tmp5_ = FALSE;
	}
	if (_tmp5_) {
		set_rgb_style (cr, I7_TRANSCRIPT_STYLE_CHANGED);
	} else {
		set_rgb_style (cr, I7_TRANSCRIPT_STYLE_UNCHANGED);
	}
	cairo_rectangle (cr, (double) x, (double) (y + command_rect.height), (double) (width / 2), (double) (height - command_rect.height));
	cairo_fill (cr);
	layout = gtk_widget_create_pango_layout (widget, priv->transcript_text);
	pango_layout_set_width (layout, ((gint) (transcript_width - (priv->text_padding * 2))) * PANGO_SCALE);
	pango_layout_set_wrap (layout, PANGO_WRAP_WORD_CHAR);
	gtk_paint_layout (gtk_widget_get_style (widget), window, state, TRUE, cell_area, widget, NULL, x + ((gint) priv->text_padding), (y + command_rect.height) + ((gint) priv->text_padding), layout);
	if (priv->expected_text == NULL) {
		set_rgb_style (cr, I7_TRANSCRIPT_STYLE_NO_EXPECTED);
	} else {
		if (_vala_strcmp0 (priv->transcript_text, priv->expected_text) != 0) {
			set_rgb_style (cr, I7_TRANSCRIPT_STYLE_NO_MATCH);
		} else {
			set_rgb_style (cr, I7_TRANSCRIPT_STYLE_EXACT_MATCH);
		}
	}
	cairo_rectangle (cr, (double) (x + (width / 2)), (double) (y + command_rect.height), (double) (width / 2), (double) (height - command_rect.height));
	cairo_fill (cr);
	g_object_unref(layout);
	layout = gtk_widget_create_pango_layout (widget, priv->expected_text);
	pango_layout_set_width (layout, ((gint) (transcript_width - (priv->text_padding * 2))) * PANGO_SCALE);
	pango_layout_set_wrap (layout, PANGO_WRAP_WORD_CHAR);
	gtk_paint_layout (gtk_widget_get_style (widget), window, state, TRUE, cell_area, widget, NULL, (x + (width / 2)) + ((gint) priv->text_padding), (y + command_rect.height) + ((gint) priv->text_padding), layout);
	gtk_paint_hline (gtk_widget_get_style (widget), window, state, cell_area, widget, NULL, x, x + width, y + command_rect.height);
	gtk_paint_vline (gtk_widget_get_style (widget), window, state, cell_area, widget, NULL, y + command_rect.height, y + height, x + (width / 2));
	g_object_unref (layout);
	cairo_destroy (cr);
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

