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
#include "lang.h"
#include "prefs.h"

#define DOMAIN_FOR_GTKSOURCEVIEW_COLOR_SCHEMES "gtksourceview-4"

enum SchemesListColumns {
	ID_COLUMN = 0,
	NAME_COLUMN,
	DESC_COLUMN,
	NUM_SCHEMES_LIST_COLUMNS
};

const char *font_set_enum[] = { "Standard", "Monospace", "Custom", NULL };
const char *font_size_enum[] = { "Standard", "Medium", "Large", "Huge", NULL };
const char *interpreter_enum[] = { "Glulxe (default)", "Git", NULL };

struct _I7PrefsWindow {
	GtkDialog parent;

	/* template children */
	GtkEntry *author_name;
	GtkCheckButton *auto_indent;
	GtkCheckButton *clean_build_files;
	GtkFontButton *custom_font;
	GtkCheckButton *enable_highlighting;
	GtkComboBoxText *font_set;
	GtkComboBoxText *font_size;
	GtkComboBoxText *glulx_combo;
	GtkWidget *prefs_notebook;
	GtkTreeView *schemes_view;
	GtkCheckButton *show_debug_tabs;
	GtkWidget *style_remove;
	GtkSourceView *tab_example;
	GtkScale *tab_ruler;
	GtkSourceView *source_example;
	GtkWidget *auto_number;
	GtkWidget *clean_index_files;
	GtkListStore *schemes_list;
};

G_DEFINE_TYPE(I7PrefsWindow, i7_prefs_window, GTK_TYPE_DIALOG);

static void select_style_scheme(GtkTreeView *view, const char *id);

/* PRIVATE METHODS */

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

static void
populate_schemes_list(I7PrefsWindow *self, I7App *theapp)
{
	gtk_list_store_clear(self->schemes_list);
	i7_app_foreach_color_scheme(theapp, (GFunc)store_color_scheme, self->schemes_list);
}

/*
 * settings_enum_set_mapping:
 * @property_value: value of the object property the setting is bound to.
 * @expected_type: GVariant type the setting expects.
 * @enum_values: an array of strings with %NULL as a sentinel at the end.
 *
 * Custom mapping function for setting combo boxes from enum GSettings keys.
 *
 * Returns: the #GVariant for the setting, or %NULL on failure.
 */
static GVariant *
settings_enum_set_mapping(const GValue *property_value, const GVariantType *expected_type, char **enum_values)
{
	int count = 0, index;

	g_assert(g_variant_type_equal(expected_type, G_VARIANT_TYPE_STRING));

	/* Count the number of values */
	while(enum_values[count])
		count++;

	index = g_value_get_int(property_value);
	if(index >= count)
		return NULL;
	return g_variant_new_string(enum_values[index]);
}

/*
 * settings_enum_get_mapping:
 * @value: value for the object property, initialized to hold the proper type
 * @settings_variant: value of the setting as a #GVariant
 * @enum_values: an array of strings with %NULL as a sentinel at the end.
 *
 * Custom mapping function for setting combo boxes from enum GSettings keys.
 *
 * Returns: %TRUE if the conversion succeeded, %FALSE otherwise.
 */
static gboolean
settings_enum_get_mapping(GValue *value, GVariant *settings_variant, char **enum_values)
{
	const char *settings_string = g_variant_get_string(settings_variant, NULL);
	int count;
	char **ptr;

	g_assert(G_VALUE_HOLDS_INT(value));

	for(count = 0, ptr = enum_values; *ptr; count++, ptr++) {
		if(strcmp(*ptr, settings_string) == 0)
			break;
	}
	if(*ptr == NULL)
		return FALSE;

	g_value_set_int(value, count);
	return TRUE;
}

/*
 * CALLBACKS
 */

static void
on_config_style_scheme_changed(GSettings *settings, const char *key, I7PrefsWindow *self)
{
	g_autofree char *newvalue = g_settings_get_string(settings, key);

	select_style_scheme(self->schemes_view, newvalue);
	update_style(GTK_SOURCE_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(self->source_example))));
}

