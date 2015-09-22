[TeX::] TeX Format.

@Purpose: To provide for weaving in the standard maths and science typesetting
software, TeX.

@

@c
void TeX::top(OUTPUT_STREAM, weave_target *wv, char *comment) {
	WRITE("%% %s\n", comment);
	@<Incorporate suitable |TeX| macro definitions into the woven output@>;
}

@ We don't use |TeX|'s |\input| mechanism for macros because it is so prone to
failures when searching directories (especially those with spaces in the
names) and then locking |TeX| into a repeated prompt for help from |stdin|
which is rather hard to escape from.

Instead we paste the entire text of our macros file into the woven |TeX|:

@<Incorporate suitable |TeX| macro definitions into the woven output@> =
	filename *Macros = Filenames::in_folder(path_to_inweb_materials, "inweb-macros.tex");
	FILE *MACROS = Platform::iso_fopen(Macros, "r");
	if (MACROS == NULL) Errors::fatal_with_file("can't open file of TeX macros", Macros);
	while (TRUE) {
		int c = fgetc(MACROS);
		if (c == EOF) break;
		PUT(c);
	}
	fclose(MACROS);

@

@c
void TeX::subheading(OUTPUT_STREAM, weave_target *wv, int level, char *comment, char *head) {
	switch (level) {
		case 1:
			WRITE("\\par\\noindent{\\bf %s}\\mark{%s}\\medskip\n",
				comment, head);
			break;
		case 2:
			WRITE("\\smallskip\\par\\noindent{\\it %s}\\smallskip\\noindent\n",
				comment);
			if (head) Formats::identifier(OUT, wv, head);
			break;
	}
}

@

@c
void TeX::toc(OUTPUT_STREAM, weave_target *wv, int stage, char *text1, char *text2, paragraph *P) {
	switch (stage) {
		case 1:
			WRITE("\\medskip\\hrule\\smallskip\\par\\noindent{\\usagefont %s.", 
				text1);
			break;
		case 2:
			WRITE("; ");
			break;
		case 3:
			WRITE("%s~%s", text1, text2);
			break;
		case 4:
			WRITE("}\\par\\medskip\\hrule\\bigskip\n");
			break;
	}
}

@

@c
void TeX::chapter_tp(OUTPUT_STREAM, weave_target *wv, chapter *C) {
	WRITE("%s\\medskip\n", C->rubric);
	for (section *S2 = C->first_section; S2; S2 = S2->next_section)
		WRITE("\\smallskip\\noindent |%s|: {\\it %s}\\qquad\n%s",
			S2->sigil, S2->sect_title, S2->sect_purpose);
}

@

@c
void TeX::paragraph_heading(OUTPUT_STREAM, weave_target *wv, char *TeX_macro,
	section *S, paragraph *P, char *heading_text, char *chaptermark, char *sectionmark,
	int weight) {
	char *orn = (P)?(P->ornament):"P";
	char *N = (P)?(P->paragraph_number):"";
	string mark; CStrings::copy(mark, "");
	CSTRING_WRITE(mark, "%s%s\\quad$\\%s$%s", chaptermark, sectionmark, orn, N);
	string modified; CStrings::copy(modified, heading_text);
	string found_text1;
	string found_text2;	
	if (ISORegexp::match_2(modified, "(%c*?): (%c*)", found_text1, found_text2))
		CSTRING_WRITE(modified, "{\\sinchhigh %s}\\quad %s", found_text1, found_text2);
	WRITE("\\%s{%s}{%s}{%s}{\\%s}{%s}%%\n",
		TeX_macro, N, modified, mark, orn, S->sigil);
}

@ Code is typeset by |TeX| within vertical strokes; these switch a sort of
typewriter-type verbatim mode on and off. To get an actual stroke, we must
escape from code mode, escape it using a backslash \|, then re-enter code
mode once again:

