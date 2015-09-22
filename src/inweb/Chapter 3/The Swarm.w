[Swarm::] The Swarm.

@Purpose: To feed multiple output requests to the weaver, and to present
weaver results, and update indexes or contents pages.

@p Swarming.
A "weave" occurs when Inweb takes a portion of the web -- one section, one
chapter, or the whole thing -- and writes it out in a human-readable form (or
in some intermediate state which can be made into one, like a |TeX| file).
There can be many weaves in a single run of Inweb, in which case we call the
flurry of weaving a "swarm". Let us hope the result is a glittering cloud,
like the swarm of locusts in the title of Chapter 25 of Laura Ingalls Wilder's
"On the Banks of Plum Creek".

This routine is called with mode |SWARM_SECTIONS|, |SWARM_CHAPTERS| or
|SWARM_INDEX|, so in a non-swarming run it isn't called at all.

@c
weave_target *swarm_leader = NULL; /* the most inclusive one we weave */

void Swarm::weave(web *W, char *subweb, int swarm_mode, theme_tag *tag, char *format) {
	swarm_leader = NULL;

	if ((swarm_mode & SWARM_CHAPTERS) || (swarm_mode & SWARM_SECTIONS)) {
		/* weave Complete web */
		if (swarm_mode & SWARM_COMPLETE)
			if (CStrings::eq(subweb, "0"))
				swarm_leader = Swarm::weave_subset(W, "0", FALSE, tag, format);

		for (chapter *C = W->first_chapter; C; C = C->next_chapter)
			if (C->imported == FALSE) {
				if (swarm_mode & SWARM_CHAPTERS) {
					/* weave single chapter */
					if ((W->chaptered == TRUE) && (Reader::sigil_within(C->ch_sigil, subweb))) {
						C->ch_weave = Swarm::weave_subset(W, C->ch_sigil, FALSE, tag, format);
						if (CStrings::ne(subweb, "")) swarm_leader = C->ch_weave;
					}
				}
				if (swarm_mode & SWARM_SECTIONS) {
					/* weave individual sections */
					for (section *S = C->first_section; S; S = S->next_section)
						if (Reader::sigil_within(S->sigil, subweb))
							S->sect_weave = Swarm::weave_subset(W, S->sigil, FALSE, tag, format);
				}
			}
	}

	Swarm::weave_index_templates(W, subweb, format);
}

@ The following is where an individual weave task begins, whether it comes
from the swarm, or has been specified at the command line (in which case
the call comes from Program Control).

@c
weave_target *Swarm::weave_subset(web *W, char *subweb, int open_afterwards, theme_tag *tag, char *format) {
	weave_target *wv = NULL;
	if (no_inweb_errors == 0) {
		Analyser::analyse_code(W);
		@<Compile a set of instructions for the weaver@>;
		if (Weaver::weave_source(W, wv) == 0) /* i.e., the number of lines woven was zero */
			Errors::fatal("empty weave request");
		Formats::post_process_weave(wv, open_afterwards); /* e.g., run through |TeX| */
		@<Report on the outcome of the weave to the console@>;
	}
	return wv;
}

@ Each individual weave generates one of the following sets of instructions:

@c
typedef struct weave_target {
	struct web *weave_web; /* which web we weave */
	string weave_sigil; /* which parts of the web in this weave */
	struct theme_tag *theme_match; /* pick out only paragraphs with this theme */
	string booklet_title;
	struct filename *weave_to; /* where to put it */
	struct weave_format *format; /* plain text, say, or HTML */
	struct filename *cover_sheet_to_use; /* leafname of the copy, or |NULL| for no cover */
	void *post_processing_results; /* optional typesetting diagnostics after running through */	
	MEMORY_MANAGEMENT
} weave_target;

@

