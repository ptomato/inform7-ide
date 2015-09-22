[Cweb::] Inweb vs Cweb.

@Purpose: Notes on how inweb relates to other literate programming tools,
notably CWEB.

@p Introduction.
Inweb is a command line tool for literate programming, a doctrine
invented by Donald Knuth in the early 1980s. Inweb stands in a genre
of LP tools ultimately all deriving from Knuth's |WEB|, and in particular
borrows from syntaxes used by |CWEB|, a collaboration between Knuth and
Sylvio Levy.

@ Two important differences between Inweb and its ancestor |CWEB| in how
they handle folded code:

(1) In Inweb, the code is tangled within braces. (Well, for C-like
languages which brace blocks of code, anyway.) This means that the scope
of the variable |j| defined above is just within the "Solve..." paragraph.
It also means that the following loops over the whole code in the paragraph,
as we might expect:

	|for (k=1; k<=5; k++) @<Do something really complex depending on k@>;|

(2) Inweb does {\it not} follow the disastrous rule of |CWEB| that every
name is equal to every other name of which it is an initial substring, so
that, say, "Finish" would be considered the same name as ``Finish with
error''. This was a rule Knuth adopted to save typing -- he habitually
wrote elephantine names, the classic being $\S 1000$ in the |TeX| source
code, which reads:

$\langle$ If the current page is empty and node $p$ is to be deleted,
{\bf goto} {\it done1}; otherwise use node $p$ to update the state of the
current page; if this node is an insertion, {\bf goto} {\it contribute};
otherwise if this node is not legal breakpoint, {\bf goto} {\it contribute}
or {\it update\_heights}; otherwise set {\it pi} to the penalty associated
with this breakpoint $\rangle$

With élan.

@p Why inweb was written.
``An unreliable programming language generating unreliable programs
constitutes a far greater risk to our environment and to our safety than
unsafe cars, toxic pesticides, or accidents at nuclear power stations.''
(Tony Hoare, quoted in Donald MacKenzie, {\it Mechanizing Proof} (MIT
Press, 2001)). The traditional response is to formalise such tools: to give
a mathematical description of input and output, and to prove that the
compiler does indeed transform one into the other. But the ideal of the
fully verified, fully verifying compiler remains a distant one, and it
seems altogether likely that when we do get it, it will be as difficult to
use as a mathematical theorem-prover. In any case, there are obvious
difficulties when the input syntax is natural language, and not easily
given formal expression.

