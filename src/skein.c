/* Copyright (C) 2006-2009, 2010, 2011, 2012, 2013, 2015, 2019 P. F. Chimento
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

#include <errno.h>

#include <gio/gio.h>
#include <glib/gi18n.h>
#include <goocanvas.h>
#include <gtk/gtk.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "node.h"
#include "skein.h"

#define VALID_ITER(iter, priv) ( \
	(iter) != NULL \
	&& (iter)->user_data != NULL \
	&& I7_IS_NODE((iter)->user_data) \
	&& (iter)->stamp == (priv)->stamp)

typedef struct _I7SkeinPrivate
{
	I7Node *root;
	I7Node *current; /* Node currently displayed in Transcript */
	I7Node *played;  /* Node currently played (yellow) */
	gboolean modified;

	gdouble hspacing;
	gdouble vspacing;

	GooCanvasLineDash *locked_dash;
	GooCanvasLineDash *unlocked_dash;

	GSettings *settings; /* skein settings */

	int stamp; /* Stamp for identifying tree iterators belonging to this model */
} I7SkeinPrivate;

enum
{
	NEEDS_LAYOUT,
	NODE_ACTIVATE,
	DIFFERS_BADGE_ACTIVATE,
	TRANSCRIPT_THREAD_CHANGED,
	LABELS_CHANGED,
	SHOW_NODE,
	MODIFIED,
	LAST_SIGNAL
};

enum
{
	PROP_0,
	PROP_CURRENT_NODE,
	PROP_PLAYED_NODE,
	PROP_HORIZONTAL_SPACING,
	PROP_VERTICAL_SPACING,
};

static guint i7_skein_signals[LAST_SIGNAL] = { 0 };

static void i7_skein_tree_model_init(GtkTreeModelIface *iface);

G_DEFINE_TYPE_EXTENDED(I7Skein, i7_skein, GOO_TYPE_CANVAS_GROUP_MODEL, 0,
	G_ADD_PRIVATE(I7Skein)
    G_IMPLEMENT_INTERFACE(GTK_TYPE_TREE_MODEL, i7_skein_tree_model_init));

/* SIGNAL HANDLERS */

static void
on_node_other_notify(I7Node *node, GParamSpec *pspec, I7Skein *self)
{
	g_signal_emit_by_name(self, "modified");
}

static void
on_node_layout_notify(I7Node *node, GParamSpec *pspec, I7Skein *self)
{
	g_signal_emit_by_name(self, "needs-layout");
	on_node_other_notify(node, pspec, self);
}

static void
on_node_label_notify(I7Node *node, GParamSpec *pspec, I7Skein *self)
{
	if(i7_node_has_label(node))
		i7_skein_lock(self, i7_skein_get_thread_bottom(self, node));
	g_signal_emit_by_name(self, "labels-changed");
	on_node_layout_notify(node, pspec, self);
}

static void
on_node_transcript_notify(I7Node *node, GParamSpec *pspec, I7Skein *self)
{
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);
	if(!i7_skein_is_node_in_current_thread(self, node))
		return;
	GtkTreePath *path = gtk_tree_path_new_from_indices(g_node_depth(node->gnode) - 1, -1);
	GtkTreeIter iter = { .stamp = priv->stamp, .user_data = node };
	gtk_tree_model_row_changed(GTK_TREE_MODEL(self), path, &iter);
	gtk_tree_path_free(path);
}

static void
node_listen(I7Skein *self, I7Node *node)
{
	g_signal_connect(node, "notify::command", G_CALLBACK(on_node_layout_notify), self);
	g_signal_connect(node, "notify::command", G_CALLBACK(on_node_transcript_notify), self);
	g_signal_connect(node, "notify::label", G_CALLBACK(on_node_label_notify), self);
	g_signal_connect(node, "notify::transcript-text", G_CALLBACK(on_node_other_notify), self);
	g_signal_connect(node, "notify::transcript-text", G_CALLBACK(on_node_transcript_notify), self);
	g_signal_connect(node, "notify::expected-text", G_CALLBACK(on_node_layout_notify), self);
	g_signal_connect(node, "notify::expected-text", G_CALLBACK(on_node_transcript_notify), self);
	g_signal_connect(node, "notify::locked", G_CALLBACK(on_node_layout_notify), self);
}

/* TYPE SYSTEM */

static void
i7_skein_init(I7Skein *self)
{
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);
	priv->root = i7_node_new(_("- start -"), "", "", "", FALSE, FALSE, FALSE, 0, GOO_CANVAS_ITEM_MODEL(self));
	node_listen(self, priv->root);
	priv->current = priv->root;
	priv->played = priv->root;
	priv->modified = TRUE;
	priv->locked_dash = goo_canvas_line_dash_new(0);
	priv->unlocked_dash = goo_canvas_line_dash_new(2, 5.0, 5.0);

	priv->settings = g_settings_new("com.inform7.IDE.preferences.skein");
	g_settings_bind(priv->settings, "horizontal-spacing", self, "horizontal-spacing", G_SETTINGS_BIND_DEFAULT);
	g_settings_bind(priv->settings, "vertical-spacing", self, "vertical-spacing", G_SETTINGS_BIND_DEFAULT);

	priv->stamp = g_random_int();
}

static void
i7_skein_set_property(GObject *self, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	I7SkeinPrivate *priv = i7_skein_get_instance_private(I7_SKEIN(self));

	switch(prop_id) {
		case PROP_CURRENT_NODE:
			i7_skein_set_current_node(I7_SKEIN(self), I7_NODE(g_value_get_object(value)));
			break;
		case PROP_HORIZONTAL_SPACING:
			priv->hspacing = g_value_get_double(value);
			g_object_notify(self, "horizontal-spacing");
			g_signal_emit_by_name(self, "needs-layout");
			break;
		case PROP_VERTICAL_SPACING:
			priv->vspacing = g_value_get_double(value);
			g_object_notify(self, "vertical-spacing");
			g_signal_emit_by_name(self, "needs-layout");
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(self, prop_id, pspec);
	}
}

static void
i7_skein_get_property(GObject *self, guint prop_id, GValue *value, GParamSpec *pspec)
{
	I7SkeinPrivate *priv = i7_skein_get_instance_private(I7_SKEIN(self));

	switch(prop_id) {
		case PROP_CURRENT_NODE:
			g_value_set_object(value, priv->current);
			break;
		case PROP_PLAYED_NODE:
			g_value_set_object(value, priv->played);
			break;
		case PROP_HORIZONTAL_SPACING:
			g_value_set_double(value, priv->hspacing);
			break;
		case PROP_VERTICAL_SPACING:
			g_value_set_double(value, priv->vspacing);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(self, prop_id, pspec);
	}
}

static void
i7_skein_finalize(GObject *self)
{
	I7SkeinPrivate *priv = i7_skein_get_instance_private(I7_SKEIN(self));

	g_object_unref(priv->root);
	goo_canvas_line_dash_unref(priv->unlocked_dash);

	G_OBJECT_CLASS(i7_skein_parent_class)->finalize(self);
}

/* Default signal handler */
static void
i7_skein_modified(I7Skein *self)
{
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);
	priv->modified = TRUE;
}

