[PlainText::] Plain Text Format.

@Purpose: To provide for weaving in plain text format, which is not very
interesting, but ought to be available.

@

@c
void PlainText::top(OUTPUT_STREAM, weave_target *wv, char *comment) {
	WRITE("[%s]\n", comment);
}

@

@c
void PlainText::subheading(OUTPUT_STREAM, weave_target *wv, int level, char *comment, char *head) {
	WRITE("%s:\n", comment);
	if ((level == 2) && (head)) { Formats::identifier(OUT, wv, head); WRITE("\n\n"); }
}

@

@c
void PlainText::toc(OUTPUT_STREAM, weave_target *wv, int stage, char *text1, char *text2, paragraph *P) {
	switch (stage) {
		case 1: WRITE("%s.", text1); break;
		case 2: WRITE("; "); break;
		case 3: WRITE("%s %s", text1, text2); break;
		case 4: WRITE("\n\n"); break;
	}
}

@

@c
void PlainText::chapter_tp(OUTPUT_STREAM, weave_target *wv, chapter *C) {
	WRITE("%s\n\n", C->rubric);
	for (section *S2 = C->first_section; S2; S2 = S2->next_section)
		WRITE("    %s: %s\n        %s\n",
			S2->sigil, S2->sect_title, S2->sect_purpose);
}

@

@c
void PlainText::paragraph_heading(OUTPUT_STREAM, weave_target *wv, char *TeX_macro,
	section *S, paragraph *P, char *heading_text, char *chaptermark, char *sectionmark,
	int weight) {
	if (P) {
		WRITE("\n");
		Formats::locale(OUT, wv, P, NULL);
		WRITE(". %s    ", heading_text);
	} else {
		WRITE("%s\n\n", heading_text);
	}
}

@

@c
void PlainText::source_code(OUTPUT_STREAM, weave_target *wv,
	int tab_stops_of_indentation, 
	char *prefatory, char *matter, char *colouring, char *concluding_comment,
	int starts, int finishes, int code_mode) {
	if (starts) {
		for (int i=0; i<tab_stops_of_indentation; i++)
			WRITE("    ");
		if (prefatory[0]) WRITE("%s ", prefatory);
	}
	WRITE("%s", matter);
	if (finishes) {		
		if (concluding_comment[0]) WRITE("[%s]", concluding_comment);
		WRITE("\n");
	}
}

@

@c
void PlainText::inline_code(OUTPUT_STREAM, weave_target *wv, int enter) {
}

@

@c
void PlainText::comment_lines(OUTPUT_STREAM, weave_target *wv, source_line *from, source_line *to) {
}

@

@c
void PlainText::display_line(OUTPUT_STREAM, weave_target *wv, char *from) {
	WRITE("    %s\n", from);
}

@

@c
void PlainText::item(OUTPUT_STREAM, weave_target *wv, int depth, char *label) {
	if (depth == 1) WRITE("%-4s  ", label);
	else WRITE("%-8s  ", label);
}

@

@c
void PlainText::bar(OUTPUT_STREAM, weave_target *wv) {
	WRITE("\n----------------------------------------------------------------------\n\n");
}

@

@c
void PlainText::figure(OUTPUT_STREAM, weave_target *wv, char *figname, int cm) {	
}

@

@c
void PlainText::cweb_macro(OUTPUT_STREAM, weave_target *wv, cweb_macro *cwm, int defn) {
	WRITE("<%s (%s)>%s",
		cwm->macro_name, cwm->defining_paragraph->paragraph_number,
		(defn)?" =":"");
}

@

@c
void PlainText::pagebreak(OUTPUT_STREAM, weave_target *wv) {
}

@

@c
void PlainText::blank_line(OUTPUT_STREAM, weave_target *wv, int in_comment) {
	WRITE("\n");
}

@

@c
void PlainText::code_note(OUTPUT_STREAM, weave_target *wv, char *comment) {
	WRITE("    ...and so on...\n");
}

@

@c
void PlainText::change_mode(OUTPUT_STREAM, weave_target *wv, int old_mode, int new_mode, int content) {
}

@

@c
void PlainText::change_colour(OUTPUT_STREAM, weave_target *wv, int col, int in_code) {
}

@

@c
void PlainText::endnote(OUTPUT_STREAM, weave_target *wv, int end) {
	WRITE("\n");
}

@

@c
void PlainText::identifier(OUTPUT_STREAM, weave_target *wv, char *id) {
	WRITE("%s", id);
}

@

@c
void PlainText::locale(OUTPUT_STREAM, weave_target *wv, paragraph *par1, paragraph *par2) {
	WRITE("%s%s", par1->ornament, par1->paragraph_number);
	if (par2) WRITE("-%s", par2->paragraph_number);
}

@

@c
void PlainText::tail(OUTPUT_STREAM, weave_target *wv, char *comment) {
	WRITE("[%s]\n", comment);
}
