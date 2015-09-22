[Platform::] Platform-Specific Definitions.

@Purpose: To provide any adaptations needed for inlib to work on different
"platforms", or operating systems.

@ When compiling any program using inlib, predefine one of the |PLATFORM_*|
symbols. This allows for |autoconf|-like tools to be used to build on Unix
platforms.

@c
#ifndef PLATFORM_MACOSX
#ifndef PLATFORM_WINDOWS
#ifndef PLATFORM_UNIX
#ifndef PLATFORM_ANDROID
#define PLATFORM_MACOSX /* the original home, and still the default */
#endif
#endif
#endif
#endif

@ Similarly, predefinition can be used to set the |CPU_WORDSIZE_MULTIPLIER|:
this is CPU's word size as a number of bits divided by 32, so in other words,
define this as 2 for 64-bit processors. It's not critical if this is got
wrong, since it only affects the efficiency of the memory manager.

@c
#ifndef CPU_WORDSIZE_MULTIPLIER
#define CPU_WORDSIZE_MULTIPLIER 1 /* the CPU word size is by default 32 bits */
#endif

@ It is assumed that our host filing system can manage at least 30-character
filenames, that space is legal as a character in a filename, and that trailing
extensions can be longer than 3 characters (in particular, that |.html| is
allowed). There are no clear rules but on Windows |MAX_PATH| is only 260,
and on Mac OS X the equivalent limit is 1024; both systems can house files
buried more deeply, but in both cases the user interface to the operating
system fails to recognise them. Some Linux implementations raise the
equivalent |PATH_MAX| limit as high as 4096. This seems a reasonable
compromise in practice:

@d MAX_FILENAME_LENGTH 1025

@ We now come to the blocks of definitions chosen by the above. The principal
differences lie in (i) where |inlib| expects to find user-installed extensions,
etc.; (ii) where to write the handful of files output during compilation but
not stored inside a project; (iii) how to tweak HTML to look better on this
platform; and (iv) how the filing system works and how to scan directories
inside it.

Note that we use the C preprocessor's |#define| below, and also make use
of |#ifdef|, rather than the higher-level |inweb| definition function |@d|:
this is because |inweb| does not allow conditional definitions, unlike
Knuth's |cweb|.

@p Mac OS X.

@c
#ifdef PLATFORM_MACOSX
#define PLATFORM_STRING "osx"
#define FOLDER_SEPARATOR '/'
#define SHELL_QUOTE_CHARACTER '\''
#define POSIX_DIRECTORY_HANDLING
#define JAVASCRIPT_MODEL_1
typedef long int pointer_sized_int;
#endif

@p Microsoft Windows.

@c
#ifdef PLATFORM_WINDOWS
#define PLATFORM_STRING "windows"
#define LOCALE_IS_ISO
#define FOLDER_SEPARATOR '\\'
#define SHELL_QUOTE_CHARACTER '\"'
#define WINDOWS_DIRECTORY_HANDLING
#define JAVASCRIPT_MODEL_2
typedef long int pointer_sized_int;
#endif

@p A Windows-safe form of |isdigit|. Annoyingly, the C specification allows
the implementation to have |char| either signed or unsigned. On Windows it's
generally signed. Now, consider what happens with a character value of
acute-e. This has an |unsigned char| value of 233. When stored in a |char|
on Windows, this becomes a value of |-23|. When this is passed to |isdigit()|,
we need to consider the prototype for |isdigit()|:

|int isdigit(int);|

So, when casting to int we get |-23|, not |233|. Unfortunately the return value
from |isdigit()| is only defined by the C specification for values in the
range 0 to 255 (and also EOF), so the return value for |-23| is undefined.
Unfortunately with Windows GCC, |isdigit(-23)| returns a non-zero value.

@c
#ifdef PLATFORM_WINDOWS
#define isdigit(x) Platform::Windows_isdigit(x)
int Platform::Windows_isdigit(int c) {
	return ((c >= '0') && (c <= '9')) ? 1 : 0;
}
#endif

@p Generic Unix. These settings are used both for the Linux versions
(both command-line, by Adam Thornton, and for Ubuntu, Fedora, Debian and
so forth, by Philip Chimento) and also for Solaris variants: they can
probably be used for any Unix-based system.

@c
#ifdef PLATFORM_UNIX
#include <strings.h>
#include <math.h>
#define PLATFORM_STRING "gnome"
#define FOLDER_SEPARATOR '/'
#define SHELL_QUOTE_CHARACTER '\''
#define POSIX_DIRECTORY_HANDLING
#define JAVASCRIPT_MODEL_1
typedef long int pointer_sized_int;
#endif

@p Android. These settings are used for Nathan Summers's Android versions.

@c
#ifdef PLATFORM_ANDROID
#include <strings.h>
#include <math.h>
#define PLATFORM_STRING "android"
#define SUPPRESS_MAIN
#define FOLDER_SEPARATOR '/'
#define SHELL_QUOTE_CHARACTER '\''
#define POSIX_DIRECTORY_HANDLING
#define JAVASCRIPT_MODEL_1
typedef long int pointer_sized_int;
#endif

@p Directory handling.
|inlib| would ideally handle all directory activities -- scanning directories
to see what files are there, creating new directories, etc. -- using the
standard POSIX environment, which is supposed to make such things
platform-independent. In practice Windows provides POSIX-like facilities
but with sufficient differences that we have instead written the
necessary routines twice: and in the rest of |inlib| we treat the pointer
to a directory structure as a |void *| to avoid making any assumptions
about what the structure behind it actually is. First: "pure" POSIX.

