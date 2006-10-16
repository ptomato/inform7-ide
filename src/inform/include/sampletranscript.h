
!---------------------------------------------------------------------------
!	SampleTranscript.h, by Emily Short (emshort@mindspring.com) 
!
!	Being a file of routines to print sample transcripts.  These are provided
!	with pauses, in case the player decides he wants to quit without seeing
!	the whole thing; this is to prevent an intolerable inundation of text.
!
!	The user of these routines is free to copy and paste them rather than 
!	including the whole file, and/or to modify them to suit his needs.
!
!	SampleTranscript does assume that the user is also using HelpRoutines.h.
!	If this is not the case, the user should uncomment the style routines
!	ES_Pause, ESI, and ESB, below.
!	
!
!---------------------------------------------------------------------------
!
!	CONTENTS:
!
!	-- CoS sample.  The sample included in the game City of Secrets. 
!		Contains annotations explaining basics of gameplay.
!
!	-- CheeseSample.  A brief gamelet set in Artisanal Restaurant.
!		Contains no annotations. 
!
!---------------------------------------------------------------------------

system_file;


!---------------------------------------------------------------------------
! Style effects -- bold-texting and pauses for display purposes
!--------------------------------------------------------------------------- 
!
!#ifdef TARGET_GLULX;
![ ES_Pause i; 
!	i = KeyCharPrimitive();  
!	if (i=='q') rtrue;
!	rfalse; 
!];
!#ifnot;
![ ES_Pause i;
!	@read_char 1 i;
!	if (i == 'q') rtrue;
!	rfalse; 
!];
!#endif;
!
!#ifdef TARGET_GLULX;
![ ESB str;	! print something in bold 
!	if (str==0) rfalse;
!	glk_set_style(style_Input);
!	print (string) str; 
!	glk_set_style(style_Normal);
!	rtrue;
!];
!#ifnot;
![ ESB str;	! print something in bold 
!	if (str==0) rfalse;
!	style bold;
!	print (string) str; 
!	style bold;
!	rtrue;
!];
!#endif;
!
!#ifdef TARGET_GLULX;
![ ESI str;	! print something in italics 
!	if (str==0) rfalse;
!	glk_set_style(style_Emphasized);
!	print (string) str; 
!	glk_set_style(style_Normal);
!	rtrue;
!];
!#ifnot;
![ ESI str;	! print something in italics 
!	if (str==0) rfalse;
!	style underline;
!	print (string) str; 
!	style underline;
!	rtrue;
!];
!#endif;
!


!---------------------------------------------------------------------------
! Transcripts
!--------------------------------------------------------------------------- 

