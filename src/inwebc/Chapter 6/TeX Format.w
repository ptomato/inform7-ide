6/tex: TeX Format.

@Purpose: To provide for weaving in the standard maths and science typesetting
software, TeX.

@

@c
void tex_top(FILE *F, weave_target *wv, char *comment) {
	fprintf(F, "%% %s\n", comment);
	@<Incorporate suitable |TeX| macro definitions into the woven output@>;
}

@ We don't use |TeX|'s |\input| mechanism for macros because it is so prone to
failures when searching directories (especially those with spaces in the
names) and then locking |TeX| into a repeated prompt for help from |stdin|
which is rather hard to escape from.

Instead we paste the entire text of our macros file into the woven |TeX|:

@<Incorporate suitable |TeX| macro definitions into the woven output@> =
	string macros_at;
	in_sprintf(macros_at, "%sinweb-macros.tex", path_to_inweb_materials);
	FILE *MACROS = fopen(macros_at, "r");
	if (MACROS == NULL) fatal_filing_system_error("can't open file of TeX macros", macros_at);
	while (TRUE) {
		int c = fgetc(MACROS);
		if (c == EOF) break;
		putc(c, F);
	}
	fclose(MACROS);

@

@c
void tex_subheading(FILE *F, weave_target *wv, int level, char *comment, char *head) {
	switch (level) {
		case 1:
			fprintf(F, "\\par\\noindent{\\bf %s}\\mark{%s}\\medskip\n",
				comment, head);
			break;
		case 2:
			fprintf(F, "\\smallskip\\par\\noindent{\\it %s}\\smallskip\\noindent\n",
				comment);
			if (head) format_identifier(F, wv, head);
			break;
	}
}

@

@c
void tex_toc(FILE *F, weave_target *wv, int stage, char *text1, char *text2, paragraph *P) {
	switch (stage) {
		case 1:
			fprintf(F, "\\medskip\\hrule\\smallskip\\par\\noindent{\\usagefont %s.", 
				text1);
			break;
		case 2:
			fprintf(F, "; ");
			break;
		case 3:
			fprintf(F, "%s~%s", text1, text2);
			break;
		case 4:
			fprintf(F, "}\\par\\medskip\\hrule\\bigskip\n");
			break;
	}
}

@

@c
void tex_chapter_tp(FILE *F, weave_target *wv, chapter *C) {
	fprintf(F, "%s\\medskip\n", C->rubric);
	for (section *S2 = C->first_section; S2; S2 = S2->next_section)
		fprintf(F, "\\smallskip\\noindent |%s|: {\\it %s}\\qquad\n%s",
			S2->sigil, S2->sect_title, S2->sect_purpose);
}

@

@c
void tex_paragraph_heading(FILE *F, weave_target *wv, char *TeX_macro,
	section *S, paragraph *P, char *heading_text, char *chaptermark, char *sectionmark,
	int weight) {
	char *orn = (P)?(P->ornament):"P";
	char *N = (P)?(P->paragraph_number):"";
	string mark; in_strcpy(mark, "");
	in_sprintf(mark, "%s%s\\quad$\\%s$%s", chaptermark, sectionmark, orn, N);
	string modified; in_strcpy(modified, heading_text);
	if (pattern_match(modified, "(%c*?): (%c*)"))
		in_sprintf(modified, "{\\sinchhigh %s}\\quad %s", found_text1, found_text2);
	fprintf(F, "\\%s{%s}{%s}{%s}{\\%s}{%s}%%\n",
		TeX_macro, N, modified, mark, orn, S->sigil);
}

@ Code is typeset by |TeX| within vertical strokes; these switch a sort of
typewriter-type verbatim mode on and off. To get an actual stroke, we must
escape from code mode, escape it using a backslash \|, then re-enter code
mode once again:

