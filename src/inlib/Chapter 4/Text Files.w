[TextFiles::] Text Files.

@Purpose: To read text files of whatever flavour, one line at a time.

@p Text file positions.
Here's how we record a position in a text file:

@c
typedef struct text_file_position {
	struct filename *text_file_filename;
	int line_count;
	int line_position;
	int skip_terminator;
	int actively_scanning; /* whether we are still interested in the rest of the file */
} text_file_position;

@ For access:

@c
int TextFiles::get_line_count(text_file_position *tfp) {
	if (tfp == NULL) return 0;
	return tfp->line_count;
}

@ And this is for a real nowhere man:

@c
text_file_position TextFiles::nowhere(void) {
	text_file_position tfp;
	tfp.text_file_filename = NULL;
	tfp.line_count = 0;
	tfp.line_position = 0;
	tfp.skip_terminator = FALSE;
	tfp.actively_scanning = FALSE;
	return tfp;
}

@p Testing existence.

@c
int TextFiles::exists(filename *F) {
	FILE *HANDLE = Platform::iso_fopen(F, "rb");
	if (HANDLE == NULL) return FALSE;
	fclose(HANDLE);
	return TRUE;
}

@p Text file iteration.
We read lines in, delimited by any of the standard line-ending characters,
and send them one at a time to a function called |iterator|.

The following routine is provided twice, once in the preferred modern form,
which stores arbitrary Unicoded material in text streams, and then once in
the older way, where large fixed-size buffers of ISO Latin-1 text are used
instead.

@c
int TextFiles::read(filename *F, char *message, int serious,
	void (iterator)(text_stream *, text_file_position *, void *),
	text_file_position *start_at, void *state) {
	FILE *HANDLE;
	text_file_position tfp;
	unicode_file_buffer ufb = TextFiles::create_ufb();
	@<Open the text file (wide version)@>;
	@<Set the initial position, seeking it in the file if need be (wide version)@>;
	@<Read in lines and send them one by one to the iterator (wide version)@>;
	fclose(HANDLE);
	return tfp.line_count;
}

@

@<Open the text file (wide version)@> =
	HANDLE = Platform::iso_fopen(F, "rb");
	if (HANDLE == NULL) {
		if (message == NULL) return 0;
		if (serious) Errors::fatal_with_file(message, F);
		else { Errors::with_file(message, F); return 0; }
	}

@ The ANSI definition of |ftell| and |fseek| says that, with text files, the
only definite position value is 0 -- meaning the beginning of the file -- and
this is what we initialise |line_position| to. We must otherwise only write
values returned by |ftell| into this field.

@<Set the initial position, seeking it in the file if need be (wide version)@> =
	if (start_at == NULL) {
		tfp.line_count = 1;
		tfp.line_position = 0;
		tfp.skip_terminator = 'X';
	} else {
		tfp = *start_at;
		if (fseek(HANDLE, (long int) (tfp.line_position), SEEK_SET)) {
			if (serious) Errors::fatal_with_file("unable to seek position in file", F);
			Errors::with_file("unable to seek position in file", F);
			return 0;
		}
	}
	tfp.actively_scanning = TRUE;
	tfp.text_file_filename = F;

@ We aim to get this right whether the lines are terminated by |0A|, |0D|,
|0A 0D| or |0D 0A|. The final line is not required to be terminated.

@<Read in lines and send them one by one to the iterator (wide version)@> =
	TEMPORARY_TEXT(line);
	int i = 0, c = ' ';
	int warned = FALSE;
	while ((c != EOF) && (tfp.actively_scanning)) {
		c = TextFiles::utf8_fgetc(HANDLE, NULL, TRUE, &ufb);
		if ((c == EOF) || (c == '\x0a') || (c == '\x0d')) {
			Str::put_at(line, i, 0);
			if ((i > 0) || (c != tfp.skip_terminator)) {
				@<Feed the completed line to the iterator routine (wide version)@>;
				if (c == '\x0a') tfp.skip_terminator = '\x0d';
				if (c == '\x0d') tfp.skip_terminator = '\x0a';
			} else tfp.skip_terminator = 'X';
			@<Update the text file position (wide version)@>;
			i = 0;
		} else {
			if (i < MAX_STRING_LENGTH) Str::put_at(line, i++, (char) c);
			else {
				if (serious) Errors::fatal_with_file("line too long", F);
				if (warned == FALSE) {
					Errors::in_text_file("line too long (truncating it)", &tfp);
					warned = TRUE;
				}
			}
		}
	}
	if ((i > 0) && (tfp.actively_scanning))
		@<Feed the completed line to the iterator routine (wide version)@>;
	DISCARD_TEXT(line);

@ We update the line counter only when a line is actually sent:

@<Feed the completed line to the iterator routine (wide version)@> =
	iterator(line, &tfp, state);
	tfp.line_count++;

@ But we update the text file position after every apparent line terminator.
This is because we might otherwise, on a Windows text file, end up with an
|ftell| position in between the |CR| and the |LF|; if we resume at that point,
later on, we'll then have an off-by-one error in the line numbering in the
resumption as compared to during the original pass.

