[Reader::] Reading Sections.

@Purpose: To read the Contents section of the web, and through that each of
the other sections in turn, and to collate all of this material into one
big linear array of source-code lines.

@p Web storage.
There's normally only one web read in during a single run of Inweb, but
this might change if we ever add batch-processing in future. A web is a set
of chapters each of which is a set of sections; webs which don't obviously
divide into chapters will be called "unchaptered", though in fact they do
have a single chapter, called simply "Sections" (and with sigil "S").

The program expressed by a web is output, or "tangled", to a number of
stand-alone files called "tangle targets". By default there is just one
of these.

We read the complete literate source of the web into memory, which is
profligate, but saves time. Most of the lines come straight from the
source files, but a few chapter heading lines are inserted if this is a
multi-chapter web.

@c
typedef struct web {
	struct pathname *path_to_web; /* relative to the current working directory */

	/* convenient statistics */
	int no_lines; /* total lines in literate source, excluding contents */
	int no_paragraphs; /* this will be at least 1 */
	int no_sections; /* again, excluding contents: it will eventually be at least 1 */
	int no_chapters; /* this will be at least 1 */
	int chaptered; /* has the author explicitly divided it into named chapters? */
	int analysed; /* has this been scanned for function usage and such? */

	struct bibliographic_datum *first_bd; /* key-value pairs for title, author and such */
	struct programming_language *main_language; /* in which most of the sections are written */

	struct chapter *first_chapter; /* two ends of a linked list */
	struct chapter *last_chapter;

	struct imported_header *first_header; /* two ends of a linked list */
	struct imported_header *last_header;

	struct c_structure *first_c_structure; /* two ends of a linked list, used only for C */
	struct c_structure *last_c_structure;

	struct tangle_target *first_target; /* linked list of tangle targets */

	struct ebook *as_ebook; /* when being woven to an ebook */
	struct pathname *redirect_weaves_to; /* ditto */
	MEMORY_MANAGEMENT
} web;

@ 

@c
web *Reader::load_web(pathname *P, pathname *I, int verbosely) {
	web *W = CREATE(web);
	W->path_to_web = P;
	W->chaptered = FALSE;
	W->first_chapter = NULL; W->last_chapter = NULL;
	W->first_header = NULL; W->last_header = NULL;
	W->first_c_structure = NULL; W->last_c_structure = NULL;
	W->first_bd = NULL;
	W->first_target = NULL;
	W->no_lines = 0; W->no_sections = 0; W->no_chapters = 0; W->no_paragraphs = 0;
	W->analysed = FALSE;
	W->as_ebook = NULL;
	W->redirect_weaves_to = NULL;
	Bibliographic::initialise_data(W);
	Reader::add_tangle_target(W, Languages::default()); /* the bulk of the web is automatically a target */
	Reader::read_contents_page(W, I, verbosely);
	Parser::parse_literate_source(W);
	return W;
}

@ This really serves no purpose, but seems to boost morale.

@c
void Reader::print_web_statistics(web *W) {
	printf("web \"%s\": ", Bibliographic::get_data(W, "Title"));
	if (W->chaptered) printf("%d chapter(s) : ", W->no_chapters);
	printf("%d section(s) : %d paragraph(s) : %d line(s)\n",
		W->no_sections, W->no_paragraphs, W->no_lines);
}

@p Chapters and sections.
Each web contains a linked list of chapters, in reading order:

@c
typedef struct chapter {
	string ch_sigil; /* e.g., |P| for Preliminaries, |7| for Chapter 7, |C| for Appendix C */
	string ch_title; /* e.g., "Chapter 3: Fresh Water Fish" */
	string rubric; /* optional; without double-quotation marks */

	string woven_pdf_leafname; /* a leafname for a standalone weave of this chapter */
	struct tangle_target *ch_target; /* |NULL| unless this chapter produces a tangle of its own */
	struct weave_target *ch_weave; /* |NULL| unless this chapter produces a weave of its own */
	struct programming_language *ch_language; /* in which most of the sections are written */

	int titling_line_inserted; /* has an interleaved chapter heading been added yet? */

	int ch_extent; /* total number of lines in the sections of this chapter */
	struct section *first_section; /* two ends of a linked list */
	struct section *last_section;

	struct web *owning_web;
	int imported; /* from a different web? */
	struct chapter *next_chapter; /* within the owning web's linked list */
	MEMORY_MANAGEMENT
} chapter;

