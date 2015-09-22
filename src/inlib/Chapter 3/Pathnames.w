[Pathnames::] Pathnames.

@Purpose: To manage references to locations in the host computer's file system.

@p About pathnames.
We use the word "pathname" to mean a file-system location of a folder (or
directory), and "filename" to mean a location of a file. For example:

	|/Users/rblackmore/Documents/Fireball|

is a pathname, whereas

	|/Users/rblackmore/Documents/Fireball/whoosh.aiff|

is a filename. All references to folder locations in the filing system will be
held internally as |pathname| objects, and all references to file locations as
|filename| objects. Once created, these are never destroyed or modified,
so that it's safe to store a pointer to a pathname or filename anywhere.

Note that a pathname may well be hypothetical, that is, it may well
describe a folder which doesn't exist on disc.

A full path is a linked list, but reverse-ordered: thus,

	|/Users/rblackmore/Documents/|

would be represented as a pointer to the |pathname| for "Documents", which
in turn points to one for |rblackmore|, which in turn points to |/Users|.
Thus the root of the filing system is represented by the null pointer.

Each |pathname| can represent only a single level in the hierarchy, and
its textual name is not allowed to contain the |FOLDER_SEPARATOR| character,
with just one exception: the |pathname| at the end of the chain is allowed
to begin with |FOLDER_SEPARATOR| to denote that it's at the root of the
host file system.

@c
typedef struct pathname {
	struct text_stream *intermediate;
	struct pathname *pathname_of_parent;
	int known_to_exist; /* corresponds to a directory in the filing system */
	MEMORY_MANAGEMENT
} pathname;

@ One pathname in particular is special, though it may not always exist:
on Unix-based systems, it ought to hold the user's home folder.

@c
pathname *home_path = NULL;

@p Creation.
A single segment of a pathname is made using a C string of its folder name.

@c
pathname *Pathnames::subfolder(pathname *P, char *folder_name) {
	return Pathnames::primitive(folder_name, NULL, 0, CStrings::strlen_unbounded(folder_name), P);
}

pathname *Pathnames::primitive(char *c_string, text_stream *str, int from, int to, pathname *par) {
	pathname *P = CREATE(pathname);
	P->pathname_of_parent = par;
	P->known_to_exist = FALSE;
	if (to-from <= 0) internal_error("empty intermediate pathname");
	P->intermediate = Str::new_with_capacity(to-from+1);
	if (c_string) {
		if (to-from > MAX_FILENAME_LENGTH-1)
			Errors::fatal_with_C_string("a filename grew too long: %s", c_string);
		for (int i = from; i < to; i++) PUT_TO(P->intermediate, c_string[i]);
	} else if (str) {
		for (int i = from; i < to; i++) PUT_TO(P->intermediate, Str::get(Str::at(str, i)));
	}
	return P;
}

@p Strings to pathnames.
The following takes a C string of a name and returns a pathname,
possibly relative to the home folder. Empty folder names are ignored
except possibly for an initial slash, so for example |paris/roubaix|,
|paris//roubaix| and |paris/roubaix/| are indistinguishable here, but
|/paris/roubaix| is different.

@c
pathname *Pathnames::from_string(char *path) {
	return Pathnames::from_string_relative(NULL, path);
}

pathname *Pathnames::from_string_relative(pathname *P, char *path) {
	pathname *at = P;
	int i = 0, pos = 0;
	if ((path[0]) && (P == NULL)) i++;
	for (; path[i]; i++)
		if (path[i] == FOLDER_SEPARATOR) {
			if (i > pos) at = Pathnames::primitive(path, NULL, pos, i, at);
			pos = i+1;
		}
	if (i > pos) at = Pathnames::primitive(path, NULL, pos, i, at);
	return at;
}

pathname *Pathnames::from_stream(text_stream *path) {
	return Pathnames::from_stream_relative(NULL, path);
}

pathname *Pathnames::from_stream_relative(pathname *P, text_stream *path) {
	pathname *at = P;
	int i = 0, pos = 0;
	if ((Str::get(Str::start(path))) && (P == NULL)) i++;
	for (; i < Str::len(path); i++)
		if (Str::get(Str::at(path, i)) == FOLDER_SEPARATOR) {
			if (i > pos) at = Pathnames::primitive(NULL, path, pos, i, at);
			pos = i+1;
		}
	if (i > pos) at = Pathnames::primitive(NULL, path, pos, i, at);
	return at;
}

@ This is used on |argv| entries at |main|, which use the locale's encoding.
It's convenient to reverse the slashes on Windows, since our command line
arguments may come from cross-platform makefiles which use forward slash
as the folder separator.

@c
pathname *Pathnames::from_command_line_argument(char *arg) {
	TEMPORARY_TEXT(ARG);
	Streams::open_from_locale_string(ARG, arg);
	LOOP_THROUGH_TEXT(at, ARG)
		if (Str::get(at) == '/')
			Str::put(at, FOLDER_SEPARATOR);
	pathname *P = Pathnames::from_stream(ARG);
	DISCARD_TEXT(ARG);
	return P;
}

@p Pathnames to strings.
Conversely, by the miracle of depth-first recursion:

@c
void Pathnames::to_string(char *to, pathname *P) {
	Pathnames::to_string_inner(to, P, FOLDER_SEPARATOR);
}

void Pathnames::to_string_Unixised(char *to, pathname *P) {
	Pathnames::to_string_inner(to, P, '/');
}

