2/cli: Command Line Arguments.

@Purpose: To parse the command line arguments with which inweb was called,
and to handle any errors it needs to issue.

@p Instructions.
The following structure encodes a set of instructions from the user (probably
from the command line) about what Inweb should do on this run:

@c
typedef struct inweb_instructions {
	int inweb_mode; /* our main mode of operation: one of the |*_MODE| constants */
	string chosen_web; /* project folder relative to cwd */
	string chosen_subweb; /* which subset of this web we apply to (often, all of it) */

	int swarm_mode; /* relevant to weaving only: one of the |*_SWARM| constants */
	string theme_setting; /* |-theme X|: theme for a topic-restricted weave */
	string weave_format; /* |-format X|: for example, |-format TeX| */

	int catalogue_switch; /* |-catalogue|: print catalogue of sections */
	int functions_switch; /* |-functions|: print catalogue of functions within sections */
	int open_pdf_switch; /* |-open-pdf|: open any woven PDF in the OS once it is made */
	int scan_switch; /* |-scan|: simply show the syntactic scan of the source */
	string tangle_setting; /* |-tangle X|: the pathname X, if supplied */
	int verbose_switch; /* |-verbose|: print names of files read to stdout */
	
	string location_setting; /* |-at X|: where this program is */
} inweb_instructions;

@p Reading the command line.

@c
inweb_instructions read_instructions_from_command_line(int argc, char **argv) {
	inweb_instructions args;
	args.inweb_mode = NO_MODE;
	args.swarm_mode = SWARM_OFF;
	args.catalogue_switch = FALSE;
	args.functions_switch = FALSE;
	args.open_pdf_switch = UNKNOWN;
	args.scan_switch = FALSE;
	args.verbose_switch = FALSE;
	in_strcpy(args.chosen_web, "");
	in_strcpy(args.chosen_subweb, "0"); /* by default, the entire web is the target */
	in_strcpy(args.tangle_setting, "");
	in_strcpy(args.theme_setting, "");
	in_strcpy(args.weave_format, "PDF");
	in_strcpy(args.location_setting, "");

	int i;
	int targets = 0;
	for (i=1; i<argc; i++) {
		char *opt = argv[i];
		int non_switch_follows = FALSE;
		if (i+1 < argc) {
			char *next_opt = argv[i+1];
			if (next_opt[0] != '-') non_switch_follows = TRUE;
		}
		if (opt[0] == '-') @<Parse this as a switch@>
		else {
			if (args.chosen_web[0] == 0) in_sprintf(args.chosen_web, "%s/", opt);
			else @<Parse this as a target sigil@>;
		}
	}
	if (args.chosen_web[0] == 0) args.inweb_mode = NO_MODE;
	return args;
}

@

@<Parse this as a switch@> =
	if (opt[1] == '-') opt++; /* allow a doubled-dash as equivalent to a single */
	if (in_string_eq(opt, "-verbose")) {
		args.verbose_switch = TRUE; continue;
	}
	if (in_string_eq(opt, "-at")) {
		if (non_switch_follows) {
			in_strcpy(args.location_setting, argv[i+1]); i++; continue;
		}
		fatal_error("-at must be followed by the pathname where inweb lives");
	}
	@<Parse analysis options@>;
	@<Parse weaver options@>;
	@<Parse tangler options@>;
	@<Parse creation option@>;
	fatal_error_with_parameter("unknown command line switch: %s", opt);

@

@<Parse analysis options@> =
	if (in_string_eq(opt, "-catalogue")) {
		args.catalogue_switch = TRUE; enter_main_mode(&args, ANALYSE_MODE); continue;
	}
	if (in_string_eq(opt, "-functions")) {
		args.functions_switch = TRUE; enter_main_mode(&args, ANALYSE_MODE); continue;
	}
	if (in_string_eq(opt, "-scan")) {
		args.scan_switch = TRUE; enter_main_mode(&args, ANALYSE_MODE); continue;
	}

@

@<Parse weaver options@> =
	if (in_string_eq(opt, "-weave")) {
		enter_main_mode(&args, WEAVE_MODE); continue;
	}
	if (in_string_eq(opt, "-open")) {
		args.open_pdf_switch = TRUE; enter_main_mode(&args, WEAVE_MODE); continue;
	}
	if (in_string_eq(opt, "-closed")) {
		args.open_pdf_switch = FALSE; enter_main_mode(&args, WEAVE_MODE); continue;
	}
	if (in_string_eq(opt, "-format")) {
		if (non_switch_follows) {
			in_strcpy(args.weave_format, argv[i+1]); i++;
			enter_main_mode(&args, WEAVE_MODE);
			continue;
		}
		fatal_error("-format must be followed by a format name");
	}
	if (in_string_eq(opt, "-theme")) {
		if (non_switch_follows) {
			in_strcpy(args.theme_setting, argv[i+1]); i++;
			enter_main_mode(&args, WEAVE_MODE);
			continue;
		}
		fatal_error("-theme must be followed by a chapter number or appendix letter");
	}

