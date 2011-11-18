/* Copyright (C) 2006-2009, 2010, 2011 P. F. Chimento
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

#include <string.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksourceview.h>
#include "configfile.h"
#include "app.h"
#include "builder.h"
#include "error.h"
#include "story.h"

/* These functions are wrappers for GSettings setting and getting functions. */

void
config_file_set_string(const gchar *key, const gchar *value)
{
	GSettings *settings = g_settings_new(PREFS_BASE_PATH);
	g_settings_set_string(settings, key, value);
	g_object_unref(settings);
}

void
config_file_set_int(const gchar *key, const gint value)
{
	GSettings *settings = g_settings_new(PREFS_BASE_PATH);
	g_settings_set_int(settings, key, value);
	g_object_unref(settings);
}

void
config_file_set_bool(const gchar *key, const gboolean value)
{
	GSettings *settings = g_settings_new(PREFS_BASE_PATH);
	g_settings_set_boolean(settings, key, value);
	g_object_unref(settings);
}

void
config_file_set_enum(const gchar *key, const int value)
{
	GSettings *settings = g_settings_new(PREFS_BASE_PATH);
	g_settings_set_enum(settings, key, value);
	g_object_unref(settings);
}

/* The string must be freed afterward. */
gchar *
config_file_get_string(const gchar *key)
{
	GSettings *settings = g_settings_new(PREFS_BASE_PATH);
	char *retval = g_settings_get_string(settings, key);
	g_object_unref(settings);
	return retval;
}

gint
config_file_get_int(const gchar *key)
{
	GSettings *settings = g_settings_new(PREFS_BASE_PATH);
	int retval = g_settings_get_int(settings, key);
	g_object_unref(settings);
	return retval;
}

gboolean
config_file_get_bool(const gchar *key)
{
	GSettings *settings = g_settings_new(PREFS_BASE_PATH);
	gboolean retval = g_settings_get_boolean(settings, key);
	g_object_unref(settings);
	return retval;
}

gint
config_file_get_enum(const gchar *key)
{
	GSettings *settings = g_settings_new(PREFS_BASE_PATH);
	int retval = g_settings_get_enum(settings, key);
	g_object_unref(settings);
	return retval;
}


static void
on_config_font_set_changed(GSettings *settings, guint id, const gchar *key, GtkWidget *combobox)
{
	int newvalue = g_settings_get_enum(settings, key);

	/* update application to reflect new value */
	I7App *theapp = i7_app_get();
	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), newvalue);
	update_font(GTK_WIDGET(theapp->prefs->source_example));
	update_font(GTK_WIDGET(theapp->prefs->tab_example));
	i7_app_foreach_document(theapp, (I7DocumentForeachFunc)i7_document_update_fonts, NULL);
}

static void
on_config_custom_font_changed(GSettings *settings, guint id, const gchar *key, GtkWidget *fontbutton)
{
	char *newvalue = g_settings_get_string(settings, key);
	/* TODO: validate new value? */

	/* update application to reflect new value */
	I7App *theapp = i7_app_get();
	gtk_font_button_set_font_name(GTK_FONT_BUTTON(fontbutton), newvalue);
	if(config_file_get_enum(PREFS_FONT_SET) == FONT_CUSTOM) {
		update_font(GTK_WIDGET(theapp->prefs->source_example));
		update_font(GTK_WIDGET(theapp->prefs->tab_example));
		i7_app_foreach_document(theapp, (I7DocumentForeachFunc)i7_document_update_fonts, NULL);
	}

	g_free(newvalue);
}

static void
on_config_font_size_changed(GSettings *settings, guint id, const gchar *key, GtkWidget *combobox)
{
	gint newvalue = g_settings_get_enum(settings, key);

	/* update application to reflect new value */
	I7App *theapp = i7_app_get();
	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), newvalue);
	update_font(GTK_WIDGET(theapp->prefs->source_example));
	update_font(GTK_WIDGET(theapp->prefs->tab_example));
	i7_app_foreach_document(theapp, (I7DocumentForeachFunc)i7_document_update_fonts, NULL);
	i7_app_foreach_document(theapp, (I7DocumentForeachFunc)i7_document_update_font_sizes, NULL);
}