@ Each chapter contains a linked list of sections, in reading order:

@c
typedef struct section {
	string sigil; /* e.g., "9/tfto" */
	string sect_title; /* e.g., "Program Control" */
	string sect_namespace; /* e.g., "Text::Languages::" */
	string sect_purpose; /* e.g., "To manage the zoo, and feed all penguins" */
	int barred; /* contains a dividing bar? */

	struct filename *source_file_for_section;

	struct tangle_target *sect_target; /* |NULL| unless this section produces a tangle of its own */
	struct weave_target *sect_weave; /* |NULL| unless this section produces a weave of its own */
	struct programming_language *sect_language; /* in which this section is written */

	int sect_extent; /* total number of lines in this section */
	struct source_line *first_line; /* two ends of a linked list */
	struct source_line *last_line;

	int sect_paragraphs; /* total number of paragraphs in this section */
	struct paragraph *first_paragraph; /* two ends of a linked list */
	struct paragraph *last_paragraph;
	struct theme_tag *tag_with; /* automatically tag paras in this section thus */

	struct cweb_macro *first_macro; /* two ends of a linked list */
	struct cweb_macro *last_macro;

	struct chapter *owning_chapter;
	struct section *next_section; /* within the owning chapter's linked list */
	
	int scratch_flag; /* temporary workspace */
	int erroneous_interface; /* problem with Interface declarations */
	MEMORY_MANAGEMENT
} section;

@p Reading the contents page.
Making the web begins by reading the contents section, which really isn't a
section at all (and perhaps we shouldn't pretend that it is by the use of the
|.w| file extension, but we probably want it to have the same file extension,
and its syntax is chosen so that syntax-colouring for regular sections doesn't
make it look odd). When the word "section" is used in the Inweb code, it
almost always means "section other than the contents".

We iterate through the contents page line by line, using the following slate
of variables to keep track of where we are. (This is where Inweb's
heritage as a former Perl program shows itself; in Perl these variables
would have had a scope extending to all nested subroutines.)

@c
typedef struct reader_state {
	struct web *current_web;
	int in_biblio;
	int in_purpose; /* Reading the bit just after the new chapter? */
	struct chapter *chapter_being_scanned;
	string chapter_folder_name; /* Where sections in the current chapter live */
	string titling_line_to_insert; /* To be inserted automagically */
	struct pathname *path_to; /* Where web material is being read from */
	struct pathname *import_from; /* Where imported webs are */
	int scan_verbosely;
	int in_original; /* Reading the original web, or an included one? */
} reader_state;

@ The iteration begins:

@c
void Reader::read_contents_page(web *W, pathname *import_path, int verbosely) {
	Reader::read_contents_page_from(W, import_path, verbosely, NULL);
	Bibliographic::check_required_data(W);
}

void Reader::read_contents_page_from(web *W, pathname *import_path, int verbosely, pathname *path) {
	reader_state RS;
	RS.current_web = W;
	RS.in_biblio = TRUE;
	RS.in_purpose = FALSE;
	RS.chapter_being_scanned = NULL;
	CStrings::copy(RS.chapter_folder_name, "");
	CStrings::copy(RS.titling_line_to_insert, "");
	RS.scan_verbosely = verbosely;
	RS.path_to = path;
	RS.import_from = import_path;
	RS.in_original = FALSE;

	if (path == NULL) {
		path = W->path_to_web;
		RS.in_original = TRUE;
	}
	
	filename *Contents = Filenames::in_folder(path, "Contents.w");
	
	int cl = TextFiles::read_with_lines_to_ISO(Contents, "can't open contents file",
		TRUE, Reader::scan_biblio_line, NULL, &RS);
	if (verbosely) printf("Read contents section: '%s' (%d lines)\n", "Contents.w", cl);
}