@<Compile a set of instructions for the weaver@> =
	wv = CREATE(weave_target);
	wv->weave_web = W;
	CStrings::copy(wv->weave_sigil, subweb);
	wv->theme_match = tag;
	CStrings::copy(wv->booklet_title, "");
	wv->format = Formats::parse_format(format);
	wv->post_processing_results = NULL;
	wv->cover_sheet_to_use = NULL;
	if ((tag) && (tag->cover_sheet_when_woven[0])) {
		pathname *P = Pathnames::subfolder(W->path_to_web, "Materials");
		wv->cover_sheet_to_use =
			Filenames::in_folder(P, wv->theme_match->cover_sheet_when_woven);
	} else {
		wv->cover_sheet_to_use =
			Filenames::in_folder(path_to_inweb_materials, "cover-sheet");
	}

	string leafname; CStrings::copy(leafname, "");
	@<Translate the subweb sigil into details of what to weave@>;
	pathname *H = W->redirect_weaves_to;
	if (H == NULL) H = Pathnames::subfolder(W->path_to_web, "Woven");
	wv->weave_to = Filenames::in_folder(H, leafname);

@ From the sigil and the theme, we work out the weave title, the leafname,
and details of any cover-sheet to use.

@<Translate the subweb sigil into details of what to weave@> =
	if (CStrings::eq(subweb, "0")) {
		CStrings::copy(wv->booklet_title, "Complete Program");
		CStrings::copy(leafname, "Complete");
		if ((wv->theme_match) && (wv->theme_match->title_when_woven[0]))
			CStrings::copy(wv->booklet_title, wv->theme_match->title_when_woven);
		if ((wv->theme_match) && (wv->theme_match->leafname_when_woven[0]))
			CSTRING_WRITE(leafname, "%s", wv->theme_match->leafname_when_woven);
	} else if (ISORegexp::match_0(subweb, "%d+")) {
		CSTRING_WRITE(wv->booklet_title, "Chapter %s", subweb);
		CStrings::copy(leafname, wv->booklet_title);
	} else if (ISORegexp::match_0(subweb, "%[A-O]")) {
		CSTRING_WRITE(wv->booklet_title, "Appendix %s", subweb);
		CStrings::copy(leafname, wv->booklet_title);
	} else if (CStrings::eq(subweb, "P")) {
		CStrings::copy(wv->booklet_title, "Preliminaries");
		CStrings::copy(leafname, wv->booklet_title);
	} else {
		CSTRING_WRITE(wv->booklet_title, "%s", subweb);
		CStrings::copy(leafname, wv->booklet_title);
		wv->cover_sheet_to_use = NULL;
	}
	for (int i=0; leafname[i]; i++)
		if ((leafname[i] == '/') || (leafname[i] == ' '))
			CStrings::set_char(leafname, i, '-');
	CStrings::concatenate(leafname, Formats::weave_file_extension(wv->format));

@ Each weave results in a compressed one-line printed report:

@<Report on the outcome of the weave to the console@> =
	printf("[%s: %s", wv->booklet_title, wv->format->format_name);
	Formats::report_on_post_processing(wv);
	printf("]\n");

@ After every swarm, we rebuild all the indexes specified in the Index
Template list for the web:

@c
void Swarm::weave_index_templates(web *W, char *subweb, char *format) {
	@<Weave one or more index files@>;
	@<Copy in one or more additional files to accompany the index@>;
}

@

@<Weave one or more index files@> =
	if (Bibliographic::data_exists(W, "Index Template")) {
		string temp_list; CStrings::copy(temp_list, "");
		CStrings::copy(temp_list, Bibliographic::get_data(W, "Index Template"));
		while (temp_list[0]) {
			string found_text1;
			string found_text2;
			filename *index_to_make = NULL;
			if (ISORegexp::match_2(temp_list, "(%c+?), (%c+)", found_text1, found_text2)) {
				index_to_make = Filenames::from_string(found_text1);
				CStrings::copy(temp_list, found_text2);
			} else {
				index_to_make = Filenames::from_string(temp_list);
				CStrings::copy(temp_list, "");
			}
			Swarm::run_contents_interpreter(W, subweb, index_to_make, Filenames::get_leafname(index_to_make));
		}
	} else {
		char *index_leaf = NULL;
		if (Formats::index_pdfs(format)) {
			if (W->chaptered) 	index_leaf = "chaptered-tex-index.html";
			else 				index_leaf = "unchaptered-tex-index.html";
		} else {
			if (W->chaptered) 	index_leaf = "chaptered-index.html";
			else 				index_leaf = "unchaptered-index.html";
		}
		if (W->as_ebook) index_leaf = "epub-index.html";
		filename *OUT = Filenames::in_folder(path_to_inweb_materials, index_leaf);
		Swarm::run_contents_interpreter(W, subweb, OUT, Str::new_from_ISO_string("index.html"));
	}

