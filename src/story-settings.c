/*  Copyright 2006 P.F. Chimento
 *  This file is part of GNOME Inform 7.
 * 
 *  GNOME Inform 7 is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  GNOME Inform 7 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNOME Inform 7; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <glib.h>
#include <gtk/gtk.h>
#include "story.h"
#include "story-private.h"
#include "panel.h"

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
	/* IFOutputSettings->IFSettingZCodeVersion (8) */
	obj = plist_object_new(PLIST_OBJECT_INTEGER);
	obj->integer.val = I7_STORY_FORMAT_Z8;
	insert_setting(dict, "IFOutputSettings", "IFSettingZCodeVersion", obj);
	/* IFOutputSettings->IFSettingNobbleRng (FALSE) */
	obj = plist_object_new(PLIST_OBJECT_BOOLEAN);
	obj->boolean.val = FALSE;
	insert_setting(dict, "IFOutputSettings", "IFSettingNobbleRng", obj);

	return dict;
}

void
on_z5_button_toggled(GtkToggleButton *button, I7Story *story)
{
	gboolean value = gtk_toggle_button_get_active(button);
	if(value)
		i7_story_set_story_format(story, I7_STORY_FORMAT_Z5);
	
	/* When the one changes, change the other */
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(story->panel[LEFT]->z5), value);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(story->panel[RIGHT]->z5), value);
}

void
on_z6_button_toggled(GtkToggleButton *button, I7Story *story)
{
	gboolean value = gtk_toggle_button_get_active(button);
	if(value)
		i7_story_set_story_format(story, I7_STORY_FORMAT_Z6);
	
	/* When the one changes, change the other */
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(story->panel[LEFT]->z6), value);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(story->panel[RIGHT]->z6), value);
}

void
on_z8_button_toggled(GtkToggleButton *button, I7Story *story)
{
	gboolean value = gtk_toggle_button_get_active(button);
	if(value)
		i7_story_set_story_format(story, I7_STORY_FORMAT_Z8);
	
	/* When the one changes, change the other */
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(story->panel[LEFT]->z8), value);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(story->panel[RIGHT]->z8), value);
}

void
on_glulx_button_toggled(GtkToggleButton *button, I7Story *story)
{
	gboolean value = gtk_toggle_button_get_active(button);
	if(value)
		i7_story_set_story_format(story, I7_STORY_FORMAT_GLULX);
	
	/* When the one changes, change the other */
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(story->panel[LEFT]->glulx), value);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(story->panel[RIGHT]->glulx), value);
}

void
on_blorb_button_toggled(GtkToggleButton *button, I7Story *story)
{
	gboolean value = gtk_toggle_button_get_active(button);
	i7_story_set_create_blorb(story, value);
	
	/* When the one changes, change the other */
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(story->panel[LEFT]->blorb), value);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(story->panel[RIGHT]->blorb), value);
}

void
on_nobble_rng_button_toggled(GtkToggleButton *button, I7Story *story)
{
	gboolean value = gtk_toggle_button_get_active(button);
	i7_story_set_nobble_rng(story, value);
	
	/* When the one changes, change the other */
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(story->panel[LEFT]->nobble_rng), value);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(story->panel[RIGHT]->nobble_rng), value);
}

/* Select all the right buttons according to the story settings */
void 
on_notify_story_format(I7Story *story) 
{
	switch(i7_story_get_story_format(story)) {
		case I7_STORY_FORMAT_Z5:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(story->panel[LEFT]->z5), TRUE);
			break;
		case I7_STORY_FORMAT_Z6:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(story->panel[LEFT]->z6), TRUE);
			break;
		case I7_STORY_FORMAT_GLULX:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(story->panel[LEFT]->glulx), TRUE);
			break;
		case I7_STORY_FORMAT_Z8:
		default:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(story->panel[LEFT]->z8), TRUE);
	}
	/* The callbacks ensure that we don't have to manually set the other ones */
}

void
on_notify_create_blorb(I7Story *story)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(story->panel[LEFT]->blorb), i7_story_get_create_blorb(story));
	/* The callbacks ensure that we don't have to manually set the other ones */
}

void
on_notify_nobble_rng(I7Story *story)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(story->panel[LEFT]->nobble_rng), i7_story_get_nobble_rng(story));
	/* The callbacks ensure that we don't have to manually set the other ones */
}

void
on_notify_elastic_tabs(I7Story *story)
{
	gboolean value = i7_story_get_elastic_tabs(story);
	gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(I7_DOCUMENT(story)->enable_elastic_tabs), value);
	i7_source_view_set_elastic_tabs(story->panel[LEFT]->sourceview, value);
	i7_source_view_set_elastic_tabs(story->panel[RIGHT]->sourceview, value);
}

/* These 'get' functions provide for a default value even though we initialize a
 dictionary with all the required keys; it may be that another port of Inform 7
 doesn't write all the keys */
I7StoryFormat 
i7_story_get_story_format(I7Story *story)
{
	g_return_val_if_fail(story || I7_IS_STORY(story), 0);

	PlistObject *settings = I7_STORY_PRIVATE(story)->settings;
	PlistObject *obj = plist_object_lookup(settings, "IFOutputSettings", "IFSettingZCodeVersion", -1);
	if(!obj)
		return I7_STORY_FORMAT_Z8; /* Default value */
	return obj->integer.val;
}

