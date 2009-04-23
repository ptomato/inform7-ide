/* This file is part of GNOME Inform 7.
 * Copyright (c) 2006-2009 P. F. Chimento <philip.chimento@gmail.com>
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
 
#ifndef _DATAFILE_H
#define _DATAFILE_H

#include <gnome.h>
#include <stdarg.h>

gchar *get_extension_path(const gchar *author, const gchar *extname);
gchar *get_datafile_path(const gchar *filename);
gchar *get_datafile_path_va(const gchar *path1, ...);
gboolean check_datafile(const gchar *filename);
gchar *get_pixmap_path(const gchar *filename);
gchar *get_binary_path(const gchar *filename);

#endif /* _DATAFILE_H */
