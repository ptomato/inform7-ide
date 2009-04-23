1/main: Main.

@Purpose: To parse command-line arguments and take the necessary steps to
obey them.

@Definitions:

@ We will need the following:

@c
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "time.h"
#include "ctype.h"

@ We identify which platform we're running on thus:

@d OSX_PLATFORM 1
@d WINDOWS_PLATFORM 2
@d UNIX_PLATFORM 3

@ Since we use flexible-sized memory allocation, |cblorb| contains few hard
maxima on the size or complexity of its input, but:

@d MAX_FILENAME_LENGTH 2048 /* total length of pathname including leaf and extension */
@d MAX_EXTENSION_LENGTH 32 /* extension part of filename, for auxiliary files */
@d MAX_VAR_NAME_LENGTH 32 /* length of name of placeholder variable like ``[AUTHOR]'' */
@d MAX_TEXT_FILE_LINE_LENGTH 10240 /* for any single line in the project's source text */
@d MAX_SOURCE_TEXT_LINES 2000000000; /* enough for 300 copies of the Linux kernel source -- plenty! */

@ Miscellaneous settings:

@d VERSION "cBlorb 1.1"

@d TRUE 1
@d FALSE 0

@ Some global variables:

@c
char SEP_CHAR = '/'; /* set to the correct value for the platform by |main()| */
int trace_mode = FALSE; /* print diagnostics to |stdout| while running? */
int error_count = 0; /* number of error messages produced so far */
int current_year_AD = 0; /* e.g., 2008 */

int blorb_file_size = 0; /* size in bytes of the blorb file written */
int no_pictures_included = 0; /* number of picture resources included in the blorb */
int no_sounds_included = 0; /* number of sound resources included in the blorb */

int use_css_code_styles = FALSE; /* use |<span class="X">| markings when setting code */
char project_folder[MAX_FILENAME_LENGTH]; /* pathname of I7 project folder, if any */
char release_folder[MAX_FILENAME_LENGTH]; /* pathname of folder for website to write, if any */
int cover_exists = FALSE; /* an image is specified as cover art */
int cover_is_in_JPEG_format = TRUE; /* as opposed to |PNG| format */

@-------------------------------------------------------------------------------

@pp Main.
Like most programs, this one parses command-line arguments, sets things up,
reads the input and then writes the output.

That's a little over-simplified, though, because it also produces auxiliary
outputs along the way, in the course of parsing the blurb file. The blorb
file is only the main output -- there might also be a web page and a solution
file, for instance.

@c
/*****/ int main(int argc, char *argv[]) {
	int platform, produce_help;
	char blurb_filename[MAX_FILENAME_LENGTH];
	char blorb_filename[MAX_FILENAME_LENGTH];

	@<Make the default settings@>;
	@<Parse command-line arguments@>;

	start_memory();
	establish_time();
	initialise_placeholders();
	print_banner();

	if (produce_help) { @<Produce help@>; return 0; }

	parse_blurb_file(blurb_filename);
	write_blorb_file(blorb_filename);
	create_requested_material();
	
	print_report();
	free_memory();
	return 0;
}

@

@<Make the default settings@> =
	platform = OSX_PLATFORM;
	produce_help = FALSE;
	release_folder[0] = 0;
	project_folder[0] = 0;
	strcpy(blurb_filename, "Release.blurb");
	strcpy(blorb_filename, "story.zblorb");

@

@<Parse command-line arguments@> =
	int arg, names = FALSE;
	for (arg = 1, names = 0; arg < argc; arg++) {
		char *p = argv[arg];
		if (strlen(p) >= MAX_FILENAME_LENGTH) {
			fprintf(stderr, "cblorb: command line argument %d too long\n", arg+1);
			return 1;
		}
		if (strcmp(p, "-help") == 0) { produce_help = TRUE; continue; }
		if (strcmp(p, "-osx") == 0) { platform = OSX_PLATFORM; continue; }
		if (strcmp(p, "-windows") == 0) { platform = WINDOWS_PLATFORM; continue; }
		if (strcmp(p, "-unix") == 0) { platform = UNIX_PLATFORM; continue; }
		if (strcmp(p, "-trace") == 0) { trace_mode = TRUE; continue; }
		if (strcmp(p, "-project") == 0) {
			arg++; if (arg == argc) @<Command line syntax error@>;
			strcpy(project_folder, argv[arg]);
			continue;
		}
		if (p[0] == '-') @<Command line syntax error@>;
		names++;
		switch (names) {
			case 1: strcpy(blurb_filename, p); break;
			case 2: strcpy(blorb_filename, p); break;
			default: @<Command line syntax error@>;
		}
	}
	
	if (platform == WINDOWS_PLATFORM) SEP_CHAR = '\\'; else SEP_CHAR = '/';
	
	if (project_folder[0] != 0) {
		if (names > 0) @<Command line syntax error@>;
		sprintf(blurb_filename, "%s%cRelease.blurb", project_folder, SEP_CHAR);
		sprintf(blorb_filename, "%s%cBuild%coutput.zblorb", project_folder, SEP_CHAR, SEP_CHAR);
	}

	if (trace_mode)
		printf("! Blurb in: <%s>\n! Blorb out: <%s>\n",
			blurb_filename, blorb_filename);

