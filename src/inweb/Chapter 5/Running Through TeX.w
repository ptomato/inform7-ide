[RunningTeX::] Running Through TeX.

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
	struct filename *PDF_filename;
	MEMORY_MANAGEMENT
} tex_results;

@

@c
int serious_error_count = 0;
tex_results *via_tex = NULL;

void RunningTeX::post_process_weave(weave_target *wv, int open_afterwards) {
	tex_results *res = CREATE(tex_results);
	wv->post_processing_results = (void *) res;
	res->overfull_hbox_count = 0;
	res->tex_error_count = 0;
	res->page_count = 0;
	res->pdf_size = 0;

	filename *console_filename = Filenames::set_extension(wv->weave_to, "console");
	filename *log_filename = Filenames::set_extension(wv->weave_to, "log");
	filename *pdf_filename = Filenames::set_extension(wv->weave_to, "pdf");
	res->PDF_filename = pdf_filename;
	
	serious_error_count = 0; via_tex = res;
	
	@<Call TeX and transcribe its output into a console file@>;
	@<Read back the console file and parse it for error messages@>;
	@<Remove the now redundant TeX, console and log files, to reduce clutter@>;

	if (open_afterwards) @<Try to open the PDF file in the host operating system@>;
}

@

@<Call TeX and transcribe its output into a console file@> =
	TEMPORARY_STREAM
	filename *tex_rel = Filenames::without_path(wv->weave_to);
	filename *console_rel = Filenames::without_path(console_filename);
	
	Shell::plain(TEMP, "cd ");
	Shell::quote_path(TEMP, Filenames::get_path_to(wv->weave_to));
	Shell::plain(TEMP, "; ");
	
	char *tool = pdftex_configuration;
	if (wv->format == dvi_format) tool = tex_configuration;
	Shell::plain(TEMP, tool);
	Shell::plain(TEMP, " -interaction=scrollmode ");
	Shell::quote_file(TEMP, tex_rel);
	Shell::plain(TEMP, ">");
	Shell::quote_file(TEMP, console_rel);
	Shell::run(TEMP);
	CLOSE_TEMPORARY_STREAM

@ |TeX| helpfully reports the size and page count of what it produces, and
we're not too proud to scrape that information out of the console file, besides
the error messages (which begin with an exclamation mark in column 1).

@<Read back the console file and parse it for error messages@> =
	TextFiles::read_with_lines_to_ISO(console_filename,
		"can't open console file", TRUE, RunningTeX::scan_console_line, NULL, NULL);

@ The log file we never wanted, but |TeX| produced it anyway; it's really a
verbose form of its console output. Now it can go. So can the console file
and even the |TeX| source, since that was mechanically generated from the
web, and so is of no lasting value. The one exception is that we keep the
console file in the event of serious errors, since otherwise it's impossible
for the user to find out what those errors were.

@<Remove the now redundant TeX, console and log files, to reduce clutter@> =
	if (serious_error_count == 0) {
		Shell::rm(console_filename);
		Shell::rm(log_filename);
		Shell::rm(wv->weave_to);
	}

@ We often want to see the PDF immediately, so:

@<Try to open the PDF file in the host operating system@> =
	if (CStrings::eq(open_configuration, ""))
		Errors::fatal("no way to open PDF (see configuration file)");
	else
		Shell::apply(open_configuration, pdf_filename);

@

@c
void RunningTeX::scan_console_line(char *line, text_file_position *tfp, void *unused_state) {
	string found_text1;
	string found_text2;	
	if (ISORegexp::match_2(line, "Output written %c*? %((%d+) page%c*?(%d+) bytes%).", found_text1, found_text2)) {
		via_tex->page_count = atoi(found_text1);
		via_tex->pdf_size = atoi(found_text2);
	}
	if (ISORegexp::match_0(line, "%c+verfull \\hbox%c+"))
		via_tex->overfull_hbox_count++;
	else if (line[0] == '!') {
		via_tex->tex_error_count++;
		serious_error_count++;
	}
}

@

@c
void RunningTeX::report_on_post_processing(weave_target *wv) {
	tex_results *res = wv->post_processing_results;
	if (res) {
		printf(": %dpp %dK", res->page_count, res->pdf_size/1024);
		if (res->overfull_hbox_count > 0)
			printf(", %d overfull hbox(es)", res->overfull_hbox_count);
		if (res->tex_error_count > 0)
			printf(", %d error(s)", res->tex_error_count);
	}
}

@ And here are some details to do with the results of post-processing.

@c
int RunningTeX::substitute_post_processing_data(char *to, weave_target *wv, char *detail) {
	if (wv) {
		tex_results *res = wv->post_processing_results;
		if (res) {
			if (CStrings::eq(detail, "PDF Size")) {
				CSTRING_WRITE(to, "%dKB", res->pdf_size/1024);
			} else if (CStrings::eq(detail, "Extent")) {
				CSTRING_WRITE(to, "%dpp", res->page_count);
			} else if (CStrings::eq(detail, "Leafname")) {
				Str::copy_to_ISO_string(to, Filenames::get_leafname(res->PDF_filename), MAX_FILENAME_LENGTH);
			} else if (CStrings::eq(detail, "Errors")) {
				CStrings::copy(to, "");
				if ((res->overfull_hbox_count > 0) || (res->tex_error_count > 0))
					CSTRING_WRITE(to, ": ");
				if (res->overfull_hbox_count > 0)
					CSTRING_WRITE(to + CStrings::len(to), "%d overfull line%s",
						res->overfull_hbox_count,
						(res->overfull_hbox_count>1)?"s":"");
				if ((res->overfull_hbox_count > 0) && (res->tex_error_count > 0))
					CSTRING_WRITE(to + CStrings::len(to), ", ");
				if (res->tex_error_count > 0)
					CSTRING_WRITE(to + CStrings::len(to), "%d TeX error%s",
						res->tex_error_count,
						(res->tex_error_count>1)?"s":"");
			} else return FALSE;
			return TRUE;
		}
	}
	return FALSE;
}
