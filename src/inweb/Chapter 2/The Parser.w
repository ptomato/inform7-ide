[Parser::] The Parser.

@Purpose: To work through the program read in, assigning each line its category,
and noting down other useful information as we go.

@pp Sequence of parsing.

@c
void Parser::parse_literate_source(web *W) {
	for (chapter *C = W->first_chapter; C; C = C->next_chapter)
		for (section *S = C->first_section; S; S = S->next_section) {
			int comment_mode = TRUE;
			int code_lcat_for_body = NO_LCAT;
			int before_bar = TRUE;
			int next_par_number = 1;
			paragraph *current_paragraph = NULL;
			for (source_line *L = S->first_line; L; L = L->next_line)
				@<Determine category for this source line@>;
		}
	for (chapter *C = W->first_chapter; C; C = C->next_chapter)
		for (section *S = C->first_section; S; S = S->next_section) {
			if (S->tag_with)
				for (paragraph *P = S->first_paragraph; P; P = P->next_paragraph_in_section)
					Parser::add_tag_to_para(P, S->tag_with, NULL);
			if (S->barred == FALSE)
				for (paragraph *P = S->first_paragraph; P; P = P->next_paragraph_in_section)
					P->ornament = "S";
		}
	Languages::further_parsing(W, W->main_language);
}

@

@<Determine category for this source line@> =
	L->is_commentary = comment_mode;
	L->category = COMMENT_BODY_LCAT; /* until set otherwise down below */
	L->owning_paragraph = current_paragraph;

	if (L->source.line_count == 0) @<Parse the line as a probable chapter heading@>;
	if (L->source.line_count == 1) @<Parse the line as a probable section heading@>;
	@<Parse the line as a possible Inweb command@>;
	@<Parse the line as a possible CWEB macro definition@>;
	if ((L->text[0] == '@') && (L->category != MACRO_DEFINITION_LCAT))
		@<Parse the line as a structural marker@>;
	if (comment_mode) @<This is a line destined for the commentary@>;
	if (comment_mode == FALSE) @<This is a line destined for the verbatim code@>;

@ This must be one of the inserted lines marking chapter headings; it doesn't
come literally from the source web.

@<Parse the line as a probable chapter heading@> =
	if (CStrings::eq(L->text, "Chapter Heading")) {
		comment_mode = TRUE;
		L->is_commentary = TRUE;
		L->category = CHAPTER_HEADING_LCAT;
	}

@ The top line of a section gives its title and sigil; in C-for-Inform, it can
also give the namespace for its functions.

@<Parse the line as a probable section heading@> =
	string rewritten; CStrings::copy(rewritten, "");
	string found_text1;
	string found_text2;		
	string found_text3;
	if (ISORegexp::match_3(L->text, "%[(%C+)%] (%C+/%C+): (%c+).",
						found_text1, found_text2, found_text3)) {
		CStrings::copy(S->sect_namespace, found_text1);
		CStrings::copy(S->sigil, found_text2);
		CStrings::copy(S->sect_title, found_text3);
		L->text_operand = Memory::new_string(found_text3);
		L->category = SECTION_HEADING_LCAT;
	} else if (ISORegexp::match_2(L->text, "(%C+/%C+): (%c+).", found_text1, found_text2)) {
		CStrings::copy(S->sigil, found_text1);
		CStrings::copy(S->sect_title, found_text2);
		L->text_operand = Memory::new_string(found_text2);
		L->category = SECTION_HEADING_LCAT;
	} else if (ISORegexp::match_2(L->text, "%[(%C+::)%] (%c+).", found_text1, found_text2)) {
		CStrings::copy(S->sect_namespace, found_text1);
		CStrings::copy(S->sect_title, found_text2);
		@<Set the sigil to an automatic abbreviation of the relative pathname@>;
		L->text_operand = Memory::new_string(found_text2);
		L->category = SECTION_HEADING_LCAT;
	} else if (ISORegexp::match_1(L->text, "(%c+).", found_text1)) {
		CStrings::copy(S->sect_title, found_text1);
		@<Set the sigil to an automatic abbreviation of the relative pathname@>;
		L->text_operand = Memory::new_string(found_text1);
		L->category = SECTION_HEADING_LCAT;
	}