@

@<Produce help@> =
	printf("This is cblorb, a component of Inform 7 for packaging up IF materials.\n\n");
	@<Show command line usage@>;
	summarise_blurb();

@

@<Command line syntax error@> =
	@<Show command line usage@>;
	return 1;

@

@<Show command line usage@> =
	printf("usage: cblorb -platform [-options] [blurbfile [blorbfile]]\n\n");
	printf("  Where -platform should be -osx (default), -windows, or -unix\n");
	printf("  As an alternative to giving filenames for the blurb and blorb,\n");
	printf("    -project Whatever.inform\n");
	printf("  sets blurbfile and blorbfile names to the natural choices.\n");
	printf("  The other possible options are:\n");
	printf("    -help ... print this usage summary\n");
	printf("    -trace ... print diagnostic information during run\n");

@p Time.
It wouldn't be a tremendous disaster if the host OS had no access to an
accurate time of day, in fact.

@c
time_t the_present;
struct tm *here_and_now;

void establish_time(void) {
	the_present = time(NULL);
	here_and_now = localtime(&the_present);
}

@ The placeholder variable [YEAR] is initialised to the year in which |cBlorb|
runs, according to the host operating system, at least. (It can of course then
be overridden by commands in the blurb file, and Inform always does this in
the blurb files it writes. But it leaves [DATESTAMP] and [TIMESTAMP] alone.)

@c
/**/ void initialise_time_variables(void) {
	char datestamp[100], infocom[100], timestamp[100];
	char *weekdays[] = { "Sunday", "Monday", "Tuesday", "Wednesday",
		"Thursday", "Friday", "Saturday" };
	char *months[] = { "January", "February", "March", "April", "May", "June",
		"July", "August", "September", "October", "November", "December" };
	set_placeholder_to_number("YEAR", here_and_now->tm_year+1900);
	sprintf(datestamp, "%s %d %s %d", weekdays[here_and_now->tm_wday],
		here_and_now->tm_mday, months[here_and_now->tm_mon], here_and_now->tm_year+1900);
	sprintf(infocom, "%02d%02d%02d",
		here_and_now->tm_year-100, here_and_now->tm_mon + 1, here_and_now->tm_mday);
	sprintf(timestamp, "%02d:%02d.%02d", here_and_now->tm_hour,
		here_and_now->tm_min, here_and_now->tm_sec);
	set_placeholder_to("DATESTAMP", datestamp, 0);
	set_placeholder_to("INFOCOMDATESTAMP", infocom, 0);
	set_placeholder_to("TIMESTAMP", timestamp, 0);
}

@p Opening and closing banners.
Note that |cBlorb| customarily prints informational messages with an initial
|!|, so that the piped output from |cBlorb| could be used as an |Include|
file in I6 code; that isn't in fact how I7 uses |cBlorb|, but it's traditional
for blorbing programs to do this.

@c
void print_banner(void) {
	printf("! %s [executing on %s at %s]\n",
		VERSION, read_placeholder("DATESTAMP"), read_placeholder("TIMESTAMP"));
	printf("! The blorb spell (safely protect a small object ");
	printf("as though in a strong box).\n");
}

@ And then at the end:

@c
void print_report(void) {
	if (error_count > 0) {
		printf("! Completed: %d error(s)\n", error_count);
		exit(1);
	}
	if (blorb_file_size > 0) {
		printf("! Completed: wrote blorb file of size %d bytes ", blorb_file_size);
		printf("(%d picture(s), %d sound(s))\n", no_pictures_included, no_sounds_included);
	} else {
		printf("! Completed: no blorb output requested\n");
	}
}

