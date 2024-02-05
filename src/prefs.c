/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2006-2013, 2019 Philip Chimento <philip.chimento@gmail.com>
 */

#include "config.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>
#include <handy.h>

#include "app.h"
#include "builder.h"
#include "configfile.h"
#include "elastic.h"
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

#define FONT_STANDARD_STR "Standard"
#define FONT_CUSTOM_STR "Custom"
const char *font_size_enum[] = { "Smallest", "Smaller", "Small", "Medium", "Large", "Larger", "Largest", NULL };
const char *interpreter_enum[] = { "Glulxe (default)", "Git", NULL };

/* COMPAT: Use G_DEFINE_ENUM_TYPE in GLib >= 2.74 */
static GType
i7_prefs_font_size_get_type(void)
{
	static GType gtype = 0;
	if (g_once_init_enter(&gtype)) {
		static const GEnumValue values[] = {
			{ FONT_SIZE_SMALLEST, "FONT_SIZE_SMALLEST", "smallest" },
			{ FONT_SIZE_SMALLER, "FONT_SIZE_SMALLER", "smaller" },
			{ FONT_SIZE_SMALL, "FONT_SIZE_SMALL", "small" },
			{ FONT_SIZE_MEDIUM, "FONT_SIZE_MEDIUM", "medium" },
			{ FONT_SIZE_LARGE, "FONT_SIZE_LARGE", "large" },
			{ FONT_SIZE_LARGER, "FONT_SIZE_LARGER", "larger" },
			{ FONT_SIZE_LARGEST, "FONT_SIZE_LARGEST", "largest" },
			{ 0, NULL, NULL },
		};
		GType new_gtype = g_enum_register_static(g_intern_static_string("I7PrefsFontSize"), values);
		g_once_init_leave(&gtype, new_gtype);
	}
	return gtype;
}

/* COMPAT: Use G_DEFINE_ENUM_TYPE in GLib >= 2.74 */
static GType
i7_prefs_interpreter_get_type(void)
{
	static GType gtype = 0;
	if (g_once_init_enter(&gtype)) {
		static const GEnumValue values[] = {
			{ INTERPRETER_GLULXE, "INTERPRETER_GLULXE", "glulxe" },
			{ INTERPRETER_GIT, "INTERPRETER_GIT", "git" },
			{ 0, NULL, NULL },
		};
		GType new_gtype = g_enum_register_static(g_intern_static_string("I7PrefsInterpreter"), values);
		g_once_init_leave(&gtype, new_gtype);
	}
	return gtype;
}

struct _I7PrefsWindow {
	HdyPreferencesWindow parent;

	/* template children */
	GtkEntry *author_name;
	GtkSwitch *auto_indent;
	GtkSwitch *clean_build_files;
	HdyPreferencesGroup *color_group;
	HdyComboRow *color_scheme;
	GtkFontButton *custom_font;
	GtkWidget *style_remove;
	GtkSourceView *source_example;
	GtkWidget *auto_number;
	GtkWidget *clean_index_files;
	HdyComboRow *docs_font_size;
	GtkSwitch *elastic_tabs;
	GtkSwitch *enable_fonts;
	GtkSwitch *enable_highlighting;
	HdyPreferencesGroup *font_group;
	HdyComboRow *glulx_interpreter;
	GtkEntry *added_nest;
	GtkButton *restore_default_font;
	GtkSwitch *show_debug_tabs;
	GtkSpinButton *tab_width;

	/* private */
	GListStore *schemes_list;
};

G_DEFINE_TYPE(I7PrefsWindow, i7_prefs_window, HDY_TYPE_PREFERENCES_WINDOW);

/* PRIVATE METHODS */

/* Helper function: format name of color scheme for dropdown */
static char *
format_color_scheme_name(GtkSourceStyleScheme *scheme) {
	const char *name = gtk_source_style_scheme_get_name(scheme);

	/* We pick up system color schemes as well. These won't have translations in
	the inform7-ide domain, so if we can't get a translation then we try it
	again in GtkSourceView's translation domain. */
	const char *try_name = gettext(name);
	if (try_name == name) {  /* Pointer equality, not strcmp */
		char *save_domain = g_strdup(textdomain(NULL));
		textdomain(DOMAIN_FOR_GTKSOURCEVIEW_COLOR_SCHEMES);
		bind_textdomain_codeset(DOMAIN_FOR_GTKSOURCEVIEW_COLOR_SCHEMES, "UTF-8");
		name = gettext(name);
		textdomain(save_domain);
		g_free(save_domain);
		return g_strdup(name);
	}

	return g_strdup(try_name);
}