@ If no sigil is supplied, we make one ourselves.

@<Set the sigil to an automatic abbreviation of the relative pathname@> =
	sprintf(S->sigil, "%s/", C->ch_sigil);

	char *from = S->sect_title;
	int w = CStrings::len(S->sigil);
	char *tail = S->sigil + w;

	int letters_from_each_word = 5;
	do {
		@<Make the tail using this many consonants from each word@>;
		if (--letters_from_each_word == 0) break;
	} while (CStrings::len(tail) > 5);

	@<Terminate with disambiguating numbers in case of collisions@>;

@ We collapse words to an initial letter plus consonants: thus "electricity"
would be "elctrcty", since we don't count "y" as a vowel here.

@<Make the tail using this many consonants from each word@> =
	int sn = 0, sw = w;
	if (from[sn] == FOLDER_SEPARATOR) sn++;
	int letters_from_current_word = 0;
	while ((from[sn]) && (from[sn] != '.')) {
		if (from[sn] == ' ') letters_from_current_word = 0;
		else {
			if (letters_from_current_word < letters_from_each_word) {
				if (from[sn] != '-') {
					int l = tolower(from[sn]);
					if ((letters_from_current_word == 0) ||
						((l != 'a') && (l != 'e') && (l != 'i') && (l != 'o') && (l != 'u'))) {
						S->sigil[sw++] = (char) l; S->sigil[sw] = 0;
						letters_from_current_word++;
					}
				}
			}
		}
		sn++;
	}

@ We never want two sections to have the same sigil.

@<Terminate with disambiguating numbers in case of collisions@> =
	char *distail = S->sigil + CStrings::len(S->sigil);
	int disnum = 0, collision = FALSE;
	do {
		if (disnum++ > 0) {
			int ldn = 5;
			if (disnum >= 1000) ldn = 4;
			else if (disnum >= 100) ldn = 3;
			else if (disnum >= 10) ldn = 2;
			else ldn = 1;
			sprintf(distail-ldn, "%d", disnum);
		}
		collision = FALSE;
		for (chapter *C = W->first_chapter; C; C = C->next_chapter)
			for (section *S2 = C->first_section; S2; S2 = S2->next_section)
				if ((S2 != S) && (strcmp(S2->sigil, S->sigil) == 0)) {
					collision = TRUE; break;
				}
	} while (collision);

@ Note that we report an error if the command isn't one we recognise: we
don't simply ignore the squares and let it fall through into the tangler.
There used to be two other syntaxes for oddball commands (to force page
breaks and to pause in woven output), but those have gone in favour of
consistency. The syntax for tagging paragraphs has also changed, from
|[[Index Under Peaches]]| to |[[Tag: Peaches]]|.

@<Parse the line as a possible Inweb command@> =
	string found_text1;		
	string found_text2;		
	if (ISORegexp::match_1(L->text, "%[%[(%c+)%]%]", found_text1)) {
		string command_text; CStrings::copy(command_text, found_text1);
		L->category = COMMAND_LCAT;
		if (ISORegexp::match_2(command_text, "(%c+?): *(%c+)", found_text1, found_text2)) {
			CStrings::copy(command_text, found_text1);
			L->text_operand = Memory::new_string(found_text2);
		}
		if (CStrings::eq(command_text, "Page Break"))
			L->command_code = PAGEBREAK_CMD;
		else if (CStrings::eq(command_text, "Grammar Index"))
			L->command_code = GRAMMAR_INDEX_CMD;
		else if (CStrings::eq(command_text, "Tag")) {
			Parser::add_tag_by_name(L, L->text_operand);
			L->command_code = TAG_CMD;
		} else if (CStrings::eq(command_text, "Figure")) {
			Parser::add_tag_by_name(L, "Figures");
			L->command_code = FIGURE_CMD;
		} else Main::error_in_web("unknown [[command]]", L);
		L->is_commentary = TRUE;
	}

@ Some paragraphs define angle-bracketed macros, and those need special
handling. We'll call these "|CWEB| macros", since our syntax here is
identical to |CWEB|'s.

