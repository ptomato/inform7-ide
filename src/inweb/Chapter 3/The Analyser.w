[Analyser::] The Analyser.

@Purpose: Here we analyse the code in the web, enabling us to see how functions
and data structures are used within the program.

@p The section catalogue.
This provides quite a useful overview of the sections. As we'll see frequently
in Chapter 4, we call out to a general routine in Chapter 5 to provide
annotations which are programming-language specific; the aim is to abstract
so that Chapter 4 contains no assumptions about the language.

@c
void Analyser::catalogue_the_sections(web *W, char *sigil, int functions_too) {
	for (chapter *C = W->first_chapter; C; C = C->next_chapter)
		if ((CStrings::eq(sigil, "0")) || (CStrings::eq(sigil, C->ch_sigil))) {
			printf("      %-9s  %-50s  \n", "--------", "--------");
			for (section *S = C->first_section; S; S = S->next_section) {
				string main_title;
				CSTRING_WRITE(main_title, "Chapter %s/%s", C->ch_sigil, S->sect_title);
				printf("%4d  %-9s  %-50s  ", S->sect_extent, S->sigil, main_title);
				Languages::analysis(S->sect_language, S, functions_too);
				printf("\n");
			}
		}
}

@p Analysing code.
We can't pretend to a full-scale static analysis of the code -- for one thing,
that would mean knowing more about the syntax of the web's language than we
actually do. So the following provides only a toolkit which other code can
use when looking for certain syntactic patterns: something which looks like
a function call, or a C structure field reference, for example. These are
all essentially based on spotting identifiers in the code, but with
punctuation around them.

Usage codes are used to define a set of allowed contexts in which to spot
these identifiers.

@d ELEMENT_ACCESS_USAGE     0x00000001 /* C-like languages: access via |->| or |.| operators to structure element */
@d FCALL_USAGE              0x00000002 /* C-like languages: function call made using brackets, |name(args)| */
@d PREFORM_IN_CODE_USAGE    0x00000004 /* C-for-Inform only: use of a Preform nonterminal as a C "constant" */
@d PREFORM_IN_GRAMMAR_USAGE 0x00000008 /* C-for-Inform only: ditto, but within Preform production rather than C code */
@d MISC_USAGE               0x00000010 /* any other appearance as an identifier */
@d ANY_USAGE                0x7fffffff /* any of the above */

@ The main analysis routine goes through a web as follows. Note that we only
perform the search here, we don't comment on the results; any action to be
taken must be handled by |Languages::post_analysis| when we're done.

@c
void Analyser::analyse_code(web *W) {
	if (W->analysed) return;
	
	@<Ask language-specific code to identify search targets, and parse the Interfaces@>;

	LOOP_WITHIN_TANGLE(W->first_target)
		switch (L->category) {
			case BEGIN_DEFINITION_LCAT:
				@<Perform analysis on the body of the definition@>;
				break;
			case CODE_BODY_LCAT:
				@<Perform analysis on a typical line of code@>;
				break;
			case PREFORM_GRAMMAR_LCAT:
				@<Perform analysis on productions in a Preform grammar@>;
				break;
		}

	Languages::post_analysis(W->main_language, W);
	W->analysed = TRUE;
}

@ First, we call any language-specific code, whose task is to identify what we
should be looking for: for example, the C-like languages code tells us (see
below) to look for names of particular functions it knows about. This code
is also expected to parse any Interface lines in a section which it recognises,
marking those by setting their |interface_line_identified| flags. Any that
are left must be erroneous.

@<Ask language-specific code to identify search targets, and parse the Interfaces@> =
	Languages::analyse_code(W->main_language, W);

	LOOP_WITHIN_TANGLE(W->first_target)
		if ((L->category == INTERFACE_BODY_LCAT) &&
			(L->interface_line_identified == FALSE) &&
			(ISORegexp::string_is_white_space(L->text) == FALSE))
			Main::error_in_web("unrecognised interface line", L);

@

@<Perform analysis on a typical line of code@> =
	Analyser::analyse_as_code(W, L, L->text, ANY_USAGE, 0);

@