@ The contents section has a syntax quite different from all other sections,
and sets out bibliographic information about the web, the sections and their
organisation, and so on.

@c
void Reader::scan_biblio_line(char *line, text_file_position *tfp, void *X) {
	reader_state *RS = (reader_state *) X;
	int begins_with_white_space = FALSE;
	while (ISORegexp::white_space(*line)) { begins_with_white_space = TRUE; line++; }
	int l = CStrings::len(line) - 1;
	while ((l>=0) && (ISORegexp::white_space(line[l]))) CStrings::truncate(line, l--);

	if (line[0] == 0) @<End bibliographic data here, at the blank line@>
	else if (RS->in_biblio) @<Read the bibliographic data block at the top@>
	else @<Read the roster of sections at the bottom@>;
}

@ At this point we've gone through the bibliographic lines at the top of the
contents page, and are soon going to read in the sections. The language needs
to be known for that, so we'll set it now.

@<End bibliographic data here, at the blank line@> =
	programming_language *pl =
		Languages::language_with_name(Bibliographic::get_data(RS->current_web, "Language"));
	RS->current_web->main_language = pl;
	RS->current_web->first_target->tangle_language = pl;
	RS->in_biblio = FALSE;

@ The bibliographic data gives lines in any order specifying values of
variables with fixed names; a blank line ends the block.

@<Read the bibliographic data block at the top@> =
	if (RS->in_original) {
		string found_text1;
		string found_text2;	
		if (ISORegexp::match_2(line, "(%c+?): (%c+?) *", found_text1, found_text2)) {
			string key; CStrings::copy(key, found_text1);
			string value; CStrings::copy(value, found_text2);
			@<Set bibliographic key-value pair@>;
		} else {
			string err; CSTRING_WRITE(err, "expected 'Setting: Value' but found '%s'", line);
			Errors::in_text_file(err, tfp);
		}
	}

@

@<Set bibliographic key-value pair@> =
	if (CStrings::eq(key, "Weave")) {
		string found_text1;
		string found_text2;	
		string found_text3;
		string found_text4;
		if (ISORegexp::match_4(value, "(%c+?): (%c+?), (%c+?), (%c+?)",
				found_text1, found_text2, found_text3, found_text4)) {
			Parser::declare_tag_from_contents(RS->current_web,
				found_text1, found_text2, found_text3, found_text4);
		} else {
			string err; CSTRING_WRITE(err,
				"expected 'Weave: Tag: title, leafname, cover' but found '%s'", line);
			Errors::in_text_file(err, tfp);
		}
	} else if (Bibliographic::datum_can_be_declared(RS->current_web, key)) {
		if (Bibliographic::datum_on_or_off(RS->current_web, key)) {
			if ((CStrings::ne(value, "On")) && (CStrings::ne(value, "Off"))) {
				string err;
				CSTRING_WRITE(err, "this setting must be 'On' or 'Off': %s", key);
				Errors::in_text_file(err, tfp);
				CStrings::copy(value, "Off");
			}
		}
		Bibliographic::set_datum(RS->current_web, key, value);
	} else {
		string err; CSTRING_WRITE(err, "no such bibliographic datum: %s", key);
		Errors::in_text_file(err, tfp);
	}

@ In the bulk of the contents, we find indented lines for sections and
unindented ones for chapters.

@<Read the roster of sections at the bottom@> =
	if (begins_with_white_space == FALSE) {
		if (*line == '"') { RS->in_purpose = TRUE; line++; }
		if (RS->in_purpose == TRUE) @<Record the purpose of the current chapter@>
		else @<Read about a new chapter@>;
	} else @<Read about, and read in, a new section@>;

@ After a declared chapter heading, subsequent lines form its purpose, until
we reach a closed quote: we then stop, but remove the quotation marks. Because
we like a spoonful of syntactic sugar on our porridge, that's why.

