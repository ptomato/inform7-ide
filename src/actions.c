/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2008-2015, 2019 Philip Chimento <philip.chimento@gmail.com>
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

/* File->New... */
void
action_new(GSimpleAction *action, GVariant *parameter, I7App *app)
{
	GtkWidget *newdialog = create_new_dialog();
	gtk_widget_show(newdialog);
}

/* File->Open... */
void
action_open(GSimpleAction *action, GVariant *parameter, I7App *app)
{
	i7_story_new_from_dialog(app);
}

/* Callback for when of the items from the File->Open Recent submenu
 is selected */
void
action_open_recent(GSimpleAction *action, GVariant *parameter, I7App *app)
{
	g_autofree char *uri = NULL;
	g_autofree char *group = NULL;
  g_variant_get(parameter, "(ss)", &uri, &group);

	g_autoptr(GFile) file = g_file_new_for_uri(uri);

	if (strcmp(group, "inform7_project") == 0) {
		i7_story_new_from_file(app, file);
	} else if (strcmp(group, "inform7_extension") == 0) {
		i7_extension_new_from_file(app, file, FALSE);
	} else if (strcmp(group, "inform7_builtin") == 0) {
		i7_extension_new_from_file(app, file, TRUE);
	} else {
		g_warning("Recent manager file does not have an Inform tag. This means "
			"it was not saved by Inform. I'll try to open it anyway.");
		i7_story_new_from_file(app, file);
	}
}

/* File->Install Extension... */
void
action_install_extension(GSimpleAction *action, GVariant *parameter, I7App *app)
{
	/* Select the Extensions tab */
	gtk_notebook_set_current_page(GTK_NOTEBOOK(app->prefs->prefs_notebook), I7_PREFS_EXTENSIONS);

	/* Show the preferences dialog */
	i7_app_present_prefs_window(app);

	/* Pretend the user clicked the Add button */
	g_signal_emit_by_name(app->prefs->extensions_add, "clicked");
}

/* File->Open Extension */
void
action_open_extension(GSimpleAction *action, GVariant *parameter, I7App *app)
{
	const char *uri;
	gboolean readonly;
	g_variant_get(parameter, "(sb)", &uri, &readonly);
	g_autoptr(GFile) file = g_file_new_for_uri(uri);
	i7_extension_new_from_file(app, file, readonly);
}

/* File->Import Into Skein... */
void
action_import_into_skein(GSimpleAction *action, GVariant *parameter, I7Story *story)
{
	GError *err = NULL;

	/* Ask the user for a file to import */
	/* TRANSLATORS: File->Import Into Skein... */
	g_autoptr(GtkFileChooserNative) dialog = gtk_file_chooser_native_new(_("Select the file to import into the skein"),
		GTK_WINDOW(story), GTK_FILE_CHOOSER_ACTION_OPEN, NULL, NULL);

	/* Create appropriate file filters */
	GtkFileFilter *filter1 = gtk_file_filter_new();
	gtk_file_filter_set_name(filter1, _("Interpreter recording files (*.rec)"));
	gtk_file_filter_add_pattern(filter1, "*.rec");
	GtkFileFilter *filter2 = gtk_file_filter_new();
	gtk_file_filter_set_name(filter2, _("All Files"));
	gtk_file_filter_add_pattern(filter2, "*");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter1);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter2);

	if (gtk_native_dialog_run(GTK_NATIVE_DIALOG(dialog)) != GTK_RESPONSE_ACCEPT)
		return;

	GFile *file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));
	if(!file)
		return; /* Fail silently */
	g_clear_object(&dialog);

	/* Provide some visual feedback that the command did something */
	if(!i7_skein_import(i7_story_get_skein(story), file, &err))
		error_dialog_file_operation(GTK_WINDOW(story), file, err, I7_FILE_ERROR_OPEN, NULL);
	else
		i7_story_show_pane(story, I7_PANE_SKEIN);

	g_object_unref(file);
}

/* File->Save */
void
action_save(GSimpleAction *action, GVariant *parameter, I7Document *document)
{
	i7_document_save(document);
}

/* File->Save As... */
void
action_save_as(GSimpleAction *action, GVariant *parameter, I7Document *document)
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
action_save_copy(GSimpleAction *action, GVariant *parameter, I7Document *document)
{
	GFile *file = i7_document_run_save_dialog(document, NULL);
	if(file) {
		i7_document_save_as(document, file);
		g_object_unref(file);
	}
}

