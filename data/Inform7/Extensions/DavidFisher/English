English by David Fisher begins here.

"To make English the language of play, and allow the standard textual responses to commands to be changed."

Include (-
Constant AGAIN1__WD     = 'again';
Constant AGAIN2__WD     = 'g//';
Constant AGAIN3__WD     = 'again';
Constant OOPS1__WD      = 'oops';
Constant OOPS2__WD      = 'o//';
Constant OOPS3__WD      = 'oops';
Constant UNDO1__WD      = 'undo';
Constant UNDO2__WD      = 'undo';
Constant UNDO3__WD      = 'undo';

Constant ALL1__WD       = 'all';
Constant ALL2__WD       = 'each';
Constant ALL3__WD       = 'every';
Constant ALL4__WD       = 'everything';
Constant ALL5__WD       = 'both';
Constant AND1__WD       = 'and';
Constant AND2__WD       = 'and';
Constant AND3__WD       = 'and';
Constant BUT1__WD       = 'but';
Constant BUT2__WD       = 'except';
Constant BUT3__WD       = 'but';
Constant ME1__WD        = 'me';
Constant ME2__WD        = 'myself';
Constant ME3__WD        = 'self';
Constant OF1__WD        = 'of';
Constant OF2__WD        = 'of';
Constant OF3__WD        = 'of';
Constant OF4__WD        = 'of';
Constant OTHER1__WD     = 'another';
Constant OTHER2__WD     = 'other';
Constant OTHER3__WD     = 'other';
Constant THEN1__WD      = 'then';
Constant THEN2__WD      = 'then';
Constant THEN3__WD      = 'then';

Constant NO1__WD        = 'n//';
Constant NO2__WD        = 'no';
Constant NO3__WD        = 'no';
Constant YES1__WD       = 'y//';
Constant YES2__WD       = 'yes';
Constant YES3__WD       = 'yes';

Constant AMUSING__WD    = 'amusing';
Constant FULLSCORE1__WD = 'fullscore';
Constant FULLSCORE2__WD = 'full';
Constant QUIT1__WD      = 'q//';
Constant QUIT2__WD      = 'quit';
Constant RESTART__WD    = 'restart';
Constant RESTORE__WD    = 'restore';
-) instead of "Vocabulary" in "Language.i6t".

Include (-
Array LanguagePronouns table

  ! word        possible GNAs                   connected
  !             to follow:                      to:
  !             a     i
  !             s  p  s  p
  !             mfnmfnmfnmfn

    'it'      $$001000111000                    NULL
    'him'     $$100000000000                    NULL
    'her'     $$010000000000                    NULL
    'them'    $$000111000111                    NULL;
-) instead of "Pronouns" in "Language.i6t".

Include (-
Array LanguageDescriptors table

  ! word        possible GNAs   descriptor      connected
  !             to follow:      type:           to:
  !             a     i
  !             s  p  s  p
  !             mfnmfnmfnmfn

    'my'      $$111111111111    POSSESS_PK      0
    'this'    $$111111111111    POSSESS_PK      0
    'these'   $$000111000111    POSSESS_PK      0
    'that'    $$111111111111    POSSESS_PK      1
    'those'   $$000111000111    POSSESS_PK      1
    'his'     $$111111111111    POSSESS_PK      'him'
    'her'     $$111111111111    POSSESS_PK      'her'
    'their'   $$111111111111    POSSESS_PK      'them'
    'its'     $$111111111111    POSSESS_PK      'it'
    'the'     $$111111111111    DEFART_PK       NULL
    'a//'     $$111000111000    INDEFART_PK     NULL
    'an'      $$111000111000    INDEFART_PK     NULL
    'some'    $$000111000111    INDEFART_PK     NULL
    'lit'     $$111111111111    light           NULL
    'lighted' $$111111111111    light           NULL
    'unlit'   $$111111111111    (-light)        NULL;
-) instead of "Descriptors" in "Language.i6t".