@<Record the purpose of the current chapter@> =
	if ((CStrings::len(line) > 0) && (line[CStrings::len(line)-1] == '"')) {
		CStrings::truncate(line, CStrings::len(line)-1); RS->in_purpose = FALSE;
	}
	if (RS->chapter_being_scanned) {
		char *r = RS->chapter_being_scanned->rubric;
		if (r[0]) CStrings::concatenate(r, " ");
		CStrings::concatenate(r, line);
	}

@ The title tells us everything we need to know about a chapter:

@<Read about a new chapter@> =
	string new_chapter_sigil; CStrings::copy(new_chapter_sigil, ""); /* e.g., P, 1, 2, 3, A, B, ... */
	string pdf_leafname; CStrings::copy(pdf_leafname, "");
	tangle_target *ind_target = RS->current_web->first_target;
	programming_language *ind_language = RS->current_web->main_language;
	
	string found_text1;
	string found_text2;	
	if (ISORegexp::match_2(line, "(%c*%C) %(Independent(%c*)%)", found_text1, found_text2)) {
		string title_alone; CStrings::copy(title_alone, found_text1);
		string language_name; CStrings::copy(language_name, found_text2);		
		@<Mark this chapter as an independent tangle target@>;
		CStrings::copy(line, title_alone);
	}

	int this_is_a_chapter = TRUE;
	if (CStrings::eq(line, "Sections")) {
		CStrings::copy(new_chapter_sigil, "S");
		CSTRING_WRITE(RS->chapter_folder_name, "Sections");
		CStrings::copy(RS->titling_line_to_insert, "");
		CStrings::copy(pdf_leafname, "Sections.pdf");
		RS->current_web->chaptered = FALSE;
	} else if (CStrings::eq(line, "Preliminaries")) {
		CStrings::copy(new_chapter_sigil, "P");
		CSTRING_WRITE(RS->chapter_folder_name, "Preliminaries");
		CStrings::copy(RS->titling_line_to_insert, "");
		CStrings::copy(pdf_leafname, "Preliminaries.pdf");
		RS->current_web->chaptered = TRUE;
	} else if (ISORegexp::match_1(line, "Header: (%c+)", found_text1)) {
		pathname *P = RS->path_to;
		if (P == NULL) P = RS->current_web->path_to_web;
		P = Pathnames::subfolder(P, "Headers");
		filename *HF = Filenames::in_folder(P, found_text1);
		Reader::add_imported_header(RS->current_web, HF);
		this_is_a_chapter = FALSE;
	} else if (ISORegexp::match_1(line, "Import: (%c+)", found_text1)) {
		pathname *imported = Pathnames::from_string_relative(RS->import_from, found_text1);
		Reader::read_contents_page_from(RS->current_web, RS->import_from, RS->scan_verbosely, imported);
		this_is_a_chapter = FALSE;
	} else if (ISORegexp::match_1(line, "Chapter (%d+): %c+", found_text1)) {
		int n = atoi(found_text1);
		CSTRING_WRITE(new_chapter_sigil, "%d", n);
		CSTRING_WRITE(RS->chapter_folder_name, "Chapter %d", n);
		CSTRING_WRITE(RS->titling_line_to_insert, "%s.", line);
		CSTRING_WRITE(pdf_leafname, "Chapter-%d.pdf", n);
		RS->current_web->chaptered = TRUE;
	} else if (ISORegexp::match_1(line, "Appendix (%c): %c+", found_text1)) {
		string letter; CStrings::copy(letter, ""); 
		CStrings::copy(letter, found_text1);
		CStrings::copy(new_chapter_sigil, letter);
		CSTRING_WRITE(RS->chapter_folder_name, "Appendix %s", letter);
		CSTRING_WRITE(RS->titling_line_to_insert, "%s.", line);
		CSTRING_WRITE(pdf_leafname, "Appendix-%s.pdf", letter);
		RS->current_web->chaptered = TRUE;
	} else {
		string err; CSTRING_WRITE(err, "segment not understood: %s", line);
		Errors::in_text_file(err, tfp);
		fprintf(stderr, "(Must be 'Chapter <number>: Title', "
			"'Appendix <letter A to O>: Title',\n");
		fprintf(stderr, "'Preliminaries' or 'Sections')\n");
	}
	
	if (this_is_a_chapter) @<Create the new chapter with these details@>;

