/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Philip Chimento <philip.chimento@gmail.com>
 */

#include "config.h"

#include <stdbool.h>
#include <unistd.h>

#include <glib.h>

#include "transcript-diff.h"

char *expected_same = "This text is exactly the same.\n\nCowabunga!\n";
char *actual_same = "This text is exactly the same.\n\nCowabunga!\n";
char *expected_whitespace = "This text is the same but for whitespace.\n\nCowabunga!\n";
char *actual_whitespace = "This text is the same but for  whitespace.\n\nCowabunga!\n\n";
char *expected_different = "This text is not the same at all.\n\nCowabunga!\n";
char *actual_different = "This text isn't the same at all.\nGeronimo!\n\n";

void
test_diffs_same(void)
{
	GList *expected_diffs = NULL;
	GList *actual_diffs = NULL;
	bool result = word_diff(expected_same, actual_same, &expected_diffs, &actual_diffs);

	g_assert_true(result);
	g_assert_null(expected_diffs);
	g_assert_null(actual_diffs);
}

void
test_diffs_whitespace(void)
{
	GList *expected_diffs = NULL;
	GList *actual_diffs = NULL;
	bool result = word_diff(expected_whitespace, actual_whitespace, &expected_diffs, &actual_diffs);

	g_assert_false(result);
	g_assert_null(expected_diffs);
	g_assert_null(actual_diffs);

	g_autofree char *expected_pango = make_pango_markup_string(expected_whitespace, expected_diffs);
	g_assert_cmpstr(expected_pango, ==, expected_whitespace);

	g_autofree char *actual_pango = make_pango_markup_string(actual_whitespace, actual_diffs);
	g_assert_cmpstr(actual_pango, ==, actual_whitespace);
}

void
test_diffs_different(void)
{
	GList *expected_diffs = NULL;
	GList *actual_diffs = NULL;
	bool result = word_diff(expected_different, actual_different, &expected_diffs, &actual_diffs);

	g_assert_false(result);
	g_assert_nonnull(expected_diffs);
	g_assert_nonnull(actual_diffs);

	g_autofree char *expected_pango = make_pango_markup_string(expected_different, expected_diffs);
	g_assert_cmpstr(expected_pango, ==, "This text <u>is</u> <u>not</u> the same at all.\n\n<u>Cowabunga!</u>\n");

	g_autofree char *actual_pango = make_pango_markup_string(actual_different, actual_diffs);
	g_assert_cmpstr(actual_pango, ==, "This text <u>isn&apos;t</u> the same at all.\n<u>Geronimo!</u>\n\n");

	g_list_free(expected_diffs);
	g_list_free(actual_diffs);
}