/* Helper function: enumeration callback for each color scheme */
static void
store_color_scheme(GtkSourceStyleScheme *scheme, GListStore *list)
{
	g_list_store_append(list, scheme);
}

static void
populate_schemes_list(I7PrefsWindow *self, I7App *theapp)
{
	g_object_freeze_notify(G_OBJECT(self->color_scheme));
	g_list_store_remove_all(self->schemes_list);
	i7_app_foreach_color_scheme(theapp, (GFunc)store_color_scheme, self->schemes_list);
	g_object_thaw_notify(G_OBJECT(self->color_scheme));
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

static gboolean
font_set_get_mapping(GValue *value, GVariant *settings_variant)
{
	g_assert(G_VALUE_HOLDS_BOOLEAN(value));

	const char *settings_string = g_variant_get_string(settings_variant, NULL);
	g_value_set_boolean(value, strcmp(settings_string, FONT_STANDARD_STR) != 0);
	return TRUE;  /* handled */
}

static GVariant *
font_set_set_mapping(const GValue *property_value, const GVariantType *expected_type)
{
	g_assert(g_variant_type_equal(expected_type, G_VARIANT_TYPE_STRING));

	bool enable_fonts = g_value_get_boolean(property_value);
	return g_variant_new_string(enable_fonts ? FONT_CUSTOM_STR : FONT_STANDARD_STR);
}

static void
select_style_scheme(I7PrefsWindow *self, const char *id)
{
	unsigned ix = 0;
	GtkSourceStyleScheme *scheme;
	while ((scheme = g_list_model_get_item(G_LIST_MODEL(self->schemes_list), ix)) != NULL) {
		const char *id_iter = gtk_source_style_scheme_get_id(scheme);
		if (strcmp(id_iter, id) == 0)
			break;
		ix++;
	};

	if (scheme == NULL)
		return;

	hdy_combo_row_set_selected_index(self->color_scheme, ix);
}

/*
 * CALLBACKS
 */

static void
on_color_scheme_selected_index_notify(HdyComboRow *combo, GParamSpec *pspec, I7PrefsWindow *self)
{
	I7App *app = I7_APP(g_application_get_default());
	GSettings *prefs = i7_app_get_prefs(app);

	int ix = hdy_combo_row_get_selected_index(combo);
	GtkSourceStyleScheme *scheme = g_list_model_get_item(G_LIST_MODEL(self->schemes_list), ix);
	const char *id = gtk_source_style_scheme_get_id(scheme);

	g_settings_set_string(prefs, PREFS_STYLE_SCHEME, id);
	gtk_widget_set_sensitive(self->style_remove, i7_app_color_scheme_is_user_scheme(app, id));
}

static void
on_config_elastic_tabstops_changed(GSettings *prefs, const char *key, I7PrefsWindow *self)
{
	if (g_settings_get_boolean(prefs, key)) {
		add_elastic_tabstops_to_view(GTK_TEXT_VIEW(self->source_example));
		elastic_recalculate_view(GTK_TEXT_VIEW(self->source_example));
	} else {
		remove_elastic_tabstops_from_view(GTK_TEXT_VIEW(self->source_example));
	}
}

static void
on_config_syntax_highlighting_changed(GSettings *settings, const char *key, I7PrefsWindow *self)
{
	bool active = g_settings_get_boolean(settings, key);
	gtk_source_buffer_set_highlight_syntax(GTK_SOURCE_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(self->source_example))), active);
}