@<Perform analysis on the body of the definition@> =
	Analyser::analyse_as_code(W, L, L->text_operand2, ANY_USAGE, 0);
	while ((L->next_line) && (L->next_line->category == CONT_DEFINITION_LCAT)) {
		L = L->next_line;
		Analyser::analyse_as_code(W, L, L->text, ANY_USAGE, 0);
	}

@ Recall -- or rather, see Chapter 5, where this all comes up -- that lines
in a Preform grammar generally take the form of some BNF grammar, where we
want only to identify any nonterminals mentioned, then a |==>| divider,
and then some C code to deal with a match. The code is subjected to analysis
just as any other code would be.

@<Perform analysis on productions in a Preform grammar@> =
	Analyser::analyse_as_code(W, L, L->text_operand2, ANY_USAGE, 0);
	Analyser::analyse_as_code(W, L, L->text_operand, PREFORM_IN_CODE_USAGE, PREFORM_IN_GRAMMAR_USAGE);

@p Identifier searching.
Here's what we actually do, then. We take the code fragment |text|, drawn
from part or all of source line |L| from web |W|, and look for any identifier
names used in one of the contexts in the bitmap |mask|. Any that we find are
passed to |Analyser::analyse_find|, along with the context they were found in (or, if
|transf| is nonzero, with |transf| as their context).

What we do is to look for instances of an identifier, defined as a maximal
string of |%i| characters or hyphens not followed by |>| characters. (Thus
|fish-or-chips| counts, but |fish-| is not an identifier when it occurs in
|fish->bone|.)

@c
void Analyser::analyse_as_code(web *W, source_line *L, char *text, int mask, int transf) {
	int start_at = -1, element_follows = FALSE;
	for (int i = 0; text[i]; i++) {
		if ((ISORegexp::identifier_char(text[i])) || ((text[i] == '-') && (text[i+1] != '>'))) {
			if (start_at == -1) start_at = i;
		} else {
			if (start_at != -1) {
				int u = MISC_USAGE;
				if (element_follows) u = ELEMENT_ACCESS_USAGE;
				else if (text[i] == '(') u = FCALL_USAGE;
				else if ((text[i] == '>') && (start_at > 0) && (text[start_at-1] == '<'))
					u = PREFORM_IN_CODE_USAGE;
				if (u & mask) {
					if (transf) u = transf;
					string identifier_found; CStrings::copy(identifier_found, "");
					int j;
					for (j = 0; start_at + j < i; j++)
						CStrings::set_char(identifier_found, j, text[start_at + j]);
					CStrings::truncate(identifier_found, j);
					Analyser::analyse_find(W, L, identifier_found, u);
				}
				start_at = -1; element_follows = FALSE;
			}
			if (text[i] == '.') element_follows = TRUE;
			else if ((text[i] == '-') && (text[i+1] == '>')) {
				element_follows = TRUE; i++;
			} else element_follows = FALSE;
		}
	}
}

@p The identifier hash table.
We clearly need rapid access to a large symbols table, and we store this as
a hash. Identifiers are hash-coded with the following simple code, which is
simplified from one used by Inform; it's the algorithm called "X 30011"
in Aho, Sethi and Ullman, {\it Compilers: Principles, Techniques and Tools} 
(1986), adapted slightly to separate out literal numbers.

@d HASH_TAB_SIZE 1000 /* the possible hash codes are 0 up to this minus 1 */
@d NUMBER_HASH 0 /* literal decimal integers, and no other words, have this hash code */

@c
int Analyser::hash_code_from_word(char *text) {
    unsigned int hash_code = 0;
    char *p = text;
    switch(*p) {
    	case '-': if (p[1] == 0) break; /* an isolated minus sign is an ordinary word */
    		/* and otherwise fall into... */
    	case '0': case '1': case '2': case '3': case '4':
    	case '5': case '6': case '7': case '8': case '9':
    		/* the first character may prove to be the start of a number: is this true? */
			for (p++; *p; p++) if (isdigit(*p) == FALSE) break;
			return NUMBER_HASH;
    }
    for (p=text; *p; p++) hash_code = (unsigned int) ((int) (hash_code*30011) + (*p));
    return (int) (1+(hash_code % (HASH_TAB_SIZE-1))); /* result of X 30011, plus 1 */
}