@ A chapter whose title marks it as Independent becomes a new tangle target,
with the same language as the main web unless stated otherwise.

@<Mark this chapter as an independent tangle target@> =
	char *p = language_name;
	while (ISORegexp::white_space(*p)) p++;
	if (*p == 0) p = Bibliographic::get_data(RS->current_web, "Language");
	ind_language = Languages::language_with_name(p);
	ind_target = Reader::add_tangle_target(RS->current_web, ind_language);

@

@<Create the new chapter with these details@> =
	chapter *C = CREATE(chapter);
	CStrings::copy(C->ch_sigil, new_chapter_sigil);
	CStrings::copy(C->ch_title, line);
	CStrings::copy(C->rubric, "");
	C->ch_target = ind_target;
	C->ch_weave = NULL;
	C->ch_language = ind_language;
	C->ch_extent = 0;
	C->titling_line_inserted = FALSE;
	C->owning_web = RS->current_web;
	C->next_chapter = NULL;
	C->first_section = NULL; C->last_section = NULL;
	C->imported = TRUE;
	if (RS->in_original) C->imported = FALSE;
	CStrings::copy(C->woven_pdf_leafname, pdf_leafname);
	
	if (RS->current_web->first_chapter == NULL) {
		RS->current_web->first_chapter = C;
		RS->current_web->last_chapter = C;
	} else {
		RS->current_web->last_chapter->next_chapter = C;
		RS->current_web->last_chapter = C;
	}
	C->owning_web->no_chapters++;
	RS->chapter_being_scanned = C;

@ That's enough on creating chapters. This is the more interesting business
of registering a new section within a chapter -- more interesting because
we also read in and process its file.

@<Read about, and read in, a new section@> =
	section *sect = CREATE(section);
	sect->owning_chapter = RS->chapter_being_scanned;
	sect->owning_chapter->owning_web->no_sections++;
	sect->next_section = NULL;

	sect->sect_extent = 0;
	sect->first_line = NULL; sect->last_line = NULL;
	sect->sect_paragraphs = 0;
	sect->first_paragraph = NULL; sect->last_paragraph = NULL;

	sect->scratch_flag = FALSE;
	sect->erroneous_interface = FALSE;
	sect->barred = FALSE;
	
	if (RS->chapter_being_scanned->first_section == NULL)
		RS->chapter_being_scanned->first_section = sect;
	else
		RS->chapter_being_scanned->last_section->next_section = sect;
	RS->chapter_being_scanned->last_section = sect;

	@<Work out the language and tangle target for the section@>;
	sect->sect_weave = NULL;

	string found_text1;
	string found_text2;	
	if (ISORegexp::match_2(line, "(%c+) %[%[Tag: (%c+)%]%] *", found_text1, found_text2)) {
		CStrings::copy(sect->sect_title, found_text1);
		sect->tag_with = Parser::add_tag_by_name(NULL, found_text2);
	} else {
		sect->tag_with = NULL;
		CStrings::copy(sect->sect_title, line);
	}

	CStrings::copy(sect->sect_namespace, "");

	string leafname_to_use;
	CSTRING_WRITE(leafname_to_use,
		"%s%s", sect->sect_title, sect->sect_language->source_file_extension);

	char *templ = strstr(leafname_to_use, " Template.i6t");
	if (templ) CStrings::copy(templ, ".i6t");

	pathname *P = RS->path_to;
	if (P == NULL) P = RS->current_web->path_to_web;
	if (RS->chapter_folder_name[0]) P = Pathnames::subfolder(P, RS->chapter_folder_name);
	sect->source_file_for_section = Filenames::in_folder(P, leafname_to_use);

	Reader::read_file(RS->current_web, sect->source_file_for_section,
		RS->titling_line_to_insert, sect, RS->scan_verbosely);

@ Just as for chapters, but a section which is an independent target with
language "Inform 6" is given the filename extension |.i6t| instead of |.w|.
This is to conform with the naming convention used within Inform, where
I6 template files -- Inweb files with language Inform 6 -- are given the
file extensions |.i6t|.

