/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2006-2010, 2014 Philip Chimento <philip.chimento@gmail.com>
 */

#ifndef _HISTORY_H
#define _HISTORY_H

#include "config.h"

#include <glib.h>

#include "panel.h"

void i7_panel_history_free(I7PanelHistory *self);

void history_goto_current(I7Panel *panel);
void history_push_pane(I7Panel *panel, I7PanelPane pane);
void history_push_tab(I7Panel *panel, I7PanelPane pane, guint tab);
void history_push_docpage(I7Panel *panel, const gchar *uri);
void history_push_extensions_page(I7Panel *panel, const char *uri);

#endif /* _HISTORY_H */