static void
i7_skein_class_init(I7SkeinClass *klass)
{
	klass->modified = i7_skein_modified;
	GObjectClass* object_class = G_OBJECT_CLASS(klass);
	object_class->set_property = i7_skein_set_property;
	object_class->get_property = i7_skein_get_property;
	object_class->finalize = i7_skein_finalize;

	/* Signals */
	/* needs-layout - skein requests view to calculate new sizes for its nodes */
	i7_skein_signals[NEEDS_LAYOUT] = g_signal_new("needs-layout",
		G_OBJECT_CLASS_TYPE(klass), G_SIGNAL_NO_RECURSE,
		G_STRUCT_OFFSET(I7SkeinClass, needs_layout), NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	/* node-activate - user double-clicked on a node */
	i7_skein_signals[NODE_ACTIVATE] = g_signal_new("node-activate",
		G_OBJECT_CLASS_TYPE(klass), 0,
		G_STRUCT_OFFSET(I7SkeinClass, node_activate), NULL, NULL,
		g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, I7_TYPE_NODE);
	/* differs-badge-activate - user double-clicked on a differs badge */
	i7_skein_signals[DIFFERS_BADGE_ACTIVATE] = g_signal_new("differs-badge-activate",
		G_OBJECT_CLASS_TYPE(klass), 0,
		G_STRUCT_OFFSET(I7SkeinClass, differs_badge_activate), NULL, NULL,
		g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, I7_TYPE_NODE);
	/* transcript-thread-changed */
	i7_skein_signals[TRANSCRIPT_THREAD_CHANGED] = g_signal_new("transcript-thread-changed",
		G_OBJECT_CLASS_TYPE(klass), 0,
		G_STRUCT_OFFSET(I7SkeinClass, transcript_thread_changed), NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	/* labels-changed - for controlling the 'labels' dropdown menu */
	i7_skein_signals[LABELS_CHANGED] = g_signal_new("labels-changed",
		G_OBJECT_CLASS_TYPE(klass), 0,
		G_STRUCT_OFFSET(I7SkeinClass, labels_changed), NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	/* show-node - skein requests its view to display a certain node */
	i7_skein_signals[SHOW_NODE] = g_signal_new("show-node",
		G_OBJECT_CLASS_TYPE(klass), 0,
		G_STRUCT_OFFSET(I7SkeinClass, show_node), NULL, NULL,
		g_cclosure_marshal_VOID__UINT_POINTER, G_TYPE_NONE, 2,
		G_TYPE_UINT, I7_TYPE_NODE);
	/* modified - skein has been modified since last save */
	i7_skein_signals[MODIFIED] = g_signal_new("modified",
		G_OBJECT_CLASS_TYPE(klass), 0,
		G_STRUCT_OFFSET(I7SkeinClass, modified), NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	/* Install properties */
	GParamFlags flags = G_PARAM_LAX_VALIDATION | G_PARAM_STATIC_STRINGS;
	g_object_class_install_property(object_class, PROP_CURRENT_NODE,
		g_param_spec_object("current-node", "Current node",
			"The node currently displayed in the Transcript",
			I7_TYPE_NODE, G_PARAM_READWRITE | flags));
	g_object_class_install_property(object_class, PROP_PLAYED_NODE,
		g_param_spec_object("played-node", "Played node",
			"The node last played in the Game view",
			I7_TYPE_NODE, G_PARAM_READABLE | flags));
	g_object_class_install_property(object_class, PROP_HORIZONTAL_SPACING,
		g_param_spec_double("horizontal-spacing", "Horizontal spacing",
			"Pixels of horizontal space between skein branches",
			20.0, 100.0, 40.0, G_PARAM_READWRITE | flags));
	g_object_class_install_property(object_class, PROP_VERTICAL_SPACING,
		g_param_spec_double("vertical-spacing", "Vertical spacing",
			"Pixels of vertical space between skein items",
			20.0, 100.0, 40.0, G_PARAM_READWRITE | flags));
}

/* TREE MODEL INTERFACE IMPLEMENTATION */

static G_GNUC_CONST GtkTreeModelFlags
i7_skein_get_flags(GtkTreeModel *model)
{
	return GTK_TREE_MODEL_LIST_ONLY | GTK_TREE_MODEL_ITERS_PERSIST;
}

static G_GNUC_CONST int
i7_skein_get_n_columns(GtkTreeModel *model)
{
	return I7_SKEIN_NUM_COLUMNS;
}

static G_GNUC_CONST GType
i7_skein_get_column_type(GtkTreeModel *model, int ncol)
{
	switch(ncol) {
		case I7_SKEIN_COLUMN_COMMAND:
		case I7_SKEIN_COLUMN_TRANSCRIPT_TEXT:
		case I7_SKEIN_COLUMN_EXPECTED_TEXT:
			return G_TYPE_STRING;
		case I7_SKEIN_COLUMN_MATCH_TYPE:
			return G_TYPE_INT;
		case I7_SKEIN_COLUMN_CURRENT:
		case I7_SKEIN_COLUMN_PLAYED:
		case I7_SKEIN_COLUMN_CHANGED:
			return G_TYPE_BOOLEAN;
		case I7_SKEIN_COLUMN_NODE_PTR:
			return I7_TYPE_NODE;
		default:
			g_assert_not_reached();
	}
}

/* We fill in the first user_data field of GtkTreeIter with a pointer to the
 I7Node referenced by the iter. */
static gboolean
i7_skein_get_iter(GtkTreeModel *model, GtkTreeIter *iter, GtkTreePath *path)
{
	I7Skein *self = I7_SKEIN(model);
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);

	int i = gtk_tree_path_get_indices(path)[0];
	I7Node *last = i7_skein_get_thread_bottom(self, priv->current);
	int thread_depth = g_node_depth(last->gnode) - 1;
	/* g_node_depth returns 0 for NULL and 1 for the root node */
	
	if(i > thread_depth)
		return FALSE;

	iter->stamp = priv->stamp;
	
	if(i == 0) {
		iter->user_data = priv->root;
		return TRUE;
	}
	if(i == thread_depth) {
		iter->user_data = last;
		return TRUE;
	}
	
	/* Make the uninitialized iterator point to the i'th node between the root
	 node and the bottom of the current node's thread */
	GNode *gnode = last->gnode;
	for( ; thread_depth > i; thread_depth--)
		gnode = gnode->parent;
	
	iter->user_data = gnode->data;
	return TRUE;
}

static GtkTreePath *
i7_skein_get_path(GtkTreeModel *model, GtkTreeIter *iter)
{
	I7Skein *self = I7_SKEIN(model);
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);
	
	g_return_val_if_fail(VALID_ITER(iter, priv), NULL);

	GtkTreePath *path = gtk_tree_path_new();
	int index = g_node_depth(((I7Node *)iter->user_data)->gnode) - 1;
	gtk_tree_path_append_index(path, index);
	return path;
}

static void
i7_skein_get_value(GtkTreeModel *model, GtkTreeIter *iter, int column, GValue *value)
{
	I7Skein *self = I7_SKEIN(model);
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);
	
	g_return_if_fail(VALID_ITER(iter, priv));
	g_return_if_fail(column >= 0 && column < I7_SKEIN_NUM_COLUMNS);

	switch(column) {
		case I7_SKEIN_COLUMN_COMMAND:
			g_value_init(value, G_TYPE_STRING);
			g_value_take_string(value, i7_node_get_command(iter->user_data));
			break;
		case I7_SKEIN_COLUMN_TRANSCRIPT_TEXT:
			g_value_init(value, G_TYPE_STRING);
			g_value_set_string(value, i7_node_get_transcript_pango_string(iter->user_data));
			break;
		case I7_SKEIN_COLUMN_EXPECTED_TEXT:
			g_value_init(value, G_TYPE_STRING);
			g_value_set_string(value, i7_node_get_expected_pango_string(iter->user_data));
			break;
		case I7_SKEIN_COLUMN_MATCH_TYPE:
			g_value_init(value, G_TYPE_INT);
			g_value_set_int(value, i7_node_get_match_type(iter->user_data));
			break;
		case I7_SKEIN_COLUMN_CURRENT:
			g_value_init(value, G_TYPE_BOOLEAN);
			g_value_set_boolean(value, priv->current == iter->user_data);
			break;
		case I7_SKEIN_COLUMN_PLAYED:
			g_value_init(value, G_TYPE_BOOLEAN);
			g_value_set_boolean(value, priv->played == iter->user_data);
			break;
		case I7_SKEIN_COLUMN_CHANGED:
			g_value_init(value, G_TYPE_BOOLEAN);
			g_value_set_boolean(value, i7_node_get_changed(iter->user_data));
			break;
		case I7_SKEIN_COLUMN_NODE_PTR:
			g_value_init(value, I7_TYPE_NODE);
			g_value_set_object(value, iter->user_data);
			break;
		default:
			g_assert_not_reached();
	}
}

/* Invalidate an iter on this model */
static void
invalidate_iter(GtkTreeIter *iter)
{
	iter->stamp = 0;
	iter->user_data = NULL;
}

static gboolean
i7_skein_iter_next(GtkTreeModel *model, GtkTreeIter *iter)
{
	I7Skein *self = I7_SKEIN(model);
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);
	
	g_return_val_if_fail(VALID_ITER(iter, priv), FALSE);

	/* Don't go beyond the bottom of "current" node's thread (end of the list) */
	I7Node *last = i7_skein_get_thread_bottom(self, priv->current);
	if(iter->user_data == last) {
		invalidate_iter(iter);
		return FALSE;
	}
	
	GNode *gnode = ((I7Node *)iter->user_data)->gnode;

	for(gnode = gnode->children; gnode; gnode = gnode->next) {
		if(i7_node_in_thread(gnode->data, last))
			break;
	}
	g_assert(gnode);

	iter->user_data = gnode->data;
	return TRUE;
}

