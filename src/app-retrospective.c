/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2022 Philip Chimento <philip.chimento@gmail.com>
 */

#include "config.h"

#include <ctype.h>
#include <stdbool.h>

#include <gio/gio.h>

#include "app.h"
#include "app-retrospective.h"
#include "story.h"
#include "configfile.h"

typedef enum {
	INFORM_10_1,
	INFORM_9_2,
	INFORM_9_1
} ArgsStyle;

/* Record parsed out of retrospective.txt */
struct _I7Retrospective {
	GObject parent;

	char *id;
	char *display_name;
	char *description;
};

G_DEFINE_TYPE(I7Retrospective, i7_retrospective, G_TYPE_OBJECT);

static ArgsStyle
get_args_style(const char *version_id)
{
	if (g_str_equal(version_id, "6L02"))
		return INFORM_9_1;
	if (g_str_equal(version_id, "6L38") || g_str_equal(version_id, "6M62"))
		return INFORM_9_2;
	return INFORM_10_1;
}

/* Helper functions for asserting the retrospective.txt file has the correct
 * format */

static bool
is_empty(const char *str)
{
	return str && *str == '\0';
}

static bool
is_comma(const char *str)
{
	if (!str)
		return FALSE;
	if (*str++ != ',')
		return FALSE;
	while(isspace(*str))
		str++;
	return *str == '\0';
}

static char *
is_present(const char *str)
{
	g_assert(str);
	return g_strdup(str);
}

/* Parse the retrospective.txt file and store the relevant data in a list store
 * (type: I7Retrospective) */
void
parse_retrospective_txt(GListStore **store_out)
{
	g_assert(store_out);

	g_autoptr(GError) error = NULL;
	g_autoptr(GBytes) resource = g_resources_lookup_data("/com/inform7/IDE/retrospective.txt",
		G_RESOURCE_LOOKUP_FLAGS_NONE, &error);
	if (!resource) {
		g_error("failed to look up retrospective data: %s", error->message);
	}

	GListStore *store = g_list_store_new(I7_TYPE_RETROSPECTIVE);

	size_t len;
	const char *retrospective_content = g_bytes_get_data(resource, &len);
	g_auto(GStrv) lines = g_strsplit_set(retrospective_content, "\r\n", -1);
	for (char **line = lines; *line != NULL; line++) {
		if (**line == '\0')
			continue;  /* Skip blank lines */

		g_auto(GStrv) parts = g_strsplit(g_strchomp(*line), "'", -1);

		I7Retrospective *record = g_object_new(I7_TYPE_RETROSPECTIVE, NULL);

		/* Since the retrospective.txt file is built into Inform, we are
		 * draconian about any irregularities in the file format. */
		g_assert(is_empty(parts[0]));
		record->id = is_present(parts[1]);
		g_assert(is_comma(parts[2]));
		record->display_name = is_present(parts[3]);
		g_assert(is_comma(parts[4]));
		record->description = is_present(parts[5]);
		g_assert(is_empty(parts[6]));

		g_list_store_append(store, record);
	}

	*store_out = store;
}

const char *
i7_retrospective_get_id(const I7Retrospective *self)
{
	return self->id;
}

const char *
i7_retrospective_get_display_name(const I7Retrospective *self)
{
	return self->display_name;
}

const char *
i7_retrospective_get_description(const I7Retrospective *self)
{
	return self->description;
}

static const char *
get_inform_format_arg(ArgsStyle style, I7StoryFormat format, bool debug)
{
	switch(format) {
		case I7_STORY_FORMAT_Z8:
			if (style == INFORM_9_1)
				return "-extension=z8";
			if (style == INFORM_10_1)
				return debug ? "-format=Inform6/16d" : "-format=Inform6/16";
			return "-format=z8";
		case I7_STORY_FORMAT_GLULX:
			if (style == INFORM_9_1)
				return "-extension=ulx";
			if (style == INFORM_10_1)
				return debug ? "-format=Inform6/32d" : "-format=Inform6/32";
			return "-format=ulx";
		default:
			;
	}
	g_assert_not_reached();
	return "error";
}

