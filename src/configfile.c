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

/* The slashes below are not directory separators, so they should be slashes! */
#define PREFS_BASE_PATH    "/apps/gnome-inform7/"
#define PREFS_APP_PATH     PREFS_BASE_PATH "app/"
#define PREFS_IDE_PATH     PREFS_BASE_PATH "ide/"
#define PREFS_EDITOR_PATH  PREFS_BASE_PATH "editor/"
#define PREFS_SYNTAX_PATH  PREFS_BASE_PATH "syntax/"
#define PREFS_WINDOW_PATH  PREFS_BASE_PATH "window/"
#define PREFS_SKEIN_PATH   PREFS_BASE_PATH "skein/"

#define PREFS_AUTHOR_NAME  PREFS_APP_PATH "AuthorName"

#define PREFS_SPELL_CHECK_DEFAULT       PREFS_IDE_PATH "spell-check-default"
#define PREFS_CLEAN_BUILD_FILES         PREFS_IDE_PATH "clean-build-files"
#define PREFS_CLEAN_INDEX_FILES         PREFS_IDE_PATH "clean-index-files"
#define PREFS_DEBUG_LOG_VISIBLE         PREFS_IDE_PATH "debug-log-visible"
#define PREFS_TOOLBAR_VISIBLE           PREFS_IDE_PATH "toolbar-default"
#define PREFS_STATUSBAR_VISIBLE         PREFS_IDE_PATH "statusbar-default"
#define PREFS_NOTEPAD_VISIBLE           PREFS_IDE_PATH "notepad-default"
#define PREFS_USE_INTERPRETER           PREFS_IDE_PATH "use-interpreter"
#define PREFS_ELASTIC_TABSTOPS_DEFAULT  PREFS_IDE_PATH "elastic-tabs-default"

#define PREFS_FONT_SET                  PREFS_EDITOR_PATH "font-set"
#define PREFS_CUSTOM_FONT               PREFS_EDITOR_PATH "custom-font"
#define PREFS_FONT_SIZE                 PREFS_EDITOR_PATH "font-size"
#define PREFS_STYLE_SCHEME              PREFS_EDITOR_PATH "style-scheme"
#define PREFS_TAB_WIDTH                 PREFS_EDITOR_PATH "tab-width"
#define PREFS_ELASTIC_TABSTOPS_PADDING  PREFS_EDITOR_PATH "elastic-tab-padding"

#define PREFS_SYNTAX_HIGHLIGHTING   PREFS_SYNTAX_PATH "syntax-highlighting"
#define PREFS_AUTO_INDENT           PREFS_SYNTAX_PATH "auto-indent"
#define PREFS_INTELLIGENCE          PREFS_SYNTAX_PATH "intelligence"
#define PREFS_AUTO_NUMBER_SECTIONS  PREFS_SYNTAX_PATH "auto-number-sections"

#define PREFS_APP_WINDOW_WIDTH   PREFS_WINDOW_PATH "app-window-width"
#define PREFS_APP_WINDOW_HEIGHT  PREFS_WINDOW_PATH "app-window-height"
#define PREFS_SLIDER_POSITION    PREFS_WINDOW_PATH "slider-position"
#define PREFS_EXT_WINDOW_WIDTH   PREFS_WINDOW_PATH "ext-window-width"
#define PREFS_EXT_WINDOW_HEIGHT  PREFS_WINDOW_PATH "ext-window-height"
#define PREFS_NOTEPAD_X          PREFS_WINDOW_PATH "notepad-pos-x"
#define PREFS_NOTEPAD_Y          PREFS_WINDOW_PATH "notepad-pos-y"
#define PREFS_NOTEPAD_WIDTH      PREFS_WINDOW_PATH "notepad-width"
#define PREFS_NOTEPAD_HEIGHT     PREFS_WINDOW_PATH "notepad-height"