@ The actual table is stored here:

@c
typedef struct hash_table {
	struct hash_table_entry *analysis_hash[HASH_TAB_SIZE]; /* linked list of identifiers sharing this hash code */
	int analysis_hash_initialised; /* when we start up, array's contents are undefined */
} hash_table;

@ Where we define:

@c
typedef struct hash_table_entry {
	char *hash_key;
	struct hash_table_entry *next_in_hash; /* i.e., in list of identifiers sharing this hash code */
	int reserved_word; /* in the language currently being woven, that is */
	struct hash_table_entry_usage *first_usage; /* a linked list of all known usages */
	struct hash_table_entry_usage *last_usage;
	MEMORY_MANAGEMENT
} hash_table_entry;

@ A single routine is used both to interrogate the hash and to lodge values
in it, as usual with symbols tables. For example, the code to handle C-like
languages prepares for code analysis by calling this routine on the name
of each C function.

@c
hash_table_entry *Analyser::find_hash_entry(section *S, char *text, int create) {
	hash_table *HT = &(S->sect_target->symbols);
	int h = Analyser::hash_code_from_word(text);
	if (h == NUMBER_HASH) return NULL;
	if (HT->analysis_hash_initialised == FALSE) {
		for (int i=0; i<HASH_TAB_SIZE; i++) HT->analysis_hash[i] = NULL;
		HT->analysis_hash_initialised = TRUE;
	}
	hash_table_entry *hte = NULL;	
	for (hte = HT->analysis_hash[h]; hte; hte = hte->next_in_hash)
		if (CStrings::eq(hte->hash_key, text))
			break;
	if ((hte == NULL) && (create)) {
		hte = CREATE(hash_table_entry);
		hte->hash_key = Memory::new_string(text);
		hte->next_in_hash = HT->analysis_hash[h];
		hte->first_usage = NULL;
		hte->last_usage = NULL;
		HT->analysis_hash[h] = hte;
	}
	return hte;		
}

@ Marking and testing these bits:

@c
void Analyser::mark_reserved_word(section *S, char *p, int e) {
	hash_table_entry *hte = Analyser::find_hash_entry(S, p, TRUE);
	hte->reserved_word |= (1 << e);
}

int Analyser::is_reserved_word(section *S, char *p, int e) {
	hash_table_entry *hte = Analyser::find_hash_entry(S, p, FALSE);
	if ((hte) && (hte->reserved_word & (1 << e))) return TRUE;
	return FALSE;
}

@ Now we turn back to the actual analysis. When we spot an identifier that
we know, we record its usage with an instance of the following. Note that
each identifier can have at most one of these records per paragraph of code,
but that it can be used in multiple ways within that paragraph: for example,
a function might be both called and used as a constant value within the
same paragraph of code.

@c
typedef struct hash_table_entry_usage {
	struct paragraph *usage_recorded_at;
	int form_of_usage; /* bitmap of the |*_USAGE| constants defined above */
	struct hash_table_entry_usage *next_usage; /* i.e., in the list of usages of this identifier */
	MEMORY_MANAGEMENT
} hash_table_entry_usage;

@ And here's how we create these usages:

@c
void Analyser::analyse_find(web *W, source_line *L, char *identifier, int u) {
	hash_table_entry *hte = Analyser::find_hash_entry(L->owning_section, identifier, FALSE);
	if (hte == NULL) return;
	hash_table_entry_usage *hteu = NULL;
	for (hteu = hte->first_usage; hteu; hteu = hteu->next_usage)
		if (L->owning_paragraph == hteu->usage_recorded_at)
			break;
	if (hteu == NULL) {
		hteu = CREATE(hash_table_entry_usage);
		hteu->form_of_usage = 0;
		hteu->next_usage = NULL;
		hteu->usage_recorded_at = L->owning_paragraph;
		if (hte->first_usage == NULL) hte->first_usage = hteu; 
		else hte->last_usage->next_usage = hteu; 
		hte->last_usage = hteu;
	}
	hteu->form_of_usage |= u;
}
