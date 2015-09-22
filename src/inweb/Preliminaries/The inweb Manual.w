[Manual::] The inweb Manual.

@Purpose: A manual for inweb, a simple literate-programming tool used by
the Inform project.

@p Introduction.
Inweb is a command line tool for literate programming. This is a doctrine
under which a program is written as a "web", a hybrid of code and commentary,
and is structurally subdivided in a form of narrative. A "web" can in principle
be written in any programming language, but it must be marked up in special
ways for Inweb to read it. Inweb is designed particularly for use with
C-like languages, but is easily extensible to others.

On a typical run, Inweb is asked to perform a given operation on an
existing web. It has four fundamental modes:

(w) weaving -- compiling the many sections into a human-readable form, such
as a web page or a PDF document, and then, optionally, displaying it;
(t) tangling -- compiling the many sections into a machine-readable form,
a single file of source code ready for a compiler or an interpreter;
(a) analysing -- displaying details about the web;
(c) creating -- making a new web.

@p Getting started.
Inweb is itself supplied as a web. The easiest way to install and use it is
to create a folder for webs, and to to place Inweb inside this; then work
in a terminal window with the webs folder as the current working directory.
Suppose we have two webs, like so:

	|webs|
	|    cBlorb|
	|    inweb|

The actual program inside a web, ready for a compiler or an interpreter, is
its "tangled" form. In the case of Inweb this used to be a "Perl script",
which could be run using a Perl interpreter, but Inweb is now written in C.
The tangled form of Inweb is supplied in the download:

	|    inweb|
	|        Tangled|
	|            inweb.c|

That file, |inweb.c|, doesn't actually do anything by itself: it needs to be
compiled through a regular C compiler first. For example, the following
would compile it through |gcc|; users of Mac OS 10.7 or later may want to
do the same but with |clang| replacing |gcc|. It may be that your downloaded
copy of Inweb already contains the compiled code in place, of course, in
which case all of this is unnecessary.

	|webs$ gcc -c -std=c99 -g -o inweb/Tangled/inweb.o inweb/Tangled/inweb.c|
	|webs$ gcc -g -o inweb/Tangled/inweb inweb/Tangled/inweb.o|

We'll assume that the compiled Inweb is now available. Users of the |bash|
shell may want to

	|alias inweb='inweb/Tangled/inweb'|

to save a little typing, but in the example commands below we'll always spell
it out.

@ As a first try-out, we'll use Inweb to tangle the web called |cBlorb|.
This is downloadable from the Inform website and is quite a large program,
so it's not a toy example; the statistics printed below, and the Inweb
version number, may be slightly different if you try this yourself.

	|webs$ ls|
	|cBlorb   inweb|
	|webs$ inweb/Tangled/inweb -tangle cBlorb|
	|inweb 6/110717|
	|web "cBlorb": 4 chapter(s) : 13 section(s) : 268 paragraph(s) : 5268 line(s)|
	|  tangling <cBlorb/Tangled/cBlorb.c> (written in C)|
	|webs$|

This works through the |cBlorb| web and generates its own tangled form -- 
|cBlorb/Tangled/cBlorb.c|, which is now a conventional C program and can
be compiled with |gcc| or similar, and then run. Alternatively:

	|webs$ inweb/Tangled/inweb cBlorb -weave|
	|inweb 6/110717|
	|web "cBlorb": 4 chapter(s) : 13 section(s) : 268 paragraph(s) : 5268 line(s)|
	|[Complete Program: 111pp 408K]|
	|webs$|

This generates |TeX| source for the human-readable form of |cBlorb| and runs
it through a variant of |TeX| called |pdftex|, and then tries to open this.
Of course this can only happen if |pdftex| is installed; in practice, you may
need to alter the simple configuration file stored at:

	|inweb/Materials/inweb-configuration.txt|

in order to tell Inweb how to invoke |TeX|. (For example, you could tell it
to use a variant like |XeTeX| instead.)

The woven web is a PDF file, then, stored at

	|inweb/Woven/Complete.pdf|

Inweb tries to open this on-screen automatically, on the grounds that you
probably want to see it, but there's no standard cross-platform way to do this;
again, you may need to tweak the configuration file.

We don't have to weave the entire web in one piece -- often we're interested
in just one section or chapter. For instance,

	|webs$ inweb/Tangled/inweb cBlorb -weave 2|
	|inweb 6/110717|
	|web "cBlorb": 4 chapter(s) : 13 section(s) : 268 paragraph(s) : 5268 line(s)|
	|[Chapter 2: 11pp 117K]|
	|webs$|

