/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * new-i7
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

#include <string.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksourceiter.h>
#include "document.h"
#include "document-private.h"
#include "app.h"
#include "builder.h"
#include "configfile.h"
#include "elastic.h"
#include "error.h"
#include "file.h"

/* CALLBACKS */

void
on_buffer_mark_set(GtkTextBuffer *buffer, GtkTextIter *location, GtkTextMark *mark, I7Document *document)
{
	I7_DOCUMENT_USE_PRIVATE(document, priv);
	if(gtk_text_mark_get_name(mark) && strcmp(gtk_text_mark_get_name(mark), "selection-bound"))
		gtk_action_group_set_sensitive(priv->selection_action_group, gtk_text_buffer_get_has_selection(buffer));
}

void
on_buffer_changed(GtkTextBuffer *buffer, I7Document *document)
{
	gtk_action_set_sensitive(document->undo, gtk_source_buffer_can_undo(GTK_SOURCE_BUFFER(buffer)));
	gtk_action_set_sensitive(document->redo, gtk_source_buffer_can_redo(GTK_SOURCE_BUFFER(buffer)));
}

void
on_buffer_modified_changed(GtkTextBuffer *buffer, I7Document *document)
{
	if(gtk_text_buffer_get_modified(buffer))
		i7_document_set_modified(document, TRUE);
}

static gboolean
filter_depth(GtkTreeModel *model, GtkTreeIter *iter, I7Document *document)
{
	gint depth;
	gtk_tree_model_get(model, iter, I7_HEADINGS_DEPTH, &depth, -1);
	return depth <= I7_DOCUMENT_PRIVATE(document)->heading_depth;
}

static void
on_findbar_close_clicked(GtkToolButton *button, I7Document *document)
{
	gtk_widget_hide(document->findbar);
	i7_document_unhighlight_quicksearch(document);
}

/* TYPE SYSTEM */

G_DEFINE_TYPE(I7Document, i7_document, GTK_TYPE_WINDOW);