void Pathnames::to_string_inner(char *to, pathname *P, char divider) {
	to[0] = 0;
	if (P) Pathnames::to_string_r(to, P, MAX_FILENAME_LENGTH-1, divider);
	else { to[0] = '.'; to[1] = divider; to[2] = 0; }
}

int Pathnames::to_string_r(char *to, pathname *P, int room_left, char divider) {
	if (P->pathname_of_parent)
		room_left = Pathnames::to_string_r(to, P->pathname_of_parent, room_left, divider);
	room_left = room_left - 1 - Str::len(P->intermediate);
	if (room_left < 0)
		Errors::fatal_with_C_string("a filename grew too long: %s", to);
	int pos = CStrings::strlen_unbounded(to);
	if (pos > 0) to[pos++] = divider;
	Str::copy_to_locale_string(to+pos, P->intermediate, room_left);
	return room_left;
}

@p Streaming.

@c
void Pathnames::write(OUTPUT_STREAM, char *format_string, void *vP) {
	pathname *P = (pathname *) vP;
	char pn[MAX_FILENAME_LENGTH];
	if (format_string[1] == '/') Pathnames::to_string_Unixised(pn, P);
	else Pathnames::to_string(pn, P);
	for (int j = 0; pn[j]; j++) PUT(pn[j]);
}

@p Relative pathnames.
Occasionally we want to shorten a pathname relative to another one:
for example,

	|/Users/rblackmore/Documents/Fireball/tablature|

relative to

	|/Users/rblackmore/Documents/|

would be

	|Fireball/tablature|

@c
void Pathnames::to_string_relative(char *to, pathname *P, pathname *R) {
	char rt[MAX_FILENAME_LENGTH], pt[MAX_FILENAME_LENGTH];
	Pathnames::to_string(rt, R);
	Pathnames::to_string(pt, P);
	int n = CStrings::strlen_unbounded(rt);
	if ((strncmp(pt, rt, (size_t) n)==0) && (pt[n]==FOLDER_SEPARATOR))
		strcpy(to, pt+n+1);
	else internal_error("pathname not relative to pathname");
}

@p Numbering.

@c
pathname *Pathnames::modify_with_number(pathname *P, char *template, int val) {
	char modified[MAX_FILENAME_LENGTH];	
	Str::copy_to_ISO_string(modified, P->intermediate, MAX_FILENAME_LENGTH - 32);
	CSTRING_WRITE(modified + CStrings::len(modified), template, val);
	return Pathnames::subfolder(P->pathname_of_parent, modified);
}

@p Home folder.
We get the path to the user's home folder from the environment variable
|HOME|, if it exists.

@c
void Pathnames::start(void) {
	char *home = (char *) (getenv("HOME"));
	if (home) {
		home_path = Pathnames::from_string(home);
		home_path->known_to_exist = TRUE;
	}
}

@p Existence in the file system.
Just because we have a pathname, it doesn't follow that any folder exists
on the file system with that path.

@c
int Pathnames::create_in_file_system(pathname *P) {
	if (P == NULL) return TRUE; /* the root of the file system always exists */
	if (P->known_to_exist) return TRUE;
	char path[MAX_FILENAME_LENGTH];
	Pathnames::to_string(path, P);
	P->known_to_exist = Platform::mkdir(path);
	return P->known_to_exist;
}

@p Reading directories.
This routine writes the contents of the given folder to a plain text file,
one item per line: e.g.

	|Emily Short/|
	|Fried eggs.txt|
	|Fred Quimby/|

They can be written in any order. Any item which is itself a folder,
rather than a file, should be terminated |/| (even if that is not the
folder separator character on the current platform). The encoding
of this file will be whatever the locale encoding is for filenames,
something which will oblige us to take care later on.

@c
int Pathnames::write_contents_to_file(filename *writeto, pathname *P) {
	char path[MAX_FILENAME_LENGTH];
	Pathnames::to_string(path, P);
	char transcoded_pathname[2*MAX_FILENAME_LENGTH];
	Platform::transcode_ISO_string_to_locale(path, transcoded_pathname);

	text_stream CONTS_struct; text_stream *CONTS = &CONTS_struct;
	void *FOLDER = Platform::opendir(transcoded_pathname);
	char leaf[MAX_FILENAME_LENGTH+1];
	if (FOLDER == NULL) return FALSE;
	if (STREAM_OPEN_TO_FILE(CONTS, writeto, ISO_ENC) == FALSE) return FALSE;
	while (Platform::readdir(FOLDER, transcoded_pathname, leaf)) {
		if (leaf[0] == '.') continue;
		WRITE_TO(CONTS, "%s\n", leaf);
	}
	STREAM_CLOSE(CONTS);
	Platform::closedir(FOLDER);
	return TRUE;
}

@ And a stream version:

@c
int Pathnames::write_contents_to_stream(OUTPUT_STREAM, pathname *P) {
	char path[MAX_FILENAME_LENGTH];
	Pathnames::to_string(path, P);
	void *FOLDER = Platform::opendir(path);
	if (FOLDER == NULL) return FALSE;
	char leaf[MAX_FILENAME_LENGTH+1];
	while (Platform::readdir(FOLDER, path, leaf)) {
		if (leaf[0] == '.') continue;
		WRITE("%s\n", leaf);
	}
	Platform::closedir(FOLDER);
	return TRUE;
}