This time we specified "2", meaning "Chapter 2", as what to weave. We now
have:

	|webs$ ls cBlorb/Woven|
	|Chapter-2.pdf   Complete.pdf|

The most dramatic action we can take is to set off a "swarm" of weaves --
one for every section, one for every chapter, one for the entire source, and
all accompanied by an indexing website, ready to be uploaded to a web server.

	|webs$ inweb/Tangled/inweb cBlorb -weave sections|
	|inweb 6/110717|
	|web "cBlorb": 4 chapter(s) : 13 section(s) : 268 paragraph(s) : 5268 line(s)|
	|[P/man: 8pp 94K]|
	|[1/main: 6pp 95K]|
	|[1/mem: 9pp 112K]|
	|[1/text: 6pp 81K]|
	|[1/blurb: 6pp 83K]|
	|[2/blorb: 10pp 109K]|
	|[3/rel: 8pp 98K]|
	|[3/sol: 10pp 105K]|
	|[3/links: 3pp 73K]|
	|[3/place: 3pp 74K]|
	|[3/templ: 3pp 69K]|
	|[3/web: 21pp 154K]|
	|[Preliminaries: 9pp 102K]|
	|[Chapter 1: 28pp 177K]|
	|[Chapter 2: 11pp 117K]|
	|[Chapter 3: 49pp 233K]|
	|[Complete Program: 94pp 358K]|
	|Weaving index file: Woven/index.html|
	|Copying additional index file: Woven/download.gif|
	|Copying additional index file: Woven/lemons.jpg|

@ Suppose we now take a look inside the |cBlorb| web. We find:

	|webs$ ls cBlorb|
	|Chapter 1  Chapter 2   Chapter 3   Contents.w  Materials   Preliminaries   Tangled    Woven|

This is typical for a medium-sized web -- one large enough to be worth dividing
into chapters.

(a) We have already seen the |Woven| and |Tangled| folders. Every web contains
these, and they hold the results of weaving and tangling, respectively. They
are always such that the entire contents can be thrown away without loss, since
they can always be generated again.

(b) The |Materials| folder is optional, and contains auxiliary files needed
when the program expressed by the web is run -- in the case of |cBlorb|, it
contains a configuration file.

(c) There is also an optional |Figures| folder for any images included in
the text, but |cBlorb| has none.

(d) The program itself is cut up into "sections", each being a file with
the extension ".w" (for "web").
(-1) One section, |Contents.w|, is special -- it must be present, it must be
called that, and it must be in the main web folder.
(-2) All other sections (and there must be at least one other) are filed
either in chapter folders -- here, "Preliminaries", "Chapter 1", "Chapter 2"
and "Chapter 3" -- or else, for a smaller (unchaptered) web, are in a
single folder called "Sections".

In other literate programming tools, notably Knuth's |WEB| and |CWEB|, the
term "section" is used for what is probably one or two paragraphs of English
prose followed by a single code excerpt. In Inweb terms, that's a
"paragraph".

Inweb is intended for medium-sized programs which will likely have dozens
of these sections (the main |inform7| web has about 260). A section is a
structural block, typically containing 500 to 1000 lines of material, which
has its own name. An ideal section file makes a standalone essay,
describing and implementing a single well-defined component of the whole
program.

@p A minimal Hello web.
Many software tools scale up badly, in that they work progressively less
well on larger tasks: Inweb scales down badly. So the "Hello" project
we'll make here looks very cumbersome for so tiny a piece of code, but
it does work.

We first make the |Hello| web. We could make this by hand, but it's easier
to ask Inweb itself to do so:

	|webs$ inweb/Tangled/inweb -create Hello|
	|inweb 6/110717|
	|Hello|
	|Hello/Figures|
	|Hello/Materials|
	|Hello/Sections|
	|Hello/Tangled|
	|Hello/Woven|
	|inweb/Materials/Contents.w -> Hello/Contents.w|
	|inweb/Materials/Main.w -> Hello/Sections/Main.w|

The output here shows Inweb creating a little nest of folders, and then
copying two files into them; it's actually text printed by the standard
Unix commands |mkdir| and |cp| in verbose mode, so the appearance may be
different on different platforms. But the effect should be the same.
The |Hello| web now exists, and works -- it can be tangled or woven, and
is a "Hello world" program written in C.

@ Uniquely, the "Contents.w" section provides neither typeset output nor
compiled code: it is instead a roster telling Inweb about the rest of the
web, and how the other sections are organised. It has a completely different
syntax from all other sections.