static void
i7_document_init(I7Document *self)
{
	I7_DOCUMENT_USE_PRIVATE(self, priv);
	I7App *theapp = i7_app_get();

	/* Set the icon */
	gtk_window_set_icon_name(GTK_WINDOW(self), "inform7");

	/* Set the minimum size so that the window can be sized smaller than the
	 widgets inside it */
	gtk_widget_set_size_request(GTK_WIDGET(self), 200, 100);
	
	/* Build the interface */
	gchar *filename = i7_app_get_datafile_path(theapp, "ui/document.ui");
	GtkBuilder *builder = create_new_builder(filename, self);
	g_free(filename);
	
	/* Create the private properties */
	priv->filename = NULL;
	priv->monitor = NULL;
	priv->accels = NULL;
	priv->buffer = GTK_SOURCE_BUFFER(load_object(builder, "buffer"));
	g_object_ref(priv->buffer);
	/* Add invisible tag to buffer */
	GtkTextTagTable *table = gtk_text_buffer_get_tag_table(GTK_TEXT_BUFFER(priv->buffer));
	priv->invisible_tag = GTK_TEXT_TAG(load_object(builder, "invisible_tag"));
	gtk_text_tag_table_add(table, priv->invisible_tag);
	/* do not unref table */
	priv->heading_depth = I7_HEADING_PART;
	priv->headings = GTK_TREE_STORE(load_object(builder, "headings_store"));
	g_object_ref(priv->headings);
	priv->filter = GTK_TREE_MODEL(load_object(builder, "headings_filtermodel"));
	g_object_ref(priv->filter);
	gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(priv->filter), (GtkTreeModelFilterVisibleFunc)filter_depth, self, NULL);
	priv->current_heading = gtk_tree_path_new_first();
	priv->highlighted_view = NULL;
	priv->modified = FALSE;
	
	/* Make the action groups. This for-loop is a temporary fix
	and can be removed once Glade supports adding actions and accelerators to an
	action group. */
	const gchar *actions[] = {
		"view_menu", "",
		"format_menu", "",
		"save", NULL, /* NULL means use the stock accelerator */
		"save_as", "<shift><ctrl>S",
		"save_copy", "",
		"revert", NULL,
		"page_setup", "",
		"print_preview", "<shift><ctrl>P",
		"print", "<ctrl>P",
		"close", NULL,
		"undo", "<ctrl>Z",
		"redo", "<shift><ctrl>Z",
		"paste", NULL,
		"select_all", "<ctrl>A",
		"find", NULL,
		"find_next", "<ctrl>G",
		"find_previous", "<shift><ctrl>G",
		"replace", "<ctrl>H",
		"search", "<shift><ctrl>F",
		"check_spelling", "<shift>F7",
		"autocheck_spelling", "",
		"view_toolbar", "",
		"view_statusbar", "",
		"show_headings", "<shift><ctrl>H",
		"current_section_only", "<alt><ctrl>Right",
		"increase_restriction", "<ctrl>Right",
		"decrease_restriction", "<ctrl>Left",
		"entire_source", "<alt><ctrl>Left",
		"previous_section", "<ctrl>Page_Up",
		"next_section", "<ctrl>Page_Down",
		"indent", "<ctrl>T",
		"unindent", "<shift><ctrl>T",
		"renumber_all_sections", "<alt><ctrl>N",
		"enable_elastic_tabs", "", 
		NULL
	};
	const gchar *selection_actions[] = {
		"scroll_selection", "<ctrl>J",
		"comment_out_selection", "<ctrl>slash",
		"uncomment_selection", "<ctrl>question", /* Ctrl-Shift-/ doesn't work, Gnome bug #614146 */
		NULL
	};
	const gchar *copy_actions[] = {
		"cut", NULL,
		"copy", NULL,
		NULL
	};
	add_actions(builder, &(priv->document_action_group), "document_actions", actions);
	add_actions(builder, &(priv->selection_action_group), "selection_actions", selection_actions);
	add_actions(builder, &(priv->copy_action_group), "copy_actions", copy_actions);
	
	self->ui_manager = gtk_ui_manager_new();
	i7_app_insert_action_groups(theapp, self->ui_manager);
	gtk_ui_manager_insert_action_group(self->ui_manager, priv->document_action_group, 0);
	gtk_ui_manager_insert_action_group(self->ui_manager, priv->selection_action_group, 0);
	gtk_ui_manager_insert_action_group(self->ui_manager, priv->copy_action_group, 0);
	
	/* Public members */
	LOAD_WIDGET(box);
	LOAD_WIDGET(statusline);
	LOAD_WIDGET(statusbar);
	LOAD_WIDGET(progressbar);
	self->findbar = NULL;
	LOAD_WIDGET(findbar_entry);
	g_object_ref(self->findbar_entry);
	LOAD_WIDGET(find_dialog);
	gtk_window_set_transient_for(GTK_WINDOW(self->find_dialog), GTK_WINDOW(self));
	LOAD_WIDGET(search_type);
	LOAD_WIDGET(find_entry);
	LOAD_WIDGET(replace_entry);
	LOAD_WIDGET(ignore_case);
	LOAD_WIDGET(reverse);
	LOAD_WIDGET(restrict_search);
	LOAD_WIDGET(find_button);
	LOAD_WIDGET(replace_button);
	LOAD_WIDGET(replace_all_button);
	LOAD_WIDGET(search_files_dialog);
	gtk_window_set_transient_for(GTK_WINDOW(self->search_files_dialog), GTK_WINDOW(self));
	LOAD_WIDGET(search_files_type);
	LOAD_WIDGET(search_files_entry);
	LOAD_WIDGET(search_files_project);
	LOAD_WIDGET(search_files_extensions);
	LOAD_WIDGET(search_files_documentation);
	LOAD_WIDGET(search_files_ignore_case);
	LOAD_WIDGET(search_files_find);
	LOAD_ACTION(priv->document_action_group, undo);
	LOAD_ACTION(priv->document_action_group, redo);
	LOAD_ACTION(priv->document_action_group, current_section_only);
	LOAD_ACTION(priv->document_action_group, increase_restriction);
	LOAD_ACTION(priv->document_action_group, decrease_restriction);
	LOAD_ACTION(priv->document_action_group, entire_source);
	LOAD_ACTION(priv->document_action_group, previous_section);
	LOAD_ACTION(priv->document_action_group, next_section);
	LOAD_ACTION(priv->document_action_group, autocheck_spelling);
	LOAD_ACTION(priv->document_action_group, check_spelling);
	LOAD_ACTION(priv->document_action_group, enable_elastic_tabs);
	gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(gtk_action_group_get_action(priv->document_action_group, "view_statusbar")), config_file_get_bool(PREFS_STATUSBAR_VISIBLE));
	gtk_container_add(GTK_CONTAINER(self), self->box);

	g_object_unref(builder);
}

static void
i7_document_finalize(GObject *self)
{
	I7_DOCUMENT_USE_PRIVATE(self, priv);
	I7App *theapp = i7_app_get();
	
	if(priv->filename)
		g_free(priv->filename);
	if(priv->monitor) {
        g_file_monitor_cancel(priv->monitor);
		g_object_unref(priv->monitor);
	}
	g_object_unref(I7_DOCUMENT(self)->ui_manager);
	g_object_unref(priv->headings);
	gtk_tree_path_free(priv->current_heading);

	i7_app_remove_document(theapp, I7_DOCUMENT(self));
	
	G_OBJECT_CLASS(i7_document_parent_class)->finalize(self);

	if(i7_app_get_num_open_documents(theapp) == 0)
		gtk_main_quit();
}

static void
i7_document_class_init(I7DocumentClass *klass)
{
	g_type_class_add_private(klass, sizeof(I7DocumentPrivate));
	
	/* Private pure virtual function */
	klass->extract_title = NULL;
	klass->set_contents_display = NULL;
	/* Public pure virtual functions */
	klass->get_default_view = NULL;
	klass->save = NULL;
	klass->save_as = NULL;
	klass->scroll_to_selection = NULL;
	klass->update_tabs = NULL;
	klass->update_fonts = NULL;
	klass->update_font_sizes = NULL;
	klass->expand_headings_view = NULL;
	klass->highlight_search = NULL;
	klass->set_spellcheck = NULL;
	klass->check_spelling = NULL;
	klass->set_elastic_tabs = NULL;
	
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = i7_document_finalize;
}

