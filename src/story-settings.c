/* Copyright (C) 2006-2009, 2010, 2011, 2014, 2015, 2019, 2021 P. F. Chimento
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

#include <glib.h>
#include <gtk/gtk.h>
#include <plist/plist.h>

#include "panel.h"
#include "story.h"

/* Helper function to insert @setting into @dict, under @key1/@key2. @key1 is
 created if it doesn't exist */
static void
insert_setting(plist_t dict, const char *key1, const char *key2, plist_t setting)
{
	plist_t category = plist_dict_get_item(dict, key1);
	if(!category) {
		category = plist_new_dict();
		plist_dict_set_item(dict, key1, category);
	}
	plist_dict_set_item(category, key2, setting);
}

/* Initialize a new settings dictionary with the defaults for our port */
plist_t
create_default_settings()
{
	plist_t obj;

	/* Create the settings dictionary */
	plist_t dict = plist_new_dict();

	/* IFCompilerOptions->IFSettingNaturalInform (TRUE) */
	obj = plist_new_bool(TRUE);
	insert_setting(dict, "IFCompilerOptions", "IFSettingNaturalInform", obj);
	/* IFLibrarySettings->IFSettingLibraryToUse ("Natural") */
	obj = plist_new_string("Natural");
	insert_setting(dict, "IFLibrarySettings", "IFSettingLibraryToUse", obj);
	/* IFMiscSettings->IFSettingElasticTabs (FALSE) */
	obj = plist_new_bool(FALSE);
	insert_setting(dict, "IFMiscSettings", "IFSettingElasticTabs", obj);
	/* IFOutputSettings->IFSettingCreateBlorb (TRUE) */
	obj = plist_new_bool(TRUE);
	insert_setting(dict, "IFOutputSettings", "IFSettingCreateBlorb", obj);
	/* IFOutputSettings->IFSettingZCodeVersion (256) */
	obj = plist_new_uint(I7_STORY_FORMAT_GLULX);
	insert_setting(dict, "IFOutputSettings", "IFSettingZCodeVersion", obj);
	/* IFOutputSettings->IFSettingNobbleRng (FALSE) */
	obj = plist_new_bool(FALSE);
	insert_setting(dict, "IFOutputSettings", "IFSettingNobbleRng", obj);

	return dict;
}

void
on_z8_button_toggled(GtkToggleButton *button, I7Story *story)
{
	gboolean value = gtk_toggle_button_get_active(button);
	if(value)
		i7_story_set_story_format(story, I7_STORY_FORMAT_Z8);
}

void
on_glulx_button_toggled(GtkToggleButton *button, I7Story *story)
{
	gboolean value = gtk_toggle_button_get_active(button);
	if(value)
		i7_story_set_story_format(story, I7_STORY_FORMAT_GLULX);
}

/* Select all the right buttons according to the story settings */
void
on_notify_story_format(I7Story *story)
{
	switch(i7_story_get_story_format(story)) {
		case I7_STORY_FORMAT_GLULX:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(story->panel[LEFT]->glulx), TRUE);
			break;
		case I7_STORY_FORMAT_Z8:
		default:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(story->panel[LEFT]->z8), TRUE);
	}
	/* The property bindings ensure that we don't have to manually set the other ones */
}

void
on_notify_elastic_tabstops(I7Story *story)
{
	gboolean value = i7_story_get_elastic_tabstops(story);
	GAction *enable_elastic_tabstops = g_action_map_lookup_action(G_ACTION_MAP(story), "enable-elastic-tabstops");
	g_simple_action_set_state(G_SIMPLE_ACTION(enable_elastic_tabstops), g_variant_new_boolean(value));
	i7_source_view_set_elastic_tabstops(story->panel[LEFT]->sourceview, value);
	i7_source_view_set_elastic_tabstops(story->panel[RIGHT]->sourceview, value);
}

/* These 'get' functions provide for a default value even though we initialize a
 dictionary with all the required keys; it may be that another port of Inform 7
 doesn't write all the keys */
I7StoryFormat
i7_story_get_story_format(I7Story *self)
{
	g_return_val_if_fail(self || I7_IS_STORY(self), 0);

	plist_t settings = i7_story_get_settings(self);
	plist_t obj = plist_access_path(settings, 2, "IFOutputSettings", "IFSettingZCodeVersion");
	if(!obj)
		return I7_STORY_FORMAT_GLULX; /* Default value */
	uint64_t retval;
	plist_get_uint_val(obj, &retval);
	return retval;
}