@ The idea here is that an HTML index may need some binary image files to
go with it, for instance.

@<Copy in one or more additional files to accompany the index@> =
	string copy_list; CStrings::copy(copy_list, "");
	if (Bibliographic::data_exists(W, "Index Extras")) {
		CStrings::copy(copy_list, Bibliographic::get_data(W, "Index Extras"));
		Swarm::copy_files_into_weave(W, copy_list);
	} else {
		if (W->as_ebook) {
			filename *F = Filenames::in_folder(path_to_inweb_materials, "crumbs.gif");
			Swarm::copy_file_into_weave(W, F);
			Epub::note_image(W->as_ebook, F);
		} else {
			Swarm::copy_file_into_weave(W, Filenames::in_folder(path_to_inweb_materials, "download.gif"));
			Swarm::copy_file_into_weave(W, Filenames::in_folder(path_to_inweb_materials, "lemons.jpg"));
			Swarm::copy_file_into_weave(W, Filenames::in_folder(path_to_inweb_materials, "crumbs.gif"));
			Swarm::copy_file_into_weave(W, Filenames::in_folder(path_to_inweb_materials, "inweb.css"));
		}
	}

@ Where:

@c
void Swarm::copy_files_into_weave(web *W, char *copy_list) {
	while (CStrings::ne(copy_list, "")) {
		string file_to_copy; CStrings::copy(file_to_copy, "");
		string found_text1;
		string found_text2;
		if (ISORegexp::match_2(copy_list, "(%c+?), (%c+)", found_text1, found_text2)) {
			CStrings::copy(file_to_copy, found_text1);
			CStrings::copy(copy_list, found_text2);
		} else {
			CStrings::copy(file_to_copy, copy_list);
			CStrings::copy(copy_list, "");
		}
		filename *OUT = Filenames::from_string(file_to_copy);
		Swarm::copy_file_into_weave(W, OUT);
	}
}

void Swarm::copy_file_into_weave(web *W, filename *F) {
	pathname *H = W->redirect_weaves_to;
	if (H == NULL) H = Pathnames::subfolder(W->path_to_web, "Woven");
	Shell::copy(F, H, "");
}

@p The Contents Interpreter.
This is a little meta-language all of its very own, with a stack for holding
nested repeat loops, and a program counter and -- well, and nothing else to
speak of, in fact, except for the slightly unusual way that loop variables
provide context by changing the subject of what is discussed rather than
by being accessed directly.

@d TRACE_CI_EXECUTION FALSE /* set true for debugging */

@d MAX_TEMPLATE_LINES 256 /* maximum number of lines in template */
@d CI_STACK_CAPACITY 8 /* maximum recursion of chapter/section iteration */

@c
typedef struct contents_processor {
	char *tlines[MAX_TEMPLATE_LINES];
	int no_tlines;
	int repeat_stack_level[CI_STACK_CAPACITY];
	void *repeat_stack_variable[CI_STACK_CAPACITY];
	void *repeat_stack_threshold[CI_STACK_CAPACITY];
	int repeat_stack_startpos[CI_STACK_CAPACITY];
	int stack_pointer;
	char *restrict_to_subweb;
} contents_processor;

@

@c
contents_processor *cp = NULL;

void Swarm::run_contents_interpreter(web *W, char *subset,
	filename *template_filename, text_stream *contents_page_leafname) {
	PRINT("Weaving index file: %S\n", contents_page_leafname);
	contents_processor actual_cp; cp = &actual_cp;
	text_stream TO_struct;
	text_stream *OUT = &TO_struct;
	cp->no_tlines = 0;
	cp->restrict_to_subweb = subset;
	@<Read in the source file containing the contents page template@>;
	@<Open the contents page file to be constructed@>;

	int lpos = 0; /* This is our program counter: a line number in the template */
	cp->stack_pointer = 0; /* And this is our stack pointer for tracking of loops */
	while (lpos < cp->no_tlines) {
		string tl;
		CStrings::copy(tl, cp->tlines[lpos++]); /* Fetch the line at the program counter and advance */
		string found_text1;
		if (ISORegexp::match_1(tl, "(%c*?) ", found_text1)) CStrings::copy(tl, found_text1); /* Strip trailing spaces */
		if (TRACE_CI_EXECUTION) 
			@<Print line and contents of repeat stack@>;
		if ((ISORegexp::match_1(tl, "%[%[(%c+)%]%]", found_text1)) ||
			(ISORegexp::match_1(tl, " %[%[(%c+)%]%]", found_text1))) {
			string command; CStrings::copy(command, found_text1);
			@<Deal with a Select command@>;
			@<Deal with a Repeat command@>;
			@<Deal with a Repeat End command@>;
		}
		@<Skip line if inside an empty loop@>;
		@<Make substitutions of square-bracketed variables in line@>;
		WRITE("%s\n", tl); /* Copy the now finished line to the output */
		
		CYCLE: ;
	}
	STREAM_CLOSE(OUT);
}

