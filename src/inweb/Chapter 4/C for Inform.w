[CForInform::] C for Inform.

@Purpose: To provide a convenient extension to C syntax for the C-for-Inform
language, which is likely never to be used for any program other than the
Inform 7 compiler.

@p Preform.
The most substantial addition in C for Inform is that it can contain
Preform grammar declarations; see the Inform source code for its syntax.

In parsing, we categorise the opening lines |PREFORM_LCAT|. Subsequent lines
of grammar are |PREFORM_GRAMMAR_LCAT|; but the lines of code inside an
internal definition (i.e., after the opening line) are just plain
|CODE_BODY_LCAT| lines.

So here are the structures to represent these concepts.

@d INFINITE_WORD_COUNT 1000000000

@c
typedef struct preform_nonterminal {
	string nt_name; /* e.g., |"<action-clause>"| */
	string unangled_name; /* e.g., |"action-clause"| */
	string as_C_identifier; /* e.g., |"action_clause_NTM"| */
	int as_function; /* defined internally, that is, parsed by a C function */
	int voracious; /* a voracious nonterminal: see {\it The English Syntax of Inform} */
	int min_word_count; /* for internals only */
	int max_word_count;
	int takes_pointer_result; /* right-hand formula defines |*XP|, not |*X| */
	struct source_line *where_defined;
	struct preform_nonterminal *next_pnt_alphabetically;
	MEMORY_MANAGEMENT
} preform_nonterminal;

typedef struct nonterminal_variable {
	string ntv_name; /* e.g., |"num"| */
	string ntv_type; /* e.g., |"int"| */
	string ntv_identifier; /* e.g., |"num_NTMV"| */
	struct source_line *first_mention; /* first usage */
	MEMORY_MANAGEMENT
} nonterminal_variable;

@

@c
preform_nonterminal *first_pnt_alphabetically = NULL;

theme_tag *Preform_theme = NULL;

@p Bibliographic extras.
A special case for C-for-Inform only is that |[[nonterminals]]| expands to a
function call to the routine automatically created by Inweb for setting up
the Preform nonterminals.

@c
int CForInform::special_data(OUTPUT_STREAM, char *data) {
	if (CStrings::eq(data, "nonterminals")) {
		WRITE("register_tangled_nonterminals();\n");
		return TRUE;
	}
	return FALSE;
}

@p Parsing extras.
This is where we look for declarations of nonterminals.

@d NOT_A_NONTERMINAL -4
@d A_FLEXIBLE_NONTERMINAL -3
@d A_VORACIOUS_NONTERMINAL -2
@d A_GRAMMAR_NONTERMINAL -1

@c
void CForInform::further_parsing(web *W) {
	LOOP_WITHIN_TANGLE(W->first_target) {
		int form = NOT_A_NONTERMINAL; /* one of the four values above, or a non-negative word count */
		string pntname; CStrings::copy(pntname, "");
		string header; CStrings::copy(header, "");
		@<Parse a Preform nonterminal header line@>;
		if (form != NOT_A_NONTERMINAL) {
			preform_nonterminal *pnt = CREATE(preform_nonterminal);
			pnt->where_defined = L;
			CStrings::copy(pnt->nt_name, pntname);
			CStrings::copy(pnt->unangled_name, pntname + 1);
			CStrings::truncate(pnt->unangled_name, CStrings::len(pnt->unangled_name)-1);
			pnt->next_pnt_alphabetically = NULL;
			if (first_pnt_alphabetically == NULL) first_pnt_alphabetically = pnt;
			else {
				int placed = FALSE;
				preform_nonterminal *last = NULL;
				for (preform_nonterminal *seq = first_pnt_alphabetically; seq;
					seq = seq->next_pnt_alphabetically) {
					if (CStrings::cmp(pntname, seq->nt_name) < 0) {
						if (seq == first_pnt_alphabetically) {
							pnt->next_pnt_alphabetically = first_pnt_alphabetically;
							first_pnt_alphabetically = pnt;
						} else {
							last->next_pnt_alphabetically = pnt;
							pnt->next_pnt_alphabetically = seq;
						}
						placed = TRUE;
						break;
					}
					last = seq;
				}
				if (placed == FALSE) last->next_pnt_alphabetically = pnt;
			}
			@<Compose a C identifier for the nonterminal@>;
			@<Work out the parsing characteristics of the nonterminal@>;
			L->preform_nonterminal_defined = pnt;
			if (Preform_theme) Parser::add_tag_to_para(L->owning_paragraph, Preform_theme, NULL);
			L->category = PREFORM_LCAT;
			L->text_operand = Memory::new_string(header);
		}
	}
}