@c
void tex_source_code(FILE *WEAVEOUT, weave_target *wv,
	int tab_stops_of_indentation, 
	char *prefatory, char *matter, char *colouring, char *concluding_comment,
	int starts, int finishes, int code_mode) {
	if (code_mode == FALSE) fprintf(WEAVEOUT, "\\smallskip\\par\\noindent");
	if (starts) {
		@<Weave a suitable horizontal advance for that many tab stops@>;
		if (prefatory[0]) fprintf(WEAVEOUT, "{\\ninebf %s} ", prefatory);
		fprintf(WEAVEOUT, "|");
	}
	int current_colour = PLAIN_CODE, colour_wanted = PLAIN_CODE;
	for (int i=0; matter[i]; i++) {
		colour_wanted = colouring[i]; @<Adjust code colour as necessary@>;
		if (matter[i] == '|') fprintf(WEAVEOUT, "|\\||");
		else fprintf(WEAVEOUT, "%c", matter[i]);
	}
	colour_wanted = PLAIN_CODE; @<Adjust code colour as necessary@>;
	if (finishes) {		
		fprintf(WEAVEOUT, "|");
		if (concluding_comment[0]) {
			if ((in_string_ne(matter, "")) || (!starts))
				fprintf(WEAVEOUT, "\\hfill\\quad ");
			fprintf(WEAVEOUT, "{\\ttninepoint\\it %s}", concluding_comment);
		}
		fprintf(WEAVEOUT, "\n");
	}
}

@ We actually use |\qquad| horizontal spaces rather than risk using |TeX|'s
messy alignment system:

@<Weave a suitable horizontal advance for that many tab stops@> =
	for (int i=0; i<tab_stops_of_indentation; i++)
		fprintf(WEAVEOUT, "\\qquad");

@

@<Adjust code colour as necessary@> =
	if (colour_wanted != current_colour) {
		string col;
		format_change_colour(wv, col, colour_wanted, TRUE);
		fprintf(WEAVEOUT, "%s", col);
		current_colour = colour_wanted;
	}

@

@c
void tex_inline_code(FILE *F, weave_target *wv, int enter) {
	fprintf(F, "|");
}

@

@c
void tex_change_colour(weave_target *wv, char *slot, int col, int in_code) {
	if (wv->format == pdf_format) {
		char *inout = "";
		if (in_code) inout = "|";
		switch (col) {
			case MACRO_CODE: in_sprintf(slot, "%s\\pdfliteral direct{1 1 0 0 k}%s", inout, inout); break;
			case FUNCTION_CODE: in_sprintf(slot, "%s\\pdfliteral direct{0 1 1 0 k}%s", inout, inout); break;
			case PLAIN_CODE: in_sprintf(slot, "%s\\special{PDF:0 g}%s", inout, inout); break;
		}
	}
}

@

@c
void tex_comment_lines(FILE *F, weave_target *wv, source_line *from, source_line *to) {
}

@

@c
void tex_display_line(FILE *F, weave_target *wv, char *text) {
	fprintf(F, "\\quotesource{%s}\n", text);
}

@

@c
void tex_item(FILE *F, weave_target *wv, int depth, char *label) {
	if (label[0]) {
		if (depth == 1) fprintf(F, "\\item{(%s)}", label);
		else fprintf(F, "\\itemitem{(%s)}", label);
	} else {
		if (depth == 1) fprintf(F, "\\item{}");
		else fprintf(F, "\\itemitem{}");
	}
}

@

@c
void tex_bar(FILE *F, weave_target *wv) {
	fprintf(F, "\\par\\medskip\\noindent\\hrule\\medskip\\noindent\n");
}

@ |TeX| itself has an almost defiant lack of support for anything pictorial,
which is one reason it didn't live up to its hope of being the definitive basis
for typography; even today the loose confederation of |TeX|-like programs and
extensions lack standard approaches. Here we're going to use |pdftex| features,
having nothing better. All we're trying for is to insert a picture, scaled
to a given width, into the text at the current position.

@c
void tex_figure(FILE *F, weave_target *wv, char *figname, int cm) {	
	if (wv->format == pdf_format) {
		fprintf(F, "\\pdfximage");
		if (cm >= 0)
			fprintf(F, " width %d cm{../Figures/%s}\n", cm, figname);
		else
			fprintf(F, "{../Figures/%s}\n", figname);
		fprintf(F, "\\smallskip\\noindent"
			"\\hbox to\\hsize{\\hfill\\pdfrefximage \\pdflastximage\\hfill}"
			"\\smallskip\n");
	}
}

