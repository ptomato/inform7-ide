/* Copyright (C) 2010, 2011 P. F. Chimento
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

#include <gdk/gdkkeysyms.h>
#include <goocanvas.h>

#include "node.h"
#include "skein.h"
#include "skein-view.h"

typedef struct _I7SkeinViewPrivate
{
	I7Skein *skein;
	gulong layout_handler;

	/* Drag-scroll information */
	gboolean dragging;
	double drag_anchor[2];
	double drag_offset[2];
} I7SkeinViewPrivate;

enum
{
	NODE_MENU_POPUP,
	LAST_SIGNAL
};

static guint i7_skein_view_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE_WITH_PRIVATE(I7SkeinView, i7_skein_view, GOO_TYPE_CANVAS);

static void
on_item_created(I7SkeinView *self, GooCanvasItem *item, GooCanvasItemModel *model, I7Skein **skeinptr)
{
	if(I7_IS_NODE(model)) {
		i7_node_calculate_size(I7_NODE(model), GOO_CANVAS_ITEM_MODEL(*skeinptr), GOO_CANVAS(self));
		g_signal_connect(item, "button-press-event", G_CALLBACK(on_node_button_press), model);
	}
	else {
		/* Find out what we were clicking on */
		switch(GPOINTER_TO_INT(g_object_get_data(G_OBJECT(model), "node-part"))) {
			case I7_NODE_PART_DIFFERS_BADGE:
				g_signal_connect(item, "button-press-event", G_CALLBACK(on_differs_badge_button_press), model);
				break;
			default:
				;
		}
	}
}

/* Changes the mouse cursor to a dragging hand (GDK_FLEUR) if @dragging is TRUE.
Otherwise, changes it back to normal. */
static void
set_drag_cursor(I7SkeinView *self, gboolean dragging)
{
	GtkWidget *widget = gtk_widget_get_toplevel(GTK_WIDGET(self));

	if(dragging) {
		GdkDisplay *display = gtk_widget_get_display(widget);
		GdkCursor *cursor = gdk_cursor_new_for_display(display, GDK_FLEUR);
		gdk_window_set_cursor(gtk_widget_get_window(widget), cursor);
		gdk_cursor_unref(cursor);
		gdk_flush();
	} else {
		gdk_window_set_cursor(gtk_widget_get_window(widget), NULL);
	}
}

/* Event handler for button press. If the middle button is pressed, turn on
 * dragging mode, where we can pan the canvas by moving the mouse. */
static gboolean
on_button_press(I7SkeinView *self, GdkEventButton *event)
{
	I7SkeinViewPrivate *priv = i7_skein_view_get_instance_private(self);

	if(priv->dragging)
		return FALSE;

	if(event->button == 2) {
		set_drag_cursor(self, TRUE);
		priv->dragging = TRUE;
		priv->drag_anchor[0] = event->x;
		priv->drag_anchor[1] = event->y;

		/* Work out the current scrolling offsets */
		GtkWidget *scrolled_window = gtk_widget_get_parent(GTK_WIDGET(self));
		g_assert(GTK_IS_SCROLLED_WINDOW(scrolled_window));
		GtkAdjustment *adj = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(scrolled_window));
		priv->drag_offset[0] = gtk_adjustment_get_value(adj);
		adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scrolled_window));
		priv->drag_offset[1] = gtk_adjustment_get_value(adj);
		goo_canvas_convert_from_pixels(GOO_CANVAS(self), &(priv->drag_offset[0]), &(priv->drag_offset[1]));
		return TRUE;
	}

	return FALSE;
}

/* Scroll canvas to event coordinates (@x, @y) relative to where the dragging
mode was turned on. */ 
static void
drag_to(I7SkeinView *self, double x, double y)
{
	I7SkeinViewPrivate *priv = i7_skein_view_get_instance_private(self);

	double dx = priv->drag_anchor[0] - x;
	double dy = priv->drag_anchor[1] - y;
	
	x = priv->drag_offset[0] + dx;
	y = priv->drag_offset[1] + dy;
	
	goo_canvas_scroll_to(GOO_CANVAS(self), x, y);
}

/* Button release handler. If the middle button is released in dragging mode, 
 turn off dragging mode and scroll the canvas to the new coordinates. */
static gboolean
on_button_release(I7SkeinView *self, GdkEventButton *event)
{
	I7SkeinViewPrivate *priv = i7_skein_view_get_instance_private(self);

	if(!priv->dragging)
		return FALSE;

	if(event->button == 2) {
		drag_to(self, event->x, event->y);
		priv->dragging = FALSE;
		set_drag_cursor(self, FALSE);
		return TRUE;
	}

	return FALSE;
}