static void
on_config_style_scheme_changed(GSettings *settings, guint id, const gchar *key, GtkWidget *list)
{
	I7App *theapp = i7_app_get();

	gchar *newvalue = g_settings_get_string(settings, key);
	/* TODO: validate new value? */

	/* update application to reflect new value */
	select_style_scheme(GTK_TREE_VIEW(list), newvalue);
	update_style(GTK_SOURCE_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(theapp->prefs->source_example))));
	i7_app_foreach_document(theapp, (I7DocumentForeachFunc)i7_document_update_fonts, NULL);
	i7_app_foreach_document(theapp, (I7DocumentForeachFunc)i7_document_update_font_styles, NULL);

	g_free(newvalue);
}

static void
on_config_tab_width_changed(GSettings *settings, guint id, const gchar *key, GtkWidget *range)
{
	gint newvalue = g_settings_get_int(settings, key);
	/* validate new value */
	if(newvalue < 0)
		g_settings_set_int(settings, key, DEFAULT_TAB_WIDTH);

	/* Use default if set to 0, as per schema description */
	if(newvalue == 0)
		newvalue = DEFAULT_TAB_WIDTH;

	/* update application to reflect new value */
	I7App *theapp = i7_app_get();
	gtk_range_set_value(GTK_RANGE(range), (gdouble)newvalue);
	update_tabs(theapp->prefs->tab_example);
	update_tabs(theapp->prefs->source_example);
	i7_app_foreach_document(theapp, (I7DocumentForeachFunc)i7_document_update_tabs, NULL);
}

static void
on_config_syntax_highlighting_changed(GSettings *settings, guint id, const gchar *key, GtkWidget *toggle)
{
	/* update application to reflect new value */
	I7App *theapp = i7_app_get();
	gboolean newvalue = g_settings_get_boolean(settings, key);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), newvalue);
	i7_app_foreach_document(theapp, (I7DocumentForeachFunc)i7_document_update_source_highlight, NULL);
	i7_app_foreach_document(theapp, (I7DocumentForeachFunc)i7_document_update_fonts, NULL);
}

static void
on_config_intelligence_changed(GSettings *settings, guint id, const gchar *key, GtkWidget *toggle)
{
	gboolean newvalue = g_settings_get_boolean(settings, key);
	/* update application to reflect new value */
	I7App *theapp = i7_app_get();
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), newvalue);
	/* make the other checkboxes dependent on this checkbox active or inactive*/
	gtk_widget_set_sensitive(theapp->prefs->auto_number, newvalue);
}

static void
on_config_elastic_tabstops_padding_changed(GSettings *settings, guint id, const gchar *key)
{
	gint newvalue = g_settings_get_int(settings, key);
	/* validate new value */
	if(newvalue < 0)
		g_settings_set_int(settings, key, DEFAULT_ELASTIC_TAB_PADDING);

	if(newvalue == 0)
		newvalue = DEFAULT_ELASTIC_TAB_PADDING;

	/* update application to reflect new value */
	i7_app_foreach_document(i7_app_get(), (I7DocumentForeachFunc)i7_document_refresh_elastic_tabstops, NULL);
}

static void
on_config_author_name_changed(GSettings *settings, guint id, const gchar *key, GtkWidget *editable)
{
	gchar *newvalue = g_settings_get_string(settings, key);
	/* update application to reflect new value */
	gtk_entry_set_text(GTK_ENTRY(editable), newvalue);
	g_free(newvalue);
}

static void
set_glulx_interpreter(I7Document *document, gpointer data)
{
	if(I7_IS_STORY(document))
		i7_story_set_use_git(I7_STORY(document), GPOINTER_TO_INT(data));
}

static void
on_config_use_git_changed(GSettings *settings, guint id, const gchar *key, GtkComboBox *box)
{
	gboolean newvalue = g_settings_get_boolean(settings, key);
	/* update application to reflect new value */
	gtk_combo_box_set_active(box, newvalue? TRUE : FALSE);
	i7_app_foreach_document(i7_app_get(), set_glulx_interpreter, GINT_TO_POINTER(newvalue));
}

