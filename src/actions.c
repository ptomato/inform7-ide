/* Copyright (C) 2008–2015, 2019 P. F. Chimento
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

/* All the callbacks for the "activate" signal of the GtkActions from the main
 menu and toolbar of document windows. */

#include "config.h"

#include <errno.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>
#include <webkit2/webkit2.h>

#include "actions.h"
#include "app.h"
#include "builder.h"
#include "document.h"
#include "configfile.h"
#include "error.h"
#include "extension.h"
#include "file.h"
#include "newdialog.h"
#include "panel.h"
#include "prefs.h"
#include "story.h"

/* Helper function to get the toplevel window that contains the action's proxy,
 in case it wasn't passed as a user data parameter to the action callback. */
static GtkWindow *
get_toplevel_for_action(GtkAction *action)
{
	GSList *list;
	GtkWidget *parent = NULL;
	for(list = gtk_action_get_proxies(action) ; list; list = g_slist_next(list)) {
		GtkWidget *toplevel = gtk_widget_get_toplevel((GtkWidget *)list->data);
		if(GTK_IS_MENU(toplevel))
			toplevel = gtk_widget_get_toplevel(gtk_menu_get_attach_widget(GTK_MENU(toplevel)));
		if(toplevel && gtk_widget_is_toplevel(toplevel)) {
			parent = toplevel;
			break;
		}
	}

	return GTK_WINDOW(parent);
}

/* File->New... */
void
action_new(GtkAction *action, I7App *app)
{
	GtkWidget *newdialog = create_new_dialog();
	gtk_widget_show(newdialog);
}

/* File->Open... */
void
action_open(GtkAction *action, I7App *app)
{
	i7_story_new_from_dialog(app);
}

/* Callback for when of the items from the File->Open Recent submenu
 is selected */
void
action_open_recent(GtkAction *action, I7App *app)
{
	GtkRecentInfo *item = gtk_recent_chooser_get_current_item(GTK_RECENT_CHOOSER(action));
	g_assert(gtk_recent_info_has_application(item, "Inform 7"));

	GFile *file = g_file_new_for_uri(gtk_recent_info_get_uri(item));

	if(gtk_recent_info_has_group(item, "inform7_project")) {
		i7_story_new_from_file(app, file);
	} else if(gtk_recent_info_has_group(item, "inform7_extension")) {
		i7_extension_new_from_file(app, file, FALSE);
	} else if(gtk_recent_info_has_group(item, "inform7_builtin")) {
		i7_extension_new_from_file(app, file, TRUE);
	} else {
		g_warning("Recent manager file does not have an Inform tag. This means "
			"it was not saved by Inform. I'll try to open it anyway.");
		i7_story_new_from_file(app, file);
	}

	g_object_unref(file);
	gtk_recent_info_unref(item);
}

/* File->Install Extension... */
void
action_install_extension(GtkAction *action, I7App *app)
{
	/* Select the Extensions tab */
	gtk_notebook_set_current_page(GTK_NOTEBOOK(app->prefs->prefs_notebook), I7_PREFS_EXTENSIONS);

	/* Show the preferences dialog */
	i7_app_present_prefs_window(app);

	/* Pretend the user clicked the Add button */
	g_signal_emit_by_name(app->prefs->extensions_add, "clicked");
}

/* Callback for when one of the items from the File->Open Extension submenu
 is selected, and it is a built-in extension */
void
on_open_extension_readonly_activate(GtkMenuItem *menuitem, GFile *file)
{
	i7_extension_new_from_file(i7_app_get(), file, TRUE);
}

/* Callback for when one of the items from the File->Open Extension submenu
 is selected, and it is a user-installed extension */
void
on_open_extension_activate(GtkMenuItem *menuitem, GFile *file)
{
	i7_extension_new_from_file(i7_app_get(), file, FALSE);
}

/* File->Import Into Skein... */
void
action_import_into_skein(GtkAction *action, I7Story *story)
{
	GError *err = NULL;

	/* Ask the user for a file to import */
	/* TRANSLATORS: File->Import Into Skein... */
	GtkWidget *dialog = gtk_file_chooser_dialog_new(
	  _("Select the file to import into the skein"),
	  GTK_WINDOW(story),
	  GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
	  GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	/* Create appropriate file filters */
	GtkFileFilter *filter1 = gtk_file_filter_new();
	gtk_file_filter_set_name(filter1, _("Interpreter recording files (*.rec)"));
	gtk_file_filter_add_pattern(filter1, "*.rec");
	GtkFileFilter *filter2 = gtk_file_filter_new();
	gtk_file_filter_set_name(filter2, _("All Files"));
	gtk_file_filter_add_pattern(filter2, "*");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter1);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter2);

	if(gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_ACCEPT) {
		gtk_widget_destroy(dialog);
		return;
	}

	GFile *file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));
	gtk_widget_destroy(dialog);
	if(!file)
		return; /* Fail silently */

	/* Provide some visual feedback that the command did something */
	if(!i7_skein_import(i7_story_get_skein(story), file, &err))
		error_dialog_file_operation(GTK_WINDOW(story), file, err, I7_FILE_ERROR_OPEN, NULL);
	else
		i7_story_show_pane(story, I7_PANE_SKEIN);

	g_object_unref(file);
}

/* File->Save */
void
action_save(GtkAction *action, I7Document *document)
{
	i7_document_save(document);
}

/* File->Save As... */
void
action_save_as(GtkAction *action, I7Document *document)
{
	GFile *file = i7_document_run_save_dialog(document, NULL);
	if(file) {
		i7_document_set_file(document, file);
		/* This hack is convenient so that if you save a built-in (read-only)
		extension to another file name, it's not read-only anymore */
		if(I7_IS_EXTENSION(document))
			i7_extension_set_read_only(I7_EXTENSION(document), FALSE);
		i7_document_save_as(document, file);
		g_object_unref(file);
	}
}

