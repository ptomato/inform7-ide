[Inlib::] Inlib.

@Purpose: A very few fundamentals to get started.

@p Introduction.
Inlib is a work in progress. It consolidates foundational code which was
previously duplicated across the source for multiple Inform-related tools: the
aim is to reduce this untidiness and to provide a uniform level of service
(for example, in handling of Unicode text, or providing for platform
dependencies). Eventually all of the Inform tools will use it: for the moment
only some do, and don't use all of its functionality. As I type this, it's
only a couple of weeks old.

To use |inlib|, the Contents section of a web should include:

	|Import: inlib|

before beginning the chapter rundown. There are then a few conventions
which must be followed. The |main| routine for the client should, as one
of its very first acts, call |Inlib::start()|, and should similarly, just
before it exits, call |Inlib::end()|: see below for details.

In addition, the client's source code needs to define a few symbols to indicate
what it needs in the way of memory allocation. Beyond that, all functionality
is optional: take it or leave it. See the example web |inexample| for a model.

@p Basic definitions.
These are all from the ANSI C standard library (or the pthread standard),
which means that Inweb will tangle them up to the top of the C source code.

@c
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wchar.h>
#ifdef PLATFORM_WINDOWS
#include "inlib/Headers/win32.h"
#else
#include <pthread.h>
#include <unistd.h>
#endif

@
We'll use three truth states, the third of which can also mean "unknown".

@d TRUE 1
@d FALSE 0
@d NOT_APPLICABLE 2

@ We'll define a few variadic macros here, because there are awkward issues
with code ordering if we leave them until later. They are written in the
old-fashioned way, for compatibility with old copies of GCC, and avoid the
need for comma deletion around empty tokens, as that is a point of
incompatibility between implementations of the C preprocessor |cpp|. All the
same, if you're porting this code, you may need to rewrite the macro with
|...| in place of |args...| in the header, and then |__VA_ARGS__| in place
of |args| in the definition: that being the modern way, apparently.

|WRITE| is essentially |sprintf| and |fprintf| combined, since it prints
formatted text to the current stream, which could be either a string or a
file. |PRINT| does the same but to |STDOUT|, and is thus essentially |printf|.

@c
#define WRITE(args...) Streams::printf(OUT, args)

#define PRINT(args...) Streams::printf(STDOUT, args)

#define WRITE_TO(stream, args...) Streams::printf(stream, args)

#define LOG(args...) Streams::printf(DL, args)

#define LOGIF(aspect, args...) { \
	if (Log::aspect_switched_on(aspect##_DA)) Streams::printf(DL, args); \
}

@ Though we are trying to minimise use of C strings (which hold only narrow
characters and have fixed, unchecked sizes), we will still sometimes need what
amounts to |sprintf|. We replace this with a variadic macro which does bounds
checking around a regular unchecked call. Note, though, that we make use of
the |snprintf| function added in ISO C99. This never writes outside the bounds
of the string, and always null-terminates it. In the event of an overflow, it
is supposed to return the length which would have been required (a number
larger than |MAX_STRING_LENGTH|), but on some platforms it incorrectly returns
-1 instead. This is why both such values will fail |CStrings::check_len|.

@c
#define CSTRING_WRITE(to, args...) \
	CStrings::check_len((int) snprintf(to, MAX_STRING_LENGTH, args))

@p The beginning and the end.
As noted above, the client needs to call these when starting up and when
shutting down.

@c
void Inlib::start(void) {
	Memory::start();

	@<Register the default stream writers@>;
	@<Register the default debugging log aspects@>;
	@<Register the default debugging log writers@>;
	@<Register the default command line switches@>;
}

@ After calling |Inlib::start()|, the client can register further stream
writing routines, following these models: they define the meaning of escape
characters in |WRITE|, our version of formatted printing. |%f|, for example,
prints a filename by calling |Filenames::write|.

@<Register the default stream writers@> =
	Streams::register_writer('f', &Filenames::write);
	Streams::register_writer('p', &Pathnames::write);
	Streams::register_writer('w', &Streams::write_wide_C_string);
	Streams::register_writer('S', &Streams::write);

@ We provide a full logging service, in which different "aspects" can be
switched on or off. Each aspect represents an activity of the program about
which a narrative is printed, or not printed, to the debugging log file.
The following are always provided, but are all off by default.

@<Register the default debugging log aspects@> =
	Log::declare_aspect(DEBUGGING_LOG_INCLUSIONS_DA, L"debugging log inclusions", FALSE, FALSE);
	Log::declare_aspect(SHELL_USAGE_DA, L"shell usage", FALSE, FALSE);
	Log::declare_aspect(MEMORY_USAGE_DA, L"memory usage", FALSE, FALSE);
	Log::declare_aspect(TEXT_FILES_DA, L"text files", FALSE, FALSE);

@ Debugging log writers are similar to stream writers, but implement the |$|
escapes only available to the debugging log. For example, |$S| calls the
|Streams::log| function to print a textual representation of the current
state of a stream.

@<Register the default debugging log writers@> =
	Streams::register_log_writer('S', &Streams::log);

@ We provide an optional service for parsing the command line. By default,
the |-log A| switch makes that aspect active, though it's hyphenated, so
for example |-log memory-usage| or |-log no-memory-usage|. The |-platform|
switch is now probably redundant but can be used to identify which platform
we're running on. |-crash| tells the tool to crash on a fatal error, rather
than to exit cleanly, to make it easier to diagnose in a debugger.

@<Register the default command line switches@> =	
	CommandLine::declare_switch(LOG_CLSW, L"log", 2);
	CommandLine::declare_switch(PLATFORM_CLSW, L"platform", 2);
	CommandLine::declare_switch(CRASH_CLSW, L"crash", 1);

@ Once the following has been called, it is not safe to use any of the
inlib facilities. It should be called on any normal exit, but not on
an early termination due to a fatal error, as this may lead to thread
safety problems.

@c
void Inlib::end(void) {
	if (Log::aspect_switched_on(MEMORY_USAGE_DA)) Memory::log_statistics();
	Log::close();
	Memory::free();
}
