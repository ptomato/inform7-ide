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
 
#ifndef CONFIG_FILE_H
#define CONFIG_FILE_H

#include <gnome.h>

gchar *get_extensions_base_path();
gchar *get_extension_path(const gchar *author, const gchar *extname);
void check_config_file();
gboolean check_external_binaries();

void config_file_set_string(gchar *path, gchar *key, const gchar *value);
gchar *config_file_get_string(gchar *path, gchar *key);
void config_file_set_int(gchar *path, gchar *key, const gint value);
gint config_file_get_int(gchar *path, gchar *key);
void config_file_set_bool(gchar *path, gchar *key, const gboolean value);
gboolean config_file_get_bool(gchar *path, gchar *key);

#endif