/* Mouse motion handler. In dragging mode, scroll the canvas to the new
 * coordinates. */
static gboolean
on_motion(I7SkeinView *self, GdkEventMotion *event)
{
	I7SkeinViewPrivate *priv = i7_skein_view_get_instance_private(self);

	if(!priv->dragging)
		return FALSE;
	
	drag_to(self, event->x, event->y);
	gdk_event_request_motions(event); /* For smoother motion */	

	return TRUE;
}

static void
i7_skein_view_init(I7SkeinView *self)
{
	I7SkeinViewPrivate *priv = i7_skein_view_get_instance_private(self);
	priv->skein = NULL;
	priv->layout_handler = 0;
	priv->dragging = FALSE;

	g_signal_connect_after(self, "item-created", G_CALLBACK(on_item_created), &priv->skein);
	g_signal_connect(self, "button-press-event", G_CALLBACK(on_button_press), NULL);
	g_signal_connect(self, "button-release-event", G_CALLBACK(on_button_release), NULL);
	g_signal_connect(self, "motion-notify-event", G_CALLBACK(on_motion), NULL);
}

static void
i7_skein_view_finalize(GObject *object)
{
	I7SkeinViewPrivate *priv = i7_skein_view_get_instance_private(I7_SKEIN_VIEW(object));

	if(priv->skein) {
		g_signal_handler_disconnect(priv->skein, priv->layout_handler);
		g_object_unref(priv->skein);
	}

	G_OBJECT_CLASS(i7_skein_view_parent_class)->finalize(object);
}

static void
i7_skein_view_class_init(I7SkeinViewClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = i7_skein_view_finalize;

	/* node-popup-menu - user right-clicked on a node */
	i7_skein_view_signals[NODE_MENU_POPUP] = g_signal_new("node-menu-popup",
		G_OBJECT_CLASS_TYPE(klass), 0,
		G_STRUCT_OFFSET(I7SkeinViewClass, node_menu_popup), NULL, NULL,
		g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, I7_TYPE_NODE);
}

/* PUBLIC FUNCTIONS */

GtkWidget *
i7_skein_view_new(void)
{
	return g_object_new(I7_TYPE_SKEIN_VIEW, NULL);
}

void
i7_skein_view_set_skein(I7SkeinView *self, I7Skein *skein)
{
	g_return_if_fail(self || I7_IS_SKEIN_VIEW(self));
	g_return_if_fail(skein || I7_IS_SKEIN(skein));
	I7SkeinViewPrivate *priv = i7_skein_view_get_instance_private(self);

	if(priv->skein == skein)
		return;

	if(priv->skein) {
		g_signal_handler_disconnect(priv->skein, priv->layout_handler);
		g_object_unref(priv->skein);
	}
	priv->skein = skein;

	if(skein == NULL) {
		goo_canvas_set_root_item_model(GOO_CANVAS(self), NULL);
		return;
	}

	goo_canvas_set_root_item_model(GOO_CANVAS(self), GOO_CANVAS_ITEM_MODEL(skein));
	g_object_ref(skein);
	priv->layout_handler = g_signal_connect(skein, "needs-layout", G_CALLBACK(i7_skein_schedule_draw), self);
	i7_skein_draw(skein, GOO_CANVAS(self));
}

I7Skein *
i7_skein_view_get_skein(I7SkeinView *self)
{
	g_return_val_if_fail(self || I7_IS_SKEIN_VIEW(self), NULL);
	I7SkeinViewPrivate *priv = i7_skein_view_get_instance_private(self);
	return priv->skein;
}

static gboolean
on_edit_popup_key_press(GtkWidget *entry, GdkEventKey *event, GtkWidget *edit_popup)
{
	switch(event->keyval) {
	case GDK_KEY_Escape:
		gtk_widget_destroy(edit_popup);
		return TRUE;
	case GDK_KEY_Return:
	case GDK_KEY_KP_Enter:
		{
			I7Node *node = I7_NODE(g_object_get_data(G_OBJECT(edit_popup), "node"));
			void (*func)(I7Node *, const gchar *) = g_object_get_data(G_OBJECT(edit_popup), "callback");
			func(node, gtk_entry_get_text(GTK_ENTRY(entry)));
			gtk_widget_destroy(edit_popup);
			return TRUE;
		}
	}
	return FALSE; /* event not handled */
}

