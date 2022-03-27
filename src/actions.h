/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2008-2010, 2019 Philip Chimento <philip.chimento@gmail.com>
 */

#ifndef _ACTIONS_H_
#define _ACTIONS_H_

#include "config.h"

#include <glib.h>
#include <gtk/gtk.h>

#include "story.h"

void action_new(GSimpleAction *action, GVariant *parameter, I7App *app);
void action_open(GSimpleAction *action, GVariant *parameter, I7App *app);
void action_open_recent(GSimpleAction *action, GVariant *parameter, I7App *app);
void action_install_extension(GSimpleAction *action, GVariant *parameter, I7App *app);
void action_open_extension(GSimpleAction *action, GVariant *parameter, I7App *app);
void action_import_into_skein(GSimpleAction *action, GVariant *parameter, I7Story *story);
void action_save(GSimpleAction *action, GVariant *parameter, I7Document *document);
void action_save_as(GSimpleAction *action, GVariant *parameter, I7Document *document);
void action_save_copy(GSimpleAction *action, GVariant *parameter, I7Document *document);
void action_revert(GSimpleAction *action, GVariant *parameter, I7Document *document);
void action_page_setup(GSimpleAction *action, GVariant *parameter, I7Document *document);
void action_print(GSimpleAction *action, GVariant *parameter, I7Document *document);
void action_close(GSimpleAction *action, GVariant *parameter, I7Document *document);
void action_quit(GSimpleAction *action, GVariant *parameter, I7App *app);
void action_undo(GSimpleAction *action, GVariant *parameter, I7Document *document);
void action_redo(GSimpleAction *action, GVariant *parameter, I7Document *document);
void action_cut(GSimpleAction *action, GVariant *parameter, I7Document *document);
void action_copy(GSimpleAction *action, GVariant *parameter, I7Document *document);
void action_paste(GSimpleAction *action, GVariant *parameter, I7Document *document);
void action_select_all(GSimpleAction *action, GVariant *parameter, I7Document *document);
void action_find(GSimpleAction *action, GVariant *parameter, I7Document *document);
void action_find_next(GSimpleAction *action, GVariant *parameter, I7Document *document);
void action_find_previous(GSimpleAction *action, GVariant *parameter, I7Document *document);
void action_replace(GSimpleAction *action, GVariant *parameter, I7Document *document);
void action_scroll_selection(GSimpleAction *action, GVariant *parameter, I7Document *document);
void action_search(GSimpleAction *action, GVariant *parameter, I7Document *document);
void action_check_spelling(GSimpleAction *action, GVariant *parameter, I7Document *document);
void action_autocheck_spelling_toggle(GSimpleAction *action, GVariant *state, I7Document *document);
void action_preferences(GSimpleAction *action, GVariant *parameter, I7App *app);
void action_view_toolbar_toggled(GSimpleAction *action, GVariant *state, I7Document *document);
void action_view_statusbar_toggled(GSimpleAction *action, GVariant *state, I7Document *document);
void action_view_notepad_toggled(GSimpleAction *action, GVariant *state, I7Story *story);
void action_show_pane(GSimpleAction *action, GVariant *parameter, I7Story *story);
void action_show_tab(GSimpleAction *action, GVariant *parameter, I7Story *story);
void action_show_headings(GSimpleAction *action, GVariant *parameter, I7Story *story);
void action_current_section_only(GSimpleAction *action, GVariant *parameter, I7Document *document);
void action_increase_restriction(GSimpleAction *action, GVariant *parameter, I7Document *document);
void action_decrease_restriction(GSimpleAction *action, GVariant *parameter, I7Document *document);
void action_entire_source(GSimpleAction *action, GVariant *parameter, I7Document *document);
void action_previous_section(GSimpleAction *action, GVariant *parameter, I7Document *document);
void action_next_section(GSimpleAction *action, GVariant *parameter, I7Document *document);
void action_indent(GSimpleAction *action, GVariant *parameter, I7Document *document);
void action_unindent(GSimpleAction *action, GVariant *parameter, I7Document *document);
void action_comment_out_selection(GSimpleAction *action, GVariant *parameter, I7Document *document);
void action_uncomment_selection(GSimpleAction *action, GVariant *parameter, I7Document *document);
void action_renumber_all_sections(GSimpleAction *action, GVariant *parameter, I7Document *document);
void action_enable_elastic_tabstops_toggled(GSimpleAction *action, GVariant *state, I7Document *document);
void action_go(GSimpleAction *action, GVariant *parameter, I7Story *story);
void action_test_me(GSimpleAction *action, GVariant *parameter, I7Story *story);
void action_stop(GSimpleAction *action, GVariant *parameter, I7Story *story);
void action_refresh_index(GSimpleAction *action, GVariant *parameter, I7Story *story);
void action_replay(GSimpleAction *action, GVariant *parameter, I7Story *story);
void action_play_all_blessed(GSimpleAction *action, GVariant *parameter, I7Story *story);
void action_show_last_command(GSimpleAction *action, GVariant *parameter, I7Story *story);
void action_show_last_command_skein(GSimpleAction *action, GVariant *parameter, I7Story *story);
void action_previous_changed_command(GSimpleAction *action, GVariant *parameter, I7Story *story);
void action_next_changed_command(GSimpleAction *action, GVariant *parameter, I7Story *story);
void action_previous_difference(GSimpleAction *action, GVariant *parameter, I7Story *story);
void action_next_difference(GSimpleAction *action, GVariant *parameter, I7Story *story);
void action_next_difference_skein(GSimpleAction *action, GVariant *parameter, I7Story *story);
void action_release(GSimpleAction *action, GVariant *parameter, I7Story *story);
void action_save_debug_build(GSimpleAction *action, GVariant *parameter, I7Story *story);
void action_open_materials_folder(GSimpleAction *action, GVariant *parameter, I7Story *story);
void action_export_ifiction_record(GSimpleAction *action, GVariant *parameter, I7Story *story);
void action_help_contents(GSimpleAction *action, GVariant *parameter, I7Story *story);
void action_help_license(GSimpleAction *action, GVariant *parameter, I7Story *story);
void action_help_extensions(GSimpleAction *action, GVariant *parameter, I7Story *story);
void action_help_recipe_book(GSimpleAction *action, GVariant *parameter, I7Story *story);
void action_visit_inform7_com(GSimpleAction *action, GVariant *parameter, I7App *app);
void action_about(GSimpleAction *action, GVariant *parameter, I7App *app);

#endif /* _ACTIONS_H_ */
