/* Copyright (C) 2014, 2015 P. F. Chimento
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
#include <glib-object.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksourcebuffer.h>
#include "text-buffer-excerpt.h"

/* TYPE SYSTEM */

enum {
	PROP_0,
	PROP_ORIGINAL_BUFFER,
	NPROPS
};

static GParamSpec *i7_text_buffer_excerpt_properties[NPROPS] = { NULL, };

G_DEFINE_TYPE(I7TextBufferExcerpt, i7_text_buffer_excerpt, GTK_TYPE_SOURCE_BUFFER);

#define I7_TEXT_BUFFER_EXCERPT_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), I7_TYPE_TEXT_BUFFER_EXCERPT, I7TextBufferExcerptPrivate))
#define I7_TEXT_BUFFER_EXCERPT_USE_PRIVATE(o,n) I7TextBufferExcerptPrivate *n = I7_TEXT_BUFFER_EXCERPT_PRIVATE(o)

typedef struct
{
	GtkTextBuffer *orig_buffer;
	/* is a GtkSourceBuffer but stored as GtkTextBuffer to avoid much runtime
	typechecking */

	GtkTextMark *start_mark;
	GtkTextMark *end_mark;
	gboolean needs_sync;
	unsigned long excerpt_changed_handler;
	unsigned long orig_changed_handler;
} I7TextBufferExcerptPrivate;

/* Sync the original buffer's modified bit to the excerpt buffer. */
static void
on_modified_changed(I7TextBufferExcerpt *self)
{
	I7_TEXT_BUFFER_EXCERPT_USE_PRIVATE(self, priv);
	gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(self), gtk_text_buffer_get_modified(priv->orig_buffer));
}

/* Sync the excerpt buffer to the original buffer. */
static gboolean
sync_excerpt_to_original(I7TextBufferExcerpt *self)
{
	I7_TEXT_BUFFER_EXCERPT_USE_PRIVATE(self, priv);

	if(!priv->needs_sync)
		return G_SOURCE_REMOVE;

	GtkTextIter self_start, self_end, range_start, range_end;
	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(self), &self_start, &self_end);
	gtk_text_buffer_get_iter_at_mark(priv->orig_buffer, &range_start, priv->start_mark);
	gtk_text_buffer_get_iter_at_mark(priv->orig_buffer, &range_end, priv->end_mark);

	if(priv->orig_changed_handler != 0)
		g_signal_handler_block(priv->orig_buffer, priv->orig_changed_handler);
	gtk_text_buffer_begin_user_action(priv->orig_buffer);
	gtk_text_buffer_delete(priv->orig_buffer, &range_start, &range_end);
	gtk_text_buffer_insert_range(priv->orig_buffer, &range_start, &self_start, &self_end);
	gtk_text_buffer_end_user_action(priv->orig_buffer);
	if(priv->orig_changed_handler != 0)
		g_signal_handler_unblock(priv->orig_buffer, priv->orig_changed_handler);

	priv->needs_sync = FALSE;
	return G_SOURCE_REMOVE;
}

static void
on_excerpt_changed(I7TextBufferExcerpt *self)
{
	I7_TEXT_BUFFER_EXCERPT_USE_PRIVATE(self, priv);

	/* We can do this in idle time so as not to make keystrokes lag. Potentially
	if the original buffer was modified, this could overwrite the modifications.
	However, it's unlikely in our use, since if the user is typing then probably
	no other user actions are going to be modifying the underlying buffer.
	Syncing the other way should be immediate, though. */
	priv->needs_sync = TRUE;
	g_idle_add((GSourceFunc)sync_excerpt_to_original, self);
}

/* Sync the original buffer to the excerpt buffer. */
static void
on_original_changed(I7TextBufferExcerpt *self)
{
	I7_TEXT_BUFFER_EXCERPT_USE_PRIVATE(self, priv);

	GtkTextIter self_start, self_end, range_start, range_end;
	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(self), &self_start, &self_end);
	gtk_text_buffer_get_iter_at_mark(priv->orig_buffer, &range_start, priv->start_mark);
	gtk_text_buffer_get_iter_at_mark(priv->orig_buffer, &range_end, priv->end_mark);

	if(priv->excerpt_changed_handler != 0)
		g_signal_handler_block(self, priv->excerpt_changed_handler);
	gtk_text_buffer_delete(GTK_TEXT_BUFFER(self), &self_start, &self_end);
	gtk_text_buffer_insert_range(GTK_TEXT_BUFFER(self), &self_start, &range_start, &range_end);
	if(priv->excerpt_changed_handler != 0)
		g_signal_handler_unblock(self, priv->excerpt_changed_handler);
}