@ Any usage of angle-macros is highlighted in several cute ways: first,
we make use of colour and we drop in the paragraph number of the definition
of the macro in small type; and second, we use cross-reference links.

@c
void tex_cweb_macro(char *matter, weave_target *wv, cweb_macro *cwm, int defn) {
	if (wv->format == pdf_format) {
		if (defn)
			in_sprintf(matter,
				"|\\pdfdest num %d fit ", 
				cwm->allocation_id + 100);
		else
			in_sprintf(matter,
				"|\\pdfstartlink attr{/C [0.9 0 0] /Border [0 0 0]} goto num %d ",
				cwm->allocation_id + 100);
	}
	in_strcat(matter, "$\\langle${\\xreffont");
	string col;
	format_change_colour(wv, col, MACRO_CODE, FALSE); in_strcat(matter, col);
	in_strcat(matter, cwm->macro_name);
	in_strcat(matter, " ");
	string rest;
	in_sprintf(rest, "{\\sevenss %s}}", cwm->defining_paragraph->paragraph_number);
	in_strcat(matter, rest);
	format_change_colour(wv, col, PLAIN_CODE, FALSE); in_strcat(matter, col);
	in_strcat(matter, col);
	in_strcat(matter, "$\\rangle$ ");		
	if (wv->format == pdf_format) {
		if (defn)
			in_strcat(matter, "$\\equiv$|");
		else
			in_strcat(matter, "\\pdfendlink|");
	}
}

@

@c
void tex_pagebreak(FILE *F, weave_target *wv) {
	fprintf(F, "\\vfill\\eject\n");
}

@

@c
void tex_blank_line(FILE *F, weave_target *wv, int in_comment) {
	if (in_comment) fprintf(F, "\\smallskip\\par\\noindent%%\n");
	else fprintf(F, "\\smallskip\n");
}

@

@c
void tex_endnote(FILE *F, weave_target *wv, int end) {
	if (end == 1) {
		fprintf(F, "\\par\\noindent\\penalty10000\n");
		fprintf(F, "{\\usagefont ");
	} else {
		fprintf(F, "}\\smallskip\n");
	}
}

@

@c
void tex_identifier(FILE *F, weave_target *wv, char *id) {
	for (int i=0; id[i]; i++) {
		switch (id[i]) {
			case '_': fprintf(F, "\\_"); break;
			case '"':
				if ((id[i] == '"') && ((i==0) || (id[i-1] == ' ')))
					fprintf(F, "``");
				else
					fprintf(F, "''");
				break;
			default: fprintf(F, "%c", id[i]);
		}
	}
}

@

@c
void tex_locale(FILE *F, weave_target *wv, paragraph *par1, paragraph *par2) {
	fprintf(F, "$\\%s$%s", par1->ornament, par1->paragraph_number);
	if (par2) fprintf(F, "-%s", par2->paragraph_number);
}

@

@c
void tex_code_note(FILE *F, weave_target *wv, char *comment) {
	fprintf(F, "\\smallskip\\par\\noindent");
	fprintf(F, "{\\ttninepoint\\it ...and so on...}\\smallskip\n");
}

@

@c
void tex_change_mode(FILE *F, weave_target *wv, int old_mode, int new_mode, int content) {
	if (old_mode != new_mode) {
		switch (old_mode) {
			case REGULAR_MATERIAL:
				switch (new_mode) {
					case CODE_MATERIAL:
						fprintf(F, "\\beginlines\n");
						break;
					case DEFINITION_MATERIAL:
						fprintf(F, "\\beginlines\n");
						break;
					case MACRO_MATERIAL:
						fprintf(F, "\\beginlines\n");
						break;
				}
				break;
			default:
				if (new_mode == REGULAR_MATERIAL)
					fprintf(F, "\\endlines\n");
				break;
		}
	}
}

@

@c
void tex_tail(FILE *F, weave_target *wv, char *comment) {
	fprintf(F, "%% %s\n", comment);
	fprintf(F, "\\end\n");
}