void
i7_document_add_menus_and_findbar(I7Document *document)
{
	I7_DOCUMENT_USE_PRIVATE(document, priv);
	GError *error = NULL;
	
	gchar *filename = i7_app_get_datafile_path(i7_app_get(), "ui/gnome-inform7.uimanager.xml");
	gtk_ui_manager_add_ui_from_file(document->ui_manager, filename, &error);
	g_free(filename);
	if(error)
		ERROR(_("Building menus failed"), error);

	document->findbar = gtk_ui_manager_get_widget(document->ui_manager, "/FindToolbar");
	gtk_widget_set_no_show_all(document->findbar, TRUE);
	gtk_toolbar_set_icon_size(GTK_TOOLBAR(document->findbar), GTK_ICON_SIZE_MENU);
	gtk_toolbar_set_style(GTK_TOOLBAR(document->findbar), GTK_TOOLBAR_BOTH_HORIZ);
	gtk_widget_hide(document->findbar);

	GtkToolItem *findbar_close = gtk_tool_button_new_from_stock(GTK_STOCK_CLOSE);
	gtk_tool_item_set_is_important(findbar_close, FALSE);
	gtk_widget_show(GTK_WIDGET(findbar_close));
	g_signal_connect(findbar_close, "clicked", G_CALLBACK(on_findbar_close_clicked), document);
	GtkToolItem *findbar_entry_container = gtk_tool_item_new();
	gtk_container_add(GTK_CONTAINER(findbar_entry_container), document->findbar_entry);
	g_object_unref(document->findbar_entry);
	gtk_widget_show_all(GTK_WIDGET(findbar_entry_container));
	gtk_toolbar_insert(GTK_TOOLBAR(document->findbar), findbar_entry_container, 0);
	gtk_toolbar_insert(GTK_TOOLBAR(document->findbar), findbar_close, -1);

	gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(gtk_action_group_get_action(priv->document_action_group, "view_toolbar")), config_file_get_bool(PREFS_TOOLBAR_VISIBLE));
	
	/* Connect the accelerators */
	priv->accels = gtk_ui_manager_get_accel_group(document->ui_manager);
	g_object_ref(priv->accels); /* Apparently adding it to the window doesn't ref it? */ 
	gtk_window_add_accel_group(GTK_WINDOW(document), priv->accels);
}

static void
i7_document_refresh_title(I7Document *document)
{
	I7_DOCUMENT_USE_PRIVATE(document, priv);
	gchar *documentname = i7_document_get_display_name(document);
	
	if(priv->modified)
	{
		gchar *title = g_strconcat("*", documentname, NULL);
		gtk_window_set_title(GTK_WINDOW(document), title);
	}
	else
		gtk_window_set_title(GTK_WINDOW(document), documentname);
	g_free(documentname);
}

void
i7_document_set_path(I7Document *document, const gchar *filename)
{
	I7_DOCUMENT_USE_PRIVATE(document, priv);
    if(priv->filename)
        g_free(priv->filename);
    priv->filename = g_strdup(filename);
    i7_document_refresh_title(document);
}

/* Returns a newly-allocated string containing the full path to this document */
gchar *
i7_document_get_path(const I7Document *document)
{
	return g_strdup(I7_DOCUMENT_PRIVATE(document)->filename);
}

/* Returns a newly-allocated string containing the filename of this document
 without the full path, converted to UTF-8, suitable for display in a window 
 titlebar */
gchar *
i7_document_get_display_name(I7Document *document)
{
	return g_filename_display_basename(I7_DOCUMENT_PRIVATE(document)->filename);
}

GtkSourceBuffer *
i7_document_get_buffer(I7Document *document)
{
	return I7_DOCUMENT_PRIVATE(document)->buffer;
}

GtkTextView *
i7_document_get_default_view(I7Document *document)
{
	return I7_DOCUMENT_GET_CLASS(document)->get_default_view(document);
}

/* Write the source to the source buffer & clear the undo history */
void
i7_document_set_source_text(I7Document *document, gchar *text)
{
	GtkSourceBuffer *buffer = I7_DOCUMENT_PRIVATE(document)->buffer;
	gtk_source_buffer_begin_not_undoable_action(buffer);
	gtk_text_buffer_set_text(GTK_TEXT_BUFFER(buffer), text, -1);
	gtk_source_buffer_end_not_undoable_action(buffer);
	gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(buffer), FALSE);
}

/* Get text in UTF-8. Allocates a new string */
gchar *
i7_document_get_source_text(I7Document *document)
{
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(I7_DOCUMENT_PRIVATE(document)->buffer);
	GtkTextIter start, end;
	gtk_text_buffer_get_bounds(buffer, &start, &end);
	return gtk_text_buffer_get_text(buffer, &start, &end, TRUE);
}

gboolean
i7_document_get_modified(I7Document *document)
{
	I7_DOCUMENT_USE_PRIVATE(document, priv);
	return priv->modified;
}

void
i7_document_set_modified(I7Document *document, gboolean modified)
{
	I7_DOCUMENT_USE_PRIVATE(document, priv);
	priv->modified = modified;
	i7_document_refresh_title(document);
}

GtkTreeModel *
i7_document_get_headings(I7Document *document)
{
	return I7_DOCUMENT_PRIVATE(document)->filter;
}

