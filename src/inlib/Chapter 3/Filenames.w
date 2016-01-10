[Filenames::] Filenames.

@Purpose: Given the different environments in which we might be running, and
the large range of files we may need from within a project folder, it's useful
to have a section of code simply to deduce filenames.

@p Storage.
Filename objects behave much like pathname ones, but they have their own
structure in order that the two are distinct C types. Individual filenames
are a single instance of:

@c
typedef struct filename {
	struct pathname *pathname_of_location;
	struct text_stream *leafname;
	MEMORY_MANAGEMENT
} filename;

@p Creation.
A single segment of a pathname is made using a C string of its folder name.

@c
filename *Filenames::in_folder(pathname *P, char *file_name) {
	return Filenames::primitive(file_name, NULL, 0, CStrings::strlen_unbounded(file_name), P);
}

filename *Filenames::in_folder_S(pathname *P, text_stream *file_name) {
	return Filenames::primitive(NULL, file_name, 0, Str::len(file_name), P);
}

filename *Filenames::primitive(char *c_string, text_stream *S, int from, int to, pathname *P) {
	filename *F = CREATE(filename);
	F->pathname_of_location = P;
	if (to-from > MAX_FILENAME_LENGTH-1)
		Errors::fatal_with_C_string("a filename grew too long: %s", c_string);
	if (to-from <= 0)
		internal_error("empty intermediate pathname");
	F->leafname = Str::new_with_capacity(to-from+1);
	if (c_string) {
		for (int i = from; i < to; i++) PUT_TO(F->leafname, c_string[i]);
	} else {
		string_position pos = Str::at(S, from);
		for (int i = from; i < to; i++, pos = Str::forward(pos))
			PUT_TO(F->leafname, Str::get(pos));
	}
	return F;
}

@p Strings to filenames.
The following takes a C string of a name and returns a filename.

@c
filename *Filenames::from_string(char *path) {
	int i = 0, pos = -1;
	for (; path[i]; i++)
		if (path[i] == FOLDER_SEPARATOR)
			pos = i;
	pathname *at = NULL;
	if (pos > 0) at = Pathnames::primitive(path, NULL, 0, pos, NULL);
	if (pos+1 < i) return Filenames::in_folder(at, path+pos+1);
	internal_error("empty leafname");
	return NULL;
}

filename *Filenames::from_stream(text_stream *path) {
	int i = 0, pos = -1;
	LOOP_THROUGH_TEXT(at, path) {
		if (Str::get(at) == FOLDER_SEPARATOR) pos = i;
		i++;
	}
	pathname *P = NULL;
	if (pos > 0) P = Pathnames::primitive(NULL, path, 0, pos, NULL);
	return Filenames::primitive(NULL, path, pos+1, Str::len(path), P);
}

@ This is used on |argv| entries at |main|, which use the locale's encoding.

When these are UTF-8 encoded, there's still ambiguity between, say,
c-cedilla represented as |E7| ("Latin Small Letter C With Cedilla") but
as the sequence |63| |0327| (lower case c, followed by "Combining Cedilla").
On Mac OS X, for example, and despite assurances to the contrary in its
documentation, the precomposed form in two characters which is passed in
a |dirent| is not then recognised if fed back into |fopen|. To avoid that,
we normalise filenames by composing their accents.

It's convenient to reverse the slashes on Windows, since our command line
arguments may come from cross-platform makefiles which use forward slash
as the folder separator.

@c
filename *Filenames::from_command_line_argument(char *arg) {
	TEMPORARY_TEXT(ARG);
	Streams::open_from_locale_string(ARG, arg);
	LOOP_THROUGH_TEXT(at, ARG)
		if (Str::get(at) == '/')
			Str::put(at, FOLDER_SEPARATOR);
	filename *F = Filenames::from_stream(ARG);
	DISCARD_TEXT(ARG);
	return F;
}

@p Filenames to strings.
And conversely once again:

