/*  Copyright (C) 2011, 2012 P. F. Chimento
 *  This file is part of GNOME Inform 7.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef APP_TEST_H
#define APP_TEST_H

#include <glib.h>
#include <app.h>

G_BEGIN_DECLS

void test_app_create(void);
void test_app_files(void);
void test_app_extensions_install_remove(void);
void test_app_extensions_get_builtin(void);
void test_app_extensions_get_version(void);
void test_app_extensions_case_insensitive(void);
void test_app_colorscheme_install_remove(void);
void test_app_colorscheme_get_current(void);

G_END_DECLS

#endif /* APP_TEST_H */