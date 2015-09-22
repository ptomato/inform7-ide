[Numbering::] Paragraph Numbering.

@Purpose: To work out paragraph numbers within each section.

@p Numbering.
Traditional LP tools have numbered paragraphs in the obvious way, starting
from 1, but we're going to try something hierarchical instead: thus paragraph
1.1 will be used with paragraph 1, and so on. It's a little ambiguous how
to do this for the best, as we'll see.

We can certainly only do it if we know exactly where macros are used. This
is something we scan for on a weave, but not on a tangle; tangled code
doesn't need to know its own paragraph numbers.

@c
void Numbering::number_web(web *W) {
	for (chapter *C = W->first_chapter; C; C = C->next_chapter) {
		for (section *S = C->first_section; S; S = S->next_section) {
			@<Scan this section to see where CWEB macros are used@>;
			@<Work out paragraph numbers within this section@>;
		}
	}
}

@ 

@<Scan this section to see where CWEB macros are used@> =
	for (source_line *L = S->first_line; L; L = L->next_line) {
		char *p = L->text;
		int mlen, mpos;
		while ((mpos = ISORegexp::find_expansion(p, '@', '<', '@', '>', &mlen)) != -1) {
			string found_macro;
			CStrings::copy(found_macro, p+mpos+2); CStrings::truncate(found_macro, mlen-4);
			p = p + mpos + mlen;
			cweb_macro *cwm = Parser::get_cweb_macro_by_name(found_macro, S);
			if (cwm) @<Add a record that the macro is used in this paragraph@>;
		}
	}

@ Each macro comes with a linked list of notes about which paragraphs use
it; necessarily paragraphs within the same section.

This paragraph you're looking at now shows the difficulty involved in
paragraph numbering. It's not a macro, so it's not obviously used by any
other paragraph. Should it be bumped up to paragraph 2? But if we do that,
we end up with numbers out of order, since the one after it would have to
be 1.1.1. Instead this one will be 1.1.1, to place it into the natural
lexicographic sequence.

@c
typedef struct macro_usage {
	struct paragraph *used_in_paragraph;
	int multiplicity; /* for example, 2 if it's used twice in this paragraph */
	struct macro_usage *next_macro_usage; /* within the list of uses of the macro */
	MEMORY_MANAGEMENT
} macro_usage;

@

@<Add a record that the macro is used in this paragraph@> =
	macro_usage *mu, *last = NULL;
	for (mu = cwm->macro_usages; mu; mu = mu->next_macro_usage) {
		last = mu;
		if (mu->used_in_paragraph == L->owning_paragraph)
			break;
	}
	if (mu == NULL) {
		mu = CREATE(macro_usage);
		mu->used_in_paragraph = L->owning_paragraph;
		mu->next_macro_usage = NULL;
		mu->multiplicity = 0;
		if (last) last->next_macro_usage = mu;
		else cwm->macro_usages = mu;
	}
	mu->multiplicity++;

@ Basically we'll form the paragraphs into a tree, or in fact a forest. If a
paragraph defines a macro then we want it to be a child node of the
paragraph where the macro is first used; it's then a matter of filling in
other nodes a bit speculatively.

@<Work out paragraph numbers within this section@> =
	@<The parent of a macro definition is the place where it's first used@>;
	@<Otherwise share the parent of a following paragraph, provided it precedes us@>;
	@<Number the still parent-less paragraphs consecutively from 1@>;
	@<Recursively derive the numbers of parented paragraphs from those of their parents@>;

@

@<The parent of a macro definition is the place where it's first used@> =
	for (paragraph *P = S->first_paragraph; P; P = P->next_paragraph_in_section) {
		macro_usage *mu = (P->defines_macro)?(P->defines_macro->macro_usages):NULL;
		if (mu) P->parent_paragraph = mu->used_in_paragraph;
	}

@

@<Otherwise share the parent of a following paragraph, provided it precedes us@> =
	for (paragraph *P = S->first_paragraph; P; P = P->next_paragraph_in_section)
		if (P->parent_paragraph == NULL)
			for (paragraph *P2 = P; P2; P2 = P2->next_paragraph_in_section)
				if (P2->parent_paragraph) {
					if (P2->parent_paragraph->allocation_id < P->allocation_id)
						P->parent_paragraph = P2->parent_paragraph;
					break;
				}

@ Now we have our tree, and we number paragraphs accordingly: root notes are
numbered 1, 2, 3, ..., and then children are numbered with suffixes .1, .2, .3,
..., under their parents.

@<Number the still parent-less paragraphs consecutively from 1@> =
	int top_level = 1;
	for (paragraph *P = S->first_paragraph; P; P = P->next_paragraph_in_section)
		if (P->parent_paragraph == NULL) {
			CSTRING_WRITE(P->paragraph_number, "%d", top_level++);
			P->next_child_number = 1;
		} else
			CStrings::copy(P->paragraph_number, "");

@

@<Recursively derive the numbers of parented paragraphs from those of their parents@> =
	for (paragraph *P = S->first_paragraph; P; P = P->next_paragraph_in_section)
		Numbering::settle_paragraph_number(P);

@ The following paragraph shows the deficiencies of the algorithm: it's going
to end up numbered 2, because it isn't used anywhere and doesn't seem to be
in the middle of a wider description. If we were analysing the source in
more depth, we'd detect that it consists only of a function definition,
then look at where the function is used. But we don't.

@c
void Numbering::settle_paragraph_number(paragraph *P) {
	if (P->paragraph_number[0]) return;
	CStrings::copy(P->paragraph_number, "X"); /* to prevent malformed sections hanging this */
	if (P->parent_paragraph) Numbering::settle_paragraph_number(P->parent_paragraph);
	CSTRING_WRITE(P->paragraph_number, "%s.%d", P->parent_paragraph->paragraph_number,
			P->parent_paragraph->next_child_number++);
	P->next_child_number = 1;
}
