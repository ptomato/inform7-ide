6/plain: Plain Text Format.

@Purpose: To provide for weaving in plain text format, which is not very
interesting, but ought to be available.

@

@c
void plain_top(FILE *F, weave_target *wv, char *comment) {
	fprintf(F, "[%s]\n", comment);
}

@

@c
void plain_subheading(FILE *F, weave_target *wv, int level, char *comment, char *head) {
	fprintf(F, "%s:\n", comment);
	if ((level == 2) && (head)) { format_identifier(F, wv, head); fprintf(F, "\n\n"); }
}

@

@c
void plain_toc(FILE *F, weave_target *wv, int stage, char *text1, char *text2, paragraph *P) {
	switch (stage) {
		case 1: fprintf(F, "%s.", text1); break;
		case 2: fprintf(F, "; "); break;
		case 3: fprintf(F, "%s %s", text1, text2); break;
		case 4: fprintf(F, "\n\n"); break;
	}
}

@

@c
void plain_chapter_tp(FILE *F, weave_target *wv, chapter *C) {
	fprintf(F, "%s\n\n", C->rubric);
	for (section *S2 = C->first_section; S2; S2 = S2->next_section)
		fprintf(F, "    %s: %s\n        %s\n",
			S2->sigil, S2->sect_title, S2->sect_purpose);
}

@

@c
void plain_paragraph_heading(FILE *F, weave_target *wv, char *TeX_macro,
	section *S, paragraph *P, char *heading_text, char *chaptermark, char *sectionmark,
	int weight) {
	if (P) {
		fprintf(F, "\n");
		format_locale(F, wv, P, NULL);
		fprintf(F, ". %s    ", heading_text);
	} else {
		fprintf(F, "%s\n\n", heading_text);
	}
}

@

@c
void plain_source_code(FILE *WEAVEOUT, weave_target *wv,
	int tab_stops_of_indentation, 
	char *prefatory, char *matter, char *colouring, char *concluding_comment,
	int starts, int finishes, int code_mode) {
	if (starts) {
		for (int i=0; i<tab_stops_of_indentation; i++)
			fprintf(WEAVEOUT, "    ");
		if (prefatory[0]) fprintf(WEAVEOUT, "%s ", prefatory);
	}
	fprintf(WEAVEOUT, "%s", matter);
	if (finishes) {		
		if (concluding_comment[0]) fprintf(WEAVEOUT, "[%s]", concluding_comment);
		fprintf(WEAVEOUT, "\n");
	}
}

@

@c
void plain_inline_code(FILE *F, weave_target *wv, int enter) {
}

@

@c
void plain_comment_lines(FILE *F, weave_target *wv, source_line *from, source_line *to) {
}

@

@c
void plain_display_line(FILE *F, weave_target *wv, char *from) {
	fprintf(F, "    %s\n", from);
}

@

@c
void plain_item(FILE *F, weave_target *wv, int depth, char *label) {
	if (depth == 1) fprintf(F, "%-4s  ", label);
	else fprintf(F, "%-8s  ", label);
}

@

@c
void plain_bar(FILE *F, weave_target *wv) {
	fprintf(F, "\n----------------------------------------------------------------------\n\n");
}

@

@c
void plain_figure(FILE *F, weave_target *wv, char *figname, int cm) {	
}

@

@c
void plain_cweb_macro(char *matter, weave_target *wv, cweb_macro *cwm, int defn) {
	sprintf(matter, "<%s (%s)>%s",
		cwm->macro_name, cwm->defining_paragraph->paragraph_number,
		(defn)?" =":"");
}

@

@c
void plain_pagebreak(FILE *F, weave_target *wv) {
}

@

@c
void plain_blank_line(FILE *F, weave_target *wv, int in_comment) {
	fprintf(F, "\n");
}

@

@c
void plain_code_note(FILE *F, weave_target *wv, char *comment) {
	fprintf(F, "    ...and so on...\n");
}

@

@c
void plain_change_mode(FILE *F, weave_target *wv, int old_mode, int new_mode, int content) {
}

@

@c
void plain_change_colour(weave_target *wv, char *slot, int col, int in_code) {
}

@

@c
void plain_endnote(FILE *F, weave_target *wv, int end) {
	fprintf(F, "\n");
}

@

@c
void plain_identifier(FILE *F, weave_target *wv, char *id) {
	fprintf(F, "%s", id);
}

@

@c
void plain_locale(FILE *F, weave_target *wv, paragraph *par1, paragraph *par2) {
	fprintf(F, "%s%s", par1->ornament, par1->paragraph_number);
	if (par2) fprintf(F, "-%s", par2->paragraph_number);
}

@

@c
void plain_tail(FILE *F, weave_target *wv, char *comment) {
	fprintf(F, "[%s]\n", comment);
}