static gboolean
i7_skein_iter_children(GtkTreeModel *model, GtkTreeIter *iter, GtkTreeIter *parent)
{
	/* This is a list, nodes have no children */
	if(parent) {
		invalidate_iter(iter);
		return FALSE;
	}

	/* If parent was NULL, return the root node */
	I7Skein *self = I7_SKEIN(model);
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);

	iter->stamp = priv->stamp;
	iter->user_data = priv->root;
	return TRUE;
}

static G_GNUC_CONST gboolean
i7_skein_iter_has_child(GtkTreeModel *model, GtkTreeIter *iter)
{
	return FALSE;
}

static int
i7_skein_iter_n_children(GtkTreeModel *model, GtkTreeIter *iter)
{
	I7Skein *self = I7_SKEIN(model);
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);

	/* If iter is NULL, return the number of toplevel nodes, i.e. the length of
	 the list */
	if(!iter) {
		I7Node *last = i7_skein_get_thread_bottom(self, priv->current);
		return g_node_depth(last->gnode);
	}
	return 0;
}

static gboolean
i7_skein_iter_nth_child(GtkTreeModel *model, GtkTreeIter *iter, GtkTreeIter *parent, int n)
{
	/* This is a list, nodes have no children */
	if(parent) {
		invalidate_iter(iter);
		return FALSE;
	}

	/* If parent was NULL, return the n'th toplevel node, i.e. the n'th node in
	 the list*/
	I7Skein *self = I7_SKEIN(model);
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);

	I7Node *last = i7_skein_get_thread_bottom(self, priv->current);
	int thread_depth = g_node_depth(last->gnode) - 1;
	/* g_node_depth returns 0 for NULL and 1 for the root node */
	
	if(n > thread_depth) {
		invalidate_iter(iter);
		return FALSE;
	}

	iter->stamp = priv->stamp;
	
	if(n == 0) {
		iter->user_data = priv->root;
		return TRUE;
	}
	if(n == thread_depth) {
		iter->user_data = last;
		return TRUE;
	}
	
	/* Make the uninitialized iterator point to the i'th node between the root
	 node and the bottom of the current node's thread */
	GNode *gnode = last->gnode;
	for( ; thread_depth > n; thread_depth--)
		gnode = gnode->parent;
	
	iter->user_data = gnode->data;
	return TRUE;
}

static gboolean
i7_skein_iter_parent(GtkTreeModel *model, GtkTreeIter *iter, GtkTreeIter *child)
{
	/* This is a list, nodes have no parents */
	invalidate_iter(iter);
	return FALSE;
}

static void
i7_skein_tree_model_init(GtkTreeModelIface *iface)
{
	iface->get_flags = i7_skein_get_flags;
	iface->get_n_columns = i7_skein_get_n_columns;
	iface->get_column_type = i7_skein_get_column_type;
	iface->get_iter = i7_skein_get_iter;
	iface->get_path = i7_skein_get_path;
	iface->get_value = i7_skein_get_value;
	iface->iter_next = i7_skein_iter_next;
	iface->iter_children = i7_skein_iter_children;
	iface->iter_has_child = i7_skein_iter_has_child;
	iface->iter_n_children = i7_skein_iter_n_children;
	iface->iter_nth_child = i7_skein_iter_nth_child;
	iface->iter_parent = i7_skein_iter_parent;
	iface->ref_node = NULL; /* optional */
	iface->unref_node = NULL; /* optional */
}

/* PUBLIC FUNCTIONS */

I7Skein *
i7_skein_new(void)
{
	return g_object_new(I7_TYPE_SKEIN, NULL);
}

GQuark
i7_skein_error_quark(void)
{
	return g_quark_from_static_string("i7-skein-error-quark");
}

I7Node *
i7_skein_get_root_node(I7Skein *self)
{
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);
	return priv->root;
}

I7Node *
i7_skein_get_current_node(I7Skein *self)
{
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);
	return priv->current;
}