static void
on_config_clean_build_files_changed(GSettings *settings, guint id, const gchar *key, GtkWidget *toggle)
{
	gboolean newvalue = g_settings_get_boolean(settings, key);
	/* update application to reflect new value */
	I7App *theapp = i7_app_get();
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), newvalue);
	/* make the other checkboxes dependent on this checkbox active or inactive*/
	gtk_widget_set_sensitive(theapp->prefs->clean_index_files, newvalue);
}

static void
on_config_debug_log_visible_changed(GSettings *settings, guint id, const gchar *key, GtkWidget *toggle)
{
	gboolean newvalue = g_settings_get_boolean(settings, key);
	/* update application to reflect new value */
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), newvalue);
	i7_app_foreach_document(i7_app_get(), (I7DocumentForeachFunc)(newvalue? i7_story_add_debug_tabs : i7_story_remove_debug_tabs), NULL);
}

static void
update_skein_spacing(I7Document *document)
{
	if(!I7_IS_STORY(document))
		return;

	gdouble horizontal = (gdouble)config_file_get_int(PREFS_HORIZONTAL_SPACING);
	gdouble vertical = (gdouble)config_file_get_int(PREFS_VERTICAL_SPACING);

	I7Story *story = I7_STORY(document);
	g_object_set(i7_story_get_skein(story),
		"horizontal-spacing", horizontal,
		"vertical-spacing", vertical,
		NULL);
	gtk_range_set_value(GTK_RANGE(story->skein_spacing_horizontal), horizontal);
	gtk_range_set_value(GTK_RANGE(story->skein_spacing_vertical), vertical);
}

static void
on_config_skein_spacing_changed(GSettings *settings, guint id, const gchar *key)
{
	i7_app_foreach_document(i7_app_get(), (I7DocumentForeachFunc)update_skein_spacing, NULL);
}

static void
on_config_generic_bool_changed(GSettings *settings, guint id, const gchar *key, GtkWidget *toggle)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), g_settings_get_boolean(settings, key));
}

struct KeyToMonitor {
	const gchar *name, *widgetname;
	void (*callback)();
};

static struct KeyToMonitor keys_to_monitor[] = {
	{ PREFS_FONT_SET, "font_set", on_config_font_set_changed },
	{ PREFS_CUSTOM_FONT, "custom_font", on_config_custom_font_changed },
	{ PREFS_FONT_SIZE, "font_size",	on_config_font_size_changed },
	{ PREFS_STYLE_SCHEME, "schemes_view", on_config_style_scheme_changed },
	{ PREFS_TAB_WIDTH, "tab_ruler", on_config_tab_width_changed },
	{ PREFS_SYNTAX_HIGHLIGHTING, "enable_highlighting", on_config_syntax_highlighting_changed },
	{ PREFS_AUTO_INDENT, "auto_indent", on_config_generic_bool_changed },
	{ PREFS_INTELLIGENCE, "follow_symbols", on_config_intelligence_changed },
	{ PREFS_AUTO_NUMBER_SECTIONS, "auto_number", on_config_generic_bool_changed },
	{ PREFS_AUTHOR_NAME, "author_name", on_config_author_name_changed },
	{ PREFS_CLEAN_BUILD_FILES, "clean_build_files", on_config_clean_build_files_changed },
	{ PREFS_CLEAN_INDEX_FILES, "clean_index_files", on_config_generic_bool_changed },
	{ PREFS_DEBUG_LOG_VISIBLE, "show_debug_tabs", on_config_debug_log_visible_changed },
	{ PREFS_USE_GIT, "glulx_combo", on_config_use_git_changed },
	{ PREFS_ELASTIC_TABSTOPS_PADDING, NULL, on_config_elastic_tabstops_padding_changed },
	{ PREFS_HORIZONTAL_SPACING, NULL, on_config_skein_spacing_changed },
	{ PREFS_VERTICAL_SPACING, NULL, on_config_skein_spacing_changed }
};