@c
void TeX::source_code(OUTPUT_STREAM, weave_target *wv,
	int tab_stops_of_indentation, 
	char *prefatory, char *matter, char *colouring, char *concluding_comment,
	int starts, int finishes, int code_mode) {
	if (code_mode == FALSE) WRITE("\\smallskip\\par\\noindent");
	if (starts) {
		@<Weave a suitable horizontal advance for that many tab stops@>;
		if (prefatory[0]) WRITE("{\\ninebf %s} ", prefatory);
		WRITE("|");
	}
	int current_colour = PLAIN_CODE, colour_wanted = PLAIN_CODE;
	for (int i=0; matter[i]; i++) {
		colour_wanted = colouring[i]; @<Adjust code colour as necessary@>;
		if (matter[i] == '|') WRITE("|\\||");
		else WRITE("%c", matter[i]);
	}
	colour_wanted = PLAIN_CODE; @<Adjust code colour as necessary@>;
	if (finishes) {		
		WRITE("|");
		if (concluding_comment[0]) {
			if ((CStrings::ne(matter, "")) || (!starts))
				WRITE("\\hfill\\quad ");
			WRITE("{\\ttninepoint\\it %s}", concluding_comment);
		}
		WRITE("\n");
	}
}

@ We actually use |\qquad| horizontal spaces rather than risk using |TeX|'s
messy alignment system:

@<Weave a suitable horizontal advance for that many tab stops@> =
	for (int i=0; i<tab_stops_of_indentation; i++)
		WRITE("\\qquad");

@

@<Adjust code colour as necessary@> =
	if (colour_wanted != current_colour) {
		Formats::change_colour(OUT, wv, colour_wanted, TRUE);
		current_colour = colour_wanted;
	}

@

@c
void TeX::inline_code(OUTPUT_STREAM, weave_target *wv, int enter) {
	WRITE("|");
}

@

@c
void TeX::change_colour(OUTPUT_STREAM, weave_target *wv, int col, int in_code) {
	if (wv->format == pdf_format) {
		char *inout = "";
		if (in_code) inout = "|";
		switch (col) {
			case MACRO_CODE: WRITE("%s\\pdfliteral direct{1 1 0 0 k}%s", inout, inout); break;
			case FUNCTION_CODE: WRITE("%s\\pdfliteral direct{0 1 1 0 k}%s", inout, inout); break;
			case PLAIN_CODE: WRITE("%s\\special{PDF:0 g}%s", inout, inout); break;
		}
	}
}

@

@c
void TeX::comment_lines(OUTPUT_STREAM, weave_target *wv, source_line *from, source_line *to) {
}

@

@c
void TeX::display_line(OUTPUT_STREAM, weave_target *wv, char *text) {
	WRITE("\\quotesource{%s}\n", text);
}

@

@c
void TeX::item(OUTPUT_STREAM, weave_target *wv, int depth, char *label) {
	if (label[0]) {
		if (depth == 1) WRITE("\\item{(%s)}", label);
		else WRITE("\\itemitem{(%s)}", label);
	} else {
		if (depth == 1) WRITE("\\item{}");
		else WRITE("\\itemitem{}");
	}
}

@

@c
void TeX::bar(OUTPUT_STREAM, weave_target *wv) {
	WRITE("\\par\\medskip\\noindent\\hrule\\medskip\\noindent\n");
}

@ |TeX| itself has an almost defiant lack of support for anything pictorial,
which is one reason it didn't live up to its hope of being the definitive basis
for typography; even today the loose confederation of |TeX|-like programs and
extensions lack standard approaches. Here we're going to use |pdftex| features,
having nothing better. All we're trying for is to insert a picture, scaled
to a given width, into the text at the current position.

@c
void TeX::figure(OUTPUT_STREAM, weave_target *wv, char *figname, int cm) {	
	if (wv->format == pdf_format) {
		WRITE("\\pdfximage");
		if (cm >= 0)
			WRITE(" width %d cm{../Figures/%s}\n", cm, figname);
		else
			WRITE("{../Figures/%s}\n", figname);
		WRITE("\\smallskip\\noindent"
			"\\hbox to\\hsize{\\hfill\\pdfrefximage \\pdflastximage\\hfill}"
			"\\smallskip\n");
	}
}