The contents section for |Hello| might be created like so:

	|Title: New|
	|Author: Anonymous|
	|Purpose: A newly created program.|
	|Language: C|
	|Licence: This program is unpublished.|
	|Build Number: 1|
	||
	|Sections|
	|	Main|

This opens with a block of name-value pairs specifying some bibliographic
details; there is then a skipped line, and the roster of sections begins.

So, we have to fill in the details. Note that the program's Title is not the
same as the folder-name for the web, which is useful if the web contains
multiple programs (see below) or if it has a long or file-system-unfriendly
name. The Purpose should be brief enough to fit onto one line. Licence can
also have the US spelling, License; Inweb treats these as equivalent.
The Build Number can have any format we like, and is optional. There are
other optional values here, too: see below.

The Language is the programming language in which the code is written. At
present Inweb supports:

	|C   C++   C for Inform   Perl   Inform 6   Inform 7   Plain Text|

Perhaps "supports" ought to be in quotation marks, because Inweb doesn't
need to know much about the underlying programming language. It would be
easy to add others; this selection just happens to be the ones we need for
the Inform project. ("C for Inform" is an idiosyncratic extension of the
C programming language used by the core Inform software; nobody else will
ever need it.)

@ After the header block of details, then, we have the roster of sections.
This is like a contents page -- the order is the order in which the sections
are presented on any website, or in any of the larger PDFs woven. For a short,
unchaptered web, we might have for instance:

	|Sections|
	|	Program Control|
	|	Command Line and Configuration|
	|	Scan Documentation|
	|	HTML and Javascript|
	|	Renderer|

And then Inweb will expect to find, for instance, the section file ``Scan
Documentation.w'' in the "Sections" folder.

A chaptered web, however, won't have a "Sections" folder. It will have a much
longer roster, such as:

	|Preliminaries|
	|	Preface|
	|	Thematic Index|
	|	Licence and Copyright Declaration|
	|	Literate Programming|
	|	BNF Grammar|
	||
	|Chapter 1: Definitions|
	|"In which some globally-used constants are defined and the standard C libraries|
	|are interfaced with, with all the differences between platforms (Mac OS X,|
	|Windows, Linux, Solaris, Sugar/XO and so forth) taken care of once and for all."|
	|	Basic Definitions|
	|	Platform-Specific Definitions|
	||
	|Chapter 2: Memory, Files, Problems and Logs|
	|"In which are low-level services for memory allocation and deallocation,|
	|file input/output, HTML and JavaScript generation, the issuing of Problem|
	|messages, and the debugging log file."|
	|	Memory|
	|	Streams|
	|	Filenames|

... and so on...

	|Appendix A: The Standard Rules (Independent Inform 7)|
	|"This is the body of Inform 7 source text automatically included with every|
	|project run through the NI compiler, and which defines most of what end users|
	|see as the Inform language."|
	|	SR0 - Preamble|
	|	SR1 - Physical World Model|
	|	SR2 - Variables and Rulebooks|

Here the sections appear in folders called Preliminaries, Chapter 1, Chapter 2,
..., Appendix A. (These are the only possibilities: Inweb doesn't allow other
forms of name for blocks of sections. There can't be a Chapter 0, though there
can be Appendix B, C, ..., O.)

In case of any doubt we can use the following command-line switch to see
how Inweb is actually reading its sections in:

	|inweb/Tangled/inweb -catalogue -verbose|

@ If we look at the single, minimal "Main.w" section in the "Hello" web
created above, we find something very rudimentary:

	|S/main: Main.|
	| |
	|@Purpose: This is the entire program.|
	| |
	|@-------------------------------------------------------------------------------|
	| |
	|@ Hello, reading world!|
	| |
	|@c|
	|int main(int argc, char *argv[]) {|
	|    printf("Hello, computing world!\n");|
	|    return 0;|
	|}|

This layout will be explained further below, but briefly: each section other
than the Contents opens with a titling line, then a pithy statement of its
purpose. It then (optionally) contains some definitions and other general
discussion up as far as "the bar", followed by code and more discussion
below it. Broadly speaking, the content above the bar is what you need to
know to use the material in the section; the content below the bar is how
it actually works. (The distinction is a little like that between the
|.h| "header files" used by C programmers, and the |.c| files of the
code these describe.)