/* File->Revert */
void
action_revert(GSimpleAction *action, GVariant *parameter, I7Document *document)
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
		_("_Cancel"), GTK_RESPONSE_CANCEL,
		_("_Revert"), GTK_RESPONSE_OK,
		NULL);
	gint result = gtk_dialog_run(GTK_DIALOG(revert_dialog));
	gtk_widget_destroy(revert_dialog);
	if(result != GTK_RESPONSE_OK)
		return; /* Only go on if the user clicked revert */

	i7_document_revert(document);
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
	I7App *theapp = I7_APP(g_application_get_default());
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
	g_autofree char *fontstring = i7_app_get_font_family(theapp);
	gtk_source_print_compositor_set_body_font_name(compositor, fontstring);

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

/* File->Print... */
void
action_print(GSimpleAction *action, GVariant *parameter, I7Document *document)
{
	GError *error = NULL;
	I7App *theapp = I7_APP(g_application_get_default());
	GtkPrintOperation *print = gtk_print_operation_new();
	gtk_print_operation_set_embed_page_setup(print, TRUE);
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
action_close(GSimpleAction *action, GVariant *parameter, I7Document *document)
{
	if (i7_document_verify_save(document))
		gtk_widget_destroy(GTK_WIDGET(document));
}

/* File->Quit */
void
action_quit(GSimpleAction *action, GVariant *parameter, I7App *app)
{
	i7_app_close_all_documents(app);
}

/* Edit->Undo */
void
action_undo(GSimpleAction *action, GVariant *parameter, I7Document *document)
{
	GtkSourceBuffer *buffer = i7_document_get_buffer(document);
	if(gtk_source_buffer_can_undo(buffer))
		gtk_source_buffer_undo(buffer);

	/* Update the "enabled" state of the undo and redo actions */
	g_simple_action_set_enabled(action, gtk_source_buffer_can_undo(buffer));
	GAction *redo = g_action_map_lookup_action(G_ACTION_MAP(document), "redo");
	g_simple_action_set_enabled(G_SIMPLE_ACTION(redo), gtk_source_buffer_can_redo(buffer));
}

/* Edit->Redo */
void
action_redo(GSimpleAction *action, GVariant *parameter, I7Document *document)
{
	GtkSourceBuffer *buffer = i7_document_get_buffer(document);
	if(gtk_source_buffer_can_redo(buffer))
		gtk_source_buffer_redo(buffer);

	/* Update the "enabled" state of the undo and redo actions */
	g_simple_action_set_enabled(action, gtk_source_buffer_can_redo(buffer));
	GAction *undo = g_action_map_lookup_action(G_ACTION_MAP(document), "undo");
	g_simple_action_set_enabled(G_SIMPLE_ACTION(undo), gtk_source_buffer_can_undo(buffer));
}

/* Edit->Cut */
void
action_cut(GSimpleAction *action, GVariant *parameter, I7Document *document)
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
action_copy(GSimpleAction *action, GVariant *parameter, I7Document *document)
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
action_paste(GSimpleAction *action, GVariant *parameter, I7Document *document)
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
action_select_all(GSimpleAction *action, GVariant *parameter, I7Document *document)
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
action_find(GSimpleAction *action, GVariant *parameter, I7Document *document)
{
	gtk_widget_show(document->findbar);
	const gchar *text = gtk_entry_get_text(GTK_ENTRY(document->findbar_entry));
	/* don't free */
	i7_document_set_quicksearch_not_found(document, !i7_document_highlight_quicksearch(document, text, TRUE));
	gtk_widget_grab_focus(document->findbar_entry);
}

/* Edit->Find Next */
void
action_find_next(GSimpleAction *action, GVariant *parameter, I7Document *document)
{
	const gchar *text = gtk_entry_get_text(GTK_ENTRY(document->findbar_entry));
	i7_document_set_quicksearch_not_found(document, !i7_document_highlight_quicksearch(document, text, TRUE));
}

/* Edit->Find Previous */
void
action_find_previous(GSimpleAction *action, GVariant *parameter, I7Document *document)
{
	const gchar *text = gtk_entry_get_text(GTK_ENTRY(document->findbar_entry));
	i7_document_set_quicksearch_not_found(document, !i7_document_highlight_quicksearch(document, text, FALSE));
}

/* Edit->Find and Replace... - For more complicated find operations, we use a
 dialog instead of the find bar */
void
action_replace(GSimpleAction *action, GVariant *parameter, I7Document *document)
{
	gtk_widget_show(document->find_dialog);
	gtk_window_present(GTK_WINDOW(document->find_dialog));
}

/* Edit->Scroll to Selection */
void
action_scroll_selection(GSimpleAction *action, GVariant *parameter, I7Document *document)
{
	i7_document_scroll_to_selection(document);
}

/* Edit->Search Files... - This is another dialog that searches any combination
 of the story, the installed extensions, and the documentation. */
void
action_search(GSimpleAction *action, GVariant *parameter, I7Document *document)
{
	gtk_widget_show(document->search_files_dialog);
	gtk_window_present(GTK_WINDOW(document->search_files_dialog));
    gtk_widget_grab_focus(document->search_files_entry);
}

/* Edit->Autocheck Spelling */
void
action_autocheck_spelling_toggle(GSimpleAction *action, GVariant *state, I7Document *document)
{
	g_simple_action_set_state(action, state);
	gboolean value = g_variant_get_boolean(state);
	i7_document_set_spellcheck(document, value);
}

/* Edit->Preferences... */
void
action_preferences(GSimpleAction *action, GVariant *parameter, I7App *app)
{
	i7_app_present_prefs_window(app);
}

/* View->Toolbar */
void
action_view_toolbar_toggled(GSimpleAction *action, GVariant *state, I7Document *document)
{
	g_simple_action_set_state(action, state);

	/* Set the default value for the next time a window is opened */
	GSettings *app_state = i7_app_get_state(I7_APP(g_application_get_default()));
	g_settings_set_value(app_state, PREFS_STATE_SHOW_TOOLBAR, state);

	gtk_widget_set_visible(document->toolbar, g_variant_get_boolean(state));
}

/* View->Statusbar */
void
action_view_statusbar_toggled(GSimpleAction *action, GVariant *state, I7Document *document)
{
	g_simple_action_set_state(action, state);

	/* Set the default value for the next time a window is opened */
	GSettings *app_state = i7_app_get_state(I7_APP(g_application_get_default()));
	g_settings_set_value(app_state, PREFS_STATE_SHOW_STATUSBAR, state);

	gtk_widget_set_visible(document->statusline, g_variant_get_boolean(state));
}

/* View->Notepad */
void
action_view_notepad_toggled(GSimpleAction *action, GVariant *state, I7Story *story)
{
	g_simple_action_set_state(action, state);

	/* Set the default value for the next time a window is opened */
	GSettings *app_state = i7_app_get_state(I7_APP(g_application_get_default()));
	g_settings_set_value(app_state, PREFS_STATE_SHOW_NOTEPAD, state);

	gtk_widget_set_visible(story->notes_window, g_variant_get_boolean(state));
}

void
action_show_pane(GSimpleAction *action, GVariant *parameter, I7Story *story)
{
	I7PanelPane pane = g_variant_get_uint32(parameter);
	i7_story_show_pane(story, pane);
}

void
action_show_tab(GSimpleAction *action, GVariant *parameter, I7Story *story)
{
	uint32_t pane, tab;
	g_variant_get(parameter, "(uu)", &pane, &tab);
	i7_story_show_tab(story, pane, tab);
}

/* View->Show Headings */
void
action_show_headings(GSimpleAction *action, GVariant *parameter, I7Story *story)
{
	i7_story_show_tab(story, I7_PANE_SOURCE, I7_SOURCE_VIEW_TAB_CONTENTS);
}

/* View->Current Section Only */
void
action_current_section_only(GSimpleAction *action, GVariant *parameter, I7Document *document)
{
	GtkTreePath *path = i7_document_get_deepest_heading(document);
	i7_document_show_heading(document, path);
}

/* View->Increase Restriction - View one level deeper in the headings hierarchy */
void
action_increase_restriction(GSimpleAction *action, GVariant *parameter, I7Document *document)
{
	GtkTreePath *path = i7_document_get_deeper_heading(document);
	i7_document_show_heading(document, path);
}

/* View->Decrease Restriction - View one level less deep in the headings hierarchy */
void
action_decrease_restriction(GSimpleAction *action, GVariant *parameter, I7Document *document)
{
	GtkTreePath *path = i7_document_get_shallower_heading(document);
	i7_document_show_heading(document, path);
}

/* View->Entire Source */
void
action_entire_source(GSimpleAction *action, GVariant *parameter, I7Document *document)
{
	i7_document_show_entire_source(document);
}

/* View->Previous Section */
void
action_previous_section(GSimpleAction *action, GVariant *parameter, I7Document *document)
{
	GtkTreePath *path = i7_document_get_previous_heading(document);
	i7_document_show_heading(document, path);
}

/* View->Next Section */
void
action_next_section(GSimpleAction *action, GVariant *parameter, I7Document *document)
{
	GtkTreePath *path = i7_document_get_next_heading(document);
	i7_document_show_heading(document, path);
}

/* Format->Indent */
void
action_indent(GSimpleAction *action, GVariant *parameter, I7Document *document)
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
action_unindent(GSimpleAction *action, GVariant *parameter, I7Document *document)
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
action_comment_out_selection(GSimpleAction *action, GVariant *parameter, I7Document *document)
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
action_uncomment_selection(GSimpleAction *action, GVariant *parameter, I7Document *document)
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
action_renumber_all_sections(GSimpleAction *action, GVariant *parameter, I7Document *document)
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
action_enable_elastic_tabstops_toggled(GSimpleAction *action, GVariant *state, I7Document *document)
{
	g_simple_action_set_state(action, state);
	gboolean value = g_variant_get_boolean(state);
	i7_document_set_elastic_tabstops(document, value);
}

/* Play->Go */
void
action_go(GSimpleAction *action, GVariant *parameter, I7Story *story)
{
	i7_story_compile(story, FALSE, FALSE, (CompileActionFunc)i7_story_run_compiler_output, NULL);
}

/* Play->Test Me */
void
action_test_me(GSimpleAction *action, GVariant *parameter, I7Story *story)
{
	i7_story_compile(story, FALSE, FALSE, (CompileActionFunc)i7_story_test_compiler_output, NULL);
}

/* Play->Stop */
void
action_stop(GSimpleAction *action, GVariant *parameter, I7Story *story)
{
	i7_story_stop_running_game(story);
}

/* Play->Refresh Index */
void
action_refresh_index(GSimpleAction *action, GVariant *parameter, I7Story *story)
{
	i7_story_compile(story, FALSE, TRUE, (CompileActionFunc)i7_story_show_pane, GUINT_TO_POINTER(I7_PANE_INDEX));
}

/* Replay->Replay Last Commands */
void
action_replay(GSimpleAction *action, GVariant *parameter, I7Story *story)
{
	i7_story_compile(story, FALSE, FALSE, (CompileActionFunc)i7_story_run_compiler_output_and_replay, NULL);
}

/* Replay->Replay Commands Blessed in Transcript */
void
action_play_all_blessed(GSimpleAction *action, GVariant *parameter, I7Story *story)
{
	i7_story_compile(story, FALSE, FALSE, (CompileActionFunc)i7_story_run_compiler_output_and_entire_skein, NULL);
}

/* Replay->Show Last Command */
void
action_show_last_command(GSimpleAction *action, GVariant *parameter, I7Story *story)
{
	I7Skein *skein = i7_story_get_skein(story);
	i7_story_show_node_in_transcript(story, i7_skein_get_played_node(skein));
}

/* Replay->Show Last Command in Skein */
void
action_show_last_command_skein(GSimpleAction *action, GVariant *parameter, I7Story *story)
{
	I7Skein *skein = i7_story_get_skein(story);
	g_signal_emit_by_name(skein, "show-node", I7_REASON_USER_ACTION, i7_skein_get_played_node(skein));
}

/* Replay->Find Previous Changed Command */
void
action_previous_changed_command(GSimpleAction *action, GVariant *parameter, I7Story *story)
{
	i7_story_previous_changed(story);
}

/* Replay->Find Next Changed Command */
void
action_next_changed_command(GSimpleAction *action, GVariant *parameter, I7Story *story)
{
	i7_story_next_changed(story);
}

/* Replay->Find Previous Difference */
void
action_previous_difference(GSimpleAction *action, GVariant *parameter, I7Story *story)
{
	i7_story_previous_difference(story);
}

/* Replay->Find Next Difference */
void
action_next_difference(GSimpleAction *action, GVariant *parameter, I7Story *story)
{
	i7_story_next_difference(story);
}

/* Replay->Show Next Difference in Skein */
void
action_next_difference_skein(GSimpleAction *action, GVariant *parameter, I7Story *story)
{
	i7_story_next_difference_skein(story);
}

/* Release->Release... (which was a 1978 song by Yes) */
void
action_release(GSimpleAction *action, GVariant *parameter, I7Story *story)
{
	/* TRANSLATORS: Release->Release... */
	i7_story_compile(story, TRUE, FALSE, (CompileActionFunc)i7_story_save_compiler_output, _("Save the game for release"));
}

/* Release->Release for Testing... */
void
action_save_debug_build(GSimpleAction *action, GVariant *parameter, I7Story *story)
{
	/* TRANSLATORS: Release->Release for Testing... */
	i7_story_compile(story, FALSE, FALSE, (CompileActionFunc)i7_story_save_compiler_output, _("Save debug build"));
}

/* Release->Open Materials Folder */
void
action_open_materials_folder(GSimpleAction *action, GVariant *parameter, I7Story *story)
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
			file_set_custom_icon(materials_file, "com.inform7.IDE.application-x-inform-materials");
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
action_export_ifiction_record(GSimpleAction *action, GVariant *parameter, I7Story *story)
{
	i7_story_compile(story, FALSE, FALSE, (CompileActionFunc)i7_story_save_ifiction, NULL);
}

/* Help->Contents */
void
action_help_contents(GSimpleAction *action, GVariant *parameter, I7Story *story)
{
	i7_story_show_doc_uri(story, "inform:///index.html");
}

/* Help->License */
void
action_help_license(GSimpleAction *action, GVariant *parameter, I7Story *story)
{
	i7_story_show_doc_uri(story, "inform:///licenses/license.html");
}

/* Help->Help on Installed Extensions */
void
action_help_extensions(GSimpleAction *action, GVariant *parameter, I7Story *story)
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
action_help_recipe_book(GSimpleAction *action, GVariant *parameter, I7Story *story)
{
	i7_story_show_doc_uri(story, "inform:///Rallegs.html");
}

/* Help->Visit Inform7.com */
void
action_visit_inform7_com(GSimpleAction *action, GVariant *parameter, I7App *app)
{
	/* TRANSLATORS: This string is used in error messages and should fit in the
	pattern "We couldn't show ___ in your browser" */
	show_uri_in_browser("http://inform7.com/", NULL, _("the Inform website"));
}

/* Help->About */
void
action_about(GSimpleAction *action, GVariant *parameter, I7App *app)
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
	g_string_append(builder, _("Inform app for Linux written by:"));
	g_string_append(builder, "\n"
		"    Philip Chimento\n"
		"\n");
	g_string_append(builder, _("Contributions by:"));
	g_string_append(builder, "\n"
		"    Adam Thornton\n"
		"    Alan de Smet\n"
		"    Andrew Geng\n"
		"    Bart Massey\n"
		"    Ben Kirwin\n"
		"    Daniel Nilsson\n"
		"    David Leverton\n"
		"    Dominic Delabruere\n"
		"    Ed Swartz\n"
		"    Evil Tabby Cat\n"
		"    Eric Forgeot\n"
		"    Ian D. Bollinger\n"
		"    interactivefiction\n"
		"    Jonathan Liu\n"
		"    Josh Giesbrecht\n"
		"    Leandro Ribeiro\n"
		"    Matteo Settenvini\n"
		"    pteromys\n"
		"    St\u00E9phane Aulery\n"
		"    Vincent Petry\n"
		"    Zachary Amsden\n"
		"    Zed Lopez\n");
	g_string_append(builder, _("In addition, the source code of the macOS app\n"
		" (by Andrew Hunter and Toby Nelson)\n"
		"and the Windows app (by David Kinder)\n"
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

	gtk_show_about_dialog(NULL,
		"program-name", "Inform App",
		"copyright", copyright,
        "version", PACKAGE_VERSION,
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
