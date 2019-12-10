/* Copyright (C) 2006-2009, 2010, 2011, 2013, 2015, 2019 P. F. Chimento
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

#include <string.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>

#include "app.h"
#include "builder.h"
#include "configfile.h"
#include "error.h"
#include "story.h"

const char *font_set_enum[] = { "Standard", "Monospace", "Custom", NULL };
const char *font_size_enum[] = { "Standard", "Medium", "Large", "Huge", NULL };
const char *interpreter_enum[] = { "Glulxe (default)", "Git", NULL };

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
GVariant *
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
gboolean
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

/* ---------  Events from now on:   ------------ */

static void
on_config_font_set_changed(GSettings *settings, const char *key)
{
	/* update application to reflect new value */
	I7App *theapp = I7_APP(g_application_get_default());
	i7_app_update_css(theapp);
	GList *windows = gtk_application_get_windows(GTK_APPLICATION(theapp));
	for (GList *iter = windows; iter != NULL; iter = iter->next) {
		if (I7_IS_DOCUMENT(iter->data))
			i7_document_update_fonts(I7_DOCUMENT(iter->data));
	}
}

static void
on_config_custom_font_changed(GSettings *settings, const char *key)
{
	/* update application to reflect new value */
	I7App *theapp = I7_APP(g_application_get_default());
	GSettings *prefs = i7_app_get_prefs(theapp);
	if(g_settings_get_enum(prefs, PREFS_FONT_SET) == FONT_CUSTOM) {
		i7_app_update_css(theapp);
		GList *windows = gtk_application_get_windows(GTK_APPLICATION(theapp));
		for (GList *iter = windows; iter != NULL; iter = iter->next) {
			if (I7_IS_DOCUMENT(iter->data))
				i7_document_update_fonts(I7_DOCUMENT(iter->data));
		}
	}
}

static void
on_config_font_size_changed(GSettings *settings, const char *key)
{
	/* update application to reflect new value */
	I7App *theapp = I7_APP(g_application_get_default());
	i7_app_update_css(theapp);
	GList *windows = gtk_application_get_windows(GTK_APPLICATION(theapp));
	for (GList *iter = windows; iter != NULL; iter = iter->next) {
		if (I7_IS_DOCUMENT(iter->data)) {
			I7Document *document = I7_DOCUMENT(iter->data);
			i7_document_update_fonts(document);
			i7_document_update_font_sizes(document);
		}
	}
}

static void
on_config_style_scheme_changed(GSettings *settings, const char *key)
{
	I7App *theapp = I7_APP(g_application_get_default());

	char *newvalue = g_settings_get_string(settings, key);
	/* TODO: validate new value? */

	/* update application to reflect new value */
	select_style_scheme(theapp->prefs->schemes_view, newvalue);
	update_style(GTK_SOURCE_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(theapp->prefs->source_example))));
	GList *windows = gtk_application_get_windows(GTK_APPLICATION(theapp));
	for (GList *iter = windows; iter != NULL; iter = iter->next) {
		if (I7_IS_DOCUMENT(iter->data)) {
			I7Document *document = I7_DOCUMENT(iter->data);
			i7_document_update_fonts(document);
			i7_document_update_font_styles(document);
		}
	}

	g_free(newvalue);
}

static void
on_config_tab_width_changed(GSettings *settings, const char *key)
{
	unsigned newvalue = g_settings_get_uint(settings, key);

	/* Use default if set to 0, as per schema description */
	if(newvalue == 0)
		newvalue = DEFAULT_TAB_WIDTH;

	/* update application to reflect new value */
	I7App *theapp = I7_APP(g_application_get_default());
	update_tabs(theapp->prefs->tab_example);
	update_tabs(theapp->prefs->source_example);
	GList *windows = gtk_application_get_windows(GTK_APPLICATION(theapp));
	for (GList *iter = windows; iter != NULL; iter = iter->next) {
		if (I7_IS_DOCUMENT(iter->data))
			i7_document_update_tabs(I7_DOCUMENT(iter->data));
	}
}

static void
on_config_indent_wrapped_changed(GSettings *settings, const char *key)
{
	/* update application to reflect new value */
	I7App *theapp = I7_APP(g_application_get_default());
	update_tabs(theapp->prefs->tab_example);
	update_tabs(theapp->prefs->source_example);
	GList *windows = gtk_application_get_windows(GTK_APPLICATION(theapp));
	for (GList *iter = windows; iter != NULL; iter = iter->next) {
		if (I7_IS_DOCUMENT(iter->data)) {
			I7Document *document = I7_DOCUMENT(iter->data);
			i7_document_update_tabs(document);
			i7_document_update_indent_tags(document, NULL, NULL);
		}
	}
}