As can be seen, the |@| escape character is very significant to Inweb. In
particular, in the first column it marks a structural junction in the
file -- the Purpose and the bar are examples of this, and so is the single
"paragraph" of code below the bar, which begins with the solitary |@|
before the word "Hello".

A paragraph has three parts, each optional, but always in the following
sequence: some textual commentary (here "Hello..."), some definitions
(here there are none), then some code after an |@c| marker (here, the C
function |main(argc, argc)|).

A section need not contain any code at all. In fact, this manual is itself
an Inweb section, in which every paragraph contains only commentary.

@p Chapter and section names.
There is in principle no limit to the number of sections in an unchaptered
web. But once there are more than nine or ten, it is usually a good idea to
group them into higher-level blocks. Inweb calls these blocks
"chapters", though they aren't always named that way. The possibilities
are as follows:

(a) "Preliminaries". Like the preliminary pages of a book -- preface,
licence details perhaps, some expository material about the method. The
actual code ought to begin in Chapter 1 (though |indoc| doesn't require
that).

(b) "Chapter 1", "Chapter 2", and so on.

(c) "Appendix A", "Appendix B", ... and so on up to "Appendix O" but
no further.

Each chapter has a one-character abbreviation: P, 1, 2, ..., A, B, ...; thus

	|inweb/Tangled/inweb cBlorb -weave B|

weaves a PDF of Appendix B alone. In general, it doesn't make sense to tangle
a single chapter alone, because they are all parts of one large program, but
there's a way to specify that certain chapters contain independent material --
this allows for the web to hold one big C program (say) and Appendix A a
text file of settings vital for its running; and in that case it would indeed
make sense to tangle just Appendix A, generating the text file.

@ A section name must contain only filename-safe characters, and it's probably
wise to make them filename-safe on all platforms: so don't include either
kind of slash, or a colon, and in general go easy on punctuation marks.

To be as descriptive as possible, section names need to be somewhat like
chapter titles in books, and this means they tend to be inconveniently
long for tabulation, or for references in small type.

Each section therefore also has an abbreviated name, which is quoted in
its titling line. This should always have the form of a chapter number,
followed by a slash, followed by a short (usually two to five character)
alphanumeric name. For instance, a section of Chapter 2 called ``Problems,
Level 3'' might have abbreviated name |2/prob3|.

In a web with no chapters, the chapter part should be S, for section.
(Hence |S/hello| above.) Sections in the Preliminaries chapter, if there
is one, are prefaced P; sections in Appendix A, B, ..., are prefaced
with the relevant letter.

Within each section, paragraphs are numbered, which gives a concise notation
identifying any paragraph in the whole program.

@p Weaving.
The weaver produces a typeset version of all or part of the web, or possibly
an index to it. In general, we activate the weaver by running Inweb like so:

	|inweb/Tangled/inweb cBlorb -weave W|

where |W| is the "target". This is usually one of the following:

(1) |all|: the whole thing;
(2) a chapter number 1, 2, 3, ..., or an appendix letter A, B, ...;
(3) the letter P, meaning the collected Preliminaries;
(4) the abbreviated name for a section, such as |2/prob3|.

The default target is |all|.

