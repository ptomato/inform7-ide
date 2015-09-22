[CLike::] C-Like Languages.

@Purpose: To provide special features for the whole C family of languages.

@p Typedefed structures.
We're going to assume that the C source code uses structures looking
something like this:

	|typedef struct fruit {|
	|    struct pip the_pips[5];|
	|    struct fruit *often_confused_with;|
	|    struct tree_species *grows_on;|
	|    int typical_weight;|
	|} fruit;|

which adopts the traditional layout conventions of Kernighan and Ritchie.
The structure definitions in this Inweb web all take the required form,
of course, and provide many more examples.

Note that a |fruit| structure contains a |pip| structure (in fact, five of
them), but only contains pointers to |tree_species| structures and itself.
C requires therefore that the structure definition for |pip| must occur
earlier in the code than that for |fruit|. This is a nuisance, so Inweb
takes care of it automatically.

@c
typedef struct c_structure {
	string structure_name;
	int tangled; /* whether the structure definition has been tangled out */
	struct source_line *typedef_begins; /* opening line of |typedef| */
	struct source_line *typedef_ends; /* closing line, where |}| appears */
	struct section *declared_owner;
	int declared_private;
	struct structure_dependence *first_dependence; /* double-ended linked list */
	struct structure_dependence *last_dependence;
	struct structure_element *first_element; /* double-ended linked list */
	struct structure_element *last_element;
	struct structure_permission *first_permission; /* double-ended linked list */
	struct structure_permission *last_permission;
	struct c_structure *next_c_structure; /* in the linked list for its web */
	struct c_structure *next_c_structure_in_para; /* and for its paragraph */
	struct c_structure *next_cst_alphabetically;
	MEMORY_MANAGEMENT
} c_structure;

c_structure *first_cst_alphabetically = NULL;

@

@c
typedef struct structure_element {
	string element_name;
	struct structure_element *next_element;
	struct source_line *element_created_at;
	int allow_sharing;
	MEMORY_MANAGEMENT
} structure_element;

@

@c
typedef struct structure_permission {
	struct section *shared_with;
	struct source_line *granted_at;
	struct structure_permission *next_permission;
	MEMORY_MANAGEMENT
} structure_permission;

@ So, for example, a dependence exists between |fruit| and |pip|. We will 
create an instance of the following, where |incorporates| will point to |pip|,
and add it to the linked list of dependencies of |fruit|.

@c
typedef struct structure_dependence {
	struct c_structure *incorporates; /* the structure being embedded */
	struct structure_dependence *next_dependence; /* in the linked list for its structure */
	MEMORY_MANAGEMENT	
} structure_dependence;

@ We also expect to see function declarations in the web; these, we will
assume, always begin on column 1 of their source files. We expect them to take
modern ANSI C style, not the long-deprecated late 1970s C style. The function
definitions in this Inweb web all take the required form, of course.

@d MAX_ARG_LINES 8 /* maximum number of lines over which a function's header can extend */

@c
typedef struct function {
	string function_name; /* e.g., |"cultivate"| */
	string function_type; /* e.g., |"tree *"| */
	string function_arguments; /* e.g., |"int rainfall)"|: note |)| */
	struct source_line *function_header_at; /* where the first line of the header begins */
	struct function *next_defined_in_paragraph;
	int within_namespace; /* written using C-for-Inform namespace dividers */
	int called_from_other_sections;
	int call_freely;
	MEMORY_MANAGEMENT
} function;

@p Parsing extras.
There are two goals here. First to find all the defined structures, and which
contain which; second to find all the functions.

@c
void CLike::further_parsing(web *W) {
	@<Create a base type for every structure definition@>;
	@<Detect any structure dependences@>;
	LOOP_WITHIN_TANGLE(W->first_target)
		if ((L->category == CODE_BODY_LCAT) ||
			(L->category == BEGIN_DEFINITION_LCAT) ||
			(L->category == CONT_DEFINITION_LCAT))
			@<Look for a function definition on this line@>;
}

@