@p File handling.

@<Read in the source file containing the contents page template@> =
	TextFiles::read_with_lines_to_ISO(template_filename,
		"can't find contents template", TRUE, Swarm::save_template_line, NULL, NULL);
	if (TRACE_CI_EXECUTION)
		PRINT("Read template <%f>: %d line(s)\n", template_filename, cp->no_tlines);

@ With the following iterator:

@c
void Swarm::save_template_line(char *line, text_file_position *tfp, void *unused_state) {
	if (cp->no_tlines < MAX_TEMPLATE_LINES)
		cp->tlines[cp->no_tlines++] = Memory::new_string(line);
}

@

@<Open the contents page file to be constructed@> =
	pathname *H = W->redirect_weaves_to;
	if (H == NULL) H = Pathnames::subfolder(W->path_to_web, "Woven");
	filename *Contents = Filenames::in_folder_S(H, contents_page_leafname);
	if (STREAM_OPEN_TO_FILE(OUT, Contents, ISO_ENC) == FALSE)
		Errors::fatal_with_file("unable to write contents file", Contents);
	if (W->as_ebook)
		Epub::note_page(W->as_ebook, Contents, "Index", "index");

@p The repeat stack and loops.

@<Print line and contents of repeat stack@> =
	printf("%04d: %s\nStack:", lpos-1, tl);
	for (int j=0; j<cp->stack_pointer; j++) {
		if (cp->repeat_stack_level[j] == CHAPTER_LEVEL)
			printf(" %d: %s/%s",
				j, ((chapter *) cp->repeat_stack_variable[j])->ch_sigil,
				((chapter *) cp->repeat_stack_threshold[j])->ch_sigil);
		else if (cp->repeat_stack_level[j] == SECTION_LEVEL)
			printf(" %d: %s/%s",
				j, ((section *) cp->repeat_stack_variable[j])->sigil,
				((section *) cp->repeat_stack_threshold[j])->sigil);
	}
	printf("\n");

@ We start the direct commands with Select, which is implemented as a
one-iteration loop in which the loop variable has the given section or
chapter as its value during the sole iteration.

@<Deal with a Select command@> =
	string found_text1;
	if (ISORegexp::match_1(command, "Select (%c*)", found_text1)) {
		string sigil; CStrings::copy(sigil, found_text1);
		section *S;
		LOOP_OVER(S, section)
			if (CStrings::eq(S->sigil, sigil)) {
				Swarm::start_CI_loop(SECTION_LEVEL, S, S, lpos);
				goto CYCLE;
			}
		chapter *C;
		LOOP_OVER(C, chapter)
			if (CStrings::eq(C->ch_sigil, sigil)) {
				Swarm::start_CI_loop(CHAPTER_LEVEL, C, C, lpos);
				goto CYCLE;
			}
		Errors::at_position("don't recognise the chapter or section abbreviation sigil",
			template_filename, lpos);			
		goto CYCLE;
	}

@ Next, a genuine loop beginning:

@<Deal with a Repeat command@> =
	int loop_level = 0;
	if (ISORegexp::match_0(command, "Repeat Chapter")) loop_level = CHAPTER_LEVEL;
	if (ISORegexp::match_0(command, "Repeat Section")) loop_level = SECTION_LEVEL;
	if (loop_level != 0) {
		void *from = NULL, *to = NULL;
		chapter *C = W->first_chapter;
		while ((C) && (C->imported)) C = C->next_chapter;
		if (loop_level == CHAPTER_LEVEL) {
			from = C;
			to = W->last_chapter;
			if (CStrings::ne(cp->restrict_to_subweb, "0")) {
				chapter *C;
				LOOP_OVER(C, chapter)
					if (CStrings::eq(C->ch_sigil, cp->restrict_to_subweb)) {
						from = C; to = C;
						break;
					}
			}
		}
		if (loop_level == SECTION_LEVEL) {
			chapter *within_chapter = Swarm::heading_topmost_on_stack(CHAPTER_LEVEL);
			if (within_chapter == NULL) {
				if (C) from = C->first_section;
				if (W->last_chapter) to = W->last_chapter->last_section;
			} else {
				from = within_chapter->first_section;
				to = within_chapter->last_section;
			}
		}
		if (from) Swarm::start_CI_loop(loop_level, from, to, lpos);
		goto CYCLE;
	}

@ And at the other bookend:

@<Deal with a Repeat End command@> =
	if ((ISORegexp::match_0(command, "End Repeat")) || (ISORegexp::match_0(command, "End Select"))) {
		if (cp->stack_pointer <= 0)
			Errors::at_position("stack underflow on contents template", template_filename, lpos);
		if (cp->repeat_stack_level[cp->stack_pointer-1] == SECTION_LEVEL) {
			section *S = cp->repeat_stack_variable[cp->stack_pointer-1];
			if (S == cp->repeat_stack_threshold[cp->stack_pointer-1])
				Swarm::end_CI_loop();
			else {
				cp->repeat_stack_variable[cp->stack_pointer-1] = S->next_section;
				lpos = cp->repeat_stack_startpos[cp->stack_pointer-1]; /* Back round loop */
			}
		} else {
			chapter *C = cp->repeat_stack_variable[cp->stack_pointer-1];
			if (C == cp->repeat_stack_threshold[cp->stack_pointer-1])
				Swarm::end_CI_loop();
			else {
				cp->repeat_stack_variable[cp->stack_pointer-1] = C->next_chapter;
				lpos = cp->repeat_stack_startpos[cp->stack_pointer-1]; /* Back round loop */
			}
		}
		goto CYCLE;
	}

@ It can happen that a section loop, at least, is empty:

@<Skip line if inside an empty loop@> =
	for (int rstl = cp->stack_pointer-1; rstl >= 0; rstl--)
		if ((cp->repeat_stack_level[cp->stack_pointer-1] == SECTION_LEVEL) &&
			(((section *) cp->repeat_stack_threshold[cp->stack_pointer-1])->next_section ==
			cp->repeat_stack_variable[cp->stack_pointer-1]))
				goto CYCLE;

@ If called with level |"Chapter"|, this returns the topmost chapter number
on the stack; and similarly for |"Section"|.

@c
void *Swarm::heading_topmost_on_stack(int level) {
	for (int rstl = cp->stack_pointer-1; rstl >= 0; rstl--)
		if (cp->repeat_stack_level[rstl] == level)
			return cp->repeat_stack_variable[rstl];
	return NULL;
}

@ This is the code for starting a loop, which stacks up the details, and
similarly for ending it by popping them again:

@d CHAPTER_LEVEL 1
@d SECTION_LEVEL 2

@c
void Swarm::start_CI_loop(int level, void *from, void *to, int pos) {
	if (cp->stack_pointer < CI_STACK_CAPACITY) {
		cp->repeat_stack_level[cp->stack_pointer] = level;
		cp->repeat_stack_variable[cp->stack_pointer] = from;
		cp->repeat_stack_threshold[cp->stack_pointer] = to;
		cp->repeat_stack_startpos[cp->stack_pointer++] = pos;
	}
}

void Swarm::end_CI_loop(void) {
	cp->stack_pointer--;
}

@p Variable substitutions.
We can now forget about this tiny stack machine: the one task left is to
take a line from the template, and make substitutions of variables into
its square-bracketed parts.