/* Sets the original buffer from which this buffer is an excerpt. This object is
now showing an excerpt of @orig_buffer. To start with, that excerpt is the
entirety of @orig_buffer; to narrow the excerpt down, use set_range(). */
static void
set_orig_buffer(I7TextBufferExcerpt *self, GtkSourceBuffer *orig_buffer)
{
	I7_TEXT_BUFFER_EXCERPT_USE_PRIVATE(self, priv);
	priv->orig_buffer = GTK_TEXT_BUFFER(g_object_ref(orig_buffer));

	GtkTextIter start, end;
	gtk_text_buffer_get_bounds(priv->orig_buffer, &start, &end);
	gtk_text_buffer_add_mark(priv->orig_buffer, priv->start_mark, &start);
	gtk_text_buffer_add_mark(priv->orig_buffer, priv->end_mark, &end);

	g_signal_connect(self, "modified-changed", G_CALLBACK(on_modified_changed), NULL);
	g_signal_connect_swapped(priv->orig_buffer, "modified-changed", G_CALLBACK(on_modified_changed), self);
	priv->excerpt_changed_handler = g_signal_connect(self, "changed", G_CALLBACK(on_excerpt_changed), NULL);
	/* This could be made more efficient by only syncing the excerpt when
	something happens inside the range of interest. */
	priv->orig_changed_handler = g_signal_connect_swapped(priv->orig_buffer, "changed", G_CALLBACK(on_original_changed), self);

	on_original_changed(self);
}

/* Release references to the original buffer. */
static void
clear_orig_buffer(I7TextBufferExcerpt *self)
{
	I7_TEXT_BUFFER_EXCERPT_USE_PRIVATE(self, priv);

	gtk_text_buffer_delete_mark(priv->orig_buffer, priv->start_mark);
	gtk_text_buffer_delete_mark(priv->orig_buffer, priv->end_mark);
	g_clear_object(&priv->orig_buffer);
	priv->excerpt_changed_handler = 0;
	priv->orig_changed_handler = 0;
}

