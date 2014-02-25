4/swarm: The Swarm.

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

void weave_swarm(web *W, char *subweb, int swarm_mode, theme_tag *tag, char *format) {
	swarm_leader = NULL;

	if ((swarm_mode == SWARM_CHAPTERS) || (swarm_mode == SWARM_SECTIONS)) {
		/* weave Complete web */
		if (in_string_eq(subweb, "0"))
			swarm_leader = weave_subset_of_web(W, "0", FALSE, tag, format);

		for (chapter *C = W->first_chapter; C; C = C->next_chapter) {
			/* weave single chapter */
			if ((W->chaptered == TRUE) && (sigil_within(C->ch_sigil, subweb))) {
				C->ch_weave = weave_subset_of_web(W, C->ch_sigil, FALSE, tag, format);
				if (in_string_ne(subweb, "")) swarm_leader = C->ch_weave;
			}

			/* weave individual sections */
			for (section *S = C->first_section; S; S = S->next_section)
				if (sigil_within(S->sigil, subweb))
					S->sect_weave = weave_subset_of_web(W, S->sigil, FALSE, tag, format);
		}
	}

	weave_index_templates(W, subweb, format);
}

@ The following is where an individual weave task begins, whether it comes
from the swarm, or has been specified at the command line (in which case
the call comes from Program Control).