Include (-
Array LanguageNumbers table
    'one' 1 'two' 2 'three' 3 'four' 4 'five' 5
    'six' 6 'seven' 7 'eight' 8 'nine' 9 'ten' 10
    'eleven' 11 'twelve' 12 'thirteen' 13 'fourteen' 14 'fifteen' 15
    'sixteen' 16 'seventeen' 17 'eighteen' 18 'nineteen' 19 'twenty' 20
    'twenty-one' 21 'twenty-two' 22 'twenty-three' 23 'twenty-four' 24
	'twenty-five' 25 'twenty-six' 26 'twenty-seven' 27 'twenty-eight' 28
	'twenty-nine' 29 'thirty' 30
;

[ LanguageNumber n f;
    if (n == 0)    { print "zero"; rfalse; }
    if (n < 0)     { print "minus "; n = -n; }
    if (n >= 1000) { print (LanguageNumber) n/1000, " thousand"; n = n%1000; f = 1; }
    if (n >= 100)  {
        if (f == 1) print ", ";
        print (LanguageNumber) n/100, " hundred"; n = n%100; f = 1;
    }
    if (n == 0) rfalse;
    #Ifdef DIALECT_US;
    if (f == 1) print " ";
    #Ifnot;
    if (f == 1) print " and ";
    #Endif;
    switch (n) {
      1:    print "one";
      2:    print "two";
      3:    print "three";
      4:    print "four";
      5:    print "five";
      6:    print "six";
      7:    print "seven";
      8:    print "eight";
      9:    print "nine";
      10:   print "ten";
      11:   print "eleven";
      12:   print "twelve";
      13:   print "thirteen";
      14:   print "fourteen";
      15:   print "fifteen";
      16:   print "sixteen";
      17:   print "seventeen";
      18:   print "eighteen";
      19:   print "nineteen";
      20 to 99: switch (n/10) {
        2:  print "twenty";
        3:  print "thirty";
        4:  print "forty";
        5:  print "fifty";
        6:  print "sixty";
        7:  print "seventy";
        8:  print "eighty";
        9:  print "ninety";
        }
        if (n%10 ~= 0) print "-", (LanguageNumber) n%10;
    }
];
-) instead of "Numbers" in "Language.i6t".

Include (-
[ LanguageTimeOfDay hours mins i;
    i = hours%12;
    if (i == 0) i = 12;
    if (i < 10) print " ";
    print i, ":", mins/10, mins%10;
    if ((hours/12) > 0) print " pm"; else print " am";
];
-) instead of "Time" in "Language.i6t".

Include (-
[ LanguageDirection d;
    switch (d) {
      n_to:    print "north";
      s_to:    print "south";
      e_to:    print "east";
      w_to:    print "west";
      ne_to:   print "northeast";
      nw_to:   print "northwest";
      se_to:   print "southeast";
      sw_to:   print "southwest";
      u_to:    print "up";
      d_to:    print "down";
      in_to:   print "in";
      out_to:  print "out";
      default: return RunTimeError(9,d);
    }
];
-) instead of "Directions" in "Language.i6t".

Include (-
[ LanguageToInformese; ];
-) instead of "Translation" in "Language.i6t".

Include (-
Constant LanguageAnimateGender   = male;
Constant LanguageInanimateGender = neuter;

Constant LanguageContractionForms = 2;     ! English has two:
                                           ! 0 = starting with a consonant
                                           ! 1 = starting with a vowel

[ LanguageContraction text;
    if (text->0 == 'a' or 'e' or 'i' or 'o' or 'u'
                or 'A' or 'E' or 'I' or 'O' or 'U') return 1;
    return 0;
];

Array LanguageArticles -->

 !   Contraction form 0:     Contraction form 1:
 !   Cdef   Def    Indef     Cdef   Def    Indef

     "The " "the " "a "      "The " "the " "an "          ! Articles 0
     "The " "the " "some "   "The " "the " "some ";       ! Articles 1

                   !             a           i
                   !             s     p     s     p
                   !             m f n m f n m f n m f n

Array LanguageGNAsToArticles --> 0 0 0 1 1 1 0 0 0 1 1 1;
-) instead of "Articles" in "Language.i6t".

Include (-
[ LanguageVerb i;
    switch (i) {
      'i//','inv','inventory':
               print "take inventory";
      'l//':   print "look";
      'x//':   print "examine";
      'z//':   print "wait";
      default: rfalse;
    }
    rtrue;
];

[ LanguageVerbLikesAdverb w;
    if (w == 'look' or 'go' or 'push' or 'walk')
        rtrue;
    rfalse;
];

[ LanguageVerbMayBeName w;
    if (w == 'long' or 'short' or 'normal'
                    or 'brief' or 'full' or 'verbose')
        rtrue;
    rfalse;
];
-) instead of "Commands" in "Language.i6t".