static void
on_config_style_scheme_changed(GSettings *settings, const char *key, I7PrefsWindow *self)
{
	g_autofree char *newvalue = g_settings_get_string(settings, key);

	select_style_scheme(self, newvalue);
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
on_group_visibility_switch_active_notify(GtkSwitch *sw, GParamSpec *pspec, GtkWidget *group)
{
	bool active = gtk_switch_get_active(sw);
	gtk_widget_set_no_show_all(group, !active);
	if (active) {
		gtk_widget_show_all(group);
	} else {
		gtk_widget_hide(group);
	}
}

static void
on_restore_default_font_clicked(GtkButton *restore, I7PrefsWindow *self)
{
	I7App *app = I7_APP(g_application_get_default());
	GSettings *prefs = i7_app_get_prefs(app);

	g_settings_reset(prefs, PREFS_CUSTOM_FONT);
	g_settings_reset(prefs, PREFS_DOCS_FONT_SIZE);
}

static gboolean
on_source_example_button_press_event(GtkSourceView *source_example, GdkEvent *event, I7PrefsWindow *self)
{
	unsigned button;
	bool has_button = gdk_event_get_button(event, &button);
	g_assert(has_button && "wrong event passed to button-press-event");
	if (button == 3 /* right button */)
		return GDK_EVENT_STOP;  /* prevent context menu */
	return GDK_EVENT_PROPAGATE;
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
	I7App *app = I7_APP(g_application_get_default());

	int ix = hdy_combo_row_get_selected_index(self->color_scheme);
	GtkSourceStyleScheme *scheme = g_list_model_get_item(G_LIST_MODEL(self->schemes_list), ix);
	const char *id = gtk_source_style_scheme_get_id(scheme);
	const char *name = gtk_source_style_scheme_get_name(scheme);

	if (!i7_app_uninstall_color_scheme(app, id)) {
		error_dialog(GTK_WINDOW(self), NULL, _("Could not remove color scheme \"%s\"."), name);
		return;
	}

	/* If the removed style scheme is the last of the list, set as new style
	 * scheme the next one, otherwise set the previous one. To make this
	 * possible, we need to get the id of the new style scheme before
	 * repopulating the list. */
	unsigned n_schemes = g_list_model_get_n_items(G_LIST_MODEL(self->schemes_list));
	if (ix < n_schemes - 1)
		ix++;
	else if (ix > 0)
		ix--;
	else
		g_assert_not_reached();  /* it shouldn't be possible to remove all */

	GtkSourceStyleScheme *new_scheme = g_list_model_get_item(G_LIST_MODEL(self->schemes_list), ix);
	const char *new_id = gtk_source_style_scheme_get_id(new_scheme);

	populate_schemes_list(self, app);

	GSettings *prefs = i7_app_get_prefs(app);
	g_settings_set_string(prefs, PREFS_STYLE_SCHEME, new_id);
}

static gboolean
on_tab_width_output(GtkSpinButton *tab_width, I7PrefsWindow *self)
{
	GtkAdjustment *adjustment = gtk_spin_button_get_adjustment(tab_width);
	double value = gtk_adjustment_get_value(adjustment);
	g_autofree char *text = NULL;
	if (value != 0.0) {
		g_autofree char *text = g_strdup_printf(ngettext("1 space", "%.*f spaces", value), 0, value);
		gtk_entry_set_text(GTK_ENTRY(tab_width), text);
	} else {
		gtk_entry_set_text(GTK_ENTRY(tab_width), _("default"));
	}

	return TRUE;  /* value handled */
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

static char *
enum_get_name(HdyEnumValueObject *enum_obj, void *data)
{
	const char **names = data;
	int value = hdy_enum_value_object_get_value(enum_obj);
	return g_strdup(names[value]);
}

/* TYPE SYSTEM */

static void
i7_prefs_window_init(I7PrefsWindow *self)
{
	gtk_widget_init_template(GTK_WIDGET(self));

	self->schemes_list = g_list_store_new(GTK_SOURCE_TYPE_STYLE_SCHEME);
}

static void
i7_prefs_window_constructed(GObject* object)
{
	I7PrefsWindow *self = I7_PREFS_WINDOW(object);

	I7App *theapp = I7_APP(g_application_get_default());

	hdy_combo_row_set_for_enum(self->glulx_interpreter, i7_prefs_interpreter_get_type(),
		enum_get_name, interpreter_enum, NULL);
	hdy_combo_row_set_for_enum(self->docs_font_size, i7_prefs_font_size_get_type(),
		enum_get_name, font_size_enum, NULL);

	populate_schemes_list(self, theapp);

	/* Set up Natural Inform highlighting on the example buffer */
	GtkSourceBuffer *buffer = GTK_SOURCE_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(self->source_example)));
	set_buffer_language(buffer, "inform7");
	gtk_source_buffer_set_style_scheme(buffer, i7_app_get_current_color_scheme(theapp));

	/* Do the style scheme list */
	hdy_combo_row_bind_name_model(self->color_scheme, G_LIST_MODEL(self->schemes_list),
		(HdyComboRowGetNameFunc)format_color_scheme_name, NULL, NULL);

	/* g_object_freeze_notify() doesn't seem to work on this signal if we
	 * connect it automatically in the UI template */
	g_signal_connect(self->color_scheme, "notify::selected-index", G_CALLBACK(on_color_scheme_selected_index_notify), self);

	G_OBJECT_CLASS(i7_prefs_window_parent_class)->constructed(object);
}