#define PREFS_HORIZONTAL_SPACING  PREFS_SKEIN_PATH "horizontal-spacing"
#define PREFS_VERTICAL_SPACING    PREFS_SKEIN_PATH "vertical-spacing"

#define DESKTOP_PREFS_STANDARD_FONT   "/org/gnome/desktop/interface/font-name"
#define DESKTOP_PREFS_MONOSPACE_FONT  "/org/gnome/desktop/interface/monospace-font-name"

struct Schemas {
	GSettings *editor, *syntax, *app, *window, *ide, *skein, *desktop;
};

static struct Schemas schemas;

static void initialize_schemas() {
	schemas.editor = g_settings_new(PREFS_EDITOR_PATH);
	schemas.syntax = g_settings_new(PREFS_SYNTAX_PATH);
	schemas.app = g_settings_new(PREFS_APP_PATH);
	schemas.window = g_settings_new(PREFS_WINDOW_PATH);
	schemas.ide = g_settings_new(PREFS_IDE_PATH);
	schemas.skein = g_settings_new(PREFS_SKEIN_PATH);
	schemas.desktop = g_settings_new(DESKTOP_PREFS_PATH);
}

#define getter_and_setter_for(type, gsettings_fun, function_name, schema, key) \
  type config_get_##function_name() { return g_settings_get_##gsettings_fun(schemas.schema, key); } \
  void config_set_##function_name(const type v) { g_settings_set_##gsettings_fun(schemas.schema, key, v); }  

#define boolean_getter_and_setter_for(function_name, schema, key) \
  getter_and_setter_for(gboolean, boolean, function_name, schema, key)
#define string_getter_and_setter_for(function_name, schema, key) \
  getter_and_setter_for(gchar *, string, function_name, schema, key)
#define integer_getter_and_setter_for(function_name, schema, key) \
  getter_and_setter_for(gint, int, function_name, schema, key)
#define enum_getter_and_setter_for(function_name, schema, key) \
  getter_and_setter_for(gint, enum, function_name, schema, key)

integer_getter_and_setter_for(horizontal_spacing, skein, PREFS_HORIZONTAL_SPACING);
integer_getter_and_setter_for(vertical_spacing, skein, PREFS_VERTICAL_SPACING);

integer_getter_and_setter_for(app_window_width, window, PREFS_APP_WINDOW_WIDTH);
integer_getter_and_setter_for(app_window_height, window, PREFS_APP_WINDOW_HEIGHT);
integer_getter_and_setter_for(ext_window_width, window, PREFS_EXT_WINDOW_WIDTH);
integer_getter_and_setter_for(ext_window_height, window, PREFS_EXT_WINDOW_HEIGHT);
integer_getter_and_setter_for(notepad_x, window, PREFS_NOTEPAD_X);
integer_getter_and_setter_for(notepad_y, window, PREFS_NOTEPAD_Y);
integer_getter_and_setter_for(notepad_width, window, PREFS_NOTEPAD_WIDTH);
integer_getter_and_setter_for(notepad_height, window, PREFS_NOTEPAD_HEIGHT);
integer_getter_and_setter_for(slider_position, window, PREFS_SLIDER_POSITION);

boolean_getter_and_setter_for(spell_check_default, ide, PREFS_SPELL_CHECK_DEFAULT);
boolean_getter_and_setter_for(clean_build_files, ide, PREFS_CLEAN_BUILD_FILES);
boolean_getter_and_setter_for(clean_index_files, ide, PREFS_CLEAN_INDEX_FILES);
boolean_getter_and_setter_for(debug_log_visible, ide, PREFS_DEBUG_LOG_VISIBLE);
boolean_getter_and_setter_for(toolbar_visible, ide, PREFS_TOOLBAR_VISIBLE);
boolean_getter_and_setter_for(statusbar_visible, ide, PREFS_STATUSBAR_VISIBLE);
boolean_getter_and_setter_for(notepad_visible, ide, PREFS_NOTEPAD_VISIBLE);
enum_getter_and_setter_for(interpreter, ide, PREFS_INTERPRETER);
boolean_getter_and_setter_for(elastic_tabstops_default, ide, PREFS_ELASTIC_TABSTOPS_DEFAULT);

