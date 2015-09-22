[HTMLFormat::] HTML Formats.

@Purpose: To provide for weaving in HTML or other similar formats, such as
EPUB books.

@p Current state.
To keep track of what we're writing, across many intermittent calls to the
routines below, we store a crude sort of state in two global variables.
(This isn't thread-safe and means we can only write one file at a time,
but in fact that's fine here.)

@d HTML_OUT 0 /* write position in HTML file is currently outside of p, pre, li */
@d HTML_IN_P 1 /* write position in HTML file is currently outside p */
@d HTML_IN_PRE 2 /* write position in HTML file is currently outside pre */
@d HTML_IN_LI 3 /* write position in HTML file is currently outside li */

@c
int html_in_para = HTML_OUT; /* one of the above */
int item_depth = 0; /* for |HTML_IN_LI| only: how many lists we're nested inside */

void HTMLFormat::p(OUTPUT_STREAM, char *class) {
	HTML::open_with_class(OUT, "p", class);
	html_in_para = HTML_IN_P;
}

void HTMLFormat::cp(OUTPUT_STREAM) {
	HTML::close(OUT, "p"); WRITE("\n");
	html_in_para = HTML_OUT;
}

void HTMLFormat::pre(OUTPUT_STREAM, char *class) {
	HTML::open_with_class(OUT, "pre", class); WRITE("\n"); INDENT;
	html_in_para = HTML_IN_PRE;
}

void HTMLFormat::cpre(OUTPUT_STREAM) {
	OUTDENT; HTML::close(OUT, "pre"); WRITE("\n");
	html_in_para = HTML_OUT;
}

@ Depth 1 means "inside a list entry"; depth 2, "inside an entry of a list
which is itself inside a list entry"; and so on.

@c
void HTMLFormat::go_to_depth(OUTPUT_STREAM, int depth) {
	if (html_in_para != HTML_IN_LI) HTMLFormat::exit_current_paragraph(OUT);
	if (item_depth == depth) {
		HTML::close(OUT, "li");
	} else {
		while (item_depth < depth) {
			HTML::open_with_class(OUT, "ul", "items"); item_depth++;
		}
		while (item_depth > depth) {
			HTML::close(OUT, "li");
			HTML::close(OUT, "ul");
			WRITE("\n"); item_depth--;
		}
	}
	if (depth > 0) {
		HTML::open_with_class(OUT, "li", "NULL");
		html_in_para = HTML_IN_LI;
	} else {
		html_in_para = HTML_OUT;
	}
}

@ The following generically gets us out of whatever we're currently into:

@c
void HTMLFormat::exit_current_paragraph(OUTPUT_STREAM) {
	switch (html_in_para) {
		case HTML_IN_P: HTMLFormat::cp(OUT); break;
		case HTML_IN_PRE: HTMLFormat::cpre(OUT); break;
		case HTML_IN_LI: HTMLFormat::go_to_depth(OUT, 0); break;
	}
}

@p Format routines.

@c
section *page_section = NULL;

void HTMLFormat::top(OUTPUT_STREAM, weave_target *wv, char *comment) {
	if (wv->format == epub_format) 
		HTML::declare_as_HTML(OUT, TRUE);
	else
		HTML::declare_as_HTML(OUT, FALSE);
	if (wv->format == epub_format)
		Epub::note_page(wv->weave_web->as_ebook, wv->weave_to, wv->booklet_title, "");
	filename *T = NULL;
	if (wv->format == epub_format) 
		T = Filenames::in_folder(path_to_inweb_materials, "epub-template");
	else
		T = Filenames::in_folder(path_to_inweb_materials, "template");
	Bibliographic::set_datum(wv->weave_web, "Booklet Title", wv->booklet_title);
	Weaver::weave_cover_from(OUT, wv->weave_web, T, wv, WEAVE_FIRST_HALF);
	filename *CSS = Filenames::in_folder(path_to_inweb_materials, "inweb.css");
	if (wv->format != epub_format)
		Shell::copy(CSS, Pathnames::subfolder(wv->weave_web->path_to_web, "Woven"), "");
	HTML::comment(OUT, comment);
	html_in_para = HTML_OUT;
}

@

@c
void HTMLFormat::subheading(OUTPUT_STREAM, weave_target *wv, int level, char *comment, char *head) {
	HTMLFormat::exit_current_paragraph(OUT);
	switch (level) {
		case 1: HTML::heading(OUT, "h3", comment); break;
		case 2: HTMLFormat::p(OUT, "purpose");
			WRITE("%s: ", comment);
			if (head) Formats::identifier(OUT, wv, head);
			HTMLFormat::cp(OUT);
			break;
	}
}

@

@c
void HTMLFormat::toc(OUTPUT_STREAM, weave_target *wv, int stage, char *text1, char *text2, paragraph *P) {
	HTMLFormat::exit_current_paragraph(OUT);
	switch (stage) {
		case 1:
			HTML::open_with_class(OUT, "ul", "toc");
			HTML::open(OUT, "li");
			break;
		case 2:
			HTML::close(OUT, "li");
			HTML::open(OUT, "li");
			break;
		case 3: {
			TEMPORARY_STREAM
			HTMLFormat::xref(TEMP, wv, P, NULL, TRUE);
			HTML::begin_link(OUT, TEMP);
			CLOSE_TEMPORARY_STREAM		
			WRITE("%s%s", (*(P->ornament) == 'S')?"&#167;":"&para;", P->paragraph_number);
			WRITE(". %s", text2);
			HTML::end_link(OUT);
			break;
		}
		case 4:
			HTML::close(OUT, "li");
			HTML::close(OUT, "ul");
			HTML::hr(OUT, "tocbar");
			WRITE("\n"); break;
	}
}

@

@c
void HTMLFormat::chapter_tp(OUTPUT_STREAM, weave_target *wv, chapter *C) {
	return;
}

@

@c
int crumbs_dropped = FALSE;

void HTMLFormat::paragraph_heading(OUTPUT_STREAM, weave_target *wv, char *TeX_macro,
	section *S, paragraph *P, char *heading_text, char *chaptermark, char *sectionmark,
	int weight) {
	page_section = S;
	if (weight == 3) return; /* Skip chapter headings */
	HTMLFormat::exit_current_paragraph(OUT);
	if (P) {
		HTMLFormat::p(OUT, "inwebparagraph");
		TEMPORARY_STREAM
		HTMLFormat::xref(TEMP, wv, P, NULL, FALSE);
		HTML::anchor(OUT, TEMP);
		CLOSE_TEMPORARY_STREAM		
		HTML::open(OUT, "b");
		WRITE("%s%s", (*(P->ornament) == 'S')?"&#167;":"&para;", P->paragraph_number);
		WRITE(". %s%s ", heading_text, (*heading_text)?".":"");
		HTML::close(OUT, "b");
	} else {
		if (crumbs_dropped == FALSE) {
			filename *C = Filenames::in_folder(path_to_inweb_materials, "crumbs.gif");
			Swarm::copy_file_into_weave(wv->weave_web, C);
			crumbs_dropped = TRUE;
		}
		HTML::open_with_class(OUT, "ul", "crumbs");
		HTMLFormat::breadcrumb(OUT, Bibliographic::get_data(wv->weave_web, "Title"), "index.html");
		string chapter_link;
		CSTRING_WRITE(chapter_link, "index.html#%s%s", (wv->weave_web->as_ebook)?"C":"", S->owning_chapter->ch_sigil);
		HTMLFormat::breadcrumb(OUT, S->owning_chapter->ch_title, chapter_link);
		HTMLFormat::breadcrumb(OUT, heading_text, NULL);
		HTML::close(OUT, "ul");
	}
}

void HTMLFormat::breadcrumb(OUTPUT_STREAM, char *text, char *link) {
	if (link) {
		HTML::open(OUT, "li");
		STRING(L);
		Str::copy_ISO_string(L, link);
		HTML::begin_link(OUT, L);
		WRITE("%s", text);
		HTML::end_link(OUT);
		HTML::close(OUT, "li");
	} else {
		HTML::open(OUT, "li");
		HTML::open(OUT, "b");
		WRITE("%s", text);
		HTML::close(OUT, "b");
		HTML::close(OUT, "li");
	}
}

@

@c
void HTMLFormat::source_code(OUTPUT_STREAM, weave_target *wv, int tab_stops_of_indentation, 
	char *prefatory, char *matter, char *colouring, char *concluding_comment,
	int starts, int finishes, int code_mode) {
	if (starts) {
		for (int i=0; i<tab_stops_of_indentation; i++)
			WRITE("    ");
		if (prefatory[0]) {
			HTML::open(OUT, "strong");
			WRITE("%s", prefatory);
			HTML::close(OUT, "strong");
			WRITE(" ");
		}
	}
	int current_colour = -1, colour_wanted = PLAIN_CODE;
	for (int i=0; matter[i]; i++) {
		colour_wanted = colouring[i]; @<Adjust code colour as necessary@>;
		if (matter[i] == '<') WRITE("&lt;");
		else if (matter[i] == '>') WRITE("&gt;");
		else if (matter[i] == '&') WRITE("&amp;");
		else WRITE("%c", matter[i]);
	}
	if (current_colour >= 0) HTML::close(OUT, "span");
	current_colour = -1;
	if (finishes) {		
		if (concluding_comment[0]) {
			if ((CStrings::ne(matter, "")) || (!starts)) WRITE("    ");
			HTML::open_with_class(OUT, "span", "comment");
			Formats::identifier(OUT, wv, concluding_comment);
			HTML::close(OUT, "span");
		}
		WRITE("\n");
	}
}

@

@<Adjust code colour as necessary@> =
	if (colour_wanted != current_colour) {
		if (current_colour >= 0) HTML::close(OUT, "span");
		Formats::change_colour(OUT, wv, colour_wanted, TRUE);
		current_colour = colour_wanted;
	}

@

@c
void HTMLFormat::inline_code(OUTPUT_STREAM, weave_target *wv, int enter) {
	if (enter) {
		if (html_in_para == HTML_OUT) HTMLFormat::p(OUT, "inwebparagraph");
		HTML::open_with_class(OUT, "code", "display");
	} else {
		HTML::close(OUT, "code");
	}
}

@

@c
void HTMLFormat::comment_lines(OUTPUT_STREAM, weave_target *wv, source_line *from, source_line *to) {
}

@

@c
void HTMLFormat::display_line(OUTPUT_STREAM, weave_target *wv, char *from) {
	HTMLFormat::exit_current_paragraph(OUT);
	HTML::open(OUT, "blockquote"); WRITE("\n"); INDENT;
	HTMLFormat::p(OUT, NULL);
	WRITE("%s", from);
	HTMLFormat::cp(OUT);
	OUTDENT; HTML::close(OUT, "blockquote"); WRITE("\n");
}

@

@c
void HTMLFormat::item(OUTPUT_STREAM, weave_target *wv, int depth, char *label) {
	HTMLFormat::go_to_depth(OUT, depth);
	if (*label) WRITE("(%s) ", label);
	else WRITE(" ");
	
}

@

@c
void HTMLFormat::bar(OUTPUT_STREAM, weave_target *wv) {
	HTMLFormat::exit_current_paragraph(OUT);
	HTML::hr(OUT, NULL);
}

@

@c
void HTMLFormat::figure(OUTPUT_STREAM, weave_target *wv, char *figname, int cm) {	
	HTMLFormat::exit_current_paragraph(OUT);
	HTML::open(OUT, "center");
	pathname *P = Pathnames::from_string("Figures");
	HTML::image(OUT, Filenames::in_folder(P, figname));
	HTML::close(OUT, "center");
	WRITE("\n");
}

@

@c
void HTMLFormat::cweb_macro(OUTPUT_STREAM, weave_target *wv, cweb_macro *cwm, int defn) {
	paragraph *P = cwm->defining_paragraph;
	WRITE("&lt;");
	HTML::open_with_class(OUT, "span", (defn)?"cwebmacrodefn":"cwebmacro");
	WRITE("%s", cwm->macro_name);
	HTML::close(OUT, "span");
	WRITE(" ");
	HTML::open_with_class(OUT, "span", "cwebmacronumber");
	WRITE("%s", P->paragraph_number);
	HTML::close(OUT, "span");
	WRITE("&gt;%s", (defn)?" =":"");
}

@

@c
void HTMLFormat::pagebreak(OUTPUT_STREAM, weave_target *wv) {
	HTMLFormat::exit_current_paragraph(OUT);
}

@

@c
void HTMLFormat::blank_line(OUTPUT_STREAM, weave_target *wv, int in_comment) {
	if (html_in_para == HTML_IN_PRE) {
		WRITE("\n");
	} else {
		int old_state = html_in_para, old_depth = item_depth;
		HTMLFormat::exit_current_paragraph(OUT);
		if ((old_state == HTML_IN_P) || ((old_state == HTML_IN_LI) && (old_depth > 1)))
			HTMLFormat::p(OUT,"inwebparagraph");
	}
}

@

@c
void HTMLFormat::code_note(OUTPUT_STREAM, weave_target *wv, char *comment) {
	HTMLFormat::exit_current_paragraph(OUT);
	HTMLFormat::p(OUT, NULL);
	WRITE("%s", comment);
	HTMLFormat::cp(OUT);
}

@

@c
void HTMLFormat::change_mode(OUTPUT_STREAM, weave_target *wv, int old_mode, int new_mode, int content) {
	// WRITE("(CM%d:%d->%d)", content, old_mode, new_mode);
	if (old_mode != new_mode) {
		if (old_mode == MACRO_MATERIAL) HTML::close(OUT, "code");
		if ((content) || (new_mode != MACRO_MATERIAL)) HTMLFormat::exit_current_paragraph(OUT);
		switch (old_mode) {
			case CODE_MATERIAL:
			case REGULAR_MATERIAL:
				switch (new_mode) {
					case CODE_MATERIAL:
						WRITE("\n");
						HTMLFormat::pre(OUT, "display");
						break;
					case DEFINITION_MATERIAL:
						WRITE("\n");
						HTMLFormat::pre(OUT, "definitions");
						break;
					case MACRO_MATERIAL:
						if (content) {
							WRITE("\n");
							HTMLFormat::p(OUT,"macrodefinition");
						}
						HTML::open_with_class(OUT, "code", "display");
						WRITE("\n");
						break;
					case REGULAR_MATERIAL:
						if (content) {
							WRITE("\n");
							HTMLFormat::p(OUT,"inwebparagraph");
						}
						break;
				}
				break;
			case MACRO_MATERIAL:
				switch (new_mode) {
					case CODE_MATERIAL:
						WRITE("\n");
						HTMLFormat::pre(OUT, "displaydefn");
						break;
					case DEFINITION_MATERIAL:
						WRITE("\n");
						HTMLFormat::pre(OUT, "definitions");
						break;
				}
				break;
			case DEFINITION_MATERIAL:
				switch (new_mode) {
					case CODE_MATERIAL:
						WRITE("\n");
						HTMLFormat::pre(OUT, "display");
						break;
					case MACRO_MATERIAL:
						WRITE("\n");
						HTMLFormat::p(OUT, "macrodefinition");
						HTML::open_with_class(OUT, "code", "display");
						WRITE("\n");
						break;
				}
				break;
			default:
				HTMLFormat::cpre(OUT);
				break;	
		}
	}
}

@

@c
void HTMLFormat::change_colour(OUTPUT_STREAM, weave_target *wv, int col, int in_code) {
	char *cl = "plain";
	switch (col) {
		case MACRO_CODE: 		cl = "cwebmacrotext"; break;
		case FUNCTION_CODE: 	cl = "functiontext"; break;
		case IDENTIFIER_CODE: 	cl = "identifier"; break;
		case ELEMENT_CODE:		cl = "element"; break;
		case RESERVED_CODE: 	cl = "reserved"; break;
		case STRING_CODE: 		cl = "string"; break;
		case CHAR_LITERAL_CODE: cl = "character"; break;
		case CONSTANT_CODE: 	cl = "constant"; break;
		case PLAIN_CODE: 		cl = "plain"; break;
		default: printf("col: %d\n", col); internal_error("bad colour"); break;
	}
	HTML::open_with_class(OUT, "span", cl);
}

@

@c
void HTMLFormat::endnote(OUTPUT_STREAM, weave_target *wv, int end) {
	if (end == 1) {
		HTMLFormat::exit_current_paragraph(OUT);
		HTMLFormat::p(OUT, "endnote");
	} else {
		HTMLFormat::cp(OUT);
	}
}

@

@c
void HTMLFormat::identifier(OUTPUT_STREAM, weave_target *wv, char *id) {
	for (int i=0; id[i]; i++) {
		if (html_in_para == HTML_OUT) HTMLFormat::p(OUT, "inwebparagraph");
		if (id[i] == '&') WRITE("&amp;");
		else if (id[i] == '<') WRITE("&lt;");
		else if (id[i] == '>') WRITE("&gt;");
		else if ((id[i] == ' ') && (id[i+1] == '-') && (id[i+2] == '-') &&
			((id[i+3] == ' ') || (id[i+3] == 0))) {
			WRITE(" &mdash;"); i+=2;
		} else WRITE("%c", id[i]);
	}
}

@

@c
void HTMLFormat::locale(OUTPUT_STREAM, weave_target *wv, paragraph *par1, paragraph *par2) {
	TEMPORARY_STREAM
	HTMLFormat::xref(TEMP, wv, par1, page_section, TRUE);
	HTML::begin_link(OUT, TEMP);
	CLOSE_TEMPORARY_STREAM
	WRITE("%s%s",
		(*(par1->ornament) == 'S')?"&#167;":"&para;",
		par1->paragraph_number);
	if (par2) WRITE("-%s", par2->paragraph_number);
	HTML::end_link(OUT);
}

@

@c
void HTMLFormat::xref(OUTPUT_STREAM, weave_target *wv, paragraph *P, section *from, int a_link) {
	string linkto;
	CStrings::copy(linkto, "");
	if ((from) && (P->under_section != from)) {
		CStrings::copy(linkto, P->under_section->sigil);
		for (int i=0; linkto[i]; i++)
			if ((linkto[i] == '/') || (linkto[i] == ' '))
				CStrings::set_char(linkto, i, '-');
		CStrings::concatenate(linkto, ".html");
	}
	WRITE("%s%s%s", linkto, (a_link)?"#":"", P->ornament);
	char *N = P->paragraph_number;
	for (int i=0; N[i]; i++)
		if (N[i] == '.') WRITE("_");
		else WRITE("%c", N[i]);
}

@

@c
void HTMLFormat::tail(OUTPUT_STREAM, weave_target *wv, char *comment) {
	HTMLFormat::exit_current_paragraph(OUT);
	HTML::comment(OUT, comment);
	HTML::completed(OUT);
	filename *T = Filenames::in_folder(path_to_inweb_materials, "template");
	Bibliographic::set_datum(wv->weave_web, "Booklet Title", wv->booklet_title);
	Weaver::weave_cover_from(OUT, wv->weave_web, T, wv, WEAVE_SECOND_HALF);
}
