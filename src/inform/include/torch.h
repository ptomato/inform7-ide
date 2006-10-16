! -------------------------------------------------------------------------
! torch.h
! 
! Copyright 2004 David Griffith <dgriffi@cs.csubak.edu>
! Use this code freely, but don't claim you wrote it.  Give proper
! attributions.
!
! This code allows one to simulate torches (the flaming, wooden kind).
! One can have good ones (ready to be ignited), burning ones, and
! burned-out ones.  
!
!
! Include this file sometime after VerbLib.
!
!
! Somewhere at the beginning of your code, you'll need to set the
! following constants:
!
! Constant TORCH_COUNT = 18;	! how many torches in the game
! Constant TORCH_VARIANCE = 7;	! some randomness
! Constant TORCH_LIFE = 50;	! how many moves a torch will last
! Constant TORCH_DECLINE = 5;	! torch starts sputtering
! Constant TORCH_DYING = 1;	! torch is about to go out
!
! Here is a handy way to give the player a burning torch at the
! begining of the game.  Use it in the Initialize() function.
!
!	move players_torch to player;
!	players_torch.power = TORCH_LIFE;
!	StartDaemon(players_torch);
!
! Make sure that you've actually created players_torch somewhere outside
! of the functions.  Do it like this:
!
!	BurningTorch players_torch;
!
! Now, when you want to have some torches sitting around for the player
! to pick up, do this:
!
!	UnlitTorch ->;
!
! That will create one unlit torch.  When picked up, the game will
! respond with "You pull the unlit torch off the wall".  Similarly, you
! can use "BurningTorch ->;" to have a burning torch stuck on a wall.
! The same goes for a dead torch.  Count up all the times you do this
! and put that number as TORCH_COUNT (see above).
!
! You shouldn't need to modify anything below this point.  If you do,
! please let me know why and how you modified the code.


Class Torch
	with name "torch",
	power,
	parse_name [ i j w;
		if (parser_action == ##TheSame) {
			if ((parser_one.&name)-->0 == (parser_two.&name)-->0) {
				return -1;
			}
			return -2;
		}
		w = (self.&name)-->0;
		for (:: i++) {
			j = NextWord();
			if (j == 'torches') parser_action=##PluralFound;
			else if (j~='torch' or w) return i;
		}
	],
	daemon [ t newthing i;
		t = --(self.power);
		if (t == 0) {
			give self ~on ~light;
			newthing = DeadTorch.create();
			give newthing moved;
			move newthing to parent(self);
			remove self;
		}
		if (self in player || self in location) {
			objectloop (i in player) {
				if (i ofclass BurningTorch) {
					if (i.power > TORCH_DECLINE)
						rtrue;
				}
			}
			if (t == TORCH_DECLINE + 1)
				"^Your torch is beginning to flicker.";
			if (t <= TORCH_DECLINE && t > TORCH_DYING)
				"^Your torch is flickering and sputtering.";
			if (t <= TORCH_DYING && t > 0)
				"^Your torch is going out.";
			if (t == 0)
				"^Your torch goes out with a fizzle.";
		}
	],
	before [newthing;
	Smell:
		print "It smells like ";
		if (self ofclass UnlitTorch)
			"oil and dirt.";
		if (self ofclass BurningTorch)
			"burning oil and dirt.";
		if (self ofclass DeadTorch)
			"oily soot.";
		"Error in Torch class (Smell)^";
	Examine:
		if (self ofclass UnlitTorch)
			"It's a torch. An oil-soaked rag is wrapped around it.";
		if (self ofclass DeadTorch)
			"It's a burned out and useless torch.";
		if (self ofclass BurningTorch) {
			print "The torch throws ";
			if (self.power < TORCH_DECLINE) {
				print "feeble";
			} else {
				print "dancing";
			}
			print " shadows about the place.";
		}
		"Error in Torch class (Examine)^";
	Burn:
		if (second ~= nothing) {
			if (second hasnt on) 
				"You can't burn ", (the) self, " with ", (the)
				second, ".";
		}
		if (self has on) "But it's already lit!";
		<<SwitchOn self second>>;
	SwitchOff:
		if (verb_word == 'turn' or 'switch')
			"Perhaps snuffing the torch would be more useful.";
		if (self ofclass UnlitTorch or DeadTorch)
			"But it's not burning.";
		if (self ofclass BurningTorch) {
			newthing = DeadTorch.create();
			move newthing to parent(self);
			give newthing moved;
			remove self;
			"You snuff out the torch.";
		}
		"Error in Torch class (SwitchOff)^";
	],

	after [newthing;
	Take:
		if (self hasnt moved)
			"You pull ", (the) noun, " off the wall.";

	SwitchOn: 
		if (second == nothing) {
			give self ~on;
			"Perhaps lighting the torch with something
			would be more useful.";
		}
		if (self ofclass UnlitTorch) {
			newthing = BurningTorch.create();
			if (newthing == nothing) {
				"Programming error!
				Can't light the torch.";
			}
			move newthing to parent(self);
			newthing.power = TORCH_LIFE + random(TORCH_VARIANCE);
			remove self;
			StartDaemon(newthing);
			"The torch burns brightly.";
		}
		if (self ofclass BurningTorch)
			"But it's already burning.";
		if (self ofclass DeadTorch) {
			give self ~on;
			"The dead torch sputters a bit, but
				refuses to light.";
		}
		"Error in Torch class (SwitchOn)^";
	],
	has switchable float;


Class UnlitTorch (TORCH_COUNT)
	class Torch
	with name "unlit" "good" "torch",
	short_name "unlit torch",
	plural "unlit torches";

Class BurningTorch (TORCH_COUNT)
	class Torch
	with name "burning" "torch",
	short_name "burning torch",
	plural "burning torches",
	has on light;

Class DeadTorch (TORCH_COUNT)
	class Torch
	with name "dead" "bad" "torch",
	short_name "dead torch",
	plural "dead torches";