void
i7_skein_set_current_node(I7Skein *self, I7Node *node)
{
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);

	if(priv->current == node)
		return;

	/* Old end of the list */
	I7Node *old_last = i7_skein_get_thread_bottom(self, priv->current);
	/* New end of the list */
	I7Node *last = i7_skein_get_thread_bottom(self, node);
	GNode *gnode = old_last->gnode;
	
	/* Emit row-inserted and row-deleted signals on our tree model interface */
	if(old_last != last) {
		/* Find a common ancestor of the old and new list bottoms */
		int depth = g_node_depth(gnode) - 1;
		for( ; gnode && !i7_node_in_thread(gnode->data, last); gnode = gnode->parent) {
			GtkTreePath *path = gtk_tree_path_new_from_indices(depth--, -1);
			gtk_tree_model_row_deleted(GTK_TREE_MODEL(self), path);
			gtk_tree_path_free(path);
		}
		/* gnode is now the common ancestor, possibly the root node if no
		 common ancestor, or possibly NULL if the tree is being rebuilt; in
		 the latter case, first add the root node to the model */
		if(gnode == NULL) {
			gnode = priv->root->gnode;
			GtkTreePath *path = gtk_tree_path_new_from_indices(0, -1);
			GtkTreeIter iter = { .stamp = priv->stamp, .user_data = priv->root };
			gtk_tree_model_row_inserted(GTK_TREE_MODEL(self), path, &iter);
			gtk_tree_path_free(path);
			depth = 0;
		}
		while(gnode != last->gnode) {
			for(gnode = gnode->children; gnode; gnode = gnode->next) {
				if(i7_node_in_thread(gnode->data, last))
					break;
			}
			GtkTreePath *path = gtk_tree_path_new_from_indices(++depth, -1);
			GtkTreeIter iter = { .stamp = priv->stamp, .user_data = gnode->data };
			gtk_tree_model_row_inserted(GTK_TREE_MODEL(self), path, &iter);
			gtk_tree_path_free(path);
		}
	}
	
	priv->current = node;
	g_object_notify(G_OBJECT(self), "current-node");
	g_signal_emit_by_name(self, "needs-layout");
}

gboolean
i7_skein_is_node_in_current_thread(I7Skein *self, I7Node *node)
{
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);
	I7Node *last = i7_skein_get_thread_bottom(self, priv->current);
	return i7_node_in_thread(node, last);
}

I7Node *
i7_skein_get_played_node(I7Skein *self)
{
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);
	return priv->played;
}

static gboolean
change_node_played(GNode *gnode, gpointer data)
{
	i7_node_set_played(I7_NODE(gnode->data), GPOINTER_TO_INT(data));
	return FALSE; /* Don't stop the traversal */
}

/* Private */
static void
i7_skein_set_played_node(I7Skein *self, I7Node *node)
{
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);

	/* Change the colors of the nodes */

	GNode *gnode;
	if(priv->played && i7_node_in_thread(priv->played, node)) {
		/* If the new played node is a descendant of the old played node, then
		 just change the colors in a line down to the new one */
		gnode = priv->played->gnode;
	}
	else {
		/* Otherwise, make every node unplayed first and start again at the
		 root node */
		g_node_traverse(priv->root->gnode, G_IN_ORDER, G_TRAVERSE_ALL, -1, (GNodeTraverseFunc)change_node_played, GINT_TO_POINTER(FALSE));
		gnode = priv->root->gnode;
	}

	/* Calculate which nodes should be "played" */
	do {
		i7_node_set_played(I7_NODE(gnode->data), TRUE);
		for(gnode = gnode->children; gnode; gnode = gnode->next) {
			if(i7_node_in_thread(I7_NODE(gnode->data), node))
				break;
		}
	} while(gnode);

	priv->played = node;
	g_object_notify(G_OBJECT(self), "played-node");
}

/* Search an xmlNode's properties for a certain one and return its content.
String must be freed. Return NULL if not found */
static gchar *
get_property_from_node(xmlNode *node, const gchar *propertyname)
{
	xmlAttr *props;
	for(props = node->properties; props; props = props->next)
		if(xmlStrEqual(props->name, (const xmlChar *)propertyname))
			return g_strdup((gchar *)props->children->content);
	return NULL;
}

/* Read the ObjectiveC "YES" and "NO" into a boolean, or return default_val if
 content is malformed */
static gboolean
get_boolean_from_content(xmlNode *node, gboolean default_val)
{
	if(xmlStrEqual(node->children->content, (xmlChar *)"YES"))
		return TRUE;
	else if(xmlStrEqual(node->children->content, (xmlChar *)"NO"))
		return FALSE;
	else
		return default_val;
}

static gchar *
get_text_content_from_node(xmlNode *node)
{
	if(node->children)
		if(xmlStrEqual(node->children->name, (xmlChar *)"text"))
			return g_strdup((gchar *)node->children->content);
	return NULL;
}

/* Doesn't actually free the node itself, but removes it from the canvas so that
 it gets freed. Has reversed arguments and returns FALSE for use in tree
 traversals. */
static gboolean
remove_node_from_canvas(GNode *gnode, I7Skein *self)
{
	if(I7_NODE(gnode->data)->tree_item)
		goo_canvas_item_model_remove(I7_NODE(gnode->data)->tree_item);
	goo_canvas_item_model_remove(GOO_CANVAS_ITEM_MODEL(gnode->data));
	return FALSE;
}

