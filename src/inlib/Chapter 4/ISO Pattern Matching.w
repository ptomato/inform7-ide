[ISORegexp::] ISO Pattern Matching.

@Purpose: An older and shortly-to-be-deprecated version of the pattern matcher
which works on fixed-size ISO Latin-1 C strings.

@p Character types.
We will define white space as spaces and tabs only, since the various kinds
of line terminator will always be stripped out before this is applied.

@c
int ISORegexp::white_space(int c) {
	if ((c == ' ') || (c == '\t')) return TRUE;
	return FALSE;
}

@ The presence of |:| here is perhaps a bit surprising, since it's illegal in
C and has other meanings in other languages, but it's legal in C-for-Inform
identifiers.

@c
int ISORegexp::identifier_char(int c) {
	if ((c == '_') || (c == ':') ||
		((c >= 'A') && (c <= 'Z')) ||
		((c >= 'a') && (c <= 'z')) ||
		((c >= '0') && (c <= '9'))) return TRUE;
	return FALSE;
}

@p Simple parsing.
The following finds the earliest minimal-length substring of a string,
delimited by two pairs of characters: for example, |<<| and |>>|. This could
easily be done as a regular expression using |ISORegexp::match|, but the routine
here is much quicker.

@c
int ISORegexp::find_expansion(char *original, char on1, char on2, char off1, char off2, int *len) {
	for (int i = 0; original[i]; i++)
		if ((original[i] == on1) && (original[i+1] == on2)) {
			for (int j=i+2; original[j]; j++)
				if ((original[j] == off1) && (original[j+1] == off2)) {
					*len = j+2-i;
					return i;
				}
		}
	return -1;
}

@ Still more simply:

@c
int ISORegexp::find_open_brace(char *p) {
	for (int i=0; p[i]; i++)
		if (p[i] == '{')
			return i;
	return -1;
}

@ Note that we count the empty string as being white space. Again, this is
equivalent to |ISORegexp::match(p, " *")|, but much faster.

@c
int ISORegexp::string_is_white_space(char *p) {
	for (int i=0; p[i]; i++)
		if (ISORegexp::white_space(p[i]) == FALSE)
			return FALSE;
	return TRUE;
}

