6/form: Weave Formats.

@Purpose: To characterise the different weaving output formats (text, typeset,
web and so on).

@p Creation.
This must be performed very early in Inweb's run.

Add PDF and DVI; formalise commands via configuration; with **


@c
weave_format *plain_text_format = NULL;
weave_format *tex_format = NULL;
weave_format *dvi_format = NULL;
weave_format *pdf_format = NULL;
weave_format *html_format = NULL;
weave_format *epub_format = NULL;

void create_weave_formats(void) {
	plain_text_format = create_weave_format("plain", ".txt", NULL);
	tex_format = create_weave_format("TeX", ".tex", NULL);
	dvi_format = create_weave_format("DVI", ".dvi", tex_format);
	pdf_format = create_weave_format("PDF", ".pdf", tex_format);
	html_format = create_weave_format("HTML", ".html", NULL);
	epub_format = create_weave_format("ePub", ".epub", html_format);
}

@ We don't store very much about any given format:

@c
typedef struct weave_format {
	char *format_name;
	char *woven_extension;
	struct weave_format *intermediate;
	MEMORY_MANAGEMENT
} weave_format;

@ And this makes them:

@c
weave_format *create_weave_format(char *name, char *ext, weave_format *intermediate) {
	weave_format *wf = CREATE(weave_format);
	wf->format_name = name;
	wf->woven_extension = ext;
	wf->intermediate = intermediate;
	return wf;
}

@ They are identified by name.

@c
weave_format *parse_format(char *name) {
	weave_format *wf;
	LOOP_OVER(wf, weave_format)
		if (in_string_eq_insensitive(name, wf->format_name))
			return wf;
	return NULL;
}

int html_like(weave_format *wf) {
	if (wf->intermediate) wf = wf->intermediate;
	if ((wf == html_format) || (wf == epub_format)) return TRUE;
	return FALSE;
}

char *weave_file_extension(weave_format *wf) {
	if (wf->intermediate) wf = wf->intermediate;
	return wf->woven_extension;
}

@p During the weave.

@c
void format_top(FILE *F, weave_target *wv, char *comment) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) return plain_top(F, wv, comment);
	if (wf == tex_format) return tex_top(F, wv, comment);
	if (html_like(wf)) return html_top(F, wv, comment);
}

void format_subheading(FILE *F, weave_target *wv, int level, char *comment, char *head) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) return plain_subheading(F, wv, level, comment, head);
	if (wf == tex_format) return tex_subheading(F, wv, level, comment, head);
	if (html_like(wf)) return html_subheading(F, wv, level, comment, head);
}

void format_toc(FILE *F, weave_target *wv, int stage, char *text1, char *text2, paragraph *P) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) return plain_toc(F, wv, stage, text1, text2, P);
	if (wf == tex_format) return tex_toc(F, wv, stage, text1, text2, P);
	if (html_like(wf)) return html_toc(F, wv, stage, text1, text2, P);
}

void format_chapter_tp(FILE *F, weave_target *wv, chapter *C) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) return plain_chapter_tp(F, wv, C);
	if (wf == tex_format) return tex_chapter_tp(F, wv, C);
	if (html_like(wf)) return html_chapter_tp(F, wv, C);
}

void format_paragraph_heading(FILE *F, weave_target *wv, char *TeX_macro,
	section *S, paragraph *P, char *heading_text, char *chaptermark, char *sectionmark,
	int weight) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format)
		return plain_paragraph_heading(F, wv, TeX_macro, S, P, heading_text, chaptermark, sectionmark, weight);
	if (wf == tex_format)
		return tex_paragraph_heading(F, wv, TeX_macro, S, P, heading_text, chaptermark, sectionmark, weight);
	if (html_like(wf))
		return html_paragraph_heading(F, wv, TeX_macro, S, P, heading_text, chaptermark, sectionmark, weight);
}

void format_source_code(FILE *F, weave_target *wv,
	int tab_stops_of_indentation, 
	char *prefatory, char *matter, char *colouring, char *concluding_comment,
	int starts, int finishes, int code_mode) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) return plain_source_code(F, wv, tab_stops_of_indentation,
		prefatory, matter, colouring, concluding_comment, starts, finishes, code_mode);
	if (wf == tex_format) return tex_source_code(F, wv, tab_stops_of_indentation,
		prefatory, matter, colouring, concluding_comment, starts, finishes, code_mode);
	if (html_like(wf)) return html_source_code(F, wv, tab_stops_of_indentation,
		prefatory, matter, colouring, concluding_comment, starts, finishes, code_mode);
}

void format_source_fragment(FILE *F, weave_target *wv, char *fragment) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) plain_inline_code(F, wv, TRUE);
	if (wf == tex_format) tex_inline_code(F, wv, TRUE);
	if (html_like(wf)) html_inline_code(F, wv, TRUE);
	string colouring;
	for (int i=0; fragment[i]; i++) colouring[i] = PLAIN_CODE;
	colouring[in_strlen(fragment)] = 0;
	format_source_code(F, wv, 0, "", fragment, colouring, "", FALSE, FALSE, TRUE);
	if (wf == plain_text_format) plain_inline_code(F, wv, FALSE);
	if (wf == tex_format) tex_inline_code(F, wv, FALSE);
	if (html_like(wf)) html_inline_code(F, wv, FALSE);
}

void format_comment_lines(FILE *F, weave_target *wv, source_line *from, source_line *to) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) return plain_comment_lines(F, wv, from, to);
	if (wf == tex_format) return tex_comment_lines(F, wv, from, to);
	if (html_like(wf)) return html_comment_lines(F, wv, from, to);
}