@ The keyword "internal" can be followed by an indication of the number
of words the nonterminal will match: usually a decimal non-negative number,
but optionally a question mark |?| to indicate voracity.

@<Parse a Preform nonterminal header line@> =
	string found_text1;
	string found_text2;	
	string found_text3;
	if (ISORegexp::match_1(L->text, "(<%p+>) ::=%c*", found_text1)) {
		form = A_GRAMMAR_NONTERMINAL;
		CStrings::copy(pntname, found_text1);
		CStrings::copy(header, found_text1);
		@<Parse the subsequent lines as Preform grammar@>;
	} else if (ISORegexp::match_2(L->text, "((<%p+>) internal %?) {%c*", found_text1, found_text2)) {
		form = A_VORACIOUS_NONTERMINAL;
		CStrings::copy(pntname, found_text2);
		CStrings::copy(header, found_text1);
	} else if (ISORegexp::match_2(L->text, "((<%p+>) internal) {%c*", found_text1, found_text2)) {
		form = A_FLEXIBLE_NONTERMINAL;
		CStrings::copy(pntname, found_text2);
		CStrings::copy(header, found_text1);
	} else if (ISORegexp::match_3(L->text, "((<%p+>) internal (%d+)) {%c*",
						found_text1, found_text2, found_text3)) {
		form = atoi(found_text3);
		CStrings::copy(pntname, found_text2);
		CStrings::copy(header, found_text1);
	}

@ After a line like |<action-clause> ::=|, the grammar follows on subsequent
lines until we hit the end of the paragraph, or a white-space line, whichever
comes first. Each line of grammar is categorised |PREFORM_GRAMMAR_LCAT|.
If we have a line with an arrow, like so:

	|porcupine tree  ==>  2|

then the text on the left goes into |text_operand| and the right into
|text_operand2|, with the arrow itself (and white space around it) cut out.

@<Parse the subsequent lines as Preform grammar@> =
	source_line *AL;
	for (AL = L; (AL) && (AL->category == CODE_BODY_LCAT); AL = AL->next_line) {
		if (ISORegexp::string_is_white_space(AL->text)) break;
		AL->category = PREFORM_GRAMMAR_LCAT;
		
		string found_text1;
		string found_text2;	
		if (ISORegexp::match_2(AL->text, "(%c+) ==> (%c*)", found_text1, found_text2)) {
			AL->text_operand = Memory::new_string(found_text1);
			AL->text_operand2 = Memory::new_string(found_text2);
		} else {
			AL->text_operand = AL->text;
			AL->text_operand2 = "";
		}
		
		@<Remove any C comment from the left side of the arrow@>;
		@<Detect any nonterminal variables being set on the right side of the arrow@>;
	}

@ In case we have a comment at the end of the grammar, like this:

	|porcupine tree  /* what happens now? */|

we want to remove it. The regular expression here isn't terribly legible, but
trust me, it's correct.

@<Remove any C comment from the left side of the arrow@> =
	string found_text1;
	if (ISORegexp::match_1(AL->text_operand, "(%c*)%/%*%c*%*%/ *", found_text1))
		AL->text_operand = Memory::new_string(found_text1);

@ Note that nonterminal variables are, by default, integers. If their names
are divided internally with a colon, however, as |<<structure:name>>|, then
they have the type |structure *|.

@<Detect any nonterminal variables being set on the right side of the arrow@> =
	string to_scan; CStrings::copy(to_scan, AL->text_operand2);
	string found_text1;
	string found_text2;	
	while (ISORegexp::match_2(to_scan, "%c*?<<(%P+?)>> =(%c*)", found_text1, found_text2)) {
		string var_given; CStrings::copy(var_given, found_text1);
		string type_given; CStrings::copy(type_given, "int");
		CStrings::copy(to_scan, found_text2);
		if (ISORegexp::match_1(var_given, "(%p+):%p+", found_text1)) {
			CSTRING_WRITE(type_given, "%s *", found_text1);
		}
		nonterminal_variable *ntv;
		LOOP_OVER(ntv, nonterminal_variable)
			if (CStrings::eq(ntv->ntv_name, var_given))
				break;
		if (ntv == NULL) @<This one's new, so create a new nonterminal variable@>;
	}

@ Nonterminal variables are actually just global C variables, and their C
identifiers need to avoid hyphens and colons. For example, |<<kind:ref>>|
has identifier |"kind_ref_NTMV"|.