@c
void Filenames::to_string(char *to, filename *F) {
	to[0] = 0;
	Pathnames::to_string(to, F->pathname_of_location);
	int pos = CStrings::strlen_unbounded(to);
	if (pos + 1 + Str::len(F->leafname) + 1 > MAX_FILENAME_LENGTH)
		Errors::fatal_with_C_string("a filename grew too long: %s", to);
	to[pos] = FOLDER_SEPARATOR;
	Str::copy_to_ISO_string(to+pos+1, F->leafname, MAX_FILENAME_LENGTH);
}

void Filenames::to_string_Unixised(char *to, filename *F) {
	to[0] = 0;
	Pathnames::to_string(to, F->pathname_of_location);
	int pos = CStrings::strlen_unbounded(to);
	if (pos + 1 + Str::len(F->leafname) + 1 > MAX_FILENAME_LENGTH)
		Errors::fatal_with_C_string("a filename grew too long: %s", to);
	to[pos] = '/';
	Str::copy_to_ISO_string(to+pos+1, F->leafname, MAX_FILENAME_LENGTH);
}

void Filenames::print_string(char *label, filename *F) {
	char written_out[MAX_FILENAME_LENGTH];
	Filenames::to_string(written_out, F);
	printf("%s: %s\n", label, written_out);
}

@ And again relative to a given pathname:

@c
void Filenames::to_string_relative(char *to, filename *F, pathname *P) {
	char ft[MAX_FILENAME_LENGTH], pt[MAX_FILENAME_LENGTH];
	Filenames::to_string(ft, F);
	Pathnames::to_string(pt, P);
	int n = CStrings::strlen_unbounded(pt);
	if ((strncmp(ft, pt, (size_t) n)==0) && (ft[n]==FOLDER_SEPARATOR))
		strcpy(to, ft+n+1);
	else internal_error("filename not relative to pathname");
}

@p Streaming.

@c
void Filenames::write(OUTPUT_STREAM, char *format_string, void *vF) {
	filename *F = (filename *) vF;
	if (F == NULL) WRITE("<no file>");
	else {
		char fn[MAX_FILENAME_LENGTH];
		if (format_string[1] == '/') Filenames::to_string_Unixised(fn, F);
		else Filenames::to_string(fn, F);
		for (int j = 0; fn[j]; j++) PUT(fn[j]);
	}
}

@p Reading off the folder.

@c
pathname *Filenames::get_path_to(filename *F) {
	if (F == NULL) return NULL;
	return F->pathname_of_location;
}

@p Reading off the leafname.

@c
filename *Filenames::without_path(filename *F) {
	return Filenames::in_folder_S(NULL, F->leafname);
}

text_stream *Filenames::get_leafname(filename *F) {
	if (F == NULL) return NULL;
	return F->leafname;
}

void Filenames::write_unextended_leafname(OUTPUT_STREAM, filename *F) {
	LOOP_THROUGH_TEXT(pos, F->leafname) {
		wchar_t c = Str::get(pos);
		if (c == '.') return;
		PUT(c);
	}
}

@p Filename extensions.
The following is cautiously written because of an oddity in Windows's handling
of filenames, which are allowed to have trailing dots or spaces, in a way
which isn't necessarily visible to the user, who may have added these by
an accidental brush of the keyboard. Thus |frog.jpg .| should be treated
as equivalent to |frog.jpg| when deciding the likely file format.

@c
void Filenames::write_extension(OUTPUT_STREAM, filename *F) {
	int on = FALSE;
	LOOP_THROUGH_TEXT(pos, F->leafname) {
		wchar_t c = Str::get(pos);
		if (c == '.') on = TRUE;
		if (on) PUT(c);
	}
}