gboolean
i7_skein_load(I7Skein *self, GFile *file, GError **error)
{
	g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
	g_return_val_if_fail(file, FALSE);

	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);

	char *filename = g_file_get_path(file);
	xmlDoc *xmldoc = xmlParseFile(filename);
	g_free(filename);
	if(!xmldoc) {
		xmlErrorPtr xml_error = xmlGetLastError();
		if(error)
			*error = g_error_new_literal(I7_SKEIN_ERROR, I7_SKEIN_ERROR_XML, g_strdup(xml_error->message));
		return FALSE;
	}

	/* Get the top XML node */
	xmlNode *top = xmlDocGetRootElement(xmldoc);
	if(!xmlStrEqual(top->name, (xmlChar *)"Skein")) {
		if(error)
			*error = g_error_new(I7_SKEIN_ERROR, I7_SKEIN_ERROR_BAD_FORMAT, "<Skein> element not found.");
		goto fail;
	}

	/* Get the ID of the root node */
	gchar *root_id = get_property_from_node(top, "rootNode");
	if(!root_id) {
		if(error)
			*error = g_error_new(I7_SKEIN_ERROR, I7_SKEIN_ERROR_BAD_FORMAT, "rootNode attribute not found.");
		goto fail;
	}

	/* Get all the item nodes and the ID of the active node */
	GHashTable *nodetable = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
	gchar *active_id = NULL;
	xmlNode *item;
	/* Create a node object for each of the XML item nodes */
	for(item = top->children; item; item = item->next) {
		if(xmlStrEqual(item->name, (xmlChar *)"activeNode"))
			active_id = get_property_from_node(item, "nodeId");
		else if(xmlStrEqual(item->name, (xmlChar *)"item")) {
			gchar *id, *command = NULL, *label = NULL;
			gchar *transcript = NULL, *expected = NULL;
			gboolean unlocked = TRUE, changed = FALSE;
			int score = 0;

			xmlNode *child;
			for(child = item->children; child; child = child->next) {
				/* Ignore "played" and "changed"; these are calculated */
				if(xmlStrEqual(child->name, (xmlChar *)"command"))
					command = get_text_content_from_node(child);
				else if(xmlStrEqual(child->name, (xmlChar *)"annotation"))
					label = get_text_content_from_node(child);
				else if(xmlStrEqual(child->name, (xmlChar *)"result"))
					transcript = get_text_content_from_node(child);
				else if(xmlStrEqual(child->name, (xmlChar *)"commentary"))
					expected = get_text_content_from_node(child);
				else if(xmlStrEqual(child->name, (xmlChar *)"changed"))
					changed = get_boolean_from_content(child, FALSE);
				else if(xmlStrEqual(child->name, (xmlChar *)"temporary")) {
					unlocked = get_boolean_from_content(child, TRUE);
					gchar *trash = get_property_from_node(child, "score");
					sscanf(trash, "%d", &score);
					g_free(trash);
				}
			}
			id = get_property_from_node(item, "nodeId"); /* freed by table */

			I7Node *skein_node = i7_node_new(command, label, transcript, expected, FALSE, !unlocked, changed, score, GOO_CANVAS_ITEM_MODEL(self));
			node_listen(self, skein_node);
			g_hash_table_insert(nodetable, id, skein_node);
			g_free(command);
			g_free(label);
			g_free(transcript);
			g_free(expected);
		}
	}

	/* Loop through the item nodes again, adding the children to each parent */
	for(item = top->children; item; item = item->next) {
		if(!xmlStrEqual(item->name, (xmlChar *)"item"))
			continue;
		gchar *id = get_property_from_node(item, "nodeId");
		I7Node *parent_node = g_hash_table_lookup(nodetable, id);
		xmlNode *child;
		for(child = item->children; child; child = child->next) {
			if(!xmlStrEqual(child->name, (xmlChar *)"children"))
				continue;
			xmlNode *list;
			for(list = child->children; list; list = list->next) {
				if(!xmlStrEqual(list->name, (xmlChar *)"child"))
					continue;
				gchar *child_id = get_property_from_node(list, "nodeId");
				g_node_append(parent_node->gnode, I7_NODE(g_hash_table_lookup(nodetable, child_id))->gnode);
				g_free(child_id);
			}
		}
		g_free(id);
	}

	/* Discard the current skein and replace with the new */
	g_node_traverse(priv->root->gnode, G_POST_ORDER, G_TRAVERSE_ALL, -1, (GNodeTraverseFunc)remove_node_from_canvas, self);
	priv->root = I7_NODE(g_hash_table_lookup(nodetable, root_id));
	priv->played = NULL;
	i7_skein_set_played_node(self, I7_NODE(g_hash_table_lookup(nodetable, active_id)));
	i7_skein_set_current_node(self, priv->root);

	g_signal_emit_by_name(self, "needs-layout");
	g_signal_emit_by_name(self, "labels-changed");
	priv->modified = FALSE;

	g_free(root_id);
	g_free(active_id);
	g_hash_table_destroy(nodetable);

	return TRUE;
fail:
	xmlFreeDoc(xmldoc);
	return FALSE;
}

static gboolean
node_write_xml(GNode *gnode, GDataOutputStream *stream)
{
	gchar *xml = i7_node_get_xml(I7_NODE(gnode->data));
	g_data_output_stream_put_string(stream, xml, NULL, NULL);
	g_free(xml);
	return FALSE; /* Do not stop the traversal */
}

gboolean
i7_skein_save(I7Skein *self, GFile *file, GError **error)
{
	g_return_val_if_fail(error == NULL || *error == NULL, FALSE);

	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);

	GFileOutputStream *fstream = g_file_replace(file, NULL, FALSE, G_FILE_CREATE_NONE, NULL, error);
	if(!fstream)
		return FALSE;
	GDataOutputStream *skein_stream = g_data_output_stream_new(G_OUTPUT_STREAM(fstream));

	char *header = g_strdup_printf(
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
			"<Skein rootNode=\"%s\" "
			"xmlns=\"http://www.logicalshift.org.uk/IF/Skein\">\n"
			"  <generator>Inform 7</generator>\n"
			"  <activeNode nodeId=\"%s\"/>\n",
			i7_node_get_unique_id(priv->root),
			i7_node_get_unique_id(priv->current));
	if(!g_data_output_stream_put_string(skein_stream, header, NULL, error))
		return FALSE;

	g_node_traverse(priv->root->gnode, G_PRE_ORDER, G_TRAVERSE_ALL, -1, (GNodeTraverseFunc)node_write_xml, skein_stream);

	if(!g_data_output_stream_put_string(skein_stream, "</Skein>\n", NULL, error))
		return FALSE;

	g_object_unref(skein_stream);
	g_object_unref(fstream);

	priv->modified = FALSE;

	return TRUE;
}

/* Imports a list of commands into the Skein */
gboolean
i7_skein_import(I7Skein *self, GFile *file, GError **error)
{
	g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
	g_return_val_if_fail(file, FALSE);

	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);

	I7Node *node = priv->root;
	gboolean added = FALSE;

	GFileInputStream *istream = g_file_read(file, NULL, error);
	if(!istream)
		return FALSE;
	GDataInputStream *stream = g_data_input_stream_new(G_INPUT_STREAM(istream));

	gchar *line;
	while((line = g_data_input_stream_read_line(stream, NULL, NULL, error))) {
		g_strstrip(line);
		if(*line) {
			gchar *node_command = g_strescape(line, "\"");
			I7Node *newnode = i7_node_find_child(node, node_command);
			if(!newnode) {
				/* Wasn't found, create new node */
				newnode = i7_node_new(node_command, "", "", "", FALSE, FALSE, FALSE, 0, GOO_CANVAS_ITEM_MODEL(self));
				node_listen(self, newnode);
				g_node_append(node->gnode, newnode->gnode);
				added = TRUE;
			}
			g_free(node_command);
			node = newnode;	
		}
		g_free(line);
	}

	if(*error)
		goto fail;

	if(added) {
		g_signal_emit_by_name(self, "needs-layout");
		g_signal_emit_by_name(self, "modified");
	}

	g_object_unref(stream);
	g_object_unref(istream);
	return TRUE;

fail:
	g_object_unref(stream);
	g_object_unref(istream);
	return FALSE;
}

/* Rewinds the skein to the beginning; if @current is TRUE, also resets the
 view in the Transcript to the beginning */
void
i7_skein_reset(I7Skein *self, gboolean current)
{
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);
	if(current)
		i7_skein_set_current_node(self, priv->root);
	i7_skein_set_played_node(self, priv->root);
}