/* Convert a path returned from signals on the filter model to a path on the
underlying headings model. Free path when done. */
GtkTreePath *
i7_document_get_child_path(I7Document *document, GtkTreePath *path)
{
	I7_DOCUMENT_USE_PRIVATE(document, priv);
	GtkTreePath *real_path = gtk_tree_model_filter_convert_path_to_child_path(GTK_TREE_MODEL_FILTER(priv->filter), path);
	g_assert(real_path);
	return real_path;
}

static void
on_document_changed(GFileMonitor *monitor, GFile *file, GFile *other_file, GFileMonitorEvent event_type, I7Document *document)
{
	switch(event_type) {
		case G_FILE_MONITOR_EVENT_CREATED:
		case G_FILE_MONITOR_EVENT_CHANGED:
		/* g_file_set_contents works by deleting and creating */
		{
			GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(document), GTK_DIALOG_DESTROY_WITH_PARENT, 
				GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
				_("The source code has been modified from outside Inform.\n"
				"Do you want to reload it?"));
			if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES) {
				gchar *filename = g_file_get_path(file);
				gchar *text = read_source_file(filename);
				g_free(filename);
				if(text) {
					i7_document_set_source_text(document, text);
					g_free(text);
				}
			}
			gtk_widget_destroy(dialog);
		}
			break;
		case G_FILE_MONITOR_EVENT_DELETED:
		case G_FILE_MONITOR_EVENT_UNMOUNTED:
		/* If the file is removed, quietly make sure the user gets a chance to
		save it before exiting */
			i7_document_set_modified(document, TRUE);
		default:
			;
	}
}

void
i7_document_monitor_file(I7Document *document, const gchar *filename)
{
	I7_DOCUMENT_USE_PRIVATE(document, priv);
	
	GError *error = NULL;
	GFile *handle = g_file_new_for_path(filename);
	priv->monitor = g_file_monitor_file(handle,	G_FILE_MONITOR_NONE, NULL, &error);
	g_object_unref(handle);
	if(!priv->monitor) {
	    WARN_S(_("Could not start file monitor"), filename, error);
		g_error_free(error);
		return;
	}
	g_signal_connect(priv->monitor, "changed", G_CALLBACK(on_document_changed), document);
}

void
i7_document_stop_file_monitor(I7Document *document)
{
	I7_DOCUMENT_USE_PRIVATE(document, priv);
	
	if(priv->monitor) {
		g_file_monitor_cancel(priv->monitor);
		g_object_unref(priv->monitor);
		priv->monitor = NULL;
	}
}
gboolean
i7_document_save(I7Document *document)
{
	return I7_DOCUMENT_GET_CLASS(document)->save(document);
}

void
i7_document_save_as(I7Document *document, const gchar *filename)
{
	I7_DOCUMENT_GET_CLASS(document)->save_as(document, filename);
}

/* If the document is not saved, ask the user whether he/she wants to save it.
Returns TRUE if we can proceed, FALSE if the user cancelled. */
gboolean 
i7_document_verify_save(I7Document *document) 
{
	if(!i7_document_get_modified(document))
		return TRUE;
	
	gchar *filename = i7_document_get_display_name(document);
	GtkWidget *save_changes_dialog = gtk_message_dialog_new_with_markup(GTK_WINDOW(document), GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_WARNING, GTK_BUTTONS_NONE,
		_("<b><big>Save changes to '%s' before closing?</big></b>"), filename);
	g_free(filename);
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(save_changes_dialog),
		_("If you don't save, your changes will be lost."));
	gtk_dialog_add_buttons(GTK_DIALOG(save_changes_dialog),
		_("Close _without saving"), GTK_RESPONSE_REJECT,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_OK,
		NULL);
	gint result = gtk_dialog_run(GTK_DIALOG(save_changes_dialog));
	gtk_widget_destroy(save_changes_dialog);
	switch(result) {
		case GTK_RESPONSE_OK: /* save */
			return i7_document_save(document);
		case GTK_RESPONSE_REJECT: /* quit without saving */
			return TRUE;
	}
	return FALSE; /* various ways of cancelling the dialog */
}

void
i7_document_close(I7Document *document)
{
	if(i7_document_get_modified(document)) {
		gchar *filename = i7_document_get_display_name(document);
		GtkWidget *save_changes_dialog = gtk_message_dialog_new_with_markup(GTK_WINDOW(document), GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_WARNING, GTK_BUTTONS_NONE,
			_("<b><big>Save changes to '%s' before closing?</big></b>"),
			filename);
		g_free(filename);
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(save_changes_dialog),
			_("If you don't save, your changes will be lost."));
		gtk_dialog_add_buttons(GTK_DIALOG(save_changes_dialog),
			_("Close _without saving"), GTK_RESPONSE_REJECT,
			GTK_STOCK_SAVE, GTK_RESPONSE_OK,
			NULL);
		gint result = gtk_dialog_run(GTK_DIALOG(save_changes_dialog));
		gtk_widget_destroy(save_changes_dialog);
		if(result == GTK_RESPONSE_OK)
			i7_document_save(document);
	}
	g_object_unref(document);
}

void
i7_document_scroll_to_selection(I7Document *document)
{
	I7_DOCUMENT_GET_CLASS(document)->scroll_to_selection(document);
}

