/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Philip Chimento <philip.chimento@gmail.com>
 */

#include "config.h"

#include <glib.h>
#include <webkit2/webkit2.h>

#include "uri-scheme.h"

/* URI SCHEME HANDLERS */

/* Internal function: find the real filename referred to by a URI starting with
 inform:. Returns NULL if not found, but if that happens then there is a bug in
 the HTML generated by Inform. TODO: make this use a cache. */
static GFile *
find_real_file_for_inform_uri_scheme(const char *path)
{
	/* Remove %xx escapes */
	g_autofree char *unescaped = g_uri_unescape_string(path, "");

	/* Replace the slashes by platform-dependent path separators */
	g_auto(GStrv) elements = g_strsplit(unescaped, "/", -1);

	g_autofree char *relative_path = NULL;
	if (elements[0] && strcmp(elements[0], "Extensions") == 0) {
		/* inform://Extensions is an exception; change it so it will be picked
		 up by the last attempt below, in the home directory */
		relative_path = g_build_filenamev(elements + 1);
	} else if (elements[0] && elements[1] && !*elements[0] && strcmp(elements[1], "Extensions") == 0) {
		/* same for inform:///Extensions and inform:/Extensions */
		relative_path = g_build_filenamev(elements + 2);
	} else
		relative_path = g_build_filenamev(elements);

	g_autoptr(GFile) parent = g_file_new_for_uri("resource:///com/inform7/IDE/inform");
	g_autoptr(GFile) real_file = g_file_resolve_relative_path(parent, relative_path);
	if (g_file_query_exists(real_file, NULL))
		return g_steal_pointer(&real_file);
	g_object_unref(g_steal_pointer(&real_file));
	g_object_unref(g_steal_pointer(&parent));

	g_autoptr(GFile) home_file = g_file_new_for_path(g_get_home_dir());
	parent = g_file_resolve_relative_path(home_file, "Inform/Documentation");
	real_file = g_file_resolve_relative_path(parent, relative_path);
	if (g_file_query_exists(real_file, NULL))
		return g_steal_pointer(&real_file);
	g_object_unref(g_steal_pointer(&real_file));
	g_object_unref(g_steal_pointer(&parent));

	parent = g_file_resolve_relative_path(home_file, "Inform/Documentation/Extensions");
	real_file = g_file_resolve_relative_path(parent, relative_path);
	if (g_file_query_exists(real_file, NULL))
		return g_steal_pointer(&real_file);

	g_warning("Could not locate real filename for URI %s. There may be a bug in"
		" the HTML generated by Inform.", path);
	return NULL;
}

static void
on_inform_file_read(GFile *real_file, GAsyncResult *result, WebKitURISchemeRequest *request)
{
	g_autoptr(GError) error = NULL;
	g_autoptr(GFile) file = real_file;
	g_autoptr(GFileInputStream) stream = g_file_read_finish(file, result, &error);

	if (stream == NULL) {
		webkit_uri_scheme_request_finish_error(request, error);
		g_autofree char *uri = g_file_get_uri(file);
		g_warning("problem reading %s: %s", uri, error->message);
		g_object_unref(request);
		return;
	}

	webkit_uri_scheme_request_finish(request, G_INPUT_STREAM(stream), -1,
		NULL /* guess mimetype from extension */);
	g_object_unref(request);
}

static void
handle_inform_uri(WebKitURISchemeRequest *request)
{
	const char *path = webkit_uri_scheme_request_get_path(request);
	g_debug("URI: inform://%s", path);
	g_autoptr(GFile) real_file = find_real_file_for_inform_uri_scheme(path);
	if (real_file == NULL) {
		g_autoptr(GError) error = g_error_new(WEBKIT_NETWORK_ERROR, WEBKIT_NETWORK_ERROR_FILE_DOES_NOT_EXIST,
			"Could not locate real filename for URI inform://%s. There may be "
			"a bug in the HTML generated by Inform.", path);
		webkit_uri_scheme_request_finish_error(request, error);
		return;
	}

	g_object_ref(request);
	g_file_read_async(g_steal_pointer(&real_file), G_PRIORITY_DEFAULT, NULL,
		(GAsyncReadyCallback)on_inform_file_read, request);
}

void
i7_uri_scheme_register(WebKitWebContext *web_context)
{
	webkit_web_context_register_uri_scheme(web_context, "inform", (WebKitURISchemeRequestCallback)handle_inform_uri, NULL, NULL);
}