/* File->Save a Copy */
void
action_save_copy(GtkAction *action, I7Document *document)
{
	GFile *file = i7_document_run_save_dialog(document, NULL);
	if(file) {
		i7_document_save_as(document, file);
		g_object_unref(file);
	}
}

/* File->Revert */
void
action_revert(GtkAction *action, I7Document *document)
{
	if (!i7_document_can_revert(document))
		return;

	/* Ask if the user is sure */
	/* TRANSLATORS: File->Revert */
	GtkWidget *revert_dialog = gtk_message_dialog_new(GTK_WINDOW(document), GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_WARNING, GTK_BUTTONS_NONE,
		_("Are you sure you want to revert to the last saved version?"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(revert_dialog),
		_("All unsaved changes will be lost."));
	gtk_dialog_add_buttons(GTK_DIALOG(revert_dialog),
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_REVERT_TO_SAVED, GTK_RESPONSE_OK,
		NULL);
	gint result = gtk_dialog_run(GTK_DIALOG(revert_dialog));
	gtk_widget_destroy(revert_dialog);
	if(result != GTK_RESPONSE_OK)
		return; /* Only go on if the user clicked revert */

	i7_document_revert(document);
}

/* File->Page Setup... */
void
action_page_setup(GtkAction *action, I7Document *document)
{
	I7App *theapp = i7_app_get();
	GtkPrintSettings *settings = i7_app_get_print_settings(theapp);

	if(!settings)
		settings = gtk_print_settings_new();

	GtkPageSetup *new_page_setup = gtk_print_run_page_setup_dialog(GTK_WINDOW(document), i7_app_get_page_setup(theapp), settings);

	i7_app_set_page_setup(theapp, new_page_setup);
}

/* Callback for drawing a page to the output when requested. Just use the
 standard GtkSourcePrintCompositor way of doing it, no customizations. */
static void
on_draw_page(GtkPrintOperation *print, GtkPrintContext *context, gint page_nr, GtkSourcePrintCompositor *compositor)
{
	gtk_source_print_compositor_draw_page(compositor, context, page_nr);
}

/* Callback for ending print operation, no customizations here either */
static void
on_end_print(GtkPrintOperation *print, GtkPrintContext *context, GtkSourcePrintCompositor *compositor)
{
	g_object_unref(compositor);
}

/* Callback for beginning the print operation, we give the printed pages our
 tab width from the preferences, and the margins from the page setup dialog. */
static void
on_begin_print(GtkPrintOperation *print, GtkPrintContext *context,
	I7Document *document)
{
	I7App *theapp = i7_app_get();
	GSettings *prefs = i7_app_get_prefs(theapp);
	GtkSourcePrintCompositor *compositor = gtk_source_print_compositor_new(i7_document_get_buffer(document));
	g_signal_connect(print, "draw-page", G_CALLBACK(on_draw_page), compositor);
	g_signal_connect(print, "end-print", G_CALLBACK(on_end_print), compositor);

	/* Design our printed page */
	unsigned tabwidth = g_settings_get_uint(prefs, PREFS_TAB_WIDTH);
	if(tabwidth == 0)
		tabwidth = DEFAULT_TAB_WIDTH;
	gtk_source_print_compositor_set_tab_width(compositor, tabwidth);
	gtk_source_print_compositor_set_wrap_mode(compositor, GTK_WRAP_WORD_CHAR);
	PangoFontDescription *font = get_font_description();
	gchar *fontstring = pango_font_description_to_string(font);
	pango_font_description_free(font);
	gtk_source_print_compositor_set_body_font_name(compositor, fontstring);
	g_free(fontstring);
	GtkPageSetup *setup = i7_app_get_page_setup(i7_app_get());
	gtk_source_print_compositor_set_top_margin(compositor, gtk_page_setup_get_top_margin(setup, GTK_UNIT_MM), GTK_UNIT_MM);
	gtk_source_print_compositor_set_bottom_margin(compositor, gtk_page_setup_get_bottom_margin(setup, GTK_UNIT_MM), GTK_UNIT_MM);
	gtk_source_print_compositor_set_left_margin(compositor, gtk_page_setup_get_left_margin(setup, GTK_UNIT_MM), GTK_UNIT_MM);
	gtk_source_print_compositor_set_right_margin(compositor, gtk_page_setup_get_right_margin(setup, GTK_UNIT_MM), GTK_UNIT_MM);

	/* Display a notification in the status bar while paginating */
	i7_document_display_status_message(document, _("Paginating..."), PRINT_OPERATIONS);
	while(!gtk_source_print_compositor_paginate(compositor, context)) {
		i7_document_display_progress_percentage(document, gtk_source_print_compositor_get_pagination_progress(compositor));
		while(gtk_events_pending())
			gtk_main_iteration();
	}
	i7_document_display_progress_percentage(document, 0.0);
	i7_document_remove_status_message(document, PRINT_OPERATIONS);

	gtk_print_operation_set_n_pages(print, gtk_source_print_compositor_get_n_pages(compositor));
}

/* File->Print Preview... */
void
action_print_preview(GtkAction *action, I7Document *document)
{
	GError *error = NULL;
	I7App *theapp = i7_app_get();
	GtkPrintOperation *print = gtk_print_operation_new();
	GtkPrintSettings *settings = i7_app_get_print_settings(theapp);

	if(settings)
		gtk_print_operation_set_print_settings(print, settings);

	g_signal_connect(print, "begin-print", G_CALLBACK(on_begin_print), document);

	GtkPrintOperationResult result = gtk_print_operation_run(print, GTK_PRINT_OPERATION_ACTION_PREVIEW, GTK_WINDOW(document), &error);
	if(result == GTK_PRINT_OPERATION_RESULT_APPLY)
		i7_app_set_print_settings(theapp, g_object_ref(gtk_print_operation_get_print_settings(print)));
	else if(result == GTK_PRINT_OPERATION_RESULT_ERROR)
		/* TRANSLATORS: File->Print Preview... */
		error_dialog(GTK_WINDOW(document), error, _("There was an error printing: "));
	g_object_unref(print);
}

/* File->Print... */
void
action_print(GtkAction *action, I7Document *document)
{
	GError *error = NULL;
	I7App *theapp = i7_app_get();
	GtkPrintOperation *print = gtk_print_operation_new();
	GtkPrintSettings *settings = i7_app_get_print_settings(theapp);

	if(settings)
		gtk_print_operation_set_print_settings(print, settings);

	g_signal_connect(print, "begin-print", G_CALLBACK(on_begin_print), document);

	GtkPrintOperationResult result = gtk_print_operation_run(print, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, GTK_WINDOW(document), &error);
	if(result == GTK_PRINT_OPERATION_RESULT_APPLY)
		i7_app_set_print_settings(theapp, g_object_ref(gtk_print_operation_get_print_settings(print)));
	else if(result == GTK_PRINT_OPERATION_RESULT_ERROR) /* TRANSLATORS: File->Print... */
		error_dialog(GTK_WINDOW(document), error, _("There was an error printing: "));
	g_object_unref(print);
}

/* File->Close */
void
action_close(GtkAction *action, I7Document *document)
{
	if(i7_document_verify_save(document)) {
		i7_app_remove_document(i7_app_get(), I7_DOCUMENT(document));
		gtk_widget_destroy(GTK_WIDGET(document));
	}
}

/* File->Quit */
void
action_quit(GtkAction *action, I7App *app)
{
	i7_app_close_all_documents(app);
}

/* Edit->Undo */
void
action_undo(GtkAction *action, I7Document *document)
{
	GtkSourceBuffer *buffer = i7_document_get_buffer(document);
	if(gtk_source_buffer_can_undo(buffer))
		gtk_source_buffer_undo(buffer);

	/* Update the "sensitive" state of the undo and redo actions */
	gtk_action_set_sensitive(action, gtk_source_buffer_can_undo(buffer));
	gtk_action_set_sensitive(document->redo, gtk_source_buffer_can_redo(buffer));
}

/* Edit->Redo */
void
action_redo(GtkAction *action, I7Document *document)
{
	GtkSourceBuffer *buffer = i7_document_get_buffer(document);
	if(gtk_source_buffer_can_redo(buffer))
		gtk_source_buffer_redo(buffer);

	/* Update the "sensitive" state of the undo and redo actions */
	gtk_action_set_sensitive(action, gtk_source_buffer_can_redo(buffer));
	gtk_action_set_sensitive(document->undo, gtk_source_buffer_can_undo(buffer));
}

/* Edit->Cut */
void
action_cut(GtkAction *action, I7Document *document)
{
	GtkWidget *widget = gtk_window_get_focus(GTK_WINDOW(document));

	/* What actually happens depends on the type of widget that is focused */
	if(WEBKIT_IS_WEB_VIEW(widget))
		webkit_web_view_execute_editing_command(WEBKIT_WEB_VIEW(widget), WEBKIT_EDITING_COMMAND_CUT);
	else if(GTK_IS_LABEL(widget) && gtk_label_get_selectable(GTK_LABEL(widget)))
		g_signal_emit_by_name(widget, "copy-clipboard", NULL);  /* just copy */
	else if(GTK_IS_ENTRY(widget) || GTK_IS_TEXT_VIEW(widget))
		g_signal_emit_by_name(widget, "cut-clipboard", NULL);
	else /* If we don't know how to cut from it, just cut from the source */
		g_signal_emit_by_name(i7_document_get_default_view(document), "cut-clipboard", NULL);
}

/* Edit->Copy */
void
action_copy(GtkAction *action, I7Document *document)
{
	GtkWidget *widget = gtk_window_get_focus(GTK_WINDOW(document));

	/* What actually happens depends on the type of widget that is focused */
	if(WEBKIT_IS_WEB_VIEW(widget))
		webkit_web_view_execute_editing_command(WEBKIT_WEB_VIEW(widget), WEBKIT_EDITING_COMMAND_COPY);
	else if((GTK_IS_LABEL(widget) && gtk_label_get_selectable(GTK_LABEL(widget)))
		|| GTK_IS_ENTRY(widget) || GTK_IS_TEXT_VIEW(widget))
		g_signal_emit_by_name(widget, "copy-clipboard", NULL);
	else /* If we don't know how to copy from it, just copy from the source */
		g_signal_emit_by_name(i7_document_get_default_view(document), "copy-clipboard", NULL);
}

/* Edit->Paste */
void
action_paste(GtkAction *action, I7Document *document)
{
	GtkWidget *widget = gtk_window_get_focus(GTK_WINDOW(document));

	/* What actually happens depends on the type of widget that is focused */
	if(GTK_IS_ENTRY(widget) || GTK_IS_TEXT_VIEW(widget))
		g_signal_emit_by_name(widget, "paste-clipboard", NULL);
	else /* If we don't know how to paste to it, just paste to the source */
		g_signal_emit_by_name(i7_document_get_default_view(document), "paste-clipboard", NULL);
}

/* Edit->Select All */
void
action_select_all(GtkAction *action, I7Document *document)
{
	GtkWidget *widget = gtk_window_get_focus(GTK_WINDOW(document));

	/* What actually happens depends on the type of widget that is focused */
	if(WEBKIT_IS_WEB_VIEW(widget))
		webkit_web_view_execute_editing_command(WEBKIT_WEB_VIEW(widget), WEBKIT_EDITING_COMMAND_SELECT_ALL);
	else if(GTK_IS_LABEL(widget) && gtk_label_get_selectable(GTK_LABEL(widget)))
		gtk_label_select_region(GTK_LABEL(widget), 0, -1);
	else if(GTK_IS_EDITABLE(widget))
		gtk_editable_select_region(GTK_EDITABLE(widget), 0, -1);
	else if(GTK_IS_TEXT_VIEW(widget))
		g_signal_emit_by_name(widget, "select-all", TRUE, NULL);
	else /* If we don't know how to select it, just select all in the source */
		g_signal_emit_by_name(i7_document_get_default_view(document), "select-all", TRUE, NULL);
}

/* Edit->Find - Unhide the find bar at the bottom */
void
action_find(GtkAction *action, I7Document *document)
{
	gtk_widget_show(document->findbar);
	const gchar *text = gtk_entry_get_text(GTK_ENTRY(document->findbar_entry));
	/* don't free */
	i7_document_set_quicksearch_not_found(document, !i7_document_highlight_quicksearch(document, text, TRUE));
	gtk_widget_grab_focus(document->findbar_entry);
}

/* Edit->Find Next */
void
action_find_next(GtkAction *action, I7Document *document)
{
	const gchar *text = gtk_entry_get_text(GTK_ENTRY(document->findbar_entry));
	i7_document_set_quicksearch_not_found(document, !i7_document_highlight_quicksearch(document, text, TRUE));
}

/* Edit->Find Previous */
void
action_find_previous(GtkAction *action, I7Document *document)
{
	const gchar *text = gtk_entry_get_text(GTK_ENTRY(document->findbar_entry));
	i7_document_set_quicksearch_not_found(document, !i7_document_highlight_quicksearch(document, text, FALSE));
}

/* Edit->Find and Replace... - For more complicated find operations, we use a
 dialog instead of the find bar */
void
action_replace(GtkAction *action, I7Document *document)
{
	gtk_widget_show(document->find_dialog);
	gtk_window_present(GTK_WINDOW(document->find_dialog));
}

/* Edit->Scroll to Selection */
void
action_scroll_selection(GtkAction *action, I7Document *document)
{
	i7_document_scroll_to_selection(document);
}

/* Edit->Search Files... - This is another dialog that searches any combination
 of the story, the installed extensions, and the documentation. */
void
action_search(GtkAction *action, I7Document *document)
{
	gtk_widget_show(document->search_files_dialog);
	gtk_window_present(GTK_WINDOW(document->search_files_dialog));
}

/* Edit->Recheck Document */
void
action_check_spelling(GtkAction *action, I7Document *document)
{
	i7_document_check_spelling(document);
}

/* Edit->Autocheck Spelling */
void
action_autocheck_spelling_toggle(GtkToggleAction *action, I7Document *document)
{
	gboolean value = gtk_toggle_action_get_active(action);
	gtk_action_set_sensitive(document->check_spelling, value);
	i7_document_set_spellcheck(document, value);
}

/* Edit->Preferences... */
void
action_preferences(GtkAction *action, I7App *app)
{
	i7_app_present_prefs_window(app);
}

/* View->Toolbar */
void
action_view_toolbar_toggled(GtkToggleAction *action, I7Document *document)
{
	gboolean show = gtk_toggle_action_get_active(action);

	/* Set the default value for the next time a window is opened */
	GSettings *state = i7_app_get_state(i7_app_get());
	g_settings_set_boolean(state, PREFS_STATE_SHOW_TOOLBAR, show);

	if(show)
		gtk_widget_show(document->toolbar);
	else
		gtk_widget_hide(document->toolbar);
}

/* View->Statusbar */
void
action_view_statusbar_toggled(GtkToggleAction *action, I7Document *document)
{
	gboolean show = gtk_toggle_action_get_active(action);

	/* Set the default value for the next time a window is opened */
	GSettings *state = i7_app_get_state(i7_app_get());
	g_settings_set_boolean(state, PREFS_STATE_SHOW_STATUSBAR, show);

	if(show)
		gtk_widget_show(document->statusline);
	else
		gtk_widget_hide(document->statusline);
}

/* View->Notepad */
void
action_view_notepad_toggled(GtkToggleAction *action, I7Story *story)
{
	gboolean show = gtk_toggle_action_get_active(action);

	/* Set the default value for the next time a window is opened */
	GSettings *state = i7_app_get_state(i7_app_get());
	g_settings_set_boolean(state, PREFS_STATE_SHOW_NOTEPAD, show);

	if(show)
		gtk_widget_show(story->notes_window);
	else
		gtk_widget_hide(story->notes_window);
}

/* View->Show Tab->Source */
void
action_show_source(GtkAction *action, I7Story *story)
{
	i7_story_show_tab(story, I7_PANE_SOURCE, I7_SOURCE_VIEW_TAB_SOURCE);
}

/* View->Show Tab->Errors */
void
action_show_results(GtkAction *action, I7Story *story)
{
	i7_story_show_pane(story, I7_PANE_RESULTS);
}

/* View->Show Tab->Index */
void
action_show_index(GtkAction *action, I7Story *story)
{
	i7_story_show_pane(story, I7_PANE_INDEX);
}

/* View->Show Tab->Skein */
void
action_show_skein(GtkAction *action, I7Story *story)
{
	i7_story_show_pane(story, I7_PANE_SKEIN);
}

/* View->Show Tab->Transcript */
void
action_show_transcript(GtkAction *action, I7Story *story)
{
	i7_story_show_pane(story, I7_PANE_TRANSCRIPT);
}

/* View->Show Tab->Story */
void
action_show_story(GtkAction *action, I7Story *story)
{
	i7_story_show_pane(story, I7_PANE_STORY);
}

/* View->Show Tab->Documentation */
void
action_show_documentation(GtkAction *action, I7Story *story)
{
	i7_story_show_pane(story, I7_PANE_DOCUMENTATION);
}

/* View->Show Tab->Extensions */
void
action_show_extensions(GtkAction *action, I7Story *story)
{
	i7_story_show_pane(story, I7_PANE_EXTENSIONS);
}

/* View->Show Tab->Settings */
void
action_show_settings(GtkAction *action, I7Story *story)
{
	i7_story_show_pane(story, I7_PANE_SETTINGS);
}

/* View->Show Index->Home */
void
action_show_home(GtkAction *action, I7Story *story)
{
	i7_story_show_tab(story, I7_PANE_INDEX, I7_INDEX_TAB_WELCOME);
}

/* View->Show Index->Actions */
void
action_show_actions(GtkAction *action, I7Story *story)
{
	i7_story_show_tab(story, I7_PANE_INDEX, I7_INDEX_TAB_ACTIONS);
}

/* View->Show Index->Contents */
void
action_show_contents(GtkAction *action, I7Story *story)
{
	i7_story_show_tab(story, I7_PANE_INDEX, I7_INDEX_TAB_CONTENTS);
}

/* View->Show Index->Kinds */
void
action_show_kinds(GtkAction *action, I7Story *story)
{
	i7_story_show_tab(story, I7_PANE_INDEX, I7_INDEX_TAB_KINDS);
}

/* View->Show Index->Phrasebook */
void
action_show_phrasebook(GtkAction *action, I7Story *story)
{
	i7_story_show_tab(story, I7_PANE_INDEX, I7_INDEX_TAB_PHRASEBOOK);
}

/* View->Show Index->Rules */
void
action_show_rules(GtkAction *action, I7Story *story)
{
	i7_story_show_tab(story, I7_PANE_INDEX, I7_INDEX_TAB_RULES);
}

/* View->Show Index->Scenes */
void
action_show_scenes(GtkAction *action, I7Story *story)
{
	i7_story_show_tab(story, I7_PANE_INDEX, I7_INDEX_TAB_SCENES);
}

/* View->Show Index->World */
void
action_show_world(GtkAction *action, I7Story *story)
{
	i7_story_show_tab(story, I7_PANE_INDEX, I7_INDEX_TAB_WORLD);
}

/* View->Show Headings */
void
action_show_headings(GtkAction *action, I7Story *story)
{
	i7_story_show_tab(story, I7_PANE_SOURCE, I7_SOURCE_VIEW_TAB_CONTENTS);
}

/* View->Current Section Only */
void
action_current_section_only(GtkAction *action, I7Document *document)
{
	GtkTreePath *path = i7_document_get_deepest_heading(document);
	i7_document_show_heading(document, path);
}

/* View->Increase Restriction - View one level deeper in the headings hierarchy */
void
action_increase_restriction(GtkAction *action, I7Document *document)
{
	GtkTreePath *path = i7_document_get_deeper_heading(document);
	i7_document_show_heading(document, path);
}

/* View->Decrease Restriction - View one level less deep in the headings hierarchy */
void
action_decrease_restriction(GtkAction *action, I7Document *document)
{
	GtkTreePath *path = i7_document_get_shallower_heading(document);
	i7_document_show_heading(document, path);
}

/* View->Entire Source */
void
action_entire_source(GtkAction *action, I7Document *document)
{
	i7_document_show_entire_source(document);
}

/* View->Previous Section */
void
action_previous_section(GtkAction *action, I7Document *document)
{
	GtkTreePath *path = i7_document_get_previous_heading(document);
	i7_document_show_heading(document, path);
}

/* View->Next Section */
void
action_next_section(GtkAction *action, I7Document *document)
{
	GtkTreePath *path = i7_document_get_next_heading(document);
	i7_document_show_heading(document, path);
}

/* Format->Indent */
void
action_indent(GtkAction *action, I7Document *document)
{
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(i7_document_get_buffer(document));
	/* Shift the selected lines in the buffer one tab to the right */
	/* Adapted from gtksourceview.c */
	GtkTextIter start, end;
	gtk_text_buffer_get_selection_bounds(buffer, &start, &end);
	/* Find out which lines to indent */
	gint start_line = gtk_text_iter_get_line(&start);
	gint end_line = gtk_text_iter_get_line(&end);
	gint i;

	/* if the end of the selection is before the first character on a line,
	don't indent it */
	if((gtk_text_iter_get_visible_line_offset(&end) == 0) && (end_line > start_line))
		end_line--;

	/* Treat it as one single undo action */
	gtk_text_buffer_begin_user_action(buffer);
	for(i = start_line; i <= end_line; i++) {
		GtkTextIter iter;
		gtk_text_buffer_get_iter_at_line(buffer, &iter, i);

		/* don't add indentation on empty lines */
		if(gtk_text_iter_ends_line(&iter))
			continue;

		gtk_text_buffer_insert(buffer, &iter, "\t", -1);
	}
	gtk_text_buffer_end_user_action(buffer);
}

/* Format->Unindent */
void
action_unindent(GtkAction *action, I7Document *document)
{
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(i7_document_get_buffer(document));

	/* Shift the selected lines in the buffer one tab to the left */
	/* Adapted from gtksourceview.c */
	GtkTextIter start, end;
	gtk_text_buffer_get_selection_bounds(buffer, &start, &end);
	/* Find out which lines to unindent */
	gint start_line = gtk_text_iter_get_line(&start);
	gint end_line = gtk_text_iter_get_line(&end);
	gint i;

	/* if the end of the selection is before the first character on a line,
	don't unindent it */
	if((gtk_text_iter_get_visible_line_offset(&end) == 0) && (end_line > start_line))
		end_line--;

	/* Treat it as one single undo action */
	gtk_text_buffer_begin_user_action(buffer);
	for(i = start_line; i <= end_line; i++) {
		GtkTextIter iter, iter2;

		gtk_text_buffer_get_iter_at_line(buffer, &iter, i);

		if(gtk_text_iter_get_char(&iter) == '\t') {
			iter2 = iter;
			gtk_text_iter_forward_char(&iter2);
			gtk_text_buffer_delete(buffer, &iter, &iter2);
		}
	}
	gtk_text_buffer_end_user_action(buffer);
}

/* Format->Comment Out Selection */
void
action_comment_out_selection(GtkAction *action, I7Document *document)
{
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(i7_document_get_buffer(document));
	GtkTextIter start, end;

	if(!gtk_text_buffer_get_selection_bounds(buffer, &start, &end))
		return;
	gchar *text = gtk_text_buffer_get_text(buffer, &start, &end, TRUE);

	/* Treat it as one single undo action */
	gtk_text_buffer_begin_user_action(buffer);
	/* Delete the entire text and reinsert it inside brackets, in order to
	 avoid excessively recalculating the syntax highlighting */
	gtk_text_buffer_delete(buffer, &start, &end);
	gchar *newtext = g_strconcat("[", text, "]", NULL);
	GtkTextMark *tempmark = gtk_text_buffer_create_mark(buffer, NULL, &end, TRUE);
	gtk_text_buffer_insert(buffer, &end, newtext, -1);
	gtk_text_buffer_end_user_action(buffer);

	g_free(text);
	g_free(newtext);

	/* Select the text again, including [] */
	gtk_text_buffer_get_iter_at_mark(buffer, &start, tempmark);
	gtk_text_buffer_select_range(buffer, &start, &end);
	gtk_text_buffer_delete_mark(buffer, tempmark);
}

/* GtkTextCharPredicate function */
static gboolean
char_equals(gunichar ch, gpointer data)
{
	return ch == (gunichar)GPOINTER_TO_UINT(data);
}

/* Format->Uncomment Selection */
void
action_uncomment_selection(GtkAction *action, I7Document *document)
{
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(i7_document_get_buffer(document));
	GtkTextIter start, end;

	if(!gtk_text_buffer_get_selection_bounds(buffer, &start, &end))
		return;

	/* Find first [ from the beginning of the selection, then the last ] between
	 there and the end of the selection */
	if(gtk_text_iter_get_char(&start) != '['
		&& !gtk_text_iter_forward_find_char(&start, char_equals, GUINT_TO_POINTER('['), &end))
		return;
	gtk_text_iter_backward_char(&end);
	if(gtk_text_iter_get_char(&end) != ']'
		&& !gtk_text_iter_backward_find_char(&end, char_equals, GUINT_TO_POINTER(']'), &start))
		return;
	gtk_text_iter_forward_char(&end);

	gchar *text = gtk_text_buffer_get_text(buffer, &start, &end, TRUE);

	/* Treat it as one single undo action */
	gtk_text_buffer_begin_user_action(buffer);
	/* Delete the comment and re-insert it without brackets */
	gtk_text_buffer_delete(buffer, &start, &end);
	gchar *newtext = g_strndup(text + 1, strlen(text) - 2);
	GtkTextMark *tempmark = gtk_text_buffer_create_mark(buffer, NULL, &end, TRUE);
	gtk_text_buffer_insert(buffer, &end, newtext, -1);
	gtk_text_buffer_end_user_action(buffer);

	g_free(text);
	g_free(newtext);

	/* Select only the uncommented text again */
	gtk_text_buffer_get_iter_at_mark(buffer, &start, tempmark);
	gtk_text_buffer_select_range(buffer, &start, &end);
	gtk_text_buffer_delete_mark(buffer, tempmark);
}

/* Format->Renumber All Sections */
void
action_renumber_all_sections(GtkAction *action, I7Document *document)
{
	GtkTextIter pos, end;
	int volume = 1, book = 1, part = 1, chapter = 1, section = 1;
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(i7_document_get_buffer(document));

	gtk_text_buffer_get_start_iter(buffer, &pos);

	/* Renumbering sections counts as one action for Undo */
	gtk_text_buffer_begin_user_action(buffer);

	while(gtk_text_iter_get_char(&pos) != 0) {
		if(gtk_text_iter_get_char(&pos) != '\n') {
			gtk_text_iter_forward_line(&pos);
			continue;
		}
		gtk_text_iter_forward_line(&pos);
		end = pos;
		gboolean not_last = gtk_text_iter_forward_line(&end);
		if(!not_last || gtk_text_iter_get_char(&end) == '\n') {
			/* Preceded and followed by a blank line,
			or only preceded by one and current line is last line */
			/* Get the entire line and its line number, chop the \n */
			gchar *text = gtk_text_iter_get_text(&pos, &end);
			gchar *lcase = g_utf8_strdown(text, -1);
			gchar *title = strchr(text, '-');
			if(title && g_str_has_suffix(title, "\n"))
				*(strrchr(title, '\n')) = '\0'; /* remove trailing \n */
			gchar *newtitle;

			if(g_str_has_prefix(lcase, "volume")) {
				newtitle = g_strdup_printf("Volume %d %s\n", volume++, title);
				gtk_text_buffer_delete(buffer, &pos, &end);
				gtk_text_buffer_insert(buffer, &pos, newtitle, -1);
				g_free(newtitle);
				book = part = chapter = section = 1;
			} else if(g_str_has_prefix(lcase, "book")) {
				newtitle = g_strdup_printf("Book %d %s\n", book++, title);
				gtk_text_buffer_delete(buffer, &pos, &end);
				gtk_text_buffer_insert(buffer, &pos, newtitle, -1);
				g_free(newtitle);
				part = chapter = section = 1;
			} else if(g_str_has_prefix(lcase, "part")) {
				newtitle = g_strdup_printf("Part %d %s\n", part++, title);
				gtk_text_buffer_delete(buffer, &pos, &end);
				gtk_text_buffer_insert(buffer, &pos, newtitle, -1);
				g_free(newtitle);
				chapter = section = 1;
			} else if(g_str_has_prefix(lcase, "chapter")) {
				newtitle = g_strdup_printf("Chapter %d %s\n", chapter++, title);
				gtk_text_buffer_delete(buffer, &pos, &end);
				gtk_text_buffer_insert(buffer, &pos, newtitle, -1);
				g_free(newtitle);
				section = 1;
			} else if(g_str_has_prefix(lcase, "section")) {
				newtitle = g_strdup_printf("Section %d %s\n", section++, title);
				gtk_text_buffer_delete(buffer, &pos, &end);
				gtk_text_buffer_insert(buffer, &pos, newtitle, -1);
				g_free(newtitle);
			}
			g_free(text);
			g_free(lcase);
		}
	}

	gtk_text_buffer_end_user_action(buffer);
}

/* Format->Enable Elastic Tabstops */
void
action_enable_elastic_tabstops_toggled(GtkToggleAction *action, I7Document *document)
{
	gboolean value = gtk_toggle_action_get_active(action);
	i7_document_set_elastic_tabstops(document, value);
}

/* Play->Go */
void
action_go(GtkAction *action, I7Story *story)
{
	i7_story_compile(story, FALSE, FALSE, (CompileActionFunc)i7_story_run_compiler_output, NULL);
}

/* Play->Test Me */
void
action_test_me(GtkAction *action, I7Story *story)
{
	i7_story_compile(story, FALSE, FALSE, (CompileActionFunc)i7_story_test_compiler_output, NULL);
}

/* Play->Stop */
void
action_stop(GtkAction *action, I7Story *story)
{
	i7_story_stop_running_game(story);
}

/* Play->Refresh Index */
void
action_refresh_index(GtkAction *action, I7Story *story)
{
	i7_story_compile(story, FALSE, TRUE, (CompileActionFunc)i7_story_show_pane, GUINT_TO_POINTER(I7_PANE_INDEX));
}

/* Replay->Replay Last Commands */
void
action_replay(GtkAction *action, I7Story *story)
{
	i7_story_compile(story, FALSE, FALSE, (CompileActionFunc)i7_story_run_compiler_output_and_replay, NULL);
}

/* Replay->Replay Commands Blessed in Transcript */
void
action_play_all_blessed(GtkAction *action, I7Story *story)
{
	i7_story_compile(story, FALSE, FALSE, (CompileActionFunc)i7_story_run_compiler_output_and_entire_skein, NULL);
}

/* Replay->Show Last Command */
void
action_show_last_command(GtkAction *action, I7Story *story)
{
	I7Skein *skein = i7_story_get_skein(story);
	i7_story_show_node_in_transcript(story, i7_skein_get_played_node(skein));
	i7_story_show_pane(story, I7_PANE_TRANSCRIPT);
}

/* Replay->Show Last Command in Skein */
void
action_show_last_command_skein(GtkAction *action, I7Story *story)
{
	I7Skein *skein = i7_story_get_skein(story);
	g_signal_emit_by_name(skein, "show-node", I7_REASON_USER_ACTION, i7_skein_get_played_node(skein));
	i7_story_show_pane(story, I7_PANE_SKEIN);
}

/* Replay->Find Previous Changed Command */
void
action_previous_changed_command(GtkAction *action, I7Story *story)
{
	i7_story_previous_changed(story);
}

/* Replay->Find Next Changed Command */
void
action_next_changed_command(GtkAction *action, I7Story *story)
{
	i7_story_next_changed(story);
}

/* Replay->Find Previous Difference */
void
action_previous_difference(GtkAction *action, I7Story *story)
{
	i7_story_previous_difference(story);
}

/* Replay->Find Next Difference */
void
action_next_difference(GtkAction *action, I7Story *story)
{
	i7_story_next_difference(story);
}

/* Replay->Show Next Difference in Skein */
void
action_next_difference_skein(GtkAction *action, I7Story *story)
{
	i7_story_next_difference_skein(story);
}

/* Release->Release... (which was a 1978 song by Yes) */
void
action_release(GtkAction *action, I7Story *story)
{
	/* TRANSLATORS: Release->Release... */
	i7_story_compile(story, TRUE, FALSE, (CompileActionFunc)i7_story_save_compiler_output, _("Save the game for release"));
}

/* Release->Release for Testing... */
void
action_save_debug_build(GtkAction *action, I7Story *story)
{
	/* TRANSLATORS: Release->Release for Testing... */
	i7_story_compile(story, FALSE, FALSE, (CompileActionFunc)i7_story_save_compiler_output, _("Save debug build"));
}

/* Release->Open Materials Folder */
void
action_open_materials_folder(GtkAction *action, I7Story *story)
{
	GError *error = NULL;
	gchar *uri;
	GFile *materials_file = i7_story_get_materials_file(story);

	/* Prompt the user to create the folder if it doesn't exist */
	if(!g_file_query_exists(materials_file, NULL)) {
		/* TRANSLATORS: Release->Open Materials Folder */
		GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(story),
			GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_OK_CANCEL, _("Could not find Materials folder"));
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			_("At the moment, this project has no Materials folder - a "
			"convenient place to put figures, sounds, manuals, hints or other "
			"matter to be packaged up with a release.\n\nWould you like to "
			"create one?"));
		gint response = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		if(response == GTK_RESPONSE_OK) {
			if(!g_file_make_directory_with_parents(materials_file, NULL, &error)) {
				/* shouldn't already exist, so don't ignore G_IO_ERROR_EXISTS */
				IO_ERROR_DIALOG(GTK_WINDOW(story), materials_file, error, _("creating Materials folder"));
				goto finally;
			}
			file_set_custom_icon(materials_file, "application-x-inform-materials");
		} else
			goto finally;
	}

	if(g_file_query_file_type(materials_file, G_FILE_QUERY_INFO_NONE, NULL) != G_FILE_TYPE_DIRECTORY) {
		/* Odd; the Materials folder is a file. We open the containing path so
		 the user can see this and correct it if they like. */
		GFile *parent = g_file_get_parent(materials_file);
		uri = g_file_get_uri(parent);
		g_object_unref(parent);
	} else {
		uri = g_file_get_uri(materials_file);
	}

	/* TRANSLATORS: this string is used in error messages and should fit in the
	pattern "We couldn't open a program to show ___" */
	show_uri_externally(uri, GTK_WINDOW(story), _("the Materials folder"));

	g_free(uri);