[ CheeseSample;
	print (ESI) "[ At any Pause in this demo, type 'Q' to quit or any other key to continue. ]^^";
	print (ESB) "Cheese Vault, Artisanal Restaurant^";
	print "The light is dim and blue.  On three sides of you are glass-fronted refrigeration
		cases, containing cheeses of all kinds.  The locks on the doors protect them from
		your depredations, however.^^";
	print "To the west is the cheese counter.^^";
	print "A fondue fork lies on the ground.^^";
	print (ESB) ">";
	if (ES_Pause()) jump end;
	print (ESB) "LOOK IN CASE^";
	print "The cases contain cheeses of many sorts -- too many for your brain to comprehend.^^";
	print (ESB) ">OPEN CASE^";
	print "That seems to be locked.^^";
	print (ESB) ">UNLOCK CASE^";
	print "(with your wallet)^";
	print "That doesn't seem to fit.^^";
	print (ESB) ">TAKE FORK^";
	print "Taken.^^";
	print (ESB) ">";
	if (ES_Pause()) jump end;
	print (ESB) "WEST^^";
	print (ESB) "Cheese Counter, Artisanal Restaurant^";
	print "You are standing before a display counter full of assorted cheeses: Colston
		Bassett Stilton; both Keen's and Montgomery cheddar; petit Basque; Sonoma Dry
		Jack; Drunken Goat cheese...  It is a cheese-lover's dream.^^";
	print "A clerk, dressed in a white uniform, presides behind the counter.^^";
	print "To the east is the cheese vault; the rest of the restaurant is to the south.^^";
	print (ESB) ">TAKE ALL^";
	print "Colston Bassett Stilton: That seems to belong to the clerk.^";
	print "Keen's cheddar: That seems to belong to the clerk.^";
	print "Montgomery cheddar: That seems to belong to the clerk.^";
	print "Petit Basque: That seems to belong to the clerk.^";
	print "Sonoma Dry Jack: That seems to belong to the clerk.^";
	print "Drunken Goat cheese: That seems to belong to the clerk.^";
	print "clerk: The clerk probably wouldn't like that.^";
	print "display counter: That's fixed in place!^^";
	print (ESB) ">";
	if (ES_Pause()) jump end;
	print (ESB) "X CHEDDAR^";
	print "Which cheddar do you mean, the Keen's or the Montgomery?^^";
	print (ESB) ">KEEN'S^";
	print "The Keen's cheddar is well-aged, firm and golden.^^";
	print (ESB) ">ASK CLERK FOR KEEN'S CHEDDAR^";
	print "The clerk cuts you a piece of the Keen's cheddar and wraps it in wax paper. 
		~That'll be $20,~ he says, looking expectant.^^";
	print (ESB) ">I^";
	print "You are carrying:^";
	print "  a wallet (which is closed)^";
	print "  a fondue fork^^";
	print (ESB) ">";
	if (ES_Pause()) jump end;
	print (ESB) "OPEN WALLET^";
	print "You open the wallet, revealing two twenty-dollar bills and a driver's license.^^";
	print (ESB) ">GIVE TWENTY TO CLERK^";
	print "You need to be holding the twenty-dollar bill first.^^";
	print (ESB) ">TAKE TWENTY^";
	print "Taken.^^";
	print (ESB) ">I^^";
	print "You are carrying:^";
	print "  a twenty-dollar bill^";
	print "  a wallet (which is open)^";
	print "    a twenty-dollar bill^";
	print "    a driver's license^";
	print "  a fondue fork^^";
 	print (ESB) ">";
	if (ES_Pause()) jump end;
	print (ESB) "GIVE TWENTY TO CLERK^";
	print "The clerk accepts your money and hands you a triangular package wrapped in wax
		paper.^^";
	print (ESB) ">X PACKAGE^";
	print "The package is wrapped and bears a label indicating that it contains over a pound
		of Keen's farmhouse cheddar.^^";
	print (ESB) ">SMELL PACKAGE^";
	print "The scent is subtle and delicious: this is not an aggressively scented cheese.^^";
	print (ESB) ">L^^";
	print (ESB) "Cheese Counter, Artisanal Restaurant^";
	print "You are standing before a display counter full of assorted cheeses: Colston
		Bassett Stilton; both Keen's and Montgomery cheddar; petit Basque; Sonoma Dry
		Jack; Drunken Goat cheese...  It is a cheese-lover's dream.^^";
	print "A clerk, dressed in a white uniform, presides behind the counter.^^";
	print "To the east is the cheese vault; the rest of the restaurant is to the south.^^";
	print (ESB) ">";
	if (ES_Pause()) jump end;
	print (ESB) "SOUTH^^";
	print (ESB) "Among the Tables^";
	print "The restaurant is a large, open room with white walls and high ceilings. 
		Nonetheless, the number of patrons makes it close going among the tables,
		especially when you must avoid waiters bearing pots of hot fondue.^^";
	print "North is the cheese counter.  To the south is the door to the street.^^";
	print (ESB) ">OPEN PACKAGE^";
	print "Not here: it'd probably get mixed up with someone's order.^^";
	print "One of the patrons jostles you on his way to the bathroom.^^";
	print (ESB) ">X PATRONS^";
	print "At the closest table, an older gentleman is consuming cheese puffs at a truly
		alarming rate.^^";
	print (ESB) ">S^";
	print "You take a step towards the doors, but are blocked by a tall man with broad
		shoulders.^^";
	print (ESB) ">G^^";
	print "You take a step towards the doors, but are blocked by a tall man with broad
		shoulders.^^";
	print (ESB) ">";
	if (ES_Pause()) jump end;
	print (ESB) "MAN, SOUTH^";
	print "The tall man ignores your request.^^";
	print (ESB) ">ATTACK MAN^";
	print "You haul off and punch the tall man in the nose, which really gets his attention.^^";
 	print "This soon turns out to be a mistake, however: not only is he about twice your size, 
 		he's also a lawyer.^^";
	print (ESB) "*** You have been arrested for assault. ***^^";
	print (ESB) ">";
	if (ES_Pause()) jump end;
	print (ESB) "UNDO^";
	print "Last move undone.^^";
	print (ESB) ">";
	if (ES_Pause()) jump end;
	print (ESB) "ATTACK MAN WITH FORK^";
	print "You stab the tall man with the fondue fork.  He yelps and looks around; you feign
		innocence, gazing in another direction entirely.^^";
	print "His search for the culprit has left your path clear.^^";
	print (ESB) ">";
	if (ES_Pause()) jump end;
	print (ESB) "S^^";
	print (ESB) "Street Outside Artisanal^";
	print "You stand out on the sidewalk, jostled by passers-by.  The street is full of
		taxis.^^";
	print (ESB) ">";
	if (ES_Pause()) jump end;
	print (ESB) "OPEN PACKAGE^";
	print "You open the package, exposing a wedge of golden cheddar.^^";
	print (ESB) ">";
	if (ES_Pause()) jump end;
	print (ESB) "EAT CHEDDAR^";
	print "You lift the cheddar to your lips and nibble off a bit.  Mmm -- paradise!^^";
	print (ESB) "*** You have ascended to dairy-related bliss! ***^^";
	.end;
	print (ESI) "[Now leaving demo mode.]^^";
	ES_Pause();
	<<look>>;
];	


