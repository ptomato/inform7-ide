/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Philip Chimento <philip.chimento@gmail.com>
 */

#include "config.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <handy.h>

#include "app-retrospective.h"
#include "project-settings.h"
#include "story.h"

G_DEFINE_ENUM_TYPE(I7StoryFormat, i7_story_format,
	G_DEFINE_ENUM_VALUE(I7_STORY_FORMAT_Z8, "z8"),
	G_DEFINE_ENUM_VALUE(I7_STORY_FORMAT_GLULX, "glulx"))

struct _I7ProjectSettings {
	HdyPreferencesPage parent;

	/* template children */
	GtkSwitch *basic_inform;
	HdyActionRow *basic_inform_row;
	GtkSwitch *blorb;
	GtkSwitch *nobble_rng;
	HdyComboRow *story_format;
	HdyComboRow *language_version;
};

G_DEFINE_TYPE(I7ProjectSettings, i7_project_settings, HDY_TYPE_PREFERENCES_PAGE);

/* PRIVATE FUNCTIONS */

static char *
story_format_get_name(HdyEnumValueObject *enum_obj, void *unused)
{
	int value = hdy_enum_value_object_get_value(enum_obj);
	switch (value) {
		case I7_STORY_FORMAT_Z8:
			return g_strdup("Z-Code Version 8 (most portable)");
		case I7_STORY_FORMAT_GLULX:
			return g_strdup("Glulx (most capable)");
		default:
			g_assert_not_reached();
	}
}

static char *
retrospective_get_name(void *item, void *unused)
{
	return g_strdup(i7_retrospective_get_display_name(I7_RETROSPECTIVE(item)));
}

static gboolean
story_format_to_index(GBinding *unused, const GValue *from, GValue *to, void *unused2)
{
	I7StoryFormat story_format = g_value_get_uint(from);
	switch (story_format) {
		case I7_STORY_FORMAT_Z8:
			g_value_set_int(to, 0);
			return TRUE;
		case I7_STORY_FORMAT_GLULX:
			g_value_set_int(to, 1);
			return TRUE;
		default:
			g_critical("Unsupported value %d for story format", story_format);
			return FALSE;
	}
}

static gboolean
index_to_story_format(GBinding *unused, const GValue *from, GValue *to, void *unused2)
{
	static const I7StoryFormat formats[2] = { I7_STORY_FORMAT_Z8, I7_STORY_FORMAT_GLULX };
	int ix = g_value_get_int(from);
	if (ix < 0 || ix >= 2) {
		g_critical("Unsupported index %d for story format combo box", ix);
		return FALSE;
	}
	g_value_set_uint(to, formats[ix]);
	return TRUE;
}

static gboolean
language_version_to_index(GBinding *unused, const GValue *from, GValue *to, void *unused2)
{
	I7App *app = I7_APP(g_application_get_default());
	GListStore *retrospectives = i7_app_get_retrospectives(app);

	const char *id = g_value_get_string(from);
	unsigned ix = 0;
	void *record;
	while ((record = g_list_model_get_item(G_LIST_MODEL(retrospectives), ix)) != NULL) {
		const char *candidate = i7_retrospective_get_id(I7_RETROSPECTIVE(record));
		if (strcmp(candidate, id) == 0) {
			g_value_set_int(to, ix);
			return TRUE;
		}
		ix++;
	}
	return FALSE;
}

static gboolean
index_to_language_version(GBinding *unused, const GValue *from, GValue *to, void *unused2)
{
	I7App *app = I7_APP(g_application_get_default());
	GListStore *retrospectives = i7_app_get_retrospectives(app);

	int ix = g_value_get_int(from);
	void *record = g_list_model_get_item(G_LIST_MODEL(retrospectives), ix);
	g_value_set_string(to, i7_retrospective_get_id(I7_RETROSPECTIVE(record)));
	return TRUE;
}

/* CALLBACKS */

void
on_language_version_selected_index_notify(HdyComboRow *language_version, GParamSpec *pspec, I7ProjectSettings *self)
{
	I7App *app = I7_APP(g_application_get_default());
	GListStore *retrospectives = i7_app_get_retrospectives(app);

	int ix = hdy_combo_row_get_selected_index(language_version);
	I7Retrospective *record = I7_RETROSPECTIVE(g_list_model_get_item(G_LIST_MODEL(retrospectives), ix));
	hdy_action_row_set_subtitle(HDY_ACTION_ROW(language_version), i7_retrospective_get_description(record));

	const char *id = i7_retrospective_get_id(record);
	if (g_str_equal(id, "6L02") || g_str_equal(id, "6L38") || g_str_equal(id, "6M62")) {
		gtk_switch_set_active(self->basic_inform, FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(self->basic_inform_row), FALSE);
	} else {
		gtk_widget_set_sensitive(GTK_WIDGET(self->basic_inform_row), TRUE);
	}
}

/* TYPE SYSTEM */

static void
i7_project_settings_init(I7ProjectSettings *self)
{
	gtk_widget_init_template(GTK_WIDGET(self));
}

static void
i7_project_settings_constructed(GObject* object)
{
	I7ProjectSettings *self = I7_PROJECT_SETTINGS(object);

	I7App *app = I7_APP(g_application_get_default());
	GListStore *retrospectives = i7_app_get_retrospectives(app);

	hdy_combo_row_bind_name_model(HDY_COMBO_ROW(self->language_version), G_LIST_MODEL(retrospectives),
		retrospective_get_name, NULL, NULL);
	hdy_combo_row_set_for_enum(HDY_COMBO_ROW(self->story_format), i7_story_format_get_type(),
		story_format_get_name, NULL, NULL);

	G_OBJECT_CLASS(i7_project_settings_parent_class)->constructed(object);
}

static void
i7_project_settings_class_init(I7ProjectSettingsClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	gtk_widget_class_set_template_from_resource(widget_class, "/com/inform7/IDE/ui/project-settings.ui");
	gtk_widget_class_bind_template_child(widget_class, I7ProjectSettings, basic_inform);
	gtk_widget_class_bind_template_child(widget_class, I7ProjectSettings, basic_inform_row);
	gtk_widget_class_bind_template_child(widget_class, I7ProjectSettings, blorb);
	gtk_widget_class_bind_template_child(widget_class, I7ProjectSettings, nobble_rng);
	gtk_widget_class_bind_template_child(widget_class, I7ProjectSettings, story_format);
	gtk_widget_class_bind_template_child(widget_class, I7ProjectSettings, language_version);
	gtk_widget_class_bind_template_callback(widget_class, on_language_version_selected_index_notify);

	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->constructed = i7_project_settings_constructed;
}

/* PUBLIC API */

I7ProjectSettings *
i7_project_settings_new(void)
{
	return I7_PROJECT_SETTINGS(g_object_new(I7_TYPE_PROJECT_SETTINGS, NULL));
}

void
i7_project_settings_bind_properties(I7ProjectSettings *self, I7Story *story)
{
	const GBindingFlags flags = G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE;
	g_object_bind_property_full(story, "story-format", self->story_format, "selected-index", flags,
		story_format_to_index, index_to_story_format, NULL, NULL);
	g_object_bind_property(story, "create-blorb", self->blorb, "active", flags);
	g_object_bind_property(story, "nobble-rng", self->nobble_rng, "active", flags);
	g_object_bind_property_full(story, "language-version", self->language_version, "selected-index", flags,
		language_version_to_index, index_to_language_version, NULL, NULL);
    g_object_bind_property(story, "basic-inform", self->basic_inform, "active", flags);
}