static void
draw_tree(I7Skein *self, I7Node *node, GooCanvas *canvas)
{
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);

	/* Draw a line from the node to its parent */
	if(node->gnode->parent) {
		/* Calculate the coordinates */
		gdouble nodex = i7_node_get_x(node);
		gdouble destx = i7_node_get_x(I7_NODE(node->gnode->parent->data));
		gdouble nodey = (gdouble)(g_node_depth(node->gnode) - 1.0) * priv->vspacing;
		gdouble desty = nodey - priv->vspacing;

		if(!node->tree_item)
			node->tree_item = goo_canvas_polyline_model_new(GOO_CANVAS_ITEM_MODEL(self), FALSE, 0, NULL);

		if(node->tree_points->coords[0] != destx || node->tree_points->coords[4] != nodex) {
			node->tree_points->coords[0] = node->tree_points->coords[2] = destx;
			node->tree_points->coords[1] = desty;
			node->tree_points->coords[3] = desty + 0.2 * priv->vspacing;
			node->tree_points->coords[4] = node->tree_points->coords[6] = nodex;
			node->tree_points->coords[5] = nodey - 0.2 * priv->vspacing;
			node->tree_points->coords[7] = nodey;
			g_object_set(node->tree_item, "points", node->tree_points, NULL);
		}

		gboolean in_current_thread = i7_skein_is_node_in_current_thread(self, node);
		GtkStyleContext *style = gtk_widget_get_style_context(GTK_WIDGET(canvas));
		GdkRGBA stroke_color;
		GtkStateFlags state = gtk_style_context_get_state(style);
		gtk_style_context_save(style);

		if (i7_node_get_locked(node)) {
			gtk_style_context_add_class(style, "locked-thread");
			gtk_style_context_get_color(style, state, &stroke_color);
			g_object_set(node->tree_item,
				"stroke-color-gdk-rgba", &stroke_color,
			    "line-dash", priv->locked_dash,
				"line-width", in_current_thread? 4.0 : 1.5,
				NULL);
		} else {
			gtk_style_context_add_class(style, "unlocked-thread");
			gtk_style_context_get_color(style, state, &stroke_color);
			g_object_set(node->tree_item,
				"stroke-color-gdk-rgba", &stroke_color,
				"line-dash", priv->unlocked_dash,
				"line-width", in_current_thread? 4.0 : 1.5,
				NULL);
		}

		gtk_style_context_restore(style);

		goo_canvas_item_model_lower(node->tree_item, NULL); /* put at bottom */
	}

	/* Draw the children's lines to this node */
	unsigned i;
	for(i = 0; i < g_node_n_children(node->gnode); i++) {
		I7Node *child = g_node_nth_child(node->gnode, i)->data;
		draw_tree(self, child, canvas);
	}
}

static void
draw_intern(I7Skein *self, GooCanvas *canvas)
{
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);

	if(GPOINTER_TO_INT(g_object_get_data(G_OBJECT(canvas), "waiting-for-draw")) == 0)
		return;

	i7_node_layout(priv->root, GOO_CANVAS_ITEM_MODEL(self), canvas, 0.0);

	gdouble treewidth = i7_node_get_tree_width(priv->root, GOO_CANVAS_ITEM_MODEL(self), canvas);
	draw_tree(self, priv->root, canvas);

	goo_canvas_set_bounds(canvas,
		-treewidth * 0.5 - priv->hspacing, -(priv->vspacing) * 0.5,
		treewidth * 0.5 + priv->hspacing, g_node_max_height(priv->root->gnode) * priv->vspacing);

	g_object_set_data(G_OBJECT(canvas), "waiting-for-draw", GINT_TO_POINTER(0));
}

void
i7_skein_draw(I7Skein *self, GooCanvas *canvas)
{
	g_object_set_data(G_OBJECT(canvas), "waiting-for-draw", GINT_TO_POINTER(1));
	draw_intern(self, canvas);
}

typedef struct {
	I7Skein *skein;
	GooCanvas *canvas;
} DrawData;

static gboolean
idle_draw(DrawData *draw_data)
{
	draw_intern(draw_data->skein, draw_data->canvas);
	return FALSE; /* one-shot */
}

static void
destroy_draw_data(DrawData *draw_data)
{
	g_slice_free(DrawData, draw_data);
}

void
i7_skein_schedule_draw(I7Skein *self, GooCanvas *canvas)
{
	g_object_set_data(G_OBJECT(canvas), "waiting-for-draw", GINT_TO_POINTER(1));
	
	DrawData *draw_data = g_slice_new0(DrawData);
	draw_data->skein = self;
	draw_data->canvas = canvas;
	gdk_threads_add_idle_full(G_PRIORITY_DEFAULT_IDLE, (GSourceFunc)idle_draw, draw_data, (GDestroyNotify)destroy_draw_data);
}

/* Remove all the rows from the tree model, in preparation for a complicated
 * modification */
static void
remove_all_from_model(I7Skein *self)
{
	int nrows = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(self), NULL);
	int count;
	for(count = nrows - 1; count >= 0; count--) {
		GtkTreePath *path = gtk_tree_path_new_from_indices(count, -1);
		gtk_tree_model_row_deleted(GTK_TREE_MODEL(self), path);
		gtk_tree_path_free(path);
	}
}

/* Put all the rows of the current thread back in the tree model, after
 * finishing a complicated modification */
static void
reinstate_all_in_model(I7Skein *self)
{
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);
	I7Node *last = i7_skein_get_thread_bottom(self, priv->current);
	GNode *gnode = priv->root->gnode;
	while(gnode) {
		int depth = g_node_depth(gnode) - 1;
		GtkTreePath *path = gtk_tree_path_new_from_indices(depth, -1);
		GtkTreeIter iter = { .stamp = priv->stamp, .user_data = gnode->data };
		gtk_tree_model_row_inserted(GTK_TREE_MODEL(self), path, &iter);
		gtk_tree_path_free(path);

		/* Advance to the next node in the thread */
		for(gnode = gnode->children; gnode; gnode = gnode->next) {
			if(i7_node_in_thread(gnode->data, last))
				break;
		}
	}
}

/* Add a new node with the given command, under the played node. Unless there
 is already a node with that command. In either case, return a pointer to that
 node. */
I7Node *
i7_skein_new_command(I7Skein *self, const gchar *command)
{
	g_return_val_if_fail(command != NULL, NULL);

	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);

	gboolean node_added = FALSE;
	gchar *node_command = g_strescape(command, "\"");

	I7Node *node = i7_node_find_child(priv->played, node_command);
	if(node == NULL) {
		/* Wasn't found, create new node */
		node = i7_node_new(node_command, "", "", "", TRUE, FALSE, FALSE, 0, GOO_CANVAS_ITEM_MODEL(self));
		node_listen(self, node);

		gboolean remove = i7_skein_is_node_in_current_thread(self, priv->played);
		if(remove)
		   remove_all_from_model(self);
		g_node_append(priv->played->gnode, node->gnode);
		if(remove)
			reinstate_all_in_model(self);
		node_added = TRUE;
	}
	g_free(node_command);

	/* Make this the new current node */
	i7_skein_set_current_node(self, node);
	i7_skein_set_played_node(self, node);

	/* Send signals */
	if(node_added)
		g_signal_emit_by_name(self, "needs-layout");
	g_signal_emit_by_name(self, "show-node", I7_REASON_COMMAND, node);
	g_signal_emit_by_name(self, "modified");

	return node;
}

/* Find the next node to use. Return TRUE if found, and if so, store a
newly-allocated copy of its command text in @command.*/
gboolean
i7_skein_next_command(I7Skein *self, gchar **command)
{
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);

	if(!g_node_is_ancestor(priv->played->gnode, priv->current->gnode))
		return FALSE;

	I7Node *next = priv->current;
	while(next->gnode->parent != priv->played->gnode)
		next = next->gnode->parent->data;
	priv->played = next;
	gchar *temp = i7_node_get_command(next);
	*command = g_strcompress(temp);
	g_free(temp);
	g_signal_emit_by_name(self, "show-node", I7_REASON_COMMAND, next);
	return TRUE;
}

