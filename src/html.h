/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Philip Chimento <philip.chimento@gmail.com>
 */

#pragma once

#include "config.h"

#include <glib.h>
#include <webkit2/webkit2.h>

typedef void (*LoadFinishedCallback)(WebKitWebView *, void *data);

void html_load_file(WebKitWebView *html, GFile *filename);
void html_load_file_at_anchor(WebKitWebView *html, GFile *file, const char *anchor);
void html_load_with_fallback(WebKitWebView *html, const char *uri, const char *fallback_uri,
    LoadFinishedCallback callback, void *data, GDestroyNotify user_data_free);
void html_load_blank(WebKitWebView *html);
