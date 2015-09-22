[Configuration::] Configuration.

@Purpose: To parse the command line arguments with which inweb was called,
and to handle any errors it needs to issue.

@p Instructions.
The following structure encodes a set of instructions from the user (probably
from the command line) about what Inweb should do on this run:

@c
typedef struct inweb_instructions {
	int inweb_mode; /* our main mode of operation: one of the |*_MODE| constants */
	struct pathname *chosen_web; /* project folder relative to cwd */
	string chosen_subweb; /* which subset of this web we apply to (often, all of it) */

	int swarm_mode; /* relevant to weaving only: one of the |*_SWARM| constants */
	string theme_setting; /* |-theme X|: theme for a topic-restricted weave */
	string weave_format; /* |-format X|: for example, |-format TeX| */

	int catalogue_switch; /* |-catalogue|: print catalogue of sections */
	int functions_switch; /* |-functions|: print catalogue of functions within sections */
	int open_pdf_switch; /* |-open-pdf|: open any woven PDF in the OS once it is made */
	int scan_switch; /* |-scan|: simply show the syntactic scan of the source */
	struct filename *tangle_setting; /* |-tangle X|: the pathname X, if supplied */
	int verbose_switch; /* |-verbose|: print names of files read to stdout */
	
	struct pathname *location_setting; /* |-at X|: where this program is */
	struct pathname *import_setting; /* |-import X|: where to find imported webs */
} inweb_instructions;

@p Reading the command line.

@c
inweb_instructions Configuration::read(int argc, char **argv) {
	inweb_instructions args;
	args.inweb_mode = NO_MODE;
	args.swarm_mode = SWARM_OFF;
	args.catalogue_switch = FALSE;
	args.functions_switch = FALSE;
	args.open_pdf_switch = NOT_APPLICABLE;
	args.scan_switch = FALSE;
	args.verbose_switch = FALSE;
	args.chosen_web = NULL;
	CStrings::copy(args.chosen_subweb, "0"); /* by default, the entire web is the target */
	args.tangle_setting = NULL;
	CStrings::copy(args.theme_setting, "");
	CStrings::copy(args.weave_format, "HTML");
	args.location_setting = NULL;
	args.import_setting = NULL;

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
			if (args.chosen_web == NULL)
				args.chosen_web = Pathnames::from_command_line_argument(opt);
			else @<Parse this as a target sigil@>;
		}
	}
	if (args.chosen_web == NULL) args.inweb_mode = NO_MODE;
	return args;
}

@

@<Parse this as a switch@> =
	if (opt[1] == '-') opt++; /* allow a doubled-dash as equivalent to a single */
	if (CStrings::eq(opt, "-verbose")) {
		args.verbose_switch = TRUE; continue;
	}
	if (CStrings::eq(opt, "-at")) {
		if (non_switch_follows) {
			args.location_setting = Pathnames::from_command_line_argument(argv[i+1]);
			i++; continue;
		}
		Errors::fatal("-at must be followed by the pathname where inweb lives");
	}
	if (CStrings::eq(opt, "-import-from")) {
		if (non_switch_follows) {
			args.import_setting = Pathnames::from_command_line_argument(argv[i+1]);
			i++; continue;
		}
		Errors::fatal("-import must be followed by the pathname where imported webs live");
	}
	@<Parse analysis options@>;
	@<Parse weaver options@>;
	@<Parse tangler options@>;
	@<Parse creation option@>;
	Errors::fatal_with_C_string("unknown command line switch: %s", opt);

@

@<Parse analysis options@> =
	if (CStrings::eq(opt, "-catalogue")) {
		args.catalogue_switch = TRUE; Configuration::enter_main_mode(&args, ANALYSE_MODE); continue;
	}
	if (CStrings::eq(opt, "-functions")) {
		args.functions_switch = TRUE; Configuration::enter_main_mode(&args, ANALYSE_MODE); continue;
	}
	if (CStrings::eq(opt, "-scan")) {
		args.scan_switch = TRUE; Configuration::enter_main_mode(&args, ANALYSE_MODE); continue;
	}

@

@<Parse weaver options@> =
	if (CStrings::eq(opt, "-weave")) {
		Configuration::enter_main_mode(&args, WEAVE_MODE); continue;
	}
	if (CStrings::eq(opt, "-open")) {
		args.open_pdf_switch = TRUE; Configuration::enter_main_mode(&args, WEAVE_MODE); continue;
	}
	if (CStrings::eq(opt, "-closed")) {
		args.open_pdf_switch = FALSE; Configuration::enter_main_mode(&args, WEAVE_MODE); continue;
	}
	if (CStrings::eq(opt, "-format")) {
		if (non_switch_follows) {
			CStrings::copy(args.weave_format, argv[i+1]); i++;
			Configuration::enter_main_mode(&args, WEAVE_MODE);
			continue;
		}
		Errors::fatal("-format must be followed by a format name");
	}
	if (CStrings::eq(opt, "-theme")) {
		if (non_switch_follows) {
			CStrings::copy(args.theme_setting, argv[i+1]); i++;
			Configuration::enter_main_mode(&args, WEAVE_MODE);
			continue;
		}
		Errors::fatal("-theme must be followed by a chapter number or appendix letter");
	}

