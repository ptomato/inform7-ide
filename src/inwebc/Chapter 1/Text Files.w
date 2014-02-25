1/text: Text Files.

@Purpose: To read text files of whatever flavour, one line at a time.

@p Text file positions.
(The code in this section is borrowed from |cblorb|, with very little change.)
Here's how we record a position in a text file:

@c
typedef struct text_file_position {
	char *text_file_filename;
	int line_count;
	int line_position;
	int skip_terminator;
	int actively_scanning; /* whether we are still interested in the rest of the file */
} text_file_position;

@ And this is for a real nowhere man:

@c
text_file_position nowhere_position(void) {
	text_file_position tfp;
	tfp.text_file_filename = "<no file>";
	tfp.line_count = 0;
	tfp.line_position = 0;
	tfp.skip_terminator = FALSE;
	tfp.actively_scanning = FALSE;
	return tfp;
}

@p Text file iteration.
We read lines in, delimited by any of the standard line-ending characters,
and send them one at a time to a function called |iterator|. 

@c
int file_read(char *filename, char *message, int serious,
	void (iterator)(char *, text_file_position *, void *), text_file_position *start_at,
	void *state) {
	char *stored_filename = new_string(filename);
	FILE *HANDLE;
	text_file_position tfp;
	@<Open the text file@>;
	@<Set the initial position, seeking it in the file if need be@>;
	@<Read in lines and send them one by one to the iterator@>;
	fclose(HANDLE);
	return tfp.line_count;
}

@

@<Open the text file@> =
	HANDLE = fopen(filename, "rb");
	if (HANDLE == NULL) {
		if (message == NULL) return 0;
		if (serious) fatal_filing_system_error(message, filename);
		else { nonfatal_filing_system_error(message, filename); return 0; }
	}

@ The ANSI definition of |ftell| and |fseek| says that, with text files, the
only definite position value is 0 -- meaning the beginning of the file -- and
this is what we initialise |line_position| to. We must otherwise only write
values returned by |ftell| into this field.

@<Set the initial position, seeking it in the file if need be@> =
	if (start_at == NULL) {
		tfp.line_count = 1;
		tfp.line_position = 0;
		tfp.skip_terminator = 'X';
	} else {
		tfp = *start_at;
		if (fseek(HANDLE, (long int) (tfp.line_position), SEEK_SET)) {
			if (serious) fatal_filing_system_error("unable to seek position in file", filename);
			nonfatal_filing_system_error("unable to seek position in file", filename);
			return 0;
		}
	}
	tfp.actively_scanning = TRUE;
	tfp.text_file_filename = stored_filename;

@ We aim to get this right whether the lines are terminated by |0A|, |0D|,
|0A 0D| or |0D 0A|. The final line is not required to be terminated.

@<Read in lines and send them one by one to the iterator@> =
	string line; in_strcpy(line, "");
	int i = 0, c = ' ';
	int warned = FALSE;
	while ((c != EOF) && (tfp.actively_scanning)) {
		c = fgetc(HANDLE);
		if ((c == EOF) || (c == '\x0a') || (c == '\x0d')) {
			in_set(line, i, 0);
			if ((i > 0) || (c != tfp.skip_terminator)) {
				@<Feed the completed line to the iterator routine@>;
				if (c == '\x0a') tfp.skip_terminator = '\x0d';
				if (c == '\x0d') tfp.skip_terminator = '\x0a';
			} else tfp.skip_terminator = 'X';
			@<Update the text file position@>;
			i = 0;
		} else {
			if (i < MAX_STRING_LENGTH) in_set(line, i++, (char) c);
			else {
				if (serious) fatal_filing_system_error("line too long", filename);
				if (warned == FALSE) {
					error_in_text_file("line too long (truncating it)", &tfp);
					warned = TRUE;
				}
			}
		}
	}
	if ((i > 0) && (tfp.actively_scanning))
		@<Feed the completed line to the iterator routine@>;


@ We update the line counter only when a line is actually sent:

@<Feed the completed line to the iterator routine@> =
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

@<Update the text file position@> =
	tfp.line_position = (int) (ftell(HANDLE));
	if (tfp.line_position == -1) {
		if (serious)
			fatal_filing_system_error("unable to determine position in file", filename);
		else
			nonfatal_filing_system_error("unable to determine position in file", filename);
	}
