/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2006-2012, 2014, 2015 Philip Chimento <philip.chimento@gmail.com>
 */

#ifndef FILE_H
#define FILE_H

#include "config.h"

#include <glib.h>
#include <gtksourceview/gtksource.h>

#include "document.h"
#include "story.h"

char *read_source_file(GFile *file);
void set_source_text(GtkSourceBuffer *buffer, gchar *text);
void delete_build_files(I7Story *story);
GFile *get_case_insensitive_extension(GFile *file);
gboolean make_directory_unless_exists(GFile *file, GCancellable *cancellable, GError **error);
gboolean file_exists_and_is_dir(GFile *file);
gboolean file_exists_and_is_symlink(GFile *file);
char *file_get_display_name(GFile *file);
void file_set_custom_icon(GFile *file, const char *icon_name);
gboolean show_uri_in_browser(const char *uri, GtkWindow *parent, const char *display_name);
gboolean show_uri_externally(const char *uri, GtkWindow *parent, const char *display_name);
gboolean show_file_in_browser(GFile *file, GtkWindow *parent);

#endif