@<Parse the line as a possible CWEB macro definition@> =
	string found_text1;
	if ((L->text[0] == '@') && (L->text[1] == '<') &&
		(ISORegexp::match_1(L->text+2, "(%c+)@> *= *", found_text1))) {
		string cweb_macro_name; CStrings::copy(cweb_macro_name, found_text1);
		L->category = MACRO_DEFINITION_LCAT;
		if (current_paragraph == NULL)
			Main::error_in_web("<...> definition begins outside of a paragraph", L);
		else @<Create a CWEB macro here@>;
		comment_mode = FALSE;
		L->is_commentary = FALSE;
		code_lcat_for_body = CODE_BODY_LCAT; /* code follows on subsequent lines */
		continue;
	}

@ We store these like so:

@c
typedef struct cweb_macro {
	string macro_name;
	struct paragraph *defining_paragraph; /* as printed in small type after the name in any usage */
	struct source_line *defn_start; /* it ends at the end of its defining paragraph */
	struct cweb_macro *next_macro; /* within the owning section's linked list */
	struct macro_usage *macro_usages; /* only computed for weaves, since unnecessary for tangles */
	MEMORY_MANAGEMENT
} cweb_macro;

@ Each section has its own linked list of CWEB macros, since the scope for
the usage of these is always a single section.

@<Create a CWEB macro here@> =
	cweb_macro *cwm = CREATE(cweb_macro);
	CStrings::copy(cwm->macro_name, cweb_macro_name);
	cwm->defining_paragraph = current_paragraph;
	current_paragraph->defines_macro = cwm;
	cwm->defn_start = L->next_line;
	cwm->next_macro = NULL;
	cwm->macro_usages = NULL;

	if (S->first_macro == NULL) S->first_macro = cwm;
	else S->last_macro->next_macro = cwm;
	S->last_macro = cwm;

@ A structural marker is introduced by an |@| in column 1, and is a structural
division in the current section.

@<Parse the line as a structural marker@> =
	string command_text;
	CStrings::copy(command_text, L->text + 1); /* i.e., strip the at-sign from the front */
	char *remainder = "";
	for (int i = 0; command_text[i]; i++)
		if (ISORegexp::white_space(command_text[i])) {
			CStrings::truncate(command_text, i);
			remainder = command_text+i+1;
			while (ISORegexp::white_space(*remainder)) remainder++;
			break;
		}
	@<Deal with a structural marker@>;
	continue;

@ There are a number of possibilities. One, where the line is a macro
definition, we have already dealt with.

@<Deal with a structural marker@> =
	if (CStrings::eq(command_text, "Purpose:")) @<Deal with Purpose@>
	else if (CStrings::eq(command_text, "Interface:")) @<Deal with Interface@>
	else if (CStrings::eq(command_text, "Definitions:")) @<Deal with Definitions@>
	else if (ISORegexp::match_0(command_text, "----+")) @<Deal with the bar@>
	else if ((CStrings::eq(command_text, "c")) ||
			(CStrings::eq(command_text, "e")) ||
			(CStrings::eq(command_text, "x"))) @<Deal with the code and extract markers@>
	else if (CStrings::eq(command_text, "d")) @<Deal with the define marker@>
	else {
		int weight = -1, new_page = FALSE;
		if (CStrings::eq(command_text, "")) weight = ORDINARY_WEIGHT;
		if (CStrings::eq(command_text, "p")) weight = SUBHEADING_WEIGHT;
		if (CStrings::eq(command_text, "pp")) { weight = SUBHEADING_WEIGHT; new_page = TRUE; }
		if (weight >= 0) @<Begin a new paragraph of this weight@>
		else Main::error_in_web("don't understand @command", L);
	}

@ Major structural markers down to the bar:

@<Deal with Purpose@> =
	if (before_bar == FALSE) Main::error_in_web("Purpose used after bar", L);
	L->category = PURPOSE_LCAT;
	L->is_commentary = TRUE;
	L->text_operand = Memory::new_string(remainder);
	CStrings::copy(S->sect_purpose, remainder);
	source_line *XL = L->next_line;
	while ((XL) && (XL->next_line) && (XL->owning_section == L->owning_section) &&
		(isalnum(XL->text[0]))) {
		CStrings::concatenate(S->sect_purpose, " ");
		CStrings::concatenate(S->sect_purpose, XL->text);
		XL->category = PURPOSE_BODY_LCAT;
		XL->is_commentary = TRUE;
		L = XL;
		XL = XL->next_line;
	}