void
i7_document_jump_to_line(I7Document *document, guint lineno)
{
	I7_DOCUMENT_USE_PRIVATE(document, priv);
	GtkTextIter start, end;
	/* Line number is counted from 0 internally, so subtract one */
	gtk_text_buffer_get_iter_at_line(GTK_TEXT_BUFFER(priv->buffer), &start, lineno - 1);
	end = start;
	gtk_text_iter_forward_to_line_end(&end);
	gtk_text_buffer_select_range(GTK_TEXT_BUFFER(priv->buffer), &start, &end);
	I7_DOCUMENT_GET_CLASS(document)->scroll_to_selection(document);
}

void 
i7_document_update_tabs(I7Document *document)
{
	I7_DOCUMENT_GET_CLASS(document)->update_tabs(document);
}

void
i7_document_update_fonts(I7Document *document)
{
	I7_DOCUMENT_GET_CLASS(document)->update_fonts(document);
}

void 
i7_document_update_font_sizes(I7Document *document)
{
	I7_DOCUMENT_GET_CLASS(document)->update_font_sizes(document);
}

void
i7_document_update_font_styles(I7Document *document)
{
	g_idle_add((GSourceFunc)update_style, I7_DOCUMENT_PRIVATE(document)->buffer);
}

/* Turn source highlighting on or off in this document's source buffer */
void 
i7_document_update_source_highlight(I7Document *document) 
{
	I7_DOCUMENT_USE_PRIVATE(document, priv);
    gtk_source_buffer_set_highlight_syntax(priv->buffer, config_file_get_bool(PREFS_SYNTAX_HIGHLIGHTING));
}

/* Recalculate the document's elastic tabs */
void
i7_document_refresh_elastic_tabs(I7Document *document)
{
	elastic_recalculate_view(i7_document_get_default_view(document));
}

void
i7_document_expand_headings_view(I7Document *document)
{
	I7_DOCUMENT_GET_CLASS(document)->expand_headings_view(document);
}

void
i7_document_set_headings_filter_level(I7Document *document, gint depth)
{
	I7DocumentPrivate *priv = I7_DOCUMENT_PRIVATE(document);
	priv->heading_depth = depth;
	gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(priv->filter));
	/* Refiltering doesn't work when moving to a higher depth, so... */
	i7_document_reindex_headings(document);
}

static int
get_heading_from_string(gchar *text)
{
	int retval = -1;
		
	gchar *lcase = g_utf8_strdown(text, -1);
	if(strcmp(lcase, "volume") == 0)
		retval = I7_HEADING_VOLUME;
	else if(strcmp(lcase, "book") == 0)
		retval = I7_HEADING_BOOK;
	else if(strcmp(lcase, "part") == 0)
		retval = I7_HEADING_PART;
	else if(strcmp(lcase, "chapter") == 0)
		retval = I7_HEADING_CHAPTER;
	else if(strcmp(lcase, "section") == 0)
		retval = I7_HEADING_SECTION;
	
	g_assert(retval != -1);
	
	g_free(lcase);
	return retval;
}

