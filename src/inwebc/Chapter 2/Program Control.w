2/pc: Program Control.

@Purpose: The top level, which decides what is to be done and then carries
this plan out.

@ Inweb has a single fundamental mode of operation: on any given run, it
is either tangling, weaving or analysing. These processes use the same input
and parsing code, but then do very different things to produce their output,
so the fork in the road is not met until halfway through Inweb's execution.

@d NO_MODE 0 /* a special mode for doing nothing except printing command-line syntax */
@d ANALYSE_MODE 1
@d TANGLE_MODE 2
@d WEAVE_MODE 3
@d CREATE_MODE 4 /* a special mode for creating a new web, not acting on an existing one */

@ This operation will be applied to a single web, and will apply to the whole
of that web unless we specify otherwise. Subsets of the web are represented by
short pieces of text called "sigils". This can be a section sigil like
|2/pine|, a chapter number like |12|, an appendix letter |A| or the
preliminaries block |P|, the special chapter |S| for the "Sections" chapter
of an unchaptered web, or the special value |0| to mean the entire web (which
is the default).

When weaving in "swarm mode", however, the user chooses a multiplicity of
operations rather than just one. Now it's no longer a matter of weaving a
particular section or chapter: we can weave all of the sections or chapters,
one after another.

@d SWARM_OFF 0
@d SWARM_INDEX 1 /* make index(es) as if swarming, but don't actually swarm */
@d SWARM_CHAPTERS 2 /* swarm the chapters */
@d SWARM_SECTIONS 3 /* that, and also all of the individual sections */

@ In order to run, Inweb needs to know where it is installed -- this
enables it to find its configuration file, the macros file, and so on.
Unless told otherwise on the command line, we'll assume Inweb is present
in the current working directory. The "materials" will then be in a further
subfolder called |Materials|.

@c
string path_to_inweb_materials; /* the materials pathname, including final separator */

string tex_configuration; /* read from configuration file, not command line */
string pdftex_configuration;
string open_configuration;

@ We count the errors in order to be able to exit with a suitable exit code.

@c
int no_inweb_errors = 0;

@p Main routine.
Note that the configuration file is read only after command-line arguments
have been parsed -- this ensures that any |-at| setting can change where
we look for the configuration file.

@c
int main(int argc, char **argv) {
	printf("%s\n", INWEB_BUILD);

	@<Initialise inweb@>;

	inweb_instructions args = read_instructions_from_command_line(argc, argv);
	in_sprintf(path_to_inweb_materials, "%sinwebc%cMaterials%c",
		args.location_setting, SEP_CHAR, SEP_CHAR);
	read_configuration_file();
	follow_instructions(&args);	

	@<Shut inweb down@>;
}

@

@<Initialise inweb@> =
	start_memory();
	create_programming_languages();
	create_weave_formats();

@

@<Shut inweb down@> =
	free_memory();
	return (no_inweb_errors == 0)?0:1;

@p Following instructions.
This is the whole program in a nutshell, and it's a pretty old-school
program: some input, some thinking, a choice of three forms of output.

@c
void follow_instructions(inweb_instructions *ins) {
	if (ins->inweb_mode == NO_MODE) @<Show command-line usage and exit@>;
	if (ins->inweb_mode == CREATE_MODE) @<Create a new web@>
	else @<Analyse, tangle or weave an existing web@>;
}

@ There's no sense writing out the whole manual; this usage note is intended
for people who run across Inweb and have no idea what it is.

@<Show command-line usage and exit@> =
	printf("[[Purpose]]\n");
	printf("Usage: inweb webname -action [-options] [target]\n");
	printf("  where 'webname' is a folder containing a web (an inweb project),\n");
	printf("  The most useful -action commands are:\n");
	printf("    -create: make a new web, creating its folder and contents\n");
	printf("    -tangle: make the program described in the web\n");
	printf("    -weave: make a human-readable booklet of the web\n");
	printf("  For options and less commonly used actions, see the inweb manual.\n");
	exit(0);

@ The |-create| option is a small utility for creating a new web -- a slightly
fiddly business, so just about worth automating. This is all it amounts to:

@<Create a new web@> =
	issue_os_command_1("mkdir -pv '%s'", ins->chosen_web);
	issue_os_command_1("mkdir -pv '%sFigures'", ins->chosen_web);
	issue_os_command_1("mkdir -pv '%sMaterials'", ins->chosen_web);
	issue_os_command_1("mkdir -pv '%sSections'", ins->chosen_web);
	issue_os_command_1("mkdir -pv '%sTangled'", ins->chosen_web);
	issue_os_command_1("mkdir -pv '%sWoven'", ins->chosen_web);
	string default_conts;
	in_sprintf(default_conts, "%sContents.w", path_to_inweb_materials);
	issue_os_command_2("cp -nv '%s' '%s'", default_conts, ins->chosen_web);
	string default_main;
	in_sprintf(default_main, "%sMain.w", path_to_inweb_materials);
	string new_sections;
	in_sprintf(new_sections, "%s%cSections", ins->chosen_web, SEP_CHAR);
	issue_os_command_2("cp -nv '%s' '%s'", default_main, new_sections);

