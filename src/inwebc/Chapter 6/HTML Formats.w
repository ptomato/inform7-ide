6/html: HTML Formats.

@Purpose: To provide for weaving in HTML or other similar formats, such as
EPUB books.

@

@d HTML_OUT 0
@d HTML_IN_P 1
@d HTML_IN_PRE 2
@d HTML_IN_ITEM 3

@c
int html_in_para = HTML_OUT;
section *page_section = NULL;

void html_top(FILE *F, weave_target *wv, char *comment) {
	string template;
	in_sprintf(template, "%stemplate", path_to_inweb_materials);
	set_bibliographic_data(wv->weave_web, "Booklet Title", wv->booklet_title);
	weave_cover_from(F, wv->weave_web, template, wv, WEAVE_FIRST_HALF);
	string css;
	in_sprintf(css, "%sinweb.css", path_to_inweb_materials);
	issue_os_command_2("cp '%s' '%sWoven'", css, wv->weave_web->path_to_web);

	fprintf(F, "<!--%s-->\n", comment);
	html_in_para = HTML_OUT;
}

@

@<Close any HTML paragraph now open@> =
	if (html_in_para != HTML_OUT) {
		switch (html_in_para) {
			case HTML_IN_P: fprintf(F, "</p>"); break;
			case HTML_IN_PRE: fprintf(F, "</pre>"); break;
			case HTML_IN_ITEM: exit_items(F, 0); break;
		}
		html_in_para = HTML_OUT;
	}

@

@c
void html_subheading(FILE *F, weave_target *wv, int level, char *comment, char *head) {
	@<Close any HTML paragraph now open@>;
	switch (level) {
		case 1: fprintf(F, "<h3>%s</h3>\n", comment); break;
		case 2: fprintf(F, "<p class=\"purpose\">%s: ", comment);
			if (head) format_identifier(F, wv, head);
			fprintf(F, "</p>\n"); break;
	}
}

@

@c
void html_toc(FILE *F, weave_target *wv, int stage, char *text1, char *text2, paragraph *P) {
	@<Close any HTML paragraph now open@>;
	switch (stage) {
		case 1: fprintf(F, "<ul class=\"toc\"><li>"); break;
		case 2: fprintf(F, "</li><li>"); break;
		case 3:
			fprintf(F, "<a href=\"");
			html_xref(F, wv, P, NULL, TRUE);
			fprintf(F, "\">");
			fprintf(F, "%s%s", (*(P->ornament) == 'S')?"&#167;":"&para;", P->paragraph_number);
			fprintf(F, ". %s</a>", text2);
			break;
		case 4: fprintf(F, "</li></ul><hr class=\"tocbar\">\n"); break;
	}
}

@

@c
void html_chapter_tp(FILE *F, weave_target *wv, chapter *C) {
	return;
}

@

@c
int crumbs_dropped = FALSE;

void html_paragraph_heading(FILE *F, weave_target *wv, char *TeX_macro,
	section *S, paragraph *P, char *heading_text, char *chaptermark, char *sectionmark,
	int weight) {
	page_section = S;
	if (weight == 3) return; /* Skip chapter headings */
	@<Close any HTML paragraph now open@>;
	if (P) {
		fprintf(F, "<p class=\"inwebparagraph\">");
		fprintf(F, "<a name=\"");
		html_xref(F, wv, P, NULL, FALSE);
		fprintf(F, "\"><b>");
		fprintf(F, "%s%s", (*(P->ornament) == 'S')?"&#167;":"&para;", P->paragraph_number);
		fprintf(F, ". %s%s</b> ", heading_text, (*heading_text)?".":"");
		html_in_para = HTML_IN_P;
	} else {
		if (crumbs_dropped == FALSE) {
			string crumb;
			in_sprintf(crumb, "%scrumbs.gif", path_to_inweb_materials);
			copy_files_into_weave(wv->weave_web, crumb);
			crumbs_dropped = TRUE;
		}
		fprintf(F, "<ul id=\"crumbs\">");
		breadcrumb(F, get_bibliographic_data(wv->weave_web, "Title"), "index.html");
		string chapter_link;
		in_sprintf(chapter_link, "index.html#%s", S->owning_chapter->ch_sigil);
		breadcrumb(F, S->owning_chapter->ch_title, chapter_link);
		breadcrumb(F, heading_text, NULL);
		fprintf(F, "</ul>\n");
	}
}

void breadcrumb(FILE *F, char *text, char *link) {
	if (link) fprintf(F, "<li><a href=\"%s\">%s</a></li>", link, text);
	else fprintf(F, "<li><b>%s</b></li>", text);
}

@

