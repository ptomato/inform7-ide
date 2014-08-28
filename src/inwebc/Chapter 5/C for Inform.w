5/cfori: C for Inform.

@Purpose: To provide a convenient extension to C syntax for the C-for-Inform
language, which is likely never to be used for any program other than the
Inform 7 compiler.

@Definitions:

@ The most substantial addition in C for Inform is that it can contain
Preform grammar declarations. These can't, of course, be tangled directly
into C code. Each one declares a "nonterminal", and there are two ways
they can be declared: as grammar, or as an "internal" routine of C code.

This example creates two nonterminals, |<action-clause>| and |<k-kind-for-template>|,
and refers to a third, |<action-applications>|. Note also the ``nonterminal
variable'' |<<num>>|, a place to put results; such variables do not need to
be predeclared. (Well, in fact they do, but the tangler handles it automatically.)

The grammar lines under |<action-clause>| are internally divided by arrows
|==>|. The text to the left and to the right of these arrows ends up in very
different places, as we shall see. For the meaning of the syntax to the left,
see {\it The English Syntax of Inform}.

In parsing, we categorise the opening lines |PREFORM_LCAT|. Subsequent lines
of grammar are |PREFORM_GRAMMAR_LCAT|; but the lines of code inside an
internal definition (i.e., after the opening line) are just plain
|CODE_BODY_LCAT| lines.

@x
<action-clause> ::=
	out of world |						==> OOW_ACT_CLAUSE
	abbreviable |						==> ABBREV_ACT_CLAUSE
	with past participle ... |			==> PP_ACT_CLAUSE
	applying to <action-applications> |	==> APPLYING_ACT_CLAUSE; <<num>> = R[1]
	requiring light						==> LIGHT_ACT_CLAUSE

<k-kind-for-template> internal {
	/* some code here */
	return FALSE;
}

@ So here are the structures to represent these concepts.

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

@-------------------------------------------------------------------------------

@p Bibliographic extras.
A special case for C-for-Inform only is that |[[nonterminals]]| expands to a
function call to the routine automatically created by Inweb for setting up
the Preform nonterminals.

