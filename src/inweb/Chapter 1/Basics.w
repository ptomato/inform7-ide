[Basics::] Basics.

@Purpose: Some fundamental definitions.

@ Inweb is itself written as an Inweb web. This circularity is traditional in
literate-programming circles, and if nothing else it's an act of self-belief.
It's also a proof of concept. Inweb is probably fairly correct, because it is
capable of tangling a program (Inweb) which can in turn tangle a really
complex and densely annotated program (Inform) which then passes an extensive
suite of tests.

@p Build identity.
First we define the build, using a notation which tangles out to the current
build number as specified in the contents section of this web.

@d INTOOL_NAME "inweb"
@d INWEB_BUILD "inweb [[Build Number]]"

@p Structures.

@p Setting up the memory manager.
We need to itemise the structures we'll want to allocate using the inlib
memory manager:

@d bibliographic_datum_MT 		NO_INLIB_MEMORY_TYPES
@d chapter_MT 					NO_INLIB_MEMORY_TYPES+1
@d section_MT 					NO_INLIB_MEMORY_TYPES+2
@d tangle_target_MT 			NO_INLIB_MEMORY_TYPES+3
@d source_line_array_MT 		NO_INLIB_MEMORY_TYPES+4
@d web_MT 						NO_INLIB_MEMORY_TYPES+5
@d paragraph_MT 				NO_INLIB_MEMORY_TYPES+6
@d cweb_macro_MT 				NO_INLIB_MEMORY_TYPES+7
@d programming_language_MT		NO_INLIB_MEMORY_TYPES+8
@d c_structure_MT 				NO_INLIB_MEMORY_TYPES+9
@d structure_dependence_MT 		NO_INLIB_MEMORY_TYPES+10
@d function_MT 					NO_INLIB_MEMORY_TYPES+11
@d preform_nonterminal_MT 		NO_INLIB_MEMORY_TYPES+12
@d nonterminal_variable_MT 		NO_INLIB_MEMORY_TYPES+13
@d weave_target_MT 				NO_INLIB_MEMORY_TYPES+14
@d macro_usage_MT 				NO_INLIB_MEMORY_TYPES+15
@d weave_format_MT 				NO_INLIB_MEMORY_TYPES+16
@d hash_table_entry_MT 			NO_INLIB_MEMORY_TYPES+17
@d hash_table_entry_usage_MT	NO_INLIB_MEMORY_TYPES+18
@d structure_element_MT 		NO_INLIB_MEMORY_TYPES+19
@d structure_permission_MT 		NO_INLIB_MEMORY_TYPES+20
@d theme_tag_MT 				NO_INLIB_MEMORY_TYPES+21
@d paragraph_tag_MT 			NO_INLIB_MEMORY_TYPES+22
@d tex_results_MT 				NO_INLIB_MEMORY_TYPES+23
@d imported_header_MT 			NO_INLIB_MEMORY_TYPES+24

@d NO_EXTRA_MEMORY_TYPES 25

@ And then expand:

@c
ALLOCATE_IN_ARRAYS(source_line, 1000)
ALLOCATE_INDIVIDUALLY(bibliographic_datum)
ALLOCATE_INDIVIDUALLY(c_structure)
ALLOCATE_INDIVIDUALLY(chapter)
ALLOCATE_INDIVIDUALLY(cweb_macro)
ALLOCATE_INDIVIDUALLY(function)
ALLOCATE_INDIVIDUALLY(hash_table_entry_usage)
ALLOCATE_INDIVIDUALLY(hash_table_entry)
ALLOCATE_INDIVIDUALLY(imported_header)
ALLOCATE_INDIVIDUALLY(macro_usage)
ALLOCATE_INDIVIDUALLY(nonterminal_variable)
ALLOCATE_INDIVIDUALLY(paragraph_tag)
ALLOCATE_INDIVIDUALLY(paragraph)
ALLOCATE_INDIVIDUALLY(preform_nonterminal)
ALLOCATE_INDIVIDUALLY(programming_language)
ALLOCATE_INDIVIDUALLY(tex_results)
ALLOCATE_INDIVIDUALLY(section)
ALLOCATE_INDIVIDUALLY(structure_dependence)
ALLOCATE_INDIVIDUALLY(structure_element)
ALLOCATE_INDIVIDUALLY(structure_permission)
ALLOCATE_INDIVIDUALLY(tangle_target)
ALLOCATE_INDIVIDUALLY(theme_tag)
ALLOCATE_INDIVIDUALLY(weave_format)
ALLOCATE_INDIVIDUALLY(weave_target)
ALLOCATE_INDIVIDUALLY(web)

@p Simple allocations.
Not all of our memory will be claimed in the form of structures: now and then
we need to use the equivalent of traditional |malloc| and |calloc| routines.

@d NO_EXTRA_MREASONS 0

@c
char *extra_memory_needs[1] = {
	"none"
};