@c
#ifdef POSIX_DIRECTORY_HANDLING
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>

int Platform::mkdir(char *path_to_folder) {
	char transcoded_pathname[2*MAX_FILENAME_LENGTH];
	Platform::transcode_ISO_string_to_locale(path_to_folder, transcoded_pathname);
	int rv;
	errno = 0;
	rv = mkdir(transcoded_pathname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	if (rv == 0) return TRUE;
	if (errno == EEXIST) return TRUE;
	return FALSE;
}

void *Platform::opendir(char *path_to_folder) {
	DIR *dirp = opendir(path_to_folder);
	return (void *) dirp;
}

int Platform::readdir(void *folder, char *path_to_folder,
	char *leafname) {
	char path_to[2*MAX_FILENAME_LENGTH+2];
	struct stat file_status;
	int rv;
	DIR *dirp = (DIR *) folder;
	struct dirent *dp;
	if ((dp = readdir(dirp)) == NULL) return FALSE;
	sprintf(path_to, "%s%c%s", path_to_folder, FOLDER_SEPARATOR, dp->d_name);
	rv = stat(path_to, &file_status);
	if (rv != 0) return FALSE;
	if (S_ISDIR(file_status.st_mode)) sprintf(leafname, "%s/", dp->d_name);
	else strcpy(leafname, dp->d_name);
	return TRUE;
}

void Platform::closedir(void *folder) {
	DIR *dirp = (DIR *) folder;
	closedir(dirp);
}

#endif

@ Now for the Windows versions of these same functions, though they are
not very different.

@c
#ifdef WINDOWS_DIRECTORY_HANDLING

#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <io.h>

int Platform::mkdir(char *path_to_folder) {
	char transcoded_pathname[2*MAX_FILENAME_LENGTH];
	Platform::transcode_ISO_string_to_locale(path_to_folder, transcoded_pathname);
	int rv;
	errno = 0;
	rv = _mkdir(transcoded_pathname);
	if (rv == 0) return TRUE;
	if (errno == EEXIST) return TRUE;
	return FALSE;
}

void *Platform::opendir(char *path_to_folder) {
	DIR *dirp = opendir(path_to_folder);
    return (void *) dirp;
}

int Platform::readdir(void *folder, char *path_to_folder,
	char *leafname) {
	char path_to[2*MAX_FILENAME_LENGTH+2];
	struct _stat file_status;
	int rv;
	DIR *dirp = (DIR *) folder;
	struct dirent *dp;
	if ((dp = readdir(dirp)) == NULL) return FALSE;
	sprintf(path_to, "%s%c%s", path_to_folder, FOLDER_SEPARATOR, dp->d_name);
	rv = _stat(path_to, &file_status);
	if (rv != 0) return FALSE;
	if (S_ISDIR(file_status.st_mode))
		sprintf(leafname, "%s%c", dp->d_name, FOLDER_SEPARATOR);
	else strcpy(leafname, dp->d_name);
	return TRUE;
}

void Platform::closedir(void *folder) {
	DIR *dirp = (DIR *) folder;
	closedir(dirp);
}

#endif

@p Locale issues.
The following is intended to handle possible differences of text encoding
in filenames, which (for Unix-based systems) depend on the current locale.
The default is UTF-8 since Mac OS X, and probably most other modern Unixes,
use this.

@c
#ifndef LOCALE_IS_ISO
#ifndef LOCALE_IS_UTF8
#define LOCALE_IS_UTF8 1
#endif
#endif

@ The function |Platform::iso_fopen| should behave exactly like |fopen|, but encoding
the filename as ISO Latin-1 text. Thus, it can actually be just |fopen|
on platforms whose locale uses ISO as its filename encoding; but on
platforms with UTF-8 encoding of filenames, transcoding is needed.

The function |Platform::iso_fopen_caseless| is used when we want to cope well if
the casing (upper vs. lower case, that is) in the filename might not be
right. On a caseless-read filing system such as that of Mac OS X (default)
or Windows, there is no issue. But for a cased filing system, we may need
to provide something.

@c
FILE *Platform::iso_fopen(filename *F, char *usage) {
	char transcoded_pathname[4*MAX_FILENAME_LENGTH];
	TEMPORARY_TEXT(FN);
	WRITE_TO(FN, "%f", F);
	Str::copy_to_locale_string(transcoded_pathname, FN, 4*MAX_FILENAME_LENGTH);
	DISCARD_TEXT(FN);
	return fopen(transcoded_pathname, usage);
}

FILE *Platform::iso_fopen_caseless(filename *F, char *usage) {
	char transcoded_pathname[4*MAX_FILENAME_LENGTH];
	TEMPORARY_TEXT(FN);
	WRITE_TO(FN, "%f", F);
	Str::copy_to_locale_string(transcoded_pathname, FN, 4*MAX_FILENAME_LENGTH);
	DISCARD_TEXT(FN);
	return CIFilingSystem::fopen(transcoded_pathname, usage);
}

@ And the locale-dependent part is:

@c
#ifdef LOCALE_IS_ISO
void Platform::transcode_ISO_string_to_locale(char *from, char *to) {
	CStrings::copy(to, from);
}
#endif

#ifdef LOCALE_IS_UTF8
void Platform::transcode_ISO_string_to_locale(char *from, char *to) {
	CStrings::transcode_ISO_string_to_UTF8(from, to);
}
#endif