static void
i7_prefs_window_finalize(GObject* object)
{
    I7PrefsWindow *self = I7_PREFS_WINDOW(object);

    g_clear_object(&self->schemes_list);

    G_OBJECT_CLASS(i7_prefs_window_parent_class)->finalize(object);
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
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, color_group);
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, color_scheme);
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, custom_font);
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, docs_font_size);
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, elastic_tabs);
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, enable_fonts);
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, enable_highlighting);
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, font_group);
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, glulx_interpreter);
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, added_nest);
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, restore_default_font);
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, show_debug_tabs);
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, source_example);
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, style_remove);
	gtk_widget_class_bind_template_child(widget_class, I7PrefsWindow, tab_width);
	gtk_widget_class_bind_template_callback(widget_class, on_group_visibility_switch_active_notify);
	gtk_widget_class_bind_template_callback(widget_class, on_restore_default_font_clicked);
	gtk_widget_class_bind_template_callback(widget_class, on_source_example_button_press_event);
	gtk_widget_class_bind_template_callback(widget_class, on_style_add_clicked);
	gtk_widget_class_bind_template_callback(widget_class, on_style_remove_clicked);
	gtk_widget_class_bind_template_callback(widget_class, on_tab_width_output);

	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->constructed = i7_prefs_window_constructed;
    object_class->finalize = i7_prefs_window_finalize;
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
		self->member, "selected-index", \
		G_SETTINGS_BIND_DEFAULT | G_SETTINGS_BIND_NO_SENSITIVITY, \
		(GSettingsBindGetMapping)settings_enum_get_mapping, \
		(GSettingsBindSetMapping)settings_enum_set_mapping, \
		enum_values, NULL)
	BIND(PREFS_AUTHOR_NAME, author_name, "text");
	BIND(PREFS_CUSTOM_FONT, custom_font, "font");
	BIND(PREFS_SYNTAX_HIGHLIGHTING, enable_highlighting, "active");
	BIND(PREFS_AUTO_INDENT, auto_indent, "active");
	BIND(PREFS_ELASTIC_TABSTOPS, elastic_tabs, "active");
	BIND(PREFS_AUTO_NUMBER, auto_number, "active");
	BIND(PREFS_CLEAN_BUILD_FILES, clean_build_files, "active");
	BIND(PREFS_CLEAN_BUILD_FILES, clean_index_files, "sensitive");
	BIND(PREFS_CLEAN_INDEX_FILES, clean_index_files, "active");
	BIND(PREFS_SHOW_DEBUG_LOG, show_debug_tabs, "active");
	BIND(PREFS_TAB_WIDTH, tab_width, "value");
	BIND_COMBO_BOX(PREFS_DOCS_FONT_SIZE, docs_font_size, font_size_enum);
	BIND_COMBO_BOX(PREFS_INTERPRETER, glulx_interpreter, interpreter_enum);
	BIND(PREFS_ADDED_NEST, added_nest, "text");
#undef BIND
#undef BIND_COMBO_BOX
	g_settings_bind_with_mapping(prefs, PREFS_FONT_SET,
		self->enable_fonts, "active",
		G_SETTINGS_BIND_DEFAULT | G_SETTINGS_BIND_NO_SENSITIVITY,
		(GSettingsBindGetMapping) font_set_get_mapping,
		(GSettingsBindSetMapping) font_set_set_mapping,
		NULL, NULL);

	/* Connect signals to GSettings; ensure signal handlers are disconnected
	 * when the preferences window is destroyed, because the GSettings will
	 * outlive it */
	g_signal_connect_object(prefs, "changed::" PREFS_ELASTIC_TABSTOPS, G_CALLBACK(on_config_elastic_tabstops_changed), self, 0);
	g_signal_connect_object(prefs, "changed::" PREFS_STYLE_SCHEME, G_CALLBACK(on_config_style_scheme_changed), self, 0);
	g_signal_connect_object(prefs, "changed::" PREFS_SYNTAX_HIGHLIGHTING, G_CALLBACK(on_config_syntax_highlighting_changed), self, 0);
	g_signal_connect_object(prefs, "changed::" PREFS_TAB_WIDTH, G_CALLBACK(on_config_tab_width_changed), self, 0);

	/* Set initial state for the widgets we just connected signals to */
	if (g_settings_get_boolean(prefs, PREFS_ELASTIC_TABSTOPS)) {
		add_elastic_tabstops_to_view(GTK_TEXT_VIEW(self->source_example));
		elastic_recalculate_view(GTK_TEXT_VIEW(self->source_example));
	}
	select_style_scheme(self, g_settings_get_string(prefs, PREFS_STYLE_SCHEME));
	if (!g_settings_get_boolean(prefs, PREFS_SYNTAX_HIGHLIGHTING))
		gtk_source_buffer_set_highlight_syntax(GTK_SOURCE_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(self->source_example))), FALSE);
	update_tabs(self->source_example);
}
