/* Copyright (C) 2006-2009, 2010 P. F. Chimento
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

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksourceview.h>
#include "prefs.h"
#include "app.h"
#include "builder.h"
#include "colorscheme.h"
#include "configfile.h"
#include "error.h"

enum SchemesListColumns {
	ID_COLUMN = 0,
	NAME_COLUMN,
	DESC_COLUMN,
	NUM_SCHEMES_LIST_COLUMNS
};

void
populate_schemes_list(GtkListStore *list)
{
	gtk_list_store_clear(list);
	GSList *schemes = get_style_schemes_sorted();
	GSList *l = schemes;
	for(l = schemes; l != NULL; l = g_slist_next(l)) {
		GtkSourceStyleScheme *scheme = GTK_SOURCE_STYLE_SCHEME(l->data);
		const gchar *id = gtk_source_style_scheme_get_id(scheme);
		const gchar *name = gtk_source_style_scheme_get_name(scheme);
		const gchar *description = gtk_source_style_scheme_get_description(scheme);

		GtkTreeIter iter;
		gtk_list_store_append(list, &iter);
		gtk_list_store_set(list, &iter,
			ID_COLUMN, id,
			NAME_COLUMN, name,
			DESC_COLUMN, description,
			-1);
	}
	g_slist_free(schemes);
}

I7PrefsWidgets *
create_prefs_window(GtkBuilder *builder)
{
	I7PrefsWidgets *self = g_slice_new0(I7PrefsWidgets);

	self->window = GTK_WIDGET(load_object(builder, "prefs_dialog"));
	self->prefs_notebook = GTK_WIDGET(load_object(builder, "prefs_notebook"));
	self->schemes_view = GTK_TREE_VIEW(load_object(builder, "schemes_view"));
	self->style_remove = GTK_WIDGET(load_object(builder, "style_remove"));
	self->tab_example = GTK_SOURCE_VIEW(load_object(builder, "tab_example"));
	self->source_example = GTK_SOURCE_VIEW(load_object(builder, "source_example"));
	self->extensions_view = GTK_TREE_VIEW(load_object(builder, "extensions_view"));
	self->extensions_add = GTK_WIDGET(load_object(builder, "extensions_add"));
	self->extensions_remove = GTK_WIDGET(load_object(builder, "extensions_remove"));
	self->auto_number = GTK_WIDGET(load_object(builder, "auto_number"));
	self->clean_index_files = GTK_WIDGET(load_object(builder, "clean_index_files"));
	self->schemes_list = GTK_LIST_STORE(load_object(builder, "schemes_list"));

	/* Only select one extension at a time */
	GtkTreeSelection *select = gtk_tree_view_get_selection(self->extensions_view);
	gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);

	/* Connect the drag'n'drop stuff */
	gtk_drag_dest_set(GTK_WIDGET(self->extensions_view),
		GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_HIGHLIGHT,
		NULL, 0, GDK_ACTION_COPY);
	/* For some reason GTK_DEST_DEFAULT_DROP causes two copies of every file to
	be sent to the widget when dropped, so we omit that. Also,
	GTK_DEST_DEFAULT_HIGHLIGHT seems to be broken. Including it anyway. */
	gtk_drag_dest_add_uri_targets(GTK_WIDGET(self->extensions_view));

	/* Do the style scheme list */
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(self->schemes_list), 0, GTK_SORT_ASCENDING);
	select = gtk_tree_view_get_selection(self->schemes_view);
	gtk_tree_selection_set_mode(select, GTK_SELECTION_BROWSE);

	return self;
}

/*
 * CALLBACKS
 */

void
on_styles_list_cursor_changed(GtkTreeView *view, I7App *app)
{
	GtkTreeSelection *selection = gtk_tree_view_get_selection(view);
	GtkTreeIter iter;
	GtkTreeModel *model;
	if(gtk_tree_selection_get_selected(selection, &model, &iter)) {
		gchar *id;
		gtk_tree_model_get(model, &iter, ID_COLUMN, &id, -1);
		config_file_set_string(PREFS_STYLE_SCHEME, id);
		gtk_widget_set_sensitive(app->prefs->style_remove, id && is_user_scheme(id));
		g_free(id);
	} else
		; /* Do nothing; no selection */
}