My biggest concern in coding Inform has been to find a way to write it which
would give some confidence in its correctness, and to make it maintainable
by other people besides myself. Having little faith in "neat" AI approaches
to program correctness -- the ideal advanced by Tony Hoare's  Grand
Challenge project for a truly verifying compiler -- I turned to the
"scruffy" side. Knuth's literate programming dogma is a different kind of
program verification. The aim is to write a program which is as much an
argument for its own correctness as it is code. This is done not so much
with formalities -- preconditions and contracts \`a la Eiffel, or heavy
use of assertions -- as with something closer to a proof as it might appear
in a scientific journal. The text mixed in with code is aimed at a human
reader.

A key step in literate programming is publishing code, which is not the
same thing as making code available for download. It means tidying up and
properly explaining code, and is a process much like writing up
roughly-correct ideas for publication in journals -- the act of tidying up
a final article reveals many small holes: odd cases not thought of, steps
missed out. The process is like a systematic code review, but it's
more than that; it involves a certain pledge by the author that the code
is, to the best of his understanding, right.

@ When he wound up his short-lived {\it Literate Programming} column in
CACM (vol. 33, no. 3, 1990) -- where it had been the successor to the
legendary {\it Programming Pearls} column -- Christopher Wyk made the
perceptive criticism that ``no one has yet volunteered to write a
program using another's system for literate programming''. Nor have I, but
I did try, and perhaps it may be helpful to explain why Inweb was written.

When Inform 7 began to be coded, in 2003, it used |CWEB| -- a combination of
programs called |cweave| and |ctangle| by Sylvio Levy, after Knuth, and to
some extent the lineal descendant today of Knuth's original Pascal-based
|WEB|. A short manual for |CWEB|, available online, is also on sale in
printed form: the software itself can be downloaded from the Comprehensive
|TeX| Archive |CTAN|. I am going to say some unkind things about |CWEB| in
a moment, so let me first acknowledge the great benefit which I have
derived from it, and record my appreciation of Levy and Knuth's work.

|CWEB| seemed to me the most established and durable of the options
available, and the alternatives, such as |noweb| and |funnelweb|, did not
seem especially advantageous for a C program. I was also unsure that either
would be very well maintained or supported (which is ironic, given that the
author of |noweb| later became an Inform user, filing numerous helpful bug
report forms). Though it was arguably abandonware, |CWEB| seemed likely to
be reliable, given its heritage and continuing use by the |TeX| community.

|CWEB| is, it must be said, clearly out of sorts with modern practice. It
is an aggressively Knuthian tool, rooted in the computing paradigms of the
1970s, when nothing was WYSIWYG and the escape character was king. Hardly
anybody reads |CWEB| source fluently, even though it is a simple system
with a tiny manual. Development environments have never heard of it; it
won't syntax-colour properly. The holistic approach subverts the business
of linking independently compiled pieces of a large program together. So my
first experiments were carried out in a spirit of scepticism. The prospect
of making C more legible by interspersing it with |TeX| macros seemed a
remote one, and since both C and |TeX| already have quite enough escape
characters as it is, the arrival of yet another -- the |@| sign -- did not
gladden the heart. And yet. |@| signs began to appear like mushrooms in the
Inform source, and I found that I was indeed habitually documenting what I
was doing rather more than usual, and gathering conceptually similar
material together, and trying to structure the code around "stories" of
what was happening. Literate programming was working, in fact.

A lengthy process of fighting against hard limits and unwarranted
assumptions in |CWEB| followed, and by 2005 both |cweave| and |ctangle| had
had to be hacked to be viable on the growing Inform source. For one thing,
both contained unnecessary constraints on the size of the source code,
making them capable of compiling |TeX| and Metafont -- their primary task
-- but little more. Both tools use tricksy forms of bitmap storage making
these limits difficult to overcome. |ctangle| proved to be fairly robust,
if slow: its only defect as such was an off-by-one bug affecting large
webs, which I never did resolve, causing all line numbering to be out by
one line in |gdb| stack backtraces, internal error messages and the like.
But the problems of |cweave| went beyond minor inconvenience. It parses C
with a top-down grammar of productions in order to work out the ideal
layout of the code, totally ignoring the actual layout as one has typed it.
This "ideal" layout is usually worse than a simple syntax-colouring text
editor could manage, and often much worse.

More seriously, the productions for C used by |CWEB| do not properly cope
with macros, and as often happens with such grammars, when they go wrong
they go horribly wrong. A glitch somewhere in a function causes the entire
body of code to be misinterpreted as, I don't know, a variable. When that
glitch is not in fact a coding error but legal and indispensable code, the
results are horrible. For some months I rewrote and rewrote |cwebmac.tex|,
the typically incomprehensible suite of |TeX| macros supplied with |CWEB|,
but I was forced to go through stages of both pre- and post-processing in
order to get anything like what I wanted out of |cweave|. It is,
unfortunately, almost impossible to amend or customise |cweave|: it hangs
entirely on the parsing grammar for C, and this is presented as cryptic
productions which have clearly been mechanically generated from some
higher-level description which is {\it not} available. It is a considerable
irony that the |CWEB| source was meant to demonstrate its own virtues as
a presentation of openly legible code, yet as a program it now clearly fails
clause 2 of the Open Source Initiative's definition of "open source":

{\it The source code must be the preferred form in which a
programmer would modify the program. Deliberately obfuscated source code
is not allowed. Intermediate forms such as the output of a preprocessor
or translator are not allowed.}

And this is a real hazard with literate programming (and is the reason
Inweb provides features to present auxiliary files in multiple languages
within the same web). Disagree with Knuth and Levy about the ideal display
position of braces? Prefer the One True Brace Style, for instance? Want to
omit those little skips between variable declarations and lines of code?
Think cases in |switch| statements ought to be indented a little? Want
static arrays to be tabulated? Think you ought to be allowed to comment out
pieces of code? You are in for hours of misery.

And so for about a year, from autumn 2005 to autumn 2006, I tangled but I
did not weave. This was hardly a showcase for the literate programming ideal.

In December 2006, I finally conceded that |ctangle| was no longer adequate
either. Inform was pushing against further limits, harder still to raise, but
the real problem was that |ctangle| made it difficult to encapsulate code
in any kind of modular way. For instance, although |ctangle| would indeed
collate |@d| definitions (the equivalent of standard C's |#define|s)
from the whole web of source code and output them together in a preliminary
block at the start of the C to be compiled, it did not perform the same
service for structure declarations. Nor would it predeclare functions
automatically (something I found annoying enough that for some time, I
used a hacky Perl script to do for me). The result was that, for some
years, Chapter 1 of the Inform source code consisted of a gigantic string of
data structure definitions, with commentary attached: it amounted to
more than 100pp of close-type A4 when woven into a PDF, and became so
gargantuan that it seemed to me a counter-example to Fred Brooks's
line "give me the table structures, and I can see what the program does".

All in all, then, |CWEB|'s suitability and performance gradually
deteriorated as Inform grew. At first, |CWEB| made good on its essential
promise to produce an extensively documented program and an accompanying
book of its source code. I believe it really would be a good tool
for, say, presenting reference code for new algorithms. But a tipping point
was reached at about 50,000 lines of source where |CWEB|'s blindness to the
idea of modular, encapsulated code was actively hindering me from
organising the source better. (While |CWEB| has notionally been extended to
|C++|, it truly belongs to the prelapsarian world of early C hacking.)
I needed a way to modularise the source further, and Inweb was born.

Inweb began as that most evil of all system tools: a 4000-line Perl script
resulting from bursts of careless invention every few months. When Woody Allen
said that all literature is a footnote to Faust, he probably wasn't thinking
of the Camel book, Perl's notorious manual, but he should have been. What can
you say about a programming language where functions have no parameters but
have to read them from an array named |_|, where |local| defines a partly
global variable, and where |my a, b| creates one local and one global, thanks
to the comma being interpreted as marking a thrown-away evaluation of |my a|
(which, {\it obviously}, returns a value) and then an evaluation in void
context of the nonexistent |b|, which is helpfully and silently created as a
global variable in the process? And yet, Perl is so quick... so easy... all
that memory is yours at the slightest whim... and why work out how to derive
data in format B from the data you already have in format A, when the devil on
your shoulder whispers that you could just as easily store it both ways and
save the trouble?

More seriously, Perl was used because it was the most standard scripting
language with dynamic memory allocation available in 2003, when Inform 7
began: the largely preferable Python and Ruby had not then established
themselves as fixtures. The Perl implementation of Inweb worked well for
some years, but eventually it rusted solid, as all Perl programs do in
the end. In July 2011 the code was streamlined to remove obsolete features
and then ported to C, for speed.