@<This one's new, so create a new nonterminal variable@> =
	ntv = CREATE(nonterminal_variable);
	CStrings::copy(ntv->ntv_name, var_given);
	CStrings::copy(ntv->ntv_type, type_given);
	for (int i=0; var_given[i]; i++)
		if ((var_given[i] == '-') || (var_given[i] == ':'))
			CStrings::set_char(var_given, i, '_');
	CSTRING_WRITE(ntv->ntv_identifier, "%s_NTMV", var_given);
	ntv->first_mention = AL;

@ When the program we are tangling is eventually running, each nonterminal
will be represented by a pointer to a unique data structure for it. Inweb
automatically compiles code to create these pointers; and here's how it
works out their names.

@<Compose a C identifier for the nonterminal@> =
	CStrings::copy(pnt->as_C_identifier, pnt->nt_name + 1);
	for (int i=0; pnt->as_C_identifier[i]; i++) {
		if (pnt->as_C_identifier[i] == '-') CStrings::set_char(pnt->as_C_identifier, i, '_');
		if (pnt->as_C_identifier[i] == '>') CStrings::truncate(pnt->as_C_identifier, i);
	}
	CStrings::concatenate(pnt->as_C_identifier, "_NTM");

@

@<Work out the parsing characteristics of the nonterminal@> =
	pnt->voracious = FALSE; if (form == A_VORACIOUS_NONTERMINAL) pnt->voracious = TRUE;
	pnt->as_function = TRUE; if (form == A_GRAMMAR_NONTERMINAL) pnt->as_function = FALSE;

	pnt->takes_pointer_result = FALSE;
	if (ISORegexp::match_0(pnt->nt_name, "<k-%c+")) pnt->takes_pointer_result = TRUE;
	if (ISORegexp::match_0(pnt->nt_name, "<s-%c+")) pnt->takes_pointer_result = TRUE;

	int min = 1, max = form;
	if (form < 0) max = INFINITE_WORD_COUNT;
	if (max == 0) min = 0;
	else if (max != INFINITE_WORD_COUNT) min = max;
	pnt->min_word_count = min;
	pnt->max_word_count = max;

@p Tangling extras.
First, at the predeclarations stage. C for Inform is going to create a special
function, right at the end of the code, which "registers" the nonterminals,
creating their run-time data structures; we must predeclare this function.
It will set values for the pointers |action_clause_NTM|, and so on; these
are global variables, which we initially declare as |NULL|.

We also declare the nonterminal variables like |kind_ref_NTMV|, initialising
all integers to zero and all pointers to |NULL|.

@c
void CForInform::additional_predeclarations(OUTPUT_STREAM, web *W) {
	LOOP_WITHIN_TANGLE(W->first_target)
		if (L->preform_nonterminal_defined) {
			preform_nonterminal *pnt = L->preform_nonterminal_defined;
			Languages::insert_line_marker(OUT, W->main_language, L);
			WRITE("nonterminal *%s = NULL;\n", pnt->as_C_identifier);
		}

	nonterminal_variable *ntv;
	LOOP_OVER(ntv, nonterminal_variable)
		WRITE("%s %s = %s;\n",
			ntv->ntv_type, ntv->ntv_identifier,
			(CStrings::eq(ntv->ntv_type, "int"))?"0":"NULL");

	WRITE("void register_tangled_nonterminals(void);\n");
}

@ And here is the promised routine, which appears at the very end of the code.
It makes use of macros and data structures defined in the Inform 7 web.

@c
void CForInform::gnabehs(OUTPUT_STREAM, web *W) {
	WRITE("void register_tangled_nonterminals(void) {\n");
	LOOP_WITHIN_TANGLE(W->first_target)
		if (L->preform_nonterminal_defined) {
			preform_nonterminal *pnt = L->preform_nonterminal_defined;
			Languages::insert_line_marker(OUT, W->main_language, L);
			if (pnt->as_function) {
				WRITE("\tINTERNAL_NONTERMINAL(\"%s\", %s, %d, %d);\n",
					pnt->nt_name, pnt->as_C_identifier,
					pnt->min_word_count, pnt->max_word_count);
				WRITE("\t%s->voracious = %d;\n",
					pnt->as_C_identifier, pnt->voracious);
			} else {
				WRITE("\tREGISTER_NONTERMINAL(\"%s\", %s);\n",
					pnt->nt_name, pnt->as_C_identifier);
			}
		}
	WRITE("}\n");
}

@p Tangling special line categories.
That's it for big structural additions to the tangled C code. Now we turn
to how to tangle the lines we've given special categories to.

We need to tangle |PREFORM_LCAT| lines (those holding nonterminal declarations)
in a special way...

@c
int CForInform::will_insert_in_tangle(source_line *L) {
	if (L->category == PREFORM_LCAT) return TRUE;
	return FALSE;
}