@

@<Parse tangler options@> =
	if (in_string_eq(opt, "-tangle")) {
		enter_main_mode(&args, TANGLE_MODE); continue;
	}
	if (in_string_eq(opt, "-tangle-to")) {
		if (non_switch_follows) {
			in_strcpy(args.tangle_setting, argv[i+1]); i++;
			enter_main_mode(&args, TANGLE_MODE); continue;
		}
		fatal_error("-tangle-to must be followed by a filename to write");		
	}

@ The single creation option is an exception, since it doesn't act on an
existing web:

@<Parse creation option@> =
	if (in_string_eq(opt, "-create")) {
		enter_main_mode(&args, CREATE_MODE); continue;
	}

@ A command-line argument not starting with a hyphen, and not already soaked
up by a preceding argument such as |-tangle-to|, is a target sigil such as
|2/eg| or |B|. Note that appendices are lettered A to O, but that |P| means
the preliminary pages.

@<Parse this as a target sigil@> =
	if (in_string_eq(opt, "index")) {
		args.swarm_mode = SWARM_INDEX;
	} else if (in_string_eq(opt, "chapters")) {
		args.swarm_mode = SWARM_CHAPTERS;
	} else if (in_string_eq(opt, "sections")) {
		args.swarm_mode = SWARM_SECTIONS;
	} else {
		if (++targets > 1) fatal_error("at most one target may be given");
		if (in_string_eq(opt, "all")) {
			in_strcpy(args.chosen_subweb, "0");
		} else if (((isalnum(opt[0])) && (opt[1] == 0)) || (pattern_match(opt, "%i+/%i+"))) {
			in_strcpy(args.chosen_subweb, opt);
			in_set(args.chosen_subweb, 0, toupper(args.chosen_subweb[0]));
		} else {
			string message;
			in_sprintf(message, "target not recognised: %s", opt);
			error_in_web(message, NULL);
			printf("The legal targets are:\n");
			printf("   all: complete web\n");
			printf("   P: all preliminaries\n");
			printf("   1: Chapter 1 (and so on)\n");
			printf("   A: Appendix A (and so on, up to Appendix O)\n");
			printf("   3/eg: section with abbreviated name \"3/eg\" (and so on)\n");
			printf("You can also, or instead, specify:\n");
			printf("   index: to weave an HTML page indexing the project\n");
			printf("   chapters: to weave all chapters as individual documents\n");
			printf("   sections: ditto with sections\n");
			exit(1);
		}
	}

@ We can only be in a single mode at a time:

@c
void enter_main_mode(inweb_instructions *args, int new_mode) {
	if (args->inweb_mode == NO_MODE) args->inweb_mode = new_mode;
	if (args->inweb_mode != new_mode)
		fatal_error("can only do one at a time - weaving, tangling or analysing");
}

@p The configuration file.
|indoc| has only a tiny configuration file, mainly to point it to other tools
it may need to use. Note that it needs none of these for tangling, so it
doesn't actually matter if the settings are wrong in such a run.

@c
void read_configuration_file(void) {
	in_strcpy(open_configuration, "");
	in_strcpy(tex_configuration, "tex");
	in_strcpy(pdftex_configuration, "pdftex");

	string config_filename;
	in_sprintf(config_filename,
		"%sinweb-configuration.txt", path_to_inweb_materials);
	file_read(config_filename, "can't open configuration file",
		TRUE, scan_config_line, NULL, NULL);
}

void scan_config_line(char *line, text_file_position *tfp, void *unused_state) {
	char *p = line;
	while (white_space(*p)) p++;
	if (p[0] == 0) return; /* skip blank lines */
	if (p[0] == '#') return; /* skip comment lines */
	int eq = 0, found = FALSE;
	for (eq = 0; p[eq]; eq++)
		if (p[eq] == '=') {
			found = TRUE;
			string setting; in_strcpy(setting, p); in_truncate(setting, eq);
			string value; in_strcpy(value, "");
			int k = eq-1;
			while ((k>=0) && (white_space(setting[k]))) in_truncate(setting, k--);
			eq++;
			while (white_space(p[eq])) eq++;
			in_strcpy(value, p+eq);
			@<Make one of the configuration settings@>;
			break;
		}
	if (found == FALSE) {
		error_in_text_file("bad configuration line", tfp);
		fprintf(stderr, "  line: %s\n", line);
	}
}

@ There's very little to see here:

@<Make one of the configuration settings@> =
	if (in_string_eq(setting, "tex")) in_strcpy(tex_configuration, value);
	else if (in_string_eq(setting, "pdftex")) in_strcpy(pdftex_configuration, value);
	else if (in_string_eq(setting, "open-command")) in_strcpy(open_configuration, value);
	else {
		string message;
		in_sprintf(message, "inweb: bad configuration setting (%s)", setting);
		error_in_web(message, NULL);
	}
