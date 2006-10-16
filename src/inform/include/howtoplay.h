! $Id: howtoplay.h,v 1.1.1.1 2006-10-16 14:30:02 pchimento Exp $
! ----------------------------------------------------------------------------
!   howtoplay.h for Inform 6, by Michael Coyne (coyne@mts.net)
!	How to Play text Copyright 2003 Fredrik Ramsberg
!
! ----------------------------------------------------------------------------
!   Installation: add the lines:
!
!       Include "Menus";    ! optional
!       Include "Howtoplay";
!
!   anywhere in your game AFTER the Include "VerbLib"; directive
!
! ----------------------------------------------------------------------------
!   Based on the structure of Roger Firth's ccpl.h
!
!   Implements a HELP verb which uses Menus.h (or any compatible menu
!   package) to display a menu of Fredrik Ramsberg's "How to Play"
!   instructions from "A Beginner's Guide to Playing Interactive Fiction".
!   Fredrik's full text can be found at:
!   http://IFGuide.ramsberg.net
!
!   Define NO_HOWTO_GRAMMAR to prevent the Help verb from being included.  You
!   will then have to provide an alternate means of running HowToMenu.select()
!
!   If you are not using Menus, then HowToMenu.select will just dump the
!   text to the screen.
!
!   If you wish to include this in an existing menu in your game instead of
!   just having it available through the help verb, you can do this:
!
!   Menu MyMenu "My Menu Object"
!      with   description "Please select one of the following items.^",
!            number 2;
!
!   Option   ->   "How to Play"
!      with   description [;
!               howtoplay.description();
!         ];
!
!	$Log: not supported by cvs2svn $
!	Revision 1.1  2004/03/15 18:32:52  glen
!	New file.
!	
!	Revision 1.2  2003/05/24 13:36:34  michael
!	Removed DOS carriage-returns introduced by opening on DOS machine.
!	
!	Revision 1.1  2003/05/24 13:32:44  michael
!	Initial revision.
!	
! ---------------------------------------------------------------------------- !

system_file;

#ifndef MENU;
Class   Menu with select [;
         howtoplay.description();
      ];
#endif;
#ifndef OPTION;
Class   Option;
#endif;

[ bf text; style bold; print (string) text; style roman; ];

Menu    HowToMenu
   with   short_name [;
              print "How to Play ", (string) Story;
            rtrue;
         ];

