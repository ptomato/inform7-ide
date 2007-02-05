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
enum { LEFT = 0, RIGHT };

GtkNotebook *get_notebook(GtkWidget *thiswidget, int right);
int get_current_notebook(GtkWidget *thiswidget);
int choose_notebook(GtkWidget *thiswidget, int newtab);
gchar *get_datafile_path(const gchar *filename);
gboolean check_datafile(const gchar *filename);
void display_status_message(GtkWidget *thiswidget, const gchar *message);
void display_status_busy(GtkWidget *thiswidget);

GtkWidget *gtk_container_get_focus_child   (GtkContainer     *container);

void
after_app_window_realize               (GtkWidget       *widget,
                                        gpointer         user_data);

gboolean
on_app_window_focus_in_event           (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data);

void
on_new_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_open_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_open_recent_activate                (GtkRecentChooser *chooser,
                                        gpointer         user_data);

void
on_install_extension_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_open_extension_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_close_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_save_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_save_as_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_revert_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_quit_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_undo_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_redo_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_cut_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_copy_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_paste_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_select_all_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_find_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_preferences_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_refresh_index_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_go_activate                         (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_replay_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_stop_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_release_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_show_inspectors_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_show_source_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_show_errors_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_show_index_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_show_skein_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_show_transcript_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_show_game_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_show_documentation_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_show_settings_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_switch_sides_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_show_actions_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_show_contents_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_show_kinds_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_show_phrasebook_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_show_rules_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_show_scenes_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_show_world_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_next_sub_panel_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_inform_help_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_gnome_notes_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_license_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_help_extensions_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_recipe_book_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_about_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_go_toolbutton_clicked               (GtkToolButton   *toolbutton,
                                        gpointer         user_data);

void
on_replay_toolbutton_clicked           (GtkToolButton   *toolbutton,
                                        gpointer         user_data);

void
on_stop_toolbutton_clicked             (GtkToolButton   *toolbutton,
                                        gpointer         user_data);

void
on_release_toolbutton_clicked          (GtkToolButton   *toolbutton,
                                        gpointer         user_data);

void
on_source_search_activate              (GtkEntry        *entry,
                                        gpointer         user_data);

void
on_docs_search_activate                (GtkEntry        *entry,
                                        gpointer         user_data);

gboolean
on_source_search_focus                 (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data);

gboolean
on_docs_search_focus                   (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data);
void
on_help_toolbutton_clicked             (GtkToolButton   *toolbutton,
                                        gpointer         user_data);

gboolean
on_app_window_delete_event             (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_app_window_destroy                  (GtkObject       *object,
                                        gpointer         user_data);

#endif
