! phtalkoo.h: the Photopia-based menu conversation object-oriented library
!
! based on phototalk.inf by Adam Cadre, the Photopia dialogue source
! object-oriented and somewhat changed into phtalkoo.h by David Glasser 21-Jul-1999

! This file is a library based on the Photopia menu conversation
! system.  Its principles differ little from Adam's binary-flag module, flags.h.
!
! Each NPC needs to inherit from the Character class.  They must each
! provide the member functions SayQ(x) and Respond(x), which print
! the xth quip's 'title' and response; Respond often does stuff in
! addition to printing the response.
!
! Each NPC should also provide an InitQuips() member function, which
! should turn on (see below) the quips that should default to being
! on.  You should put the following in your Initialise function:
!     objectloop (o ofclass Character) { o.InitQuips(); }
! and define a variable o.
!
! NPC.SetQuip(42, 1) turns the NPC's quip #42 on; NPC.SetQuip(42, 0)
! turns it off.  NPC.QuipOn(42) and NPC.QuipOff(42) are shortcuts for
! the above.  NPC.QuipsOff(N, a, b, ...) turns off N quips, which are the
! rest of the arguments; N must be less than 7, as Inform only accepts 7
! arguments per function.  There is also NPC.QuipsOn, which does what you'd
! expect.  Finally, NPC.TestQuip(42) returns 1 if quip 42 is on or 0 if not.
!
! Quips are 0-based; by default, you can use fifty-six quips, from 0
! to 55.
!
! NOTE TO PHOTOTALK.INF USERS: In Adam's file, QuipOn was the tester
! and there was no shortcut for setting.  Here, TestQuip tests and
! QuipOn sets.
!
! You may wish to add life and/or orders routines to the Character
! class, and/or modify the various messages below.
!
! If any given character needs more than 56 quips, you can change
! (either in the class below or by overloading on the specific character)
! the qflag declaration to have N/8 zeros, and MAXQUIP to be N-1, where
! N is the amount of quips you want for the character, and is a multiple of
! 8.
!
! You may want to Replace some or all of AskSub, AskForSub, TellSub,
! GiveSub, ShowSub, and AnswerSub with:
!      [ FooSub; "Please use >TALK TO PLAYER to interact in this game."; ];
! and put the following into the Character class:
!      orders [; "Please use >TALK TO PLAYER to interact in this game."; ]
!
! An example can be found, commented out, at the end of this file.

Class Character
   with
   	qflag 0 0 0 0 0 0 0, ! 56 should be enough
	MAXQUIP 55,  ! like a constant
	before [x z ok selected;
		TalkTo:
		for (x=0 : x <= self.MAXQUIP : x++) {
			if (self.TestQuip(x)) { ok++; }
		}
		
		if (ok > 0) {
			print "What would you like to say?^^";

			! List the lines you have to choose from.
			
			for (x=0: x <= self.MAXQUIP: x++) {
				if (self.TestQuip(x)) {
					z++;
					print "[", z, "] ";
					self.SayQ(x);
				}
     		}

			! Get the choice and respond to it.
     
			new_line;
			do {
				print "Select an option or 0 to say nothing >> ";
				read buffer parse DrawStatusLine;
				selected = TryNumber(1);
			} until ((selected >= 0) && (selected <= z));
 			if (selected == 0) print "^You decide not to say anything after all.^";

			if (selected ~= 0) {
				ok = 0; new_line;
				for (x=0: x <= self.MAXQUIP: x++) {
					if (self.TestQuip(x)) {
						ok++;
						if (ok == selected) {
							self.Respond(x);
						} ! end if it's the right one
					} !end if it's even on
				} !end for all the possible quips
			} !end if we want any response
			
			rtrue;
		} !end if there are any quips
		
		"You can't think of anything in particular to say.";
	],
	SetQuip [ line onoff     y z;
		if (line > self.MAXQUIP) {
			"Oh, dear!  Too high a quip in ", (the) self, ".SetQuip! [BUG]";
		}
		y = line / 8;  ! y is the bytecount
		z = line % 8;  ! z is the bitnum
		z = TwoPower(z);  ! now a byte with only that bit set
		if (onoff == 1) {
			self.&qflag->y = self.&qflag->y | z; }
		if (onoff == 0) {
			self.&qflag->y = self.&qflag->y & ~z; }
		],
	QuipOn [line; self.SetQuip(line, 1); ],
	QuipOff [line; self.SetQuip(line, 0); ],
	QuipsOn [n a b c d e f; ! this function can probably be written better
		if ((n < 0) || (n > 6)) { ! 0 is ignored
			"QuipsOn called with first arg ", n, "!";
		}
		if (n == 0) return; self.QuipOn(a);
		if (n == 1) return; self.QuipOn(b);
		if (n == 2) return; self.QuipOn(c);
		if (n == 3) return; self.QuipOn(d);
		if (n == 4) return; self.QuipOn(e);
		if (n == 5) return; self.QuipOn(f);
		],
	QuipsOff [n a b c d e f; ! this function can probably be written better
		if ((n < 0) || (n > 6)) { ! 0 is ignored
			"QuipsOff called with first arg ", n, "!";
		}
		if (n == 0) return; self.QuipOff(a);
		if (n == 1) return; self.QuipOff(b);
		if (n == 2) return; self.QuipOff(c);
		if (n == 3) return; self.QuipOff(d);
		if (n == 4) return; self.QuipOff(e);
		if (n == 5) return; self.QuipOff(f);
		],
	TestQuip [line      y z;
		if (line > self.MAXQUIP) {
			"Oh, dear!  Too high a quip in ", (the) self, ".TestQuip! [BUG]";
		}
		y = line / 8;
		z = line % 8;
		z = TwoPower(z);
		if (self.&qflag->y & z == z) { rtrue; }
		rfalse;
	],
	SayQ [; "Something? [BUG]";],
	Respond [; "I don't know what to say. [BUG]"; ],
	InitQuips [; "~My InitQuips should be overloaded,~ says ", (the) self, ". [BUG]";],

	has animate
