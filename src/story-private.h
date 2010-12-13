/***************************************************************************
 *            story-private.h
 *
 *  Sat Sep 27 22:21:52 2008
 *  Copyright  2008  P. F. Chimento
 *  <philip.chimento@gmail.com>
 ****************************************************************************/

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */
 
#ifndef STORY_PRIVATE_H
#define STORY_PRIVATE_H

#include <glib.h>
#include <gtk/gtk.h>
#include "story.h"
#include "skein.h"
#include "osxcart/plist.h"

typedef struct {
	/* Action Groups */
	GtkUIManager *ui_manager;
	GtkActionGroup *story_action_group;
	GtkActionGroup *unimplemented_action_group;
	/* Widget with last input focus */
	GtkWidget *last_focused;
	/* Other text buffers */
    GtkTextBuffer *notes;
	GtkTextBuffer *progress;
	GtkTextBuffer *debug_log;
	GtkSourceBuffer *i6_source;
	/* The Settings.plist object */
	PlistObject *settings;
	/* The manifest.plist object */
	PlistObject *manifest;
	/* Compiling */
	CompileActionFunc compile_finished_callback;
	gpointer compile_finished_callback_data;
	gchar *copyblorbto;
	gchar *compiler_output;
	/* Skein / running */
	I7Skein *skein;
	gboolean test_me;
} I7StoryPrivate;

#define I7_STORY_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), I7_TYPE_STORY, I7StoryPrivate))
#define I7_STORY_USE_PRIVATE(o,n) I7StoryPrivate *n = I7_STORY_PRIVATE(o)

#endif /* STORY_PRIVATE_H */

 
