6/rtex: Running Through TeX.

@Purpose: To post-process a weave by running it through TeX, or one of its
variant typesetting programs.

@p Running TeX.
Although we are running |pdftex|, a modern variant of |TeX|, rather than the
original, they are very similar as command-line tools; the difference is
that the output is a PDF file rather than a DVI file, Knuth's original stab
at the same basic idea.

In particular, we call it in "scrollmode" so that any errors whizz by
rather than interrupting or halting the session. Because of that, we spool
the output onto a console file which we can then read in and parse to find
the number of errors actually generated. Prime among errors is the ``overfull
hbox error'', a defect of |TeX| resulting from its inability to adjust
letter spacing, so that it requires us to adjust the copy to fit the margins
of the page properly. (In practice we get this here by having code lines which
are too wide to display.)

@c
typedef struct tex_results {
	int overfull_hbox_count;
	int tex_error_count;
	int page_count;
	int pdf_size;
	string PDF_filename;
	MEMORY_MANAGEMENT
} tex_results;

@

@c
int serious_error_count = 0;
tex_results *via_tex = NULL;

void tex_post_process_weave(weave_target *wv, int open_afterwards) {
	tex_results *res = CREATE(tex_results);
	wv->post_processing_results = (void *) res;
	res->overfull_hbox_count = 0;
	res->tex_error_count = 0;
	res->page_count = 0;
	res->pdf_size = 0;
	
	string tex_leafname; in_strcpy(tex_leafname, "");
	string path_to_tex; in_strcpy(path_to_tex, "");
	string console_filename; in_strcpy(console_filename, "");
	string console_leafname; in_strcpy(console_leafname, "");
	string log_filename; in_strcpy(log_filename, "");
	string pdf_filename; in_strcpy(pdf_filename, "");
	@<Work out these filenames and leafnames@>;
	
	serious_error_count = 0; via_tex = res;
	
	@<Call TeX and transcribe its output into a console file@>;
	@<Read back the console file and parse it for error messages@>;
	@<Remove the now redundant TeX, console and log files, to reduce clutter@>;

	if (open_afterwards) @<Try to open the PDF file in the host operating system@>;
}

@

@<Work out these filenames and leafnames@> =
	if (pattern_match(wv->weave_to, "(%c+)/(%c+?)")) {
		in_strcpy(path_to_tex, found_text1);
		in_strcpy(tex_leafname, found_text2);
	} else {
		in_strcpy(tex_leafname, wv->weave_to);
		in_strcpy(path_to_tex, "");
	}
	copy_with_changed_extension(console_leafname, tex_leafname, "tex", "console");
	copy_with_changed_extension(console_filename, wv->weave_to, "tex", "console");
	copy_with_changed_extension(log_filename, wv->weave_to, "tex", "log");
	copy_with_changed_extension(pdf_filename, wv->weave_to, "tex", "pdf");
	in_strcpy(res->PDF_filename, pdf_filename);

@

@<Call TeX and transcribe its output into a console file@> =
	string console_command; in_strcpy(console_command, "");
	if (path_to_tex[0]) in_sprintf(console_command, "cd \"%s\"; ", path_to_tex);
	else in_strcpy(console_command, "");
	char *tool = pdftex_configuration;
	if (wv->format == dvi_format) tool = tex_configuration;
	in_sprintf(console_command + in_strlen(console_command),
		"%s -interaction=scrollmode \"%s\" >\"%s\"",
		tool,
		tex_leafname,
		console_leafname);
	issue_os_command_0(console_command);

@ |TeX| helpfully reports the size and page count of what it produces, and
we're not too proud to scrape that information out of the console file, besides
the error messages (which begin with an exclamation mark in column 1).

@<Read back the console file and parse it for error messages@> =
	file_read(console_filename, "can't open console file", TRUE, scan_console_line, NULL, NULL);

@ The log file we never wanted, but |TeX| produced it anyway; it's really a
verbose form of its console output. Now it can go. So can the console file
and even the |TeX| source, since that was mechanically generated from the
web, and so is of no lasting value. The one exception is that we keep the
console file in the event of serious errors, since otherwise it's impossible
for the user to find out what those errors were.

@<Remove the now redundant TeX, console and log files, to reduce clutter@> =
	if (serious_error_count == 0) {
		issue_os_command_1("rm \"%s\"", console_filename);
		issue_os_command_1("rm \"%s\"", log_filename);
		issue_os_command_1("rm \"%s\"", wv->weave_to);
	}

@ We often want to see the PDF immediately, so:

@<Try to open the PDF file in the host operating system@> =
	if (in_string_eq(open_configuration, ""))
		fatal_error("no way to open PDF (see configuration file)");
	else
		issue_os_command_2("%s \"%s\"", open_configuration, pdf_filename);

@

@c
void scan_console_line(char *line, text_file_position *tfp, void *unused_state) {
	if (pattern_match(line, "Output written %c*? %((%d+) page%c*?(%d+) bytes%).")) {
		via_tex->page_count = atoi(found_text1);
		via_tex->pdf_size = atoi(found_text2);
	}
	if (pattern_match(line, "%c+verfull \\hbox%c+"))
		via_tex->overfull_hbox_count++;
	else if (line[0] == '!') {
		via_tex->tex_error_count++;
		serious_error_count++;
	}
}

@

@c
void tex_report_on_post_processing(weave_target *wv) {
	tex_results *res = wv->post_processing_results;
	if (res) {
		printf(": %dpp %dK", res->page_count, res->pdf_size/1024);
		if (res->overfull_hbox_count > 0)
			printf(", %d overfull hbox(es)", res->overfull_hbox_count);
		if (res->tex_error_count > 0)
			printf(", %d error(s)", res->tex_error_count);
	}
}

@

@c
void copy_with_changed_extension(char *to, char *from, char *old_ext, char *new_ext) {
	in_strcpy(to, from);
	int n = in_strlen(to) - in_strlen(old_ext) - 1;
	if ((n>=0) && (to[n] == '.') && (in_string_eq(to+n+1, old_ext)))
		in_strcpy(to+n+1, new_ext);
}

@ And here are some details to do with the results of post-processing.

@c
int tex_substitute_post_processing_data(char *to, weave_target *wv, char *detail) {
	if (wv) {
		tex_results *res = wv->post_processing_results;
		if (res) {
			if (in_string_eq(detail, "PDF Size")) {
				in_sprintf(to, "%dKB", res->pdf_size/1024);
			} else if (in_string_eq(detail, "Extent")) {
				in_sprintf(to, "%dpp", res->page_count);
			} else if (in_string_eq(detail, "Leafname")) {
				if (pattern_match(res->PDF_filename, "%c+/(%c+?)"))
					in_strcpy(to, found_text1);
				else
					in_strcpy(to, res->PDF_filename);
			} else if (in_string_eq(detail, "Errors")) {
				in_strcpy(to, "");
				if ((res->overfull_hbox_count > 0) || (res->tex_error_count > 0))
					in_sprintf(to, ": ");
				if (res->overfull_hbox_count > 0)
					in_sprintf(to + in_strlen(to), "%d overfull line%s",
						res->overfull_hbox_count,
						(res->overfull_hbox_count>1)?"s":"");
				if ((res->overfull_hbox_count > 0) && (res->tex_error_count > 0))
					in_sprintf(to + in_strlen(to), ", ");
				if (res->tex_error_count > 0)
					in_sprintf(to + in_strlen(to), "%d TeX error%s",
						res->tex_error_count,
						(res->tex_error_count>1)?"s":"");
			} else return FALSE;
			return TRUE;
		}
	}
	return FALSE;
}