@

@<Parse tangler options@> =
	if (CStrings::eq(opt, "-tangle")) {
		Configuration::enter_main_mode(&args, TANGLE_MODE); continue;
	}
	if (CStrings::eq(opt, "-tangle-to")) {
		if (non_switch_follows) {
			args.tangle_setting = Filenames::from_command_line_argument(argv[i+1]); i++;
			Configuration::enter_main_mode(&args, TANGLE_MODE); continue;
		}
		Errors::fatal("-tangle-to must be followed by a filename to write");		
	}

@ The single creation option is an exception, since it doesn't act on an
existing web:

@<Parse creation option@> =
	if (CStrings::eq(opt, "-create")) {
		Configuration::enter_main_mode(&args, CREATE_MODE); continue;
	}

@ A command-line argument not starting with a hyphen, and not already soaked
up by a preceding argument such as |-tangle-to|, is a target sigil such as
|2/eg| or |B|. Note that appendices are lettered A to O, but that |P| means
the preliminary pages.

@<Parse this as a target sigil@> =
	if (CStrings::eq(opt, "index")) {
		args.swarm_mode = SWARM_INDEX;
	} else if (CStrings::eq(opt, "chapters")) {
		args.swarm_mode = SWARM_CHAPTERS;
	} else if (CStrings::eq(opt, "sections")) {
		args.swarm_mode = SWARM_SECTIONS;
	} else {
		if (++targets > 1) Errors::fatal("at most one target may be given");
		if (CStrings::eq(opt, "all")) {
			CStrings::copy(args.chosen_subweb, "0");
		} else if (((isalnum(opt[0])) && (opt[1] == 0)) || (ISORegexp::match_0(opt, "%i+/%i+"))) {
			CStrings::copy(args.chosen_subweb, opt);
			CStrings::set_char(args.chosen_subweb, 0, toupper(args.chosen_subweb[0]));
		} else {
			string message;
			CSTRING_WRITE(message, "target not recognised: %s", opt);
			Main::error_in_web(message, NULL);
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
void Configuration::enter_main_mode(inweb_instructions *args, int new_mode) {
	if (args->inweb_mode == NO_MODE) args->inweb_mode = new_mode;
	if (args->inweb_mode != new_mode)
		Errors::fatal("can only do one at a time - weaving, tangling or analysing");
}

@p The configuration file.
|indoc| has only a tiny configuration file, mainly to point it to other tools
it may need to use. Note that it needs none of these for tangling, so it
doesn't actually matter if the settings are wrong in such a run.

@c
void Configuration::read_configuration_file(void) {
	CStrings::copy(open_configuration, "");
	CStrings::copy(tex_configuration, "tex");
	CStrings::copy(pdftex_configuration, "pdftex");

	TextFiles::read_with_lines_to_ISO(
		Filenames::in_folder(path_to_inweb_materials, "inweb-configuration.txt"),
		"can't open configuration file",
		TRUE, Configuration::scan_config_line, NULL, NULL);
}

void Configuration::scan_config_line(char *line, text_file_position *tfp, void *unused_state) {
	char *p = line;
	while (ISORegexp::white_space(*p)) p++;
	if (p[0] == 0) return; /* skip blank lines */
	if (p[0] == '#') return; /* skip comment lines */
	int eq = 0, found = FALSE;
	for (eq = 0; p[eq]; eq++)
		if (p[eq] == '=') {
			found = TRUE;
			string setting; CStrings::copy(setting, p); CStrings::truncate(setting, eq);
			string value; CStrings::copy(value, "");
			int k = eq-1;
			while ((k>=0) && (ISORegexp::white_space(setting[k]))) CStrings::truncate(setting, k--);
			eq++;
			while (ISORegexp::white_space(p[eq])) eq++;
			CStrings::copy(value, p+eq);
			@<Make one of the configuration settings@>;
			break;
		}
	if (found == FALSE) {
		Errors::in_text_file("bad configuration line", tfp);
		fprintf(stderr, "  line: %s\n", line);
	}
}

@ There's very little to see here:

@<Make one of the configuration settings@> =
	if (CStrings::eq(setting, "tex")) CStrings::copy(tex_configuration, value);
	else if (CStrings::eq(setting, "pdftex")) CStrings::copy(pdftex_configuration, value);
	else if (CStrings::eq(setting, "open-command")) CStrings::copy(open_configuration, value);
	else {
		string message;
		CSTRING_WRITE(message, "inweb: bad configuration setting (%s)", setting);
		Main::error_in_web(message, NULL);
	}
