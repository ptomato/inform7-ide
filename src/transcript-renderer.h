/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2011 Philip Chimento <philip.chimento@gmail.com>
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
