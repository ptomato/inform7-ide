[Formats::] Weave Formats.

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

void Formats::create_weave_formats(void) {
	plain_text_format = Formats::create_weave_format("plain", ".txt", NULL);
	tex_format = Formats::create_weave_format("TeX", ".tex", NULL);
	dvi_format = Formats::create_weave_format("DVI", ".dvi", tex_format);
	pdf_format = Formats::create_weave_format("PDF", ".pdf", tex_format);
	html_format = Formats::create_weave_format("HTML", ".html", NULL);
	epub_format = Formats::create_weave_format("ePub", ".epub", html_format);
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
weave_format *Formats::create_weave_format(char *name, char *ext, weave_format *intermediate) {
	weave_format *wf = CREATE(weave_format);
	wf->format_name = name;
	wf->woven_extension = ext;
	wf->intermediate = intermediate;
	return wf;
}

@ They are identified by name.

@c
weave_format *Formats::parse_format(char *name) {
	weave_format *wf;
	LOOP_OVER(wf, weave_format)
		if (CStrings::eq_insensitive(name, wf->format_name))
			return wf;
	return NULL;
}

int Formats::html_like(weave_format *wf) {
	if (wf->intermediate) wf = wf->intermediate;
	if ((wf == html_format) || (wf == epub_format)) return TRUE;
	return FALSE;
}

char *Formats::weave_file_extension(weave_format *wf) {
	if (wf->intermediate) wf = wf->intermediate;
	return wf->woven_extension;
}

@p Before and after.

@c
int Formats::begin_weaving(web *W, char *format) {
	if (Formats::parse_format(format) == epub_format) {
		TEMPORARY_TEXT(T)
		WRITE_TO(T, "%s", Bibliographic::get_data(W, "Title"));
		W->as_ebook = Epub::new(T);
		filename *CSS = Filenames::in_folder(path_to_inweb_materials, "inweb.css");
		Epub::use_CSS(W->as_ebook, CSS);
		Epub::attach_metadata(W->as_ebook, L"identifier", T);
		pathname *P = Pathnames::subfolder(W->path_to_web, "Woven");
		W->redirect_weaves_to = Epub::begin_construction(W->as_ebook, P, NULL);
		Shell::copy(CSS, W->redirect_weaves_to, "");
		return SWARM_SECTIONS;
	}
	return SWARM_OFF;
}

void Formats::end_weaving(web *W, char *format) {
	if (Formats::parse_format(format) == epub_format) {
		Epub::end_construction(W->as_ebook);
	}
}

@p During the weave.

@c
void Formats::top(OUTPUT_STREAM, weave_target *wv, char *comment) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) { PlainText::top(OUT, wv, comment); return; }
	if (wf == tex_format) { TeX::top(OUT, wv, comment); return; }
	if (Formats::html_like(wf)) { HTMLFormat::top(OUT, wv, comment); return; }
}

void Formats::subheading(OUTPUT_STREAM, weave_target *wv, int level, char *comment, char *head) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) { PlainText::subheading(OUT, wv, level, comment, head); return; }
	if (wf == tex_format) { TeX::subheading(OUT, wv, level, comment, head); return; }
	if (Formats::html_like(wf)) { HTMLFormat::subheading(OUT, wv, level, comment, head); return; }
}

void Formats::toc(OUTPUT_STREAM, weave_target *wv, int stage, char *text1, char *text2, paragraph *P) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) { PlainText::toc(OUT, wv, stage, text1, text2, P); return; }
	if (wf == tex_format) { TeX::toc(OUT, wv, stage, text1, text2, P); return; }
	if (Formats::html_like(wf)) { HTMLFormat::toc(OUT, wv, stage, text1, text2, P); return; }
}

void Formats::chapter_tp(OUTPUT_STREAM, weave_target *wv, chapter *C) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) { PlainText::chapter_tp(OUT, wv, C); return; }
	if (wf == tex_format) { TeX::chapter_tp(OUT, wv, C); return; }
	if (Formats::html_like(wf)) { HTMLFormat::chapter_tp(OUT, wv, C); return; }
}

void Formats::paragraph_heading(OUTPUT_STREAM, weave_target *wv, char *TeX_macro,
	section *S, paragraph *P, char *heading_text, char *chaptermark, char *sectionmark,
	int weight) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format)
		{ PlainText::paragraph_heading(OUT, wv, TeX_macro, S, P, heading_text, chaptermark, sectionmark, weight); return; }
	if (wf == tex_format)
		{ TeX::paragraph_heading(OUT, wv, TeX_macro, S, P, heading_text, chaptermark, sectionmark, weight); return; }
	if (Formats::html_like(wf))
		{ HTMLFormat::paragraph_heading(OUT, wv, TeX_macro, S, P, heading_text, chaptermark, sectionmark, weight); return; }
}