void
i7_document_reindex_headings(I7Document *document)
{
	I7DocumentPrivate *priv = I7_DOCUMENT_PRIVATE(document);
	I7App *theapp = i7_app_get();
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(priv->buffer);
    GtkTreeStore *tree = priv->headings;
    gtk_tree_store_clear(tree);
    GtkTreeIter title, volume, book, part, chapter, section, current;
    gboolean volume_used = FALSE, book_used = FALSE, part_used = FALSE, chapter_used = FALSE;
	gboolean at_least_one = FALSE;
	
	GtkTextIter lastline, thisline, nextline, end;
    gtk_text_buffer_get_start_iter(buffer, &lastline);
    gtk_text_buffer_get_iter_at_line(buffer, &thisline, 1);
	gtk_text_buffer_get_iter_at_line(buffer, &nextline, 2);
	gchar *text = gtk_text_iter_get_text(&lastline, &thisline);
	/* Include \n */
	gchar *realtitle = I7_DOCUMENT_GET_CLASS(document)->extract_title(document, text);
	g_free(text);
	
	gtk_tree_store_append(tree, &title, NULL);
	gtk_tree_store_set(tree, &title,
		I7_HEADINGS_TITLE, realtitle,
		I7_HEADINGS_LINE, 1,
		I7_HEADINGS_DEPTH, -1,
		I7_HEADINGS_BOLD, PANGO_WEIGHT_BOLD,
		-1);
	g_free(realtitle);
	
    while(gtk_text_iter_forward_lines(&lastline, 3)) {
		/* Swap the iterators around using end as a temporary variable */
		end = nextline;
		nextline = lastline;
		lastline = thisline;
		thisline = end;
		gtk_text_iter_forward_to_line_end(&end);
		
		GMatchInfo *match = NULL;
		text = NULL;
		if(gtk_text_iter_ends_line(&lastline) && gtk_text_iter_ends_line(&nextline) /* Blank line before and after */
			&& (text = gtk_text_iter_get_text(&thisline, &end))
			&& g_regex_match(theapp->regices[I7_APP_REGEX_HEADINGS], text, 0, &match)) 
		{
			gchar *level = g_match_info_fetch_named(match, "level");
			int depth = get_heading_from_string(level);
			g_free(level);
			gchar *secnum = g_match_info_fetch_named(match, "secnum");
			gchar *sectitle = g_match_info_fetch_named(match, "sectitle");
			guint lineno = gtk_text_iter_get_line(&thisline) + 1;
			/* Line numbers counted from 0 */
			
			switch(depth) {
				case I7_HEADING_VOLUME:
					gtk_tree_store_append(tree, &volume, &title);
					current = volume;
					volume_used = TRUE;
					break;
				case I7_HEADING_BOOK:
					gtk_tree_store_append(tree, &book, volume_used? &volume : &title);
					current = book;
					book_used = TRUE;
					break;
				case I7_HEADING_PART:
					gtk_tree_store_append(tree, &part, book_used? &book : volume_used? &volume : &title);
					current = part;
					part_used = TRUE;
					break;
				case I7_HEADING_CHAPTER:
					gtk_tree_store_append(tree, &chapter, part_used? &part : book_used? &book : volume_used? &volume : &title);
					current = chapter;
					chapter_used = TRUE;
					break;
				default: /* section */
					gtk_tree_store_append(tree, &section, chapter_used? &chapter : part_used? &part : book_used? &book : volume_used? &volume : &title);
					current = section;
			}
			
			gtk_tree_store_set(tree, &current,
				I7_HEADINGS_TITLE, text,
				I7_HEADINGS_LINE, lineno,
				I7_HEADINGS_DEPTH, depth,
				I7_HEADINGS_SECTION_NUMBER, secnum,
				I7_HEADINGS_SECTION_NAME, sectitle,
				I7_HEADINGS_BOLD, PANGO_WEIGHT_NORMAL,
				-1);
			/* Do not free strings (?) */
			at_least_one = TRUE;
		}
		
		if(text)
			g_free(text);
		if(match)
			g_match_info_free(match);
    }
	i7_document_expand_headings_view(document);
	
	/* Display appropriate messages in the contents view */
	/* If there is at least one child of the root node in the filtered model,
	then the contents can be shown normally. */
	g_assert(gtk_tree_model_get_iter_first(priv->filter, &current));
	if(gtk_tree_model_iter_has_child(priv->filter, &current))
		I7_DOCUMENT_GET_CLASS(document)->set_contents_display(document, I7_CONTENTS_NORMAL);
	else {
		/* If there is no child showing in the filtered model, but there is one
		in the original headings model, then the filtered model is set to too
		shallow a level. */
		g_assert(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(priv->headings), &current));
		if(gtk_tree_model_iter_has_child(GTK_TREE_MODEL(priv->headings), &current))
			I7_DOCUMENT_GET_CLASS(document)->set_contents_display(document, I7_CONTENTS_TOO_SHALLOW);
		else
			/* Otherwise, there simply were no headings recognized. */
			I7_DOCUMENT_GET_CLASS(document)->set_contents_display(document, I7_CONTENTS_NO_HEADINGS);
	}
}

void
i7_document_show_heading(I7Document *document, GtkTreePath *path)
{
	I7DocumentPrivate *priv = I7_DOCUMENT_PRIVATE(document);
	GtkTreeModel *headings = GTK_TREE_MODEL(priv->headings);
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(priv->buffer);
	
	GtkTreeIter iter;
	g_assert(gtk_tree_model_get_iter(headings, &iter, path));
	
	guint startline = 0;
	I7Heading depth = I7_HEADING_NONE;
	gtk_tree_model_get(headings, &iter,
		I7_HEADINGS_LINE, &startline,
		I7_HEADINGS_DEPTH, &depth, 
		-1);
	startline--;

	/* Remove the invisible tag */
	GtkTextIter start, end;
	gtk_text_buffer_get_start_iter(buffer, &start);
	gtk_text_buffer_get_end_iter(buffer, &end);
	gtk_text_buffer_remove_tag(buffer, priv->invisible_tag, &start, &end);
	
	gtk_tree_path_free(priv->current_heading);
	priv->current_heading = path;
	
	/* If the user clicked on the title, show the entire source */
	if(depth == I7_HEADING_NONE) {
		/* we have now shown the entire source */
		gtk_action_set_sensitive(document->previous_section, FALSE);
		gtk_action_set_sensitive(document->next_section, FALSE);
		gtk_action_set_sensitive(document->decrease_restriction, FALSE);
		gtk_action_set_sensitive(document->entire_source, FALSE);
		gtk_text_buffer_place_cursor(buffer, &start);
		return;
	}
	
	gtk_action_set_sensitive(document->previous_section, TRUE);
	gtk_action_set_sensitive(document->decrease_restriction, TRUE);
	gtk_action_set_sensitive(document->entire_source, TRUE);
	
	GtkTextIter mid;
	gtk_text_buffer_get_iter_at_line(buffer, &mid, startline);
	gtk_text_buffer_apply_tag(buffer, priv->invisible_tag, &start, &mid);
	gtk_text_buffer_place_cursor(buffer, &mid);
	
	GtkTreeIter next_iter = iter;
	/* if there is a next heading at the current level, display until there */
	if(!gtk_tree_model_iter_next(headings, &next_iter))
		/* if not, then if there is no next heading at the same level, display 
		until the next heading one level out */
		if(!(gtk_tree_model_iter_parent(headings, &next_iter, &iter) 
			&& gtk_tree_model_iter_next(headings, &next_iter))) 
		{
			/* otherwise, there is no next heading, display until the end of the
			source text */
			gtk_action_set_sensitive(document->next_section, FALSE);
			return;
		}
	
	guint endline = 0;
	gtk_tree_model_get(headings, &next_iter, I7_HEADINGS_LINE, &endline, -1);
	/* the line should be counted from zero, and also we need to back up
	by one line so as not to display the heading */
	endline -= 2;
	gtk_text_buffer_get_iter_at_line(buffer, &mid, endline);
	gtk_text_buffer_apply_tag(buffer, priv->invisible_tag, &mid, &end);
	
	gtk_action_set_sensitive(document->next_section, TRUE);
}