void
on_style_add_clicked(GtkButton *button, I7App *app)
{
	/* From gedit/dialogs/gedit-preferences-dialog.c */
	GtkWidget *chooser = gtk_file_chooser_dialog_new(_("Add Color Scheme"),
		GTK_WINDOW(app->prefs->window),	GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_ADD, GTK_RESPONSE_ACCEPT,
		NULL);
	gtk_window_set_destroy_with_parent(GTK_WINDOW(chooser), TRUE);

	/* Filters */
	GtkFileFilter *filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, _("Color Scheme Files"));
	gtk_file_filter_add_pattern(filter, "*.xml");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), filter);
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(chooser), filter);

	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, _("All Files"));
	gtk_file_filter_add_pattern(filter, "*");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), filter);

	gtk_dialog_set_default_response(GTK_DIALOG(chooser), GTK_RESPONSE_ACCEPT);

	if(gtk_dialog_run(GTK_DIALOG(chooser)) != GTK_RESPONSE_ACCEPT) {
		gtk_widget_destroy(chooser);
		return;
	}

	gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));
	if(!filename)
		return;

	gtk_widget_destroy(chooser);

	const gchar *scheme_id = install_scheme(filename);
	g_free(filename);

	if(!scheme_id) {
		error_dialog(GTK_WINDOW(app->prefs->window), NULL, _("The selected color scheme cannot be installed."));
		return;
	}

	populate_schemes_list(app->prefs->schemes_list);
	config_file_set_string(PREFS_STYLE_SCHEME, scheme_id);
}

void
on_style_remove_clicked(GtkButton *button, I7App *app)
{
	GtkTreeSelection *selection = gtk_tree_view_get_selection(app->prefs->schemes_view);
	GtkTreeModel *model;
	GtkTreeIter iter;
	if(gtk_tree_selection_get_selected(selection, &model, &iter)) {
		gchar *id;
		gchar *name;
		gtk_tree_model_get(model, &iter,
			ID_COLUMN, &id,
			NAME_COLUMN, &name,
			-1);

		if(!uninstall_scheme(id))
			error_dialog(GTK_WINDOW(app->prefs->window), NULL, _("Could not remove color scheme \"%s\"."), name);
		else {
			gchar *new_id = NULL;
			GtkTreeIter new_iter;
			gboolean new_iter_set = FALSE;

			/* If the removed style scheme is the last of the list, set as new
			 default style scheme the previous one, otherwise set the next one.
			 To make this possible, we need to get the id of the new default
			 style scheme before re-populating the list. */
			GtkTreePath *path = gtk_tree_model_get_path(model, &iter);
			/* Try to move to the next path */
			gtk_tree_path_next(path);
			if(!gtk_tree_model_get_iter(model, &new_iter, path)) {
				/* It seems the removed style scheme was the last of the list.
				 Try to move to the previous one */
				gtk_tree_path_free(path);
				path = gtk_tree_model_get_path(model, &iter);
				gtk_tree_path_prev(path);
				if(gtk_tree_model_get_iter(model, &new_iter, path))
					new_iter_set = TRUE;
			}
			else
				new_iter_set = TRUE;
			gtk_tree_path_free(path);

			if(new_iter_set)
				gtk_tree_model_get(model, &new_iter,
					ID_COLUMN, &new_id,
					-1);

			if(!new_id)
				new_id = g_strdup("inform");

			populate_schemes_list(app->prefs->schemes_list);
			config_file_set_string(PREFS_STYLE_SCHEME, new_id);
			g_free(new_id);
		}
		g_free(id);
		g_free(name);
	}
}

gboolean
on_extensions_view_drag_drop(GtkWidget *widget, GdkDragContext *drag_context, gint x, gint y, guint time)
{
	/* Iterate through the list of target types provided by the source */
	GdkAtom target_type = NULL;
	GList *iter;
	for(iter = drag_context->targets; iter != NULL; iter = g_list_next(iter)) {
		gchar *type_name = gdk_atom_name(GDK_POINTER_TO_ATOM(iter->data));
		/* Select 'text/uri-list' from the list of available targets */
		if(!strcmp(type_name, "text/uri-list")) {
			g_free(type_name);
			target_type = GDK_POINTER_TO_ATOM(iter->data);
			break;
		}
		g_free(type_name);
	}
	/* If URI list not supported, then cancel */
	if(!target_type)
		return FALSE;

	/* Request the data from the source. */
	gtk_drag_get_data(
	  widget,         /* this widget, which will get 'drag-data-received' */
	  drag_context,   /* represents the current state of the DnD */
	  target_type,    /* the target type we want */
	  time);            /* time stamp */
	return TRUE;
}


