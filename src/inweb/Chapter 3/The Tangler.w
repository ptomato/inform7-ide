[Tangler::] The Tangler.

@Purpose: To transcribe a version of the text in the web into a form which
can be compiled as a program.

@p The Master Tangler.
Here's what has happened so far, on a |-tangle| run of Inweb: on any
other sort of run, of course, we would never be in this section of code.
The web was read completely into memory, and then fully parsed, with all
of the arrays and hashes populated. Program Control then sent us straight
here for the tangling to begin...

@c
void Tangler::go(web *W, tangle_target *target, filename *dest_file) {
	programming_language *lang = target->tangle_language;
	PRINT("  tangling <%/f> (written in %s)\n", dest_file, lang->language_name);

	if (Languages::tangles(lang) == FALSE) /* for documentation webs, for instance */
		Errors::fatal_with_C_string("can't tangle material in the language", lang->language_name);

	text_stream TO_struct;
	text_stream *OUT = &TO_struct;
	if (STREAM_OPEN_TO_FILE(OUT, dest_file, ISO_ENC) == FALSE)
		Errors::fatal_with_file("unable to write tangled file", dest_file);
	@<Perform the actual tangle@>;
	STREAM_CLOSE(OUT);
	
	@<Tangle any imported headers@>;
	Languages::additional_tangling(lang, W, target);
}

@ All of the sections are tangled together into one big file, the structure
of which can be seen below.

@d LOOP_OVER_PARAGRAPHS(T)
	for (chapter *C = W->first_chapter; C; C = C->next_chapter)
		for (section *S = C->first_section; S; S = S->next_section)
			if (S->sect_target == T)
				for (paragraph *P = S->first_paragraph; P; P = P->next_paragraph_in_section)

@<Perform the actual tangle@> =
	/* (a) The shebang line, a header for scripting languages, and other heading matter */
	Languages::shebang(OUT, lang, W, target);

	/* (b) Results of |@d| declarations */
	@<Tangle all the constant definitions in section order@>;

	/* (c) Miscellaneous automated C predeclarations */
	Languages::additional_predeclarations(OUT, lang, W);

	/* (d) Above-the-bar code from all of the sections (global variables, and such) */
	LOOP_OVER_PARAGRAPHS(target)
		if ((S->barred) && (P->placed_early) && (P->defines_macro == NULL))
			Tangler::tangle_paragraph(OUT, P);

	/* (e) Below-the-bar code: the bulk of the program itself */
	LOOP_OVER_PARAGRAPHS(target)
		if (((S->barred == FALSE) || (P->placed_early == FALSE)) && (P->defines_macro == NULL))
			Tangler::tangle_paragraph(OUT, P);

	/* (f) Opposite of the shebang: a footer */
	Languages::gnabehs(OUT, lang, W);

@ This is the result of all those |@d| definitions; note that these sometimes
extend across multiple lines.

@<Tangle all the constant definitions in section order@> =
	LOOP_WITHIN_TANGLE(target)
		if (L->category == BEGIN_DEFINITION_LCAT) {
			Languages::start_definition(OUT, lang,
				L->text_operand,
				L->text_operand2, S, L);
			while ((L->next_line) && (L->next_line->category == CONT_DEFINITION_LCAT)) {
				L = L->next_line;
				Languages::prolong_definition(OUT, lang, L->text, S, L);
			}
			Languages::end_definition(OUT, lang, S, L);
		}

@

@<Tangle any imported headers@> =
	for (imported_header *H = W->first_header; H; H = H->next_header)
		Shell::copy(H->header_file, Pathnames::subfolder(W->path_to_web, "Tangled"), "");

@ So here is the main tangler for a single paragraph. We basically expect to
act only on |CODE_BODY_LCAT| lines (those containing actual code), unless
something quirky has been done to support a language feature.

@c
void Tangler::tangle_paragraph(OUTPUT_STREAM, paragraph *P) {
	int contiguous = FALSE;
	for (source_line *L = P->first_line_in_paragraph;
		((L) && (L->owning_paragraph == P)); L = L->next_line) {
		if (Languages::will_insert_in_tangle(P->under_section->sect_language, L)) {
			@<Insert line marker if necessary to show the origin of this code@>;
			Languages::insert_in_tangle(OUT, P->under_section->sect_language, L);	
		}
		if ((L->category != CODE_BODY_LCAT) || (L->suppress_tangling)) {
			contiguous = FALSE;
		} else {
			@<Insert line marker if necessary to show the origin of this code@>;
			Tangler::tangle_code(OUT, L->text, P->under_section, L); WRITE("\n");
		}
	}
}

