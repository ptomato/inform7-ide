/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2006-2011 Philip Chimento <philip.chimento@gmail.com>
 */

#include "config.h"

#include <glib.h>
#include <webkit2/webkit2.h>

#include "html.h"

/**
 * html_load_file:
 * @html: a #WebKitWebView
 * @file: a #GFile
 *
 * Have @html display the HTML file referenced by @file.
 */
void
html_load_file(WebKitWebView *html, GFile *file)
{
	g_return_if_fail(html);
	g_return_if_fail(file);

	char *uri = g_file_get_uri(file);
	webkit_web_view_load_uri(html, uri);
	g_free(uri);
}

/**
 * html_load_file_at_anchor:
 * @html: a #WebKitWebView
 * @file: a #GFile
 * @anchor: a tag on the page
 *
 * Have @html display the HTML file referenced by @file, and jump to the tag
 * specified by @anchor.
 */
void
html_load_file_at_anchor(WebKitWebView *html, GFile *file, const char *anchor)
{
	g_return_if_fail(html);
	g_return_if_fail(file);
	g_return_if_fail(anchor);

	char *uri = g_file_get_uri(file);
	char *real_uri = g_strconcat(uri, "#", anchor, NULL);
	g_free(uri);
	webkit_web_view_load_uri(html, real_uri);
	g_free(real_uri);
}

/* Blank the html widget */
void
html_load_blank(WebKitWebView *html)
{
	g_return_if_fail(html);
	webkit_web_view_load_uri(html, "about:blank");
}