void 
i7_story_set_story_format(I7Story *story, I7StoryFormat format)
{
	g_return_if_fail(story || I7_IS_STORY(story));
	g_return_if_fail(format == I7_STORY_FORMAT_Z5 || format == I7_STORY_FORMAT_Z6 || format == I7_STORY_FORMAT_Z8 || format == I7_STORY_FORMAT_GLULX);

	PlistObject *settings = I7_STORY_PRIVATE(story)->settings;
	PlistObject *obj = plist_object_lookup(settings, "IFOutputSettings", "IFSettingZCodeVersion", -1);
	if(!obj) {
		obj = plist_object_new(PLIST_OBJECT_INTEGER);
		obj->integer.val = format;
		insert_setting(settings, "IFOutputSettings", "IFSettingZCodeVersion", obj);
		g_object_notify(G_OBJECT(story), "story-format");
		return;
	} 
	if(obj->integer.val != format) {
		obj->integer.val = format;
		g_object_notify(G_OBJECT(story), "story-format");
	}
}

gboolean 
i7_story_get_create_blorb(I7Story *story)
{
	g_return_val_if_fail(story || I7_IS_STORY(story), 0);

	PlistObject *settings = I7_STORY_PRIVATE(story)->settings;
	PlistObject *obj = plist_object_lookup(settings, "IFOutputSettings", "IFSettingCreateBlorb", -1);
	if(!obj)
		return TRUE; /* Default value */
	return obj->boolean.val;
}

void 
i7_story_set_create_blorb(I7Story *story, gboolean create_blorb)
{
	g_return_if_fail(story || I7_IS_STORY(story));

	PlistObject *settings = I7_STORY_PRIVATE(story)->settings;
	PlistObject *obj = plist_object_lookup(settings, "IFOutputSettings", "IFSettingCreateBlorb", -1);
	if(!obj) {
		obj = plist_object_new(PLIST_OBJECT_BOOLEAN);
		obj->boolean.val = create_blorb;
		insert_setting(settings, "IFOutputSettings", "IFSettingCreateBlorb", obj);
		g_object_notify(G_OBJECT(story), "create-blorb");
		return;
	} 
	if(obj->boolean.val != create_blorb) {
		obj->boolean.val = create_blorb;
		g_object_notify(G_OBJECT(story), "create-blorb");
	}
}

gboolean 
i7_story_get_nobble_rng(I7Story *story)
{
	g_return_val_if_fail(story || I7_IS_STORY(story), 0);

	PlistObject *settings = I7_STORY_PRIVATE(story)->settings;
	PlistObject *obj = plist_object_lookup(settings, "IFOutputSettings", "IFSettingNobbleRng", -1);
	if(!obj)
		return FALSE; /* Default value */
	return obj->boolean.val;
}

void 
i7_story_set_nobble_rng(I7Story *story, gboolean nobble_rng)
{
	g_return_if_fail(story || I7_IS_STORY(story));

	PlistObject *settings = I7_STORY_PRIVATE(story)->settings;
	PlistObject *obj = plist_object_lookup(settings, "IFOutputSettings", "IFSettingNobbleRng", -1);
	if(!obj) {
		obj = plist_object_new(PLIST_OBJECT_BOOLEAN);
		obj->boolean.val = nobble_rng;
		insert_setting(settings, "IFOutputSettings", "IFSettingNobbleRng", obj);
		g_object_notify(G_OBJECT(story), "nobble-rng");
		return;
	} 
	if(obj->boolean.val != nobble_rng) {
		obj->boolean.val = nobble_rng;
		g_object_notify(G_OBJECT(story), "nobble-rng");
	}
}

gboolean 
i7_story_get_elastic_tabs(I7Story *story)
{
	g_return_val_if_fail(story || I7_IS_STORY(story), 0);

	PlistObject *settings = I7_STORY_PRIVATE(story)->settings;
	PlistObject *obj = plist_object_lookup(settings, "IFMiscSettings", "IFSettingElasticTabs", -1);
	if(!obj)
		return FALSE; /* Default value */
	return obj->boolean.val;
}

void 
i7_story_set_elastic_tabs(I7Story *story, gboolean elastic_tabs)
{
	g_return_if_fail(story || I7_IS_STORY(story));

	PlistObject *settings = I7_STORY_PRIVATE(story)->settings;
	PlistObject *obj = plist_object_lookup(settings, "IFMiscSettings", "IFSettingElasticTabs", -1);
	if(!obj) {
		obj = plist_object_new(PLIST_OBJECT_BOOLEAN);
		obj->boolean.val = elastic_tabs;
		insert_setting(settings, "IFMiscSettings", "IFSettingElasticTabs", obj);
		g_object_notify(G_OBJECT(story), "elastic-tabs");
		return;
	} 
	if(obj->boolean.val != elastic_tabs) {
		obj->boolean.val = elastic_tabs;
		g_object_notify(G_OBJECT(story), "elastic-tabs");
	}
}