void
on_extensions_view_drag_data_received(GtkWidget *widget, GdkDragContext *drag_context, gint x, gint y, GtkSelectionData *selectiondata, guint info, guint time)
{
	gboolean dnd_success = TRUE;
	gchar *type_name = NULL;

	/* Check that we got data from source */
	if((selectiondata == NULL) || (selectiondata->length < 0))
		dnd_success = FALSE;

	/* Check that we got the format we can use */
	else if(strcmp((type_name = gdk_atom_name(selectiondata->type)), "text/uri-list"))
		dnd_success = FALSE;

	else {
		/* Do stuff with the data */
		gchar **extension_files = g_uri_list_extract_uris((gchar *)selectiondata->data);
		int foo;
		/* Get a list of URIs to the dropped files */
		for(foo = 0; extension_files[foo] != NULL; foo++) {
			GError *err = NULL;
			gchar *filename = g_filename_from_uri(extension_files[foo], NULL, &err);
			if(!filename) {
				WARN(_("Invalid URI"), err);
				g_error_free(err);
				continue;
			}

			/* Check whether a directory was dropped. if so, install contents */
			/* NOTE: not recursive (that would be kind of silly anyway) */
			if(g_file_test(filename, G_FILE_TEST_IS_DIR)) {
				GDir *dir = g_dir_open(filename, 0, &err);
				if(err) {
					error_dialog(NULL, err, _("Error opening directory %s: "), filename);
					g_free(filename);
					return;
				}
				const gchar *dir_entry;
				while((dir_entry = g_dir_read_name(dir)) != NULL)
					if(!g_file_test(dir_entry, G_FILE_TEST_IS_DIR)) {
						gchar *entry_with_path = g_build_filename(filename, dir_entry, NULL);
						i7_app_install_extension(i7_app_get(), entry_with_path);
						g_free(entry_with_path);
					}
				g_dir_close(dir);

			} else
				/* just install it */
				i7_app_install_extension(i7_app_get(), filename);

			g_free(filename);
		}
		g_strfreev(extension_files);
	}

	if(type_name)
		g_free(type_name);
	gtk_drag_finish(drag_context, dnd_success, FALSE, time);
}


void
on_font_set_changed(GtkComboBox *combobox, I7App *app)
{
	config_file_set_enum(PREFS_FONT_SET, gtk_combo_box_get_active(combobox), font_set_lookup_table);
}

void
on_custom_font_font_set(GtkFontButton *button, I7App *app)
{
	config_file_set_string(PREFS_CUSTOM_FONT, gtk_font_button_get_font_name(button));
}

void
on_font_size_changed(GtkComboBox *combobox, I7App *app)
{
	config_file_set_enum(PREFS_FONT_SIZE, gtk_combo_box_get_active(combobox), font_size_lookup_table);
}

void
on_tab_ruler_value_changed(GtkRange *range, I7App *app)
{
	config_file_set_int(PREFS_TAB_WIDTH, (int)gtk_range_get_value(range));
}

gchar*
on_tab_ruler_format_value(GtkScale *scale, gdouble value, I7App *app)
{
	if(value)
		return g_strdup_printf(ngettext("1 space", "%.*f spaces", value), gtk_scale_get_digits(scale), value);
	return g_strdup("default");
}

/* Check whether the user has selected something (not an author name) that can
be removed, and if so, enable the remove button */
void
on_extensions_view_cursor_changed(GtkTreeView *view, I7App *app)
{
	GtkTreeSelection *selection = gtk_tree_view_get_selection(view);
	GtkTreeIter iter;
	GtkTreeModel *model;
	if(gtk_tree_selection_get_selected(selection, &model, &iter)) {
		gboolean readonly;
		gtk_tree_model_get(model, &iter,
			I7_APP_EXTENSION_READ_ONLY, &readonly,
			-1);
		gtk_widget_set_sensitive(app->prefs->extensions_remove, !readonly);
		/* Only enable the "Remove" button if the selection is not read only */
	} else
		gtk_widget_set_sensitive(app->prefs->extensions_remove, FALSE);
		/* if there is no selection */
}

/* Convenience function */
static void
install_extensions(const gchar *filename, I7App *app)
{
	i7_app_install_extension(app, filename);
}

void
on_extensions_add_clicked(GtkButton *button, I7App *app)
{
	GtkWidget *dialog = gtk_file_chooser_dialog_new(
	  _("Select the extensions to install"),
	  GTK_WINDOW(app->prefs->window),
	  GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
	  GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);
	/* Create appropriate file filters */
	GtkFileFilter *filter1 = gtk_file_filter_new();
	gtk_file_filter_set_name(filter1, _("Inform 7 Extensions (*.i7x)"));
	gtk_file_filter_add_pattern(filter1, "*.i7x");
	GtkFileFilter *filter2 = gtk_file_filter_new();
	gtk_file_filter_set_name(filter2, _("All Files"));
	gtk_file_filter_add_pattern(filter2, "*");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter1);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter2);

	if(gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_ACCEPT) {
		gtk_widget_destroy(dialog);
		return;
	}

	/* Install each selected extension */
	GSList *extlist = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));
	g_slist_foreach(extlist, (GFunc)install_extensions, app);

	/* Free stuff */
	g_slist_foreach(extlist, (GFunc)g_free, NULL);
	g_slist_free(extlist);
	gtk_widget_destroy(dialog);
}