finally:
	g_object_unref(materials_file);
}

/* Release->Export iFiction Record... */
void
action_export_ifiction_record(GtkAction *action, I7Story *story)
{
	i7_story_compile(story, FALSE, FALSE, (CompileActionFunc)i7_story_save_ifiction, NULL);
}

/* Help->Contents */
void
action_help_contents(GtkAction *action, I7Story *story)
{
	i7_story_show_doc_uri(story, "inform:///index.html");
}

/* Help->License */
void
action_help_license(GtkAction *action, I7Story *story)
{
	i7_story_show_doc_uri(story, "inform:///licenses/license.html");
}

/* Help->Help on Installed Extensions */
void
action_help_extensions(GtkAction *action, I7Story *story)
{
	GFile *home_file = g_file_new_for_path(g_get_home_dir());
	GFile *child1 = g_file_get_child(home_file, "Inform");
	GFile *child2 = g_file_get_child(child1, "Documentation");
	GFile *file = g_file_get_child(child2, "Extensions.html");
	g_object_unref(home_file);
	g_object_unref(child1);
	g_object_unref(child2);

	i7_story_show_docpage(story, file);
	g_object_unref(file);
}

/* Help->Recipe Book */
void
action_help_recipe_book(GtkAction *action, I7Story *story)
{
	i7_story_show_doc_uri(story, "inform:///Rallegs.html");
}

