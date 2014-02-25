2/errs: Error Messages.

@Purpose: To handle any errors we need to issue.

@p Error messages.
Ah, they kill you; or they don't. The fatal kind cause an exit code of 2, to
distinguish this from a proper completion of Inweb in which non-fatal
errors occur.

@c
void fatal_error(char *message) {
	fatal_error_with_parameter("%s", message);
}

void fatal_error_with_parameter(char *message, char *parameter) {
	fprintf(stderr, "inweb: fatal error: ");
	fprintf(stderr, message, parameter);
	fprintf(stderr, "\n");
	free_memory(); /* note that this is unable to cause fatal errors */
	exit(2);
}

@ The trick with error messages is to indicate where they occur, and we can
specify this at three levels of abstraction:

@c
void error_in_web(char *message, source_line *sl) {
	if (sl) {
		error_in_text_file(message, &(sl->source));
		fprintf(stderr, "%07d  %s\n", sl->source.line_count, sl->text);
	} else {
		error_in_text_file(message, NULL);
	}
}

void error_in_text_file(char *message, text_file_position *here) {
	if (here)
		error_at_position(message, here->text_file_filename, here->line_count);
	else
		error_at_position(message, NULL, 0);
}

void error_at_position(char *message, char *file, int line) {
	fprintf(stderr, "inweb: ");
	if (file) fprintf(stderr, "%s, line %d: ", file, line);
	fprintf(stderr, "%s\n", message);
	no_inweb_errors++;
}

@ Filing system errors are just about worth functions of their own:

@c
void fatal_filing_system_error(char *erm, char *fn) {
	string err;
	in_sprintf(err, "%s: file '%s'", erm, fn);
	fatal_error(err);
}

void nonfatal_filing_system_error(char *erm, char *fn) {
	string err;
	in_sprintf(err, "%s: file '%s'", erm, fn);
	error_in_text_file(err, NULL);
}
