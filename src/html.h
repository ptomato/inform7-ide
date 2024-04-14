/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Philip Chimento <philip.chimento@gmail.com>
 */

#pragma once

#include "config.h"

#include <glib.h>
#include <webkit2/webkit2.h>

void html_load_file(WebKitWebView *html, GFile *filename);
void html_load_file_at_anchor(WebKitWebView *html, GFile *file, const char *anchor);
void html_load_blank(WebKitWebView *html);
