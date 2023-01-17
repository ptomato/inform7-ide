/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2006-2011, 2014, 2015, 2019, 2021, 2022 Philip Chimento <philip.chimento@gmail.com>
 */

#include "config.h"

#include <glib.h>
#include <glib/gi18n.h>
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
	/* IFOutputSettings->IFSettingCompilerVersion ("****", meaning 'current') */
	obj = plist_new_string("****");
	insert_setting(dict, "IFOutputSettings", "IFSettingCompilerVersion", obj);

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
on_language_version_chooser_changed(GtkComboBox *chooser, GtkLabel *description)
{
	const char *id = gtk_combo_box_get_active_id(chooser);
	g_return_if_fail(id != NULL);

	I7App *theapp = I7_APP(g_application_get_default());
	const char *copy = i7_app_get_retrospective_description(theapp, id);
	gtk_label_set_label(description, copy);
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

char *
i7_story_get_language_version(I7Story *self)
{
	g_return_val_if_fail(self || I7_IS_STORY(self), NULL);

	plist_t settings = i7_story_get_settings(self);
	plist_t obj = plist_access_path(settings, 2, "IFOutputSettings", "IFSettingCompilerVersion");
	if (!obj)
		return g_strdup("****");  /* Default value */

	char *plist_val;
	plist_get_string_val(obj, &plist_val);
	char *retval = g_strdup(plist_val);
	/*plist_mem_*/free(plist_val);
	return retval;
}

void
i7_story_set_language_version(I7Story *self, const char *ver)
{
	g_return_if_fail(self || I7_IS_STORY(self));

	i7_document_set_modified(I7_DOCUMENT(self), TRUE);

	I7App *theapp = I7_APP(g_application_get_default());
	if (!i7_app_is_valid_retrospective_id(theapp, ver)) {
		g_warning("Unknown compiler version '%s', defaulting to current version", ver);
		ver = "****";
	}

	plist_t settings = i7_story_get_settings(self);
	plist_t obj = plist_access_path(settings, 2, "IFOutputSettings", "IFSettingCompilerVersion");
	if (!obj) {
		obj = plist_new_string(ver);
		insert_setting(settings, "IFOutputSettings", "IFSettingCompilerVersion", obj);
		g_object_notify(G_OBJECT(self), "language-version");
		return;
	}

	char *plist_val;
	plist_get_string_val(obj, &plist_val);
	if (strcmp(plist_val, ver) != 0) {
		plist_set_string_val(obj, ver);
		g_object_notify(G_OBJECT(self), "language-version");
	}
	/*plist_mem_*/free(plist_val);
}