@c
void html_source_code(FILE *WEAVEOUT, weave_target *wv, int tab_stops_of_indentation, 
	char *prefatory, char *matter, char *colouring, char *concluding_comment,
	int starts, int finishes, int code_mode) {
	if (starts) {
		for (int i=0; i<tab_stops_of_indentation; i++)
			fprintf(WEAVEOUT, "    ");
		if (prefatory[0]) fprintf(WEAVEOUT, "<strong>%s</strong> ", prefatory);
	}
	int current_colour = -1, colour_wanted = PLAIN_CODE;
	for (int i=0; matter[i]; i++) {
		colour_wanted = colouring[i]; @<Adjust code colour as necessary@>;
		if (matter[i] == '<') fprintf(WEAVEOUT, "&lt;");
		else if (matter[i] == '>') fprintf(WEAVEOUT, "&gt;");
		else if (matter[i] == '&') fprintf(WEAVEOUT, "&amp;");
		else fprintf(WEAVEOUT, "%c", matter[i]);
	}
	colour_wanted = PLAIN_CODE; @<Adjust code colour as necessary@>;
	if (finishes) {		
		if (concluding_comment[0]) {
			if ((in_string_ne(matter, "")) || (!starts)) fprintf(WEAVEOUT, "    ");
			fprintf(WEAVEOUT, "<span class=\"comment\">");
			format_identifier(WEAVEOUT, wv, concluding_comment);
			fprintf(WEAVEOUT, "</span>");
		}
		fprintf(WEAVEOUT, "\n");
	}
}

@

@<Adjust code colour as necessary@> =
	if (colour_wanted != current_colour) {
		if (current_colour >= 0) fprintf(WEAVEOUT, "</span>");
		string col;
		format_change_colour(wv, col, colour_wanted, TRUE);
		fprintf(WEAVEOUT, "%s", col);
		current_colour = colour_wanted;
	}

@

@c
void html_inline_code(FILE *F, weave_target *wv, int enter) {
	if (enter) fprintf(F, "<code class=\"display\">");
	else fprintf(F, "</code>");
}

@

@c
void html_comment_lines(FILE *F, weave_target *wv, source_line *from, source_line *to) {
}

@

@c
void html_display_line(FILE *F, weave_target *wv, char *from) {
	@<Close any HTML paragraph now open@>;
	fprintf(F, "<blockquote>%s</blockquote>\n", from);
}

@

@c
int item_depth = 0;
void html_item(FILE *F, weave_target *wv, int depth, char *label) {
	if (html_in_para != HTML_IN_ITEM) {
		if (html_in_para != HTML_OUT) fprintf(F, "</p>");
		item_depth = 0;
	}
	exit_items(F, depth);
		
	if (*label) fprintf(F, "(%s) ", label);
	else fprintf(F, " ");
	html_in_para = HTML_IN_ITEM;
}

void exit_items(FILE *F, int depth) {
	if (item_depth == depth) {
		fprintf(F, "</li>");
	} else {
		while (item_depth < depth) {
			fprintf(F, "<ul class=\"items\">"); item_depth++;
		}
		while (item_depth > depth) {
			fprintf(F, "</li></ul>"); item_depth--;
		}
	}
	if (depth > 0) fprintf(F, "<li>");
}

@

@c
void html_bar(FILE *F, weave_target *wv) {
	@<Close any HTML paragraph now open@>;
	fprintf(F, "<hr>\n");
}

@

@c
void html_figure(FILE *F, weave_target *wv, char *figname, int cm) {	
	@<Close any HTML paragraph now open@>;
	fprintf(F, "<center><img src=\"Figures%c%s\"></center>\n", SEP_CHAR, figname);
}

@

@c
void html_cweb_macro(char *matter, weave_target *wv, cweb_macro *cwm, int defn) {
	paragraph *P = cwm->defining_paragraph;
	in_sprintf(matter, "&lt;");
	in_sprcat(matter, "<span class=\"cwebmacro%s\">%s</span>",
		(defn)?"defn":"", cwm->macro_name);
	if (defn == FALSE)
		in_sprcat(matter, "</a>");
	in_sprcat(matter, " <span class=\"cwebmacronumber\">%s</span>&gt;%s",
		P->paragraph_number, (defn)?" =":"");
}

@

@c
void html_pagebreak(FILE *F, weave_target *wv) {
	@<Close any HTML paragraph now open@>;
}

@

@c
void html_blank_line(FILE *F, weave_target *wv, int in_comment) {
	if (in_comment) {
		@<Close any HTML paragraph now open@>;
		fprintf(F, "<p>"); html_in_para = HTML_IN_P;
	} else fprintf(F, "\n");
}

@

@c
void html_code_note(FILE *F, weave_target *wv, char *comment) {
	@<Close any HTML paragraph now open@>;
	fprintf(F, "<p>%s</p>\n", comment);
}

@