void
i7_story_set_story_format(I7Story *self, I7StoryFormat format)
{
	g_return_if_fail(self || I7_IS_STORY(self));
	g_return_if_fail(format == I7_STORY_FORMAT_Z5 || format == I7_STORY_FORMAT_Z6 || format == I7_STORY_FORMAT_Z8 || format == I7_STORY_FORMAT_GLULX);

	i7_document_set_modified(I7_DOCUMENT(self), TRUE);

	plist_t settings = i7_story_get_settings(self);
	plist_t obj = plist_access_path(settings, 2, "IFOutputSettings", "IFSettingZCodeVersion");
	if(!obj) {
		obj = plist_new_uint(format);
		insert_setting(settings, "IFOutputSettings", "IFSettingZCodeVersion", obj);
		g_object_notify(G_OBJECT(self), "story-format");
		return;
	}
	uint64_t value;
	plist_get_uint_val(obj, &value);
	if (value != format) {
		plist_set_uint_val(obj, format);
		g_object_notify(G_OBJECT(self), "story-format");
	}
}

gboolean
i7_story_get_create_blorb(I7Story *self)
{
	g_return_val_if_fail(self || I7_IS_STORY(self), 0);

	plist_t settings = i7_story_get_settings(self);
	plist_t obj = plist_access_path(settings, 2, "IFOutputSettings", "IFSettingCreateBlorb");
	if(!obj)
		return TRUE; /* Default value */
	uint8_t retval;
	plist_get_bool_val(obj, &retval);
	return retval;
}

void
i7_story_set_create_blorb(I7Story *self, gboolean create_blorb)
{
	g_return_if_fail(self || I7_IS_STORY(self));

	i7_document_set_modified(I7_DOCUMENT(self), TRUE);

	plist_t settings = i7_story_get_settings(self);
	plist_t obj = plist_access_path(settings, 2, "IFOutputSettings", "IFSettingCreateBlorb");
	if(!obj) {
		obj = plist_new_bool(create_blorb);
		insert_setting(settings, "IFOutputSettings", "IFSettingCreateBlorb", obj);
		g_object_notify(G_OBJECT(self), "create-blorb");
		return;
	}
	uint8_t value;
	plist_get_bool_val(obj, &value);
	if (value != create_blorb) {
		plist_set_bool_val(obj, create_blorb);
		g_object_notify(G_OBJECT(self), "create-blorb");
	}
}

gboolean
i7_story_get_nobble_rng(I7Story *self)
{
	g_return_val_if_fail(self || I7_IS_STORY(self), 0);

	plist_t settings = i7_story_get_settings(self);
	plist_t obj = plist_access_path(settings, 2, "IFOutputSettings", "IFSettingNobbleRng");
	if(!obj)
		return FALSE; /* Default value */
	uint8_t retval;
	plist_get_bool_val(obj, &retval);
	return retval;
}

void
i7_story_set_nobble_rng(I7Story *self, gboolean nobble_rng)
{
	g_return_if_fail(self || I7_IS_STORY(self));

	i7_document_set_modified(I7_DOCUMENT(self), TRUE);

	plist_t settings = i7_story_get_settings(self);
	plist_t obj = plist_access_path(settings, 2, "IFOutputSettings", "IFSettingNobbleRng");
	if(!obj) {
		obj = plist_new_bool(nobble_rng);
		insert_setting(settings, "IFOutputSettings", "IFSettingNobbleRng", obj);
		g_object_notify(G_OBJECT(self), "nobble-rng");
		return;
	}
	uint8_t value;
	plist_get_bool_val(obj, &value);
	if (value != nobble_rng) {
		plist_set_bool_val(obj, nobble_rng);
		g_object_notify(G_OBJECT(self), "nobble-rng");
	}
}

gboolean
i7_story_get_elastic_tabstops(I7Story *self)
{
	g_return_val_if_fail(self || I7_IS_STORY(self), 0);

	plist_t settings = i7_story_get_settings(self);
	plist_t obj = plist_access_path(settings, 2, "IFMiscSettings", "IFSettingElasticTabs");
	if(!obj)
		return FALSE; /* Default value */
	uint8_t retval;
	plist_get_bool_val(obj, &retval);
	return retval;
}

void
i7_story_set_elastic_tabstops(I7Story *self, gboolean elastic_tabstops)
{
	g_return_if_fail(self || I7_IS_STORY(self));

	i7_document_set_modified(I7_DOCUMENT(self), TRUE);

	plist_t settings = i7_story_get_settings(self);
	plist_t obj = plist_access_path(settings, 2, "IFMiscSettings", "IFSettingElasticTabs");
	if(!obj) {
		obj = plist_new_bool(elastic_tabstops);
		insert_setting(settings, "IFMiscSettings", "IFSettingElasticTabs", obj);
		g_object_notify(G_OBJECT(self), "elastic-tabstops");
		return;
	}
	uint8_t value;
	plist_get_bool_val(obj, &value);
	if (value != elastic_tabstops) {
		plist_set_bool_val(obj, elastic_tabstops);
		g_object_notify(G_OBJECT(self), "elastic-tabstops");
	}
}
