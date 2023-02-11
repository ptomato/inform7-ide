/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Philip Chimento <philip.chimento@gmail.com>
 */

#include "config.h"

#include <stdbool.h>

#include <glib-object.h>
#include <gtk/gtk.h>

#include "blob.h"

/* These tests basically just exercise the API. */

typedef struct {
	I7Blob *blob;
} BlobFixture;

static void
blob_setup(BlobFixture *fx, const void *)
{
	fx->blob = i7_blob_new();
	g_object_ref_sink(fx->blob);
}

static void
blob_teardown(BlobFixture *fx, const void *)
{
	g_object_unref(fx->blob);
}

static void
test_blob_create(BlobFixture *fx, const void *)
{
	g_assert_true(I7_IS_BLOB(fx->blob));
}

static void
test_blob_status(BlobFixture *fx, const void *)
{
	i7_blob_set_status(fx->blob, "status message", true);

	i7_blob_set_status(fx->blob, "status message 2", false);
}

static void
test_blob_progress(BlobFixture *fx, const void *)
{
	i7_blob_set_progress(fx->blob, 0.0, NULL);
	i7_blob_set_progress(fx->blob, 0.5, NULL);
	i7_blob_set_progress(fx->blob, 1.0, NULL);

	i7_blob_pulse_progress(fx->blob);

	i7_blob_clear_progress(fx->blob);
}

static void
test_blob_progress_cancellable(BlobFixture *fx, const void *)
{
	GCancellable *cancel = g_cancellable_new();
	i7_blob_set_progress(fx->blob, 0.5, cancel);

	g_cancellable_cancel(cancel);

	i7_blob_clear_progress(fx->blob);
}

void
add_blob_tests(void)
{
#define ADD_BLOB_TEST(name) \
	g_test_add("/blob/" #name, BlobFixture, NULL, blob_setup, test_blob_##name, blob_teardown);
	ADD_BLOB_TEST(create)
	ADD_BLOB_TEST(status)
	ADD_BLOB_TEST(progress)
	ADD_BLOB_TEST(progress_cancellable)
#undef ADD_BLOB_TEST
}