boolean_getter_and_setter_for(syntax_highlighting, syntax, PREFS_SYNTAX_HIGHLIGHTING);
boolean_getter_and_setter_for(auto_indent, syntax, PREFS_AUTO_INDENT);
boolean_getter_and_setter_for(intelligence, syntax, PREFS_INTELLIGENCE);
boolean_getter_and_setter_for(auto_number_sections, syntax, PREFS_AUTO_NUMBER_SECTIONS);

enum_getter_and_setter_for(font_set, editor, PREFS_FONT_SET);
enum_getter_and_setter_for(font_size, editor, PREFS_FONT_SIZE);
string_getter_and_setter_for(custom_font, editor, PREFS_CUSTOM_FONT);
string_getter_and_setter_for(style_scheme, editor, PREFS_STYLE_SCHEME);
integer_getter_and_setter_for(tab_width, editor, PREFS_TAB_WIDTH);
integer_getter_and_setter_for(elastic_tabstops_padding, editor, PREFS_ELASTIC_TABSTOPS_PADDING);

string_getter_and_setter_for(author_name, app, PREFS_AUTHOR_NAME);

/* ---------  Events from now on:   ------------ */

static void
on_config_font_set_changed(GSettings *settings, const char *key)
{
	/* update application to reflect new value */
	I7App *theapp = i7_app_get();
	update_font(GTK_WIDGET(theapp->prefs->source_example));
	update_font(GTK_WIDGET(theapp->prefs->tab_example));
	i7_app_foreach_document(theapp, (I7DocumentForeachFunc)i7_document_update_fonts, NULL);
}

static void
on_config_custom_font_changed(GSettings *settings, const char *key)
{
	/* update application to reflect new value */
	I7App *theapp = i7_app_get();
	if(config_get_font_set() == FONT_CUSTOM) {
		update_font(GTK_WIDGET(theapp->prefs->source_example));
		update_font(GTK_WIDGET(theapp->prefs->tab_example));
		i7_app_foreach_document(theapp, (I7DocumentForeachFunc)i7_document_update_fonts, NULL);
	}

	g_free(newvalue);
}

static void
on_config_font_size_changed(GSettings *settings, const char *key)
{
	/* update application to reflect new value */
	I7App *theapp = i7_app_get();
	update_font(GTK_WIDGET(theapp->prefs->source_example));
	update_font(GTK_WIDGET(theapp->prefs->tab_example));
	i7_app_foreach_document(theapp, (I7DocumentForeachFunc)i7_document_update_fonts, NULL);
	i7_app_foreach_document(theapp, (I7DocumentForeachFunc)i7_document_update_font_sizes, NULL);
}

static void
on_config_style_scheme_changed(GSettings *settings, const char *key, GtkWidget *list)
{
	I7App *theapp = i7_app_get();

	gchar *newvalue = g_settings_get_string(settings, key);
	/* TODO: validate new value? */

	/* update application to reflect new value */
	// select_style_scheme(GTK_TREE_VIEW(list), newvalue); // TODO: check me
	update_style(GTK_SOURCE_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(theapp->prefs->source_example))));
	i7_app_foreach_document(theapp, (I7DocumentForeachFunc)i7_document_update_fonts, NULL);
	i7_app_foreach_document(theapp, (I7DocumentForeachFunc)i7_document_update_font_styles, NULL);

	g_free(newvalue);
}

static void
on_config_tab_width_changed(GSettings *settings, const char *key)
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
	update_tabs(theapp->prefs->tab_example);
	update_tabs(theapp->prefs->source_example);
	i7_app_foreach_document(theapp, (I7DocumentForeachFunc)i7_document_update_tabs, NULL);
}

