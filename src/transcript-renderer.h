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

#ifndef __TRANSCRIPT_RENDERER_H__
#define __TRANSCRIPT_RENDERER_H__

#include "config.h"

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define I7_TYPE_CELL_RENDERER_TRANSCRIPT         (i7_cell_renderer_transcript_get_type())
#define I7_CELL_RENDERER_TRANSCRIPT(o)           (G_TYPE_CHECK_INSTANCE_CAST((o), I7_TYPE_CELL_RENDERER_TRANSCRIPT, I7CellRendererTranscript))
#define I7_CELL_RENDERER_TRANSCRIPT_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), I7_TYPE_CELL_RENDERER_TRANSCRIPT, I7CellRendererTranscriptClass))
#define I7_IS_CELL_RENDERER_TRANSCRIPT(o)        (G_TYPE_CHECK_INSTANCE_TYPE((o), I7_TYPE_CELL_RENDERER_TRANSCRIPT))
#define I7_IS_CELL_RENDERER_TRANSCRIPT_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), I7_TYPE_CELL_RENDERER_TRANSCRIPT))
#define I7_CELL_RENDERER_TRANSCRIPT_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS((o), I7_TYPE_CELL_RENDERER_TRANSCRIPT, I7CellRendererTranscriptClass))

typedef struct  {
	GtkCellRendererClass parent_class;
} I7CellRendererTranscriptClass;

typedef struct {
	GtkCellRenderer parent_instance;
} I7CellRendererTranscript;

GType i7_cell_renderer_transcript_get_type(void) G_GNUC_CONST;
I7CellRendererTranscript *i7_cell_renderer_transcript_new(void);

G_END_DECLS

#endif /* __TRANSCRIPT_RENDERER_H__ */