@c
int c_for_inform_special_data(FILE *F, char *data) {
	if (in_string_eq(data, "nonterminals")) {
		fprintf(F, "register_tangled_nonterminals();\n");
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
void c_for_inform_further_parsing(web *W) {
	LOOP_WITHIN_TANGLE(W->first_target) {
		int form = NOT_A_NONTERMINAL; /* one of the four values above, or a non-negative word count */
		string pntname; in_strcpy(pntname, "");
		string header; in_strcpy(header, "");
		@<Parse a Preform nonterminal header line@>;
		if (form != NOT_A_NONTERMINAL) {
			preform_nonterminal *pnt = CREATE(preform_nonterminal);
			pnt->where_defined = L;
			in_strcpy(pnt->nt_name, pntname);
			in_strcpy(pnt->unangled_name, pntname + 1);
			in_truncate(pnt->unangled_name, in_strlen(pnt->unangled_name)-1);
			pnt->next_pnt_alphabetically = NULL;
			if (first_pnt_alphabetically == NULL) first_pnt_alphabetically = pnt;
			else {
				int placed = FALSE;
				preform_nonterminal *last = NULL;
				for (preform_nonterminal *seq = first_pnt_alphabetically; seq;
					seq = seq->next_pnt_alphabetically) {
					if (in_string_cmp(pntname, seq->nt_name) < 0) {
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
			if (Preform_theme) add_tag_to_para(L->owning_paragraph, Preform_theme, NULL);
			L->category = PREFORM_LCAT;
			L->text_operand = new_string(header);
		}
	}
}

@ The keyword "internal" can be followed by an indication of the number
of words the nonterminal will match: usually a decimal non-negative number,
but optionally a question mark |?| to indicate voracity.

@<Parse a Preform nonterminal header line@> =
	if (pattern_match(L->text, "(<%p+>) ::=%c*")) {
		form = A_GRAMMAR_NONTERMINAL;
		in_strcpy(pntname, found_text1);
		in_strcpy(header, found_text1);
		@<Parse the subsequent lines as Preform grammar@>;
	} else if (pattern_match(L->text, "((<%p+>) internal %?) {%c*")) {
		form = A_VORACIOUS_NONTERMINAL;
		in_strcpy(pntname, found_text2);
		in_strcpy(header, found_text1);
	} else if (pattern_match(L->text, "((<%p+>) internal) {%c*")) {
		form = A_FLEXIBLE_NONTERMINAL;
		in_strcpy(pntname, found_text2);
		in_strcpy(header, found_text1);
	} else if (pattern_match(L->text, "((<%p+>) internal (%d+)) {%c*")) {
		form = atoi(found_text3);
		in_strcpy(pntname, found_text2);
		in_strcpy(header, found_text1);
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
		if (string_is_white_space(AL->text)) break;
		AL->category = PREFORM_GRAMMAR_LCAT;
		
		if (pattern_match(AL->text, "(%c+) ==> (%c*)")) {
			AL->text_operand = new_string(found_text1);
			AL->text_operand2 = new_string(found_text2);
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
	if (pattern_match(AL->text_operand, "(%c*)%/%*%c*%*%/ *"))
		AL->text_operand = new_string(found_text1);

@ Note that nonterminal variables are, by default, integers. If their names
are divided internally with a colon, however, as |<<structure:name>>|, then
they have the type |structure *|.

@<Detect any nonterminal variables being set on the right side of the arrow@> =
	string to_scan; in_strcpy(to_scan, AL->text_operand2);
	while (pattern_match(to_scan, "%c*?<<(%P+?)>> =(%c*)")) {
		string var_given; in_strcpy(var_given, found_text1);
		string type_given; in_strcpy(type_given, "int");
		in_strcpy(to_scan, found_text2);
		if (pattern_match(var_given, "(%p+):%p+")) {
			in_sprintf(type_given, "%s *", found_text1);
		}
		nonterminal_variable *ntv;
		LOOP_OVER(ntv, nonterminal_variable)
			if (in_string_eq(ntv->ntv_name, var_given))
				break;
		if (ntv == NULL) @<This one's new, so create a new nonterminal variable@>;
	}

@ Nonterminal variables are actually just global C variables, and their C
identifiers need to avoid hyphens and colons. For example, |<<kind:ref>>|
has identifier |"kind_ref_NTMV"|.

@<This one's new, so create a new nonterminal variable@> =
	ntv = CREATE(nonterminal_variable);
	in_strcpy(ntv->ntv_name, var_given);
	in_strcpy(ntv->ntv_type, type_given);
	for (int i=0; var_given[i]; i++)
		if ((var_given[i] == '-') || (var_given[i] == ':'))
			in_set(var_given, i, '_');
	in_sprintf(ntv->ntv_identifier, "%s_NTMV", var_given);
	ntv->first_mention = AL;

@ When the program we are tangling is eventually running, each nonterminal
will be represented by a pointer to a unique data structure for it. Inweb
automatically compiles code to create these pointers; and here's how it
works out their names.

@<Compose a C identifier for the nonterminal@> =
	in_strcpy(pnt->as_C_identifier, pnt->nt_name + 1);
	for (int i=0; pnt->as_C_identifier[i]; i++) {
		if (pnt->as_C_identifier[i] == '-') in_set(pnt->as_C_identifier, i, '_');
		if (pnt->as_C_identifier[i] == '>') in_truncate(pnt->as_C_identifier, i);
	}
	in_strcat(pnt->as_C_identifier, "_NTM");

@

@<Work out the parsing characteristics of the nonterminal@> =
	pnt->voracious = FALSE; if (form == A_VORACIOUS_NONTERMINAL) pnt->voracious = TRUE;
	pnt->as_function = TRUE; if (form == A_GRAMMAR_NONTERMINAL) pnt->as_function = FALSE;

	pnt->takes_pointer_result = FALSE;
	if (pattern_match(pnt->nt_name, "<k-%c+")) pnt->takes_pointer_result = TRUE;
	if (pattern_match(pnt->nt_name, "<s-%c+")) pnt->takes_pointer_result = TRUE;

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
void c_for_inform_additional_predeclarations(FILE *F, web *W) {
	LOOP_WITHIN_TANGLE(W->first_target)
		if (L->preform_nonterminal_defined) {
			preform_nonterminal *pnt = L->preform_nonterminal_defined;
			language_insert_line_marker(F, W->main_language, L);
			fprintf(F, "nonterminal *%s = NULL;\n", pnt->as_C_identifier);
		}

	nonterminal_variable *ntv;
	LOOP_OVER(ntv, nonterminal_variable)
		fprintf(F, "%s %s = %s;\n",
			ntv->ntv_type, ntv->ntv_identifier,
			(in_string_eq(ntv->ntv_type, "int"))?"0":"NULL");

	fprintf(F, "void register_tangled_nonterminals(void);\n");
}

@ And here is the promised routine, which appears at the very end of the code.
It makes use of macros and data structures defined in the Inform 7 web.

@c
void c_for_inform_gnabehs(FILE *F, web *W) {
	fprintf(F, "void register_tangled_nonterminals(void) {\n");
	LOOP_WITHIN_TANGLE(W->first_target)
		if (L->preform_nonterminal_defined) {
			preform_nonterminal *pnt = L->preform_nonterminal_defined;
			language_insert_line_marker(F, W->main_language, L);
			if (pnt->as_function) {
				fprintf(F, "\tINTERNAL_NONTERMINAL(\"%s\", %s, %d, %d);\n",
					pnt->nt_name, pnt->as_C_identifier,
					pnt->min_word_count, pnt->max_word_count);
				fprintf(F, "\t%s->voracious = %d;\n",
					pnt->as_C_identifier, pnt->voracious);
			} else {
				fprintf(F, "\tREGISTER_NONTERMINAL(\"%s\", %s);\n",
					pnt->nt_name, pnt->as_C_identifier);
			}
		}
	fprintf(F, "}\n");
}

@p Tangling special line categories.
That's it for big structural additions to the tangled C code. Now we turn
to how to tangle the lines we've given special categories to.

We need to tangle |PREFORM_LCAT| lines (those holding nonterminal declarations)
in a special way...

@c
int c_for_inform_will_insert_in_tangle(source_line *L) {
	if (L->category == PREFORM_LCAT) return TRUE;
	return FALSE;
}

@ ...and this is how. As can be seen, each nonterminal turns into a C function.
In the case of an internal definition, like

	|<k-kind-for-template> internal {|

we tangle this opening line to

	|int k_kind_for_template_NTM(int w1, int w2, int *X, void **XP) {|

that is, to a function which returns |TRUE| if it makes a match on the text
excerpt from word number |w1| to |w2| in Inform's source text, |FALSE|
otherwise; if it matches and produces an integer and/or pointer result,
these are copied into |*X| and |*XP|. The remaining lines of the function
are tangled unaltered, i.e., following the same rules as for the body of
any other C function.

@c
void c_for_inform_insert_in_tangle(FILE *F, source_line *L) {
	preform_nonterminal *pnt = L->preform_nonterminal_defined;
	if (pnt->as_function) {
		fprintf(F, "int %sR(int w1, int w2, int *X, void **XP) {\n",
			pnt->as_C_identifier);
	} else {
		fprintf(F, "int %sC(int *X, void **XP, int *R, void **RP, int w1, int w2) {\n",
			pnt->as_C_identifier);
		@<Compile the body of the compositor function@>;
		fprintf(F, "}\n");
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
	fprintf(F, "\treturn TRUE;\n");

@ In the absence of any |==>| formulae, we simply set |*X| to the default
result supplied; this is the production number within the grammar (0 for the
first line, 1 for the second, and so on) by default, with an undefined pointer.

@<None of the grammar lines provided an arrow and formula@> =
	fprintf(F, "\t*X = R[0];\n");

@

@<At least one of the grammar lines provided an arrow and formula@> =
	fprintf(F, "\tswitch(R[0]) {\n");
	int c = 0;
	for (source_line *AL = L->next_line;
		((AL) && (AL->category == PREFORM_GRAMMAR_LCAT));
		AL = AL->next_line, c++) {
		char *formula = AL->text_operand2;
		if (formula[0]) {
			fprintf(F, "\t\tcase %d: ", c);
			@<Tangle the formula on the right-hand side of the arrow@>;
			fprintf(F, ";\n");
			fprintf(F, "#pragma clang diagnostic push\n");
			fprintf(F, "#pragma clang diagnostic ignored \"-Wunreachable-code\"\n");
			fprintf(F, "break;\n");
			fprintf(F, "#pragma clang diagnostic pop\n");
		}
	}
	fprintf(F, "\t\tdefault: *X = R[0]; break;\n");
	fprintf(F, "\t}\n");

@ We assume that the RHS of the arrow is an expression to be evaluated,
and that it produces an integer or a pointer according to what the
non-terminal expects as its main result. But we make one exception: if
the formula begins with a CWEB macro, then it can't be an expression,
and instead we read it as code in a void context. (This code will, we
assume, set |*X| and/or |*XP| in some ingenious way of its own.)

Within the body of the formula, we allow two pseudo-macros to work: |W1[n]|
expands to the word number beginning word range |n| in the match which
we're compositing, and |W2[n]| to its end. These actually expand like so:

	|action_clause_NTM->range_result_w1[n]|
	|action_clause_NTM->range_result_w2[n]|

which saves a good deal of typing. (A regular C preprocessor macro couldn't
easily do this, because it needs to include the identifier name of the
nonterminal being parsed.)

@<Tangle the formula on the right-hand side of the arrow@> =
	if (!pattern_match(formula, "@<%c*")) {
		if (pnt->takes_pointer_result) fprintf(F, "*XP = ");
		else fprintf(F, "*X = ");
	}
	string expanded; in_strcpy(expanded, "");
	for (int i=0; formula[i]; i++) {
		if ((formula[i] == 'W') && ((formula[i+1] == '1') || (formula[i+1] == '2')) &&
			(formula[i+2] == '[') && (isdigit(formula[i+3])) && (formula[i+4] == ']')) {
				in_sprintf(expanded+in_strlen(expanded),
					"%s->range_result_w%c[%c]", pnt->as_C_identifier,
					formula[i+1], formula[i+3]);
				i += 4;
		} else {
			in_sprintf(expanded+in_strlen(expanded), "%c", formula[i]);
		}
	}
	tangle_code(F, expanded, AL->owning_section, AL);

@p Tangling typical C code.
Going down from line level to the tangling of little excerpts of C code,
we also provide for some other special extensions to C.

@c
void c_for_inform_tangle_code(FILE *F, char *original) {
	int fcall_mode = FALSE;
	for (int i = 0; original[i]; i++) {
		@<Double-colons are namespace dividers in function names@>;
		if (original[i] == '<') {
			if (original[i+1] == '<') {
				@<Double-angles sometimes delimit Preform variable names@>;
			} else {
				@<Single-angles sometimes delimit Preform nonterminal names@>;
			}
		}
		if ((original[i] == ')') && (fcall_mode)) {
			fcall_mode = FALSE;
			fprintf(F, ", NULL, NULL");
		}
		fputc(original[i], F);
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
		fprintf(F, "__"); i++;
		continue;
	}

@ Angle brackets around a valid Preform variable name expand into its
C identifier; for example, |<<R>>| becomes |most_recent_result|.
We take no action if it's not a valid name, so |<<fish>>| becomes
just |<<fish>>|.

@<Double-angles sometimes delimit Preform variable names@> =
	if (pattern_match(original+i, "<<(%P+)>>(%c*)")) {
		string putative; in_strcpy(putative, found_text1);
		char *pv_identifier = nonterminal_variable_identifier(putative);
		if (pv_identifier) {
			fprintf(F, "%s", pv_identifier);
			i += in_strlen(putative) + 3;
			continue;
		}
	}

@ Similarly for nonterminals; |<k-kind>| might become |k_kind_NTM|.
Here, though, there's a complication:

	|if (<k-kind>(a1, a2)) { ...|

must expand to:

	|if (Text__Languages__parse_nt_against_word_range(k_kind_NTM, a1, a2, NULL, NULL)) { ...|

This is all syntactic sugar to make it easier to see parsing in action.
Anyway, it means we have to set |fcall_mode| to remember to add in the
two |NULL| arguments when we hit the |)| a little later. We're doing all
of this very laxly (it would go wrong if the arguments |a1| and |a2|
contained brackets, or comments or quoted text got in the way), but
as before: it only needs to work for Inform, and Inform doesn't cause
any trouble.

@<Single-angles sometimes delimit Preform nonterminal names@> =
	if (pattern_match(original+i, "(<%p+>)(%c*)")) {
		string putative; in_strcpy(putative, found_text1);
		preform_nonterminal *pnt = nonterminal_by_name(putative);
		if (pnt) {
			i += in_strlen(putative) - 1;
			if (original[i+1] == '(') fcall_mode = TRUE;
			if (fcall_mode) fprintf(F, "Preform__parse_nt_against_word_range(");
			fprintf(F, "%s", pnt->as_C_identifier);
			if (fcall_mode) {
				fprintf(F, ", "); i++; 
			}
			continue;
		}
	}

@ We needed two little routines to find nonterminals and their variables by
name. They're not very efficient, but experience shows that even on a web
the size of Inform 7, there's no significant gain from speeding them up
(with, say, a hash table).

@c
preform_nonterminal *nonterminal_by_name(char *name) {
	preform_nonterminal *pnt;
	LOOP_OVER(pnt, preform_nonterminal)
		if (in_string_eq(name, pnt->nt_name))
			return pnt;
	return NULL;
}

@ The special variables |<<R>>| and |<<RP>>| hold the results,
integer and pointer, for the most recent successful match. They're defined
in the Inform 7 web (see the code for parsing text against Preform grammars),
not by Inweb.

@c
char *nonterminal_variable_identifier(char *name) {
	if (in_string_eq(name, "r")) return "most_recent_result";
	if (in_string_eq(name, "rp")) return "most_recent_result_p";
	nonterminal_variable *ntv;
	LOOP_OVER(ntv, nonterminal_variable)
		if (in_string_eq(ntv->ntv_name, name))
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
void c_for_inform_additional_tangling(programming_language *pl, web *W, tangle_target *target) {
	string defn_to;
	in_sprintf(defn_to, "%sTangled%c%s", W->path_to_web, SEP_CHAR, "Syntax.preform");

	FILE *SYNTAX = fopen(defn_to, "w");
	if (SYNTAX == NULL) 
		fatal_filing_system_error("unable to open Preform file for output", defn_to);
	printf("Writing Preform syntax to: %s\n", defn_to);
	fprintf(SYNTAX, "[This is English.preform, generated by inweb: do not edit.]\n\n");
	fprintf(SYNTAX, "language English\n");

	@<Actually write out the Preform syntax@>;
	fclose(SYNTAX);
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
				fprintf(SYNTAX, "\n%s internal\n", pnt->nt_name);
			else
				fprintf(SYNTAX, "\n%s ::=\n", L->text_operand);
			for (source_line *AL = L->next_line;
				((AL) && (AL->category == PREFORM_GRAMMAR_LCAT));
				AL = AL->next_line) {
				fprintf(SYNTAX, "%s", AL->text_operand);
				if (pattern_match(AL->text_operand2, "%c+Issue (%c+) problem%c+"))
					fprintf(SYNTAX, "[issues %s]", found_text1);
				fprintf(SYNTAX, "\n");
			}
		}

@p Indexing Preform grammar.

@c
void weave_grammar_index(FILE *F) {
	fprintf(F, "\\raggedright\\tolerance=10000");
	preform_nonterminal *pnt;
	for (pnt = first_pnt_alphabetically; pnt;
		pnt = pnt->next_pnt_alphabetically) {
		fprintf(F, "\\line{\\nonterminal{%s}%s"
			"\\leaders\\hbox to 1em{\\hss.\\hss}\\hfill {\\xreffont %s}}\n",
			pnt->unangled_name,
			(pnt->as_function)?" (internal)":"",
			pnt->where_defined->owning_section->sigil);
		int said_something = FALSE;
		@<List where the nonterminal appears in other Preform declarations@>;
		@<List where the nonterminal is called from Inform code@>;
		if (said_something == FALSE)
			fprintf(F, "\\par\\hangindent=3em{\\it unused}\n\n");
	}
	fprintf(F, "\\penalty-1000\n");
	fprintf(F, "\\smallbreak\n");
	fprintf(F, "\\hrule\\smallbreak\n");
}

@

@<List where the nonterminal is called from Inform code@> =
	section *S;
	LOOP_OVER(S, section) S->scratch_flag = FALSE;
	hash_table_entry *hte = find_hash_entry(pnt->where_defined->owning_section, pnt->unangled_name, FALSE);
	for (hash_table_entry_usage *hteu = hte->first_usage; hteu; hteu = hteu->next_usage)
		if (hteu->form_of_usage & PREFORM_IN_CODE_USAGE)
			hteu->usage_recorded_at->under_section->scratch_flag = TRUE;
	int use_count = 0;
	LOOP_OVER(S, section)
		if (S->scratch_flag)
			use_count++;
	if (use_count > 0) {
		said_something = TRUE;
		fprintf(F, "\\par\\hangindent=3em{\\it called from} ");
		int c = 0;
		LOOP_OVER(S, section)
			if (S->scratch_flag) {
				if (c++ > 0) fprintf(F, ", ");
				fprintf(F, "{\\xreffont %s}", S->sigil);
			}
		fprintf(F, "\n\n");
	}

@

@<List where the nonterminal appears in other Preform declarations@> =
	section *S;
	LOOP_OVER(S, section) S->scratch_flag = FALSE;
	hash_table_entry *hte = find_hash_entry(pnt->where_defined->owning_section, pnt->unangled_name, FALSE);
	for (hash_table_entry_usage *hteu = hte->first_usage; hteu; hteu = hteu->next_usage)
		if (hteu->form_of_usage & PREFORM_IN_GRAMMAR_USAGE)
			hteu->usage_recorded_at->under_section->scratch_flag = TRUE;
	int use_count = 0;
	LOOP_OVER(S, section)
		if (S->scratch_flag)
			use_count++;
	if (use_count > 0) {
		said_something = TRUE;
		fprintf(F, "\\par\\hangindent=3em{\\it used by other nonterminals in} ");
		int c = 0;
		LOOP_OVER(S, section)
			if (S->scratch_flag) {
				if (c++ > 0) fprintf(F, ", ");
				fprintf(F, "{\\xreffont %s}", S->sigil);
			}
		fprintf(F, "\n\n");
	}

@p Weaving.


@c
void c_for_inform_new_tag_declared(theme_tag *tag) {
	if (in_string_eq(tag->tag_name, "Preform")) Preform_theme = tag;
}

int skipping_internal = FALSE, preform_production_count = 0;

int c_for_inform_skip_in_weaving(weave_target *wv, source_line *L) {
	if ((Preform_theme) && (wv->theme_match == Preform_theme)) {
		if (pattern_match(L->text, "}%c*")) { skipping_internal = FALSE; return TRUE; }
		if (skipping_internal) return TRUE;
		if (pattern_match(L->text, "<%c*?> internal%c*")) skipping_internal = TRUE;
	}
	return FALSE;
}

void c_for_inform_begin_weave(section *S, weave_target *wv) {
}

@

@c
int c_for_inform_syntax_colour(FILE *WEAVEOUT, weave_target *wv,
	web *W, chapter *C, section *S, source_line *L, char *matter, char *colouring) {
	return FALSE;
}

int c_for_inform_weave_code_line(FILE *WEAVEOUT, weave_target *wv,
	web *W, chapter *C, section *S, source_line *L, char *matter, char *concluding_comment) {
	if ((Preform_theme) && (wv->theme_match == Preform_theme)) {
		if (L->preform_nonterminal_defined) preform_production_count = 0;
		if (L->preform_nonterminal_defined) {
			fprintf(WEAVEOUT, "\\nonterminal{%s} |::=|",
				L->preform_nonterminal_defined->unangled_name);
			if (L->preform_nonterminal_defined->as_function) {
				fprintf(WEAVEOUT, "\\quad{\\it internal definition");	
				if (L->preform_nonterminal_defined->voracious)
					fprintf(WEAVEOUT, " (voracious)");
				else if (L->preform_nonterminal_defined->min_word_count ==
					L->preform_nonterminal_defined->min_word_count)
					fprintf(WEAVEOUT, " (%d word%s)",
						L->preform_nonterminal_defined->min_word_count,
						(L->preform_nonterminal_defined->min_word_count != 1)?"s":"");
				fprintf(WEAVEOUT, "}");
			}
			fprintf(WEAVEOUT, "\n");
			return TRUE;
		} else {
			if (L->category == PREFORM_GRAMMAR_LCAT) {
				string problem; in_strcpy(problem, "");
				if (pattern_match(matter, "Issue (%c*?) problem")) in_strcpy(problem, found_text1);
				if (pattern_match(matter, "FAIL_NONTERMINAL %+")) in_strcpy(problem, "fail and skip");
				else if (pattern_match(matter, "FAIL_NONTERMINAL")) in_strcpy(problem, "fail");
				preform_production_count++;
				in_sprintf(matter, "|%s|", L->text_operand);
				while (pattern_match(matter, "(%c+?)|(%c+)"))
					in_sprintf(matter, "%s___stroke___%s",
						found_text1, found_text2);
				while (pattern_match(matter, "(%c*?)___stroke___(%c*)"))
					in_sprintf(matter, "%s|\\||%s",
						found_text1, found_text2);
				while (pattern_match(matter, "(%c*)<(%c*?)>(%c*)"))
					in_sprintf(matter, "%s|\\nonterminal{%s}|%s",
						found_text1, found_text2, found_text3);
				string label; in_strcpy(label, "");
				int N = preform_production_count;
				int L = ((N-1)%26) + 1;
				if (N <= 26) in_sprintf(label, "%c", 'a'+L-1);
				else if (N <= 52) in_sprintf(label, "%c%c", 'a'+L-1, 'a'+L-1);
				else if (N <= 78) in_sprintf(label, "%c%c%c", 'a'+L-1, 'a'+L-1, 'a'+L-1);
				else {
					int n = (N-1)/26;
					in_sprintf(label, "%c${}^{%d}$", 'a'+L-1, n);
				}
				fprintf(WEAVEOUT, "\\qquad {\\hbox to 0.4in{\\it %s\\hfil}}%s", label, matter);
				if (problem[0])
					fprintf(WEAVEOUT, "\\hfill$\\longrightarrow$ {\\ttninepoint\\it %s}", problem);
				else if (concluding_comment[0]) {
					fprintf(WEAVEOUT, " \\hfill{\\ttninepoint\\it ");
					if (concluding_comment) format_identifier(WEAVEOUT, wv, concluding_comment);
					fprintf(WEAVEOUT, "}");
				}
				fprintf(WEAVEOUT, "\n");
				return TRUE;
			}
		}
	}
	return FALSE;
}

@p Analysis.

@c
void c_for_inform_analyse_code(programming_language *pl, web *W) {
	preform_nonterminal *pnt;
	LOOP_OVER(pnt, preform_nonterminal)
		find_hash_entry(pnt->where_defined->owning_section, pnt->unangled_name, TRUE);
	LOOP_WITHIN_TANGLE(W->first_target) {
		if (L->category == INTERFACE_BODY_LCAT) {
			if (pattern_match(L->text, "-- Defines {-(%c+?):(%c+)} *")) {
				L->interface_line_identified = TRUE;
				int form = 0;
				if (in_string_eq(found_text1, "call")) form = 1;
				if (in_string_eq(found_text1, "callv")) form = 2;
				if (in_string_eq(found_text1, "routine")) form = 3;
				if (in_string_eq(found_text1, "array")) form = 4;
				if (form == 0) {
					error_in_web("bad {-format:...}", L);
					continue;
				}
				string function_name; in_strcpy(function_name, "");
				switch (form) {
					case 1: case 2: in_strcpy(function_name, found_text2); break;
					case 3: in_sprintf(function_name, "%s_routine", found_text2); break;
					case 4: in_sprintf(function_name, "%s_array", found_text2); break;
				}
				function *fn = get_function_with_name(function_name);
				if (fn == NULL) {
					error_in_web("no such function exists", L);
					fprintf(stderr, "  >> %s\n", function_name);
					continue;
				}
				fn->called_from_other_sections = TRUE;
			}
		}
	}
}