GtkTreePath *
i7_document_get_previous_heading(I7Document *document)
{
	/* Don't need to use iters here since paths can go up or back */
	GtkTreePath *path = gtk_tree_path_copy(I7_DOCUMENT_PRIVATE(document)->current_heading);
	/* if there is no previous heading on this level, display the previous
	heading one level out */
	if(!gtk_tree_path_prev(path))
		if(gtk_tree_path_get_depth(path) > 1)
			gtk_tree_path_up(path);
	return path;
}

GtkTreePath *
i7_document_get_next_heading(I7Document *document)
{
	I7DocumentPrivate *priv = I7_DOCUMENT_PRIVATE(document);
	GtkTreeModel *headings = GTK_TREE_MODEL(priv->headings);
	GtkTreeIter iter;
	gtk_tree_model_get_iter(headings, &iter, priv->current_heading);
	GtkTreeIter next_iter = iter;
	/* if there is a next heading at the current level, display that */
	if(!gtk_tree_model_iter_next(headings, &next_iter))
		/* if not, then if there is no next heading at the same level, display 
		the next heading one level out */
		if(!(gtk_tree_model_iter_parent(headings, &next_iter, &iter) 
			&& gtk_tree_model_iter_next(headings, &next_iter)))
			/* otherwise, there is no next heading, display until the end of the
			source text */
			next_iter = iter;
	GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(priv->headings), &next_iter);
	return path;
}

GtkTreePath *
i7_document_get_shallower_heading(I7Document *document)
{
	/* Don't need to use iters here */
	GtkTreePath *path = gtk_tree_path_copy(I7_DOCUMENT_PRIVATE(document)->current_heading);
	if(gtk_tree_path_get_depth(path) > 1)
		gtk_tree_path_up(path);
	return path;
}

static guint
get_current_line_number(GtkTextBuffer *buffer)
{
	GtkTextMark *insert_mark = gtk_text_buffer_get_insert(buffer);
	GtkTextIter insert;
	gtk_text_buffer_get_iter_at_mark(buffer, &insert, insert_mark);
	return (guint)(gtk_text_iter_get_line(&insert) + 1);
}

GtkTreePath *
i7_document_get_deeper_heading(I7Document *document)
{
	I7DocumentPrivate *priv = I7_DOCUMENT_PRIVATE(document);
	GtkTreeModel *headings = GTK_TREE_MODEL(priv->headings);
	
	guint cur_line = get_current_line_number(GTK_TEXT_BUFFER(priv->buffer));
	GtkTreeIter iter, next_iter;
	guint line = 0;
	/* This perhaps won't deal quite as well with changes on the fly */
	gtk_tree_model_get_iter(headings, &iter, priv->current_heading);
	if(gtk_tree_model_iter_has_child(headings, &iter)) {
		gtk_tree_model_iter_nth_child(headings, &next_iter, &iter, 0);
		do {
			gtk_tree_model_get(headings, &next_iter, I7_HEADINGS_LINE, &line, -1);
			if(line > cur_line)
				break;
			iter = next_iter;
		} while(gtk_tree_model_iter_next(headings, &next_iter));
	}
	
	/* iter is the heading we want to go to; if it had no children, it stays
	the same */
	GtkTreePath *path = gtk_tree_model_get_path(headings, &iter);
	return path;
}

GtkTreePath *
i7_document_get_deepest_heading(I7Document *document)
{
	I7DocumentPrivate *priv = I7_DOCUMENT_PRIVATE(document);
	GtkTreeModel *headings = GTK_TREE_MODEL(priv->headings);

	guint cur_line = get_current_line_number(GTK_TEXT_BUFFER(priv->buffer));
	GtkTreeIter iter, next_iter;
	guint line = 0;
	/* Could start at current_heading to be more efficient, but starting at the
	root allows us to deal with changes that have occurred since the last
	navigation action in the headings pane */
	gtk_tree_model_get_iter_first(headings, &iter); /* line 0 */
	while(TRUE) {
		if(gtk_tree_model_iter_has_child(headings, &iter))
			gtk_tree_model_iter_nth_child(headings, &next_iter, &iter, 0);
		else {
			next_iter = iter;
			if(!gtk_tree_model_iter_next(headings, &next_iter) 
				&& !(gtk_tree_model_iter_parent(headings, &next_iter, &iter) 
				&& gtk_tree_model_iter_next(headings, &next_iter))) 
				/* We've reached the end */
				break;
		}
		gtk_tree_model_get(headings, &next_iter, I7_HEADINGS_LINE, &line, -1);
		if(line > cur_line)
			break;
		iter = next_iter;
	}
	
	/* Now iter is the heading we want to go to */
	GtkTreePath *path = gtk_tree_model_get_path(headings, &iter);
	return path;
}