@<Work out the language and tangle target for the section@> =
	sect->sect_language = RS->chapter_being_scanned->ch_language; /* by default */
	string found_text1;
	string found_text2;	
	if (ISORegexp::match_2(line, "(%c*%C) %(Independent(%c*)%)", found_text1, found_text2)) {
		string title_alone; CStrings::copy(title_alone, found_text1);
		string language_name; CStrings::copy(language_name, found_text2);
		@<Mark this section as an independent tangle target@>;
		CStrings::copy(line, title_alone);
	} else {
		sect->sect_target = RS->chapter_being_scanned->ch_target;
	}

@

@<Mark this section as an independent tangle target@> =
	char *p = language_name;
	while (ISORegexp::white_space(*p)) p++;
	if (*p == 0) p = Bibliographic::get_data(RS->current_web, "Language");
	programming_language *pl = Languages::language_with_name(p);
	sect->sect_language = pl;
	sect->sect_target = Reader::add_tangle_target(RS->current_web, pl);

@p Reading source files.
Note that we assume here that trailing whitespace on a line (up to but not
including the line break) is not significant in the language being tangled for.

@c
void Reader::read_file(web *W, filename *OUT, char *titling_line, section *sect,
	int verbosely) {
	section *current_section = sect;

	if ((titling_line) && (titling_line[0]) &&
		(sect->owning_chapter->titling_line_inserted == FALSE)) {
		sect->owning_chapter->titling_line_inserted = TRUE;
		string line; CStrings::copy(line, "");
		text_file_position *tfp = NULL;
		CSTRING_WRITE(line, "Chapter Heading");
		@<Accept this as a line belonging to this section and chapter@>;
	}

	int cl = TextFiles::read_with_lines_to_ISO(OUT, "can't open section file", TRUE,
		Reader::scan_source_line, NULL, (void *) current_section);
	if (verbosely)
		printf("Read section: '%s' (%d lines)\n", sect->sect_title, cl);
}

@ Which results in the following being called on each line of the file:

@c
void Reader::scan_source_line(char *line, text_file_position *tfp, void *state) {
	section *current_section = (section *) state;
	int l = CStrings::len(line) - 1;
	while ((l>=0) && (ISORegexp::white_space(line[l]))) CStrings::truncate(line, l--);
	
	@<Accept this as a line belonging to this section and chapter@>;
}

@

@<Accept this as a line belonging to this section and chapter@> =
	source_line *sl = Lines::new_source_line(line, tfp);

	/* enter this in its section's linked list of lines: */
	sl->owning_section = current_section;
	if (current_section->first_line == NULL) current_section->first_line = sl;
	else current_section->last_line->next_line = sl;
	current_section->last_line = sl;

	/* we haven't detected paragraph boundaries yet, so: */
	sl->owning_paragraph = NULL;

	/* and keep count: */
	sl->owning_section->sect_extent++;
	sl->owning_section->owning_chapter->ch_extent++;
	sl->owning_section->owning_chapter->owning_web->no_lines++;

@p Scanning webs.
This scanner is intended for debugging Inweb, and simply shows the main
result of reading in and parsing the web:

@c
void Reader::scan_line_categories(web *W, char *sigil) {
	printf("Scan of source lines for '%s'\n", sigil);
	int count = 1;
	chapter *C = Reader::get_chapter_for_sigil(W, sigil);
	if (C) {
		for (section *S = C->first_section; S; S = S->next_section)
			for (source_line *L = S->first_line; L; L = L->next_line)
				@<Trace the content and category of this source line@>;
	} else {
		section *S = Reader::get_section_for_sigil(W, sigil);
		if (S) {
			for (source_line *L = S->first_line; L; L = L->next_line)
				@<Trace the content and category of this source line@>
		} else {
			for (chapter *C = W->first_chapter; C; C = C->next_chapter)
				for (section *S = C->first_section; S; S = S->next_section)
					for (source_line *L = S->first_line; L; L = L->next_line)
						@<Trace the content and category of this source line@>;
		}
	}
}

@