@ In addition, three special targets run the weaver in "swarm mode", which
could perhaps be more happily named. This automates a mass of individual
weaving tasks to generate multiple PDFs suitable for offering as downloads
from a website. (It's more efficient than simply batch-processing uses
of Inweb from the shell, since the source only needs to be scanned once.
It's also a lot less trouble.) These targets are, in ascending order of
size:

(5) |index|: create a web page from a template which gives a tidy download
directory of the chapters and sections (see below);
(6) |chapters|: weave a single PDF for each chapter block (Preliminaries,
if present, all numbered chapters present, all lettered appendices present),
then also |index|;
(7) |sections|: weave a single PDF for each section (other than the contents),
then also weave |chapters| and |index|.

Note that it's also possible to restrict the weaver to the sections in a given
chapter or Appendix only, like so:

	|inweb/Tangled/inweb inform7 -weave A sections|

weaves the sections of Appendix A only, and makes up the index page to show
only those. (This is useful for publishing only part of a web.)

@ The weaving process consists of several steps:

(i) A suitable file of |TeX| source, with a name such as |Chapter-2.tex|,
|2-prob3.tex|, etc., is written out into the |Woven| subfolder of the web.
Spaces are removed from its filename since |TeX| typically has dire
problems with filenames including spaces.

(ii) This is then run through a |TeX|-to-PDF tool such as |pdftex| or |XeTeX|.
(The choice of and path to this can be altered in the Inweb configuration file.)
The console output is transcribed to a file rather than being echoed on
screen: Inweb instead prints a concise summary such as

	|[2/prob3: 12pp 99K]|

(...) meaning that a 12-page PDF file, 99K in size, has been generated, and that
there were no |TeX| errors -- because if there had been, the summary would
have said so.

(iii) The |TeX| source and log file are deleted. So is the console output
file showing what the |TeX| agent verbosely chattered to |stdout|, unless
there were any errors, in which case all three are preserved for the user
to investigate.

(iv) If the Inweb configuration file supplies a command for opening PDFs
in the local operating system's PDF viewer, then Inweb now uses it by
default, but this can be explicitly switched on with the |-open| switch or
off with the |-closed| one. The configuration file supplied in the standard
Inweb distribution simply uses a command called |open|, which in Mac OS X
duplicates the effect of double-clicking the file in the Finder, so that it
opens in the user's preferred PDF viewer (Preview, say).

@p Cover sheets in TeX.
The really large PDFs, for chapters and for the whole thing, have a cover
sheet attached. The standard design for this is pretty dull, but it can be
overridden by spelling out how to weave a booklet in the "Contents.w" section:

	|Weave: all: Complete Program, Complete, cover-sheet.tex|

This tells Inweb how to weave the book(let) for a given theme -- here,
it's |all|, meaning the entire web -- by giving the title, the leafname,
and the filename for the cover sheet. This must be present in the "Materials"
folder for the web, of |TeX| source for the cover sheet. (It already has all
of the Inweb macros loaded, so needn't |\input| anything, and it should
not |\end|.) Another example might be:

	|Weave: Interface: Interfacing with Core Inform, Interface, cover-sheet.tex|

Within the cover sheet copy, doubled square brackets can be used to insert
any of the values in the "Contents.w" section -- for instance,

	|\noindent{{\sinchhigh\noindent [[Build Number]]}}|

In addition:
(a) |[[Cover Sheet]]| expands to the default cover sheet -- this is convenient
if all you want to do is to add a note at the bottom of the standard look.
(b) |[[Booklet Title]]| expands to text such as "Chapter 3", appropriate
to the weave being made.
(c) |[[Capitalized Title]]| is a form of the title in block capital letters.
(d) |[[Build Date]]| is the last modification date.

@p Indexing.
The weaver's |index| target, produced either as a stand-alone or to
accompany a swarm of chapters or sections, is generated using a template
file. In fact, this can almost any kind of report, or even a multiplicity
of reports: the "Contents.w" section can specify one or more templates,
like so --

	|Index Template: index.html, sizes.txt|

If no such setting is made, Inweb will by default use its own standard
template, which is a single HTML index page.

The "Contents.w" can also specify a list of binary files:

	|Index Extras: corporate-logo.gif|

These are copied verbatim from the web's Materials folder into its Woven one.
(When Inweb is making its default web page, it copies two such images --
a botanical painting of some lemons which is for some reason the Inweb
banner, and a download icon.) Of course the effect is the same as if we
always kept these files in Woven, but we don't want to do that because we
want to preserve the rule that Woven contains no master copies -- the contents
can always be thrown away without loss.

@ Each index is made by taking the named template file and running it through
the "template interpreter" to generate a file of the same name in the
|Woven| folder. The template interpreter is basically a filter: that is, it
works through one line at a time, and most of the time it simply copies
the input to the output. The filtering consists of making the following
replacements. Any text in the form |[[...]]| is substituted with the
value |...|, which can be any of:

(a) A bibliographic variable, set at the top of the |Contents.w| section.

(b) One of the following details about the entire-web PDF (see below):

	|[[Complete Leafname]]  [[Complete Extent]]  [[Complete PDF Size]]|

(b) One of the following details about the "current chapter" (again, see below):

	|[[Chapter Title]]  [[Chapter Purpose]]  [[Chapter Leafname]]|
	|[[Chapter Extent]]  [[Chapter PDF Size]]  [[Chapter Errors]]|

(...) The leafname is that of the typeset PDF; the extent is a page count;
the errors result is a usually blank report.

(c) One of the following details about the "current section" (again, see below):

	|[[Section Title]]  [[Section Purpose]]  [[Section Leafname]]|
	|[[Section Extent]]  [[Section PDF Size]]  [[Section Errors]]|
	|[[Section Lines]]  [[Section Paragraphs]]  [[Section Mean]]|
	|[[Section Source]]|

(...) Lines and Paragraphs are counts of the number of each; the Source
substitution is the leafname of the original |.w| file. The Mean is the
average number of lines per paragraph: where this is large, the section
is rather raw and literate programming is not being used to the full.

@ But the template interpreter isn't merely "editing the stream", because
it can also handle repetitions. The following commands must occupy entire
lines:

|[[Repeat Chapter]]| and |[[Repeat Section]]| begin blocks of lines which
are repeated for each chapter or section: the material to be repeated
continues to the matching |[[End Repeat]| line. The ``current chapter or
section'' mentioned above is the one selected in the current innermost
loop of that description.

|[[Select ...]]| and |[[End Select]| form a block which behaves like 
a repetition, but happens just once, for the named chapter or section.

For example, the following pattern:

	|To take chapter 3 as an example, for instance, we find:|
	|[[Select 3]]|
	|[[Repeat Section]]|
	|    Section [[Section Title]]: [[Section Code]]: [[Section Lines]] lines.|
	|[[End Repeat]]|
	|[[End Select]]|

weaves a report somewhat like this:

	|To take chapter 3 as an example, for instance, we find:|
	|    Section Lexer: 3/lex: 1011 lines.|
	|    Section Read Source Text: 3/read: 394 lines.|
	|    Section Lexical Writing Back: 3/lwb: 376 lines.|
	|    Section Lexical Services: 3/lexs: 606 lines.|
	|    Section Vocabulary: 3/vocab: 338 lines.|
	|    Section Built-In Words: 3/words: 1207 lines.|

@p Tangling.
This is more simply described. For almost all webs, there is only one possible
way to tangle:

	|inweb/Tangled/inweb cBlorb -tangle|

However, if we want the tangled result to have a different name or destination
from the normal one, we can write:

	|inweb/Tangled/inweb cBlorb -tangle-to ../stuff/etc/cblorb.c|

Exactly what happens during a tangle will be described later, when we get
to details on the syntax of section files. Basically, it makes the entire
program as a single source file with the commentary removed, and (for
C-like languages, anyway) restrings it all into a convenient order: for
instance all C functions are predeclared, structure definitions are made in
an order such that if A contains B as an element then B is declared before
A regardless of where they occur in the source text, all above-the-bar
definition material is available from every section, and so forth.

@ As trailed above, it is legal in some circumstances to tangle only
part of a web. The command syntax is just like that for weaving:

	|inweb/Tangled/inweb inform7 -tangle A|
	|inweb/Tangled/inweb inform7 -tangle B/light|

However, this is only allowed if the chapter or section involved was
listed in the "Contents.w" roster as being Independent. This explains why
we had:

	|Appendix A: The Standard Rules (Independent Inform 7)|
	|"This is the body of Inform 7 source text automatically included with every|
	|project run through the NI compiler, and which defines most of what end users|
	|see as the Inform language."|
	|	SR0 - Preamble|
	|	SR1 - Physical World Model|

The bracketed "(Independent)" marks out Appendix A as a different tangle
target to the rest of the web. In this case, we've also marked it out as having
a different language -- the rest is a C program, but this is an Inform 7 one.

Similarly, we can mark a section as independent:

	|	Light Template (Independent Inform 6)|

Independent chapters and sections are missed out when tangling the main part
of the web, of course.

@p Analysing.
There's not much to see here, but sometimes it's useful to see a contents
listing:

|-catalogue|

(...) prints an annotated table of contents of the web source, listing each
section and its C structures, together with a note of any other sections
sharing these data structures.

|-functions|

(...) prints a much longer catalogue, including names of all functions defined
in the sections.

@pp The web source code format.
To recap: the web is hierarchically organised on four levels -- chapter,
section, named paragraph, anonymous paragraph. (We haven't seen named
paragraphs yet, but they are about to appear.) Each |.w| file corresponds
to a single section, except for the "Contents.w" section, which is special
and to which the following does not apply.

@p Below the bar.
Broadly speaking, material below the bar is the program itself: the routines
of code, intermingled with commentary. This is a sequence of paragraphs.

Each paragraph is introduced by an "at" symbol |@| in column 1. (Outside
of column 1, an |@| is a literal at symbol, except for the |@< ... @>|
notation -- for which see below.) Some paragraphs are named, while others
are anonymous. Here are the three varieties of paragraph break:

|@ This begins an anonymous paragraph, and runs straight into text...|

|@p Named Paragraph. The text after the symbol, up to the full stop, is the title.|

|@pp| produces a named paragraph but also forces a page-break before it,
so that it will start at the top of a fresh page.

@ The content of a paragraph is divided into comment, definitions and code,
always occurring in that order. Definitions and code are each optional,
but there is always comment -- even if it is sometimes just a space where
comment could have been written but wasn't.

Text following a |@| marker, or following the end of the title of a |@p|
marker, is taken as the comment. Comment material is ignored by the
tangler, and contributes nothing to the final compiled program.

Definitions are indicated by an |@d| escape which begins a line. A
definition can be read almost exactly as if it were a |#define|
preprocessor macro for C, but there are two differences: firstly, the
tangler automatically gathers up all definitions and moves them (in order)
to the start of the C code, so there is no need for a definition to be made
earlier in the web source than it is used; and secondly, a definition
automatically continues to the next |@| escape without any need for
continuation backslashes. This means that long, multi-line macros can be
written much as ordinary code.

Code begins with a |@c| escape which begins a line. There can be at most
one of these in any given paragraph. From that escape to the end of the
paragraph, the content is literal C code.

The following example makes a long macro definition as a template, just to
demonstrate the point about multi-line definitions:

	|@p Example Paragraph. Here I could write a whole essay, or nothing much.|
	|@d MAX_BANANA_SHIPMENT 100|
	|@d EAT_FRUIT(variety)|
	|    int consume_by_##variety(variety *frp) {|
	|        return frp->eat_by_date;|
	|    }|
	|@c|
	|banana my_banana; /* initialised somewhere else, let's suppose */|
	|EAT_FRUIT(banana) /* expands with the definition above */|
	|void consider_fruit(void) {|
	|    printf("The banana has an eat-by date of %d.", consume_by_banana(&my_banana));|
	|}|

Note that the C code can contain comments, just as any C program can. These
are woven just like commentary text at the start of paragraphs, except that
they appear in italic type.

@p Folding code.
The single most important feature of Inweb, and other literate programming
systems, is the ability to fold up pieces of code into what look like lines
of pseudocode. For example,

	|@ So, this is a program to see if even numbers from 4 to 100 can all|
	|be written as a sum of two primes. Christian Goldbach asked Euler in 1742|
	|if every even number can be written this way, and we still don't know.|
	||
	|@c|
	|int main(int argc, char *argv[]) {|
	|    int i;|
	|    for (i=4; i<100; i=i+2) {|
	|        printf("%d =", i);|
	|        @<Solve Goldbach's conjecture for i@>;|
	|        printf("\n");|
	|    }|
	|}|

Here, the interesting part of the code has been abstracted into a named
paragraph. The definition could then follow:

	|@ We'll print each different pair of primes adding up to i. We|
	|only check in the range 2 <= j <= i+2 to avoid counting pairs|
	|twice over (thus 8 = 3+5 = 5+3, but that's hardly two different ways).|
	||
	|@<Solve Goldbach's conjecture for i@> =|
	|    int j;|
	|    for (j=2; j<=i/2; j++)|
	|        if ((isprime(j)) && (isprime(i-j)))|
	|            printf(" %d+%d", j, i-j);|

What did we gain by this? Really the point was to simplify the presentation
by presenting the code in layers. There was an outer layer doing routine
book-keeping, and an inner part which had to understand some basic number
theory, and each layer is easier to understand without having to look at the
other one.

Of course, that kind of code abstraction is also what a function call does.
But there are several differences: (a) we don't have to pass arguments to it,
because it isn't another function -- it's within this function and has
access to all its variables; (b) we aren't restricted to a C identifier,
so we can write a more natural description of what it does.

@p The bar.
The section is bisected by a horizontal bar:

|@-------------------------------------------------------------------------------|

which acts as a dividing line. (A vertical bar would be more of a challenge
for coders and editors, but there we are.) In fact this can have any width
from four hyphens upwards, so the smallest legal bar would be: |@----|.

@p Above the bar.
Broadly speaking, material above the bar identifies the section and how it
relates to other sections in the web. No functions should be declared here:
it should be structures, definitions, global variables and arrays,
commentary.

The titling line at the top of the section has already been described: it
has the form

	|abbrev: Unabbreviated Name.|

and it is always followed by a blank line, and then by:

	|@Purpose: ...|

This should be a brief summary of what the code in this section is for.
For instance, ``To manage initial and current values of named values, which may
be either constants or variables, but which have global scope.''

@ When the web is for a C-like language, we may want to include an interface
declaration next --

	|@Interface:|

If present, this is followed by a sequence of lines explicitly declaring
what data structures it owns. For example:

	|-- Owns struct quantity (private)|

The ownership declaration indicates that this section will |typedef| a
C structure called |quantity|, and that it is private: Inweb will not
permit code in any other section to access its data. Structures need not
be private; for instance:

	|-- Owns struct phrase (public)|
	|   !- shared with Chapter 7/Data Type Checking.w|
	|   !- shared with Chapter 7/Type Checking.w|

By default, Inweb does not require each section to have a correct Interface
declaration -- for most small webs, it's too much trouble for no particular
gain. But if the "Contents.w" section includes the setting --

	|Strict Usage Rules: On|

then Inweb will indeed complain if the interface is wrongly declared.

Thus, unless the Contents section goes out if its way to ask for this,
there's no need ever to declare the Interface.

@ The last body of material above the bar, which is yet again optional,
is written:

	|@Definitions:|

Beneath this heading, if it is present, is a sequence of 1 or more paragraphs
exactly like those below the bar, except that no functions should be defined.

This is a good place to make global variable, array, constant, or type
definitions. The tangler automatically moves these to the start of the code
and ensures that everything works regardless of ordering. Note that no C
function predeclarations are ever needed: again, the tangler makes those
itself automatically. Functions can therefore be declared and used in any
order.

@p Tagging paragraphs.
Two theme tags are provided by Inweb itself:

(a) "Structures", a tag automatically applied in C-like languages to any
paragraph containing a typedef for a struct;

(b) "Figures", a tag automatically applied to any paragraph which displays
a figure.

@p Markup of commentary.
Comment material in paragraphs is set more or less as it reads, but with a
number of niceties. A doubled hyphen becomes an em-rule; double-quotation
marks may automatically smarten.

Lines beginning with what look like bracketed list numbers or letters are
set as such, running on into little indented paragraphs. Thus

	|(a) Intellectual property has the shelf life of a banana. (Bill Gates)|
	|(b) He is the very pineapple of politeness! (Richard Brinsley Sheridan)|
	|(c) Harvard takes perfectly good plums as students, and turns them into|
	|prunes. (Frank Lloyd Wright)|

will be typeset thus:

(a) Intellectual property has the shelf life of a banana. (Bill Gates)
(b) He is the very pineapple of politeness! (Richard Brinsley Sheridan)
(c) Harvard takes perfectly good plums as students, and turns them into
prunes. (Frank Lloyd Wright)

A line which begins |(...)| will be treated as a continuation of indented
matter (following on from some break-off such as a source quotation).
A line which begins |(-X)| will be treated as if it were |(X)|, but
indented one tab stop further in, like so:

(c) ...
(-d) Pick a song and sing a yellow nectarine. (Scott Weiland)

If a series of lines is indented with tab characters and consists of
courier-type code extracts, it will be set as a running-on series of
code lines.

A line written thus:

	|>> The monkey carries the blue scarf.|

is typeset as an extract of text thus:

>> The monkey carries the blue scarf.

@ Pictures must be in PNG or PDF format and can be included with lines like:

	|[[Figure: Fig_0_1.pdf]]|
	|[[Figure: 10cm: Fig_0_2.png]]|

In the second example, we constrain the width of the image to be exactly that
given: it is scaled accordingly.

The weaver expects that any pictures needed will be stored in a subfolder of
the web called |Figures|: for instance, the weaver would seek |Fig_2_3.pdf| at
pathname |Figures/Fig_2_3.pdf|.

@pp The "C for Inform" language.
This is a small extension of C, provided at the tangling stage within Inweb,
for the |inform7| web only: no other project can usefully employ it, so the
notes here will be brief.

Here the code to be tangled is ANSI standard C, but with the following
additional syntaxes, which are expanded into valid C by Inweb. Note that
although I7 makes heavy use of memory allocation and linked list navigation
macros, |CREATE| and |LOOP_OVER|, they and similar constructions are
defined as ordinary C preprocessor macros and are documented where they are
defined. They are not formally part of the Inweb language.

There are two language extensions:

(a) Doubled colons are allowed as namespace dividers in the names of functions:
for example, |Fruit::peel|. This form of naming must be used if and only if
the function is called from another section, and the initial namespace must
match the one declared at the top of the section.

(b) Preform syntax can be written directly into the C code for the compiler,
and this is automatically tangled out in two forms: to the Preform definitions
in a syntax file, and to the C code which deals with matches against them.
The paragraph tag "Preform" also has some quirks. This isn't a good place
to document all of that.