static GtkWidget *
popup_edit_window(GtkWidget *parent, gint x, gint y, const gchar *text)
{
	gint px, py;
	gdk_window_get_origin(gtk_widget_get_window(parent), &px, &py);

	GtkWidget *edit_popup = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	/* stackoverflow.com/questions/1925568/how-to-give-keyboard-focus-to-a-pop-up-gtk-window */
	gtk_widget_set_can_focus(edit_popup, TRUE);
	gtk_window_set_decorated(GTK_WINDOW(edit_popup), FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(edit_popup), GDK_WINDOW_TYPE_HINT_POPUP_MENU);
	gtk_window_set_transient_for(GTK_WINDOW(edit_popup), GTK_WINDOW(parent));
	gtk_window_set_gravity(GTK_WINDOW(edit_popup), GDK_GRAVITY_CENTER);

	GtkWidget *entry = gtk_entry_new();
	gtk_widget_add_events(entry, GDK_FOCUS_CHANGE_MASK);
	gtk_entry_set_text(GTK_ENTRY(entry), text);

	gtk_editable_select_region(GTK_EDITABLE(entry), 0, -1);
	gtk_container_add(GTK_CONTAINER(edit_popup), entry);
	g_signal_connect_swapped(entry, "focus-out-event", G_CALLBACK(gtk_widget_destroy), edit_popup);
	g_signal_connect(entry, "key-press-event", G_CALLBACK(on_edit_popup_key_press), edit_popup);

	gtk_widget_show_all(edit_popup);
	gtk_window_move(GTK_WINDOW(edit_popup), x + px, y + py);
	/* GDK_GRAVITY_CENTER seems to have no effect? */
	gtk_widget_grab_focus(entry);
	gtk_window_present(GTK_WINDOW(edit_popup));

	return edit_popup;
}

void
i7_skein_view_edit_node(I7SkeinView *self, I7Node *node)
{
	gint x, y;
	if(!i7_node_get_command_coordinates(node, &x, &y, GOO_CANVAS(self)))
		return;
	GtkWidget *parent = gtk_widget_get_toplevel(GTK_WIDGET(self));
	gchar *command = i7_node_get_command(node);
	GtkWidget *edit_popup = popup_edit_window(parent, x, y, command);
	g_free(command);

	/* Associate this window with the node we are editing */
	g_object_set_data(G_OBJECT(edit_popup), "node", node);
	g_object_set_data(G_OBJECT(edit_popup), "callback", i7_node_set_command);
}

void
i7_skein_view_edit_label(I7SkeinView *self, I7Node *node)
{
	gint x, y;
	if(!i7_node_get_label_coordinates(node, &x, &y, GOO_CANVAS(self)))
		return;
	GtkWidget *parent = gtk_widget_get_toplevel(GTK_WIDGET(self));
	gchar *label = i7_node_get_label(node);
	GtkWidget *edit_popup = popup_edit_window(parent, x, y, label);
	g_free(label);

	/* Associate this window with the node we are editing */
	g_object_set_data(G_OBJECT(edit_popup), "node", node);
	g_object_set_data(G_OBJECT(edit_popup), "callback", i7_node_set_label);
}

void
i7_skein_view_show_node(I7SkeinView *self, I7Node *node, I7SkeinShowNodeReason why)
{
	switch(why) {
		case I7_REASON_COMMAND:
		case I7_REASON_USER_ACTION:
		case I7_REASON_TRANSCRIPT:
		{
			I7Skein *skein = i7_skein_view_get_skein(self);
			gdouble vspacing, x, y, width, height;

			/* Work out the position of the node */
			g_object_get(skein, "vertical-spacing", &vspacing, NULL);
			x = i7_node_get_x(node);
			y = (gdouble)(g_node_depth(node->gnode) - 1) * vspacing;

			/* Work out the size of the viewport */
			GtkWidget *scrolled_window = gtk_widget_get_parent(GTK_WIDGET(self));
			g_assert(GTK_IS_SCROLLED_WINDOW(scrolled_window));
			GtkAdjustment *adj = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(scrolled_window));
			width = gtk_adjustment_get_page_size(adj);
			adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scrolled_window));
			height = gtk_adjustment_get_page_size(adj);

			goo_canvas_scroll_to(GOO_CANVAS(self), x - width * 0.5, y - height * 0.5);
		}
			break;
		default:
			g_assert_not_reached();
	}
}