@<Trace the content and category of this source line@> =
	printf("%07d  %16s  %s\n", count++, Lines::category_name(L->category), L->text);

@p Looking up chapters and sections.

@c
chapter *Reader::get_chapter_for_sigil(web *W, char *sigil) {
	if (W)
		for (chapter *C = W->first_chapter; C; C = C->next_chapter)
			if (CStrings::eq(C->ch_sigil, sigil))
				return C;
	return NULL;
}

section *Reader::get_section_for_sigil(web *W, char *sigil) {
	if (W)
		for (chapter *C = W->first_chapter; C; C = C->next_chapter)
			for (section *S = C->first_section; S; S = S->next_section)
				if (CStrings::eq(S->sigil, sigil))
					return S;
	return NULL;
}

section *Reader::section_by_filename(web *W, text_stream *filename) {
	if (W)
		for (chapter *C = W->first_chapter; C; C = C->next_chapter)
			for (section *S = C->first_section; S; S = S->next_section) {
				TEMPORARY_TEXT(SFN);
				WRITE_TO(SFN, "%f", S->source_file_for_section);
				int rv = Str::eq(SFN, filename);
				DISCARD_TEXT(SFN);
				if (rv) return S;
			}
	return NULL;
}

@p Sigils and containment.
This provides a sort of $\subseteq$ relation on sigils, testing if the portion
of the web represented by |sig1| is contained inside the portion represented
by |sig2|.

@c
int Reader::sigil_within(char *sig1, char *sig2) {
	if (CStrings::eq(sig2, "0")) return TRUE;
	if (CStrings::eq(sig1, sig2)) return TRUE;
	string found_text1;	
	if (ISORegexp::match_0(sig2, "%c+/%c+")) return FALSE;
	if (ISORegexp::match_1(sig1, "(%c+)/%c+", found_text1)) {
		if (CStrings::eq(found_text1, sig2)) return TRUE;
	} 
	return FALSE;
}

@p Tangle targets.
In Knuth's original conception of literate programming, a web produces
just one piece of tangled output -- the program for compilation. But this
assumes that the underlying program is so simple that it won't require
ancillary files, configuration data, and such; and this is often just as
complex and worth explaining as the program itself. So Inweb allows a
web to contain multiple tangle targets, each of which contains a union of
sections. Each section belongs to exactly one tangle target; by default
a web contains just one target, which contains all of the sections.

@c
typedef struct tangle_target {
	struct programming_language *tangle_language; /* common to the entire contents */
	struct hash_table symbols; /* a table of identifiable names in this program */
	struct tangle_target *next_target; /* within the web's linked list */
	MEMORY_MANAGEMENT
} tangle_target;

@

@c
tangle_target *Reader::add_tangle_target(web *W, programming_language *language) {
	tangle_target *tt = CREATE(tangle_target);
	tt->tangle_language = language;
	tt->next_target = NULL;
	if (W->first_target == NULL) W->first_target = tt;
	else {
		tangle_target *tto = W->first_target;
		while ((tto) && (tto->next_target)) tto = tto->next_target;
		tto->next_target = tt;
	}
	return tt;
}

@ And the following provides a way to iterate through the lines in a tangle,
while keeping the variables |C|, |S| and |L| pointing to the current chapter,
section and line.

@d LOOP_WITHIN_TANGLE(T)
	for (chapter *C = W->first_chapter; C; C = C->next_chapter)
		for (section *S = C->first_section; S; S = S->next_section)
			if (S->sect_target == T)
				for (source_line *L = S->first_line; L; L = L->next_line)

@ Similarly but more simply:

@c
typedef struct imported_header {
	struct filename *header_file;
	struct imported_header *next_header;
	MEMORY_MANAGEMENT
} imported_header;

@

@c
void Reader::add_imported_header(web *W, filename *HF) {
	imported_header *H = CREATE(imported_header);
	H->header_file = HF;
	if (W->first_header == NULL) {
		W->first_header = H;
		W->last_header = H;
	} else {
		W->last_header->next_header = H;
		W->last_header = H;
	}
	H->next_header = NULL;
}