Properly speaking, |ftell| returns a long |int|, not an |int|, but on a
32-bit-or-more integer machine, this gives us room for files to run to 2GB.
Text files seldom come that large.

@<Update the text file position (wide version)@> =
	tfp.line_position = (int) (ftell(HANDLE));
	if (tfp.line_position == -1) {
		if (serious)
			Errors::fatal_with_file("unable to determine position in file", F);
		else
			Errors::with_file("unable to determine position in file", F);
	}

@ And here is the ISO Latin-1 version:

@c
int TextFiles::read_with_lines_to_ISO(filename *F, char *message, int serious,
	void (iterator)(char *, text_file_position *, void *), text_file_position *start_at,
	void *state) {
	FILE *HANDLE;
	text_file_position tfp;
	unicode_file_buffer ufb = TextFiles::create_ufb();
	@<Open the text file@>;
	@<Set the initial position, seeking it in the file if need be@>;
	@<Read in lines and send them one by one to the iterator@>;
	fclose(HANDLE);
	return tfp.line_count;
}

@

@<Open the text file@> =
	HANDLE = Platform::iso_fopen(F, "rb");
	if (HANDLE == NULL) {
		if (message == NULL) return 0;
		if (serious) Errors::fatal_with_file(message, F);
		else { Errors::with_file(message, F); return 0; }
	}

@

@<Set the initial position, seeking it in the file if need be@> =
	if (start_at == NULL) {
		tfp.line_count = 1;
		tfp.line_position = 0;
		tfp.skip_terminator = 'X';
	} else {
		tfp = *start_at;
		if (fseek(HANDLE, (long int) (tfp.line_position), SEEK_SET)) {
			if (serious) Errors::fatal_with_file("unable to seek position in file", F);
			Errors::with_file("unable to seek position in file", F);
			return 0;
		}
	}
	tfp.actively_scanning = TRUE;
	tfp.text_file_filename = F;

@

@<Read in lines and send them one by one to the iterator@> =
	char line[MAX_STRING_LENGTH]; CStrings::copy(line, "");
	int i = 0, c = ' ';
	int warned = FALSE;
	while ((c != EOF) && (tfp.actively_scanning)) {
		c = TextFiles::utf8_fgetc(HANDLE, NULL, TRUE, &ufb);
		if ((c == EOF) || (c == '\x0a') || (c == '\x0d')) {
			CStrings::set_char(line, i, 0);
			if ((i > 0) || (c != tfp.skip_terminator)) {
				@<Feed the completed line to the iterator routine@>;
				if (c == '\x0a') tfp.skip_terminator = '\x0d';
				if (c == '\x0d') tfp.skip_terminator = '\x0a';
			} else tfp.skip_terminator = 'X';
			@<Update the text file position@>;
			i = 0;
		} else {
			if (i < MAX_STRING_LENGTH) CStrings::set_char(line, i++, (char) c);
			else {
				if (serious) Errors::fatal_with_file("line too long", F);
				if (warned == FALSE) {
					Errors::in_text_file("line too long (truncating it)", &tfp);
					warned = TRUE;
				}
			}
		}
	}
	if ((i > 0) && (tfp.actively_scanning))
		@<Feed the completed line to the iterator routine@>;


@

@<Feed the completed line to the iterator routine@> =
	iterator(line, &tfp, state);
	tfp.line_count++;

@

@<Update the text file position@> =
	tfp.line_position = (int) (ftell(HANDLE));
	if (tfp.line_position == -1) {
		if (serious)
			Errors::fatal_with_file("unable to determine position in file", F);
		else
			Errors::with_file("unable to determine position in file", F);
	}

@ The routine being iterated can indicate that it has had enough by
calling the following:

@c
void TextFiles::lose_interest(text_file_position *tfp) {
	tfp->actively_scanning = FALSE;
}

@p Reading UTF-8 files.
The following routine reads a sequence of Unicode characters from a UTF-8
encoded file, but returns them as a sequence of ISO Latin-1 characters, a
trick it can only pull off by escaping non-ISO characters. This is done by
taking character number $N$ and feeding it out, one character at a time, as
the text "[unicode $N$]", writing the number in decimal. Only one UTF-8
file like this will be being read at a time, and the routine will be
repeatedly called until |EOF| or a line division.

Strictly speaking, we transmit not as ISO Latin-1 but as that subset of ISO
which have corresponding (different) codes in the ZSCII character set. This
excludes some typewriter symbols and a handful of letterforms, as we shall
see.

There are two exceptions: |TextFiles::utf8_fgetc| can also return the usual C
end-of-file pseudo-character |EOF|, and it can also return the Unicode BOM
(byte-ordering marker) pseudo-character, which is legal at the start of a
file and which is automatically prepended by some text editors and
word-processors when they save a UTF-8 file (though in fact it is not
required by the UTF-8 specification). Anyone calling |TextFiles::utf8_fgetc| must
check the return value for |EOF| every time, and for |0xFEFF| every time we
might be at the start of the file being read.

