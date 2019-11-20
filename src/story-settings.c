/* Copyright (C) 2006-2009, 2010, 2011, 2014, 2015 P. F. Chimento
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

#include "panel.h"
#include "story.h"

/* Helper function to insert @obj into @dict, under @key1/@key2. @key1 is
 created if it doesn't exist */
static void
insert_setting(PlistObject *dict, const gchar *key1, const gchar *key2, PlistObject *setting)
{
	PlistObject *category = plist_object_lookup(dict, key1, -1);
	if(!category) {
		category = plist_object_new(PLIST_OBJECT_DICT);
		g_hash_table_insert(dict->dict.val, g_strdup(key1), category);
	}
	g_hash_table_insert(category->dict.val, g_strdup(key2), setting);
}

/* Initialize a new settings dictionary with the defaults for our port */
PlistObject *
create_default_settings()
{
	PlistObject *obj, *dict;

	/* Create the settings dictionary */
	dict = plist_object_new(PLIST_OBJECT_DICT);

	/* IFCompilerOptions->IFSettingNaturalInform (TRUE) */
	obj = plist_object_new(PLIST_OBJECT_BOOLEAN);
	obj->boolean.val = TRUE;
	insert_setting(dict, "IFCompilerOptions", "IFSettingNaturalInform", obj);
	/* IFLibrarySettings->IFSettingLibraryToUse ("Natural") */
	obj = plist_object_new(PLIST_OBJECT_STRING);
	obj->string.val = g_strdup("Natural");
	insert_setting(dict, "IFLibrarySettings", "IFSettingLibraryToUse", obj);
	/* IFMiscSettings->IFSettingElasticTabs (FALSE) */
	obj = plist_object_new(PLIST_OBJECT_BOOLEAN);
	obj->boolean.val = FALSE;
	insert_setting(dict, "IFMiscSettings", "IFSettingElasticTabs", obj);
	/* IFOutputSettings->IFSettingCreateBlorb (TRUE) */
	obj = plist_object_new(PLIST_OBJECT_BOOLEAN);
	obj->boolean.val = TRUE;
	insert_setting(dict, "IFOutputSettings", "IFSettingCreateBlorb", obj);
	/* IFOutputSettings->IFSettingZCodeVersion (256) */
	obj = plist_object_new(PLIST_OBJECT_INTEGER);
	obj->integer.val = I7_STORY_FORMAT_GLULX;
	insert_setting(dict, "IFOutputSettings", "IFSettingZCodeVersion", obj);
	/* IFOutputSettings->IFSettingNobbleRng (FALSE) */
	obj = plist_object_new(PLIST_OBJECT_BOOLEAN);
	obj->boolean.val = FALSE;
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
	gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(I7_DOCUMENT(story)->enable_elastic_tabstops), value);
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

	PlistObject *settings = i7_story_get_settings(self);
	PlistObject *obj = plist_object_lookup(settings, "IFOutputSettings", "IFSettingZCodeVersion", -1);
	if(!obj)
		return I7_STORY_FORMAT_GLULX; /* Default value */
	return obj->integer.val;
}

void
i7_story_set_story_format(I7Story *self, I7StoryFormat format)
{
	g_return_if_fail(self || I7_IS_STORY(self));
	g_return_if_fail(format == I7_STORY_FORMAT_Z5 || format == I7_STORY_FORMAT_Z6 || format == I7_STORY_FORMAT_Z8 || format == I7_STORY_FORMAT_GLULX);

	i7_document_set_modified(I7_DOCUMENT(self), TRUE);

	PlistObject *settings = i7_story_get_settings(self);
	PlistObject *obj = plist_object_lookup(settings, "IFOutputSettings", "IFSettingZCodeVersion", -1);
	if(!obj) {
		obj = plist_object_new(PLIST_OBJECT_INTEGER);
		obj->integer.val = format;
		insert_setting(settings, "IFOutputSettings", "IFSettingZCodeVersion", obj);
		g_object_notify(G_OBJECT(self), "story-format");
		return;
	}
	if(obj->integer.val != (int) format) {
		obj->integer.val = format;
		g_object_notify(G_OBJECT(self), "story-format");
	}
}