static void
on_config_tab_width_changed(GSettings *settings, const char *key, I7PrefsWindow *self)
{
	unsigned newvalue = g_settings_get_uint(settings, key);

	/* Use default if set to 0, as per schema description */
	if (newvalue == 0)
		newvalue = DEFAULT_TAB_WIDTH;

	update_tabs(self->source_example);
}

static void
on_styles_list_cursor_changed(GtkTreeView *view, I7PrefsWindow *self)
{
	GtkTreeSelection *selection = gtk_tree_view_get_selection(view);
	GtkTreeIter iter;
	GtkTreeModel *model;
	if(gtk_tree_selection_get_selected(selection, &model, &iter)) {
		I7App *app = I7_APP(g_application_get_default());
		GSettings *prefs = i7_app_get_prefs(app);
		gchar *id;
		gtk_tree_model_get(model, &iter, ID_COLUMN, &id, -1);
		g_settings_set_string(prefs, PREFS_STYLE_SCHEME, id);
		gtk_widget_set_sensitive(self->style_remove, id && i7_app_color_scheme_is_user_scheme(app, id));
		g_free(id);
	} else {
		; /* Do nothing; no selection */
	}
}

static void
on_style_add_clicked(GtkButton *button, I7PrefsWindow *self)
{
	/* From gedit/dialogs/gedit-preferences-dialog.c */
	g_autoptr(GtkFileChooserNative) chooser = gtk_file_chooser_native_new(_("Add Color Scheme"),
		GTK_WINDOW(self), GTK_FILE_CHOOSER_ACTION_OPEN, _("_Add"), NULL);

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

	I7App *app = I7_APP(g_application_get_default());
	const char *scheme_id = i7_app_install_color_scheme(app, file);
	g_object_unref(file);

	if(!scheme_id) {
		error_dialog(GTK_WINDOW(self), NULL, _("The selected color scheme cannot be installed."));
		return;
	}

	populate_schemes_list(self, app);

	GSettings *prefs = i7_app_get_prefs(app);
	g_settings_set_string(prefs, PREFS_STYLE_SCHEME, scheme_id);
}

static void
on_style_remove_clicked(GtkButton *button, I7PrefsWindow *self)
{
	GtkTreeSelection *selection = gtk_tree_view_get_selection(self->schemes_view);
	GtkTreeModel *model;
	GtkTreeIter iter;
	if(gtk_tree_selection_get_selected(selection, &model, &iter)) {
		gchar *id;
		gchar *name;
		gtk_tree_model_get(model, &iter,
			ID_COLUMN, &id,
			NAME_COLUMN, &name,
			-1);

		I7App *app = I7_APP(g_application_get_default());

		if(!i7_app_uninstall_color_scheme(app, id))
			error_dialog(GTK_WINDOW(self), NULL, _("Could not remove color scheme \"%s\"."), name);
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

			populate_schemes_list(self, app);

			GSettings *prefs = i7_app_get_prefs(app);
			g_settings_set(prefs, PREFS_STYLE_SCHEME, new_id);

			g_free(new_id);
		}
		g_free(id);
		g_free(name);
	}
}

static char *
on_tab_ruler_format_value(GtkScale *scale, double value, I7PrefsWindow *self)
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

static void
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

/* TYPE SYSTEM */

static void
i7_prefs_window_init(I7PrefsWindow *self)
{
	gtk_widget_init_template(GTK_WIDGET(self));
}

static void
i7_prefs_window_constructed(GObject* object)
{
	I7PrefsWindow *self = I7_PREFS_WINDOW(object);

	I7App *theapp = I7_APP(g_application_get_default());

	populate_schemes_list(self, theapp);

	/* Set up Natural Inform highlighting on the example buffer */
	GtkSourceBuffer *buffer = GTK_SOURCE_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(self->source_example)));
	set_buffer_language(buffer, "inform7");
	gtk_source_buffer_set_style_scheme(buffer, i7_app_get_current_color_scheme(theapp));

	/* Do the style scheme list */
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(self->schemes_list), 0, GTK_SORT_ASCENDING);
	GtkTreeSelection *select = gtk_tree_view_get_selection(self->schemes_view);
	gtk_tree_selection_set_mode(select, GTK_SELECTION_BROWSE);

	G_OBJECT_CLASS(i7_prefs_window_parent_class)->constructed(object);
}

