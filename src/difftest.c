#include <unistd.h>
#include <glib.h>
#include "transcript-diff.h"

char *expected_same = "This text is exactly the same.\n\nCowabunga!\n";
char *actual_same = "This text is exactly the same.\n\nCowabunga!\n";
char *expected_whitespace = "This text is the same but for whitespace.\n\nCowabunga!\n";
char *actual_whitespace = "This text is the same but for  whitespace.\n\nCowabunga!\n\n";
char *expected_different = "This text is not the same at all.\n\nCowabunga!\n";
char *actual_different = "This text isn't the same at all.\nGeronimo!\n\n";

static void
print_diffs(const char *expected, const char *actual)
{
	GList *expected_diffs = NULL;
	GList *actual_diffs = NULL;
	char *buf;

	if(word_diff(expected, actual, &expected_diffs, &actual_diffs)) {
		g_print("Exactly alike!\n");
		return;
	}
	g_print("Not exactly alike!\n");
	buf = make_pango_markup_string(expected, expected_diffs);
	g_print(" Expected: '%s'\n", buf);
	g_free(buf);
	buf = make_pango_markup_string(actual, actual_diffs);
	g_print(" Actual: '%s'\n", buf);
	g_free(buf);
}

int
main(int argc, char **argv)
{
	g_print("Same:\n");
	print_diffs(expected_same, actual_same);
	g_print("Same except whitespace:\n");
	print_diffs(expected_whitespace, actual_whitespace);
	g_print("Different:\n");
	print_diffs(expected_different, actual_different);
	return 0;
}