@

@<Deal with Interface@> =
	if (before_bar == FALSE) Main::error_in_web("Interface used after bar", L);
	L->category = INTERFACE_LCAT;
	L->is_commentary = TRUE;
	source_line *XL = L->next_line;
	while ((XL) && (XL->next_line) && (XL->owning_section == L->owning_section)) {
		if (XL->text[0] == '@') break;
		XL->category = INTERFACE_BODY_LCAT;
		L = XL;
		XL = XL->next_line;
	}

@

@<Deal with Definitions@> =
	if (before_bar == FALSE) Main::error_in_web("Definitions used after bar", L);
	L->category = DEFINITIONS_LCAT;
	L->is_commentary = TRUE;
	before_bar = TRUE;
	next_par_number = 1;

@ An |@| sign in the first column, followed by a row of four or more dashes,
constitutes the optional division bar in a section.

@<Deal with the bar@> =
	if (before_bar == FALSE) Main::error_in_web("second bar in the same section", L);
	L->category = BAR_LCAT;
	L->is_commentary = TRUE;
	comment_mode = TRUE;
	S->barred = TRUE;
	before_bar = FALSE;
	next_par_number = 1;

@ Here we handle |@c| (code), |@e| (early code) and |@x| (code-like extract).
These have identical behaviour except for whether or not to tangle what
follows:

@<Deal with the code and extract markers@> =
	L->category = BEGIN_VERBATIM_LCAT;
	if ((CStrings::eq(command_text, "e")) && (current_paragraph))
		current_paragraph->placed_early = TRUE;
	if (CStrings::eq(command_text, "x")) code_lcat_for_body = TEXT_EXTRACT_LCAT;
	else code_lcat_for_body = CODE_BODY_LCAT;
	comment_mode = FALSE;

@ This is for |@d|. Definitions are intended to translate to C preprocessor
macros, Inform 6 |Constant|s, and so on.

@<Deal with the define marker@> =
	L->category = BEGIN_DEFINITION_LCAT;
	code_lcat_for_body = CONT_DEFINITION_LCAT;
	string found_text1;
	string found_text2;	
	if (ISORegexp::match_2(remainder, "(%C+) (%c+)", found_text1, found_text2)) {
		L->text_operand = Memory::new_string(found_text1); /* name of term defined */
		L->text_operand2 = Memory::new_string(found_text2); /* Value */
	} else {
		L->text_operand = Memory::new_string(remainder); /* name of term defined */
		L->text_operand2 = ""; /* no value given */
	}
	Analyser::mark_reserved_word(S, L->text_operand, CONSTANT_CODE);
	comment_mode = FALSE;
	L->is_commentary = FALSE;

@ Here we handle plain |@|, |@p| and |@pp|. The noteworthy thing here is the
way we fool around with the text on the line of the paragraph opening. This is
one of the few cases where Inweb has retained the stream-based style of
|CWEB|, where escape characters can appear anywhere in a line and line breaks
aren't very significant. Thus

	|@p The chronology of French weaving. Auguste de Papillon (1734-56) soon|

is split into two, so that the title of the paragraph is just ``The chronology
of French weaving'' and the remainder,

	|Auguste de Papillon (1734-56) soon|

will be woven exactly as the succeeding lines will be.

@<Begin a new paragraph of this weight@> =
	comment_mode = TRUE;
	L->is_commentary = TRUE;
	L->category = PARAGRAPH_START_LCAT;
	if (weight == 1) L->category = PB_PARAGRAPH_START_LCAT;
	L->text_operand = ""; /* title */
	string found_text1;
	string found_text2;		
	if ((weight == SUBHEADING_WEIGHT) && (ISORegexp::match_2(remainder, "(%c+). (%c+)", found_text1, found_text2))) {
		L->text_operand = Memory::new_string(found_text1);
		L->text_operand2 = Memory::new_string(found_text2);
	} else if ((weight == SUBHEADING_WEIGHT) && (ISORegexp::match_1(remainder, "(%c+). *", found_text1))) {
		L->text_operand = Memory::new_string(found_text1);
		L->text_operand2 = Memory::new_string("");
	} else {
		L->text_operand = Memory::new_string("");
		L->text_operand2 = Memory::new_string(remainder);
	}
	@<Create a new paragraph, starting here, as new current paragraph@>;

	L->owning_paragraph = current_paragraph;
	W->no_paragraphs++;

