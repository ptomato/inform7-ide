1/text: Text Files.

@Purpose: To read text files of whatever flavour, one line at a time.

@Interface:

-- Owns struct text_file_position (private)

@Definitions:

@

@c
typedef struct text_file_position {
	char text_file_filename[MAX_FILENAME_LENGTH];
	int line_count;
	int line_position;
	int skip_terminator;
	int actively_scanning; /* whether we are still interested in the rest of the file */
} text_file_position;

@-------------------------------------------------------------------------------

@p Text file positions.
This is useful for error messages:

@c
/**/ void describe_file_position(text_file_position *tfp) {
	if (tfp == NULL) return;
	fprintf(stderr, "%s, line %d: ", tfp->text_file_filename, tfp->line_count);
}

@

@c
/**/ int tfp_get_line_count(text_file_position *tfp) {
	if (tfp == NULL) return 0;
	return tfp->line_count;
}

@

@c
/**/ void tfp_lose_interest(text_file_position *tfp) {
	tfp->actively_scanning = FALSE;
}


@p Error messages.
|cBlorb| is only minimally helpful when diagnosing problems, because it's
intended to be used as the back end of a system which only generates correct
blurb files, so that everything will work -- ideally, the Inform user will
never know that |cBlorb| exists.

@c
text_file_position *error_position = NULL;
/**/ void set_error_position(text_file_position *tfp) {
	error_position = tfp;
}

/**/ void error(char *erm) {
 	describe_file_position(error_position);
   fprintf(stderr, "Error: %s\n", erm);
    error_count++;
}

/**/ void error_1(char *erm, char *s) {
	describe_file_position(error_position);
    fprintf(stderr, "Error: %s: '%s'\n", erm, s);
    error_count++;
}

/**/ void fatal(char *erm) {
	describe_file_position(error_position);
    fprintf(stderr, "Fatal error: %s\n", erm);
    exit(1);
}

/**/ void fatal_fs(char *erm, char *fn) {
	describe_file_position(error_position);
    fprintf(stderr, "Fatal error: %s: filename '%s'\n", erm, fn);
    exit(1);
}

@p File handling.
We read lines in, delimited by any of the standard line-ending characters,
and send them one at a time to a function called |iterator|. 

@c
/**/ void file_read(char *filename, char *message, int serious,
	void (iterator)(char *, text_file_position *), text_file_position *start_at) {
	FILE *HANDLE;
	text_file_position tfp;
	@<Open the text file@>;
	@<Set the initial position, seeking it in the file if need be@>;
	@<Read in lines and send them one by one to the iterator@>;
	fclose(HANDLE);
}

@

@<Open the text file@> =
	if (strlen(filename) >= MAX_FILENAME_LENGTH) {
		if (serious) fatal_fs("filename too long", filename);
		error_1("filename too long", filename);
		return;
	}
	HANDLE = fopen(filename, "r");
	if (HANDLE == NULL) {
		if (message == NULL) return;
		if (serious) fatal_fs(message, filename);
		else { error_1(message, filename); return; }
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
			if (serious) fatal_fs("unable to seek position in file", filename);
			error_1("unable to seek position in file", filename);
			return;
		}
	}
	tfp.actively_scanning = TRUE;
	strcpy(tfp.text_file_filename, filename);

@ We aim to get this right whether the lines are terminated by |0A|, |0D|,
|0A 0D| or |0D 0A|. The final line is not required to be terminated.

@<Read in lines and send them one by one to the iterator@> =
	char line[MAX_TEXT_FILE_LINE_LENGTH+1];
	int i = 0, c = ' ';
	while ((c != EOF) && (tfp.actively_scanning)) {
		c = fgetc(HANDLE);
		if ((c == EOF) || (c == '\x0a') || (c == '\x0d')) {
			line[i] = 0;
			if ((i > 0) || (c != tfp.skip_terminator)) {
				@<Feed the completed line to the iterator routine@>;
				if (c == '\x0a') tfp.skip_terminator = '\x0d';
				if (c == '\x0d') tfp.skip_terminator = '\x0a';
			} else tfp.skip_terminator = 'X';
			@<Update the text file position@>;
			i = 0;
		} else {
			if (i < MAX_TEXT_FILE_LINE_LENGTH) line[i++] = (char) c;
			else {
				if (serious) fatal_fs("line too long", filename);
				error_1("line too long (truncating it)", filename);
			}
		}
	}
	if ((i > 0) && (tfp.actively_scanning))
		@<Feed the completed line to the iterator routine@>;