@<Make substitutions of square-bracketed variables in line@> =
	int slen, spos;
	while ((spos = ISORegexp::find_expansion(tl, '[', '[', ']', ']', &slen)) >= 0) {
		string left_part; CStrings::copy(left_part, tl); CStrings::truncate(left_part, spos);
		string varname; CStrings::copy(varname, tl+spos+2); CStrings::truncate(varname, slen-4);
		string substituted; CStrings::copy(substituted, varname);
		string right_part; CStrings::copy(right_part, tl+spos+slen);
		string found_text1;
		if (Bibliographic::data_exists(W, varname)) {
			@<Substitute any bibliographic datum named@>;
		} else if (ISORegexp::match_1(varname, "Chapter (%c+)", found_text1)) {
			string detail; CStrings::copy(detail, found_text1);
			chapter *C = Swarm::heading_topmost_on_stack(CHAPTER_LEVEL);
			if (C == NULL) 
				Errors::at_position("no chapter is currently selected",
					template_filename, lpos);
			else @<Substitute a detail about the currently selected Chapter@>;
		} else if (ISORegexp::match_1(varname, "Section (%c+)", found_text1)) {
			string detail; CStrings::copy(detail, found_text1);
			section *S = Swarm::heading_topmost_on_stack(SECTION_LEVEL);
			if (S == NULL) 
				Errors::at_position("no section is currently selected",
					template_filename, lpos);
			else @<Substitute a detail about the currently selected Section@>;
		} else if (ISORegexp::match_1(varname, "Complete (%c+)", found_text1)) {
			string detail; CStrings::copy(detail, found_text1);
			@<Substitute a detail about the complete PDF@>;
		} else {
			CSTRING_WRITE(substituted, "<b>%s</b>", varname);
		}
		CSTRING_WRITE(tl, "%s%s%s", left_part, substituted, right_part);
	}

@ This is why, for instance, |[[Author]]| is replaced by the author's name:

@<Substitute any bibliographic datum named@> =
	CStrings::copy(substituted, Bibliographic::get_data(W, varname));

@ We store little about the complete-web-in-one-file PDF:

@<Substitute a detail about the complete PDF@> =
	if (swarm_leader)
		if (Formats::substitute_post_processing_data(substituted, swarm_leader, detail) == FALSE)
			CSTRING_WRITE(substituted, "%s for complete web", detail);

@ And here for Chapters:

@<Substitute a detail about the currently selected Chapter@> =
	if (CStrings::eq(detail, "Title")) {
		CStrings::copy(substituted, C->ch_title);
	} else if (CStrings::eq(detail, "Code")) {
		CStrings::copy(substituted, C->ch_sigil);
	} else if (CStrings::eq(detail, "Purpose")) {
		CStrings::copy(substituted, C->rubric);
	} else if (Formats::substitute_post_processing_data(substituted, C->ch_weave, detail)) {
		;
	} else {
		CSTRING_WRITE(substituted, "%s for %s", varname, C->ch_title);
	}

@ And this, finally, is a very similar construction for Sections.

@<Substitute a detail about the currently selected Section@> =
	if (CStrings::eq(detail, "Title")) {
		CStrings::copy(substituted, S->sect_title);
	} else if (CStrings::eq(detail, "Purpose")) {
		CStrings::copy(substituted, S->sect_purpose);
	} else if (CStrings::eq(detail, "Code")) {
		CStrings::copy(substituted, S->sigil);
	} else if (CStrings::eq(detail, "Lines")) {
		CSTRING_WRITE(substituted, "%d", S->sect_extent);
	} else if (CStrings::eq(detail, "Source")) {
		Filenames::to_string(substituted, S->source_file_for_section);
	} else if (CStrings::eq(detail, "Page")) {
		string linkto; CStrings::copy(linkto, S->sigil);
		for (int i=0; linkto[i]; i++)
			if ((linkto[i] == '/') || (linkto[i] == ' '))
				CStrings::set_char(linkto, i, '-');
		CStrings::concatenate(linkto, ".html");
		CStrings::copy(substituted, linkto);
	} else if (CStrings::eq(detail, "Paragraphs")) {
		CSTRING_WRITE(substituted, "%d", S->sect_paragraphs);
	} else if (CStrings::eq(detail, "Mean")) {
		int denom = S->sect_paragraphs;
		if (denom == 0) denom = 1;
		CSTRING_WRITE(substituted, "%d", S->sect_extent/denom);
	} else if (Formats::substitute_post_processing_data(substituted, S->sect_weave, detail)) {
		;
	} else {
		CSTRING_WRITE(substituted, "%s for %s", varname, S->sect_title);
	}
