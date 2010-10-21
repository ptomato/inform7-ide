/* This file is part of GNOME Inform 7.
 * Copyright (c) 2006-2009 P. F. Chimento <philip.chimento@gmail.com>
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
 
#include <gnome.h>
#include <gconf/gconf-client.h>

#include "configfile.h"
#include "elastic.h"
#include "error.h"
#include "extension.h"
#include "inspector.h"
#include "prefs.h"
#include "story.h"
#include "support.h"
#include "taberrors.h"
#include "tabskein.h"

/* Enum-to-string lookup tables */
GConfEnumStringPair font_styling_lookup_table[] = {
    { FONT_STYLING_NONE, "None" },
    { FONT_STYLING_SUBTLE, "Subtle" },
    { FONT_STYLING_OFTEN, "Often" }
};

GConfEnumStringPair font_set_lookup_table[] = {
    { FONT_SET_STANDARD, "Standard" },
    { FONT_SET_PROGRAMMER, "Programmer" },
    { FONT_SET_CUSTOM, "Custom" }
};

GConfEnumStringPair font_size_lookup_table[] = {
    { FONT_SIZE_STANDARD, "Standard" },
    { FONT_SIZE_MEDIUM, "Medium" },
    { FONT_SIZE_LARGE, "Large" },
    { FONT_SIZE_HUGE, "Huge" }
};

GConfEnumStringPair change_colors_lookup_table[] = {
    { CHANGE_COLORS_NEVER, "Never" },
    { CHANGE_COLORS_OCCASIONALLY, "Occasionally" },
    { CHANGE_COLORS_OFTEN, "Often" }
};

GConfEnumStringPair color_set_lookup_table[] = {
    { COLOR_SET_SUBDUED, "Subdued" },
    { COLOR_SET_STANDARD, "Standard" },
    { COLOR_SET_PSYCHEDELIC, "Psychedelic" }
};

/* Make a full key name out of the relative path and key name. String must be
freed afterwards. */
static gchar *
make_keyname(const gchar *path, const gchar *key)
{
    return g_strconcat(GCONF_BASE_PATH, path, "/", key, NULL);
    /* The slash is not a directory separator */
}

/* These functions are wrappers for GConf setting and getting functions,
that give us a nice error dialog if they fail. */

void
config_file_set_string(const gchar *path, const gchar *key, const gchar *value)
{
    GConfClient *client = gconf_client_get_default();
    gchar *keyname = make_keyname(path, key);
    gconf_client_set_string(client, keyname, value, NULL);

    g_object_unref(client);
    g_free(keyname);
}

void
config_file_set_int(const gchar *path, const gchar *key, const gint value)
{
    GConfClient *client = gconf_client_get_default();
    gchar *keyname = make_keyname(path, key);
    gconf_client_set_int(client, keyname, value, NULL);

    g_object_unref(client);
    g_free(keyname);
}

void
config_file_set_bool(const gchar *path, const gchar *key, const gboolean value)
{
    GConfClient *client = gconf_client_get_default();
    gchar *keyname = make_keyname(path, key);
    gconf_client_set_bool(client, keyname, value, NULL); 
    
    g_object_unref(client);
    g_free(keyname);
}

void 
config_file_set_enum(const gchar *path, const gchar *key, const gint value,
                     GConfEnumStringPair lookup_table[])
{
    config_file_set_string(path, key, 
      gconf_enum_to_string(lookup_table, value));
}

/* The string must be freed afterward. Returns NULL if key is unset*/
gchar *
config_file_get_string(const gchar *path, const gchar *key)
{
    GConfClient *client = gconf_client_get_default();
    
    gchar *keyname = make_keyname(path, key);
    gchar *value = gconf_client_get_string(client, keyname, NULL);
    
    g_object_unref(client);
    g_free(keyname);
    return value;
}

gint
config_file_get_int(const gchar *path, const gchar *key)
{
    GConfClient *client = gconf_client_get_default();
    
    gchar *keyname = make_keyname(path, key);
    gint value = gconf_client_get_int(client, keyname, NULL);
    
    g_object_unref(client);
    g_free(keyname);
    return value;
}

gboolean
config_file_get_bool(const gchar *path, const gchar *key)
{
    GConfClient *client = gconf_client_get_default();
    
    gchar *keyname = make_keyname(path, key);
    gboolean value = gconf_client_get_bool(client, keyname, NULL);
    
    g_object_unref(client);
    g_free(keyname);
    return value;
}