@c
typedef struct unicode_file_buffer {
	char unicode_feed_buffer[32]; /* holds a single escape such as "[unicode 3106]" */
	int ufb_counter; /* position in the unicode feed buffer */
} unicode_file_buffer;

unicode_file_buffer TextFiles::create_ufb(void) {
	unicode_file_buffer ufb;
	ufb.ufb_counter = -1;
	return ufb;
}

int TextFiles::utf8_fgetc(FILE *from, char **or_from, int escape_oddities,
	unicode_file_buffer *ufb) {
	int c = EOF, conts;
	if (ufb->ufb_counter >= 0) {
		if (ufb->unicode_feed_buffer[ufb->ufb_counter] == 0) ufb->ufb_counter = -1;
		else return ufb->unicode_feed_buffer[ufb->ufb_counter++];
	}
	if (from) c = fgetc(from); else if (or_from) c = ((unsigned char) *((*or_from)++));
	if (c == EOF) return c; /* ruling out EOF leaves a genuine byte from the file */
	if (c<0x80) return c; /* in all other cases, a UTF-8 continuation sequence begins */

	@<Unpack one to five continuation bytes to obtain the Unicode character code@>;
    @<Return non-ASCII codes in the intersection of ISO Latin-1 and ZSCII as literals@>;
    @<Return Unicode fancy equivalents as simpler literals@>;

	if (c == 0xFEFF) return c; /* the Unicode BOM non-character */

	if (escape_oddities == FALSE) return c;

	sprintf(ufb->unicode_feed_buffer, "[unicode %d]", c);
	ufb->ufb_counter = 1;
	return '[';
}

@ Not every byte sequence is legal in a UTF-8 file: if we find a malformed
continuation, we process it as a question mark rather than throwing a
fatal error (which is pretty well the only alternative here). The user
is likely to see problem messages later on which arise from the question
marks, and that will have to do.

@<Unpack one to five continuation bytes to obtain the Unicode character code@> =
    if (c<0xC0) return '?'; /* malformed UTF-8 */
	if (c<0xE0) { c = c & 0x1f; conts = 1; }
	else if (c<0xF0) { c = c & 0xf; conts = 2; }
	else if (c<0xF8) { c = c & 0x7; conts = 3; }
	else if (c<0xFC) { c = c & 0x3; conts = 4; }
	else { c = c & 0x1; conts = 5; }
	while (conts > 0) {
		int d = EOF;
		if (from) d = fgetc(from); else if (or_from) d = ((unsigned char) *((*or_from)++));
		if (d == EOF) return '?'; /* malformed UTF-8 */
		c = c << 6;
		c = c + (d & 0x3F);
		conts--;
	}

@ For the ZSCII character set, see {\it The Inform 6 Designer's Manual}, or
{\it The Z-Machine Standards Document}. It offers a range of west European
accented letters which almost, but not quite, matches those on offer in
ISO Latin-1. This omits certain obscure glyphs: among them, the Icelandic
lower case eth, which is good because this character value is used for hackery
with error messages, so it is good to know that it cannot occur from the
source text.

We let the multiplication sign |0xd7| through even though ZSCII doesn't
support it, but convert it to an "x": this is so that we can parse numbers
in scientific notation.

@<Return non-ASCII codes in the intersection of ISO Latin-1 and ZSCII as literals@> =
	if ((c == 0xa1) || (c == 0xa3) || (c == 0xbf)) return c; /* pound sign, inverted ! and ? */
	if (c == 0xd7) return 'x'; /* convert multiplication sign to lower case "x" */
	if ((c >= 0xc0) && (c <= 0xff)) { /* accented West European letters, but... */
		if ((c != 0xd0) && (c != 0xf0) && /* not Icelandic eths */
		    (c != 0xde) && (c != 0xfe) && /* nor Icelandic thorns */
			(c != 0xf7)) /* nor division signs */
			return c;
	}

@ |inlib| errs on the safe side, accepting em-rules and non-breaking spaces, etc.,
where it would normally expect hyphens and ordinary spaces: this is intended
for the benefit of users with helpful word-processors which autocorrect
hyphens into em-rules when they are flanked by spaces, and so on.

@<Return Unicode fancy equivalents as simpler literals@> =
	if (c == 0x85) return '\x0d'; /* NEL, or "next line" */
	if (c == 0xa0) return ' '; /* non-breaking space */
	if ((c >= 0x2000) && (c <= 0x200a)) return ' '; /* space variants */
	if ((c >= 0x2010) && (c <= 0x2014)) return '-'; /* rules and dashes */
	if ((c >= 0x2018) && (c <= 0x2019)) return '\''; /* smart single quotes */
	if ((c >= 0x201c) && (c <= 0x201d)) return '"'; /* smart double quotes */
	if ((c >= 0x2028) && (c <= 0x2029)) return '\x0d'; /* fancy newlines */
