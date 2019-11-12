/* Copyright (C) 2006-2009, 2010, 2011, 2018 P. F. Chimento
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

