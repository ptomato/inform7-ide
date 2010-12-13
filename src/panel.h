/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * new-i7
 * Copyright (C) P. F. Chimento 2008 <philip.chimento@gmail.com>
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

#ifndef _PANEL_H_
#define _PANEL_H_

#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>
#include "source-view.h"

#define I7_TYPE_PANEL             (i7_panel_get_type())
#define I7_PANEL(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), I7_TYPE_PANEL, I7Panel))
#define I7_PANEL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), I7_TYPE_PANEL, I7PanelClass))
#define I7_IS_PANEL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), I7_TYPE_PANEL))
#define I7_IS_PANEL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), I7_TYPE_PANEL))
#define I7_PANEL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), I7_TYPE_PANEL, I7PanelClass))

/* The names of the panes in each notebook */
typedef enum {
    I7_PANE_SOURCE = 0,
    I7_PANE_ERRORS,
    I7_PANE_INDEX,
    I7_PANE_SKEIN,
    I7_PANE_TRANSCRIPT,
    I7_PANE_GAME,
    I7_PANE_DOCUMENTATION,
    I7_PANE_SETTINGS,
    I7_PANEL_NUM_PANES
} I7PanelPane;

/* The names of the sub tabs in "Errors" */
typedef enum {
    I7_ERRORS_TAB_PROGRESS = 0,
    I7_ERRORS_TAB_DEBUGGING,
	I7_ERRORS_TAB_PROBLEMS,
    I7_ERRORS_TAB_INFORM6,
    I7_ERRORS_NUM_TABS
} I7PaneErrorsTab;

/* The names of the sub tabs in "Index" */
typedef enum {
    I7_INDEX_TAB_ACTIONS = 0,
    I7_INDEX_TAB_CONTENTS,
    I7_INDEX_TAB_KINDS,
    I7_INDEX_TAB_PHRASEBOOK,
    I7_INDEX_TAB_RULES,
    I7_INDEX_TAB_SCENES,
    I7_INDEX_TAB_WORLD,
    I7_INDEX_NUM_TABS
} I7PaneIndexTab;

typedef struct {
	GtkVBox parent_instance;
	/* Public pointers to widgets for convenience */
	GtkWidget *toolbar;
	GtkWidget *notebook;
	I7SourceView *sourceview;
	GtkToolItem *labels;
	GtkWidget *labels_menu;
	GtkAction *labels_action;
	GtkWidget *z5;
	GtkWidget *z8;
	GtkWidget *z6;
	GtkWidget *glulx;
	GtkWidget *blorb;
	GtkWidget *nobble_rng;
	GtkWidget *debugging_scrolledwindow;
	GtkWidget *inform6_scrolledwindow;
	GtkWidget *tabs[I7_PANEL_NUM_PANES];
	GtkWidget *source_tabs[I7_SOURCE_VIEW_NUM_TABS];
	GtkWidget *errors_tabs[I7_ERRORS_NUM_TABS];
	GtkWidget *index_tabs[I7_INDEX_NUM_TABS];
} I7Panel;

typedef struct {
	GtkVBoxClass parent_class;
	void (*select_view)(I7Panel *self, I7PanelPane pane);
	void (*paste_code)(I7Panel *self, gchar *text);
	void (*jump_to_line)(I7Panel *self, guint line);
	void (*display_docpage)(I7Panel *self, gchar *uri);
} I7PanelClass;

GType i7_panel_get_type() G_GNUC_CONST;
GtkWidget *i7_panel_new();
void i7_panel_reset_queue(I7Panel *self, I7PanelPane pane, gint tab, const gchar *page);
void i7_panel_goto_docpage(I7Panel *self, const gchar *file);
void i7_panel_goto_docpage_at_anchor(I7Panel *self, const gchar *file, const gchar *anchor);
void i7_panel_update_tabs(I7Panel *self);
void i7_panel_update_fonts(I7Panel *self);
void i7_panel_update_font_sizes(I7Panel *self);

#endif /* _PANEL_H_ */
