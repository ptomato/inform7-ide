/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Philip Chimento <philip.chimento@gmail.com>
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

typedef struct {
	LoadFinishedCallback callback;
	void *user_data;  /* owned */
	GDestroyNotify user_data_free;
	char *fallback_uri;  /* owned */
	unsigned long changed_handler_id;
	unsigned long failed_handler_id;
} LoadWithFallbackClosure;

/* Helper function: load the "disconnected" page if an error occurred */
static gboolean
on_load_failed(WebKitWebView *html, WebKitLoadEvent status, char *uri, GError *web_error, LoadWithFallbackClosure *data)
{
	webkit_web_view_load_uri(html, data->fallback_uri);
	return GDK_EVENT_STOP;
}

static void
on_load_changed(WebKitWebView *html, WebKitLoadEvent status, LoadWithFallbackClosure *data)
{
	if(status != WEBKIT_LOAD_FINISHED)
		return;

	g_signal_handler_disconnect(html, data->changed_handler_id);
	data->changed_handler_id = 0;
	g_signal_handler_disconnect(html, data->failed_handler_id);
	data->failed_handler_id = 0;

	data->callback(html, data->user_data);

	if (data->user_data_free)
		data->user_data_free(data->user_data);
	g_free(data->fallback_uri);
	g_free(data);
}

/**
 * html_load_with_fallback:
 * @html: the web view
 * @uri: a URI to load
 * @fallback_uri: a URI to load instead, if @file does not exist or has errors
 * @callback: callback to call when the operation is complete
 * @user_data: user data to pass to callback
 * @user_data_free: free function for @user_data
 *
 * Loads @file in @html, falling back to @fallback_uri.
 * Regardless of whether the operation succeeds or fails, @callback is called
 * with @html and @user_data. Takes ownership of @user_data and calls
 * @user_data_free when it is no longer needed.
 */
void
html_load_with_fallback(WebKitWebView *html, const char *uri, const char *fallback_uri,
    LoadFinishedCallback callback, void *user_data, GDestroyNotify user_data_free)
{
	g_return_if_fail(html);
	g_return_if_fail(uri);
	g_return_if_fail(fallback_uri);

	LoadWithFallbackClosure *data = g_new0(LoadWithFallbackClosure, 1);
	data->callback = callback;
	data->user_data = user_data;
	data->user_data_free = user_data_free;
	data->fallback_uri = g_strdup(fallback_uri);

	data->changed_handler_id = g_signal_connect(html, "load-changed", G_CALLBACK(on_load_changed), data);
	data->failed_handler_id = g_signal_connect(html, "load-failed", G_CALLBACK(on_load_failed), data);

	webkit_web_view_load_uri(html, uri);
}

/* Blank the html widget */
void
html_load_blank(WebKitWebView *html)
{
	g_return_if_fail(html);
	webkit_web_view_load_uri(html, "about:blank");
}

