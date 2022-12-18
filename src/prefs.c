/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2006-2013, 2019 Philip Chimento <philip.chimento@gmail.com>
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

#define DOMAIN_FOR_GTKSOURCEVIEW_COLOR_SCHEMES "gtksourceview-4"

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
	the inform7-ide domain, so if we can't get a translation then we try it
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
	I7App *theapp = I7_APP(g_application_get_default());
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

	/* Do the style scheme list */
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(self->schemes_list), 0, GTK_SORT_ASCENDING);
	GtkTreeSelection *select = gtk_tree_view_get_selection(self->schemes_view);
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
		GSettings *prefs = i7_app_get_prefs(app);
		gchar *id;
		gtk_tree_model_get(model, &iter, ID_COLUMN, &id, -1);
		g_settings_set_string(prefs, PREFS_STYLE_SCHEME, id);
		gtk_widget_set_sensitive(app->prefs->style_remove, id && i7_app_color_scheme_is_user_scheme(app, id));
		g_free(id);
	} else {
		; /* Do nothing; no selection */
	}
}

void
on_style_add_clicked(GtkButton *button, I7App *app)
{
	/* From gedit/dialogs/gedit-preferences-dialog.c */
	g_autoptr(GtkFileChooserNative) chooser = gtk_file_chooser_native_new(_("Add Color Scheme"),
		GTK_WINDOW(app->prefs->window),	GTK_FILE_CHOOSER_ACTION_OPEN, _("_Add"), NULL);

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

	if (gtk_native_dialog_run(GTK_NATIVE_DIALOG(chooser)) != GTK_RESPONSE_ACCEPT)
		return;

	GFile *file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(chooser));
	if(!file)
		return;

	const char *scheme_id = i7_app_install_color_scheme(app, file);
	g_object_unref(file);

	if(!scheme_id) {
		error_dialog(GTK_WINDOW(app->prefs->window), NULL, _("The selected color scheme cannot be installed."));
		return;
	}

	populate_schemes_list(app->prefs->schemes_list);

	GSettings *prefs = i7_app_get_prefs(app);
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
				new_id = g_strdup(DEFAULT_STYLE_SCHEME);

			populate_schemes_list(app->prefs->schemes_list);

			GSettings *prefs = i7_app_get_prefs(app);
			g_settings_set(prefs, PREFS_STYLE_SCHEME, new_id);

			g_free(new_id);
		}
		g_free(id);
		g_free(name);
	}
}

gchar*
on_tab_ruler_format_value(GtkScale *scale, gdouble value, I7App *app)
{
	if(value)
		return g_strdup_printf(ngettext("1 space", "%.*f spaces", value), gtk_scale_get_digits(scale), value);
	return g_strdup("default");
}

/* Update the highlighting styles for this buffer */
gboolean
update_style(GtkSourceBuffer *buffer)
{
	I7App *theapp = I7_APP(g_application_get_default());
	gtk_source_buffer_set_style_scheme(buffer, i7_app_get_current_color_scheme(theapp));
	return FALSE; /* one-shot idle function */
}

/* Update the tab stops for a GtkSourceView */
gboolean
update_tabs(GtkSourceView *view)
{
	I7App *theapp = I7_APP(g_application_get_default());
	GSettings *prefs = i7_app_get_prefs(theapp);
	unsigned spaces = g_settings_get_uint(prefs, PREFS_TAB_WIDTH);
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