static void
i7_text_buffer_excerpt_get_property(GObject *object, unsigned prop_id, GValue *value, GParamSpec *pspec)
{
	I7TextBufferExcerpt *self = I7_TEXT_BUFFER_EXCERPT(object);
	I7_TEXT_BUFFER_EXCERPT_USE_PRIVATE(self, priv);

	switch(prop_id) {
		case PROP_ORIGINAL_BUFFER:
			g_value_set_object(value, priv->orig_buffer);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
i7_text_buffer_excerpt_set_property(GObject *object, unsigned prop_id, const GValue *value, GParamSpec *pspec)
{
	I7TextBufferExcerpt *self = I7_TEXT_BUFFER_EXCERPT(object);

	switch(prop_id) {
		case PROP_ORIGINAL_BUFFER:
			set_orig_buffer(self, g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
i7_text_buffer_excerpt_finalize(GObject *object)
{
	I7TextBufferExcerpt *self = I7_TEXT_BUFFER_EXCERPT(object);
	I7_TEXT_BUFFER_EXCERPT_USE_PRIVATE(self, priv);

	clear_orig_buffer(self);
	g_clear_object(&priv->start_mark);
	g_clear_object(&priv->end_mark);

	G_OBJECT_CLASS(i7_text_buffer_excerpt_parent_class)->finalize(object);
}

static void
i7_text_buffer_excerpt_class_init(I7TextBufferExcerptClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(I7TextBufferExcerptPrivate));

	object_class->get_property = i7_text_buffer_excerpt_get_property;
	object_class->set_property = i7_text_buffer_excerpt_set_property;
	object_class->finalize = i7_text_buffer_excerpt_finalize;

	i7_text_buffer_excerpt_properties[PROP_ORIGINAL_BUFFER] =
		g_param_spec_object("original-buffer", _("Original buffer"),
		    _("The text buffer from which this is an excerpt"),
			GTK_TYPE_SOURCE_BUFFER, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(object_class, NPROPS, i7_text_buffer_excerpt_properties);
}

static void
i7_text_buffer_excerpt_init(I7TextBufferExcerpt *self)
{
	I7_TEXT_BUFFER_EXCERPT_USE_PRIVATE(self, priv);
	char *start_mark_name = g_strdup_printf("text-buffer-excerpt-%p-start", self);
	char *end_mark_name = g_strdup_printf("text-buffer-excerpt-%p-end", self);

	priv->start_mark = gtk_text_mark_new(start_mark_name, TRUE);
	priv->end_mark = gtk_text_mark_new(end_mark_name, FALSE);

	g_free(start_mark_name);
	g_free(end_mark_name);
}

/*
 * i7_text_buffer_excerpt_new:
 * @orig_buffer: the original buffer from which this is an excerpt.
 *
 * Create a new #I7TextBufferExcerpt.
 *
 * If creating an #I7TextBufferExcerpt through other means than this function,
 * you'll need to remember to set the #GtkTextBuffer:tag-table property to
 * @orig_buffer's tag table and the #GtkSourceBuffer:undo-manager property to
 * @orig_buffer's undo manager.
 * FIXME: Override constructor() to do this automatically.
 *
 * Returns: (transfer full): the new object.
 */
I7TextBufferExcerpt *
i7_text_buffer_excerpt_new(GtkSourceBuffer *orig_buffer)
{
	g_return_val_if_fail(GTK_IS_SOURCE_BUFFER(orig_buffer), NULL);

	GtkSourceUndoManager *undo_manager;
	g_object_get(orig_buffer,
		"undo-manager", &undo_manager,
		NULL);

	return I7_TEXT_BUFFER_EXCERPT(g_object_new(I7_TYPE_TEXT_BUFFER_EXCERPT,
		"original-buffer", orig_buffer,
		"tag-table", gtk_text_buffer_get_tag_table(GTK_TEXT_BUFFER(orig_buffer)),
		"undo-manager", undo_manager,
		NULL));
}

/*
 * i7_text_buffer_excerpt_get_original_buffer:
 * @self: the object
 *
 * Get the original #GtkSourceBuffer from which this buffer is an excerpt.
 *
 * Returns: (transfer none): the #GtkSourceBuffer previously set on this object.
 */
GtkSourceBuffer *
i7_text_buffer_excerpt_get_original_buffer(I7TextBufferExcerpt *self)
{
	g_return_val_if_fail(I7_IS_TEXT_BUFFER_EXCERPT(self), NULL);

	I7_TEXT_BUFFER_EXCERPT_USE_PRIVATE(self, priv);
	return GTK_SOURCE_BUFFER(priv->orig_buffer);
}

/*
 * i7_text_buffer_excerpt_get_range:
 * @self: the object
 * @start: (out caller-allocates): #GtkTextIter to fill in with the start of the
 * excerpted range
 * @end: (out caller-allocates): #GtkTextIter to fill in with the end of the
 * excerpted range
 *
 * Gets the range from the original buffer of which an excerpt is currently
 * showing in the text buffer excerpt.
 * The iterators @start and @end are on the original buffer.
 *
 * If no range has been set, then @start and @end will be set to the bounds of
 * the original buffer.
 */
void
i7_text_buffer_excerpt_get_range(I7TextBufferExcerpt *self, GtkTextIter *start, GtkTextIter *end)
{
	g_return_if_fail(I7_IS_TEXT_BUFFER_EXCERPT(self));

	I7_TEXT_BUFFER_EXCERPT_USE_PRIVATE(self, priv);

	gtk_text_buffer_get_iter_at_mark(priv->orig_buffer, start, priv->start_mark);
	gtk_text_buffer_get_iter_at_mark(priv->orig_buffer, end, priv->end_mark);
}

/*
 * i7_text_buffer_excerpt_set_range:
 * @self: the object
 * @start: #GtkTextIter in the original buffer pointing to the start of the
 * range to excerpt
 * @end: #GtkTextIter in the original buffer pointing to the end of the range to
 * excerpt
 *
 * Sets the range from the original buffer to show an excerpt from.
 */
void
i7_text_buffer_excerpt_set_range(I7TextBufferExcerpt *self, const GtkTextIter *start, const GtkTextIter *end)
{
	g_return_if_fail(I7_IS_TEXT_BUFFER_EXCERPT(self));

	I7_TEXT_BUFFER_EXCERPT_USE_PRIVATE(self, priv);

	g_return_if_fail(gtk_text_iter_get_buffer(start) == priv->orig_buffer);
	g_return_if_fail(gtk_text_iter_get_buffer(end) == priv->orig_buffer);

	gtk_text_buffer_move_mark(priv->orig_buffer, priv->start_mark, start);
	gtk_text_buffer_move_mark(priv->orig_buffer, priv->end_mark, end);

	on_original_changed(self);
}

/*
 * i7_text_buffer_excerpt_clear_range:
 * @self: the object
 *
 * Sets the buffer excerpt to display the entire original buffer.
 * This is equivalent to calling i7_text_buffer_excerpt_set_range() with @start
 * and @end equal to the start and end iterators of the original buffer.
 */
void
i7_text_buffer_excerpt_clear_range(I7TextBufferExcerpt *self)
{
	g_return_if_fail(I7_IS_TEXT_BUFFER_EXCERPT(self));

	I7_TEXT_BUFFER_EXCERPT_USE_PRIVATE(self, priv);

	GtkTextIter start, end;
	gtk_text_buffer_get_bounds(priv->orig_buffer, &start, &end);
	i7_text_buffer_excerpt_set_range(self, &start, &end);
}

/*
 * i7_text_buffer_excerpt_iters_in_range:
 * @self: the object
 * @start: a #GtkTextIter on the original buffer
 * @end: a #GtkTextIter on the original buffer
 *
 * Tells whether the range indicated by @start and @end is in the excerpted
 * range.
 * Note that this is a slightly different answer than gtk_text_iter_in_range()
 * would give, because the end bound is inclusive.
 *
 * Returns: %TRUE if @start and @end are both in the excerpted range, %FALSE
 * otherwise.
 */
gboolean
i7_text_buffer_excerpt_iters_in_range(I7TextBufferExcerpt *self, const GtkTextIter *start, const GtkTextIter *end)
{
	g_return_val_if_fail(I7_IS_TEXT_BUFFER_EXCERPT(self), FALSE);

	I7_TEXT_BUFFER_EXCERPT_USE_PRIVATE(self, priv);

	g_return_val_if_fail(gtk_text_iter_get_buffer(start) == priv->orig_buffer, FALSE);
	g_return_val_if_fail(gtk_text_iter_get_buffer(end) == priv->orig_buffer, FALSE);

	GtkTextIter excerpt_start, excerpt_end;
	i7_text_buffer_excerpt_get_range(self, &excerpt_start, &excerpt_end);
	return gtk_text_iter_in_range(start, &excerpt_start, &excerpt_end) &&
		(gtk_text_iter_in_range(end, &excerpt_start, &excerpt_end) || gtk_text_iter_equal(end, &excerpt_end));
}

/*
 * i7_text_buffer_excerpt_iters_from_buffer_iters:
 * @self: the object
 * @start (inout): a #GtkTextIter on the original buffer
 * @end (inout): a #GtkTextIter on the original buffer
 *
 * Converts @start and @end to their corresponding iters on the excerpt buffer.
 * The iters are valid after this method call but point to a different buffer
 * than they previously did.
 *
 * If @start or @end are not in the excerpt (see
 * i7_text_buffer_excerpt_iters_in_range()) then @start will be coerced to the
 * start of the excerpt, and/or @end will be coerced to the end.
 *
 * Returns: %FALSE if either @start or @end was coerced, %TRUE otherwise.
 */
gboolean
i7_text_buffer_excerpt_iters_from_buffer_iters(I7TextBufferExcerpt *self, GtkTextIter *start, GtkTextIter *end)
{
	g_return_if_fail(I7_IS_TEXT_BUFFER_EXCERPT(self));

	I7_TEXT_BUFFER_EXCERPT_USE_PRIVATE(self, priv);

	g_return_if_fail(gtk_text_iter_get_buffer(start) == priv->orig_buffer);
	g_return_if_fail(gtk_text_iter_get_buffer(end) == priv->orig_buffer);

	GtkTextIter excerpt_start, excerpt_end;
	gtk_text_buffer_get_iter_at_mark(priv->orig_buffer, &excerpt_start, priv->start_mark);
	gtk_text_buffer_get_iter_at_mark(priv->orig_buffer, &excerpt_end, priv->end_mark);
	int excerpt_offset = gtk_text_iter_get_offset(&excerpt_start);
	int excerpt_end_offset = gtk_text_iter_get_offset(&excerpt_end);
	int start_offset = gtk_text_iter_get_offset(start) - excerpt_offset;
	int end_offset = gtk_text_iter_get_offset(end) - excerpt_offset;

	gboolean was_coerced = FALSE;
	if(start_offset < 0 || start_offset > excerpt_end_offset) {
		start_offset = 0;
		was_coerced = TRUE;
	}
	if(end_offset < 0 || end_offset > excerpt_end_offset) {
		end_offset = -1;
		was_coerced = TRUE;
	}

	gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(self), start, start_offset);
	gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(self), end, end_offset);

	return !was_coerced;
}