static void
i7_prefs_window_class_init(I7PrefsWindowClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	gtk_widget_class_set_template_from_resource(widget_class, "/com/inform7/IDE/ui/prefs.ui");
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, author_name);
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, auto_indent);
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, auto_number);
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, clean_build_files);
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, clean_index_files);
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, custom_font);
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, enable_highlighting);
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, font_set);
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, font_size);
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, glulx_combo);
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, prefs_notebook);
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, schemes_list);
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, schemes_view);
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, show_debug_tabs);
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, source_example);
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, style_remove);
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, tab_example);
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, tab_ruler);
	gtk_widget_class_bind_template_callback(widget_class, gtk_widget_destroy);
	gtk_widget_class_bind_template_callback(widget_class, on_style_add_clicked);
	gtk_widget_class_bind_template_callback(widget_class, on_style_remove_clicked);
	gtk_widget_class_bind_template_callback(widget_class, on_styles_list_cursor_changed);
	gtk_widget_class_bind_template_callback(widget_class, on_tab_ruler_format_value);

	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->constructed = i7_prefs_window_constructed;
}

/* PUBLIC API */

I7PrefsWindow *
i7_prefs_window_new(void)
{
	return I7_PREFS_WINDOW(g_object_new(I7_TYPE_PREFS_WINDOW, NULL));
}

void
i7_prefs_window_bind_settings(I7PrefsWindow *self, GSettings *prefs)
{
	/* Bind widgets to GSettings */
#define BIND(key, member, property) \
	g_settings_bind(prefs, key, self->member, property, \
		G_SETTINGS_BIND_DEFAULT | G_SETTINGS_BIND_NO_SENSITIVITY)
#define BIND_COMBO_BOX(key, member, enum_values) \
	g_settings_bind_with_mapping(prefs, key, \
		self->member, "active", \
		G_SETTINGS_BIND_DEFAULT | G_SETTINGS_BIND_NO_SENSITIVITY, \
		(GSettingsBindGetMapping)settings_enum_get_mapping, \
		(GSettingsBindSetMapping)settings_enum_set_mapping, \
		enum_values, NULL)
	BIND(PREFS_AUTHOR_NAME, author_name, "text");
	BIND(PREFS_CUSTOM_FONT, custom_font, "font-name");
	BIND(PREFS_SYNTAX_HIGHLIGHTING, enable_highlighting, "active");
	BIND(PREFS_AUTO_INDENT, auto_indent, "active");
	BIND(PREFS_AUTO_NUMBER, auto_number, "active");
	BIND(PREFS_CLEAN_BUILD_FILES, clean_build_files, "active");
	BIND(PREFS_CLEAN_BUILD_FILES, clean_index_files, "sensitive");
	BIND(PREFS_CLEAN_INDEX_FILES, clean_index_files, "active");
	BIND(PREFS_SHOW_DEBUG_LOG, show_debug_tabs, "active");
	BIND_COMBO_BOX(PREFS_FONT_SET, font_set, font_set_enum);
	BIND_COMBO_BOX(PREFS_FONT_SIZE, font_size, font_size_enum);
	BIND_COMBO_BOX(PREFS_INTERPRETER, glulx_combo, interpreter_enum);
#undef BIND
#undef BIND_COMBO_BOX
	g_settings_bind(prefs, PREFS_TAB_WIDTH,
		gtk_range_get_adjustment(GTK_RANGE(self->tab_ruler)), "value", G_SETTINGS_BIND_DEFAULT);

	g_signal_connect(prefs, "changed::" PREFS_STYLE_SCHEME, G_CALLBACK(on_config_style_scheme_changed), self);
	g_signal_connect(prefs, "changed::" PREFS_TAB_WIDTH, G_CALLBACK(on_config_tab_width_changed), self);

	select_style_scheme(self->schemes_view, g_settings_get_string(prefs, PREFS_STYLE_SCHEME));
}
