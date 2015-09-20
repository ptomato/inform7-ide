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

#ifndef TEXT_BUFFER_EXCERPT_H
#define TEXT_BUFFER_EXCERPT_H

#include <glib-object.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksourcebuffer.h>

G_BEGIN_DECLS

#define I7_TYPE_TEXT_BUFFER_EXCERPT             (i7_text_buffer_excerpt_get_type())
#define I7_TEXT_BUFFER_EXCERPT(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), I7_TYPE_TEXT_BUFFER_EXCERPT, I7TextBufferExcerpt))
#define I7_TEXT_BUFFER_EXCERPT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), I7_TYPE_TEXT_BUFFER_EXCERPT, I7TextBufferExcerptClass))
#define I7_IS_TEXT_BUFFER_EXCERPT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), I7_TYPE_TEXT_BUFFER_EXCERPT))
#define I7_IS_TEXT_BUFFER_EXCERPT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), I7_TYPE_TEXT_BUFFER_EXCERPT))
#define I7_TEXT_BUFFER_EXCERPT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), I7_TYPE_TEXT_BUFFER_EXCERPT, I7TextBufferExcerptClass))

typedef struct _I7TextBufferExcerpt I7TextBufferExcerpt;
typedef struct _I7TextBufferExcerptClass I7TextBufferExcerptClass;

struct _I7TextBufferExcerpt
{
	GtkSourceBuffer parent_instance;
};

struct _I7TextBufferExcerptClass
{
	GtkSourceBufferClass parent_class;
};

GType i7_text_buffer_excerpt_get_type(void) G_GNUC_CONST;
I7TextBufferExcerpt *i7_text_buffer_excerpt_new(GtkSourceBuffer *orig_buffer);
GtkSourceBuffer *i7_text_buffer_excerpt_get_original_buffer(I7TextBufferExcerpt *self);
void i7_text_buffer_excerpt_get_range(I7TextBufferExcerpt *self, GtkTextIter *start, GtkTextIter *end);
void i7_text_buffer_excerpt_set_range(I7TextBufferExcerpt *self, const GtkTextIter *start, const GtkTextIter *end);
void i7_text_buffer_excerpt_clear_range(I7TextBufferExcerpt *self);
gboolean i7_text_buffer_excerpt_iters_in_range(I7TextBufferExcerpt *self, const GtkTextIter *start, const GtkTextIter *end);
gboolean i7_text_buffer_excerpt_iters_from_buffer_iters(I7TextBufferExcerpt *self, GtkTextIter *start, GtkTextIter *end);

G_END_DECLS

#endif /* TEXT_BUFFER_EXCERPT_H */
