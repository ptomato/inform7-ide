/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Philip Chimento <philip.chimento@gmail.com>
 */

#pragma once

#include <glib.h>
#include <app.h>

void test_app_create(void);
void test_app_files(void);
void test_app_extensions_install_remove(void);
void test_app_extensions_get_builtin(void);
void test_app_extensions_get_version(void);
void test_app_extensions_case_insensitive(void);
void test_app_colorscheme_install_remove(void);
void test_app_colorscheme_get_current(void);