/* Help->Visit Inform7.com */
void
action_visit_inform7_com(GtkAction *action, I7App *app)
{
	/* TRANSLATORS: This string is used in error messages and should fit in the
	pattern "We couldn't show ___ in your browser" */
	show_uri_in_browser("http://inform7.com/", NULL, _("the Inform website"));
}

/* Help->Suggest a Feature */
void
action_suggest_feature(GtkAction *action, I7App *app)
{
	show_uri_in_browser("http://inform7.uservoice.com/", NULL,
		/* TRANSLATORS: This string is used in error messages and should fit in
		the pattern "We couldn't show ___ in your browser" */
		_("the Inform feature suggestion page"));
}

/* Help->Report a Bug */
void
action_report_bug(GtkAction *action, I7App *app)
{
	show_uri_in_browser("http://inform7.com/mantis", NULL,
		/* TRANSLATORS: This string is used in error messages and should fit in
		the pattern "We couldn't show ___ in your browser" */
		_("the Inform bug reports page"));
}

/* Help->About */
void
action_about(GtkAction *action, I7App *app)
{
	/* TRANSLATORS: Help->About ; %s is the copyright year. */
	char *copyright = g_strdup_printf(_("Copyright 2006\u2013%s " /* UTF8 en-dash */
		"Philip Chimento (front end),\n"
	    "Graham Nelson et al. (compiler)."), COPYRIGHT_YEAR);
	/* Build the credits string in such a way that it's robust to changes and
	translations don't get invalidated every time someone is added */
	GString *builder = g_string_new(_("Inform written by:"));
	g_string_append(builder, "\n"
		"    Graham Nelson\n"
		"\n");
	g_string_append(builder, _("Glulx compiler written by:"));
	g_string_append(builder, "\n"
		"    Graham Nelson\n"
		"    Andrew Plotkin\n"
		"\n");
	g_string_append(builder, _("Inform front-end written by:"));
	g_string_append(builder, "\n"
		"    Philip Chimento\n"
		"\n");
	g_string_append(builder, _("Contributions by:"));
	g_string_append(builder, "\n"
		"    Adam Thornton\n"
		"    Alan de Smet\n"
		"    Bart Massey\n"
		"    Daniel Nilsson\n"
		"    David Leverton\n"
		"    Evil Tabby Cat\n"
		"    Eric Forgeot\n"
		"    Jonathan Liu\n"
		"    Leandro Ribeiro\n"
		"    Matteo Settenvini\n"
		"    St\u00E9phane Aulery\n"
		"    Vincent Petry\n"
		"    Zachary Amsden\n");
	g_string_append(builder, _("In addition, Andrew Hunter's OS X version\n"
		"and David Kinder's Windows version\n"
		"proved invaluable."));
	g_string_append(builder, "\n\n");
	g_string_append(builder, _("Interface designed by:"));
	g_string_append(builder, "\n"
		"    Graham Nelson\n"
		"    Andrew Hunter\n"
		"\n");
	g_string_append(builder, _("Chimara written by:"));
	g_string_append(builder, "\n"
		"    Philip Chimento\n"
		"    Marijn van Vliet\n"
		"\n");
	g_string_append(builder, _("Elastic tabstops invented by:"));
	g_string_append(builder, "\n"
		"    Nick Gravgaard\n"
		"\n");
	g_string_append(builder, _("Contributions to the compiler:"));
	g_string_append(builder, "\n"
		"    Emily Short\n"
		"    Gunther Schmidl\n"
		"    Andrew Plotkin\n"
		"    Jason Penney\n"
		"    Joe Mason\n"
		"    Cedric Knight\n"
		"    David Kinder\n"
		"    Roger Firth\n"
		"    Michael Coyne\n"
		"    David Cornelson\n"
		"    Neil Cerutti\n"
		"    Kevin Bracey\n");
	char **authors = g_new0(char *, 2);  /* terminating 0 */
	authors[0] = g_string_free(builder, FALSE);
	char *translators_and_languages[] = {
		"\u00C1ngel Eduardo Garc\u00EDa", N_("Spanish"),
		"helado de brownie", N_("Spanish"),
		"Jhames Bolumbero", N_("Spanish"),
		"St\u00E9phane Aulery", N_("French"),
		NULL, NULL
	};
	builder = g_string_new("");
	char * const *iter = translators_and_languages;
	while(*iter != NULL) {
		g_string_append(builder, *iter++);
		g_string_append_printf(builder, " (%s)\n", gettext(*iter++));
	}
	char *translator_credits = g_string_free(builder, FALSE);

	GtkWindow *parent = get_toplevel_for_action(action);
	gtk_show_about_dialog(parent,
		"program-name", "Inform",
		"copyright", copyright,
		"comments", "Inform (" INFORM6_COMPILER_VERSION "/" PACKAGE_VERSION ")",
		"website", "http://inform7.com",
		"website-label", "inform7.com",
		/* TRANSLATORS: Caution, UTF8 right arrow */
		"license", _("See Help\u2192License for licensing information."),
		"authors", authors,
		"translator-credits", translator_credits,
		"logo-icon-name", "com.inform7.IDE",
		"title", _("About Inform"),
		NULL);
	g_free(copyright);
	g_strfreev(authors);
	g_free(translator_credits);
}