gboolean
i7_story_get_create_blorb(I7Story *self)
{
	g_return_val_if_fail(self || I7_IS_STORY(self), 0);

	PlistObject *settings = i7_story_get_settings(self);
	PlistObject *obj = plist_object_lookup(settings, "IFOutputSettings", "IFSettingCreateBlorb", -1);
	if(!obj)
		return TRUE; /* Default value */
	return obj->boolean.val;
}

void
i7_story_set_create_blorb(I7Story *self, gboolean create_blorb)
{
	g_return_if_fail(self || I7_IS_STORY(self));

	i7_document_set_modified(I7_DOCUMENT(self), TRUE);

	PlistObject *settings = i7_story_get_settings(self);
	PlistObject *obj = plist_object_lookup(settings, "IFOutputSettings", "IFSettingCreateBlorb", -1);
	if(!obj) {
		obj = plist_object_new(PLIST_OBJECT_BOOLEAN);
		obj->boolean.val = create_blorb;
		insert_setting(settings, "IFOutputSettings", "IFSettingCreateBlorb", obj);
		g_object_notify(G_OBJECT(self), "create-blorb");
		return;
	}
	if(obj->boolean.val != create_blorb) {
		obj->boolean.val = create_blorb;
		g_object_notify(G_OBJECT(self), "create-blorb");
	}
}

gboolean
i7_story_get_nobble_rng(I7Story *self)
{
	g_return_val_if_fail(self || I7_IS_STORY(self), 0);

	PlistObject *settings = i7_story_get_settings(self);
	PlistObject *obj = plist_object_lookup(settings, "IFOutputSettings", "IFSettingNobbleRng", -1);
	if(!obj)
		return FALSE; /* Default value */
	return obj->boolean.val;
}

void
i7_story_set_nobble_rng(I7Story *self, gboolean nobble_rng)
{
	g_return_if_fail(self || I7_IS_STORY(self));

	i7_document_set_modified(I7_DOCUMENT(self), TRUE);

	PlistObject *settings = i7_story_get_settings(self);
	PlistObject *obj = plist_object_lookup(settings, "IFOutputSettings", "IFSettingNobbleRng", -1);
	if(!obj) {
		obj = plist_object_new(PLIST_OBJECT_BOOLEAN);
		obj->boolean.val = nobble_rng;
		insert_setting(settings, "IFOutputSettings", "IFSettingNobbleRng", obj);
		g_object_notify(G_OBJECT(self), "nobble-rng");
		return;
	}
	if(obj->boolean.val != nobble_rng) {
		obj->boolean.val = nobble_rng;
		g_object_notify(G_OBJECT(self), "nobble-rng");
	}
}

gboolean
i7_story_get_elastic_tabstops(I7Story *self)
{
	g_return_val_if_fail(self || I7_IS_STORY(self), 0);

	PlistObject *settings = i7_story_get_settings(self);
	PlistObject *obj = plist_object_lookup(settings, "IFMiscSettings", "IFSettingElasticTabs", -1);
	if(!obj)
		return FALSE; /* Default value */
	return obj->boolean.val;
}

void
i7_story_set_elastic_tabstops(I7Story *self, gboolean elastic_tabstops)
{
	g_return_if_fail(self || I7_IS_STORY(self));

	i7_document_set_modified(I7_DOCUMENT(self), TRUE);

	PlistObject *settings = i7_story_get_settings(self);
	PlistObject *obj = plist_object_lookup(settings, "IFMiscSettings", "IFSettingElasticTabs", -1);
	if(!obj) {
		obj = plist_object_new(PLIST_OBJECT_BOOLEAN);
		obj->boolean.val = elastic_tabstops;
		insert_setting(settings, "IFMiscSettings", "IFSettingElasticTabs", obj);
		g_object_notify(G_OBJECT(self), "elastic-tabstops");
		return;
	}
	if(obj->boolean.val != elastic_tabstops) {
		obj->boolean.val = elastic_tabstops;
		g_object_notify(G_OBJECT(self), "elastic-tabstops");
	}
}
