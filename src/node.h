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

#ifndef __NODE_H__
#define __NODE_H__

#include "config.h"

#include <glib-object.h>
#include <goocanvas.h>

G_BEGIN_DECLS

#define I7_TYPE_NODE         (i7_node_get_type())
#define I7_NODE(o)           (G_TYPE_CHECK_INSTANCE_CAST((o), I7_TYPE_NODE, I7Node))
#define I7_NODE_CLASS(c)     (G_TYPE_CHECK_CLASS_CAST((c), I7_TYPE_NODE, I7NodeClass))
#define I7_IS_NODE(o)        (G_TYPE_CHECK_INSTANCE_TYPE((o), I7_TYPE_NODE))
#define I7_IS_NODE_CLASS(c)  (G_TYPE_CHECK_CLASS_TYPE((c), I7_TYPE_NODE))
#define I7_NODE_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS((o), I7_TYPE_NODE, I7NodeClass))

typedef struct _I7NodeClass I7NodeClass;
typedef struct _I7Node I7Node;

struct _I7NodeClass {
	GooCanvasGroupModelClass parent_class;
};

struct _I7Node {
	GooCanvasGroupModel parent_instance;
	GNode *gnode;
	GooCanvasItemModel *tree_item; /* The tree line associated with the node */
	GooCanvasPoints *tree_points; /* The points of tree_item */
};

/* Clickable parts of the node */
typedef enum {
	I7_NODE_PART_NONE,
	I7_NODE_PART_DIFFERS_BADGE
} I7NodePart;

typedef enum {
	I7_NODE_CANT_COMPARE = -1,
	I7_NODE_NO_MATCH,
	I7_NODE_NEAR_MATCH,
	I7_NODE_EXACT_MATCH
} I7NodeMatchType;

GType i7_node_get_type(void) G_GNUC_CONST;
I7Node *i7_node_new(const gchar *line, const gchar *label, const gchar *transcript,
	const gchar *expected, gboolean played, gboolean locked, gboolean changed,
    int score, GooCanvasItemModel *skein);

/* Properties */
gchar *i7_node_get_command(I7Node *self);
void i7_node_set_command(I7Node *self, const gchar *line);
gchar *i7_node_get_label(I7Node *self);
void i7_node_set_label(I7Node *self, const gchar *label);
gboolean i7_node_has_label(I7Node *self);
gchar *i7_node_get_transcript_text(I7Node *self);
void i7_node_set_transcript_text(I7Node *self, const gchar *transcript);
gchar *i7_node_get_expected_text(I7Node *self);
const char *i7_node_get_transcript_pango_string(I7Node *self);
const char *i7_node_get_expected_pango_string(I7Node *self);
I7NodeMatchType i7_node_get_match_type(I7Node *self);
gboolean i7_node_get_different(I7Node *self);
gboolean i7_node_get_changed(I7Node *self);
gboolean i7_node_get_locked(I7Node *self);
void i7_node_set_locked(I7Node *self, gboolean locked);
gboolean i7_node_get_played(I7Node *self);
void i7_node_set_played(I7Node *self, gboolean played);
gboolean i7_node_get_blessed(I7Node *self);
void i7_node_bless(I7Node *self);
gint i7_node_get_score(I7Node *self);
void i7_node_set_score(I7Node *self, gint score);

/* Tree functions */
gboolean i7_node_in_thread(I7Node *self, I7Node *endnode);
gboolean i7_node_is_root(I7Node *self);
I7Node *i7_node_find_child(I7Node *self, const gchar *command);
I7Node *i7_node_get_next_difference_below(I7Node *node);
I7Node *i7_node_get_next_difference(I7Node *node);

/* Serialization */
const gchar *i7_node_get_unique_id(I7Node *self);
gchar *i7_node_get_xml(I7Node *self);

/* Drawing on a GooCanvas */
gdouble i7_node_get_x(I7Node *self);
gdouble i7_node_get_tree_width(I7Node *self, GooCanvasItemModel *skein, GooCanvas *canvas);
void i7_node_layout(I7Node *self, GooCanvasItemModel *skein, GooCanvas *canvas, gdouble x);
void i7_node_calculate_size(I7Node *self, GooCanvasItemModel *skein, GooCanvas *canvas);
void i7_node_invalidate_size(I7Node *self);
gboolean i7_node_get_command_coordinates(I7Node *self, GdkRectangle *rect, GooCanvas *canvas);
gboolean i7_node_get_label_coordinates(I7Node *self, GdkRectangle *rect, GooCanvas *canvas);

/* Signal handlers for use in other skein source files*/
gboolean on_node_button_press(GooCanvasItem *item, GooCanvasItem *target_item, GdkEventButton *event, I7Node *self);
gboolean on_differs_badge_button_press(GooCanvasItem *item, GooCanvasItem *target_item, GdkEventButton *event, GooCanvasItemModel *model);

#endif /* __NODE_H__ */