/* Check if the config keys exist and if not, set them to defaults. */
void
init_config_file(GtkBuilder *builder)
{
	/* Initialize the GConf client */
	GSettings *settings = g_settings_new(PREFS_BASE_PATH);

	/* Add listeners to specific keys and pass them their associated widgets as data */
	int i;
	for(i = 0; i < G_N_ELEMENTS(keys_to_monitor); i++)
	{
		char *signal = g_strconcat("changed::", keys_to_monitor[i].name, NULL);
		g_signal_connect(settings, signal, G_CALLBACK(keys_to_monitor[i].callback),
			(keys_to_monitor[i].widgetname)? GTK_WIDGET(load_object(builder, keys_to_monitor[i].widgetname)) : NULL);
		g_free(signal);
	}

	g_object_unref(settings);
}

/* Notify every config key so that the preferences dialog picks it up */
void
trigger_config_file(void)
{
	/* Initialize the GConf client */
	GSettings *settings = g_settings_new (PREFS_BASE_PATH);
	/* Trigger all the keys for which we have listeners */
	int i;
	for(i = 0; i < G_N_ELEMENTS(keys_to_monitor); i++)
	{
		char *signal = g_strconcat("changed::", keys_to_monitor[i].name, NULL);
		g_signal_emit_by_name (settings, signal, keys_to_monitor[i].name);
		g_free (signal);
	}

	g_object_unref(settings);
}

/*
 * get_desktop_standard_font:
 *
 * Return the Gnome desktop document font as a font description. Must be freed.
 */
PangoFontDescription *
get_desktop_standard_font(void)
{
	PangoFontDescription *retval;
	char *font = config_file_get_string(DESKTOP_PREFS_STANDARD_FONT);
	if(!font)
		return pango_font_description_from_string(STANDARD_FONT_FALLBACK);
	retval = pango_font_description_from_string(font);
	g_free(font);
	return retval;
}

/*
 * get_desktop_monospace_font:
 *
 * Return the Gnome desktop monospace font as a font description. Must be freed.
 */
PangoFontDescription *
get_desktop_monospace_font(void)
{
	PangoFontDescription *retval;
	char *font = config_file_get_string(DESKTOP_PREFS_MONOSPACE_FONT);
	if(!font)
		return pango_font_description_from_string(MONOSPACE_FONT_FALLBACK);
	retval = pango_font_description_from_string(font);
	g_free(font);
	return retval;
}

/*
 * get_font_family:
 *
 * Return a font description for the font setting. Must be freed.
 */
static PangoFontDescription *
get_font_family(void)
{
	gchar *customfont;
	switch(config_file_get_enum(PREFS_FONT_SET)) {
		case FONT_MONOSPACE:
			return get_desktop_monospace_font();
		case FONT_CUSTOM:
		{
			char *font = config_file_get_string(PREFS_CUSTOM_FONT);
			if(font) {
				PangoFontDescription *retval = pango_font_description_from_string(font);
				g_free(font);
				return retval;
			}
			/* else fall through */
		}
		default:
			;
	}
	return get_desktop_standard_font();
}

/* Return the font size in Pango units for the font size setting */
gint
get_font_size(PangoFontDescription *font)
{
	double size = pango_font_description_get_size(font);
	if(pango_font_description_get_size_is_absolute(font))
		size *= 96.0 / 72.0; /* a guess; not likely to be absolute anyway */
	if(size == 0.0)
		size = DEFAULT_SIZE_STANDARD * PANGO_SCALE;

	switch(config_file_get_enum(PREFS_FONT_SIZE)) {
		case FONT_SIZE_MEDIUM:
			size *= RELATIVE_SIZE_MEDIUM  ;
			break;
		case FONT_SIZE_LARGE:
			size *= RELATIVE_SIZE_LARGE;
			break;
		case FONT_SIZE_HUGE:
			size *= RELATIVE_SIZE_HUGE;
			break;
		default:
			size *= RELATIVE_SIZE_STANDARD;
	}
	return size;
}

/* Get the current font as a PangoFontDescription.
Must be freed with pango_font_description_free. */
PangoFontDescription *
get_font_description(void)
{
	PangoFontDescription *font = get_font_family();
	pango_font_description_set_size(font, get_font_size(font));
	return font;
}
