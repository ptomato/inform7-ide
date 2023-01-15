/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2006-2011, 2013, 2015, 2019 Philip Chimento <philip.chimento@gmail.com>
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
on_config_docs_font_size_changed(GSettings *settings, const char *key)
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
	GList *windows = gtk_application_get_windows(GTK_APPLICATION(theapp));
	for (GList *iter = windows; iter != NULL; iter = iter->next) {
		if (I7_IS_DOCUMENT(iter->data))
			i7_document_update_tabs(I7_DOCUMENT(iter->data));
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
	{ PREFS_FONT_SET, on_config_font_set_changed },
	{ PREFS_CUSTOM_FONT, on_config_custom_font_changed },
	{ PREFS_DOCS_FONT_SIZE, on_config_docs_font_size_changed },
	{ PREFS_STYLE_SCHEME, on_config_style_scheme_changed },
	{ PREFS_TAB_WIDTH, on_config_tab_width_changed },
	{ PREFS_SHOW_DEBUG_LOG, on_config_debug_log_visible_changed },
	{ PREFS_INTERPRETER, on_config_use_interpreter_changed },
	{ PREFS_TABSTOPS_PADDING,on_config_elastic_tabstops_padding_changed }
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