void Formats::source_code(OUTPUT_STREAM, weave_target *wv,
	int tab_stops_of_indentation, 
	char *prefatory, char *matter, char *colouring, char *concluding_comment,
	int starts, int finishes, int code_mode) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) { PlainText::source_code(OUT, wv, tab_stops_of_indentation,
		prefatory, matter, colouring, concluding_comment, starts, finishes, code_mode); return; }
	if (wf == tex_format) { TeX::source_code(OUT, wv, tab_stops_of_indentation,
		prefatory, matter, colouring, concluding_comment, starts, finishes, code_mode); return; }
	if (Formats::html_like(wf)) { HTMLFormat::source_code(OUT, wv, tab_stops_of_indentation,
		prefatory, matter, colouring, concluding_comment, starts, finishes, code_mode); return; }
}

void Formats::source_fragment(OUTPUT_STREAM, weave_target *wv, char *fragment) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) PlainText::inline_code(OUT, wv, TRUE);
	if (wf == tex_format) TeX::inline_code(OUT, wv, TRUE);
	if (Formats::html_like(wf)) HTMLFormat::inline_code(OUT, wv, TRUE);
	string colouring;
	for (int i=0; fragment[i]; i++) colouring[i] = PLAIN_CODE;
	colouring[CStrings::len(fragment)] = 0;
	Formats::source_code(OUT, wv, 0, "", fragment, colouring, "", FALSE, FALSE, TRUE);
	if (wf == plain_text_format) PlainText::inline_code(OUT, wv, FALSE);
	if (wf == tex_format) TeX::inline_code(OUT, wv, FALSE);
	if (Formats::html_like(wf)) HTMLFormat::inline_code(OUT, wv, FALSE);
}

void Formats::comment_lines(OUTPUT_STREAM, weave_target *wv, source_line *from, source_line *to) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) { PlainText::comment_lines(OUT, wv, from, to); return; }
	if (wf == tex_format) { TeX::comment_lines(OUT, wv, from, to); return; }
	if (Formats::html_like(wf)) { HTMLFormat::comment_lines(OUT, wv, from, to); return; }
}

void Formats::display_line(OUTPUT_STREAM, weave_target *wv, char *from) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) { PlainText::display_line(OUT, wv, from); return; }
	if (wf == tex_format) { TeX::display_line(OUT, wv, from); return; }
	if (Formats::html_like(wf)) { HTMLFormat::display_line(OUT, wv, from); return; }
}

void Formats::item(OUTPUT_STREAM, weave_target *wv, int depth, char *label) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) { PlainText::item(OUT, wv, depth, label); return; }
	if (wf == tex_format) { TeX::item(OUT, wv, depth, label); return; }
	if (Formats::html_like(wf)) { HTMLFormat::item(OUT, wv, depth, label); return; }
}

void Formats::bar(OUTPUT_STREAM, weave_target *wv) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) { PlainText::bar(OUT, wv); return; }
	if (wf == tex_format) { TeX::bar(OUT, wv); return; }
	if (Formats::html_like(wf)) { HTMLFormat::bar(OUT, wv); return; }
}

void Formats::figure(OUTPUT_STREAM, weave_target *wv, char *figname, int cm) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) { PlainText::figure(OUT, wv, figname, cm); return; }
	if (wf == tex_format) { TeX::figure(OUT, wv, figname, cm); return; }
	if (Formats::html_like(wf)) { HTMLFormat::figure(OUT, wv, figname, cm); return; }
}

void Formats::cweb_macro(OUTPUT_STREAM, weave_target *wv, cweb_macro *cwm, int defn) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) { PlainText::cweb_macro(OUT, wv, cwm, defn); return; }
	if (wf == tex_format) { TeX::cweb_macro(OUT, wv, cwm, defn); return; }
	if (Formats::html_like(wf)) { HTMLFormat::cweb_macro(OUT, wv, cwm, defn); return; }
}

void Formats::pagebreak(OUTPUT_STREAM, weave_target *wv) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) { PlainText::pagebreak(OUT, wv); return; }
	if (wf == tex_format) { TeX::pagebreak(OUT, wv); return; }
	if (Formats::html_like(wf)) { HTMLFormat::pagebreak(OUT, wv); return; }
}