@ And otherwise we read and fully parse a web, and then do something with it:

@<Analyse, tangle or weave an existing web@> =
	web *W = load_web(ins->chosen_web, ins->verbose_switch);
	print_web_statistics(W);
	if (ins->inweb_mode == ANALYSE_MODE) @<Analyse the web@>;
	if (ins->inweb_mode == TANGLE_MODE) @<Tangle the web@>;
	if (ins->inweb_mode == WEAVE_MODE) @<Weave the web@>;

@ "Analysis" invokes any combination of the following diagnostic tools:

@<Analyse the web@> =
	if (ins->swarm_mode != SWARM_OFF)
		fatal_error("only specific parts of the web can be analysed");
	if (ins->catalogue_switch) catalogue_the_sections(W, ins->chosen_subweb, FALSE);
	if (ins->functions_switch) catalogue_the_sections(W, ins->chosen_subweb, TRUE);
	if (ins->scan_switch) scan_line_categories(W, ins->chosen_subweb);

@ We can tangle to any one of what might be several targets, numbered upwards
from 0. Target 0 always exists, and is the main program forming the web. For
many webs, this will in fact be the only target, but Inweb also allows
marked sections of a web to be independent targets -- the idea here is to
allow an Appendix in the web to contain a configuration file, or auxiliary
program, needed for the main program to work; this might be written in a
quite different language from the rest of the web, and tangles to a different
output, but needs to be part of the web since it's essential to an understanding
of the whole system.

In this section we determine |tn|, the target number wanted, and |tangle_to|,
the filename of the tangled code to write. This may have been set at the command
line (in which case |ins->tangle_setting| contains the filename), but otherwise
we impose a sensible choice based on the target.

@<Tangle the web@> =
	string tangle_to; in_strcpy(tangle_to, "");
	tangle_target *tn = NULL;
	if (in_string_eq(ins->chosen_subweb, "0")) {
		@<Work out main tangle destination@>;
	} else if (get_section_for_sigil(W, ins->chosen_subweb)) {
		@<Work out an independent tangle destination, from one section of the web@>;
	} else {
		@<Work out an independent tangle destination, from one chapter of the web@>;
	}
	if (tangle_to[0] == 0) { fatal_error("no tangle destination known"); }
	string temp;
	in_strcpy(temp, tangle_to);
	if (ins->tangle_setting[0]) in_strcpy(tangle_to, ins->tangle_setting);
	else in_sprintf(tangle_to, "%sTangled%c%s", W->path_to_web, SEP_CHAR, temp);
	if (tn == NULL) tn = W->first_target;
	tangle_source(W, tn, tangle_to);

@ Here the target number is 0, and the tangle is of the main part of the web,
which for many small webs will be the entire thing.

@<Work out main tangle destination@> =
	tn = NULL;
	if (bibliographic_data_exists(W, "Short Title"))
		in_strcpy(tangle_to, get_bibliographic_data(W, "Short Title"));
	else
		in_strcpy(tangle_to, get_bibliographic_data(W, "Title"));
	in_strcat(tangle_to, W->main_language->file_extension);

@ If someone tangles, say, |2/eg| then the default filename is "Example Section".

@<Work out an independent tangle destination, from one section of the web@> =
	section *S = get_section_for_sigil(W, ins->chosen_subweb);
	tn = S->sect_target;
	if (tn == NULL) fatal_error("section cannot be independently tangled");
	in_strcpy(tangle_to, S->leafname);

@ If someone tangles, say, |B| meaning "Appendix B: Important Warnings" then
the default filename is "Important Warnings".

@<Work out an independent tangle destination, from one chapter of the web@> =
	chapter *C;
	LOOP_OVER(C, chapter)
		if (in_string_eq(ins->chosen_subweb, C->ch_sigil))
			if (C->ch_target) {
				if (pattern_match(C->ch_title, "%c+?: (%c+)"))
					in_strcpy(tangle_to, found_text1);
				else
					in_strcpy(tangle_to, C->ch_title);
				tn = C->ch_target;
				break;
			}
	if (tn == NULL)
		fatal_error("only the entire web, or specific sections, can be tangled");

@ Weaving is not actually easier, it's just more thoroughly delegated:

@<Weave the web@> =
	number_web(W);
	theme_tag *tag = tag_by_name(ins->theme_setting);
	if ((ins->theme_setting[0]) && (tag == NULL))
		fatal_error_with_parameter("no such theme as '%s'", ins->theme_setting);
	if (ins->swarm_mode == SWARM_OFF) {
		int shall_we_open = ins->open_pdf_switch;
		if (shall_we_open == UNKNOWN) { /* i.e., if it wasn't set at the command line */
			if (in_string_ne(open_configuration, "")) shall_we_open = TRUE;
			else shall_we_open = FALSE;
		}
		weave_subset_of_web(W, ins->chosen_subweb, shall_we_open, tag, ins->weave_format);
	} else {
		weave_swarm(W, ins->chosen_subweb, ins->swarm_mode, tag, ins->weave_format);
	}