;

[ TalkToSub;  ! One method of communication to rule them all
   if (noun == player) "Talking to yourself is not particularly fun.";
   if (~~(noun ofclass Character)) "Generally, it's best to talk to living things.";
                  print (The) noun;
                  if (noun has pluralname) print " don't";
                          else print " doesn't";
                              " seem interested.";
];

[ TwoPower a;
	switch(a) {
	0:	return $$000000001;
	1:	return $$000000010;
	2:	return $$000000100;
	3:	return $$000001000;
	4:	return $$000010000;
	5:	return $$000100000;
	6:	return $$001000000;
	7:	return $$010000000;
	}
	"Error: TwoPower out of range: ", a, "!"; 
];

Verb 'talk'
   * 'to' creature -> TalkTo
   * 'to' noun -> TalkTo
;

!Constant Story "Joe Schmoe";
!Constant Headline "^An Interactive phtalkoo.h Test^
!				Copyright (c) 1998 by David S. Glasser.^";
!
!Include "Parser.h";				! Inform Library Part 1
!
!Include "phtalkoo.h";
!
![ Initialise o;
!	location = here;
!	print "^Joe Schmoe is yor frend!^^";
!	objectloop (o ofclass Character) { o.InitQuips(); }
!];
!
!Include "VerbLib.h";			! Inform Library Part 2
!Include "Grammar.h";			! Inform Library Part 3
!
!Object here "Joe Schmoe's Place"
!with description "You are in Joe Schmoe's place." has light;
!
!Character -> JoeSchmoe "Joe Schmoe"
!	with name 'Joe' 'Schmoe',
!	description "This is Joe.  It's yor frend!",
!	SayQ [line; switch (line) {
!			0: "Who are you?";
!			1:   "Why does Inform hate you?";
!			2:     "You probably just left out a semi-colon.";
!			3:     "I don't care.";
!			4:   "Ha, ha.  Inform hates you.";
!			5: "Do you like a monkey?";
!			6:   "Only crazy people like monkeys.";
!			7:   "No, I said ~a monkey~, not ~monkeys~.";
!			8:   "I like monkeys too.";
!        }
!        ],
!	Respond [line; switch (line) {
!			0:	self.QuipOff(0); self.QuipsOn(2, 1,4);
!				"~My name is Joe Schmoe and Inform hates me.~";
!			1:	self.QuipOff(1); self.QuipsOn(2, 2,3);
!				"~I tried to compile a game and it gave me forty errors.~";
!			2:	deadflag = 2; ! no need to set quips when the game's over
!				"~Thanks so much!  You're the greatest!~";
!			3,4:deadflag = 1;
!				"~I hate you!~^Joe kills you.";
!			5:	self.QuipOff(5); self.QuipsOn(3, 6,7,8);
!				"~Of course I like monkeys.~";
!			6:	deadflag = 1;
!				"~I hate you!~^Joe kills you.";
!			7:	deadflag = 1;
!				"~I hate pedants!~^Joe kills you.";
!			8:	deadflag = 2;
!				"~Now we can be friends!~";
!			}
!	],
!	InitQuips [; self.QuipsOn(2, 0,5); ],
!	
!	has male proper
!;