@ The tangled file is, as the term suggests, a tangle, with lines coming
from many different origins. Some programming languages (C, for instance)
support a notation to tell the compiler that code has come from somewhere
else; if so, here's where we use it.

@<Insert line marker if necessary to show the origin of this code@> =
	if (contiguous == FALSE) {
		contiguous = TRUE;
		Languages::insert_line_marker(OUT, P->under_section->sect_language, L);
	}

@p The Code Tangler.
All of the final tangled code passes through the following routine.
Almost all of the time, it simply prints |original| verbatim to the file |OUT|.

@c
void Tangler::tangle_code(OUTPUT_STREAM, char *original, section *S, source_line *L) {
	int mlen, slen;
	int mpos = ISORegexp::find_expansion(original, '@', '<', '@', '>', &mlen);
	int spos = ISORegexp::find_expansion(original, '[', '[', ']', ']', &slen);
	if ((mpos >= 0) && ((spos == -1) || (mpos <= spos))) 
		@<Expand a CWEB macro@>
	else if (spos >= 0)
		@<Expand a double-square command@>
	else
		Languages::tangle_code(OUT, S->sect_language, original); /* this is usually what happens */
}

@ The first form of escape is a CWEB macro in the middle of code. For
example, we handle

	|if (banana_count == 0) @<Yes, we have no bananas@>;|

by calling the lower-level tangler on |if (banana_count == 0) | (a substring
which we know can't involve any macros, since we are detecting macros from
left to right, and this is to the left of the one we found); then by tangling
the definition of "Yes, we have no bananas"; then by calling the upper-level
code tangler on |;|. (In this case, of course, there's nothing much there,
but in principle it could contain further macros.)

Note that when we've expanded "Yes, we have no bananas" we have certainly
placed code into the tangled file from a different location; that will insert
a |#line| marker for the definition location; and we don't want the eventual
C compiler to think that the code which follows is also from that location.
So we insert a fresh line marker.

@<Expand a CWEB macro@> =
	string temp; CStrings::copy(temp, original); CStrings::truncate(temp, mpos);
	Languages::tangle_code(OUT, S->sect_language, temp);

	programming_language *lang = S->sect_language;
	for (int i=0; i<mlen-4; i++) CStrings::set_char(temp, i, original[mpos+2+i]); CStrings::truncate(temp, mlen-4);
	cweb_macro *cwm = Parser::get_cweb_macro_by_name(temp, S);
	if (cwm) {
		Languages::before_macro_expansion(OUT, lang, cwm);
		Tangler::tangle_paragraph(OUT, cwm->defining_paragraph);
		Languages::after_macro_expansion(OUT, lang, cwm);
		Languages::insert_line_marker(OUT, lang, L);
	} else {
		Main::error_in_web("unknown macro", L);
		fprintf(stderr, "Macro is '%s'\n", temp);
		Languages::comment(OUT, lang, temp); /* recover by putting macro name in comment */
	}

	Tangler::tangle_code(OUT, original + mpos + mlen, S, L);

@ This is a similar matter, except that it expands bibliographic data:

	|printf("This is build [[Build Number]].\n");|

takes the bibliographic data for "Build Number" (as set on the web's contents
page) and substitutes that, so that we end up with (say)

	|printf("This is build 5Q47.\n");|

In some languages there are also special expansions (for example, in
C-for-Inform |[[nonterminals]]| has a special meaning).

If the text in double-squares isn't recognised, that's not an error: it simply
passes straight through. So |[[water]]| becomes just |[[water]]|.

@<Expand a double-square command@> =
	web *W = S->owning_chapter->owning_web;

	string temp; CStrings::copy(temp, "");
	for (int i=0; i<spos; i++) CStrings::set_char(temp, i, original[i]); CStrings::truncate(temp, spos);
	Languages::tangle_code(OUT, S->sect_language, temp);

	for (int i=0; i<slen-4; i++) CStrings::set_char(temp, i, original[spos+2+i]); CStrings::truncate(temp, slen-4);
	if (Languages::special_data(OUT, S->sect_language, temp) == FALSE) {
		if (Bibliographic::look_up_datum(W, temp))
			WRITE("%s", Bibliographic::get_data(W, temp));
		else
			WRITE("[[%s]]", temp);
	}

	Tangler::tangle_code(OUT, original + spos + slen, S, L);