@ One of our tasks here is to dice up the section into paragraphs. These begin
with headings of different "weights". In earlier versions of Inweb there
were more elaborate possibilities, but we now have just:

@d ORDINARY_WEIGHT 0 /* an ordinary |@| paragraph */
@d SUBHEADING_WEIGHT 1 /* an |@p| or |@pp| paragraph */

@c
typedef struct paragraph {
	int above_bar; /* placed above the dividing bar in its section */
	int placed_early; /* should appear early in the tangled code */
	char *ornament; /* a "P" for a pilcrow or "S" for section-marker */
	string paragraph_number; /* used in combination with the ornament */
	int next_child_number; /* used when working out paragraph numbers */
	struct paragraph *parent_paragraph; /* ditto */
	int starts_on_new_page; /* relevant for weaving to |TeX| only, of course */
	int weight; /* typographic prominence: one of the |*_WEIGHT| values */
	struct cweb_macro *defines_macro; /* there can only be one */
	struct function *first_defined_in_paragraph; /* there can be several */
	struct c_structure *first_c_structure_in_para; /* similarly */
	struct c_structure *last_c_structure_in_para; /* similarly */
	struct paragraph_tag *tags; /* a linked list */

	struct source_line *first_line_in_paragraph; /* a linked list */

	struct section *under_section;
	struct paragraph *next_paragraph_in_section; /* within the owning section's linked list */
	MEMORY_MANAGEMENT
} paragraph;

@

@<Create a new paragraph, starting here, as new current paragraph@> =
	paragraph *P = CREATE(paragraph);
	P->above_bar = before_bar;
	P->placed_early = before_bar;
	if (before_bar) P->ornament = "P"; else P->ornament = "S";
	CSTRING_WRITE(P->paragraph_number, "%d", next_par_number++);
	P->parent_paragraph = NULL;
	P->next_child_number = 1;
	P->starts_on_new_page = FALSE;
	P->weight = weight;
	P->first_line_in_paragraph = L;
	P->next_paragraph_in_section = NULL;
	P->defines_macro = NULL;
	P->first_defined_in_paragraph = NULL;
	P->first_c_structure_in_para = NULL; P->last_c_structure_in_para = NULL;
	P->tags = NULL;

	P->under_section = S;
	S->sect_paragraphs++;
	if (S->first_paragraph) S->last_paragraph->next_paragraph_in_section = P;
	else S->first_paragraph = P;
	S->last_paragraph = P;

	current_paragraph = P;