gint 
config_file_get_enum(const gchar *path, const gchar *key,
                     GConfEnumStringPair lookup_table[])
{
    gchar *string = config_file_get_string(path, key);
    gint retval = -1;
    gconf_string_to_enum(lookup_table, string, &retval);
    g_free(string);
    return retval;
}

/* returns -1 if unset or invalid */
static int
get_enum_from_entry(GConfEntry *entry, GConfEnumStringPair lookup_table[])
{
    GConfValue *value = gconf_entry_get_value(entry);
    if(!value || value->type != GCONF_VALUE_STRING)
        return -1;
    int enumvalue = -1;
    if(!gconf_string_to_enum(lookup_table, gconf_value_get_string(value), 
      &enumvalue))
        return -1;
    return enumvalue;
}

static void
set_key_to_default(GConfClient *client, GConfEntry *entry)
{
    gconf_client_set(client, gconf_entry_get_key(entry), 
      gconf_client_get_default_from_schema(client, gconf_entry_get_key(entry),
      NULL), NULL);
}

static void
on_config_font_set_changed(GConfClient *client, guint id, GConfEntry *entry,
                           GtkWidget* combobox)
{
    int newvalue = get_enum_from_entry(entry, font_set_lookup_table);
    /* validate new value */
    if(newvalue == -1) {
        set_key_to_default(client, entry);
        return;
    }
    /* update application to reflect new value */
    if(gtk_combo_box_get_active(GTK_COMBO_BOX(combobox)) != newvalue)
        gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), newvalue);
    update_font(lookup_widget(combobox, "source_example"));
    update_font(lookup_widget(combobox, "tab_example"));
    for_each_story_window_idle((GSourceFunc)update_app_window_fonts);
    for_each_extension_window_idle((GSourceFunc)update_ext_window_fonts);
}

static void
on_config_custom_font_changed(GConfClient *client, guint id, GConfEntry *entry, 
                              GtkWidget *fontbutton)
{
    const gchar *newvalue = 
      gconf_value_get_string(gconf_entry_get_value(entry));
	if(!newvalue)
		return; /* otherwise gconf_client_notify() causes segfault */
	
    /* update application to reflect new value */
    if(strncmp(newvalue, 
      gtk_font_button_get_font_name(GTK_FONT_BUTTON(fontbutton)),
      strlen(newvalue)) != 0)
        gtk_font_button_set_font_name(GTK_FONT_BUTTON(fontbutton), newvalue);
    update_font(lookup_widget(fontbutton, "source_example"));
    update_font(lookup_widget(fontbutton, "tab_example"));
    for_each_story_window_idle((GSourceFunc)update_app_window_fonts);
    for_each_extension_window_idle((GSourceFunc)update_ext_window_fonts);
}

static void
on_config_font_styling_changed(GConfClient *client, guint id, GConfEntry *entry,
                               GtkWidget *combobox)
{
    int newvalue = get_enum_from_entry(entry, font_styling_lookup_table);
    /* validate new value */
    if(newvalue == -1) {
        set_key_to_default(client, entry);
        return;
    }
    /* update application to reflect new value */
    if(gtk_combo_box_get_active(GTK_COMBO_BOX(combobox)) != newvalue)
        gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), newvalue);
    update_style(GTK_SOURCE_VIEW(lookup_widget(combobox, "source_example")));
    for_each_story_window_idle((GSourceFunc)update_app_window_fonts);
    for_each_extension_window_idle((GSourceFunc)update_ext_window_fonts);
}

static void
on_config_font_size_changed(GConfClient *client, guint id, GConfEntry *entry,
                            GtkWidget *combobox)
{
    int newvalue = get_enum_from_entry(entry, font_size_lookup_table);
    /* validate new value */
    if(newvalue == -1) {
        set_key_to_default(client, entry);
        return;
    }
    /* update application to reflect new value */
    if(gtk_combo_box_get_active(GTK_COMBO_BOX(combobox)) != newvalue)
        gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), newvalue);
    update_font(lookup_widget(combobox, "source_example"));
    update_font(lookup_widget(combobox, "tab_example"));
    for_each_story_window_idle((GSourceFunc)update_app_window_fonts);
    for_each_extension_window_idle((GSourceFunc)update_ext_window_fonts);
    for_each_story_window_idle((GSourceFunc)update_app_window_font_sizes);
}