@c
weave_target *weave_subset_of_web(web *W, char *subweb, int open_afterwards, theme_tag *tag, char *format) {
	weave_target *wv = NULL;
	if (no_inweb_errors == 0) {
		analyse_code(W);
		@<Compile a set of instructions for the weaver@>;
		if (weave_source(W, wv) == 0) /* i.e., the number of lines woven was zero */
			fatal_error("empty weave request");
		post_process_weave(wv, open_afterwards); /* e.g., run through |TeX| */
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
	string weave_to; /* where to put it */
	struct weave_format *format; /* plain text, say, or HTML */
	string cover_sheet_to_use; /* leafname of the copy, or |""| for no cover */
	void *post_processing_results; /* optional typesetting diagnostics after running through */	
	MEMORY_MANAGEMENT
} weave_target;

@

@<Compile a set of instructions for the weaver@> =
	wv = CREATE(weave_target);
	wv->weave_web = W;
	in_strcpy(wv->weave_sigil, subweb);
	wv->theme_match = tag;
	in_strcpy(wv->booklet_title, "");
	wv->format = parse_format(format);
	wv->post_processing_results = NULL;
	if ((tag) && (tag->cover_sheet_when_woven[0]))
		in_sprintf(wv->cover_sheet_to_use, "%sMaterials%c%s",
			W->path_to_web, SEP_CHAR, wv->theme_match->cover_sheet_when_woven);
	else
		in_sprintf(wv->cover_sheet_to_use, "%scover-sheet", path_to_inweb_materials);
	
	string path_to_loom; in_sprintf(path_to_loom, "%sWoven%c", W->path_to_web, SEP_CHAR);

	string leafname; in_strcpy(leafname, "");
	@<Translate the subweb sigil into details of what to weave@>;
	in_sprintf(wv->weave_to, "%s%s", path_to_loom, leafname);

@ From the sigil and the theme, we work out the weave title, the leafname,
and details of any cover-sheet to use.

@<Translate the subweb sigil into details of what to weave@> =
	if (in_string_eq(subweb, "0")) {
		in_strcpy(wv->booklet_title, "Complete Program");
		in_strcpy(leafname, "Complete");
		if ((wv->theme_match) && (wv->theme_match->title_when_woven[0]))
			in_strcpy(wv->booklet_title, wv->theme_match->title_when_woven);
		if ((wv->theme_match) && (wv->theme_match->leafname_when_woven[0]))
			in_sprintf(leafname, "%s", wv->theme_match->leafname_when_woven);
	} else if (pattern_match(subweb, "%d+")) {
		in_sprintf(wv->booklet_title, "Chapter %s", subweb);
		in_strcpy(leafname, wv->booklet_title);
	} else if (pattern_match(subweb, "%[A-O]")) {
		in_sprintf(wv->booklet_title, "Appendix %s", subweb);
		in_strcpy(leafname, wv->booklet_title);
	} else if (in_string_eq(subweb, "P")) {
		in_strcpy(wv->booklet_title, "Preliminaries");
		in_strcpy(leafname, wv->booklet_title);
	} else {
		in_sprintf(wv->booklet_title, "%s", subweb);
		in_strcpy(leafname, wv->booklet_title);
		in_strcpy(wv->cover_sheet_to_use, "");
	}
	for (int i=0; leafname[i]; i++)
		if ((leafname[i] == '/') || (leafname[i] == ' '))
			in_set(leafname, i, '-');
	in_strcat(leafname, weave_file_extension(wv->format));

@ Each weave results in a compressed one-line printed report:

@<Report on the outcome of the weave to the console@> =
	printf("[%s: %s", wv->booklet_title, wv->format->format_name);
	report_on_post_processing(wv);
	printf("]\n");

@ After every swarm, we rebuild all the indexes specified in the Index
Template list for the web:

@c
void weave_index_templates(web *W, char *subweb, char *format) {
	@<Weave one or more index files@>;
	@<Copy in one or more additional files to accompany the index@>;
}

@

@<Weave one or more index files@> =
	string temp_list; in_strcpy(temp_list, "");
	string leaf; in_strcpy(leaf, "");
	if (bibliographic_data_exists(W, "Index Template"))
		in_strcpy(temp_list, get_bibliographic_data(W, "Index Template"));
	else {
		in_sprintf(temp_list, "%s", path_to_inweb_materials);
		if (index_pdfs(format)) {
			if (W->chaptered) in_sprcat(temp_list, "chaptered-tex-index.html");
			else in_sprcat(temp_list, "unchaptered-tex-index.html");
		} else {
			if (W->chaptered) in_sprcat(temp_list, "chaptered-index.html");
			else in_sprcat(temp_list, "unchaptered-index.html");
		}
		in_strcpy(leaf, "index.html");
	}
	while (temp_list[0]) {
		string index_to_make; in_strcpy(index_to_make, "");
		if (pattern_match(temp_list, "(%c+?), (%c+)")) {
			in_strcpy(index_to_make, found_text1);
			in_strcpy(temp_list, found_text2);
		} else {
			in_strcpy(index_to_make, temp_list);
			in_strcpy(temp_list, "");
		}
		if (leaf[0] == 0) {
			if (pattern_match(index_to_make, "%c+/(%c+?)"))
				in_strcpy(leaf, found_text1);
			else
				in_strcpy(leaf, index_to_make);
		}
		printf("Weaving index file: Woven/%s\n", leaf);
		run_contents_interpreter(W, subweb, index_to_make, leaf);
		in_strcpy(leaf, "");
	}

@ The idea here is that an HTML index may need some binary image files to
go with it, for instance.

@<Copy in one or more additional files to accompany the index@> =
	string copy_list; in_strcpy(copy_list, "");
	if (bibliographic_data_exists(W, "Index Extras"))
		in_strcpy(copy_list, get_bibliographic_data(W, "Index Extras"));
	else
		in_sprintf(copy_list,
			"%sdownload.gif, %slemons.jpg, %scrumbs.gif, %sinweb.css",
			path_to_inweb_materials,
			path_to_inweb_materials,
			path_to_inweb_materials,
			path_to_inweb_materials);
	copy_files_into_weave(W, copy_list);

@ Where:

@c
void copy_files_into_weave(web *W, char *copy_list) {
	while (in_string_ne(copy_list, "")) {
		string file_to_copy; in_strcpy(file_to_copy, "");
		if (pattern_match(copy_list, "(%c+?), (%c+)")) {
			in_strcpy(file_to_copy, found_text1);
			in_strcpy(copy_list, found_text2);
		} else {
			in_strcpy(file_to_copy, copy_list);
			in_strcpy(copy_list, "");
		}
		printf("Copying additional index file: %s\n", file_to_copy);
		issue_os_command_2("cp '%s' '%sWoven'", file_to_copy, W->path_to_web);
	}
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

void run_contents_interpreter(web *W, char *subset,
	char *path_to_template, char *contents_page_leafname) {
	contents_processor actual_cp; cp = &actual_cp;
	FILE *CONTS = NULL;
	cp->no_tlines = 0;
	cp->restrict_to_subweb = subset;
	@<Read in the source file containing the contents page template@>;
	@<Open the contents page file to be constructed@>;

	int lpos = 0; /* This is our program counter: a line number in the template */
	cp->stack_pointer = 0; /* And this is our stack pointer for tracking of loops */
	while (lpos < cp->no_tlines) {
		string tl;
		in_strcpy(tl, cp->tlines[lpos++]); /* Fetch the line at the program counter and advance */
		if (pattern_match(tl, "(%c*?) ")) in_strcpy(tl, found_text1); /* Strip trailing spaces */
		if (TRACE_CI_EXECUTION) 
			@<Print line and contents of repeat stack@>;
		if ((pattern_match(tl, "%[%[(%c+)%]%]")) || (pattern_match(tl, " %[%[(%c+)%]%]"))) {
			string command; in_strcpy(command, found_text1);
			@<Deal with a Select command@>;
			@<Deal with a Repeat command@>;
			@<Deal with a Repeat End command@>;
		}
		@<Skip line if inside an empty loop@>;
		@<Make substitutions of square-bracketed variables in line@>;
		fprintf(CONTS, "%s\n", tl); /* Copy the now finished line to the output */
		
		CYCLE: ;
	}
	fclose (CONTS);
}

@p File handling.

@<Read in the source file containing the contents page template@> =
	file_read(path_to_template, "can't find contents template", TRUE, save_template_line, NULL, NULL);
	if (TRACE_CI_EXECUTION)
		printf("Read template <%s>: %d line(s)\n",
			path_to_template, cp->no_tlines);

@ With the following iterator:

@c
void save_template_line(char *line, text_file_position *tfp, void *unused_state) {
	if (cp->no_tlines < MAX_TEMPLATE_LINES)
		cp->tlines[cp->no_tlines++] = new_string(line);
}

@

@<Open the contents page file to be constructed@> =
	string path_to_contents;
	in_sprintf(path_to_contents, "%sWoven%c%s", W->path_to_web, SEP_CHAR,
		contents_page_leafname);
	CONTS = fopen(path_to_contents, "w");
	if (CONTS == NULL) {
		fprintf(stderr, "inweb: warning: unable to generate index because can't "
			"open to write %s\n", path_to_contents);
		return;
	}

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
	if (pattern_match(command, "Select (%c*)")) {
		string sigil; in_strcpy(sigil, found_text1);
		section *S;
		LOOP_OVER(S, section)
			if (in_string_eq(S->sigil, sigil)) {
				start_CI_loop(SECTION_LEVEL, S, S, lpos);
				goto CYCLE;
			}
		chapter *C;
		LOOP_OVER(C, chapter)
			if (in_string_eq(C->ch_sigil, sigil)) {
				start_CI_loop(CHAPTER_LEVEL, C, C, lpos);
				goto CYCLE;
			}
		error_at_position("don't recognise the chapter or section abbreviation sigil",
			path_to_template, lpos);			
		goto CYCLE;
	}

@ Next, a genuine loop beginning:

@<Deal with a Repeat command@> =
	int loop_level = 0;
	if (pattern_match(command, "Repeat Chapter")) loop_level = CHAPTER_LEVEL;
	if (pattern_match(command, "Repeat Section")) loop_level = SECTION_LEVEL;
	if (loop_level != 0) {
		void *from = NULL, *to = NULL;
		if (loop_level == CHAPTER_LEVEL) {
			from = FIRST_OBJECT(chapter);
			to = LAST_OBJECT(chapter);
			if (in_string_ne(cp->restrict_to_subweb, "0")) {
				chapter *C;
				LOOP_OVER(C, chapter)
					if (in_string_eq(C->ch_sigil, cp->restrict_to_subweb)) {
						from = C; to = C;
						break;
					}
			}
		}
		if (loop_level == SECTION_LEVEL) {
			chapter *within_chapter = heading_topmost_on_stack(CHAPTER_LEVEL);
			if (within_chapter == NULL) {
				from = FIRST_OBJECT(section);
				to = LAST_OBJECT(section);
			} else {
				from = within_chapter->first_section;
				to = within_chapter->last_section;
			}
		}
		if (from) start_CI_loop(loop_level, from, to, lpos);
		goto CYCLE;
	}

@ And at the other bookend:

@<Deal with a Repeat End command@> =
	if ((pattern_match(command, "End Repeat")) || (pattern_match(command, "End Select"))) {
		if (cp->stack_pointer <= 0)
			error_at_position("stack underflow on contents template", path_to_template, lpos);
		if (cp->repeat_stack_level[cp->stack_pointer-1] == SECTION_LEVEL) {
			section *S = cp->repeat_stack_variable[cp->stack_pointer-1];
			if (S == cp->repeat_stack_threshold[cp->stack_pointer-1])
				end_CI_loop();
			else {
				cp->repeat_stack_variable[cp->stack_pointer-1] = S->next_section;
				lpos = cp->repeat_stack_startpos[cp->stack_pointer-1]; /* Back round loop */
			}
		} else {
			chapter *C = cp->repeat_stack_variable[cp->stack_pointer-1];
			if (C == cp->repeat_stack_threshold[cp->stack_pointer-1])
				end_CI_loop();
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
void *heading_topmost_on_stack(int level) {
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
void start_CI_loop(int level, void *from, void *to, int pos) {
	if (cp->stack_pointer < CI_STACK_CAPACITY) {
		cp->repeat_stack_level[cp->stack_pointer] = level;
		cp->repeat_stack_variable[cp->stack_pointer] = from;
		cp->repeat_stack_threshold[cp->stack_pointer] = to;
		cp->repeat_stack_startpos[cp->stack_pointer++] = pos;
	}
}

void end_CI_loop(void) {
	cp->stack_pointer--;
}

@p Variable substitutions.
We can now forget about this tiny stack machine: the one task left is to
take a line from the template, and make substitutions of variables into
its square-bracketed parts.

@<Make substitutions of square-bracketed variables in line@> =
	int slen, spos;
	while ((spos = find_expansion(tl, '[', '[', ']', ']', &slen)) >= 0) {
		string left_part; in_strcpy(left_part, tl); in_truncate(left_part, spos);
		string varname; in_strcpy(varname, tl+spos+2); in_truncate(varname, slen-4);
		string substituted; in_strcpy(substituted, varname);
		string right_part; in_strcpy(right_part, tl+spos+slen);
		if (bibliographic_data_exists(W, varname)) {
			@<Substitute any bibliographic datum named@>;
		} else if (pattern_match(varname, "Chapter (%c+)")) {
			string detail; in_strcpy(detail, found_text1);
			chapter *C = heading_topmost_on_stack(CHAPTER_LEVEL);
			if (C == NULL) 
				error_at_position("no chapter is currently selected",
					path_to_template, lpos);
			else @<Substitute a detail about the currently selected Chapter@>;
		} else if (pattern_match(varname, "Section (%c+)")) {
			string detail; in_strcpy(detail, found_text1);
			section *S = heading_topmost_on_stack(SECTION_LEVEL);
			if (S == NULL) 
				error_at_position("no section is currently selected",
					path_to_template, lpos);
			else @<Substitute a detail about the currently selected Section@>;
		} else if (pattern_match(varname, "Complete (%c+)")) {
			string detail; in_strcpy(detail, found_text1);
			@<Substitute a detail about the complete PDF@>;
		} else {
			in_sprintf(substituted, "<b>%s</b>", varname);
		}
		in_sprintf(tl, "%s%s%s", left_part, substituted, right_part);
	}

@ This is why, for instance, |[[Author]]| is replaced by the author's name:

@<Substitute any bibliographic datum named@> =
	in_strcpy(substituted, get_bibliographic_data(W, varname));

@ We store little about the complete-web-in-one-file PDF:

@<Substitute a detail about the complete PDF@> =
	if (swarm_leader)
		if (substitute_post_processing_data(substituted, swarm_leader, detail) == FALSE)
			in_sprintf(substituted, "%s for complete web", detail);

@ And here for Chapters:

@<Substitute a detail about the currently selected Chapter@> =
	if (in_string_eq(detail, "Title")) {
		in_strcpy(substituted, C->ch_title);
	} else if (in_string_eq(detail, "Code")) {
		in_strcpy(substituted, C->ch_sigil);
	} else if (in_string_eq(detail, "Purpose")) {
		in_strcpy(substituted, C->rubric);
	} else if (substitute_post_processing_data(substituted, C->ch_weave, detail)) {
		;
	} else {
		in_sprintf(substituted, "%s for %s", varname, C->ch_title);
	}

@ And this, finally, is a very similar construction for Sections.

@<Substitute a detail about the currently selected Section@> =
	if (in_string_eq(detail, "Title")) {
		in_strcpy(substituted, S->sect_title);
	} else if (in_string_eq(detail, "Purpose")) {
		in_strcpy(substituted, S->sect_purpose);
	} else if (in_string_eq(detail, "Code")) {
		in_strcpy(substituted, S->sigil);
	} else if (in_string_eq(detail, "Lines")) {
		in_sprintf(substituted, "%d", S->sect_extent);
	} else if (in_string_eq(detail, "Source")) {
		in_strcpy(substituted, S->pathname_relative_to_web);
	} else if (in_string_eq(detail, "Page")) {
		string linkto; in_strcpy(linkto, S->sigil);
		for (int i=0; linkto[i]; i++)
			if ((linkto[i] == '/') || (linkto[i] == ' '))
				in_set(linkto, i, '-');
		in_strcat(linkto, ".html");
		in_strcpy(substituted, linkto);
	} else if (in_string_eq(detail, "Paragraphs")) {
		in_sprintf(substituted, "%d", S->sect_paragraphs);
	} else if (in_string_eq(detail, "Mean")) {
		int denom = S->sect_paragraphs;
		if (denom == 0) denom = 1;
		in_sprintf(substituted, "%d", S->sect_extent/denom);
	} else if (substitute_post_processing_data(substituted, S->sect_weave, detail)) {
		;
	} else {
		in_sprintf(substituted, "%s for %s", varname, S->sect_title);
	}