/* Get a list of the commands from @from_node to @to_node. Returns NULL if
 @from_node is not an ancestor of @to_node. */
GSList *
i7_skein_get_commands_to_node(I7Skein *self, I7Node *from_node, I7Node *to_node)
{
	GSList *commands = NULL;
	GNode *pointer = from_node->gnode;
	while(g_node_is_ancestor(pointer, to_node->gnode)) {
		I7Node *next = to_node;
		while(next->gnode->parent != pointer)
			next = next->gnode->parent->data;
		pointer = next->gnode;
		gchar *skein_command = i7_node_get_command(next);
		commands = g_slist_prepend(commands, g_strcompress(skein_command));
		g_free(skein_command);
	}
	commands = g_slist_reverse(commands);
	return commands;
}

/* Get a list of the commands from the root node to the play pointer */
GSList *
i7_skein_get_commands(I7Skein *self)
{
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);
	return i7_skein_get_commands_to_node(self, priv->root, priv->played);
}

/* Update the status of the last played node */
void
i7_skein_update_after_playing(I7Skein *self, const gchar *transcript)
{
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);

	i7_node_set_played(priv->played, TRUE);
	if(strlen(transcript)) {
		i7_node_set_transcript_text(priv->played, transcript);
		g_signal_emit_by_name(self, "show-node", I7_REASON_TRANSCRIPT, priv->played);
	}
}

gboolean
i7_skein_get_command_from_history(I7Skein *self, gchar **command, int history)
{
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);

	/* Find the node to return the command from */
	GNode *gnode = priv->current->gnode;
	while(--history > 0) {
		gnode = gnode->parent;
		if(gnode == NULL)
			return FALSE;
	}
	/* Don't return the root node */
	if(G_NODE_IS_ROOT(gnode))
		return FALSE;

	*command = g_strdelimit(i7_node_get_command(I7_NODE(gnode->data)), "\b\f\n\r\t", ' ');
	return TRUE;
}

/* Add an empty node as the child of node */
I7Node *
i7_skein_add_new(I7Skein *self, I7Node *node)
{
	I7Node *newnode = i7_node_new("", "", "", "", FALSE, FALSE, FALSE, 0, GOO_CANVAS_ITEM_MODEL(self));
	node_listen(self, newnode);

	remove_all_from_model(self);
	g_node_append(node->gnode, newnode->gnode);
	reinstate_all_in_model(self);

	g_signal_emit_by_name(self, "needs-layout");
	g_signal_emit_by_name(self, "modified");

	return newnode;
}

I7Node *
i7_skein_add_new_parent(I7Skein *self, I7Node *node)
{
	I7Node *newnode = i7_node_new("", "", "", "", FALSE, FALSE, FALSE, 0, GOO_CANVAS_ITEM_MODEL(self));
	node_listen(self, newnode);

	remove_all_from_model(self);
	g_node_insert(node->gnode->parent, g_node_child_position(node->gnode->parent, node->gnode), newnode->gnode);
	g_node_unlink(node->gnode);
	g_node_append(newnode->gnode, node->gnode);
	reinstate_all_in_model(self);

	g_signal_emit_by_name(self, "needs-layout");
	g_signal_emit_by_name(self, "modified");

	return newnode;
}

/* Remove @node and all nodes below it */
gboolean
i7_skein_remove_all(I7Skein *self, I7Node *node)
{
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);

	if(G_NODE_IS_ROOT(node->gnode))
		return FALSE;

	if(i7_node_in_thread(node, priv->played))
		i7_skein_set_played_node(self, priv->root);
	if(i7_skein_is_node_in_current_thread(self, node))
		i7_skein_set_current_node(self, priv->root);
	
	remove_all_from_model(self);
	g_node_unlink(node->gnode);
	g_node_traverse(node->gnode, G_POST_ORDER, G_TRAVERSE_ALL, -1, (GNodeTraverseFunc)remove_node_from_canvas, self);
	reinstate_all_in_model(self);
	
	g_signal_emit_by_name(self, "needs-layout");
	g_signal_emit_by_name(self, "modified");
	return TRUE;
}

gboolean
i7_skein_remove_single(I7Skein *self, I7Node *node)
{
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);

	if(G_NODE_IS_ROOT(node->gnode))
		return FALSE;

	if(i7_node_in_thread(node, priv->played))
		i7_skein_set_played_node(self, priv->root);
	if(i7_skein_is_node_in_current_thread(self, node))
		i7_skein_set_current_node(self, priv->root);

	remove_all_from_model(self);
	if(!G_NODE_IS_LEAF(node->gnode)) {
		int i;
		for(i = g_node_n_children(node->gnode) - 1; i >= 0; i--) {
			GNode *iter = g_node_nth_child(node->gnode, i);
			g_node_unlink(iter);
			g_node_insert_after(node->gnode->parent, node->gnode, iter);
		}
	}
	g_node_unlink(node->gnode);
	remove_node_from_canvas(node->gnode, self);
	reinstate_all_in_model(self);
	
	g_signal_emit_by_name(self, "needs-layout");
	g_signal_emit_by_name(self, "modified");
	return TRUE;
}

void
i7_skein_lock(I7Skein *self, I7Node *node)
{
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);

	GNode *iter;
	for(iter = node->gnode; iter; iter = iter->parent)
		i7_node_set_locked(I7_NODE(iter->data), TRUE);

	/* TODO change thread colors */
	priv->modified = TRUE;
}

static void
i7_skein_unlock_recurse(I7Skein *self, I7Node *node)
{
	i7_node_set_locked(node, FALSE);
	GNode *iter;
	for(iter = node->gnode->children; iter; iter = iter->next)
		i7_skein_unlock(self, I7_NODE(iter->data));

	/* TODO change thread colors */
}

void
i7_skein_unlock(I7Skein *self, I7Node *node)
{
	i7_skein_unlock_recurse(self, node);
	g_signal_emit_by_name(self, "modified");
}

static void
i7_skein_trim_recurse(I7Skein *self, I7Node *node, gint min_score)
{
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);

	unsigned i = 0;
	while(i < g_node_n_children(node->gnode)) {
		I7Node *child = g_node_nth_child(node->gnode, i)->data;
		if(i7_node_get_locked(child) || i7_node_get_score(child) > min_score) {
			i7_skein_trim_recurse(self, child, min_score);
			i++;
		} else {
			if(i7_skein_is_node_in_current_thread(self, child)) {
				i7_skein_set_current_node(self, priv->root);
				i7_skein_set_played_node(self, priv->root);
			}

			if(i7_skein_remove_all(self, child) == FALSE)
				i++;
		}
	}
}

static gint
int_compare_reversed(gconstpointer a, gconstpointer b)
{
	return GPOINTER_TO_INT(a) < GPOINTER_TO_INT(b);
}

static gboolean
look_for_scores(GNode *gnode, GList **listptr)
{
	gint score = i7_node_get_score(I7_NODE(gnode->data));

	if(g_list_index(*listptr, GINT_TO_POINTER(score)) == -1)
		*listptr = g_list_insert_sorted(*listptr, GINT_TO_POINTER(score), int_compare_reversed);

	return FALSE; /* don't stop the traversal */
}