@ We update the line counter only when a line is actually sent:

@<Feed the completed line to the iterator routine@> =
	iterator(line, &tfp);
	tfp.line_count++;

@ But we update the text file position after every apparent line terminator.
This is because we might otherwise, on a Windows text file, end up with an
|ftell| position in between the |CR| and the |LF|; if we resume at that point,
later on, we'll then have an off-by-one error in the line numbering in the
resumption as compared to during the original pass.

Properly speaking, |ftell| returns a long |int|, not an |int|, but on a
32-bit integer machine -- which Inform requires -- this gives us room for files
to run to 2GB. Text files seldom come that large.

@<Update the text file position@> =
	tfp.line_position = (int) (ftell(HANDLE));
	if (tfp.line_position == -1) {
		if (serious) fatal_fs("unable to determine position in file", filename);
		error_1("unable to determine position in file", filename);
	}

@p Two string utilities.

@c
/**/ char *trim_white_space(char *original) {
	int i;
	for (i=0; white_space(original[i]); i++) ;
	original += i;
	for (i=strlen(original)-1; ((i>=0) && (white_space(original[i]))); i--)
		original[i] = 0;
	return original;
}

@

@c
/**/ void extract_word(char *fword, char *line, int size, int word) {
	int i = 0;
	fword[0] = 0;
	while (word > 0) {
		word--;
		while (white_space(line[i])) i++;
		int j = 0;
		while ((line[i]) && (!white_space(line[i]))) {
			if (j < size-1) fword[j++] = tolower(line[i]);
			i++;
		}
		fword[j] = 0;
		if (line[i] == 0) break;
	}
	if (word > 0) fword[0] = 0;
}

@ Where we define white space as spaces and tabs only:

@c
int white_space(int c) { if ((c == ' ') || (c == '\t')) return TRUE; return FALSE; }

@p Other file utilities.
Although this section is called ``Text Files'', it also has a couple of
general-purpose file utilities:

@c
/**/ char *get_filename_extension(char *filename) {
	int i = strlen(filename) - 1;
	while ((i>=0) && (filename[i] != '.') && (filename[i] != SEP_CHAR)) i--;
	if ((i<0) || (filename[i] == SEP_CHAR)) return filename + strlen(filename);
	return filename + i;
}

/**/ char *get_filename_leafname(char *filename) {
	int i = strlen(filename) - 1;
	while ((i>=0) && (filename[i] != SEP_CHAR)) i--;
	return filename + i + 1;
}

/**/ int file_exists(char *filename) {
	FILE *TEST = fopen(filename, "r");
	if (TEST) { fclose(TEST); return TRUE; }
	return FALSE;
}

/**/ long int file_size(char *filename) {
	FILE *TEST_FILE = fopen(filename, "rb");
	if (TEST_FILE) {
		if (fseek(TEST_FILE, 0, SEEK_END) == 0) {
			long int file_size = ftell(TEST_FILE);
			if (file_size == -1L) fatal_fs("ftell failed on linked file", filename);
			return file_size;
		} else fatal_fs("fseek failed on linked file", filename);
		fclose(TEST_FILE);
	}
	return -1L;
}

/**/ void copy_file(char *from, char *to) {
	if ((from == NULL) || (to == NULL) || (strcmp(from, to) == 0))
		fatal("files confused in copier");

	FILE *FROM = fopen(from, "rb"); if (FROM == NULL) fatal_fs("unable to read file", from);
	FILE *TO = fopen(to, "wb"); if (TO == NULL) fatal_fs("unable to write to file", to);

	while (TRUE) {
		int c = fgetc(FROM);
		if (c == EOF) break;
		putc(c, TO);
	}

	fclose(FROM); fclose(TO);
}