static void
on_config_change_colors_changed(GConfClient *client, guint id, 
                                GConfEntry *entry, GtkWidget *combobox)
{
    int newvalue = get_enum_from_entry(entry, change_colors_lookup_table);
    /* validate new value */
    if(newvalue == -1) {
        set_key_to_default(client, entry);
        return;
    }
    /* update application to reflect new value */
    if(gtk_combo_box_get_active(GTK_COMBO_BOX(combobox)) != newvalue)
        gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), newvalue);
    update_style(GTK_SOURCE_VIEW(lookup_widget(combobox, "source_example")));
    for_each_story_window_idle((GSourceFunc)update_app_window_fonts);
    for_each_extension_window_idle((GSourceFunc)update_ext_window_fonts);
}

static void
on_config_color_set_changed(GConfClient *client, guint id, GConfEntry *entry,
                            GtkWidget *combobox)
{
    int newvalue = get_enum_from_entry(entry, color_set_lookup_table);
    /* validate new value */
    if(newvalue == -1) {
        set_key_to_default(client, entry);
        return;
    }
    /* update application to reflect new value */
    if(gtk_combo_box_get_active(GTK_COMBO_BOX(combobox)) != newvalue)
        gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), newvalue);
    update_style(GTK_SOURCE_VIEW(lookup_widget(combobox, "source_example")));
    for_each_story_window_idle((GSourceFunc)update_app_window_fonts);
    for_each_extension_window_idle((GSourceFunc)update_ext_window_fonts);
}

static void
on_config_tab_width_changed(GConfClient *client, guint id, GConfEntry *entry,
                            GtkWidget *range)
{
    int newvalue = gconf_value_get_int(gconf_entry_get_value(entry));
    /* validate new value */
    if(newvalue < 0) {
        set_key_to_default(client, entry);
        return;
    }
    /* update application to reflect new value */
    if((int)gtk_range_get_value(GTK_RANGE(range)) != newvalue)
        gtk_range_set_value(GTK_RANGE(range), (gdouble)newvalue);
    update_tabs(GTK_SOURCE_VIEW(lookup_widget(range, "tab_example")));
    update_tabs(GTK_SOURCE_VIEW(lookup_widget(range, "source_example")));
    for_each_story_window_idle((GSourceFunc)update_app_window_tabs);
    for_each_extension_window_idle((GSourceFunc)update_ext_window_tabs);
}

static void
on_config_inspectors_changed(GConfClient *client, guint id, GConfEntry *entry, 
                             GtkWidget *toggle)
{
    gboolean newvalue = gconf_value_get_bool(gconf_entry_get_value(entry));
    /* update application to reflect new value */
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle)) != newvalue)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), newvalue);
    update_inspectors();
}

static void
on_config_syntax_highlighting_changed(GConfClient *client, guint id,
                                      GConfEntry *entry)
{
    /* update application to reflect new value */
    for_each_story_buffer(&update_source_highlight);
    for_each_extension_buffer(&update_source_highlight);
    for_each_story_window_idle((GSourceFunc)update_app_window_fonts);
    for_each_extension_window_idle((GSourceFunc)update_ext_window_fonts);
}

static void
on_config_intelligence_changed(GConfClient *client, guint id, GConfEntry *entry,
                               GtkWidget *toggle)
{
    gboolean newvalue = gconf_value_get_bool(gconf_entry_get_value(entry));
    /* update application to reflect new value */
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle)) != newvalue)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), newvalue);
    /* make the other checkboxes dependent on this checkbox active or inactive*/
    gtk_widget_set_sensitive(lookup_widget(toggle,
      "prefs_intelligent_inspector_toggle"), newvalue);
    gtk_widget_set_sensitive(lookup_widget(toggle,
      "prefs_auto_number_toggle"), newvalue);
    
    update_style(GTK_SOURCE_VIEW(lookup_widget(toggle, "source_example")));
    for_each_story_window_idle((GSourceFunc)update_app_window_fonts);
    for_each_extension_window_idle((GSourceFunc)update_ext_window_fonts);
}

static void
on_config_intelligent_inspector_changed(GConfClient *client, guint id,
                                        GConfEntry *entry, GtkWidget *toggle)
{
    gboolean newvalue = gconf_value_get_bool(gconf_entry_get_value(entry));
    /* update application to reflect new value */
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle)) != newvalue)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), newvalue);
    /* TODO: Change headings inspector */
}

static void
on_config_elastic_tabstops_changed(GConfClient *client, guint id, 
								   GConfEntry *entry, GtkWidget *toggle)
{
	gboolean newvalue = gconf_value_get_bool(gconf_entry_get_value(entry));
	/* update application to reflect new value */
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle)) != newvalue)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), newvalue);
	if(newvalue) {
		for_each_story(add_elastic_tabstops_to_story);
		for_each_extension(add_elastic_tabstops_to_extension);
	} else {
		for_each_story(remove_elastic_tabstops_from_story);
		for_each_extension(remove_elastic_tabstops_from_extension);
	}
}

