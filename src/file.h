/* Copyright (C) 2006-2009, 2010, 2011, 2012, 2014 P. F. Chimento
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

#ifndef FILE_H
#define FILE_H

#include <glib.h>
#include <gtksourceview/gtksourcebuffer.h>
#include "story.h"
#include "document.h"

char *read_source_file(GFile *file);
void set_source_text(GtkSourceBuffer *buffer, gchar *text);
GFile *get_file_from_save_dialog(I7Document *document, GFile *default_file);
void delete_build_files(I7Story *story);
GFile *get_case_insensitive_extension(GFile *file);
gboolean make_directory_unless_exists(GFile *file, GCancellable *cancellable, GError **error);
gboolean file_exists_and_is_dir(GFile *file);
gboolean file_exists_and_is_symlink(GFile *file);
char *file_get_display_name(GFile *file);
void file_set_custom_icon(GFile *file, const char *icon_name);

#endif