static void
on_config_elastic_tabstops_padding_changed(GSettings *settings, const char *key)
{
	/* update application to reflect new value */
	I7App *theapp = I7_APP(g_application_get_default());
	GList *windows = gtk_application_get_windows(GTK_APPLICATION(theapp));
	for (GList *iter = windows; iter != NULL; iter = iter->next) {
		if (I7_IS_DOCUMENT(iter->data))
			i7_document_refresh_elastic_tabstops(I7_DOCUMENT(iter->data));
	}
}

static void
on_config_use_interpreter_changed(GSettings *settings, const char *key)
{
	int newvalue = g_settings_get_enum(settings, key);
	I7App *theapp = I7_APP(g_application_get_default());
	GList *windows = gtk_application_get_windows(GTK_APPLICATION(theapp));
	for (GList *iter = windows; iter != NULL; iter = iter->next) {
		if (I7_IS_STORY(iter->data))
			i7_story_set_use_git(I7_STORY(iter->data), newvalue);
	}
}

static void
on_config_debug_log_visible_changed(GSettings *settings, const char *key)
{
	gboolean newvalue = g_settings_get_boolean(settings, key);
	/* update application to reflect new value */
	I7App *theapp = I7_APP(g_application_get_default());
	GList *windows = gtk_application_get_windows(GTK_APPLICATION(theapp));
	for (GList *iter = windows; iter != NULL; iter = iter->next) {
		if (I7_IS_STORY(iter->data)) {
			if (newvalue)
				i7_story_add_debug_tabs(I7_STORY(iter->data));
			else
				i7_story_remove_debug_tabs(I7_STORY(iter->data));
		}
	}
}

struct KeyToMonitor {
	const char *key;
	void (*callback)();
};

static struct KeyToMonitor keys_to_monitor[] = {
	{ "font-set", on_config_font_set_changed },
	{ "custom-font", on_config_custom_font_changed },
	{ "font-size", on_config_font_size_changed },
	{ "style-scheme", on_config_style_scheme_changed },
	{ "tab-width", on_config_tab_width_changed },
	{ "indent-wrapped", on_config_indent_wrapped_changed },
	{ "show-debug-log", on_config_debug_log_visible_changed },
	{ "use-interpreter", on_config_use_interpreter_changed },
	{ "elastic-tabstops-padding",on_config_elastic_tabstops_padding_changed }
};

/* Set up signals for the config keys */
void
init_config_file(GSettings *prefs)
{
	/* Add listeners to specific keys */
	unsigned i;
	for(i = 0; i < G_N_ELEMENTS(keys_to_monitor); i++)
	{
		if (keys_to_monitor[i].callback != NULL) {
			char *signal = g_strconcat("changed::", keys_to_monitor[i].key, NULL);
			g_signal_connect(prefs, signal, G_CALLBACK(keys_to_monitor[i].callback), NULL);
			g_free(signal);
		}
	}
}

/*
 * get_font_family:
 *
 * Return a string representing the font setting suitable to use in CSS. Must be
 * freed.
 */
char *
get_font_family(void)
{
	GSettings *prefs = i7_app_get_prefs(I7_APP(g_application_get_default()));

	switch(g_settings_get_enum(prefs, PREFS_FONT_SET)) {
		case FONT_MONOSPACE:
			return g_strdup("monospace");
		case FONT_CUSTOM:
			return g_settings_get_string(prefs, PREFS_CUSTOM_FONT);
		default:
			;
	}
	return g_strdup("sans");
}

/* Return a relative font size for the font size setting */
double
get_font_size(void)
{
	I7App *theapp = I7_APP(g_application_get_default());
	double size;

	switch(g_settings_get_enum(i7_app_get_prefs(theapp), PREFS_FONT_SIZE)) {
		case FONT_SIZE_MEDIUM:
			size = RELATIVE_SIZE_MEDIUM;
			break;
		case FONT_SIZE_LARGE:
			size = RELATIVE_SIZE_LARGE;
			break;
		case FONT_SIZE_HUGE:
			size = RELATIVE_SIZE_HUGE;
			break;
		default:
			size = RELATIVE_SIZE_STANDARD;
	}
	return size;
}
