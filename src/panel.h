/* Copyright (C) 2008, 2009, 2010, 2011, 2014, 2015, 2019 P. F. Chimento
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

#ifndef _PANEL_H_
#define _PANEL_H_

#include "config.h"

#include <glib.h>
#include <glib-object.h>
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
	I7_PANE_RESULTS,
	I7_PANE_INDEX,
	I7_PANE_SKEIN,
	I7_PANE_TRANSCRIPT,
	I7_PANE_STORY,
	I7_PANE_DOCUMENTATION,
	I7_PANE_EXTENSIONS,
	I7_PANE_SETTINGS,
	I7_PANEL_NUM_PANES,
	I7_PANE_NONE = -1
} I7PanelPane;

/* The names of the sub tabs in "Results" */
typedef enum {
	I7_RESULTS_TAB_PROGRESS = 0,
	I7_RESULTS_TAB_DEBUGGING,
	I7_RESULTS_TAB_REPORT,
	I7_RESULTS_TAB_INFORM6,
	I7_RESULTS_NUM_TABS
} I7PaneResultsTab;

/* The names of the sub tabs in "Index" */
typedef enum {
	I7_INDEX_TAB_WELCOME = 0,
	I7_INDEX_TAB_CONTENTS,
	I7_INDEX_TAB_ACTIONS,
	I7_INDEX_TAB_KINDS,
	I7_INDEX_TAB_PHRASEBOOK,
	I7_INDEX_TAB_RULES,
	I7_INDEX_TAB_SCENES,
	I7_INDEX_TAB_WORLD,
	I7_INDEX_NUM_TABS,
	I7_INDEX_TAB_NONE = -1
} I7PaneIndexTab;

typedef struct {
	GtkVBox parent_instance;
	/* Public pointers to widgets for convenience */
	GtkWidget *toolbar;
	GtkWidget *notebook;
	I7SourceView *sourceview;
	GMenu *labels_menu;
	GtkWidget *z8;
	GtkWidget *glulx;
	GtkWidget *blorb;
	GtkWidget *nobble_rng;
	GtkWidget *debugging_scrolledwindow;
	GtkWidget *inform6_scrolledwindow;
	GtkWidget *transcript_menu;
	GtkWidget *labels;
	GtkWidget *layout;
	GtkWidget *trim;
	GtkWidget *play_all_blessed;
	GtkWidget *next_difference_skein;
	GtkWidget *next_difference;
	GtkWidget *previous_difference;
	GtkWidget *bless_all;
	GtkWidget *goto_home;
	GtkWidget *goto_examples;
	GtkWidget *goto_general_index;
	GtkWidget *goto_extensions_home;
	GtkWidget *goto_definitions;
	GtkWidget *goto_public_library;
	GtkTreeViewColumn *transcript_column;
	GtkCellRenderer *transcript_cell;
	GtkWidget *tabs[I7_PANEL_NUM_PANES];
	GtkWidget *source_tabs[I7_SOURCE_VIEW_NUM_TABS];
	GtkWidget *results_tabs[I7_RESULTS_NUM_TABS];
	GtkWidget *index_tabs[I7_INDEX_NUM_TABS];
	GSimpleActionGroup *actions;
} I7Panel;

typedef struct {
	GtkVBoxClass parent_class;
	void (*select_view)(I7Panel *self, I7PanelPane pane);
	void (*paste_code)(I7Panel *self, gchar *text);
	void (*jump_to_line)(I7Panel *self, guint line);
	void (*display_docpage)(I7Panel *self, gchar *uri);
	void (*display_extensions_docpage)(I7Panel *self, char *uri);
	void (*display_index_page)(I7Panel *self, I7PaneIndexTab tabnum);
} I7PanelClass;

typedef struct {
  I7PanelPane pane;
  int tab;
  char *page;
} I7PanelHistory;

extern const char * const i7_panel_index_names[];

GType i7_panel_get_type() G_GNUC_CONST;
GtkWidget *i7_panel_new();
void i7_panel_reset_queue(I7Panel *self, I7PanelPane pane, int tab, const char *page_uri);
void i7_panel_goto_docpage(I7Panel *self, GFile *file);
void i7_panel_goto_doc_uri(I7Panel *self, const char *uri);
void i7_panel_goto_docpage_at_anchor(I7Panel *self, GFile *file, const char *anchor);
void i7_panel_goto_extensions_docpage(I7Panel *self, GFile *file);
void i7_panel_update_tabs(I7Panel *self);
void i7_panel_update_fonts(I7Panel *self);
void i7_panel_update_font_sizes(I7Panel *self);
void i7_panel_push_history_item(I7Panel *self, I7PanelHistory *item);
I7PanelHistory *i7_panel_get_current_history_item(I7Panel *self);

#endif /* _PANEL_H_ */