static void
on_config_elastic_tab_padding_changed(GConfClient *client, guint id,
									  GConfEntry *entry)
{
	int newvalue = gconf_value_get_int(gconf_entry_get_value(entry));
	/* validate new value */
	if(newvalue < 0) {
		set_key_to_default(client, entry);
		return;
	}
	/* update application to reflect new value */
	for_each_story_window_idle((GSourceFunc)update_app_window_elastic);
	for_each_extension_window_idle((GSourceFunc)update_ext_window_elastic);
}

static void
on_config_author_name_changed(GConfClient *client, guint id, GConfEntry *entry,
                              GtkWidget *editable)
{
    const gchar *newvalue = 
        gconf_value_get_string(gconf_entry_get_value(entry));
    /* update application to reflect new value */
    if(strcmp(gtk_entry_get_text(GTK_ENTRY(editable)), newvalue) != 0)
        gtk_entry_set_text(GTK_ENTRY(editable), newvalue);
}

static void
on_config_use_git_changed(GConfClient *client, guint id, GConfEntry *entry,
						  GtkWidget *glulxcombo)
{
	gboolean newvalue = gconf_value_get_bool(gconf_entry_get_value(entry));
	/* update application to reflect new value */
	gtk_combo_box_set_active(GTK_COMBO_BOX(glulxcombo), newvalue? 0 : 1);
	/* Don't bother restarting the terp, the change will take effect on the
	 next invocation */
}

static void
on_config_clean_build_files_changed(GConfClient *client, guint id,
                                    GConfEntry *entry, GtkWidget *toggle)
{
    gboolean newvalue = gconf_value_get_bool(gconf_entry_get_value(entry));
    /* update application to reflect new value */
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle)) != newvalue)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), newvalue);
    /* make the other checkboxes dependent on this checkbox active or inactive*/
    gtk_widget_set_sensitive(lookup_widget(toggle, "prefs_clean_index_toggle"),
        newvalue);
}

static void
on_config_show_debug_log_changed(GConfClient *client, guint id,
                                 GConfEntry *entry, GtkWidget *toggle)
{
    gboolean newvalue = gconf_value_get_bool(gconf_entry_get_value(entry));
    /* update application to reflect new value */
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle)) != newvalue)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), newvalue);
    for_each_story_window(newvalue? add_debug_tabs : remove_debug_tabs);
}

static gboolean
update_skein_spacing(GtkWidget *window)
{
    Story *thestory = get_story(window);
    skein_invalidate_layout(thestory->theskein);
    skein_layout_and_redraw(thestory->theskein, thestory);
    return FALSE; /* one-shot idle function */
}

static void
on_config_skein_spacing_changed(GConfClient *client, guint id,
                                GConfEntry *entry, GtkWidget *window)
{
    for_each_story_window_idle((GSourceFunc)update_skein_spacing);
}

static void
on_config_generic_bool_changed(GConfClient *client, guint id, GConfEntry *entry,
                               GtkWidget *toggle)
{
    gboolean newvalue = gconf_value_get_bool(gconf_entry_get_value(entry));
    /* update application to reflect new value */
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle)) != newvalue)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), newvalue);
}

struct KeyToMonitor {
    const gchar *name;
    GConfClientNotifyFunc callback;
    const gchar *widgetname;
};

