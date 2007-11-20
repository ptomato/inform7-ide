/*  Copyright 2006 P.F. Chimento
 *  This file is part of GNOME Inform 7.
 * 
 *  GNOME Inform 7 is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  GNOME Inform 7 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNOME Inform 7; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#ifndef APP_WINDOW_H
#define APP_WINDOW_H

#include <gnome.h>

#if !GTK_CHECK_VERSION(2,10,0)
# define SUCKY_GNOME 1
#endif

/* The names of the tabs in each notebook */
enum {
    TAB_FIRST = 0,
    TAB_SOURCE = TAB_FIRST,
    TAB_ERRORS,
    TAB_INDEX,
    TAB_SKEIN,
    TAB_TRANSCRIPT,
    TAB_GAME,
    TAB_DOCUMENTATION,
    TAB_SETTINGS,
    TAB_LAST = TAB_SETTINGS
};

/* The names of the sub tabs in "Errors" */
enum {
    TAB_ERRORS_FIRST = 0,
    TAB_ERRORS_PROGRESS = TAB_ERRORS_FIRST,
    TAB_ERRORS_PROBLEMS,
    TAB_ERRORS_LAST = TAB_ERRORS_PROBLEMS,
    TAB_ERRORS_DEBUGGING,
    TAB_ERRORS_INFORM6      /* These last two are optional */
};

/* The names of the sub tabs in "Index" */
enum {
    TAB_INDEX_FIRST = 0,
    TAB_INDEX_ACTIONS = TAB_INDEX_FIRST,
    TAB_INDEX_CONTENTS,
    TAB_INDEX_KINDS,
    TAB_INDEX_PHRASEBOOK,
    TAB_INDEX_RULES,
    TAB_INDEX_SCENES,
    TAB_INDEX_WORLD,
    TAB_INDEX_LAST = TAB_INDEX_WORLD
};

/* For choose_notebook and suchlike */
#ifndef LEFT
# define LEFT 0
#endif
#ifndef RIGHT
# define RIGHT 1
#endif

GtkWidget *get_focused_widget(GtkWidget *thiswidget);
GtkNotebook *get_notebook(GtkWidget *thiswidget, int right);
GtkNotebook *get_current_notebook(GtkWidget *thiswidget);
int get_current_notebook_side(GtkWidget *thiswidget);
int choose_notebook(GtkWidget *thiswidget, int newtab);
void display_status_message(GtkWidget *thiswidget, const gchar *message);
void display_status_busy(GtkWidget *thiswidget);
void display_status_percentage(GtkWidget *thiswidget, gdouble fraction);
void clear_status(GtkWidget *thiswidget);
GtkWidget *create_open_recent_submenu();
GtkWidget *create_open_extension_submenu();

void
after_app_window_realize               (GtkWidget       *widget,
                                        gpointer         user_data);

gboolean
on_app_window_focus_in_event           (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data);

void
on_back_l_clicked                      (GtkToolButton   *toolbutton,
                                        gpointer         user_data);

void
on_forward_l_clicked                   (GtkToolButton   *toolbutton,
                                        gpointer         user_data);

void
on_back_r_clicked                      (GtkToolButton   *toolbutton,
                                        gpointer         user_data);

void
on_forward_r_clicked                   (GtkToolButton   *toolbutton,
                                        gpointer         user_data);

void
after_notebook_l_switch_page           (GtkNotebook     *notebook,
                                        GtkNotebookPage *page,
                                        guint            page_num,
                                        gpointer         user_data);

void
after_errors_notebook_l_switch_page    (GtkNotebook     *notebook,
                                        GtkNotebookPage *page,
                                        guint            page_num,
                                        gpointer         user_data);

void
after_index_notebook_l_switch_page     (GtkNotebook     *notebook,
                                        GtkNotebookPage *page,
                                        guint            page_num,
                                        gpointer         user_data);

void
after_notebook_r_switch_page           (GtkNotebook     *notebook,
                                        GtkNotebookPage *page,
                                        guint            page_num,
                                        gpointer         user_data);

void
after_errors_notebook_r_switch_page    (GtkNotebook     *notebook,
                                        GtkNotebookPage *page,
                                        guint            page_num,
                                        gpointer         user_data);

void
after_index_notebook_r_switch_page     (GtkNotebook     *notebook,
                                        GtkNotebookPage *page,
                                        guint            page_num,
                                        gpointer         user_data);

gboolean
on_app_window_delete_event             (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_app_window_destroy                  (GtkObject       *object,
                                        gpointer         user_data);

#endif