@<Create a base type for every structure definition@> =
	c_structure *current_bt = NULL;
	LOOP_WITHIN_TANGLE(W->first_target) {
		string found_text1;
		string found_text2;	
		if (ISORegexp::match_1(L->text, "typedef struct (%i+) %c*", found_text1)) {
			string sname; CStrings::copy(sname, found_text1);
			current_bt = CLike::attach_structure(W, sname, L);
			Parser::add_tag_by_name(L, "Structures");
		} else if (((L->text)[0] == '}') && (current_bt)) {
			current_bt->typedef_ends = L;
			current_bt = NULL;
		} else if ((current_bt) && (current_bt->typedef_ends == NULL)) {
			char *p = L->text;
			while (ISORegexp::white_space(*p)) p++;
			char *modifier_patterns[] = {
				"(struct )(%C%c*)", "(signed )(%C%c*)", "(unsigned )(%C%c*)",
				"(short )(%C%c*)", "(long )(%C%c*)", "(static )(%C%c*)", NULL };
			int seek_modifiers = TRUE;
			while (seek_modifiers) {
				seek_modifiers = FALSE;
				for (int i = 0; modifier_patterns[i]; i++)
					if (ISORegexp::match_2(p, modifier_patterns[i], found_text1, found_text2)) {
						p = found_text2;
						seek_modifiers = TRUE;
						break;
					}
			}
			if (*p != '/') {
				while ((*p) && (ISORegexp::white_space(*p) == FALSE)) p++;
				while ((ISORegexp::white_space(*p)) || (*p == '*') || (*p == '(') || (*p == ')')) p++;
				if (p[0]) {
					string elname; CStrings::copy(elname, p);
					for (int i=0; elname[i]; i++)
						if (ISORegexp::identifier_char(elname[i]) == FALSE)
							CStrings::truncate(elname, i);
					Analyser::mark_reserved_word(L->owning_section, elname, ELEMENT_CODE);
					structure_element *elt = CREATE(structure_element);
					CStrings::copy(elt->element_name, elname);
					elt->next_element = NULL;
					elt->allow_sharing = FALSE;
					elt->element_created_at = L;
					if (W->main_language == C_FOR_INFORM_LANGUAGE) {
						if (CStrings::eq(elname, "word_ref1")) elt->allow_sharing = TRUE;
						if (CStrings::eq(elname, "word_ref2")) elt->allow_sharing = TRUE;
						if (CStrings::eq(elname, "next")) elt->allow_sharing = TRUE;
						if (CStrings::eq(elname, "down")) elt->allow_sharing = TRUE;
						if (CStrings::eq(elname, "allocation_id")) elt->allow_sharing = TRUE;
					}
					if (current_bt->first_element == NULL)
						current_bt->first_element = elt;
					else
						current_bt->last_element->next_element = elt;
					current_bt->last_element = elt;
				}
			}
		} else if (ISORegexp::match_0(L->text, "typedef char%c+")) {
			L->category = TYPEDEF_LCAT;
		}
	}

@ Note that since asterisks do not match |%i| in pattern-matching, a line like

	|    struct fruit *often_confused_with;|

does not pass the test here.

@<Detect any structure dependences@> =
	c_structure *current_bt;
	LOOP_OVER(current_bt, c_structure) {
		for (source_line *L = current_bt->typedef_begins;
			((L) && (L != current_bt->typedef_ends));
			L = L->next_line) {
			string found_text1;
			if (ISORegexp::match_1(L->text, " struct (%i+) %i%c*", found_text1))
				@<One structure appears to contain a copy of another one@>;
		}
	}

@

@<One structure appears to contain a copy of another one@> =
	string used_structure; CStrings::copy(used_structure, found_text1);
	for (c_structure *str = W->first_c_structure; str; str = str->next_c_structure)
		if ((str != current_bt) &&
			(CStrings::eq(used_structure, str->structure_name))) {
			structure_dependence *dpd = CREATE(structure_dependence);
			dpd->incorporates = str;
			if (current_bt->first_dependence == NULL) {
				current_bt->first_dependence = dpd;
				current_bt->last_dependence = dpd;
			} else {
				current_bt->last_dependence->next_dependence = dpd;
				current_bt->last_dependence = dpd;
			}
		}

@

@c
c_structure *CLike::find_structure(web *W, char *name) {
	for (c_structure *str = W->first_c_structure; str; str = str->next_c_structure)
		if (CStrings::eq(name, str->structure_name))
			return str;
	return NULL;
}

function *CLike::get_function_with_name(char *name) {
	function *fn;
	LOOP_OVER(fn, function)
		if (CStrings::eq(name, fn->function_name))
			return fn;
	return NULL;
}

@

