/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * new-i7
 * Copyright (C) P. F. Chimento 2010 <philip.chimento@gmail.com>
 * 
 * new-i7 is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * new-i7 is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SKEIN_VIEW_H_
#define _SKEIN_VIEW_H_

#include <glib-object.h>
#include "skein.h"
#include "node.h"

G_BEGIN_DECLS

#define I7_TYPE_SKEIN_VIEW             (i7_skein_view_get_type ())
#define I7_SKEIN_VIEW(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), I7_TYPE_SKEIN_VIEW, I7SkeinView))
#define I7_SKEIN_VIEW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), I7_TYPE_SKEIN_VIEW, I7SkeinViewClass))
#define I7_IS_SKEIN_VIEW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), I7_TYPE_SKEIN_VIEW))
#define I7_IS_SKEIN_VIEW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), I7_TYPE_SKEIN_VIEW))
#define I7_SKEIN_VIEW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), I7_TYPE_SKEIN_VIEW, I7SkeinViewClass))

typedef struct _I7SkeinViewClass I7SkeinViewClass;
typedef struct _I7SkeinView I7SkeinView;

struct _I7SkeinViewClass
{
	GooCanvasClass parent_class;
	/* Signals */
	void(* node_menu_popup) (I7SkeinView *self, I7Node *node);
};

struct _I7SkeinView
{
	GooCanvas parent_instance;
};

GType i7_skein_view_get_type(void) G_GNUC_CONST;
GtkWidget *i7_skein_view_new(void);
void i7_skein_view_set_skein(I7SkeinView *self, I7Skein *skein);
I7Skein *i7_skein_view_get_skein(I7SkeinView *self);
void i7_skein_view_edit_node(I7SkeinView *self, I7Node *node);
void i7_skein_view_edit_label(I7SkeinView *self, I7Node *node);
void i7_skein_view_show_node(I7SkeinView *self, I7Node *node, I7SkeinShowNodeReason why);

G_END_DECLS

#endif /* _SKEIN_VIEW_H_ */
