[CommandLine::] Command Line Arguments.

@Purpose: To parse the command line arguments with which inweb was called,
and to handle any errors it needs to issue.

@p Setting names.

@d LOG_CLSW 0
@d PLATFORM_CLSW 1
@d CRASH_CLSW 2

@d NO_INLIB_CLSWS 3

@c
typedef struct command_line_switch {
	int switch_id;
	struct text_stream *switch_name; /* e.g., |log| */
	int valency; /* 1 for bare, 2 for one argument follows */
	MEMORY_MANAGEMENT
} command_line_switch;

@

@c
void CommandLine::declare_switch(int id, wchar_t *name_literal, int val) {
	command_line_switch *cls = CREATE(command_line_switch);
	cls->switch_name = Str::new_from_wide_string(name_literal);
	cls->switch_id = id;
	cls->valency = val;
}

@p Reading the command line.

@c
void CommandLine::read(int argc, char **argv, void *state, void (*f)(int, text_stream *, void *)) {
	for (int i=1; i<argc; i++) {
		int switched = FALSE;
		char *p = argv[i];
		while (p[0] == '-') { p++; switched = TRUE; } /* allow a doubled-dash as a single */
		STRING(opt);
		Streams::write_locale_string(opt, p);
		if (switched) @<Parse this as a switch@>
		else Errors::fatal_with_text("unknown command line argument: %S", opt);
	}
}

@

@<Parse this as a switch@> =
	int found = FALSE;
	command_line_switch *cls;
	LOOP_OVER(cls, command_line_switch)
		if (Str::eq(opt, cls->switch_name)) {
			STRING(arg);
			if (cls->valency > 1) {
				if (i+1 == argc) Errors::fatal_with_text("no argument for: -%S", opt);
				Streams::write_locale_string(arg, argv[++i]);
			}
			switch (cls->switch_id) {
				case CRASH_CLSW: Errors::enter_debugger_mode(); break;
				case LOG_CLSW: @<Parse debugging log inclusion@>; break;
				case PLATFORM_CLSW: @<Parse platform name@>; break;
				default:
					if (f) {
						if (cls->valency == 1) (*f)(cls->switch_id, NULL, state);
						else (*f)(cls->switch_id, arg, state);
					}
					break;
			}
			found = TRUE;
		}
	if (found == FALSE)
		Errors::fatal_with_text("unknown command line switch: -%S", opt);

@

@<Parse debugging log inclusion@> =
	if (Log::get_debug_log_filename() == NULL) {
		filename *F = Filenames::in_folder(Pathnames::from_string(INTOOL_NAME),
			"debug-log.txt");
		Log::set_debug_log_filename(F);
	}
	Log::open();
	Log::set_aspect_from_command_line(arg);

@

@<Parse platform name@> =
	/* ignore for now */