@c
c_structure *CLike::attach_structure(web *W, char *name, source_line *L) {
	c_structure *str = CREATE(c_structure);
	CStrings::copy(str->structure_name, name);
	Analyser::mark_reserved_word(L->owning_section, str->structure_name, RESERVED_CODE);
	str->typedef_begins = L;
	str->declared_owner = NULL;
	str->tangled = FALSE;
	str->typedef_ends = NULL;
	str->next_c_structure = NULL;
	str->next_c_structure_in_para = NULL;
	str->first_dependence = NULL;
	str->last_dependence = NULL;
	str->first_element = NULL;
	str->last_element = NULL;
	if (W->first_c_structure == NULL) {
		W->first_c_structure = str;
		W->last_c_structure = str;
	} else {
		W->last_c_structure->next_c_structure = str;
		W->last_c_structure = str;
	}
	paragraph *P = L->owning_paragraph;
	if (P) {
		if (P->first_c_structure_in_para == NULL) {
			P->first_c_structure_in_para = str;
			P->last_c_structure_in_para = str;
		} else {
			P->last_c_structure_in_para->next_c_structure_in_para = str;
			P->last_c_structure_in_para = str;
		}
	}
	str->next_cst_alphabetically = NULL;
	if (first_cst_alphabetically == NULL) first_cst_alphabetically = str;
	else {
		int placed = FALSE;
		c_structure *last = NULL;
		for (c_structure *seq = first_cst_alphabetically; seq;
			seq = seq->next_cst_alphabetically) {
			if (CStrings::cmp(name, seq->structure_name) < 0) {
				if (seq == first_cst_alphabetically) {
					str->next_cst_alphabetically = first_cst_alphabetically;
					first_cst_alphabetically = str;
				} else {
					last->next_cst_alphabetically = str;
					str->next_cst_alphabetically = seq;
				}
				placed = TRUE;
				break;
			}
			last = seq;
		}
		if (placed == FALSE) last->next_cst_alphabetically = str;
	}
	return str;
}

