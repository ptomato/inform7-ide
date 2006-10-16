!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! -*- Inform -*- !!!!
!       DOORS           An INFORM 6.10 library creating easy-to-code doors
!                       By L. Ross Raszewski <rraszews@skipjack.bluecrab.org>
!
!  According to "The Inform Designer's Manual, 3rd edition" (reprinted 
! without permission)
! To set up a door:
!
!   (a) give the object the door attribute;
!   (b) set its door_to property to the destination;
!   (c) set its door_dir property to the direction which that would be,
!       such as n_to;
!   (d) make the room's map connection in that direction point to the door
!       itself.
!
! Yeah, right.  The majority of doors you'll meet in real life are two-sided
! and anyone who's tried to code up a two-sided door in Inform knows what 
! they're in for. You'll need something like a
!       door_to [; if (location == shrine) return jungle; return shrine;],
! and a 
!       door_dir [; if (location == shrine) return u_to; return d_to;],
! Now, I'll agree this isn't all that bad, but I'm sure you've wished it 
! was easier.  Many a newbie has died trying to code up something lke this.
!
! Not anymore.
!
! Observe if you will the Connector class.
!
! Suppose you need a door between roomA and roomB, where roomA is to the east 
! of roomB.
!
!
!  Connector door "door"
!        with name "door",
!             e_to roomA,
!             w_to roomB,
!             when_open "The door is open",  
!             when_closed "The door is closed",
!        has openable;
!
! Voila!  This simple syntax is equivalent to the conventional syntax:
!
! Object door "door"
!        with name "door",
!             door_to [; if (location == roomA) return roomB; return roomA;],
!             door_dir [; if (location == roomB) return e_to; return w_to;],
!             when_open "The door is open",  
!             when_closed "The door is closed",
!             found_in roomA roomB;
!        has openable door static;
!
! I can hear you now.  "What?  I save three lousy lines?"  Well, yeah, but in
! a game with a lot of doors, you've saved yourself some syntax. And it 
! certaintly is more intuitive.
! 
! The only catch, for the sake of economy, is that you have to call the 
! function InitDoors(); in your initialise.
!
! Also by the author:
!               The AltMenuing system
!       Center          Centers a line of text in either window.
!       Domenu          Improved menuing with multiple description lines
!       Altmenu         Object oriented menuing system
!       Hints           Altmenu hint system
!       Sound           The Inform Sound System
!       YesNo           pseudo-rhetorical Yes or no questions
!       Date            Datekeeping and printing
!       Footnote        Autonumbering footnotes
!       Locktest        Default key selection
!       Newlock         Key-side lock definition
!       Ordinal         Ordinal number printing
!       Whatis          "What is a" questions
!       Senses          Recursive sensory perception
!       Pmove           Move objects into the tree as the youngest object
!       Movie           Non-interactive cut-scenes
!       Manual          inform "instruction manual" system
!       
!  Coming soon:
!       Converse        Hit-word based conversation
!       MenuTalk        Menu-based Conversation
!       UniCursr        Unicursor- Text adventures with the contol simplicity
!                       of omnifunction click graphic games (ie. Type "clock" 
!                       to examine, take, or manipulate the clock)
!
!       Please write me and tell me what you think.

Class	Connector
 with	door_dir [;
		if (location==self.sidea) return self.dira;
		return self.dirb;
	],
	door_to [;
		if (location==self.sidea) return self.sideb;
		return self.sidea;
	],
	sidea 0,
	dira 0,
	sideb 0,
	dirb 0,
	found_in 0 0,
  has	door static;

[ InitDoors o i j;
	objectloop (j ofclass Connector) { 
		objectloop (o in compass) {
			i = o.door_dir;
			if (j provides i) {
				j.sidea = j.i;
				j.dirb = i;
			}
		}

		objectloop (o in compass) {
			i = o.door_dir;
			if ((j provides i) && (j.dirb ~= i)) {
				j.sideb = j.i;
				j.dira = i;
			}
		}

		j.&found_in-->0 = j.sidea;
		j.&found_in-->1 = j.sideb;
	}
];