/*static void
print_score(gpointer item, gpointer data)
{
	g_print("%d ", GPOINTER_TO_INT(item));
}*/

void
i7_skein_trim(I7Skein *self, I7Node *node, gint max_temps)
{
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);

	GList *scores_in_use = NULL;
	g_node_traverse(priv->root->gnode, G_PRE_ORDER, G_TRAVERSE_ALL, -1, (GNodeTraverseFunc)look_for_scores, &scores_in_use);

	/*g_print("Scores in use: ");
	g_list_foreach(scores_in_use, print_score, NULL);
	g_print("\n");*/

	/* Keep only the highest @max_temps scores (and those that are locked, of course) */
	gint min_score = GPOINTER_TO_INT(g_list_nth_data(scores_in_use, max_temps));
	/*g_print("Min score: %d\n", min_score);*/
	g_list_free(scores_in_use);

	i7_skein_trim_recurse(self, node, min_score);
	g_signal_emit_by_name(self, "needs-layout");
	g_signal_emit_by_name(self, "modified");
}

static void
get_labels(GNode *gnode, GSList **labels)
{
	I7Node *node = gnode->data;
	if(i7_node_has_label(node)) {
		I7SkeinNodeLabel *nodelabel = g_slice_new0(I7SkeinNodeLabel);
		nodelabel->label = i7_node_get_label(node);
		nodelabel->node = node;
		*labels = g_slist_prepend(*labels, nodelabel);
	}
	g_node_children_foreach(gnode, G_TRAVERSE_ALL, (GNodeForeachFunc)get_labels, labels);
}

/* Free the list after use with the convenience function
 i7_skein_free_node_label_list() */
GSList *
i7_skein_get_labels(I7Skein *self)
{
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);
	GSList *labels = NULL;
	get_labels(priv->root->gnode, &labels);
	labels = g_slist_sort_with_data(labels, (GCompareDataFunc)strcmp, NULL);
	return labels;
}

static void
free_node_label(I7SkeinNodeLabel *label)
{
	g_free(label->label);
	g_slice_free(I7SkeinNodeLabel, label);
}

void
i7_skein_free_node_label_list(GSList *labels)
{
	g_slist_foreach(labels, (GFunc)free_node_label, NULL);
	g_slist_free(labels);
}

static gboolean
has_labels(I7Node *node)
{
	if(i7_node_has_label(node))
		return TRUE;

	GNode *iter;
	for(iter = node->gnode->children; iter; iter = iter->next)
		if(has_labels(iter->data))
			return TRUE;
	return FALSE;
}

gboolean
i7_skein_has_labels(I7Skein *self)
{
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);
	return has_labels(priv->root);
}

/*
 * i7_skein_bless:
 * @self: the skein
 * @node: the node to bless
 * @all: whether to bless all of @node's ancestors too
 *
 * Blesses @node, and optionally all of its ancestors. (Modifies the skein.)
 */
void
i7_skein_bless(I7Skein *self, I7Node *node, gboolean all)
{
	GNode *iter;
	for(iter = node->gnode; iter; iter = all? iter->parent : NULL)
		i7_node_bless(I7_NODE(iter->data));

	g_signal_emit_by_name(self, "modified");
}

gboolean
i7_skein_can_bless(I7Skein *self, I7Node *node, gboolean all)
{
	gboolean can_bless = FALSE;
	GNode *iter;

	for(iter = node->gnode; iter; iter = all? iter->parent : NULL)
		can_bless |= i7_node_get_changed(I7_NODE(iter->data));

	return can_bless;
}

I7Node *
i7_skein_get_thread_top(I7Skein *self, I7Node *node)
{
	while(TRUE) {
		if(G_NODE_IS_ROOT(node->gnode)) {
			g_assert_not_reached();
			return node;
		}
		if(g_node_n_children(node->gnode->parent) != 1)
			return node;
		if(G_NODE_IS_ROOT(node->gnode->parent))
			return node;
		node = node->gnode->parent->data;
	}
}

I7Node *
i7_skein_get_thread_bottom(I7Skein *self, I7Node *node)
{
	while(TRUE) {
		if(g_node_n_children(node->gnode) != 1)
			return node;
		node = node->gnode->children->data;
	}
}

/* Helper function: recursive function for i7_skein_get_blessed_thread_ends().
Start it at the root node. Prepends to @node_list a list of all the thread ends
under @node that need to be played through in order to have played through every
blessed knot in the skein at least once. Returns %TRUE if anything was prepended
to @node_list, %FALSE if not. */
static gboolean
get_blessed_thread_ends_recurse(GSList **node_list, I7Node *node)
{
	gboolean filled = FALSE;

	/* See if any of this node's children adds a node to the list */
	GNode *gnode = node->gnode;
	for(gnode = node->gnode->children; gnode; gnode = gnode->next) {
		if(get_blessed_thread_ends_recurse(node_list, (I7Node *)gnode->data))
			filled = TRUE;
	}

	/* If this node caused something to get filled, then return TRUE */
	if(filled)
		return TRUE;

	/* If this node has expected text, then add it to the list */
	if(i7_node_get_blessed(node)) {
		*node_list = g_slist_prepend(*node_list, node);
		return TRUE;
	}
	return FALSE;
}

/*
 * i7_skein_get_blessed_thread_ends:
 * @skein: the skein_command
 *
 * Makes a list of all the thread ends that need to be played through in order
 * to have played through every blessed knot in the skein at least once.
 *
 * Returns: singly-linked list of knots at the end of the threads. Free the
 * list using g_slist_free(), but not its contents.
 */
GSList *
i7_skein_get_blessed_thread_ends(I7Skein *self)
{
	GSList *retval = NULL;
	get_blessed_thread_ends_recurse(&retval, i7_skein_get_root_node(self));
	retval = g_slist_reverse(retval);
	return retval;
}

/* Returns whether the skein was modified since last save or load */
gboolean
i7_skein_get_modified(I7Skein *self)
{
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);
	return priv->modified;
}

static gboolean
invalidate(GNode *gnode)
{
	i7_node_invalidate_size(I7_NODE(gnode->data));
	return FALSE;
}

void
i7_skein_set_font(I7Skein *self, PangoFontDescription *font)
{
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);
	g_object_set(self, "font-desc", font, NULL);
	g_node_traverse(priv->root->gnode, G_PRE_ORDER, G_TRAVERSE_ALL, -1, (GNodeTraverseFunc)invalidate, NULL);
	g_signal_emit_by_name(self, "needs-layout");
}

/* DEBUG */
static void
i7_skein_node_dump(I7Node *node)
{
	gchar *command = i7_node_get_command(node);
	g_printerr("(%s)", command);
	g_free(command);
}

static void
dump_node_data(GNode *node, gpointer foo)
{
	i7_skein_node_dump(I7_NODE(node->data));
	if(g_node_n_children(node)) {
		g_printerr("->(");
		g_node_children_foreach(node, G_TRAVERSE_ALL, dump_node_data, foo);
		g_printerr(")");
	}
}

void
i7_skein_dump(I7Skein *self)
{
	I7SkeinPrivate *priv = i7_skein_get_instance_private(self);
	dump_node_data(priv->root->gnode, NULL);
	g_printerr("\n");
}
