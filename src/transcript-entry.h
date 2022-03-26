/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2022 Philip Chimento <philip.chimento@gmail.com>
 */

#pragma once

#include "config.h"

#include <glib-object.h>
#include <gtk/gtk.h>

#include "node.h"

G_BEGIN_DECLS

#define I7_TYPE_TRANSCRIPT_ENTRY i7_transcript_entry_get_type()
G_DECLARE_FINAL_TYPE(I7TranscriptEntry, i7_transcript_entry, I7, TRANSCRIPT_ENTRY, GtkGrid)

GtkWidget *i7_transcript_entry_new(I7Node *node);

G_END_DECLS