void
on_extensions_remove_clicked(GtkButton *button, I7App *app)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(app->prefs->extensions_view);
	if(!gtk_tree_selection_get_selected(selection, &model, &iter))
		return;

	gchar *extname, *author;
	gboolean readonly;
	GtkTreeIter parent;
	gtk_tree_model_get(model, &iter,
		I7_APP_EXTENSION_TEXT, &extname,
		I7_APP_EXTENSION_READ_ONLY, &readonly,
		-1);

	/* Bail out if this is a built-in extension or it doesn't have an author in the tree */
	if(readonly || !gtk_tree_model_iter_parent(model, &parent, &iter)) {
		g_free(extname);
		return;
	}

	gtk_tree_model_get(model, &parent, I7_APP_EXTENSION_TEXT, &author, -1);

	GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(app->prefs->window),
		GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
		/* TRANSLATORS: Are you sure you want to remove EXTENSION_NAME by AUTHOR_NAME?*/
		_("Are you sure you want to remove %s by %s?"), extname, author);
	gint result = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	/* Delete the extension and remove its directory if empty */
	if(result == GTK_RESPONSE_YES)
		i7_app_delete_extension(app, author, extname);

	g_free(extname);
	g_free(author);
}

void
on_enable_highlighting_toggled(GtkToggleButton *togglebutton, I7App *app)
{
	config_file_set_bool(PREFS_SYNTAX_HIGHLIGHTING, gtk_toggle_button_get_active(togglebutton));
}

void
on_follow_symbols_toggled(GtkToggleButton *togglebutton, I7App *app)
{
	config_file_set_bool(PREFS_INTELLIGENCE, gtk_toggle_button_get_active(togglebutton));
}

void
on_auto_indent_toggled(GtkToggleButton *togglebutton, I7App *app)
{
	config_file_set_bool(PREFS_AUTO_INDENT, gtk_toggle_button_get_active(togglebutton));
}

void
on_auto_number_toggled(GtkToggleButton *togglebutton, I7App *app)
{
	config_file_set_bool(PREFS_AUTO_NUMBER_SECTIONS, gtk_toggle_button_get_active(togglebutton));
}

void
on_author_name_changed(GtkEditable *editable, I7App *app)
{
	config_file_set_string(PREFS_AUTHOR_NAME, gtk_entry_get_text(GTK_ENTRY(editable)));
}

void
on_glulx_combo_changed(GtkComboBox *combobox, I7App *app)
{
	config_file_set_bool(PREFS_USE_GIT, gtk_combo_box_get_active(combobox) == 1);
}

void
on_clean_build_files_toggled(GtkToggleButton *togglebutton, I7App *app)
{
	config_file_set_bool(PREFS_CLEAN_BUILD_FILES, gtk_toggle_button_get_active(togglebutton));
}

void
on_clean_index_files_toggled(GtkToggleButton *togglebutton, I7App *app)
{
	config_file_set_bool(PREFS_CLEAN_INDEX_FILES, gtk_toggle_button_get_active(togglebutton));
}

void
on_show_debug_tabs_toggled(GtkToggleButton *togglebutton, I7App *app)
{
	config_file_set_bool(PREFS_DEBUG_LOG_VISIBLE, gtk_toggle_button_get_active(togglebutton));
}


/* Update the highlighting styles for this buffer */
gboolean
update_style(GtkSourceBuffer *buffer)
{
	set_highlight_styles(buffer);
	return FALSE; /* one-shot idle function */
}

/* Change the font that this widget uses */
gboolean
update_font(GtkWidget *widget)
{
	PangoFontDescription *font = get_font_description();
	gtk_widget_modify_font(widget, font);
	pango_font_description_free(font);

	return FALSE; /* one-shot idle function */
}

/* Update the tab stops for a GtkSourceView */
gboolean
update_tabs(GtkSourceView *view)
{
	gint spaces = config_file_get_int(PREFS_TAB_WIDTH);
	if(spaces == 0)
		spaces = DEFAULT_TAB_WIDTH;
	gtk_source_view_set_tab_width(view, spaces);

	return FALSE; /* one-shot idle function */
}

void
select_style_scheme(GtkTreeView *view, const gchar *id)
{
	GtkTreeModel *model = gtk_tree_view_get_model(view);
	GtkTreeIter iter;
	if(!gtk_tree_model_get_iter_first(model, &iter))
		return;
	gchar *style;
	while(TRUE) {
		gtk_tree_model_get(model, &iter, ID_COLUMN, &style, -1);
		if(strcmp(style, id) == 0) {
			g_free(style);
			break;
		}
		g_free(style);
		if(!gtk_tree_model_iter_next(model, &iter))
			return;
	}

	GtkTreeSelection *selection = gtk_tree_view_get_selection(view);
	gtk_tree_selection_select_iter(selection, &iter);
}