static struct KeyToMonitor keys_to_monitor[] = {
    { "EditorSettings/FontSet", 
      (GConfClientNotifyFunc)on_config_font_set_changed, "prefs_font_set" },
    { "EditorSettings/CustomFont", 
      (GConfClientNotifyFunc)on_config_custom_font_changed,
      "prefs_custom_font" },
    { "EditorSettings/FontStyling", 
      (GConfClientNotifyFunc)on_config_font_styling_changed,
      "prefs_font_styling" },
    { "EditorSettings/FontSize", 
      (GConfClientNotifyFunc)on_config_font_size_changed, "prefs_font_size" },
    { "EditorSettings/ChangeColors", 
      (GConfClientNotifyFunc)on_config_change_colors_changed,
      "prefs_change_colors" },
    { "EditorSettings/ColorSet", 
      (GConfClientNotifyFunc)on_config_color_set_changed, "prefs_color_set" },
    { "EditorSettings/TabWidth", 
      (GConfClientNotifyFunc)on_config_tab_width_changed, "tab_ruler" },
    { "InspectorSettings/NotesVisible", 
      (GConfClientNotifyFunc)on_config_inspectors_changed, 
      "prefs_notes_toggle" },
    { "InspectorSettings/HeadingsVisible", 
      (GConfClientNotifyFunc)on_config_inspectors_changed,
      "prefs_headings_toggle" },
    { "InspectorSettings/SkeinVisible", 
      (GConfClientNotifyFunc)on_config_inspectors_changed, 
      "prefs_skein_toggle" },
    { "InspectorSettings/SearchVisible", 
      (GConfClientNotifyFunc)on_config_inspectors_changed,
      "prefs_search_toggle" },
    { "SyntaxSettings/SyntaxHighlighting", 
      (GConfClientNotifyFunc)on_config_syntax_highlighting_changed, NULL },
    { "SyntaxSettings/Intelligence",
      (GConfClientNotifyFunc)on_config_intelligence_changed,
      "prefs_follow_symbols_toggle" },
    { "SyntaxSettings/IntelligentHeadingsInspector",
      (GConfClientNotifyFunc)on_config_intelligent_inspector_changed,
      "prefs_intelligent_inspector_toggle" },
    { "SyntaxSettings/AutoIndent",
      (GConfClientNotifyFunc)on_config_generic_bool_changed,
      "prefs_auto_indent_toggle" },
    { "SyntaxSettings/AutoNumberSections",
      (GConfClientNotifyFunc)on_config_generic_bool_changed,
      "prefs_auto_number_toggle" },
	{ "EditorSettings/ElasticTabstops", 
	  (GConfClientNotifyFunc)on_config_elastic_tabstops_changed,
	  "prefs_elastic_tabstops_toggle" },
	{ "EditorSettings/ElasticTabPadding",
	  (GConfClientNotifyFunc)on_config_elastic_tab_padding_changed, NULL },
    { "AppSettings/AuthorName",
      (GConfClientNotifyFunc)on_config_author_name_changed, "prefs_author" },
	{ "IDESettings/UseGit", (GConfClientNotifyFunc)on_config_use_git_changed,
	  "prefs_glulx_combo" },
    { "IDESettings/CleanBuildFiles",
      (GConfClientNotifyFunc)on_config_clean_build_files_changed,
      "prefs_clean_build_toggle" },
    { "IDESettings/CleanIndexFiles",
      (GConfClientNotifyFunc)on_config_generic_bool_changed,
      "prefs_clean_index_toggle" },
    { "IDESettings/DebugLogVisible",
      (GConfClientNotifyFunc)on_config_show_debug_log_changed,
      "prefs_show_log_toggle" },
    { "SkeinSettings/HorizontalSpacing",
      (GConfClientNotifyFunc)on_config_skein_spacing_changed, NULL },
    { "SkeinSettings/VerticalSpacing",
      (GConfClientNotifyFunc)on_config_skein_spacing_changed, NULL }
};

#define NUM_KEYS_TO_MONITOR \
    (sizeof(keys_to_monitor) / sizeof(keys_to_monitor[0]))

/* Check if the config keys exist and if not, set them to defaults. */
void
init_config_file()
{
    extern GtkWidget *prefs_dialog;

    /* Initialize the GConf client */
    GConfClient *client = gconf_client_get_default();
    gconf_client_set_error_handling(client, GCONF_CLIENT_HANDLE_ALL);

    /* Start monitoring the directories */
    gconf_client_add_dir(client, "/apps/gnome-inform7",
                         GCONF_CLIENT_PRELOAD_RECURSIVE, NULL);

    /* Add listeners to specific keys and pass them the global prefs dialog */
    int i;
    for(i = 0; i < NUM_KEYS_TO_MONITOR; i++) {
        gchar *keyname = g_strconcat(GCONF_BASE_PATH, keys_to_monitor[i].name,
                                     NULL);
        gconf_client_notify_add(client, keyname, keys_to_monitor[i].callback,
            (keys_to_monitor[i].widgetname)?
                (gpointer)lookup_widget(prefs_dialog, 
                    keys_to_monitor[i].widgetname) :
                NULL,
            NULL, NULL);
        g_free(keyname);
    }

    g_object_unref(client);
}

void
trigger_config_keys()
{
    GConfClient *client = gconf_client_get_default();
    /* Trigger all the keys for which we have listeners */
    int i;
    for(i = 0; i < NUM_KEYS_TO_MONITOR; i++) {
        gchar *keyname = g_strconcat(GCONF_BASE_PATH, keys_to_monitor[i].name,
                                     NULL);
        gconf_client_notify(client, keyname);
        g_free(keyname);
    }
    g_object_unref(client);
}


