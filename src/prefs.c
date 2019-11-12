/* Copyright (C) 2006-2009, 2010, 2011, 2012, 2013 P. F. Chimento
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

#include "config.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>

#include "app.h"
#include "builder.h"
#include "configfile.h"
#include "error.h"
#include "prefs.h"

#define DOMAIN_FOR_GTKSOURCEVIEW_COLOR_SCHEMES "gtksourceview-3.0"

enum SchemesListColumns {
	ID_COLUMN = 0,
	NAME_COLUMN,
	DESC_COLUMN,
	NUM_SCHEMES_LIST_COLUMNS
};

/* Helper function: enumeration callback for each color scheme */
static void
store_color_scheme(GtkSourceStyleScheme *scheme, GtkListStore *list)
{
	const char *id = gtk_source_style_scheme_get_id(scheme);
	const char *name = gtk_source_style_scheme_get_name(scheme);
	const char *description = gtk_source_style_scheme_get_description(scheme);

	/* We pick up system color schemes as well. These won't have translations in
	the gnome-inform7 domain, so if we can't get a translation then we try it
	again in GtkSourceView's translation domain. */
	const char *try_name = gettext(name);
	if (try_name == name) {  /* Pointer equality, not strcmp */
		char *save_domain = g_strdup(textdomain(NULL));
		textdomain(DOMAIN_FOR_GTKSOURCEVIEW_COLOR_SCHEMES);
		bind_textdomain_codeset(DOMAIN_FOR_GTKSOURCEVIEW_COLOR_SCHEMES, "UTF-8");
		name = gettext(name);
		description = gettext(description);
		textdomain(save_domain);
		g_free(save_domain);
	} else {
		name = try_name;
		description = gettext(description);
	}

	GtkTreeIter iter;
	gtk_list_store_append(list, &iter);
	gtk_list_store_set(list, &iter,
		ID_COLUMN, id,
		NAME_COLUMN, name,
		DESC_COLUMN, description,
		-1);
}

void
populate_schemes_list(GtkListStore *list)
{
	I7App *theapp = i7_app_get();
	GSettings *prefs = i7_app_get_prefs(theapp);
	gtk_list_store_clear(list);
	i7_app_foreach_color_scheme(theapp, (GFunc)store_color_scheme, list);
	select_style_scheme(theapp->prefs->schemes_view, g_settings_get_string(prefs, PREFS_STYLE_SCHEME));
}

I7PrefsWidgets *
create_prefs_window(GSettings *prefs, GtkBuilder *builder)
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

	/* Bind widgets to GSettings */
#define BIND(key, obj, property) \
	g_settings_bind(prefs, key, load_object(builder, obj), property, \
		G_SETTINGS_BIND_DEFAULT | G_SETTINGS_BIND_NO_SENSITIVITY)
#define BIND_COMBO_BOX(key, obj, enum_values) \
	g_settings_bind_with_mapping(prefs, key, \
		load_object(builder, obj), "active", \
		G_SETTINGS_BIND_DEFAULT | G_SETTINGS_BIND_NO_SENSITIVITY, \
		(GSettingsBindGetMapping)settings_enum_get_mapping, \
		(GSettingsBindSetMapping)settings_enum_set_mapping, \
		enum_values, NULL)
	BIND(PREFS_AUTHOR_NAME, "author_name", "text");
	BIND(PREFS_CUSTOM_FONT, "custom_font", "font-name");
	BIND(PREFS_SYNTAX_HIGHLIGHTING, "enable_highlighting", "active");
	BIND(PREFS_AUTO_INDENT, "auto_indent", "active");
	BIND(PREFS_INDENT_WRAPPED, "indent_wrapped", "active");
	BIND(PREFS_INTELLIGENCE, "follow_symbols", "active");
	BIND(PREFS_INTELLIGENCE, "auto_number", "sensitive");
	BIND(PREFS_AUTO_NUMBER, "auto_number", "active");
	BIND(PREFS_CLEAN_BUILD_FILES, "clean_build_files", "active");
	BIND(PREFS_CLEAN_BUILD_FILES, "clean_index_files", "sensitive");
	BIND(PREFS_CLEAN_INDEX_FILES, "clean_index_files", "active");
	BIND(PREFS_SHOW_DEBUG_LOG, "show_debug_tabs", "active");
	BIND_COMBO_BOX(PREFS_FONT_SET, "font_set", font_set_enum);
	BIND_COMBO_BOX(PREFS_FONT_SIZE, "font_size", font_size_enum);
	BIND_COMBO_BOX(PREFS_INTERPRETER, "glulx_combo", interpreter_enum);
