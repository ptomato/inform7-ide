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

#include <string.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <webkit/webkit.h>
#include "html.h"
#include "error.h"

/* Have the html widget display the HTML file in filename */
void 
html_load_file(WebKitWebView *html, const gchar *filename) 
{
    g_return_if_fail(html);
    g_return_if_fail(filename || strlen(filename));
    
    GError *error = NULL;
	gchar *uri = g_filename_to_uri(filename, NULL, &error);
	if(!uri) {
		WARN_S(_("Could not convert filename to URI"), filename, error);
		return;
	}
	webkit_web_view_open(html, uri); /* SUCKY DEBIAN Deprecated since 1.1.1 */
	g_free(uri);
}

/* Have the html widget display the HTML file and jump to the anchor */
void
html_load_file_at_anchor(WebKitWebView *html, const gchar *file, const gchar *anchor)
{
	g_return_if_fail(html);
    g_return_if_fail(file || strlen(file));
	g_return_if_fail(anchor);
    
    GError *error = NULL;
	gchar *uri = g_filename_to_uri(file, NULL, &error);
	if(!uri) {
		WARN_S(_("Could not convert filename to URI"), file, error);
		return;
	}
	gchar *real_uri = g_strconcat(uri, "#", anchor, NULL);
	g_free(uri);
	webkit_web_view_open(html, real_uri); /* SUCKY DEBIAN Deprecated since 1.1.1 */
	g_free(real_uri);
}

/* Blank the html widget */
void 
html_load_blank(WebKitWebView *html) 
{
    g_return_if_fail(html);
	webkit_web_view_open(html, "about:blank"); /* SUCKY DEBIAN Deprecated since 1.1.1 */
}

/* Reload the html widget */
void
html_refresh(WebKitWebView *html)
{
	g_return_if_fail(html);
	webkit_web_view_reload(html);
}