static void
on_config_syntax_highlighting_changed(GSettings *settings, const char *key)
{
	/* update application to reflect new value */
	I7App *theapp = i7_app_get();
	i7_app_foreach_document(theapp, (I7DocumentForeachFunc)i7_document_update_source_highlight, NULL);
	i7_app_foreach_document(theapp, (I7DocumentForeachFunc)i7_document_update_fonts, NULL);
}

static void
on_config_intelligence_changed(GSettings *settings, const char *key)
{
	gboolean newvalue = g_settings_get_boolean(settings, key);
	/* update application to reflect new value */
	I7App *theapp = i7_app_get();
	/* make the other checkboxes dependent on this checkbox active or inactive*/
	gtk_widget_set_sensitive(theapp->prefs->auto_number, newvalue);
}

static void
on_config_elastic_tabstops_padding_changed(GSettings *settings, const char *key)
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
set_glulx_interpreter(I7Document *document, gpointer data)
{
	if(I7_IS_STORY(document))
		i7_story_set_use_git(I7_STORY(document), GPOINTER_TO_INT(data));
}

static void
on_config_use_interpreter_changed(GSettings *settings, const char *key)
{
	int newvalue = g_settings_get_enum(settings, key);
	i7_app_foreach_document(i7_app_get(), set_glulx_interpreter, GINT_TO_POINTER(newvalue));
}

static void
on_config_clean_build_files_changed(GSettings *settings, const gchar *key)
{
	gboolean newvalue = g_settings_get_boolean(settings, key);
	/* update application to reflect new value */
	I7App *theapp = i7_app_get();
	/* make the other checkboxes dependent on this checkbox active or inactive*/
	gtk_widget_set_sensitive(theapp->prefs->clean_index_files, newvalue);
}

static void
on_config_debug_log_visible_changed(GSettings *settings, const char *key)
{
	gboolean newvalue = g_settings_get_boolean(settings, key);
	/* update application to reflect new value */
	i7_app_foreach_document(i7_app_get(), (I7DocumentForeachFunc)(newvalue? i7_story_add_debug_tabs : i7_story_remove_debug_tabs), NULL);
}

static void
update_skein_spacing(I7Document *document)
{
	if(!I7_IS_STORY(document))
		return;

	double horizontal = (double)config_get_horizontal_spacing();
	double vertical = (double)config_get_vertical_spacing();

	I7Story *story = I7_STORY(document);
	g_object_set(i7_story_get_skein(story),
		"horizontal-spacing", horizontal,
		"vertical-spacing", vertical,
		NULL);
	gtk_range_set_value(GTK_RANGE(story->skein_spacing_horizontal), horizontal);
	gtk_range_set_value(GTK_RANGE(story->skein_spacing_vertical), vertical);
}

static void
on_config_skein_spacing_changed(GSettings *settings, const char *key)
{
	i7_app_foreach_document(i7_app_get(), (I7DocumentForeachFunc)update_skein_spacing, NULL);
}

struct KeyToMonitor {
	GSettings **schema;
	const char *key, *widgetname;
	void (*callback)();
};

