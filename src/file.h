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
 
#ifndef FILE_H
#define FILE_H

#include <gnome.h>

#include "story.h"
#include "extension.h"

gboolean verify_save(GtkWidget *thiswidget);
void save_project(GtkWidget *thiswidget, gchar *directory);
struct story *open_project(gchar *directory);
gboolean verify_save_ext(GtkWidget *thiswidget);
struct extension *open_extension(gchar *filename);
void save_extension(GtkWidget *thiswidget);
void install_extension(const gchar *filename);
void finish_release(struct story *thestory, gboolean everything_ok);
void delete_build_files(struct story *thestory);
GnomeVFSMonitorHandle *monitor_file(const gchar *file_uri,
  GnomeVFSMonitorCallback callback, gpointer data);
void project_changed(GnomeVFSMonitorHandle *handle, const gchar *monitor_uri,
  const gchar *info_uri, GnomeVFSMonitorEventType event_type, gpointer data);
void extension_changed(GnomeVFSMonitorHandle *handle, const gchar *monitor_uri,
  const gchar *info_uri, GnomeVFSMonitorEventType event_type, gpointer data);
    
#endif