@p Any last special rules for lines. In commentary mode, we look for any
fancy display requests, and also look to see if there are interface lines
under an |@Interface:| heading (because that happens in commentary mode,
though this isn't obvious).

@<This is a line destined for the commentary@> =
	string found_text1;
	if (ISORegexp::match_1(L->text, ">> (%c+)", found_text1)) {
		L->category = SOURCE_DISPLAY_LCAT;
		L->text_operand = Memory::new_string(found_text1);
	}

@ Note that in an |@d| definition, a blank line is treated as the end of the
definition. (This is unnecessary for C, and is a point of difference with
|CWEB|, but is needed for languages which don't allow multi-line definitions.)

@<This is a line destined for the verbatim code@> =
	if ((L->category != BEGIN_DEFINITION_LCAT) && (L->category != COMMAND_LCAT))
		L->category = code_lcat_for_body;

	if ((L->category == CONT_DEFINITION_LCAT) && (ISORegexp::string_is_white_space(L->text))) {
		L->category = COMMENT_BODY_LCAT;
		L->is_commentary = TRUE;
		code_lcat_for_body = COMMENT_BODY_LCAT;
		comment_mode = TRUE;
	}

	Languages::subcategorise_code(S->sect_language, L);

@p CWEB macro search.
There's an important difference here between Inweb 2 and Inweb 1: the
scope for looking up CWEB macro names is a single section, not the entire web.
So you can't expand a macro from another section, but then again, you can
use the same macro name twice in different sections; and lookup is much faster.

@c
cweb_macro *Parser::get_cweb_macro_by_name(char *name, section *scope) {
	cweb_macro *cwm;
	for (cwm = scope->first_macro; cwm; cwm = cwm->next_macro)
		if (CStrings::eq(name, cwm->macro_name))
			return cwm;
	return NULL;
}

@p Thematic tags.
The idea here is that certain paragraphs in a large web can be tagged by the
author as having a particular theme. Each different theme name has a |theme_tag|
structure.

@c
typedef struct theme_tag {
	string tag_name;
	string title_when_woven;
	string leafname_when_woven;
	string cover_sheet_when_woven;
	MEMORY_MANAGEMENT
} theme_tag;

@ Here we find a tag from its name, case-sensitively.

@c
theme_tag *Parser::tag_by_name(char *p) {
	theme_tag *tag;
	LOOP_OVER(tag, theme_tag)
		if (CStrings::eq(p, tag->tag_name))
			return tag;
	return NULL;
}

@ Each paragraph can have any number of different tags, kept in a linked
list, though this is more usually empty. Each attachment of a tag to a
paragraph gets its own |paragraph_tag| structure.

@c
typedef struct paragraph_tag {
	struct theme_tag *the_tag;
	struct paragraph_tag *next_tag;
	string caption;
	MEMORY_MANAGEMENT
} paragraph_tag;

@ There are two ways tags can be created: by declaration on the contents
page, or simply by turning up as markers inside the web somewhere. The former
works better, though, if we want to make fancy booklets from these tags,
because it allows us to specify a cover sheet, a title and so on.

Here's the declaration way:

@c
theme_tag *Parser::declare_tag_from_contents(web *W, char *name, char *title, char *leaf, char *cover) {
	theme_tag *tag = Parser::add_tag_by_name(NULL, name);
	CStrings::copy(tag->title_when_woven, title);
	CStrings::copy(tag->leafname_when_woven, leaf);
	CStrings::copy(tag->cover_sheet_when_woven, cover);
	Languages::new_tag_declared(tag);
	return tag;
}

@ And here is the more basic way:

@c
theme_tag *Parser::add_tag_by_name(source_line *L, char *p) {
	string name; CStrings::copy(name, p);
	string caption; CStrings::copy(caption, "");
	string found_text1;
	string found_text2;	
	if (ISORegexp::match_2(name, "(%c+?): (%c+)", found_text1, found_text2)) {
		CStrings::copy(name, found_text1);
		CStrings::copy(caption, found_text2);
	}
	theme_tag *tag = Parser::tag_by_name(name);
	if (tag == NULL) @<Create a new thematic tag@>;
	if ((L) && (L->owning_paragraph)) Parser::add_tag_to_para(L->owning_paragraph, tag, caption);
	return tag;
}

@

@<Create a new thematic tag@> =
	tag = CREATE(theme_tag);
	CStrings::copy(tag->tag_name, name);
	CStrings::copy(tag->title_when_woven, name);
	CStrings::copy(tag->leafname_when_woven, name);
	CStrings::copy(tag->cover_sheet_when_woven, "");

@ And more basically still:

@c
void Parser::add_tag_to_para(paragraph *P, theme_tag *tag, char *caption) {
	if (P) {
		paragraph_tag *pt = CREATE(paragraph_tag);
		pt->the_tag = tag;
		if (caption) CStrings::copy(pt->caption, caption);
		else CStrings::copy(pt->caption, "");
		pt->next_tag = P->tags; P->tags = pt; /* insert at front of linked list */
	}
}

@ Finally, this tests whether a given location is marked with a given tag:

@c
int Parser::is_tagged_with(source_line *L, theme_tag *tag) {
	if (tag == NULL) return TRUE;
	if (L)
		if (L->owning_paragraph)
			for (paragraph_tag *pt = L->owning_paragraph->tags; pt; pt = pt->next_tag)
				if (tag == pt->the_tag)
					return TRUE;
	return FALSE;
}