@p A Worse PCRE.
I originally wanted to call the function in this section |a_better_sscanf|, then
thought perhaps |a_worse_PCRE| would be more true. (PCRE is Philip Hazel's superb
C implementation of regular-expression parsing, but I didn't need its full strength,
and I didn't want to complicate the build process by linking to it.)

This is a very minimal regular expression parser, simply for convenience of parsing
short texts against particularly simple patterns. For example:

	|"fish (%d+) ([a-zA-Z_][a-zA-Z0-9_]*) *"|

matches the word fish, then any amount of whitespace, then a string of digits
which are copied into |found_text1|, then whitespace again, and then an
alphanumeric identifier to be copied into |found_text2|, and finally optional
whitespace. If no match is made, the contents of the found strings are undefined.

Up to two bracketed subexpressions can be extracted, and are copied into the
strings passed to the regexp matcher, which are thus rather like the Perl
variables |$1| and |$2|.

@ The internal state of the matcher is stored as follows:

@c
typedef struct match_position {
	int tpos; /* position within text being matched */
	int ppos; /* position within pattern */
	int bc; /* count of bracketed subexpressions so far begun */
	int bl; /* bracket indentation level */
	int bracket_nesting[4]; /* which subexpression numbers (0, 1, 2, 3) correspond to which nesting */
	int brackets_start[4], brackets_end[4]; /* positions in text being matched, inclusive */
} match_position;

@

@c
int ISORegexp::match_0(char *text, char *pattern) {
	return ISORegexp::match_r(text, pattern, NULL, NULL, NULL, NULL, NULL);
}

int ISORegexp::match_1(char *text, char *pattern, char *ft1) {
	return ISORegexp::match_r(text, pattern, NULL, ft1, NULL, NULL, NULL);
}

int ISORegexp::match_2(char *text, char *pattern, char *ft1, char *ft2) {
	return ISORegexp::match_r(text, pattern, NULL, ft1, ft2, NULL, NULL);
}

int ISORegexp::match_3(char *text, char *pattern, char *ft1, char *ft2, char *ft3) {
	return ISORegexp::match_r(text, pattern, NULL, ft1, ft2, ft3, NULL);
}

int ISORegexp::match_4(char *text, char *pattern, char *ft1, char *ft2, char *ft3, char *ft4) {
	return ISORegexp::match_r(text, pattern, NULL, ft1, ft2, ft3, ft4);
}

@

@c
int ISORegexp::match_r(char *text, char *pattern, match_position *scan_from, char *ft1, char *ft2, char *ft3, char *ft4) {
	match_position at;
	if (scan_from) at = *scan_from;
	else { at.tpos = 0; at.ppos = 0; at.bc = 0; at.bl = 0; }

	while ((text[at.tpos]) || (pattern[at.ppos])) {
		@<Parentheses in the match pattern set up substrings to extract@>;

		int chcl, /* what class of characters to match: a |*_CLASS| value */
			range_from, range_to, /* for |LITERAL_CLASS| only */
			reverse = FALSE; /* require a non-match rather than a match */
		@<Extract the character class to match from the pattern@>;

		int rep_from = 1, rep_to = 1; /* minimum and maximum number of repetitions */
		int greedy = TRUE; /* go for a maximal-length match if possible */
		@<Extract repetition markers from the pattern@>;

		int reps = 0;
		@<Count how many repetitions can be made here@>;
		if (reps < rep_from) return FALSE;

		/* we can now accept anything from |rep_from| to |reps| repetitions */
		if (rep_from == reps) { at.tpos += reps; continue; }
		@<Try all possible match lengths until we find a match@>;

		/* no match length worked, so no match */
		return FALSE;
	}
	@<Copy the bracketed texts found into the global strings@>;
	return TRUE;
}

@

@<Parentheses in the match pattern set up substrings to extract@> =
	if (pattern[at.ppos] == '(') {
		if (at.bl < 4) at.bracket_nesting[at.bl] = -1;
		if (at.bc < 4) {
			at.bracket_nesting[at.bl] = at.bc;
			at.brackets_start[at.bc] = at.tpos; at.brackets_end[at.bc] = -1;
		}
		at.bl++; at.bc++; at.ppos++;
		continue;
	}
	if (pattern[at.ppos] == ')') {
		at.bl--;
		if ((at.bl >= 0) && (at.bl < 4) && (at.bracket_nesting[at.bl] >= 0))
			at.brackets_end[at.bracket_nesting[at.bl]] = at.tpos-1;
		at.ppos++;
		continue;
	}

@

@<Extract the character class to match from the pattern@> =
	int len;
	chcl = ISORegexp::get_cclass(pattern, at.ppos, &len, &range_from, &range_to, &reverse);
	at.ppos += len;

@ This is standard regular-expression notation, except that I haven't bothered
to implement numeric repetition counts, which we won't need:

@<Extract repetition markers from the pattern@> =
	if (chcl == WHITESPACE_CLASS) {
		rep_from = 1; rep_to = CStrings::len(text)-at.tpos;
	}
	if (pattern[at.ppos] == '+') {
		rep_from = 1; rep_to = CStrings::len(text)-at.tpos; at.ppos++;
	} else if (pattern[at.ppos] == '*') {
		rep_from = 0; rep_to = CStrings::len(text)-at.tpos; at.ppos++;
	}
	if (pattern[at.ppos] == '?') { greedy = FALSE; at.ppos++; }

@

@<Count how many repetitions can be made here@> =
	for (reps = 0; ((text[at.tpos+reps]) && (reps <= rep_to)); reps++)
		if (ISORegexp::test_cclass(text+at.tpos+reps, chcl,
			range_from, range_to, pattern, reverse) == FALSE)
			break;

@

@<Try all possible match lengths until we find a match@> =
	int from = rep_from, to = reps, dj = 1, from_tpos = at.tpos;
	if (greedy) { from = reps; to = rep_from; dj = -1; }
	for (int j = from; j != to+dj; j += dj) {
		at.tpos = from_tpos + j;
		if (ISORegexp::match_r(text, pattern, &at, ft1, ft2, ft3, ft4))
			return TRUE;
	}

@

@<Copy the bracketed texts found into the global strings@> =
	if ((at.bc > 0) && (ft1)) {
		CStrings::copy(ft1, text + at.brackets_start[0]);
		ft1[at.brackets_end[0]-at.brackets_start[0]+1] = 0;
	}
	if ((at.bc > 1) && (ft2)) {
		CStrings::copy(ft2, text + at.brackets_start[1]);
		ft2[at.brackets_end[1]-at.brackets_start[1]+1] = 0;
	}
	if ((at.bc > 2) && (ft3)) {
		CStrings::copy(ft3, text + at.brackets_start[2]);
		ft3[at.brackets_end[2]-at.brackets_start[2]+1] = 0;
	}
	if ((at.bc > 3) && (ft4)) {
		CStrings::copy(ft4, text + at.brackets_start[3]);
		ft4[at.brackets_end[3]-at.brackets_start[3]+1] = 0;
	}

@ So then: most characters in the pattern are taken literally (if the pattern
says |q|, the only match is with a lower-case letter "q"), except that:

(a) a space means "one or more characters of white space";
(b) |%d| means any decimal digit;
(c) |%c| means any character at all;
(d) |%C| means any character which isn't white space;
(e) |%i| means any character from the identifier class (see above);
(f) |%p| means any character which can be used in the name of a Preform
nonterminal, which is to say, an identifier character or a hyphen;
(g) |%P| means the same or else a colon.
(h) |%t| means a tab.

|%| otherwise makes a literal escape; a space means any whitespace character;
square brackets enclose literal alternatives, and note as usual with grep
engines that |[]xyz]| is legal and makes a set of four possibilities, the
first of which is a literal close square; within a set, a hyphen makes a
character range; an initial |^| negates the result; and otherwise everything
is literal.

