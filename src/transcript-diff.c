/* Copyright (C) 2011, 2012, 2015 P. F. Chimento
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

#include "config.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include <glib.h>

/* Prerequisites for including Gnulib's diffseq algorithm */
#include <limits.h>
#include <stdbool.h>
#define ELEMENT char *
#define EQUAL(a,b) (strcmp((a), (b)) == 0)
#define OFFSET ssize_t
#define EXTRA_CONTEXT_FIELDS \
	GList *expected_diffs; \
	GList *actual_diffs;
#define NOTE_DELETE(ctxt,xoff) \
	G_STMT_START { \
		if(*(xv[(xoff)]) != '\0') { \
			(ctxt)->expected_diffs = g_list_prepend((ctxt)->expected_diffs, GSIZE_TO_POINTER((xoff))); \
		} \
	} G_STMT_END
#define NOTE_INSERT(ctxt,yoff) \
	G_STMT_START { \
		if(*(yv[(yoff)]) != '\0') { \
			(ctxt)->actual_diffs = g_list_prepend((ctxt)->actual_diffs, GSIZE_TO_POINTER((yoff))); \
		} \
	} G_STMT_END
#define USE_HEURISTIC
#define lint /* To suppress GCC warnings */

#include "diffseq.h"

/*
 * word_diff:
 * Compares strings @expected and @actual for approximate equality. Returns TRUE
 * if they are _exactly_ equal, FALSE if not. @expected_diffs and @actual_diffs
 * are the return locations for lists of word indices that are different in
 * @expected and @actual. The indices are stored using GSIZE_TO_POINTER().
 * If the function returns FALSE but NULL (the empty list) is returned in
 * @expected_diffs and @actual_diffs, then all the words are the same and
 * therefore the strings only differ by whitespace.
 * You should free the lists when done.
 */
gboolean
word_diff(const char *expected, const char *actual, GList **expected_diffs, GList **actual_diffs)
{
	/* If strings are exactly the same, we have our answer */
	if(strcmp(expected, actual) == 0)
		return TRUE;

	char **expected_words = g_strsplit_set(expected, " \n\r\t", -1);
	char **actual_words = g_strsplit_set(actual, " \n\r\t", -1);
	ssize_t expected_limit = 0;
	ssize_t actual_limit = 0;
	ssize_t *work_buffer;
	struct context ctxt;
	char **iter;

	/* Find lengths of word vectors */
	for(iter = expected_words; *iter; iter++)
		expected_limit++;
	for(iter = actual_words; *iter; iter++)
		actual_limit++;

	/* Allocate a work buffer */
	work_buffer = g_new0(ssize_t, 2 * (expected_limit + actual_limit + 3));

	/* Call the Gnulib diff algorithm */
	ctxt.xvec = expected_words; /* char * const * */
	ctxt.yvec = actual_words;
	ctxt.fdiag = work_buffer + actual_limit + 1;
	ctxt.bdiag = ctxt.fdiag + expected_limit + actual_limit + 3;
	ctxt.expected_diffs = *expected_diffs;
	ctxt.actual_diffs = *actual_diffs;
	ctxt.heuristic = TRUE;
	compareseq(0, expected_limit, 0, actual_limit, &ctxt);

	g_strfreev(expected_words);
	g_strfreev(actual_words);
	g_free(work_buffer);

	*expected_diffs = g_list_reverse(ctxt.expected_diffs);
	*actual_diffs = g_list_reverse(ctxt.actual_diffs);

	return FALSE;
}

char *
make_pango_markup_string(const char *string, GList *diffs)
{
	if(diffs == NULL)
		return g_strdup(string);

	char **words = g_strsplit_set(string, " \n\r\t", -1);
	char **word;
	ssize_t count;
	GString *result = g_string_new("");

	for(word = words, count = 0; *word; word++, count++) {
		/* Copy whitespace */
		while(isspace(*string))
			g_string_append_c(result, *string++);

		char *escaped_word = g_markup_escape_text(*word, -1);

		if(diffs && count == (ssize_t)(GPOINTER_TO_SIZE(diffs->data))) {
			diffs = g_list_next(diffs);
			g_string_append_printf(result, "<u>%s</u>", escaped_word);
		} else {
			g_string_append(result, escaped_word);
		}

		g_free(escaped_word);

		string += strlen(*word);
	}

	g_strfreev(words);

	/* Copy final whitespace */
	while(isspace(*string))
		g_string_append_c(result, *string++);

	return g_string_free(result, FALSE); /* return C-string */
}