void format_display_line(FILE *F, weave_target *wv, char *from) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) return plain_display_line(F, wv, from);
	if (wf == tex_format) return tex_display_line(F, wv, from);
	if (html_like(wf)) return html_display_line(F, wv, from);
}

void format_item(FILE *F, weave_target *wv, int depth, char *label) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) return plain_item(F, wv, depth, label);
	if (wf == tex_format) return tex_item(F, wv, depth, label);
	if (html_like(wf)) return html_item(F, wv, depth, label);
}

void format_bar(FILE *F, weave_target *wv) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) return plain_bar(F, wv);
	if (wf == tex_format) return tex_bar(F, wv);
	if (html_like(wf)) return html_bar(F, wv);
}

void format_figure(FILE *F, weave_target *wv, char *figname, int cm) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) return plain_figure(F, wv, figname, cm);
	if (wf == tex_format) return tex_figure(F, wv, figname, cm);
	if (html_like(wf)) return html_figure(F, wv, figname, cm);
}

void format_cweb_macro(char *matter, weave_target *wv, cweb_macro *cwm, int defn) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) return plain_cweb_macro(matter, wv, cwm, defn);
	if (wf == tex_format) return tex_cweb_macro(matter, wv, cwm, defn);
	if (html_like(wf)) return html_cweb_macro(matter, wv, cwm, defn);
}

void format_pagebreak(FILE *F, weave_target *wv) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) return plain_pagebreak(F, wv);
	if (wf == tex_format) return tex_pagebreak(F, wv);
	if (html_like(wf)) return html_pagebreak(F, wv);
}

void format_blank_line(FILE *F, weave_target *wv, int in_comment) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) return plain_blank_line(F, wv, in_comment);
	if (wf == tex_format) return tex_blank_line(F, wv, in_comment);
	if (html_like(wf)) return html_blank_line(F, wv, in_comment);
}

void format_code_note(FILE *F, weave_target *wv, char *comment) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) return plain_code_note(F, wv, comment);
	if (wf == tex_format) return tex_code_note(F, wv, comment);
	if (html_like(wf)) return html_code_note(F, wv, comment);
}

void format_change_mode(FILE *F, weave_target *wv, int old_mode, int new_mode, int content) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) return plain_change_mode(F, wv, old_mode, new_mode, content);
	if (wf == tex_format) return tex_change_mode(F, wv, old_mode, new_mode, content);
	if (html_like(wf)) return html_change_mode(F, wv, old_mode, new_mode, content);
}

@

@d MACRO_CODE 1
@d FUNCTION_CODE 2
@d RESERVED_CODE 3
@d ELEMENT_CODE 4
@d IDENTIFIER_CODE 5
@d CHAR_LITERAL_CODE 6
@d CONSTANT_CODE 7
@d STRING_CODE 8
@d PLAIN_CODE 9

@c
void format_change_colour(weave_target *wv, char *slot, int col, int in_code) {
	in_strcpy(slot, "");
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) return plain_change_colour(wv, slot, col, in_code);
	if (wf == tex_format) return tex_change_colour(wv, slot, col, in_code);
	if (html_like(wf)) return html_change_colour(wv, slot, col, in_code);
}

void format_endnote(FILE *F, weave_target *wv, int end) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) return plain_endnote(F, wv, end);
	if (wf == tex_format) return tex_endnote(F, wv, end);
	if (html_like(wf)) return html_endnote(F, wv, end);
}

void format_identifier(FILE *F, weave_target *wv, char *id) {
	format_identifier_r(F, wv, id, FALSE);
}

void format_identifier_r(FILE *F, weave_target *wv, char *id, int within) {
	for (int i=0; id[i]; i++) {
		if (id[i] == '\\') i++;
		else if (id[i] == '|') {
			string before; in_strcpy(before, id); before[i] = 0;
			string after; in_strcpy(after, id+i+1);
			format_identifier_r(F, wv, before, within);
			format_identifier_r(F, wv, after, (within)?FALSE:TRUE);
			return;
		}
	}
	if (within) {
		format_source_fragment(F, wv, id);
	} else {
		weave_format *wf = wv->format->intermediate;
		if (wf == NULL) wf = wv->format;
		if (wf == plain_text_format) return plain_identifier(F, wv, id);
		if (wf == tex_format) return tex_identifier(F, wv, id);
		if (html_like(wf)) return html_identifier(F, wv, id);
	}
}

void format_locale(FILE *F, weave_target *wv, paragraph *par1, paragraph *par2) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) return plain_locale(F, wv, par1, par2);
	if (wf == tex_format) return tex_locale(F, wv, par1, par2);
	if (html_like(wf)) return html_locale(F, wv, par1, par2);
}

void format_tail(FILE *F, weave_target *wv, char *comment) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) return plain_tail(F, wv, comment);
	if (wf == tex_format) return tex_tail(F, wv, comment);
	if (html_like(wf)) return html_tail(F, wv, comment);
}

@p Post-processing.

@c
void post_process_weave(weave_target *wv, int open_afterwards) {
	if (wv->format->intermediate == tex_format)
		tex_post_process_weave(wv, open_afterwards);
}

void report_on_post_processing(weave_target *wv) {
	if (wv->format->intermediate == tex_format)
		tex_report_on_post_processing(wv);
}

int substitute_post_processing_data(char *to, weave_target *wv, char *detail) {
	if (wv->format->intermediate == tex_format)
		return tex_substitute_post_processing_data(to, wv, detail);
	return FALSE;
}

int index_pdfs(char *format) {
	if (parse_format(format) == pdf_format) return TRUE;
	return FALSE;
}