Include (-
Constant NKEY__TX       = "N = next subject";
Constant PKEY__TX       = "P = previous";
Constant QKEY1__TX      = "  Q = resume game";
Constant QKEY2__TX      = "Q = previous menu";
Constant RKEY__TX       = "RETURN = read subject";

Constant NKEY1__KY      = 'N';
Constant NKEY2__KY      = 'n';
Constant PKEY1__KY      = 'P';
Constant PKEY2__KY      = 'p';
Constant QKEY1__KY      = 'Q';
Constant QKEY2__KY      = 'q';

Constant SCORE__TX      = "Score: ";
Constant MOVES__TX      = "Moves: ";
Constant TIME__TX       = "Time: ";
Global CANTGO__TX     = "You can't go that way.";
Global FORMER__TX     = "your former self";
Global YOURSELF__TX   = "yourself";
Constant YOU__TX        = "You";
Constant DARKNESS__TX   = "Darkness";

Constant THOSET__TX     = "those things";
Constant THAT__TX       = "that";
Constant OR__TX         = " or ";
Constant NOTHING__TX    = "nothing";
Global IS__TX         = " is";
Global ARE__TX        = " are";
Global IS2__TX        = "is ";
Global ARE2__TX       = "are ";
Global IS3__TX        = "is";
Global ARE3__TX       = "are";
Constant AND__TX        = " and ";
#ifdef I7_SERIAL_COMMA;
Constant LISTAND__TX   = ", and ";
Constant LISTAND2__TX  = " and ";
#ifnot;
Constant LISTAND__TX   = " and ";
Constant LISTAND2__TX  = " and ";
#endif; ! I7_SERIAL_COMMA
Constant WHOM__TX       = "whom ";
Constant WHICH__TX      = "which ";
Constant COMMA__TX      = ", ";
-) instead of "Short Texts" in "Language.i6t".

Include (-
[ ThatorThose obj;      ! Used in the accusative
    if (obj == player)            { print "you"; return; }
    if (obj has pluralname)       { print "those"; return; }
    if (obj has animate) {
        if (obj has female)       { print "her"; return; }
        else
            if (obj hasnt neuter) { print "him"; return; }
    }
    print "that";
];

[ ItorThem obj;
    if (obj == player)            { print "yourself"; return; }
    if (obj has pluralname)       { print "them"; return; }
    if (obj has animate) {
        if (obj has female)       { print "her"; return; }
        else
            if (obj hasnt neuter) { print "him"; return; }
    }
    print "it";
];

[ IsorAre obj;
    if (obj has pluralname || obj == player) print "are"; else print "is";
];

[ HasorHave obj;
    if (obj has pluralname || obj == player) print "have"; else print "has";
];

[ CThatorThose obj;     ! Used in the nominative
    if (obj == player)            { print "You"; return; }
    if (obj has pluralname)       { print "Those"; return; }
    if (obj has animate) {
        if (obj has female)       { print "She"; return; }
        else
            if (obj hasnt neuter) { print "He"; return; }
    }
    print "That";
];

[ CTheyreorThats obj;
    if (obj == player)             { print "You're"; return; }
    if (obj has pluralname)        { print "They're"; return; }
    if (obj has animate) {
        if (obj has female)        { print "She's"; return; }
        else if (obj hasnt neuter) { print "He's"; return; }
    }
    print "That's";
];

[ HisHerTheir o; if (o has pluralname) { print "their"; return; }
	if (o has female) { print "her"; return; }
	if (o has neuter) { print "its"; return; }
	print "his";
];

[ HimHerItself o; if (o has pluralname) { print "theirselves"; return; }
	if (o has female) { print "herself"; return; }
	if (o has neuter) { print "itself"; return; }
	print "himself";
];
-) instead of "Printed Inflections" in "Language.i6t".

Include (-
! The standard messages are implemented in I7 source text, not here in
! I6 code.
-) instead of "Long Texts" in "Language.i6t".

English ends here.