@ ...and this is how. As can be seen, each nonterminal turns into a C function.
In the case of an internal definition, like

	|<k-kind-for-template> internal {|

we tangle this opening line to

	|int k_kind_for_template_NTM(wording W, int *X, void **XP) {|

that is, to a function which returns |TRUE| if it makes a match on the text
excerpt in Inform's source text, |FALSE| otherwise; if it matches and produces
an integer and/or pointer result, these are copied into |*X| and |*XP|. The
remaining lines of the function are tangled unaltered, i.e., following the
same rules as for the body of any other C function.

@c
void CForInform::insert_in_tangle(OUTPUT_STREAM, source_line *L) {
	preform_nonterminal *pnt = L->preform_nonterminal_defined;
	if (pnt->as_function) {
		WRITE("int %sR(wording W, int *X, void **XP) {\n",
			pnt->as_C_identifier);
	} else {
		WRITE("int %sC(int *X, void **XP, int *R, void **RP, wording W) {\n",
			pnt->as_C_identifier);
		@<Compile the body of the compositor function@>;
		WRITE("}\n");
	}
}

@ On the other hand, a grammar nonterminal tangles to a "compositor function".
Thus the opening line

	|<action-clause> ::=|

tangles to a function header:

	|int action_clause_NTMC(int *X, void **XP, int *R, void **RP, int w1, int w2) {|

Subsequent lines of the nonterminal are categorised |PREFORM_GRAMMAR_LCAT|
and thus won't tangle to code at all, by the usual rules; so we tangle from
them directly here.

Composition is what happens {\it after} a successful match of the text in the
word range |w1| to |w2|. The idea is that, especially if the pattern was
complicated, we will need to "compose" the results of parsing individual
pieces of it into a result for the whole. These partial results can be found
in the arrays |R[n]| and |RP[n]| passed as parameters; recall that every
nonterminal has in principle {\it both} an integer {\it and} a pointer result,
though often one or both is undefined.

A simple example would be

	|<cardinal-number> + <cardinal-number> ==> R[1] + R[2]|

where the composition function would be called on a match of, say, "$5 + 7$",
and would find the values 5 and 7 in |R[1]| and |R[2]| respectively. It would
then add these together, store 12 in |*X|, and return |TRUE| to show that all
was well.

A more typical example, drawn from the actual Inform 7 web, is:

	|<k-kind-of-kind> <k-formal-kind-variable> ==> Kinds::variable_construction(R[2], RP[1])|

which says that the composite result -- the right-hand formula -- is formed by
calling a particular routine on the integer result of subexpression 2
(|<k-formal-kind-variable>|) and the pointer result of subexpression 1
(|<k-kind-of-kind>|). The answer, the composite result, that is, must be
placed in |*X| and |*XP|. (Composition functions are also allowed to
invalidate the result, by returning |FALSE|, and have other tricks up their
sleeves, but none of that is handled by Inweb: see the Inform 7 web for more
on this.)

@<Compile the body of the compositor function@> =
	int needs_collation = FALSE;
	for (source_line *AL = L->next_line;
		((AL) && (AL->category == PREFORM_GRAMMAR_LCAT));
		AL = AL->next_line)
			if (AL->text_operand2[0])
				needs_collation = TRUE;
	if (needs_collation) @<At least one of the grammar lines provided an arrow and formula@>
	else @<None of the grammar lines provided an arrow and formula@>;
	WRITE("\treturn TRUE;\n");

@ In the absence of any |==>| formulae, we simply set |*X| to the default
result supplied; this is the production number within the grammar (0 for the
first line, 1 for the second, and so on) by default, with an undefined pointer.

@<None of the grammar lines provided an arrow and formula@> =
	WRITE("\t*X = R[0];\n");

@

@<At least one of the grammar lines provided an arrow and formula@> =
	WRITE("\tswitch(R[0]) {\n");
	int c = 0;
	for (source_line *AL = L->next_line;
		((AL) && (AL->category == PREFORM_GRAMMAR_LCAT));
		AL = AL->next_line, c++) {
		char *formula = AL->text_operand2;
		if (formula[0]) {
			WRITE("\t\tcase %d: ", c);
			@<Tangle the formula on the right-hand side of the arrow@>;
			WRITE(";\n");
			WRITE("#pragma clang diagnostic push\n");
			WRITE("#pragma clang diagnostic ignored \"-Wunreachable-code\"\n");
			WRITE("break;\n");
			WRITE("#pragma clang diagnostic pop\n");
		}
	}
	WRITE("\t\tdefault: *X = R[0]; break;\n");
	WRITE("\t}\n");

@ We assume that the RHS of the arrow is an expression to be evaluated,
and that it produces an integer or a pointer according to what the
non-terminal expects as its main result. But we make one exception: if
the formula begins with a CWEB macro, then it can't be an expression,
and instead we read it as code in a void context. (This code will, we
assume, set |*X| and/or |*XP| in some ingenious way of its own.)

Within the body of the formula, we allow a pseudo-macro to work: |WR[n]|
expands to word range |n| in the match which we're compositing. This actually
expands like so:

	|action_clause_NTM->range_result[n]|

which saves a good deal of typing. (A regular C preprocessor macro couldn't
easily do this, because it needs to include the identifier name of the
nonterminal being parsed.)

@<Tangle the formula on the right-hand side of the arrow@> =
	if (!ISORegexp::match_0(formula, "@<%c*")) {
		if (pnt->takes_pointer_result) WRITE("*XP = ");
		else WRITE("*X = ");
	}
	string expanded; CStrings::copy(expanded, "");
	for (int i=0; formula[i]; i++) {
		if ((formula[i] == 'W') && (formula[i+1] == 'R') &&
			(formula[i+2] == '[') && (isdigit(formula[i+3])) && (formula[i+4] == ']')) {
				CSTRING_WRITE(expanded+CStrings::len(expanded),
					"%s->range_result[%c]", pnt->as_C_identifier, formula[i+3]);
				i += 4;
		} else {
			CSTRING_WRITE(expanded+CStrings::len(expanded), "%c", formula[i]);
		}
	}
	Tangler::tangle_code(OUT, expanded, AL->owning_section, AL);

@p Tangling typical C code.
Going down from line level to the tangling of little excerpts of C code,
we also provide for some other special extensions to C.

@c
void CForInform::tangle_code(OUTPUT_STREAM, char *original) {
	int fcall_pos = -1;
	for (int i = 0; original[i]; i++) {
		@<Double-colons are namespace dividers in function names@>;
		if (original[i] == '<') {
			if (original[i+1] == '<') {
				@<Double-angles sometimes delimit Preform variable names@>;
			} else {
				@<Single-angles sometimes delimit Preform nonterminal names@>;
			}
		}
		if (i == fcall_pos) {
			fcall_pos = -1;
			WRITE(", NULL, NULL");
		}
		PUT(original[i]);
	}
}

@ For example, a function name like:

	|Text::Parsing::get_next|

must be rewritten as

	|Text__Parsing__get_next|

since colons aren't valid in C identifiers. The following is prone to all
kinds of misreadings, of course; it picks up any use of |::| between an
alphanumberic character and a letter. In particular, code like

	|printf("Trying Text::Parsing::get_next now.\n");|

will be rewritten as

	|printf("Trying Text__Parsing__get_next now.\n");|

This is probably unwanted, but it doesn't matter, because these Inform-only
extension features of Inweb aren't intended for general use: only for
Inform, where no misreadings occur.

@<Double-colons are namespace dividers in function names@> =
	if ((i > 0) && (original[i] == ':') && (original[i+1] == ':') &&
		(isalpha(original[i+2])) && (isalnum(original[i-1]))) {
		WRITE("__"); i++;
		continue;
	}

@ Angle brackets around a valid Preform variable name expand into its
C identifier; for example, |<<R>>| becomes |most_recent_result|.
We take no action if it's not a valid name, so |<<fish>>| becomes
just |<<fish>>|.

@<Double-angles sometimes delimit Preform variable names@> =
	string found_text1;
	if (ISORegexp::match_1(original+i, "<<(%P+)>>%c*", found_text1)) {
		string putative; CStrings::copy(putative, found_text1);
		char *pv_identifier = CForInform::nonterminal_variable_identifier(putative);
		if (pv_identifier) {
			WRITE("%s", pv_identifier);
			i += CStrings::len(putative) + 3;
			continue;
		}
	}

@ Similarly for nonterminals; |<k-kind>| might become |k_kind_NTM|.
Here, though, there's a complication:

	|if (<k-kind>(W)) { ...|

must expand to:

	|if (Text__Languages__parse_nt_against_word_range(k_kind_NTM, W, NULL, NULL)) { ...|

This is all syntactic sugar to make it easier to see parsing in action.
Anyway, it means we have to set |fcall_pos| to remember to add in the
two |NULL| arguments when we hit the |)| a little later. We're doing all
of this fairly laxly, but as before: it only needs to work for Inform,
and Inform doesn't cause any trouble.

@<Single-angles sometimes delimit Preform nonterminal names@> =
	string found_text1;
	if (ISORegexp::match_1(original+i, "(<%p+>)%c*", found_text1)) {
		string putative; CStrings::copy(putative, found_text1);
		preform_nonterminal *pnt = CForInform::nonterminal_by_name(putative);
		if (pnt) {
			i += CStrings::len(putative) - 1;
			if (original[i+1] == '(') {
				int arity = 1;
				for (int j = i+2, bl = 1; ((original[j]) && (bl > 0)); j++) {
					if (original[j] == '(') bl++;
					if (original[j] == ')') { bl--; if (bl == 0) fcall_pos = j; }
					if ((original[j] == ',') && (bl == 1)) arity++;
				}
				WRITE("Preform__parse_nt_against_word_range(");
			}
			WRITE("%s", pnt->as_C_identifier);
			if (fcall_pos >= 0) {
				WRITE(", "); i++;
			}
			continue;
		}
	}

@ We needed two little routines to find nonterminals and their variables by
name. They're not very efficient, but experience shows that even on a web
the size of Inform 7, there's no significant gain from speeding them up
(with, say, a hash table).

@c
preform_nonterminal *CForInform::nonterminal_by_name(char *name) {
	preform_nonterminal *pnt;
	LOOP_OVER(pnt, preform_nonterminal)
		if (CStrings::eq(name, pnt->nt_name))
			return pnt;
	return NULL;
}

@ The special variables |<<R>>| and |<<RP>>| hold the results,
integer and pointer, for the most recent successful match. They're defined
in the Inform 7 web (see the code for parsing text against Preform grammars),
not by Inweb.

@c
char *CForInform::nonterminal_variable_identifier(char *name) {
	if (CStrings::eq(name, "r")) return "most_recent_result";
	if (CStrings::eq(name, "rp")) return "most_recent_result_p";
	nonterminal_variable *ntv;
	LOOP_OVER(ntv, nonterminal_variable)
		if (CStrings::eq(ntv->ntv_name, name))
			return ntv->ntv_identifier;
	return NULL;
}

@p Additional tangling.
We saw above that the grammar lines following a non-internal declaration
were divided into actual grammar, then an arrow, then a formula. The formulae
were tangled into "composition functions", but the grammar itself was
simply thrown away. It doesn't appear anywhere in the C code tangled by
Inweb.

So what does happen to it? The answer is that it's transcribed into an
auxiliary file called "Syntax.preform", which Inform, once it is compiled,
will read in at run-time. This is how that happens:

@c
void CForInform::additional_tangling(programming_language *pl, web *W, tangle_target *target) {
	if (NUMBER_CREATED(preform_nonterminal) > 0) {
		pathname *P = Pathnames::subfolder(W->path_to_web, "Tangled");
		filename *Syntax = Filenames::in_folder(P, "Syntax.preform");

		text_stream TO_struct;
		text_stream *OUT = &TO_struct;
		if (STREAM_OPEN_TO_FILE(OUT, Syntax, ISO_ENC) == FALSE)
			Errors::fatal_with_file("unable to write Preform file", Syntax);

		WRITE_TO(STDOUT, "Writing Preform syntax to: %/f\n", Syntax);

		WRITE("[This is English.preform, generated by inweb: do not edit.]\n\n");
		WRITE("language English\n");

		@<Actually write out the Preform syntax@>;
		STREAM_CLOSE(OUT);
	}
}

@ See the {\it English Syntax of Inform} document for a heavily annotated
form of the result of the following. Note a useful convention: if the
right-hand side of the arrow in a grammar line uses a CWEB macro which
mentions a problem message, then we transcribe a Preform comment to that
effect. (This really is a comment: Inform ignores it, but it makes the
file more comprehensible to human eyes.) For example,

	|<article> kind ==> @<Issue C8PropertyOfKind problem@>|

(The code in this CWEB macro will indeed issue this problem message, we
assume.)

@<Actually write out the Preform syntax@> =
	LOOP_WITHIN_TANGLE(target)
		if (L->category == PREFORM_LCAT) {
			preform_nonterminal *pnt = L->preform_nonterminal_defined;
			if (pnt->as_function) 
				WRITE("\n%s internal\n", pnt->nt_name);
			else
				WRITE("\n%s ::=\n", L->text_operand);
			for (source_line *AL = L->next_line;
				((AL) && (AL->category == PREFORM_GRAMMAR_LCAT));
				AL = AL->next_line) {
				WRITE("%s", AL->text_operand);
				string found_text1;
				if (ISORegexp::match_1(AL->text_operand2, "%c+Issue (%c+) problem%c+", found_text1))
					WRITE("[issues %s]", found_text1);
				WRITE("\n");
			}
		}

@p Indexing Preform grammar.

@c
void CForInform::weave_grammar_index(OUTPUT_STREAM) {
	WRITE("\\raggedright\\tolerance=10000");
	preform_nonterminal *pnt;
	for (pnt = first_pnt_alphabetically; pnt;
		pnt = pnt->next_pnt_alphabetically) {
		WRITE("\\line{\\nonterminal{%s}%s"
			"\\leaders\\hbox to 1em{\\hss.\\hss}\\hfill {\\xreffont %s}}\n",
			pnt->unangled_name,
			(pnt->as_function)?" (internal)":"",
			pnt->where_defined->owning_section->sigil);
		int said_something = FALSE;
		@<List where the nonterminal appears in other Preform declarations@>;
		@<List where the nonterminal is called from Inform code@>;
		if (said_something == FALSE)
			WRITE("\\par\\hangindent=3em{\\it unused}\n\n");
	}
	WRITE("\\penalty-1000\n");
	WRITE("\\smallbreak\n");
	WRITE("\\hrule\\smallbreak\n");
}

@

@<List where the nonterminal is called from Inform code@> =
	section *S;
	LOOP_OVER(S, section) S->scratch_flag = FALSE;
	hash_table_entry *hte = Analyser::find_hash_entry(pnt->where_defined->owning_section, pnt->unangled_name, FALSE);
	for (hash_table_entry_usage *hteu = hte->first_usage; hteu; hteu = hteu->next_usage)
		if (hteu->form_of_usage & PREFORM_IN_CODE_USAGE)
			hteu->usage_recorded_at->under_section->scratch_flag = TRUE;
	int use_count = 0;
	LOOP_OVER(S, section)
		if (S->scratch_flag)
			use_count++;
	if (use_count > 0) {
		said_something = TRUE;
		WRITE("\\par\\hangindent=3em{\\it called from} ");
		int c = 0;
		LOOP_OVER(S, section)
			if (S->scratch_flag) {
				if (c++ > 0) WRITE(", ");
				WRITE("{\\xreffont %s}", S->sigil);
			}
		WRITE("\n\n");
	}

@

@<List where the nonterminal appears in other Preform declarations@> =
	section *S;
	LOOP_OVER(S, section) S->scratch_flag = FALSE;
	hash_table_entry *hte = Analyser::find_hash_entry(pnt->where_defined->owning_section, pnt->unangled_name, FALSE);
	for (hash_table_entry_usage *hteu = hte->first_usage; hteu; hteu = hteu->next_usage)
		if (hteu->form_of_usage & PREFORM_IN_GRAMMAR_USAGE)
			hteu->usage_recorded_at->under_section->scratch_flag = TRUE;
	int use_count = 0;
	LOOP_OVER(S, section)
		if (S->scratch_flag)
			use_count++;
	if (use_count > 0) {
		said_something = TRUE;
		WRITE("\\par\\hangindent=3em{\\it used by other nonterminals in} ");
		int c = 0;
		LOOP_OVER(S, section)
			if (S->scratch_flag) {
				if (c++ > 0) WRITE(", ");
				WRITE("{\\xreffont %s}", S->sigil);
			}
		WRITE("\n\n");
	}

@p Weaving.


@c
void CForInform::new_tag_declared(theme_tag *tag) {
	if (CStrings::eq(tag->tag_name, "Preform")) Preform_theme = tag;
}

int skipping_internal = FALSE, preform_production_count = 0;

int CForInform::skip_in_weaving(weave_target *wv, source_line *L) {
	if ((Preform_theme) && (wv->theme_match == Preform_theme)) {
		if (ISORegexp::match_0(L->text, "}%c*")) { skipping_internal = FALSE; return TRUE; }
		if (skipping_internal) return TRUE;
		if (ISORegexp::match_0(L->text, "<%c*?> internal%c*")) skipping_internal = TRUE;
	}
	return FALSE;
}

void CForInform::begin_weave(section *S, weave_target *wv) {
}

@

@c
int CForInform::syntax_colour(OUTPUT_STREAM, weave_target *wv,
	web *W, chapter *C, section *S, source_line *L, char *matter, char *colouring) {
	return FALSE;
}

int CForInform::weave_code_line(OUTPUT_STREAM, weave_target *wv,
	web *W, chapter *C, section *S, source_line *L, char *matter, char *concluding_comment) {
	if ((Preform_theme) && (wv->theme_match == Preform_theme)) {
		if (L->preform_nonterminal_defined) preform_production_count = 0;
		if (L->preform_nonterminal_defined) {
			WRITE("\\nonterminal{%s} |::=|",
				L->preform_nonterminal_defined->unangled_name);
			if (L->preform_nonterminal_defined->as_function) {
				WRITE("\\quad{\\it internal definition");	
				if (L->preform_nonterminal_defined->voracious)
					WRITE(" (voracious)");
				else if (L->preform_nonterminal_defined->min_word_count ==
					L->preform_nonterminal_defined->min_word_count)
					WRITE(" (%d word%s)",
						L->preform_nonterminal_defined->min_word_count,
						(L->preform_nonterminal_defined->min_word_count != 1)?"s":"");
				WRITE("}");
			}
			WRITE("\n");
			return TRUE;
		} else {
			if (L->category == PREFORM_GRAMMAR_LCAT) {
				string problem; CStrings::copy(problem, "");
				string found_text1;
				string found_text2;	
				string found_text3;	
				if (ISORegexp::match_1(matter, "Issue (%c*?) problem", found_text1)) CStrings::copy(problem, found_text1);
				if (ISORegexp::match_0(matter, "FAIL_NONTERMINAL %+")) CStrings::copy(problem, "fail and skip");
				else if (ISORegexp::match_0(matter, "FAIL_NONTERMINAL")) CStrings::copy(problem, "fail");
				preform_production_count++;
				CSTRING_WRITE(matter, "|%s|", L->text_operand);
				while (ISORegexp::match_2(matter, "(%c+?)|(%c+)", found_text1, found_text2))
					CSTRING_WRITE(matter, "%s___stroke___%s",
						found_text1, found_text2);
				while (ISORegexp::match_2(matter, "(%c*?)___stroke___(%c*)", found_text1, found_text2))
					CSTRING_WRITE(matter, "%s|\\||%s",
						found_text1, found_text2);
				while (ISORegexp::match_3(matter, "(%c*)<(%c*?)>(%c*)",
						found_text1, found_text2, found_text3))
					CSTRING_WRITE(matter, "%s|\\nonterminal{%s}|%s",
						found_text1, found_text2, found_text3);
				string label; CStrings::copy(label, "");
				int N = preform_production_count;
				int L = ((N-1)%26) + 1;
				if (N <= 26) CSTRING_WRITE(label, "%c", 'a'+L-1);
				else if (N <= 52) CSTRING_WRITE(label, "%c%c", 'a'+L-1, 'a'+L-1);
				else if (N <= 78) CSTRING_WRITE(label, "%c%c%c", 'a'+L-1, 'a'+L-1, 'a'+L-1);
				else {
					int n = (N-1)/26;
					CSTRING_WRITE(label, "%c${}^{%d}$", 'a'+L-1, n);
				}
				WRITE("\\qquad {\\hbox to 0.4in{\\it %s\\hfil}}%s", label, matter);
				if (problem[0])
					WRITE("\\hfill$\\longrightarrow$ {\\ttninepoint\\it %s}", problem);
				else if (concluding_comment[0]) {
					WRITE(" \\hfill{\\ttninepoint\\it ");
					if (concluding_comment) Formats::identifier(OUT, wv, concluding_comment);
					WRITE("}");
				}
				WRITE("\n");
				return TRUE;
			}
		}
	}
	return FALSE;
}

@p Analysis.

@c
void CForInform::analyse_code(programming_language *pl, web *W) {
	preform_nonterminal *pnt;
	LOOP_OVER(pnt, preform_nonterminal)
		Analyser::find_hash_entry(pnt->where_defined->owning_section, pnt->unangled_name, TRUE);
	LOOP_WITHIN_TANGLE(W->first_target) {
		if (L->category == INTERFACE_BODY_LCAT) {
			string found_text1;
			string found_text2;
			if (ISORegexp::match_2(L->text, "-- Defines {-(%c+?):(%c+)} *", found_text1, found_text2)) {
				L->interface_line_identified = TRUE;
				int form = 0;
				if (CStrings::eq(found_text1, "call")) form = 1;
				if (CStrings::eq(found_text1, "callv")) form = 2;
				if (CStrings::eq(found_text1, "routine")) form = 3;
				if (CStrings::eq(found_text1, "array")) form = 4;
				if (form == 0) {
					Main::error_in_web("bad {-format:...}", L);
					continue;
				}
				string function_name; CStrings::copy(function_name, "");
				switch (form) {
					case 1: case 2: CStrings::copy(function_name, found_text2); break;
					case 3: CSTRING_WRITE(function_name, "%s_routine", found_text2); break;
					case 4: CSTRING_WRITE(function_name, "%s_array", found_text2); break;
				}
				function *fn = CLike::get_function_with_name(function_name);
				if (fn == NULL) {
					Main::error_in_web("no such function exists", L);
					fprintf(stderr, "  >> %s\n", function_name);
					continue;
				}
				fn->called_from_other_sections = TRUE;
			}
		}
	}
}
