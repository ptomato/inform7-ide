1/basic: Basics.

@Purpose: Some fundamental definitions.

@p About this code.
Inweb is itself written as an Inweb web. This hazardous circularity is
traditional in literate-programming circles, much as people like to make
compilers compile themselves: after all, if even the authors aren't willing
to trust the code, why should anyone else? So this is a kind of demonstration
of self-belief. Inweb is probably fairly correct, because it is capable
of tangling a program (Inweb) which can in turn tangle a really complex
and densely annotated program (Inform) which then passes an extensive suite
of tests.

@p Basic definitions.
These are all from the ANSI C standard library, which means that Inweb will
tangle them up to the top of the C source code.

@c
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

@ We'll use three truth states:

@d TRUE 1
@d FALSE 0
@d UNKNOWN 2

@ Now we define the build, using a notation which tangles out to the current
build number as specified in the contents section of this web.

@d INWEB_BUILD "inweb [[Build Number]]"

@ The following needs to depend on the host operating system, but doesn't
yet do so.

@d SEP_CHAR '/' /* local file-system filename separator */

@p Strings.
We need to handle C strings long enough to contain any plausible filename, and
any run of a dozen or so lines of code; but we have no real need to handle
strings of unlimited length, nor to be parsimonious with memory. So we'll
store strings in the standard C way, as an array of characters with a null
terminator, but we will enforce bounds-checking to prevent overflows on exotic
or malicious input.

The following defines a type for a string long enough for our purposes.
It should be at least as long as the constant sometimes called |PATH_MAX|,
the maximum length of a pathname, which is 1024 on Mac OS X.

@d MAX_STRING_LENGTH 4096

@c
typedef char string[MAX_STRING_LENGTH+1];

@ Any out-of-range access immediately halts the program; this is drastic, but
an attempt to continue execution after a string overflow might conceivably
result in a malformatted shell command being passed to the operating system,
which is too hazardous to risk.

@c
int check_len(int n) {
	if ((n > MAX_STRING_LENGTH) || (n < 0)) fatal_error("String overflow\n");
	return n;
}

@ The following is then protected from reading out of range if given a
non-terminated string, though this should never actually happen.

@c
int in_strlen(char *str) {
	for (int i=0; i<=MAX_STRING_LENGTH; i++)
		if (str[i] == 0) return i;
	str[MAX_STRING_LENGTH] = 0;
	return MAX_STRING_LENGTH;
}

@ The following is a bounds-checked way to say |str[i] = to|:

@c
void in_set(char *str, int i, int to) {
	if ((i < 0) || (i > MAX_STRING_LENGTH)) check_len(i);
	else str[i] = to;
}

@ When we're setting a character to null, we're truncating the string, and
it makes the code clearer if we call it that:

@c
void in_truncate(char *str, int len) {
	if ((len < 0) || (len > MAX_STRING_LENGTH)) { check_len(len); str[0] = 0; }
	else str[len] = 0;
}

@ We then have a replacement for |strcpy|, identical except that it's
bounds-checked:

@c
void in_strcpy(char *to, char *from) {
	check_len(in_strlen(from));
	int i;
	for (i=0; ((from[i]) && (i < MAX_STRING_LENGTH)); i++) to[i] = from[i];
	to[i] = 0;
}

@ Similarly for |strcat|:

@c
void in_strcat(char *to, char *from) {
	int i, L = in_strlen(to);
	check_len(L + in_strlen(from));
	for (i=0; ((from[i]) && (L+i < MAX_STRING_LENGTH)); i++) to[L+i] = from[i];
	to[L+i] = 0;
}

@ And for |sprintf|, which we replace with a variadic macro which does bounds
checking around a regular unchecked call. Note, though, that we make use of
the |snprintf| function added in ISO C99. This never writes outside the bounds
of the string, and always null-terminates it. In the event of an overflow, it
is supposed to return the length which would have been required (a number
larger than |MAX_STRING_LENGTH|), but on some platforms it incorrectly returns
-1 instead. This is why both such values will fail |check_len|.

|in_sprcat| is a variant which concatenates onto the string.

@c
#define in_sprintf(to, args...) \
	check_len(snprintf(to, MAX_STRING_LENGTH, args))
#define in_sprcat(to, args...) \
	check_len(snprintf(to+in_strlen(to), MAX_STRING_LENGTH-in_strlen(to), args))

@ String comparisons will be done with the following, not |strcmp| directly:

@c
int in_string_eq(char *A, char *B) {
	return (in_string_cmp(A, B) == 0)?TRUE:FALSE;
}

int in_string_ne(char *A, char *B) {
	return (in_string_cmp(A, B) == 0)?FALSE:TRUE;
}

@ On the rare occasions when we need to sort alphabetically we'll also call:

@c
int in_string_cmp(char *A, char *B) {
	if ((A == NULL) || (A[0] == 0)) {
		if ((B == NULL) || (B[0] == 0)) return 0;
		return -1;
	}
	if ((B == NULL) || (B[0] == 0)) return 1;
	return strcmp(A, B);
}

@ For case-insensitive comparisons, where speed doesn't matter:

@c
int in_string_eq_insensitive(char *A, char *B) {
	int i;
	for (i=0; ((A[i]) && (B[i])); i++)
		if (tolower(A[i]) != tolower(B[i])) return FALSE;
	if ((A[i] == 0) && (B[i] == 0)) return TRUE;
	return FALSE;
}

@p Operating system interface.
This is a program which has to issue commands to its host operating system,
to copy files, pass them through |TeX|, and so on. All of that is done using
the C standard library |system| function; the commands invoked are all
standard for POSIX, so will work on OS X and Linux, but on a Windows system
they would need to be read in a POSIX-style environment like Cygwin.

@c
void issue_os_command_0(char *command) {
	system(command);
}

void issue_os_command_1(char *command, char *fn1) {
	string osc;
	in_sprintf(osc, command, fn1);
	issue_os_command_0(osc);
}

void issue_os_command_2(char *command, char *fn1, char *fn2) {
	string osc;
	in_sprintf(osc, command, fn1, fn2);
	issue_os_command_0(osc);
}