static struct KeyToMonitor keys_to_monitor[] = {
	{ &schemas.editor, PREFS_FONT_SET, "font_set", on_config_font_set_changed },
	{ &schemas.editor, PREFS_CUSTOM_FONT, "custom_font", on_config_custom_font_changed },
	{ &schemas.editor, PREFS_FONT_SIZE, "font_size",	on_config_font_size_changed },
	{ &schemas.editor, PREFS_STYLE_SCHEME, "schemes_view", on_config_style_scheme_changed },
	{ &schemas.editor, PREFS_TAB_WIDTH, "tab_ruler", on_config_tab_width_changed },
	{ &schemas.syntax, PREFS_SYNTAX_HIGHLIGHTING, "enable_highlighting", on_config_syntax_highlighting_changed },
	{ &schemas.syntax, PREFS_AUTO_INDENT, "auto_indent", NULL },
	{ &schemas.syntax, PREFS_INTELLIGENCE, "follow_symbols", on_config_intelligence_changed },
	{ &schemas.syntax, PREFS_AUTO_NUMBER_SECTIONS, "auto_number", NULL },
	{ &schemas.app, PREFS_AUTHOR_NAME, "author_name", NULL },
	{ &schemas.ide, PREFS_CLEAN_BUILD_FILES, "clean_build_files", on_config_clean_build_files_changed },
	{ &schemas.ide, PREFS_CLEAN_INDEX_FILES, "clean_index_files", NULL },
	{ &schemas.ide, PREFS_DEBUG_LOG_VISIBLE, "show_debug_tabs", on_config_debug_log_visible_changed },
	{ &schemas.ide, PREFS_INTERPRETER, "glulx_combo", on_config_use_interpreter_changed },
	{ &schemas.editor, PREFS_ELASTIC_TABSTOPS_PADDING, NULL, on_config_elastic_tabstops_padding_changed },
	{ &schemas.skein, PREFS_HORIZONTAL_SPACING, NULL, on_config_skein_spacing_changed },
	{ &schemas.skein, PREFS_VERTICAL_SPACING, NULL, on_config_skein_spacing_changed }
};

static void
bind_widget(GtkWidget *widget, GSettings *settings, const char *key)
{
	if (widget == NULL) return;
	const char *property = NULL;
	if(GTK_IS_ENTRY(widget))
		property = "text";
	else if(GTK_IS_TOGGLE_BUTTON(widget))
		property = "active";
	else if(GTK_IS_COMBO_BOX(widget))
		property = "active"; // behaves incorrectly. why?
	else if(GTK_IS_RULER(widget))
		property = "position";
	else if(GTK_IS_FONT_BUTTON(widget))
		property = "font-name";

	// GTK_TREE_VIEW // TODO: requires custom mapping functions.

	g_settings_bind(settings, key, widget, property, G_SETTINGS_BIND_DEFAULT);
}


/* Check if the config keys exist and if not, set them to defaults. */
void
init_config_file(GtkBuilder *builder)
{
	initialize_schemas();

	/* Add listeners to specific keys and pass them their associated widgets as data */
	int i;
	for(i = 0; i < G_N_ELEMENTS(keys_to_monitor); i++)
	{
		GSettings *settings = *keys_to_monitor[i].schema;

		if(keys_to_monitor[i].widgetname != NULL) {
			GtkWidget *widget = GTK_WIDGET(load_object(builder, keys_to_monitor[i].widgetname));
			bind_widget(widget, settings, keys_to_monitor[i].key);
		}

		if (keys_to_monitor[i].callback != NULL) {
			char *signal = g_strconcat("changed::", keys_to_monitor[i].key, NULL);
			g_signal_connect(G_OBJECT(settings), signal, G_CALLBACK(keys_to_monitor[i].callback), NULL);
			g_free(signal);
		}
	}
}

/* Desktop preferences stuff: */

/*
 * get_desktop_standard_font:
 *
 * Return the Gnome desktop document font as a font description. Must be freed.
 */
PangoFontDescription *
get_desktop_standard_font(void)
{
	PangoFontDescription *retval;
	char *font = g_settings_get_string(schemas.desktop, DESKTOP_PREFS_STANDARD_FONT);
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
	char *font = g_settings_get_string(schemas.desktop, DESKTOP_PREFS_MONOSPACE_FONT);
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
	char *customfont;
	switch(config_get_font_set()) {
		case FONT_MONOSPACE:
			return get_desktop_monospace_font();
		case FONT_CUSTOM:
			customfont = config_get_custom_font();
			if(customfont)
				return pango_font_description_from_string(customfont);
			/* else fall through */
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

	switch(config_get_font_size()) {
		case FONT_SIZE_MEDIUM:
			size *= RELATIVE_SIZE_MEDIUM;
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