char **
i7_app_get_inform_command_line(I7App *self, const char *version_id, int format, bool debug, bool reproducible, bool basic_inform, GFile *project_file)
{
	g_autoptr(GPtrArray) builder = g_ptr_array_new_with_free_func(g_free);

	ArgsStyle style = get_args_style(version_id);

	g_autoptr(GFile) inform_compiler = NULL;
	if (style == INFORM_10_1)
		inform_compiler = i7_app_get_binary_file(self, "inform7");
	else
		inform_compiler = i7_app_get_retrospective_binary_file(self, version_id, "ni");
	char *inform_path = g_file_get_path(inform_compiler);
	g_ptr_array_add(builder, inform_path);

	g_autoptr(GFile) internal_dir = NULL;
	if (style == INFORM_10_1)
		internal_dir = i7_app_get_internal_dir(self);
	else
		internal_dir = i7_app_get_retrospective_internal_dir(self, version_id);
	char *internal_path = g_file_get_path(internal_dir);
	if (style == INFORM_9_1)
		g_ptr_array_add(builder, g_strdup("-rules"));
	else
		g_ptr_array_add(builder, g_strdup("-internal"));
	g_ptr_array_add(builder, internal_path);

	GSettings *prefs = i7_app_get_prefs(self);
	g_autofree char *added_nest_dir = g_settings_get_string(prefs, PREFS_ADDED_NEST);
	if (added_nest_dir[0] != '\0') {
		if (style == INFORM_9_1 || style == INFORM_9_2)
			g_ptr_array_add(builder, g_strdup("-external"));
		else if (style == INFORM_10_1)
			g_ptr_array_add(builder, g_strdup("-nest"));
		g_ptr_array_add(builder, g_strdup(added_nest_dir));
	}

	g_ptr_array_add(builder, g_strdup(get_inform_format_arg(style, (I7StoryFormat)format, debug)));

	char *project_path = g_file_get_path(project_file);
	if (style == INFORM_9_1)
		g_ptr_array_add(builder, g_strdup("-package"));
	else
		g_ptr_array_add(builder, g_strdup("-project"));
	g_ptr_array_add(builder, project_path);

	if (!debug)
		g_ptr_array_add(builder, g_strdup("-release")); /* Omit "not for relase" material */

	if(reproducible)
		g_ptr_array_add(builder, g_strdup("-rng"));

    if (basic_inform) {
        g_assert(style != INFORM_9_1 && style != INFORM_9_2);
        g_ptr_array_add(builder, g_strdup("-basic"));
    }

	g_ptr_array_add(builder, NULL);
	return (char **)g_ptr_array_steal(builder, NULL);
}

char **
i7_app_get_inblorb_command_line(I7App *self, const char *version_id, GFile *blorb_file)
{
	g_autoptr(GPtrArray) builder = g_ptr_array_new_with_free_func(g_free);

	ArgsStyle style = get_args_style(version_id);

	g_autoptr(GFile) inblorb = NULL;
	if (style == INFORM_10_1)
		inblorb = i7_app_get_binary_file(self, "inblorb");
	else
		inblorb = i7_app_get_retrospective_binary_file(self, version_id, "cBlorb");
	char *inblorb_path = g_file_get_path(inblorb);
	g_ptr_array_add(builder, inblorb_path);

	if (style != INFORM_10_1)
		g_ptr_array_add(builder, g_strdup("-unix"));

	g_ptr_array_add(builder, g_strdup("Release.blurb"));

	char *blorb_path = g_file_get_path(blorb_file);
	g_ptr_array_add(builder, blorb_path);

	g_ptr_array_add(builder, NULL);
	return (char **)g_ptr_array_steal(builder, NULL);
}

/* TYPE SYSTEM */

static void
i7_retrospective_init(I7Retrospective *self)
{
}

static void
i7_retrospective_finalize(GObject *obj)
{
	I7Retrospective *self = I7_RETROSPECTIVE(obj);
	g_clear_pointer(&self->id, g_free);
	g_clear_pointer(&self->display_name, g_free);
	g_clear_pointer(&self->description, g_free);
}

static void
i7_retrospective_class_init(I7RetrospectiveClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = i7_retrospective_finalize;
}
