4/tang: The Tangler.

@Purpose: To transcribe a version of the text in the web into a form which
can be compiled as a program.

@-------------------------------------------------------------------------------

@p The Master Tangler.
Here's what has happened so far, on a |-tangle| run of Inweb: on any
other sort of run, of course, we would never be in this section of code.
The web was read completely into memory, and then fully parsed, with all
of the arrays and hashes populated. Program Control then sent us straight
here for the tangling to begin...

@c
void tangle_source(web *W, tangle_target *target, char *dest_file) {
	programming_language *lang = target->tangle_language;
	printf("  tangling <%s> (written in %s)\n", dest_file, lang->language_name);

	if (language_tangles(lang) == FALSE) /* for documentation webs, for instance */
		fatal_error_with_parameter("can't tangle material in the language", lang->language_name);
			
	FILE *F = fopen(dest_file, "w");
	if (F == NULL) 
		fatal_filing_system_error("unable to open tangled file for output", dest_file);

	@<Perform the actual tangle@>;
	fclose(F);
	
	language_additional_tangling(lang, W, target);
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
	language_shebang(F, lang, W, target);

	/* (b) Results of |@d| declarations */
	@<Tangle all the constant definitions in section order@>;

	/* (c) Miscellaneous automated C predeclarations */
	language_additional_predeclarations(F, lang, W);

	/* (d) Above-the-bar code from all of the sections (global variables, and such) */
	LOOP_OVER_PARAGRAPHS(target)
		if ((S->barred) && (P->placed_early) && (P->defines_macro == NULL))
			tangle_paragraph(F, P);

	/* (e) Below-the-bar code: the bulk of the program itself */
	LOOP_OVER_PARAGRAPHS(target)
		if (((S->barred == FALSE) || (P->placed_early == FALSE)) && (P->defines_macro == NULL))
			tangle_paragraph(F, P);

	/* (f) Opposite of the shebang: a footer */
	language_gnabehs(F, lang, W);

@ This is the result of all those |@d| definitions; note that these sometimes
extend across multiple lines.

@<Tangle all the constant definitions in section order@> =
	LOOP_WITHIN_TANGLE(target)
		if (L->category == BEGIN_DEFINITION_LCAT) {
			language_start_definition(F, lang,
				L->text_operand,
				L->text_operand2, S, L);
			while ((L->next_line) && (L->next_line->category == CONT_DEFINITION_LCAT)) {
				L = L->next_line;
				language_prolong_definition(F, lang, L->text, S, L);
			}
			language_end_definition(F, lang, S, L);
		}

@ So here is the main tangler for a single paragraph. We basically expect to
act only on |CODE_BODY_LCAT| lines (those containing actual code), unless
something quirky has been done to support a language feature.

@c
void tangle_paragraph(FILE *F, paragraph *P) {
	int contiguous = FALSE;
	for (source_line *L = P->first_line_in_paragraph;
		((L) && (L->owning_paragraph == P)); L = L->next_line) {
		if (language_will_insert_in_tangle(P->under_section->sect_language, L)) {
			@<Insert line marker if necessary to show the origin of this code@>;
			language_insert_in_tangle(F, P->under_section->sect_language, L);	
		}
		if ((L->category != CODE_BODY_LCAT) || (L->suppress_tangling)) {
			contiguous = FALSE;
		} else {
			@<Insert line marker if necessary to show the origin of this code@>;
			tangle_code(F, L->text, P->under_section, L); fprintf(F, "\n");
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
		language_insert_line_marker(F, P->under_section->sect_language, L);
	}

@p The Code Tangler.
All of the final tangled code passes through the following routine.
Almost all of the time, it simply prints |original| verbatim to the file |F|.

@c
void tangle_code(FILE *F, char *original, section *S, source_line *L) {
	int mlen, slen;
	int mpos = find_expansion(original, '@', '<', '@', '>', &mlen);
	int spos = find_expansion(original, '[', '[', ']', ']', &slen);
	if ((mpos >= 0) && ((spos == -1) || (mpos <= spos))) 
		@<Expand a CWEB macro@>
	else if (spos >= 0)
		@<Expand a double-square command@>
	else
		language_tangle_code(F, S->sect_language, original); /* this is usually what happens */
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
	string temp; in_strcpy(temp, original); in_truncate(temp, mpos);
	language_tangle_code(F, S->sect_language, temp);

	programming_language *lang = S->sect_language;
	for (int i=0; i<mlen-4; i++) in_set(temp, i, original[mpos+2+i]); in_truncate(temp, mlen-4);
	cweb_macro *cwm = get_cweb_macro_by_name(temp, S);
	if (cwm) {
		language_before_macro_expansion(F, lang, cwm);
		tangle_paragraph(F, cwm->defining_paragraph);
		language_after_macro_expansion(F, lang, cwm);
		language_insert_line_marker(F, lang, L);
	} else {
		error_in_web("unknown macro", L);
		fprintf(stderr, "Macro is '%s'\n", temp);
		language_comment(F, lang, temp); /* recover by putting macro name in comment */
	}

	tangle_code(F, original + mpos + mlen, S, L);

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

	string temp; in_strcpy(temp, "");
	for (int i=0; i<spos; i++) in_set(temp, i, original[i]); in_truncate(temp, spos);
	language_tangle_code(F, S->sect_language, temp);

	for (int i=0; i<slen-4; i++) in_set(temp, i, original[spos+2+i]); in_truncate(temp, slen-4);
	if (language_special_data(F, S->sect_language, temp) == FALSE) {
		if (look_up_bibliographic_datum(W, temp))
			fprintf(F, "%s", get_bibliographic_data(W, temp));
		else
			fprintf(F, "[[%s]]", temp);
	}

	tangle_code(F, original + spos + slen, S, L);