/* Remove the invisible tag */
void
i7_document_show_entire_source(I7Document *document)
{
	I7DocumentPrivate *priv = I7_DOCUMENT_PRIVATE(document);
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(priv->buffer);
	
	GtkTextIter start, end;
	gtk_text_buffer_get_start_iter(buffer, &start);
	gtk_text_buffer_get_end_iter(buffer, &end);
	gtk_text_buffer_remove_tag(buffer, priv->invisible_tag, &start, &end);
	
	gtk_action_set_sensitive(document->previous_section, FALSE);
	gtk_action_set_sensitive(document->next_section, FALSE);
	gtk_action_set_sensitive(document->decrease_restriction, FALSE);
	gtk_action_set_sensitive(document->entire_source, FALSE);
	
	gtk_tree_path_free(priv->current_heading);
	priv->current_heading = gtk_tree_path_new_first();
}

/* Displays the message text in the status bar of the current window. */
void 
i7_document_display_status_message(I7Document *document, const gchar *message, const gchar *context) 
{
	GtkStatusbar *status = GTK_STATUSBAR(document->statusbar);
	guint id = gtk_statusbar_get_context_id(status, context);
	gtk_statusbar_pop(status, id);
	gtk_statusbar_push(status, id, message);
}

void
i7_document_remove_status_message(I7Document *document, const gchar *context)
{
	GtkStatusbar *status = GTK_STATUSBAR(document->statusbar);
	guint id = gtk_statusbar_get_context_id(status, context);
	gtk_statusbar_pop(status, id);
}

struct StatusData {
	GtkStatusbar *status;
	guint context_id;
	guint message_id;
};

static gboolean 
end_flash_message(struct StatusData *data)
{
	gtk_statusbar_remove(data->status, data->context_id, data->message_id);
	g_slice_free(struct StatusData, data);
	return FALSE;
}

void
i7_document_flash_status_message(I7Document *document, const gchar *message, const gchar *context)
{
	struct StatusData *data = g_slice_new0(struct StatusData);
	data->status = GTK_STATUSBAR(document->statusbar);
	data->context_id = gtk_statusbar_get_context_id(data->status, context);
	gtk_statusbar_pop(data->status, data->context_id);
	data->message_id = gtk_statusbar_push(data->status, data->context_id, message);
	g_timeout_add_seconds(1, (GSourceFunc)end_flash_message, data);
}

/* Pulses the progress bar */
void 
i7_document_display_progress_busy(I7Document *document) 
{
    gtk_progress_bar_pulse(GTK_PROGRESS_BAR(document->progressbar));
}

/* Displays a percentage in the progress indicator */
void 
i7_document_display_progress_percentage(I7Document *document, gdouble fraction) 
{
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(document->progressbar), fraction);
}

/* Displays a message in the progress indicator */
void
i7_document_display_progress_message(I7Document *document, const gchar *message)
{
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(document->progressbar), message);
}

/* Clears the message and progress percentage */
void
i7_document_clear_progress(I7Document *document)
{
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(document->progressbar), 0.0);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(document->progressbar), NULL);
}

static void
on_menu_item_select(GtkItem *item, GtkStatusbar *statusbar)
{
	gchar *hint = NULL;
	g_object_get(gtk_widget_get_action(GTK_WIDGET(item)), "tooltip", &hint, NULL);
	if(hint) {
		guint id = gtk_statusbar_get_context_id(statusbar, "MenuItemHints");
		gtk_statusbar_push(statusbar, id, hint);
		g_free(hint);
	}
}

static void
on_menu_item_deselect(GtkItem *item, GtkStatusbar *statusbar)
{
	guint id = gtk_statusbar_get_context_id(statusbar, "MenuItemHints");
	gtk_statusbar_pop(statusbar, id);
}

/* Helper function to attach the menu tooltips of widget to the statusbar */
static void
attach_menu_hints(GtkWidget *widget, gpointer statusbar)
{
	if(GTK_IS_MENU_ITEM(widget)) {
		GtkWidget *submenu = gtk_menu_item_get_submenu(GTK_MENU_ITEM(widget));
		if(submenu)
			gtk_container_foreach(GTK_CONTAINER(submenu), attach_menu_hints, statusbar);
		g_signal_connect(widget, "select", G_CALLBACK(on_menu_item_select), statusbar);
		g_signal_connect(widget, "deselect", G_CALLBACK(on_menu_item_deselect), statusbar);
	}
}

void
i7_document_attach_menu_hints(I7Document *document, GtkMenuBar *menu)
{
	gtk_container_foreach(GTK_CONTAINER(menu), attach_menu_hints, GTK_STATUSBAR(document->statusbar));
}

void 
i7_document_set_spellcheck(I7Document *document, gboolean spellcheck)
{
	I7_DOCUMENT_GET_CLASS(document)->set_spellcheck(document, spellcheck);
}

void 
i7_document_check_spelling(I7Document *document)
{
	I7_DOCUMENT_GET_CLASS(document)->check_spelling(document);
}

void
i7_document_set_elastic_tabs(I7Document *document, gboolean elastic)
{
	gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(document->enable_elastic_tabs), elastic);
	I7_DOCUMENT_GET_CLASS(document)->set_elastic_tabs(document, elastic);
}