@ Any usage of angle-macros is highlighted in several cute ways: first,
we make use of colour and we drop in the paragraph number of the definition
of the macro in small type; and second, we use cross-reference links.

@c
void TeX::cweb_macro(OUTPUT_STREAM, weave_target *wv, cweb_macro *cwm, int defn) {
	if (wv->format == pdf_format) {
		if (defn)
			WRITE("|\\pdfdest num %d fit ", 
				cwm->allocation_id + 100);
		else
			WRITE("|\\pdfstartlink attr{/C [0.9 0 0] /Border [0 0 0]} goto num %d ",
				cwm->allocation_id + 100);
	}
	WRITE("$\\langle${\\xreffont");
	Formats::change_colour(OUT, wv, MACRO_CODE, FALSE);
	WRITE(cwm->macro_name);
	WRITE(" ");
	string rest;
	CSTRING_WRITE(rest, "{\\sevenss %s}}", cwm->defining_paragraph->paragraph_number);
	WRITE(rest);
	Formats::change_colour(OUT, wv, PLAIN_CODE, FALSE);
	WRITE("$\\rangle$ ");		
	if (wv->format == pdf_format) {
		if (defn)
			WRITE("$\\equiv$|");
		else
			WRITE("\\pdfendlink|");
	}
}

@

@c
void TeX::pagebreak(OUTPUT_STREAM, weave_target *wv) {
	WRITE("\\vfill\\eject\n");
}

@

@c
void TeX::blank_line(OUTPUT_STREAM, weave_target *wv, int in_comment) {
	if (in_comment) WRITE("\\smallskip\\par\\noindent%%\n");
	else WRITE("\\smallskip\n");
}

@

@c
void TeX::endnote(OUTPUT_STREAM, weave_target *wv, int end) {
	if (end == 1) {
		WRITE("\\par\\noindent\\penalty10000\n");
		WRITE("{\\usagefont ");
	} else {
		WRITE("}\\smallskip\n");
	}
}

@

@c
void TeX::identifier(OUTPUT_STREAM, weave_target *wv, char *id) {
	int math_mode = FALSE;
	for (int i=0; id[i]; i++) {
		switch (id[i]) {
			case '$': math_mode = (math_mode)?FALSE:TRUE; break;
			case '_': if (math_mode) WRITE("_"); else WRITE("\\_"); break;
			case '"':
				if ((id[i] == '"') && ((i==0) || (id[i-1] == ' ') || (id[i-1] == '(')))
					WRITE("``");
				else
					WRITE("''");
				break;
			default: WRITE("%c", id[i]);
		}
	}
}

@

@c
void TeX::locale(OUTPUT_STREAM, weave_target *wv, paragraph *par1, paragraph *par2) {
	WRITE("$\\%s$%s", par1->ornament, par1->paragraph_number);
	if (par2) WRITE("-%s", par2->paragraph_number);
}

@

@c
void TeX::code_note(OUTPUT_STREAM, weave_target *wv, char *comment) {
	WRITE("\\smallskip\\par\\noindent");
	WRITE("{\\ttninepoint\\it ...and so on...}\\smallskip\n");
}

@

@c
void TeX::change_mode(OUTPUT_STREAM, weave_target *wv, int old_mode, int new_mode, int content) {
	if (old_mode != new_mode) {
		switch (old_mode) {
			case REGULAR_MATERIAL:
				switch (new_mode) {
					case CODE_MATERIAL:
						WRITE("\\beginlines\n");
						break;
					case DEFINITION_MATERIAL:
						WRITE("\\beginlines\n");
						break;
					case MACRO_MATERIAL:
						WRITE("\\beginlines\n");
						break;
				}
				break;
			default:
				if (new_mode == REGULAR_MATERIAL)
					WRITE("\\endlines\n");
				break;
		}
	}
}

@

@c
void TeX::tail(OUTPUT_STREAM, weave_target *wv, char *comment) {
	WRITE("%% %s\n", comment);
	WRITE("\\end\n");
}