#undef BIND
#undef BIND_COMBO_BOX
	g_settings_bind(prefs, PREFS_TAB_WIDTH,
		gtk_range_get_adjustment(GTK_RANGE(load_object(builder, "tab_ruler"))), "value", G_SETTINGS_BIND_DEFAULT);

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
	select_style_scheme(self->schemes_view, g_settings_get_string(prefs, PREFS_STYLE_SCHEME));

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
		I7App *theapp = i7_app_get();
		GSettings *prefs = i7_app_get_prefs(theapp);
		gchar *id;
		gtk_tree_model_get(model, &iter, ID_COLUMN, &id, -1);
		g_settings_set_string(prefs, PREFS_STYLE_SCHEME, id);
		gtk_widget_set_sensitive(app->prefs->style_remove, id && i7_app_color_scheme_is_user_scheme(i7_app_get(), id));
		g_free(id);
	} else {
		; /* Do nothing; no selection */
	}
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

	GFile *file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(chooser));
	if(!file)
		return;

	gtk_widget_destroy(chooser);

	const char *scheme_id = i7_app_install_color_scheme(app, file);
	g_object_unref(file);

	if(!scheme_id) {
		error_dialog(GTK_WINDOW(app->prefs->window), NULL, _("The selected color scheme cannot be installed."));
		return;
	}

	populate_schemes_list(app->prefs->schemes_list);

	I7App *theapp = i7_app_get();
	GSettings *prefs = i7_app_get_prefs(theapp);
	g_settings_set_string(prefs, PREFS_STYLE_SCHEME, scheme_id);
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

		if(!i7_app_uninstall_color_scheme(app, id))
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

			I7App *theapp = i7_app_get();
			GSettings *prefs = i7_app_get_prefs(theapp);
			g_settings_set(prefs, PREFS_STYLE_SCHEME, new_id);

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
	for(iter = gdk_drag_context_list_targets(drag_context); iter != NULL; iter = g_list_next(iter)) {
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
	GFile *file;
	GFileInfo *file_info;
	gchar *type_name = NULL;

	/* Check that we got data from source */
	if(selectiondata == NULL || gtk_selection_data_get_length(selectiondata) < 0)
		goto fail;

	/* Check that we got the format we can use */
	type_name = gdk_atom_name(gtk_selection_data_get_data_type(selectiondata));
	if(strcmp(type_name, "text/uri-list") != 0)
		goto fail;

	/* Do stuff with the data */
	char **extension_files = g_uri_list_extract_uris((char *)gtk_selection_data_get_data(selectiondata));
	int foo;
	/* Get a list of URIs to the dropped files */
	for(foo = 0; extension_files[foo] != NULL; foo++) {
		GError *err = NULL;
		file = g_file_new_for_uri(extension_files[foo]);
		file_info = g_file_query_info(file, G_FILE_ATTRIBUTE_STANDARD_TYPE, G_FILE_QUERY_INFO_NONE, NULL, &err);
		if(!file_info) {
			IO_ERROR_DIALOG(NULL, file, err, _("accessing a URI"));
			goto fail2;
		}

		/* Check whether a directory was dropped. if so, install contents */
		/* NOTE: not recursive (that would be kind of silly anyway) */
		if(g_file_info_get_file_type(file_info) == G_FILE_TYPE_DIRECTORY) {

			GFileEnumerator *dir = g_file_enumerate_children(file, "standard::*", G_FILE_QUERY_INFO_NONE, NULL, &err);
			if(!dir) {
				IO_ERROR_DIALOG(NULL, file, err, _("opening a directory"));
				goto fail3;
			}

			GFileInfo *entry_info;
			while((entry_info = g_file_enumerator_next_file(dir, NULL, &err)) != NULL) {
				if(g_file_info_get_file_type(entry_info) != G_FILE_TYPE_DIRECTORY) {
					GFile *extension_file = g_file_get_child(file, g_file_info_get_name(entry_info));
					i7_app_install_extension(i7_app_get(), extension_file);
					g_object_unref(extension_file);
				}
				g_object_unref(entry_info);
			}
			g_file_enumerator_close(dir, NULL, &err);
			g_object_unref(dir);

			if(err) {
				IO_ERROR_DIALOG(NULL, file, err, _("reading a directory"));
				goto fail3;
			}

		} else {
			/* just install it */
			i7_app_install_extension(i7_app_get(), file);
		}

		g_object_unref(file_info);
		g_object_unref(file);
	}

	g_strfreev(extension_files);
	g_free(type_name);
	gtk_drag_finish(drag_context, TRUE, FALSE, time);
	return;

fail3:
	g_object_unref(file_info);
fail2:
	g_object_unref(file);
	g_strfreev(extension_files);
fail:
	g_free(type_name);
	gtk_drag_finish(drag_context, FALSE, FALSE, time);
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
install_extensions(GFile *file, I7App *app)
{
	i7_app_install_extension(app, file);
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
	GSList *extlist = gtk_file_chooser_get_files(GTK_FILE_CHOOSER(dialog));
	g_slist_foreach(extlist, (GFunc)install_extensions, app);

	/* Free stuff */
	g_slist_foreach(extlist, (GFunc)g_object_unref, NULL);
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

/* Update the highlighting styles for this buffer */
gboolean
update_style(GtkSourceBuffer *buffer)
{
	gtk_source_buffer_set_style_scheme(buffer, i7_app_get_current_color_scheme(i7_app_get()));
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
	I7App *theapp = i7_app_get();
	GSettings *prefs = i7_app_get_prefs(theapp);
	unsigned spaces = g_settings_get_uint(prefs, PREFS_TAB_WIDTH);
	if(spaces == 0)
		spaces = DEFAULT_TAB_WIDTH;
	gtk_source_view_set_tab_width(view, spaces);

	/* Set the hanging indent on wrapped lines to be a number of pixels equal
	 * to twice the number of spaces in a tab; i.e. we estimate a space to be
	 * four pixels. Not always true, but close enough.*/
	gboolean indent_wrapped = g_settings_get_boolean(prefs, PREFS_INDENT_WRAPPED);
	if(!indent_wrapped)
		spaces = 0;
	g_object_set(view,
	    "indent", -2 * spaces,
	    NULL);

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