@d ANY_CLASS 1
@d DIGIT_CLASS 2
@d WHITESPACE_CLASS 3
@d NONWHITESPACE_CLASS 4
@d IDENTIFIER_CLASS 5
@d PREFORM_CLASS 6
@d PREFORMC_CLASS 7
@d LITERAL_CLASS 8
@d TAB_CLASS 9

@c
int ISORegexp::get_cclass(char *pattern, int ppos, int *len, int *from, int *to, int *reverse) {
	if (pattern[ppos] == '^') { ppos++; *reverse = TRUE; } else { *reverse = FALSE; }
	switch (pattern[ppos]) {
		case '%':
			ppos++;
			*len = 2;
			switch (pattern[ppos]) {
				case 'd': return DIGIT_CLASS;
				case 'c': return ANY_CLASS;
				case 'C': return NONWHITESPACE_CLASS;
				case 'i': return IDENTIFIER_CLASS;
				case 'p': return PREFORM_CLASS;
				case 'P': return PREFORMC_CLASS;
				case 't': return TAB_CLASS;
			}
			*from = ppos; *to = ppos; return LITERAL_CLASS;
		case '[':
			*from = ppos+2;
			while ((pattern[ppos]) && (pattern[ppos] != ']')) ppos++;
			*to = ppos - 1; *len = ppos - *from + 1;
			return LITERAL_CLASS;
		case ' ':
			*len = 1; return WHITESPACE_CLASS;
	}
	*len = 1; *from = ppos; *to = ppos; return LITERAL_CLASS;				
}

@

@c
int ISORegexp::test_cclass(char *text, int chcl, int range_from, int range_to, char *drawn_from, int reverse) {
	int match = FALSE;
	switch (chcl) {
		case ANY_CLASS: if (text[0]) match = TRUE; break;
		case DIGIT_CLASS: if (isdigit(text[0])) match = TRUE; break;
		case WHITESPACE_CLASS: if (ISORegexp::white_space(text[0])) match = TRUE; break;
		case TAB_CLASS: if (text[0] == '\t') match = TRUE; break;
		case NONWHITESPACE_CLASS: if (!(ISORegexp::white_space(text[0]))) match = TRUE; break;
		case IDENTIFIER_CLASS: if (ISORegexp::identifier_char(text[0])) match = TRUE; break;
		case PREFORM_CLASS: if ((text[0] == '-') || (text[0] == '_') ||
			((text[0] >= 'a') && (text[0] <= 'z')) ||
			((text[0] >= '0') && (text[0] <= '9'))) match = TRUE; break;
		case PREFORMC_CLASS: if ((text[0] == '-') || (text[0] == '_') || (text[0] == ':') ||
			((text[0] >= 'a') && (text[0] <= 'z')) ||
			((text[0] >= '0') && (text[0] <= '9'))) match = TRUE; break;
		case LITERAL_CLASS:
			for (int j = range_from; j <= range_to; j++) {
				int c1 = drawn_from[j], c2 = c1;
				if ((j+1 < range_to) && (drawn_from[j+1] == '-')) { c2 = drawn_from[j+2]; j += 2; }
				int c = *text;
				if ((c >= c1) && (c <= c2)) {
					match = TRUE; break;
				}
			}
			break;
	}
	if (reverse) match = (match)?FALSE:TRUE;
	return match;
}