filename *Filenames::set_extension(filename *F, char *extension) {
	TEMPORARY_TEXT(NEWLEAF);
	LOOP_THROUGH_TEXT(pos, F->leafname) {
		wchar_t c = Str::get(pos);
		if (c == '.') break;
		PUT_TO(NEWLEAF, c);
	}
	if (extension) {
		if (extension[0] == '.') extension++;
		if (extension[0]) WRITE_TO(NEWLEAF, ".%s", extension);
	}
	filename *N = Filenames::in_folder_S(F->pathname_of_location, NEWLEAF);
	DISCARD_TEXT(NEWLEAF);
	return N;
}

@p Guessing file formats.
The following guesses the file format from its file extension:

@d FORMAT_PERHAPS_HTML 1
@d FORMAT_PERHAPS_JPEG 2
@d FORMAT_PERHAPS_PNG 3
@d FORMAT_PERHAPS_OGG 4
@d FORMAT_PERHAPS_AIFF 5
@d FORMAT_PERHAPS_MIDI 6
@d FORMAT_PERHAPS_MOD 7
@d FORMAT_PERHAPS_GLULX 8
@d FORMAT_PERHAPS_ZCODE 9
@d FORMAT_PERHAPS_SVG 10
@d FORMAT_PERHAPS_GIF 11
@d FORMAT_UNRECOGNISED 0

@c
int Filenames::guess_format(filename *F) {
	TEMPORARY_TEXT(EXT);
	Filenames::write_extension(EXT, F);

	TEMPORARY_TEXT(NORMALISED);
	LOOP_THROUGH_TEXT(pos, EXT) {
		wchar_t c = Str::get(pos);
		if (c != ' ') PUT_TO(NORMALISED, Characters::w_tolower(c));
	}
	DISCARD_TEXT(EXT);

	int verdict = FORMAT_UNRECOGNISED;
	if (Str::eq_C_string(NORMALISED, L".html")) verdict = FORMAT_PERHAPS_HTML;
	else if (Str::eq_C_string(NORMALISED, L".htm")) verdict = FORMAT_PERHAPS_HTML;
	else if (Str::eq_C_string(NORMALISED, L".jpg")) verdict = FORMAT_PERHAPS_JPEG;
	else if (Str::eq_C_string(NORMALISED, L".jpeg")) verdict = FORMAT_PERHAPS_JPEG;
	else if (Str::eq_C_string(NORMALISED, L".png")) verdict = FORMAT_PERHAPS_PNG;
	else if (Str::eq_C_string(NORMALISED, L".ogg")) verdict = FORMAT_PERHAPS_OGG;
	else if (Str::eq_C_string(NORMALISED, L".aiff")) verdict = FORMAT_PERHAPS_AIFF;
	else if (Str::eq_C_string(NORMALISED, L".aif")) verdict = FORMAT_PERHAPS_AIFF;
	else if (Str::eq_C_string(NORMALISED, L".midi")) verdict = FORMAT_PERHAPS_MIDI;
	else if (Str::eq_C_string(NORMALISED, L".mid")) verdict = FORMAT_PERHAPS_MIDI;
	else if (Str::eq_C_string(NORMALISED, L".mod")) verdict = FORMAT_PERHAPS_MOD;
	else if (Str::eq_C_string(NORMALISED, L".svg")) verdict = FORMAT_PERHAPS_SVG;
	else if (Str::eq_C_string(NORMALISED, L".gif")) verdict = FORMAT_PERHAPS_GIF;
	else if (Str::len(NORMALISED) > 0) {
		if ((Str::get(Str::at(NORMALISED, 0)) == 'z') &&
			(Characters::w_isdigit(Str::get(Str::at(NORMALISED, 1)))) &&
			(Str::len(NORMALISED) == 2))
			verdict = FORMAT_PERHAPS_ZCODE;
		else if (Str::get(Str::back(Str::end(NORMALISED))) == 'x')
			verdict = FORMAT_PERHAPS_GLULX;
	}
	DISCARD_TEXT(NORMALISED);
	return verdict;
}