[ CoSSample;
	print (ESI) "[ At any Pause in this demo, type 'Q' to quit or any other key to continue. ]^^";
	print "Interactive fiction is a cross between story and game.  It puts you in an  
	environment or location, which it describes, usually with a title and description,  
	like this:^^"; 
	print (ESB) "Bedroom^^"; 
	print "The tiny bedroom of your tiny apartment, currently made to look even smaller than  
		usual by the clutter that fills the room.  Drawers stand open and objects cover  
		your bed, most prominently the trunk-sized suitcase now half-full.^^On top of your  
		dresser is a framed photograph of your mother when she was younger, and another  
		very old one of your father, standing on a beach somewhere in the south.^^";  
	print (ESI) "Now, you will see a prompt which looks like this: ";
	print ">"; 
	print (ESI) "^^This is your invitation to type  
		something.  A very common thing is to examine items in your environment.  It is generally  
		a good idea to look at everything that is mentioned as being in the room, so that  
		you don't miss anything important.  For instance:^^"; 
	print ">";
	if (ES_Pause()) jump end;   
	print (ESB) "EXAMINE THE SUITCASE^"; 
	print "It used to be your mother's, and hasn't been out of the closet since the two of  
		you went to visit your grandmother in Smirnisc when you were five years old.  The lock  
		seems still to be good, however, and the hinges still work.^^At the moment it is about  
		half full.^^";  
	print (ESI) "For brevity, you can generally leave out articles, and you can  
		also abbreviate important commands (more about this later.)  Thus, EXAMINE  
		may be abbreviated to X, like so:^^"; 
	print ">";
	if (ES_Pause()) jump end;   
	print (ESB) "X CLOTHES^"; 
	print "At the bottom of the suitcase, still wrapped in tissue paper, are your brand new  
		suit and shoes, ready for Peter's wedding.  Almost everything else in the case is  
		eminently unsuitable for the trip: neatly folded piles of the uniform grey trousers  
		and shirts which you wear for your work at the factory.  Remembering that you are going  
		south, you've left out the heavier of your sweaters; but that leaves you with half the  
		case unfilled and not much left to pack.^^";  
	print (ESI) 
		"Naturally, there are also many other things you can do.  Common actions include taking  
		and dropping objects, moving, turning, or pushing them; turning things on and off;  
		putting one object on top of or inside another; etc.^^"; 
	print ">";
		if (ES_Pause()) jump end;    
	print (ESB) "TAKE PHOTOGRAPH^"; 
	print "Which do you mean, the photograph of your mother or the photograph of your father?^^";  
	print (ESI) "Here you've requested the computer to do something, but it doesn't have enough  
		information.  You can clarify in this situation just by adding another word that  
		makes the distinction clear:^^"; 
	print ">";
		if (ES_Pause()) jump end;  
	print (ESB) "MOTHER^"; 
	print "Taken.^^";
	print ">";
	if (ES_Pause()) jump end;  
	print (ESB) "X MOTHER^"; 
	print "In the photograph, your mother is grinning fiercely at the camera during one of  
		the rare bright days of summer.  Even in the fair light and at that age, however, signs  
		of wear and hard work show, from the lines around her eyes to hardening of her hands.^^"; 
	print ">"; 
	if (ES_Pause()) jump end; 
	print (ESB) "PUT MOTHER IN SUITCASE^"; 
	print "You put the photograph of your mother in the suitcase.^^";
	print ">";
	if (ES_Pause()) jump end; 
	print (ESB) "CLOSE SIUTCASE^"; 
	print "You can't see any such thing.^^"; 
	print (ESI) "If you confuse the computer by misspelling a word, you can fix it without retyping  
		the whole command by using >OOPS followed by the correct form, like this:^^"; 
	print ">";
		if (ES_Pause()) jump end;   
	print (ESB) "OOPS SUITCASE^"; 
	print "Closed.^^";
	print ">"; 
	if (ES_Pause()) jump end; 
	print (ESB) "TAKE IT^"; 
	print "(the suitcase)^Taken.^^"; 
	print (ESI) "As you see, the computer is also sometimes to make assumptions about what you mean  
		when you use pronouns.^^Now, you've taken an object, so you're carrying something.  When  
		you want to find out what you're holding at the moment, you can type INVENTORY (or INV, or I):^^";
	print ">";
		if (ES_Pause()) jump end;   
	print (ESB) "I^"; 
	print "You are carrying:^  a suitcase (which is closed)^  a parka (being worn) 
		^  a pair of gloves (being worn)^^";
	print ">"; 
	if (ES_Pause()) jump end; 
	print (ESB) "TAKE OFF GLOVES^"; 
	print "You take off the gloves.^^";
	print ">";
	if (ES_Pause()) jump end; 
	print (ESB) "DROP GLOVES^"; 
	print "Dropped.^^"; 
	print (ESI) "Changes to the game environment like this are reflected when you look around:^^"; 
	print ">";
		if (ES_Pause()) jump end;   
	print (ESB) "LOOK^"; 
	print (ESB) "Bedroom^^"; 
	print "The tiny bedroom of your tiny apartment, currently made to look even smaller than  
		usual by the clutter that fills the room.  Drawers stand open.^^On top of your  
		dresser is a framed photograph of your father, standing on a beach somewhere in the south.^^ 
		You can see a pair of gloves here.^^";  
	print (ESI) "Moving around is accomplished via compass directions (N, S, E, W, NE, etc.) and UP and  
		DOWN (abbreviated U and D respectively).  When no doors are described in a room, IN or OUT  
		will sometimes work instead.^^";  
	print ">"; 
		if (ES_Pause()) jump end;   
	print (ESB) "OUT^";
	print (ESB) "Front Room^^"; 
	print "The dingy carpet and grey walls make this room unappealing, though recently you've  
		managed to spruce it up a little.  There are a couple of (mismatched but serviceable) armchairs  
		now, and a southern-style brass lamp.  The viewscreen covers most of one wall, in place   
		of the window that doesn't exist, and there is a desk shoved into one corner.^^Doors lead  
		south into your bedroom, west into the kitchen, and north to the outside.^^On the coffee table  
		are a letter from Peter, the remote control for your entertainment system, and your keys.^^";   
	print (ESI) 
		"Unlike graphical adventure systems, the text parser is capable of interpreting fairly  
		complicated commands involving multiple objects.^^"; 
	print ">";
	if (ES_Pause()) jump end;   
	print (ESB) "TAKE ALL FROM TABLE^"; 
	print "letter from Peter: Taken.^remote control: Taken.^keys: Taken.^^";
	print ">";
	if (ES_Pause()) jump end; 
	print (ESB) "OPEN SUITCASE.  PUT ALL BUT KEYS IN SUITCASE.  CLOSE SUITCASE.^"; 
	print "Opened.^^suitcase: You can't put the suitcase inside itself.^letter from Peter: You  
		put the letter from Peter into the suitcase.^remote control: You put the remote control  
		into the suitcase.^^Closed.^^";
	print ">"; 
	if (ES_Pause()) jump end;   
	print (ESB) "WEST^"; 
	print (ESB) "Kitchen^^"; 
	print "The people who designed this little pit of an apartment apparently considered eating  
		even less important than most other functions.  Accordingly, the 'kitchen' is fitted out  
		only with a cupboard and a rehydrator.  The front room is visible through the doorway to the  
		east.^^Veiled grey light sifts in through the skylight  
		in the ceiling, through  
		a layer of snow.  You would resent the added expense on your heating bill for all the  
		energy wasted by that opening if it weren't the only source of natural light in the place.^^";  
	print ">";
	if (ES_Pause()) jump end;  
	print (ESB) "OPEN CUPBOARD^"; 
	print "Opening the cupboard reveals a box of crackers and a canister of industrial-strength  
		cleanser.^^";
	print ">"; 
	if (ES_Pause()) jump end; 
	print (ESB) "TAKE CRACKERS.  EAT CRACKERS.^"; 
	print "Taken.^^You crunch down a couple of crackers.  Not much good by themselves, on the  
		whole.^^"; 
	print (ESI) "At this point, you have done enough that you might not want to have to do it all over  
		if something went wrong.  To make a saved file, just type >SAVE.  If you want to continue  
		from that point again later, fetch it back with >RESTORE, and your game system will prompt  
		you to select a save-file.^^Or you could try living dangerously...^^>";  
	if (ES_Pause()) jump end;  
	print (ESB) "EAT CLEANSER^"; 
	print "(first taking the industrial-strength cleanser)^Taken.^^You scarf down the cleanser.   
		Almost immediately the unfortunate effects begin to manifest themselves...^^"; 
	print (ESB) "   *** YOU HAVE POISONED YOURSELF ***^^"; 
	print "Do you wish to RESTART, RESTORE, UNDO or QUIT? >";  
	if (ES_Pause()) jump end; 
	print (ESB) "UNDO^^"; 
	print "Previous turn undone.^^"; 
	print (ESI) "In point of fact, there are almost no actions allowed in this game that will actually  
		kill you, and few that will make the game unfinishable.  But you may still want to make  
		saved copies of the game so that you can go back later.  Undo is also useful if you make  
		a decision that you regret, and can be used at any time, even if you have not committed  
		a fatal error.^^"; 
	print ">";
	if (ES_Pause()) jump end;  
	print (ESB) "S^"; 
	print "You can go only east from here.^^"; 
	print (ESI) 
		"If you ever try to go a direction that's not allowed, the game will tell you which  
		ways you are allowed to go.^^";  
	print ">";
	if (ES_Pause()) jump end; 
	print (ESB) "E^"; 
	print (ESB)  "Front Room^^"; 
	print "The dingy carpet and grey walls make this room unappealing, though recently you've  
		managed to spruce it up a little.  There are a couple of (mismatched but serviceable) armchairs  
		now, and a southern-style brass lamp.  The viewscreen covers most of one wall, in place   
		of the window that doesn't exist, and there is a desk shoved into one corner.^^Doors lead  
		south into your bedroom, west into the kitchen, and north to the outside.^^"; 
	print (ESI)
		"Note that at the moment, the game shows you the description of a room in full, even if  
		you have already seen that description.  If you decide that you would rather keep things  
		brief, you may type >BRIEF to get only short descriptions of the movable objects in a room  
		when you return to one you have already seen.^^"; 
	print ">";
	if (ES_Pause()) jump end; 
	print (ESB) "N^"; 
	print "You can't very well leave without your travelling papers and ticket.^^"; 
	print ">";
	if (ES_Pause()) jump end; 
	print (ESB) "X DESK^"; 
	print "The desk is the only thing one could call an heirloom around here.  The external panels  
		are wood, but the rolltop is fitted with an excellent lock and lined with fire-retardant  
		material.  In the absence of a safe, this is where you keep your most important possessions.^^"; 
	print ">";
	if (ES_Pause()) jump end; 
	print (ESB) "OPEN DESK^"; 
	print "It appears to be locked.^^";	 
	print ">";
	if (ES_Pause()) jump end; 
	print (ESB) "UNLOCK DESK WITH KEYS^"; 
	print "You unlock the desk and roll open the rolltop, revealing your travelling papers and  
		identicard, among a stack of other papers: paid bills, membership in the assembly union,  
		correspondence from Peter from the few occasions he's resorted to letters rather  
		than electronic mail.  And the train ticket that he sent you for this journey.^^";	 
	print ">";
	if (ES_Pause()) jump end; 
	print (ESB) "TAKE ALL FROM DESK.^"; 
	print "You take the papers from the desk.  Looks like you're about ready to go, then.^^";
	print ">";
	if (ES_Pause()) jump end; 
	print (ESB) "N^"; 
	print "You give one last glance around the apartment.  It will be a couple of weeks before  
		you get back, at the soonest -- and the thought of your return depresses you instantly.   
		(Maybe you won't come back.)^^Then you open the door and walk out in search of the world  
		beyond Valodsci.^^"; 
	print (ESB) "  *** The adventure begins. ***^^^"; 
	.end; 
	print (ESI) "[Now leaving demo mode.]^^";
	ES_Pause();
	<<look>>;
];  
 
