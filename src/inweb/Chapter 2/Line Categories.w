[Lines::] Line Categories.

@Purpose: To store individual lines from webs, and to categorise them according
to their meaning.

@p Line storage.
In the next section, we'll read in an entire web, building its hierarchical
structure of chapters, sections and eventually paragraphs. But before we do
that, we'll define the structure used to store a single line of the web.

Line, in this context, means text-file-line; it's an unusual feature of
Inweb, compared to other literate programming tools, that we will use line
breaks as significant.

@c
typedef struct source_line {
	char *text; /* the text as read in */
	char *text_operand; /* meaning depends on category */
	char *text_operand2; /* meaning depends on category */

	int category; /* what sort of line this is: an |*_LCAT| value */
	int command_code; /* used only for |COMMAND_LCAT| lines: a |*_CMD| value */
	int is_commentary; /* flag */
	struct function *function_defined; /* if any C-like function is defined on this line */
	struct preform_nonterminal *preform_nonterminal_defined; /* similarly */
	int suppress_tangling; /* if e.g., lines are tangled out of order */
	int interface_line_identified; /* only relevant during parsing of Interface lines */

	struct text_file_position source; /* which file this was read in from, if any */

	struct section *owning_section; /* for interleaved title lines, it's the one about to start */
	struct source_line *next_line; /* within the owning section's linked list */
	struct paragraph *owning_paragraph; /* for lines falling under paragraphs; |NULL| if not */
} source_line;

@

@c
source_line *Lines::new_source_line(char *line, text_file_position *tfp) {
	source_line *sl = CREATE(source_line);
	sl->text = Memory::new_string(line);
	sl->text_operand = "";
	sl->text_operand2 = "";

	sl->category = NO_LCAT; /* that is, unknown category as yet */
	sl->command_code = NO_CMD;
	sl->is_commentary = FALSE;
	sl->function_defined = NULL;
	sl->preform_nonterminal_defined = NULL;
	sl->suppress_tangling = FALSE;
	sl->interface_line_identified = FALSE;

	if (tfp) sl->source = *tfp; else sl->source = TextFiles::nowhere();

	sl->owning_section = NULL;
	sl->next_line = NULL;
	sl->owning_paragraph = NULL;
	return sl;
}

@p Categories.
The line categories are enumerated as follows. We briefly note what the text
operands (TO and TO2) are set to, if anything: most of the time they're blank.

@d NO_LCAT 0                 /* none set as yet */

@d CHAPTER_HEADING_LCAT 1    /* chapter heading line inserted automatically, not read from web */
@d SECTION_HEADING_LCAT 2    /* section heading line, at top of file */
@d PURPOSE_LCAT 3            /* first line of |@Purpose:| declaration; TO is rest of line */
@d INTERFACE_LCAT 4          /* line holding the |@Interface:| heading */
@d INTERFACE_BODY_LCAT 5     /* line within the interface, under this heading */
@d DEFINITIONS_LCAT 6        /* line holding the |@Definitions:| heading */
@d BAR_LCAT 7                /* a bar line |@---------------|... */
@d PB_PARAGRAPH_START_LCAT 8 /* |@p| or |@pp| paragraph start: TO is title, TO2 is rest of line */
@d PARAGRAPH_START_LCAT 9    /* simple |@| paragraph start: TO is blank, TO2 is rest of line */
@d COMMENT_BODY_LCAT 10      /* text following a paragraph header, which is all comment */
@d SOURCE_DISPLAY_LCAT 11    /* commentary line beginning |>>| for display: TO is display text */
@d MACRO_DEFINITION_LCAT 12  /* line on which a CWEB macro is defined with an |=| sign */
@d BEGIN_VERBATIM_LCAT 13    /* an |@c|, |@e| or |@x| line below which is code, early code or extract */
@d CODE_BODY_LCAT 14         /* the rest of the paragraph under an |@c| or |@e| or macro definition */
@d TEXT_EXTRACT_LCAT 15      /* the rest of the paragraph under an |@x| */
@d TOGGLE_WEAVING_LCAT 16    /* a line telling the weaver to toggle code weaving on or off */
@d BEGIN_DEFINITION_LCAT 17  /* an |@d| definition: TO is term, TO2 is this line's part of defn */
@d CONT_DEFINITION_LCAT 18   /* subsequent lines of an |@d| definition */
@d COMMAND_LCAT 19           /* a |[[Command]]| line, with the operand set to the |*_CMD| value */
@d C_LIBRARY_INCLUDE_LCAT 20 /* C-like languages only: a |#include| for an ANSI C header file */
@d TYPEDEF_LCAT 21           /* C-like languages only: a |typedef| which isn't a structure definition */
@d PREFORM_LCAT 22           /* C-for-Inform only: opening line of a Preform nonterminal */
@d PREFORM_GRAMMAR_LCAT 23   /* C-for-Inform only: line of Preform grammar */
@d PURPOSE_BODY_LCAT 24      /* continuation lines of |@Purpose:| declaration */

@ We want to print these out nicely for the sake of a |-scan| analysis run
of Inweb:

@c
char *Lines::category_name(int cat) {
	switch (cat) {
		case NO_LCAT: return "(uncategorised)";

		case BAR_LCAT: return "BAR";
		case BEGIN_DEFINITION_LCAT: return "BEGIN_DEFINITION";
		case BEGIN_VERBATIM_LCAT: return "BEGIN_CODE";
		case C_LIBRARY_INCLUDE_LCAT: return "C_LIBRARY_INCLUDE";
		case CHAPTER_HEADING_LCAT: return "CHAPTER_HEADING";
		case CODE_BODY_LCAT: return "CODE_BODY";
		case COMMAND_LCAT: return "COMMAND";
		case COMMENT_BODY_LCAT: return "COMMENT_BODY";
		case CONT_DEFINITION_LCAT: return "CONT_DEFINITION";
		case DEFINITIONS_LCAT: return "DEFINITIONS";
		case INTERFACE_BODY_LCAT: return "INTERFACE_BODY";
		case INTERFACE_LCAT: return "INTERFACE";
		case MACRO_DEFINITION_LCAT: return "MACRO_DEFINITION";
		case PARAGRAPH_START_LCAT: return "PARAGRAPH_START";
		case PB_PARAGRAPH_START_LCAT: return "PB_PARAGRAPH_START";
		case PREFORM_LCAT: return "PREFORM";
		case PREFORM_GRAMMAR_LCAT: return "PREFORM_GRAMMAR";
		case PURPOSE_LCAT: return "PURPOSE";
		case SECTION_HEADING_LCAT: return "SECTION_HEADING";
		case SOURCE_DISPLAY_LCAT: return "SOURCE_DISPLAY";
		case TEXT_EXTRACT_LCAT: return "TEXT_EXTRACT";
		case TOGGLE_WEAVING_LCAT: return "TOGGLE_WEAVING";
		case TYPEDEF_LCAT: return "TYPEDEF";
	}
	return "(?unknown)";
}

@p Command codes.
Command-category lines are further divided up into the following:

@d NO_CMD 0
@d PAGEBREAK_CMD 1
@d GRAMMAR_INDEX_CMD 2
@d FIGURE_CMD 3
@d TAG_CMD 4
