[CStrings::] C Strings.

@Purpose: A minimal library for handling C-style strings.

@p C Strings.
The intools are migrating away from C-style strings in favour of the newer
streams system for holding text, but these utilities are still worth keeping
for the time being.

We need to handle C strings long enough to contain any plausible filename, and
any run of a dozen or so lines of code; but we have no real need to handle
strings of unlimited length, nor to be parsimonious with memory. So we'll
store strings in the standard C way, as an array of characters with a null
terminator, but we will enforce bounds-checking to prevent overflows on exotic
or malicious input.

The following defines a type for a string long enough for our purposes.
It should be at least as long as the constant sometimes called |PATH_MAX|,
the maximum length of a pathname, which is 1024 on Mac OS X.

@d MAX_STRING_LENGTH 8*1024

@c
typedef char string[MAX_STRING_LENGTH+1];

@ Occasionally we need access to the real, unbounded strlen:

@c
int CStrings::strlen_unbounded(const char *p) {
	return (int) strlen(p);
}

@ Any out-of-range access immediately halts the program; this is drastic, but
an attempt to continue execution after a string overflow might conceivably
result in a malformatted shell command being passed to the operating system,
which is too hazardous to risk.

@c
int CStrings::check_len(int n) {
	if ((n > MAX_STRING_LENGTH) || (n < 0)) Errors::fatal("String overflow\n");
	return n;
}

@ The following is then protected from reading out of range if given a
non-terminated string, though this should never actually happen.

@c
int CStrings::len(char *str) {
	for (int i=0; i<=MAX_STRING_LENGTH; i++)
		if (str[i] == 0) return i;
	str[MAX_STRING_LENGTH] = 0;
	return MAX_STRING_LENGTH;
}


@ The following is a bounds-checked way to say |str[i] = to|:

@c
void CStrings::set_char(char *str, int i, int to) {
	if ((i < 0) || (i > MAX_STRING_LENGTH)) CStrings::check_len(i);
	else str[i] = (char) to;
}

@ When we're setting a character to null, we're truncating the string, and
it makes the code clearer if we call it that:

@c
void CStrings::truncate(char *str, int len) {
	if ((len < 0) || (len > MAX_STRING_LENGTH)) { CStrings::check_len(len); str[0] = 0; }
	else str[len] = 0;
}

@ We then have a replacement for |strcpy|, identical except that it's
bounds-checked:

@c
void CStrings::copy(char *to, char *from) {
	CStrings::check_len(CStrings::len(from));
	int i;
	for (i=0; ((from[i]) && (i < MAX_STRING_LENGTH)); i++) to[i] = from[i];
	to[i] = 0;
}

@ Similarly for |strcat|:

@c
void CStrings::concatenate(char *to, char *from) {
	int i, L = CStrings::len(to);
	CStrings::check_len(L + CStrings::len(from));
	for (i=0; ((from[i]) && (L+i < MAX_STRING_LENGTH)); i++) to[L+i] = from[i];
	to[L+i] = 0;
}

@ Remove spaces and tabs from both ends:

@c
char *CStrings::trim_white_space(char *original) {
	int i;
	for (i=0; Characters::is_space_or_tab(original[i]); i++) ;
	original += i;
	for (i=CStrings::strlen_unbounded(original)-1; ((i>=0) && (Characters::is_space_or_tab(original[i]))); i--)
		original[i] = 0;
	return original;
}

@ Extract the Wth word (counting from 1), truncated to a given maximum
size in characters.

@c
void CStrings::extract_word(char *fword, char *line, int size, int word) {
	int i = 0;
	fword[0] = 0;
	while (word > 0) {
		word--;
		while (Characters::is_space_or_tab(line[i])) i++;
		int j = 0;
		while ((line[i]) && (!Characters::is_space_or_tab(line[i]))) {
			if (j < size-1) fword[j++] = Characters::tolower(line[i]);
			i++;
		}
		fword[j] = 0;
		if (line[i] == 0) break;
	}
	if (word > 0) fword[0] = 0;
}

@ String comparisons will be done with the following, not |strcmp| directly:

@c
int CStrings::eq(char *A, char *B) {
	return (CStrings::cmp(A, B) == 0)?TRUE:FALSE;
}

int CStrings::ne(char *A, char *B) {
	return (CStrings::cmp(A, B) == 0)?FALSE:TRUE;
}

@ On the rare occasions when we need to sort alphabetically we'll also call:

@c
int CStrings::cmp(char *A, char *B) {
	if ((A == NULL) || (A[0] == 0)) {
		if ((B == NULL) || (B[0] == 0)) return 0;
		return -1;
	}
	if ((B == NULL) || (B[0] == 0)) return 1;
	return strcmp(A, B);
}

@ For case-insensitive comparisons, where speed doesn't matter:

@c
int CStrings::eq_insensitive(char *A, char *B) {
	int i;
	for (i=0; ((A[i]) && (B[i])); i++)
		if (tolower(A[i]) != tolower(B[i])) return FALSE;
	if ((A[i] == 0) && (B[i] == 0)) return TRUE;
	return FALSE;
}

@ And the following is needed to deal with extension filenames on platforms
whose locale is encoded as UTF-8.

@d UTF8_ENC 1 /* Write as UTF-8 without BOM */
@d ISO_ENC 2 /* Write as ISO Latin-1 (i.e., no conversion needed) */

@c
void CStrings::transcode_ISO_string_to_UTF8(char *p, char *dest) {
	int i, j;
	for (i=0, j=0; p[i]; i++) {
		int charcode = (int) (((unsigned char *)p)[i]);
		if (charcode >= 128) {
			dest[j++] = (char) (0xC0 + (charcode >> 6));
			dest[j++] = (char) (0x80 + (charcode & 0x3f));
		} else {
			dest[j++] = p[i];
		}
	}
	dest[j] = 0;
}