@c
void html_change_mode(FILE *F, weave_target *wv, int old_mode, int new_mode, int content) {
	if (old_mode != new_mode) {
		if ((content) || (new_mode != MACRO_MATERIAL)) @<Close any HTML paragraph now open@>;
		switch (old_mode) {
			case REGULAR_MATERIAL:
				switch (new_mode) {
					case CODE_MATERIAL:
						fprintf(F, "\n<pre class=\"display\">\n");
						break;
					case DEFINITION_MATERIAL:
						fprintf(F, "\n<pre class=\"definitions\">\n");
						break;
					case MACRO_MATERIAL:
						if (content) fprintf(F, "\n<p class=\"macrodefinition\">");
						fprintf(F, "<code class=\"display\">\n");
						break;
				}
				break;
			case MACRO_MATERIAL:
				fprintf(F, "</code></p>");
				switch (new_mode) {
					case CODE_MATERIAL:
						fprintf(F, "\n<pre class=\"displaydefn\">\n");
						break;
					case DEFINITION_MATERIAL:
						fprintf(F, "\n<pre class=\"definitions\">\n");
						break;
				}
				break;
			case DEFINITION_MATERIAL:
				fprintf(F, "</pre>");
				switch (new_mode) {
					case CODE_MATERIAL:
						fprintf(F, "\n<pre class=\"display\">\n");
						break;
					case MACRO_MATERIAL:
						fprintf(F, "\n<p class=\"macrodefinition\"><code class=\"display\">\n");
						break;
				}
				break;
			default:
				fprintf(F, "</pre>\n");
				break;	
		}
	}
}

@

@c
void html_change_colour(weave_target *wv, char *slot, int col, int in_code) {
	switch (col) {
		case MACRO_CODE: in_sprintf(slot, "<span class=\"cwebmacrotext\">"); break;
		case FUNCTION_CODE: in_sprintf(slot, "<span class=\"functiontext\">"); break;
		case IDENTIFIER_CODE: in_sprintf(slot, "<span class=\"identifier\">"); break;
		case ELEMENT_CODE: in_sprintf(slot, "<span class=\"element\">"); break;
		case RESERVED_CODE: in_sprintf(slot, "<span class=\"reserved\">"); break;
		case STRING_CODE: in_sprintf(slot, "<span class=\"string\">"); break;
		case CHAR_LITERAL_CODE: in_sprintf(slot, "<span class=\"character\">"); break;
		case CONSTANT_CODE: in_sprintf(slot, "<span class=\"constant\">"); break;
		case PLAIN_CODE: in_sprintf(slot, "<span class=\"plain\">"); break;
		default: printf("col: %d\n", col); fatal_error("internal: bad colour"); break;
	}
}

@

@c
void html_endnote(FILE *F, weave_target *wv, int end) {
	if (end == 1) {
		@<Close any HTML paragraph now open@>;
		fprintf(F, "<p class=\"endnote\">");
	} else {
		fprintf(F, "</p>\n");
	}
}

@

@c
void html_identifier(FILE *F, weave_target *wv, char *id) {
	for (int i=0; id[i]; i++)
		if (id[i] == '&') fprintf(F, "&amp;");
		else if (id[i] == '<') fprintf(F, "&lt;");
		else if (id[i] == '>') fprintf(F, "&gt;");
		else if ((id[i] == ' ') && (id[i+1] == '-') && (id[i+2] == '-') &&
			((id[i+3] == ' ') || (id[i+3] == 0))) {
			fprintf(F, " &mdash;"); i+=2;
		} else fprintf(F, "%c", id[i]);
}

@

@c
void html_locale(FILE *F, weave_target *wv, paragraph *par1, paragraph *par2) {
	fprintf(F, "<a href=\"");
	html_xref(F, wv, par1, page_section, TRUE);
	fprintf(F, "\">");
	fprintf(F, "%s%s",
		(*(par1->ornament) == 'S')?"&#167;":"&para;",
		par1->paragraph_number);
	if (par2) fprintf(F, "-%s", par2->paragraph_number);
	fprintf(F, "</a>");
}

@

@c
void html_xref(FILE *F, weave_target *wv, paragraph *P, section *from, int a_link) {
	string linkto;
	in_strcpy(linkto, "");
	if ((from) && (P->under_section != from)) {
		in_strcpy(linkto, P->under_section->sigil);
		for (int i=0; linkto[i]; i++)
			if ((linkto[i] == '/') || (linkto[i] == ' '))
				in_set(linkto, i, '-');
		in_strcat(linkto, ".html");
	}
	fprintf(F, "%s%s%s", linkto, (a_link)?"#":"", P->ornament);
	char *N = P->paragraph_number;
	for (int i=0; N[i]; i++)
		if (N[i] == '.') fprintf(F, "_");
		else fprintf(F, "%c", N[i]);
}

@

@c
void html_tail(FILE *F, weave_target *wv, char *comment) {
	@<Close any HTML paragraph now open@>;
	fprintf(F, "<!--%s-->\n", comment);
	string template;
	in_sprintf(template, "%stemplate", path_to_inweb_materials);
	set_bibliographic_data(wv->weave_web, "Booklet Title", wv->booklet_title);
	weave_cover_from(F, wv->weave_web, template, wv, WEAVE_SECOND_HALF);
}