@ Second round: we recognise a C function as being a line which takes the form

	|type identifier(args...|

where we parse |type| only minimally. In C for Inform (only), the identifier can
contain namespace dividers written |::|.

@<Look for a function definition on this line@> =
	char *p = L->text;
	if (!(ISORegexp::white_space(p[0]))) {
		string qualifiers; CStrings::copy(qualifiers, "");
		string modified; CStrings::copy(modified, "");
		@<Parse past any type modifiers@>;
		string found_text1;
		string found_text2;	
		string found_text3;
		string found_text4;	
		if (ISORegexp::match_4(p, "(%i+) (%**)(%i+)%((%c*)",
			found_text1, found_text2, found_text3, found_text4)) {
			string ftype; CStrings::copy(ftype, found_text1);
			string asts; CStrings::copy(asts, found_text2);
			string fname; CStrings::copy(fname, found_text3);
			string arguments; CStrings::copy(arguments, found_text4);
			@<Soak up further arguments from continuation lines after the declaration@>;
			Analyser::mark_reserved_word(L->owning_section, fname, FUNCTION_CODE);
	
			function *fn = CREATE(function);
			CStrings::copy(fn->function_name, fname);
			CStrings::copy(fn->function_arguments, arguments);
			CSTRING_WRITE(fn->function_type, "%s%s %s", qualifiers, ftype, asts);
			fn->within_namespace = FALSE;
			fn->called_from_other_sections = FALSE;
			fn->call_freely = FALSE;
			if ((CStrings::eq(fn->function_name, "isdigit")) &&
				(W->main_language == C_FOR_INFORM_LANGUAGE))
				fn->call_freely = TRUE;
			fn->function_header_at = L;
			fn->next_defined_in_paragraph = NULL;
			paragraph *P = L->owning_paragraph;
			if (P) {
				if (P->first_defined_in_paragraph == NULL)
					P->first_defined_in_paragraph = fn;
				else {
					function *ofn = P->first_defined_in_paragraph;
					while ((ofn) && (ofn->next_defined_in_paragraph))
						ofn = ofn->next_defined_in_paragraph;
					ofn->next_defined_in_paragraph = fn;
				}
			}
			L->function_defined = fn;
			
			if (W->main_language == C_FOR_INFORM_LANGUAGE)
				@<Check that the function has its namespace correctly declared@>;
		}
	}

@ C has a whole soup of reserved words applying to types, but most of them
can't apply to the return type of a function. We do, however, iterate so that
forms like |static long long int| will work.

@<Parse past any type modifiers@> =
	char *modifier_patterns[] = {
		"(signed )(%C%c*)", "(unsigned )(%C%c*)",
		"(short )(%C%c*)", "(long )(%C%c*)", "(static )(%C%c*)", NULL };
	int seek_modifiers = TRUE;
	while (seek_modifiers) {
		seek_modifiers = FALSE;
		string found_text1;
		string found_text2;	
		for (int i = 0; modifier_patterns[i]; i++)
			if (ISORegexp::match_2(p, modifier_patterns[i], found_text1, found_text2)) {
				CStrings::concatenate(qualifiers, found_text1);
				CStrings::copy(modified, found_text2);
				p = modified; seek_modifiers = TRUE; break;
			}
	}

@ In some cases the function's declaration runs over several lines:

	|void World::Subjects::make_adj_const_domain(inference_subject *infs,|
	|	instance *nc, property *prn) {|

Having read the first line, |arguments| would contain |inference_subject *infs,|
and would thus be incomplete. We continue across subsequent lines until we
reach an open brace |{|.

@<Soak up further arguments from continuation lines after the declaration@> =
	source_line *AL = L;
	int arg_lc = 1;
	while ((AL) && (arg_lc <= MAX_ARG_LINES) && (ISORegexp::find_open_brace(arguments) == -1)) {
		if (AL->next_line == NULL) {
			string err_mess;
			sprintf(err_mess, "Function '%s' has a malformed declaration", fname);
			Main::error_in_web(err_mess, L);
			break;
		}
		AL = AL->next_line;
		CStrings::concatenate(arguments, " ");
		CStrings::concatenate(arguments, AL->text);
		arg_lc++;
	}
	int n = ISORegexp::find_open_brace(arguments);
	if (n >= 0) CStrings::truncate(arguments, n);

@

@<Check that the function has its namespace correctly declared@> =
	char *declared_namespace = "";
	string found_text1;
	if (ISORegexp::match_1(fname, "(%c+::)%c*", found_text1)) {
		declared_namespace = found_text1;
		fn->within_namespace = TRUE;
	} else if ((strcmp(fname, "main") == 0) && (strcmp(S->sect_namespace, "Main::") == 0))
		declared_namespace = "Main::";
	if (strcmp(declared_namespace, S->sect_namespace) != 0) {
		string err_mess;
		if (declared_namespace[0] == 0)
			sprintf(err_mess, "Function '%s' should have namespace prefix '%s'",
				fname, S->sect_namespace);
		else if (S->sect_namespace[0] == 0)
			sprintf(err_mess, "Function '%s' declared in a section with no namespace",
				fname);
		else
			sprintf(err_mess, "Function '%s' declared in a section with the wrong namespace '%s'",
				fname, S->sect_namespace);
		Main::error_in_web(err_mess, L);
	}

@

@c
void CLike::subcategorise_code(programming_language *pl, source_line *L) {
	string found_text1;
	if (ISORegexp::match_1(L->text, "#include <(%C+)>%c*", found_text1)) {
		string library_file; CStrings::copy(library_file, found_text1);
		char *ansi_libs[] = {
			"assert.h", "ctype.h", "errno.h", "float.h", "limits.h",
			"locale.h", "math.h", "setjmp.h", "signal.h", "stdarg.h",
			"stddef.h", "stdio.h", "stdlib.h", "string.h", "time.h",
			NULL
		};
		for (int j = 0; ansi_libs[j]; j++)
			if (CStrings::eq(library_file, ansi_libs[j]))
				L->category = C_LIBRARY_INCLUDE_LCAT;
	}
}

@p Tangling extras.
We need to include ANSI library files before declaring structures because
otherwise |FILE| and suchlike types won't exist yet. It might seem reasonable
to include all of the |#include| files right now, but that defeats any
conditional compilation, which Inform (for instance) needs in order to make
platform-specific details to handle directories without POSIX in Windows. So
we'll just advance the common ANSI inclusions.

@c
void CLike::shebang(OUTPUT_STREAM, programming_language *pl, web *W, tangle_target *target) {
	LOOP_WITHIN_TANGLE(target)
		if (L->category == C_LIBRARY_INCLUDE_LCAT) {
			Tangler::tangle_code(OUT, L->text, S, L);
			WRITE("\n");
		}
}

@ Next we predeclare the structures, and then the functions -- in that order
since the function types may involve the typedef names for the structures.

@c
void CLike::additional_predeclarations(OUTPUT_STREAM, programming_language *pl, web *W) {
	@<Predeclare simple typedefs@>;
	@<Predeclare the structures in a well-founded order@>;
	@<Predeclare the functions@>;
}

@

@<Predeclare simple typedefs@> =
	LOOP_WITHIN_TANGLE(W->first_target)
		if (L->category == TYPEDEF_LCAT) {
			Languages::tangle_code(OUT, W->main_language, L->text);
			WRITE("\n");
		}

@ The easier case first:

@<Predeclare the functions@> =
	LOOP_WITHIN_TANGLE(W->first_target)
		if (L->function_defined) {
			function *fn = L->function_defined;
			Languages::insert_line_marker(OUT, W->main_language, L);
			WRITE("%s ", fn->function_type);
			Languages::tangle_code(OUT, W->main_language, fn->function_name);
			WRITE("(%s;\n", fn->function_arguments);
		}

@ It's easy enough to make sure structures are tangled so that inner ones
precede outer, but we need to be careful to be terminating if the source
code we're given is not well founded because of an error by its programmer:
for example, that structure A contains B contains C contains A. We do this
with the |tangled| flag, which is |FALSE| if a structure hasn't been
started yet, |NOT_APPLICABLE| if it's in progress, and |TRUE| if it's
finished.

@<Predeclare the structures in a well-founded order@> =
	for (c_structure *str = W->first_c_structure; str; str = str->next_c_structure)
		str->tangled = FALSE;
	for (c_structure *str = W->first_c_structure; str; str = str->next_c_structure)
		CLike::tangle_structure(OUT, pl, str);

@ Using the following recursion, which is therefore terminating:

@c
void CLike::tangle_structure(OUTPUT_STREAM, programming_language *pl, c_structure *str) {
	if (str->tangled != FALSE) return;
	str->tangled = NOT_APPLICABLE;
	for (structure_dependence *dpd = str->first_dependence; dpd; dpd = dpd->next_dependence)
		CLike::tangle_structure(OUT, pl, dpd->incorporates);
	str->tangled = TRUE;
	Languages::insert_line_marker(OUT, pl, str->typedef_begins);
	for (source_line *L = str->typedef_begins; L; L = L->next_line) {
		WRITE("%s\n", L->text);
		L->suppress_tangling = TRUE;
		if (L == str->typedef_ends) break;
	}
}

@ In order for C compilers to report C syntax errors on the correct line,
despite rearranging by automatic tools, C conventionally recognises the
preprocessor directive |#line| to tell it that a contiguous extract follows
from the given file; we generate this automatically. In its usual zany way,
Perl recognises the exact same syntax, thus in principle overloading its
comment notation |#|.

@c
void CLike::insert_line_marker(OUTPUT_STREAM, programming_language *pl, source_line *L) {
	WRITE("#line %d \"%/f\"\n",
		L->source.line_count,
		L->source.text_file_filename);
}

@ We write comments the old-fashioned way, that is, not using |//|.

@c
void CLike::comment(OUTPUT_STREAM, programming_language *pl, char *comm) {
	WRITE("/* %s */\n", comm);
}

@ Likewise:

@c
int CLike::parse_comment(programming_language *pl,
	char *line, char *part_before_comment, char *part_within_comment) {
	string found_text1;
	string found_text2;	
	if (ISORegexp::match_2(line, "(%c*)/%* *(%c*?) *%*/ *", found_text1, found_text2)) {
		CStrings::copy(part_before_comment, found_text1);
		CStrings::copy(part_within_comment, found_text2);
		return TRUE;
	}
	return FALSE;
}

@ The following is an interesting point of difference with Knuth's CWEB. Macros
here are code, and are grouped: they have their own variable scope, and so on.
This means that

	|if (x == y) @<Do something dramatic@>;|

will behave as expected if the definition of "Do something dramatic" consists
of two or more statements; Inweb tangles this to

	|if (x == y)|
	|{|
	|...|
	|}|

so that the condition applies to all of them. (The new-line before the opening
brace protects us against problems with Perl comments; Perl shares this code.)

@c
void CLike::before_macro_expansion(OUTPUT_STREAM, programming_language *pl, cweb_macro *cwm) {
	WRITE("\n{\n");
}

void CLike::after_macro_expansion(OUTPUT_STREAM, programming_language *pl, cweb_macro *cwm) {
	WRITE("}\n");
}

@p Weaving.
We typeset the names of functions being defined in red.

@c
void CLike::begin_weave(section *S, weave_target *wv) {
	Analyser::mark_reserved_word(S, "FILE", RESERVED_CODE);

	Analyser::mark_reserved_word(S, "auto", RESERVED_CODE);
	Analyser::mark_reserved_word(S, "break", RESERVED_CODE);
	Analyser::mark_reserved_word(S, "case", RESERVED_CODE);
	Analyser::mark_reserved_word(S, "char", RESERVED_CODE);
	Analyser::mark_reserved_word(S, "const", RESERVED_CODE);
	Analyser::mark_reserved_word(S, "continue", RESERVED_CODE);
	Analyser::mark_reserved_word(S, "default", RESERVED_CODE);
	Analyser::mark_reserved_word(S, "do", RESERVED_CODE);
	Analyser::mark_reserved_word(S, "double", RESERVED_CODE);
	Analyser::mark_reserved_word(S, "else", RESERVED_CODE);
	Analyser::mark_reserved_word(S, "enum", RESERVED_CODE);
	Analyser::mark_reserved_word(S, "extern", RESERVED_CODE);
	Analyser::mark_reserved_word(S, "float", RESERVED_CODE);
	Analyser::mark_reserved_word(S, "for", RESERVED_CODE);
	Analyser::mark_reserved_word(S, "goto", RESERVED_CODE);
	Analyser::mark_reserved_word(S, "if", RESERVED_CODE);
	Analyser::mark_reserved_word(S, "int", RESERVED_CODE);
	Analyser::mark_reserved_word(S, "long", RESERVED_CODE);
	Analyser::mark_reserved_word(S, "register", RESERVED_CODE);
	Analyser::mark_reserved_word(S, "return", RESERVED_CODE);
	Analyser::mark_reserved_word(S, "short", RESERVED_CODE);
	Analyser::mark_reserved_word(S, "signed", RESERVED_CODE);
	Analyser::mark_reserved_word(S, "sizeof", RESERVED_CODE);
	Analyser::mark_reserved_word(S, "static", RESERVED_CODE);
	Analyser::mark_reserved_word(S, "struct", RESERVED_CODE);
	Analyser::mark_reserved_word(S, "switch", RESERVED_CODE);
	Analyser::mark_reserved_word(S, "typedef", RESERVED_CODE);
	Analyser::mark_reserved_word(S, "union", RESERVED_CODE);
	Analyser::mark_reserved_word(S, "unsigned", RESERVED_CODE);
	Analyser::mark_reserved_word(S, "void", RESERVED_CODE);
	Analyser::mark_reserved_word(S, "volatile", RESERVED_CODE);
	Analyser::mark_reserved_word(S, "while", RESERVED_CODE);
}

@

@c
int colouring_state = PLAIN_CODE;
int CLike::syntax_colour(OUTPUT_STREAM, programming_language *pl, weave_target *wv,
	web *W, chapter *C, section *S, source_line *L, char *matter, char *colouring) {
	for (int i=0; matter[i]; i++) {
		int skip = FALSE, one_off = -1, will_be = -1;
		switch (colouring_state) {
			case PLAIN_CODE:
				switch (matter[i]) {
					case '\"': colouring_state = STRING_CODE; break;
					case '\'': colouring_state = CHAR_LITERAL_CODE; break;
				}
				if ((ISORegexp::identifier_char(matter[i])) && (matter[i] != ':')) {
					if ((!(isdigit(matter[i]))) ||
						((i>0) && (colouring[i-1] == IDENTIFIER_CODE)))
						one_off = IDENTIFIER_CODE;
				}
				break;
			case CHAR_LITERAL_CODE:
				switch (matter[i]) {
					case '\\': skip = TRUE; break;
					case '\'': will_be = PLAIN_CODE; break;
				}
				break;
			case STRING_CODE:
				switch (matter[i]) {
					case '\\': skip = TRUE; break;
					case '\"': will_be = PLAIN_CODE; break;
				}
				break;
		}
		if (one_off >= 0) colouring[i] = (char) one_off; else colouring[i] = (char) colouring_state;
		if (will_be >= 0) colouring_state = (char) will_be;
		if ((skip) && (matter[i+1])) i++;
	}
	int ident_from = -1;
	for (int i=0; matter[i]; i++) {
		if ((matter[i] == ':') && (matter[i+1] == ':') &&
			(colouring[i-1] == IDENTIFIER_CODE) && (colouring[i+2] == IDENTIFIER_CODE)) {
			colouring[i] = IDENTIFIER_CODE;
			colouring[i+1] = IDENTIFIER_CODE;
		}
		if (colouring[i] == IDENTIFIER_CODE) {
			if (ident_from == -1) ident_from = i;
		} else {
			if (ident_from >= 0)
				CLike::colour_ident(S, matter, colouring, ident_from, i-1);
			ident_from = -1;
		}
	}
	if (ident_from >= 0)
		CLike::colour_ident(S, matter, colouring, ident_from, CStrings::len(matter)-1);
	return FALSE;
}

void CLike::colour_ident(section *S, char *matter, char *colouring, int from, int to) {
	string id; CStrings::copy(id, matter+from); CStrings::truncate(id, to-from+1);
	int override = -1;

	if (Analyser::is_reserved_word(S, id, FUNCTION_CODE)) override = FUNCTION_CODE;
	if (Analyser::is_reserved_word(S, id, RESERVED_CODE)) override = RESERVED_CODE;
	if (Analyser::is_reserved_word(S, id, CONSTANT_CODE)) override = CONSTANT_CODE;
	if (Analyser::is_reserved_word(S, id, ELEMENT_CODE)) {
		int at = --from;
		while ((at > 0) && (ISORegexp::white_space(matter[at]))) at--;
		if (((at >= 0) && (matter[at] == '.')) ||
			((at >= 0) && (matter[at-1] == '-') && (matter[at] == '>')))
			override = ELEMENT_CODE;
	}

	if (override >= 0) {
		for (int i=from; i<=to; i++) colouring[i] = (char) override;
	}
}

int CLike::weave_code_line(OUTPUT_STREAM, programming_language *pl, weave_target *wv,
	web *W, chapter *C, section *S, source_line *L, char *matter, char *concluding_comment) {
	return FALSE;
}

@p Analysis.

@c
void CLike::analysis(programming_language *pl, section *S, int functions_too) {
	c_structure *str;
	LOOP_OVER(str, c_structure)
		if (str->typedef_begins->owning_section == S)
			printf(" %s ", str->structure_name);
	if (functions_too) {
		function *fn;
		LOOP_OVER(fn, function)
			if (fn->function_header_at->owning_section == S)
				printf("\n                     %s", fn->function_name);
	}
}

void CLike::analyse_code(programming_language *pl, web *W) {
	function *fn;
	LOOP_OVER(fn, function)
		Analyser::find_hash_entry(fn->function_header_at->owning_section, fn->function_name, TRUE);	
	c_structure *st;
	LOOP_OVER(st, c_structure)
		for (structure_element *elt = st->first_element; elt; elt = elt->next_element)
			if (elt->allow_sharing == FALSE)
				Analyser::find_hash_entry(elt->element_created_at->owning_section, elt->element_name, TRUE);

	c_structure *str = NULL;
	LOOP_WITHIN_TANGLE(W->first_target) {
		if (L->category == INTERFACE_BODY_LCAT) {
			string found_text1;
			string found_text2;	
			if (ISORegexp::match_2(L->text, "-- Owns struct (%c+) %((%c+)%) *", found_text1, found_text2)) {
				int private_flag = NOT_APPLICABLE;
				if (CStrings::eq(found_text2, "private")) private_flag = TRUE;
				if (CStrings::eq(found_text2, "public")) private_flag = FALSE;
				if (private_flag == NOT_APPLICABLE) {
					Main::error_in_web("should be marked private or public", L);
					private_flag = TRUE;
				}
				str = CLike::find_structure(W, found_text1);
				if (str == NULL)
					Main::error_in_web("should be marked private or public", L);
				else {
					str->declared_owner = S;
					str->declared_private = private_flag;
				}
				L->interface_line_identified = TRUE;
			} else if (ISORegexp::match_1(L->text, " *!- shared with (%c+?) *", found_text1)) {
				if (str == NULL)
					Main::error_in_web("no structure yet to be shared", L);
				else {
					TEMPORARY_TEXT(FT);
					WRITE_TO(FT, "%s", found_text1);
					section *SW = Reader::section_by_filename(W, FT);
					DISCARD_TEXT(FT);
					if (SW == NULL)
						Main::error_in_web("no such section", L);
					else {
						structure_permission *sp = CREATE(structure_permission);
						sp->shared_with = SW;
						sp->next_permission = NULL;
						sp->granted_at = L;
						if (str->first_permission == NULL)
							str->first_permission = sp;
						else
							str->last_permission->next_permission = sp;
						str->last_permission = sp;
					}
				}
				L->interface_line_identified = TRUE;
			} else if (ISORegexp::match_0(L->text, "-- Defines {-%c+?:%c+} *")) {
				L->interface_line_identified = TRUE;
			}
		}
	}
}

void CLike::post_analysis(programming_language *pl, web *W) {
	int check_namespaces = FALSE;
	if (CStrings::eq(Bibliographic::get_data(W, "Namespaces"), "On")) check_namespaces = TRUE;
	function *fn;
	LOOP_OVER(fn, function) {
		hash_table_entry *hte = Analyser::find_hash_entry(fn->function_header_at->owning_section, fn->function_name, FALSE);
		if (hte)
			for (hash_table_entry_usage *hteu = hte->first_usage; hteu; hteu = hteu->next_usage)
				if ((hteu->form_of_usage & FCALL_USAGE) || (fn->within_namespace))
					if (hteu->usage_recorded_at->under_section != fn->function_header_at->owning_section)
						fn->called_from_other_sections = TRUE;
		if ((fn->within_namespace != fn->called_from_other_sections)
			&& (check_namespaces)
			&& (fn->call_freely == FALSE)) {
			if (fn->within_namespace)
				Main::error_in_web(
					"Being internally called, this function mustn't belong to a :: namespace",
					fn->function_header_at);
			else
				Main::error_in_web(
					"Being externally called, this function must belong to a :: namespace",
					fn->function_header_at);
		}
	}
	if (CStrings::eq(Bibliographic::get_data(W, "Strict Usage Rules"), "On")) {
		c_structure *str;
		LOOP_OVER(str, c_structure) {
			if (str->declared_owner == NULL) {
				Main::error_in_web("undeclared structure",
					str->typedef_begins);
				str->typedef_begins->owning_section->erroneous_interface = TRUE;
			}
			CLike::section_subset(str, TRUE);
			for (structure_permission *sp = str->first_permission; sp; sp = sp->next_permission) {
				sp->shared_with->scratch_flag |= 2;
				if (sp->shared_with->scratch_flag == 2) {
					Main::error_in_web("structure declared as shared, but it isn't", 
						sp->granted_at);
					str->typedef_begins->owning_section->erroneous_interface = TRUE;
				}
			}
			section *S;
			LOOP_OVER(S, section)
				if (S->scratch_flag == 1) {
					Main::error_in_web("structure has missing shared declarations", 
						str->typedef_begins);
					str->typedef_begins->owning_section->erroneous_interface = TRUE;
					break;
				}
		}
		section *S;
		LOOP_OVER(S, section)
			if (S->erroneous_interface) {
				printf("Interface for section %s should be:\n", S->sect_title);
				@<Write out a correct structure interface@>;
			}
	}
}

@

@<Write out a correct structure interface@> =
	c_structure *str;
	for (str = first_cst_alphabetically; str;
		str = str->next_cst_alphabetically)
		if (str->typedef_begins->owning_section == S) {
			int c = CLike::section_subset(str, FALSE);
			printf("-- Owns struct %s (%s)\n", str->structure_name,
				(c > 0)?"public":"private");
			section *SS;
			LOOP_OVER(SS, section)
				if (SS->scratch_flag)
					PRINT("\t!- shared with %S\n", SS->source_file_for_section);
		}

@

@c
int CLike::section_subset(c_structure *str, int report) {
	int c = 0;
	section *S;
	LOOP_OVER(S, section) S->scratch_flag = 0;
	for (structure_element *elt = str->first_element; elt; elt = elt->next_element) {
		int d = 0;
		hash_table_entry *hte = Analyser::find_hash_entry(elt->element_created_at->owning_section, elt->element_name, FALSE);
		if (hte)
			for (hash_table_entry_usage *hteu = hte->first_usage; hteu; hteu = hteu->next_usage) {
				if (hteu->form_of_usage & ELEMENT_ACCESS_USAGE) {
					d++;
					if (hteu->usage_recorded_at->under_section != str->typedef_begins->owning_section) {
						if (hteu->usage_recorded_at->under_section->scratch_flag == 0) c++;
						hteu->usage_recorded_at->under_section->scratch_flag = 1;
					}
				}
			}
		if ((report) && (d == 0) && (elt->allow_sharing == FALSE))
			Main::error_in_web("element is never used", 
				elt->element_created_at);
	}
	return c;
}