Option   ->   howtoplay
   with   short_name [;
              print "How to Play ", (string) Story;
            rtrue;
         ],
         description [;
         print "These general instructions are an excerpt from:
         ~A Beginner's Guide to Playing Interactive Fiction~. 
         The full text can be found at http://IFGuide.ramsberg.net",

         (bf) "^^The game starts";
         print "
         ^When you start a game, you will first see an introduction, usually
         consisting of one or a few screenfuls of text, giving you some
         background on who you are, where you are, and perhaps even what your
         objectives in the game are. Whenever the game has printed a
         screenful of text, it will wait until you press ENTER or some other
         key, so that you get a chance to read everything before it scrolls
         off the top of the screen.^",

         (bf) "^How to interact";
         print "
         ^When the introduction is over, you will get a prompt, usually ~>~,
         but it may be a little different from game to game. The prompt means
         that the game is now waiting for you to tell it what you want to do.
         You do this by typing in imperative commands, as if you were
         commanding someone. Let's say the introduction told you that you are
         in a kitchen, and that you can see a closed glass jar standing on
         the kitchen counter. Commands you could try at this point include
         TAKE THE JAR, or OPEN THE JAR, or perhaps EXAMINE THE JAR
         (Throughout this document, things that are written in capital
         letters are complete commands that can be typed into an IF game.
         They don't have to be typed in capital letters when entered into a
         game). If you want to, you can skip the articles: TAKE JAR will work
         just at well as TAKE THE JAR. If there are several different jars
         you could mean, the game may ask you which one you mean. Just type
         one or more words that uniquely identifies one of the items. For
         instance, if the game says ~Which jar do you mean, the blue glass
         jar or the green glass jar?~, you might reply BLUE to take the blue
         one. You can also choose to ignore the question altogether, just
         typing a new command.^",

         (bf) "^Movement";
         print "
         ^To go to another location, most games expect you to type in which
         direction you want to go. You can type GO SOUTH, but just SOUTH will
         also do the trick, as will S (which is the commonly accepted
         abbreviation for SOUTH). Other directions and their abbreviations
         are NORTH (N), EAST (E), WEST (W), NORTHEAST (NE), SOUTHEAST (SE),
         NORTHWEST (NW), SOUTHWEST (SW), UP (U), DOWN (D), IN and OUT. If you
         are aboard a ship of some kind you may also be able to use FORE,
         AFT, STARBOARD and PORT.^
         ^Other ways to move around may include commands like ENTER CAR, GO
         CAR, SIT ON MOTORCYCLE, GET ON BIKE, CLIMB ONTO SHIP, JUMP ONTO
         PLATFORM, DIVE INTO LAKE, BOARD SHIP, EXIT CAR, EXIT, LEAVE, GET
         OUT. Exactly which commands are ";
#ifdef DIALECT_US;
         print "recognized";
#ifnot;
         print "recognised";
#endif;
         print " vary from game to game as well as from situation to
         situation in those games. When interacting with IF games, always
         try to express yourself as simply as possible.
         If you have tried several ways of expressing yourself and the game
         refuses to understand what you want to do, you are most probably on
         the wrong track; it's time to try something completely different.^",

         (bf) "^Common verbs";
         print "
         ^As you know by now, you can use the verb TAKE to pick up items in
         the game. Of course, you can also use DROP to drop items. Most
         modern games actually ";
#ifdef DIALECT_US;
         print "recognize";
#ifnot;
         print "recognise";
#endif;
         print " a hundred different verbs or more. With some of the most
         used verbs, you can also use multiple items, like this: TAKE GREEN
         BALL AND SCREWDRIVER or DROP ALL or PUT ALL BUT HAMMER IN BAG.
         You'll find that ALL is often a very useful word, although it only
         works with certain verbs, most notably TAKE and
         DROP. Here are some of most important verbs, with examples:^";

         font off;
         print "^
         LOOK or L       L or LOOK AT BOB or LOOK IN JAR or LOOK UNDER BED^
         TAKE            TAKE KNIFE^
         DROP            DROP KNIFE^
         EXAMINE or X    EXAMINE KNIFE or X KNIFE^
         SEARCH          SEARCH DESK^
         INVENTORY or I  I^
         OPEN            OPEN DRAWER^
         CLOSE           CLOSE DRAWER^
         LOCK            LOCK DOOR WITH RUSTY KEY^
         UNLOCK          UNLOCK DOOR WITH RUSTY KEY^
         ASK             ASK JOHN ABOUT POLICE OFFICER^
         TELL            TELL JOHN ABOUT MURDER^
         SAY             SAY HELLO TO JOHN^
         GIVE            GIVE RABBIT TO BOB^
         SHOW            SHOW KNIFE TO POLICE OFFICER^
         WAIT or Z       Z^
         AGAIN or G      G^";
         font on;

         print "^Other verbs you will need from time to time include ATTACK,
         BUY, COVER, DRINK, EAT, FILL, JUMP, KISS, KNOCK, LISTEN, MOVE, PULL,
         PUSH, REMOVE, READ, SIT, SLEEP, STAND, THROW, TIE, TOUCH, TURN,
         TYPE, UNTIE, WEAR. There are lots more. Hopefully they will seem
         natural to you when you need them.^",

         (bf) "^How time works";
         print "
         ^Almost all IF games count time in turns, rather than hours and
         minutes. Every time you type something and press ENTER, one turn
         passes. This also means that until you press ENTER, no time passes.
         You could think of a turn as being something like a minute, but how
         long it actually is depends on what you do during that turn. If you
         want time to pass, but don't want to perform any actions, just type
         WAIT or Z. This will prove useful while waiting for someone to
         arrive or something to get ready in the oven etc (in the game world,
         not in the real world!).^

         ^There are games that use real-time instead of turn-based play, but
         they are few and far between, and they will tell you about their
         real-time system at the beginning of the game.^",

         (bf) "^Talking to people";
         print "
         ^The most useful ways of talking to people usually involve the verbs
         ASK and TELL. When using them, try to pin down the best keyword for
         what you are interested in, rather than longer constructs. For
         example, TELL BOB ABOUT HOW I SAW SHEILA GIVE A STRANGE AMULET TO
         ANOTHER WOMAN is not likely to yield any useful results, but TELL
         BOB ABOUT AMULET or perhaps TELL BOB ABOUT SHEILA may indeed be
         useful. In other words, you tell the game the subject you want to
         talk about or ask about, not exactly what to say. The game will try
         to make reasonable assumptions on what you want to say regarding the
         subject.^";

         print "^Also note that many games are quite primitive when it comes
         to modelling people. The author has to put in an enormous amount of
         work to make people in the game behave realistically and respond
         well to conversation. In general, don't expect too much from people
         in the game, but there are of course games that shine in this area
         too. You'll also see that some authors prefer menu-based
         conversation, to facilitate interaction.^

         ^To tell someone else to do something, type the name of the person,
         a comma, and then a command. Example: BOB, BREAK THE JAR. Just like
         in real life, most people won't automatically do something just
         because you tell them to. If you think Bob knows what to do with the
         jar, you can also try GIVE JAR TO BOB or SHOW JAR TO BOB.^",

         (bf) "^Special verbs";
         print "^All games ";
#ifdef DIALECT_US;
         print "recognize";
#ifnot;
         print "recognise";
#endif;
         print " some verbs that don't do anything in the game world, but
         tells the game something about how you want it to behave,
         or some special task you want it to peform. These verbs include:^";

         font off;
         print "^
         UNDO            Takes back the last move you made.^
         QUIT or Q       Ends the current game.^
         RESTART         Starts the game over from the beginning.^
         SAVE            Saves your current position to a file on disk.^
         RESTORE         Loads a previously saved game position.^
         HELP or ABOUT   Shows some information about the game and its
                        author, in some cases even hints to some of the
                     puzzles.^
         VERBOSE         Tells the game you want a long description of every
                         room you enter, even if you've been there before.^
         BRIEF           Tells the game you want a long description the first
                         time you enter a room, and a short description when
                     you come back. This is the default mode.^
         SUPERBRIEF      Tells the game you always want short descriptions of
                     all rooms.^";
         font on;

         print (bf) "^Getting stuck and unstuck";
         print "
         ^While playing IF, you will get stuck. This is part of the deal --
         where there are puzzles, there will also be stuckness. If you grow
         tired of being stuck in the same spot for too long, you can either
         type HELP in the game to see if there are any hints available, or
         you can ask other players for hints. A good place to ask for hints
         is the newsgroup rec.games.int-fiction (can be reached at 
         http://groups.google.com/groups?group=rec.games.int-fiction ).
         That's also one of the best
         places to meet other IF players, discuss games you've played, get
         tips on games you should play and more.^

         ^Oh, one last thing about playing interactive fiction. Make a map as
         you play. You are very likely to need it.";

];

[ HowToSub;      HowToMenu.select(); ];

#IFNDEF NO_HOWTO_GRAMMAR;
Verb meta 'help' 'howto' 'instructions'
    *             -> HowTo;
#ENDIF;

! ---------------------------------------------------------------------------- !