void Formats::blank_line(OUTPUT_STREAM, weave_target *wv, int in_comment) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) { PlainText::blank_line(OUT, wv, in_comment); return; }
	if (wf == tex_format) { TeX::blank_line(OUT, wv, in_comment); return; }
	if (Formats::html_like(wf)) { HTMLFormat::blank_line(OUT, wv, in_comment); return; }
}

void Formats::code_note(OUTPUT_STREAM, weave_target *wv, char *comment) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) { PlainText::code_note(OUT, wv, comment); return; }
	if (wf == tex_format) { TeX::code_note(OUT, wv, comment); return; }
	if (Formats::html_like(wf)) { HTMLFormat::code_note(OUT, wv, comment); return; }
}

void Formats::change_mode(OUTPUT_STREAM, weave_target *wv, int old_mode, int new_mode, int content) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) { PlainText::change_mode(OUT, wv, old_mode, new_mode, content); return; }
	if (wf == tex_format) { TeX::change_mode(OUT, wv, old_mode, new_mode, content); return; }
	if (Formats::html_like(wf)) { HTMLFormat::change_mode(OUT, wv, old_mode, new_mode, content); return; }
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
void Formats::change_colour(OUTPUT_STREAM, weave_target *wv, int col, int in_code) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) { PlainText::change_colour(OUT, wv, col, in_code); return; }
	if (wf == tex_format) { TeX::change_colour(OUT, wv, col, in_code); return; }
	if (Formats::html_like(wf)) { HTMLFormat::change_colour(OUT, wv, col, in_code); return; }
}

void Formats::endnote(OUTPUT_STREAM, weave_target *wv, int end) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) { PlainText::endnote(OUT, wv, end); return; }
	if (wf == tex_format) { TeX::endnote(OUT, wv, end); return; }
	if (Formats::html_like(wf)) { HTMLFormat::endnote(OUT, wv, end); return; }
}

void Formats::identifier(OUTPUT_STREAM, weave_target *wv, char *id) {
	Formats::identifier_r(OUT, wv, id, FALSE);
}

void Formats::identifier_r(OUTPUT_STREAM, weave_target *wv, char *id, int within) {
	for (int i=0; id[i]; i++) {
		if (id[i] == '\\') i++;
		else if (id[i] == '|') {
			string before; CStrings::copy(before, id); before[i] = 0;
			string after; CStrings::copy(after, id+i+1);
			Formats::identifier_r(OUT, wv, before, within);
			Formats::identifier_r(OUT, wv, after, (within)?FALSE:TRUE);
			return;
		}
	}
	if (within) {
		Formats::source_fragment(OUT, wv, id);
	} else {
		weave_format *wf = wv->format->intermediate;
		if (wf == NULL) wf = wv->format;
		if (wf == plain_text_format) { PlainText::identifier(OUT, wv, id); return; }
		if (wf == tex_format) { TeX::identifier(OUT, wv, id); return; }
		if (Formats::html_like(wf)) { HTMLFormat::identifier(OUT, wv, id); return; }
	}
}

void Formats::locale(OUTPUT_STREAM, weave_target *wv, paragraph *par1, paragraph *par2) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) { PlainText::locale(OUT, wv, par1, par2); return; }
	if (wf == tex_format) { TeX::locale(OUT, wv, par1, par2); return; }
	if (Formats::html_like(wf)) { HTMLFormat::locale(OUT, wv, par1, par2); return; }
}

void Formats::tail(OUTPUT_STREAM, weave_target *wv, char *comment) {
	weave_format *wf = wv->format->intermediate;
	if (wf == NULL) wf = wv->format;
	if (wf == plain_text_format) { PlainText::tail(OUT, wv, comment); return; }
	if (wf == tex_format) { TeX::tail(OUT, wv, comment); return; }
	if (Formats::html_like(wf)) { HTMLFormat::tail(OUT, wv, comment); return; }
}

@p Post-processing.

@c
void Formats::post_process_weave(weave_target *wv, int open_afterwards) {
	if (wv->format->intermediate == tex_format)
		RunningTeX::post_process_weave(wv, open_afterwards);
}

void Formats::report_on_post_processing(weave_target *wv) {
	if (wv->format->intermediate == tex_format)
		Formats::report_on_post_processing(wv);
}

int Formats::substitute_post_processing_data(char *to, weave_target *wv, char *detail) {
	if (wv->format->intermediate == tex_format)
		return RunningTeX::substitute_post_processing_data(to, wv, detail);
	return FALSE;
}

int Formats::index_pdfs(char *format) {
	if (Formats::parse_format(format) == pdf_format) return TRUE;
	return FALSE;
}
