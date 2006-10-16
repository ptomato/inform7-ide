!---------------------------------------------------------------------------
!	NewbieGrammar.h, by Emily Short (emshort@mindspring.com) 
!
!	Being grammar statements and a couple of verbs to accomodate and 
!	respond to nonstandard kinds of input.  It interprets who and what
!	questions, as well as correcting some standard errors and redirecting
!	other questions to Help.
!
!	The current contents of this file are dictated by people's r*if posts about
!	their frustrations with early attempts to play IF, and on the messages
!	log on phpZork, which reveals what an assortment of random players
!	have been known to attempt.
!
!	Note that the general trend of these grammar lines are to catch things that
!	are not what one might consider "legitimate" IF commands, or could not
!	be translated easily into another IF action.  There is a separate library,
!	called ExpertGrammar, that provides some extra verb synonyms and equivalencies
!	-- many of which were also suggested by analysis of newbie commands.
!	The two libraries are separated because it is possible that one will want
!	the ExpertGrammar without all the extra encumbrances of this library file.
!
!	Version 0.6 -- still imperfect.  If you have suggestions about
!		more things the library should handle, please contact me to 
!		suggest them.
!
!	Thanks to Roger Firth for his corrections.
!
!	5/21/03
!
!
!
!	RIGHTS:
!	
!	This library file may be treated as public domain.  It may be
!	included with or without credit to the original author.  It may be
!	modified at the user's discretion.  It may be freely redistributed.  
!
!	CONTENTS:
!
!		Parsing for the following forms:
!
!		PLEASE... <anything> calls TooPolite, which tells the player such
!			forms are not necessary
!
!		I AM, YOU ARE, HE/SHE/IT IS... calls NewbieGrammar, which reminds
!			the player to use imperative verbs.
!
!		WHO AM I? calls MustXMe, which teaches the syntax for EXAMINE ME,
!			then executes that command.
!
!		WHO ARE YOU? calls ParserHelp, which explains the parser a bit.
!
!		WHERE AM I? calls MustLook, which teaches LOOK, then executes that
!			command.
!
!		WHERE DO I FIND MORE GAMES LIKE THIS (and a wide range of variants)
!			calls ArchiveIntro, which explains about the IF archive.
!
!		WHERE CAN I GO NOW? (and variants) calls Exits, which is defined
!			in this file just to tell the player to look around.  If you 
!			would like to improve the functionality, you might want to look
!			at one of the libraries that handles an EXITS command.
!
!		WHAT DO I DO NEXT? (and a wide range of variants) calls
!			Help.
!		
!		WALK TO (and variants) calls LocalGo, which tells you
!			that movement within a room is unnecessary.
!
!		GO BACK (and variants) calls NoReturn, which tells you
!			to be specific about which direction you want to return.
!
!		CHECK FOR, LOOK FOR (and variants) calls SpecificSearch, which
!			tells you to be specific about how to search for something
!
!		Moreover, there is a catch-all, omnipresent item, which
!		goes everywhere the player goes and reacts to
!		
!		-- attempts to use body parts to do things.  This is to counteract 
!			newbie commands such as
!			OPEN DOOR WITH FOOT, INFLATE RAFT WITH MOUTH, etc.
!		-- vague instructions, such as KILL SOMEONE or TAKE SOMETHING
!		-- vague locations such as HERE or ANYWHERE
!
!	POSSIBLE DIRECTIONS:
!
!	A couple of things one might also do if one really wanted to take this
!	further: 
!
!	--	Use the UnknownVerb entrypoint to try to catch cases where the player is
!		using a known noun to begin a sentence.  Call NewbieGrammar in this case.
!
!	-- 	Use ChooseObjects to make the allbodyparts item dispreferred in any
!		disambiguation
!
!	-- 	Use BeforeParsing to look for certain standard adverbs, or else modify
!		the "I only understood you as far as..." library message to remind the 
!		player that adverbs are not allowed.  Many of the defective commands
!		found in a study of newbie command structure included things such as
! 
!		>SWIM ANYWAY
!		>JUMP ACROSS
!		>HELP PLEASE
!
!	-- Implement a fully functioning "GO BACK" verb that would return the 
!		player to his previous location.  A. O. Muniz has already released
!		a library to do this with Platypus; with the standard Inform library
!		it is a bit more challenging, but not impossible.
!
!
!	INSTALLATION: 
!
!	Include "NewbieGrammar" after Grammar in your gamefile. 
!
!	Note that if you are actually using who, what, or where verbs in your
!	gave for legitimate purposes, you will need to change the grammar
!	lines accordingly.
!
!	Set a ChooseObjects to make sure that allbodyparts doesn't show up 
!	on commands like GET ALL.
!
!	It is also suggested that you Replace HelpSub before including this 
!	file.  What is here is pretty minimal.  If you wish to avoid compiling
!	the grammar lines for 'help' (perhaps because you plan to use something
!	else), define the constant NO_HELP_GRAMMAR before including this file.
!
!--------------------------------------------------------------------------- 
 
Object allbodyparts,
	with
	short_name [;
		print "that";
		rtrue;
	],
	number 0,
	source 0, 
	parse_name 
	[ i j w;
		for (::)
		{	
			j = 0; w = NextWord();
			if ((self.number == 0 or 1) && 
				((w == 'my' or 'head' or 'hands' or 'hand' or 'ear' or 'ears') ||
				 (w == 'fist' or 'fists' or 'finger' or 'fingers' or 'thumb' or 'thumbs') ||
				 (w == 'arm' or 'arms' or 'leg' or 'legs' or 'foot' or 'feet') ||
				 (w == 'eye' or 'eyes' or 'face' or 'nose' or 'mouth' or 'teeth') ||
				 (w == 'tooth' or 'tongue' or 'lips' or 'lip')))
			{	self.number = 1; j = 1;
			} 
			if ((self.number == 0 or 2) && 
				(w == 'someone' or 'something' or 'anyone' or 'anything')) 
			{	self.number = 2; j = 1;
			}   
			if ((self.number == 0 or 3) && 
				(w == 'here' or 'everywhere')) 
			{	self.number = 3; j = 1;
			}    
			if (j~=0)  i++; else return i;
		}
	], 
	found_in [;
		rtrue;
	],
	react_before [;
		if ((noun && noun == self) || (second && second == self))
		{	
			self.message(self.number);
			self.number = 0;
			rtrue;
		}
		rfalse;
	],
	message [ x;
		switch(x)
		{
		1:  "Generally speaking, there is no need to refer to your body
			parts individually in interactive fiction.  WEAR SHOES ON FEET
			will not necessarily be implemented, for instance; WEAR SHOES
			is enough.  And unless you get some hint to the contrary,
			you probably cannot OPEN DOOR WITH FOOT or 
			PUT THE SAPPHIRE RING IN MY MOUTH.";
		2:  "The game will not arbitrarily guess what you want.  Be specific --
			use a noun for an object you can see around you.";
		3: switch(action)
			{
				##Look, ##Search: <<look>>;
				default: "You don't really need to refer to places in the game
					this way.";
			}
		}
	],
	has proper scenery;
	
[ IsAmAre;
	if (NextWord()=='is' or 'am' or 'are' or 'was' or 'were') 
		return GPR_PREPOSITION;
	return GPR_FAIL;
]; 
[ DoCan;
	if (NextWord()=='do' or 'does' or 'would' or 'will' or 'shall' or
			 'can' or 'could' or 'should' or 'may' or 'must') 
		return GPR_PREPOSITION;
	return GPR_FAIL;
]; 

[ Anybody;
	if (NextWord()=='I//' or 'me' or 'he' or 'she' or 'it' or
		'we' or 'you' or 'they' or 'person' or 'one' or
		'someone' or 'somebody' or 'anyone' or 'anybody')
		return GPR_PREPOSITION;
	return GPR_FAIL;
];

[ More;
	if (NextWord()=='more' or 'other' or 'another' or 'others')
		return GPR_PREPOSITION;
	return GPR_FAIL;
];

[ SomeDirection w flag num;
	for(::)
	{
		w = NextWord();
		if (w==0) 
		{	
			if (flag)
			{	return GPR_PREPOSITION;
			}
			return GPR_FAIL;
		}
		if (w == 'left' or 'right' or 'straight' or 'ahead' or 'back'
			or 'backwards' or 'forward' or 'backward' or 'forwards' or 'on' 
			or 'onward' or 'onwards' or 'around') 
		{	flag = 1; num++;
		} 
	}
];			

[ InternalNouns w flag num;
	for(::)
	{
		w = NextWord();
		if (w==0) 
		{	
			if (flag)
			{	return GPR_PREPOSITION;
			}
			return GPR_FAIL;
		}
		if (w == 'song' or 'music' or 'songs') 
		{	flag = 1; num++;
		}
		if (w == 'thought' or 'idea' or 'concept') 
		{	flag = 1; num++;
		}
		if (w == 'the' or 'a' or 'some' or 'any')
		{	flag = 0; num++;
		} 
	}
];			

[ ThePoint w flag num;
	for(::)
	{
		w = NextWord();
		if (w==0) 
		{	
			if (flag)
			{	return GPR_PREPOSITION;
			}
			return GPR_FAIL;
		}
		if (w == 'point' or 'point?' or 'idea' or 'idea?' or 'goal' 
			or 'goal?' or 'purpose' or 'purpose?') 
		{	flag = 1; num++;
		}
		else
		{	if (w == 'the' or 'a')
			{	flag = 0; num++;
			}
			else 
			{	if (flag)
				{	return GPR_PREPOSITION;
				}
				return GPR_FAIL;
			}
		} 
	}
];	
[ IsThisGame w flag num;
	for(::)
	{
		w = NextWord();
		if (w==0) 
		{	
			if (flag)
			{	return GPR_PREPOSITION;
			}
			return GPR_FAIL;
		}
		if (w == 'this' or 'these' or 'this?' or 'these?') 
		{	flag = 1; num++;
		}
		if (w == 'kind' or 'kinds' or 'of' or 'sort' or 'sorts' or 'like'
			or 'such' or 'a' or 'as' or 'all' or 'is' or 'are')
		{	flag = 0; num++;
		}
		if (w == 'game' or 'games' or 'story' or 'stories'
			or 'game?' or 'games?' or 'story?' or 'stories?' or
			'interactive' or 'fiction' or 'text' or 'adventure'
			or 'adventures' or 'fiction?' or 'adventures?')
		{	flag = 1; num++;
		}
	}
];		

[ ThisGame w flag num;
	for(::)
	{
		w = NextWord();
		if (w==0) 
		{	
			if (flag)
			{	return GPR_PREPOSITION;
			}
			return GPR_FAIL;
		}
		if (w == 'this' or 'these' or 'this?' or 'these?') 
		{	flag = 1; num++;
		}
		if (w == 'kind' or 'kinds' or 'of' or 'sort' or 'sorts' or 'like'
			or 'such' or 'a' or 'as' or 'all')
		{	flag = 0; num++;
		}
		if (w == 'game' or 'games' or 'story' or 'stories'
			or 'game?' or 'games?' or 'story?' or 'stories?' or
			'interactive' or 'fiction' or 'text' or 'adventure'
			or 'adventures' or 'fiction?' or 'adventures?')
		{	flag = 1; num++;
		}
	}
];				
[ GetFind;
	if (NextWord()=='get' or 'find' or 'acquire') 
		return GPR_PREPOSITION;
	return GPR_FAIL;
];
[ HelpName;
	if (NextWord()=='help' or 'assistance' or 'instructions'
		or 'help?' or 'assistance?' or 'instructions?') 
		return GPR_PREPOSITION;
	return GPR_FAIL;
];
[ Meant; 
	if (NextWord()=='supposed' or 'meant' or 'intended') 
		return GPR_PREPOSITION;
	return GPR_FAIL;
];
[ NextThing;
	if (NextWord()=='now' or 'now?' or 'next' or 'next?'
		or 'here' or 'here?') 
		return GPR_PREPOSITION;
	return GPR_FAIL;
];
[ PlayUse;
	if (NextWord()=='play' or 'play?' or 'use' or 'use?' or 'operate' or 'operate?'
		or 'type' or 'type?' or 'do' or 'do?' or 'understand' or 'understand?'
		or 'learn' or 'learn?' or 'work' or 'work?') 
		return GPR_PREPOSITION;
	return GPR_FAIL;
];

Verb meta 'hello' 'hi' 'howdy' 'greetings'
				*								 -> HelloThere;

Verb meta 'you' 'he' 'she' 'it' 'they' 'we' 'its' 'theyre' 'were' 
	'youre' 'hes' 'shes'
				* topic							 -> NewbieGrammar; 
				
Verb meta 'who' 'wh' 'whos' 
				* IsAmAre 'I'/'I?' -> MustXMe
				* IsAmAre 'you'/'you?' -> ParserHelp
				* IsAmAre noun			-> Examine;
Verb meta 'where'  
				* IsAmAre 'I'/'I?'/'this'/'here'/'this?'/'here?' ->MustLook
				* IsAmAre noun	-> RightHere
				* IsAmAre ThePoint ThisGame 					-> BoredHelp
				* IsAmAre topic -> SpecificSearch
				* DoCan Anybody 'go'/'go?'		-> Exits
				* DoCan Anybody 'go' NextThing  -> Exits
				* DoCan Anybody GetFind ThisGame ->
					ArchiveIntro
				* DoCan Anybody GetFind More ThisGame ->
					ArchiveIntro
				* DoCan Anybody 'look' 'for' ThisGame ->
					ArchiveIntro
				* DoCan Anybody 'look' 'for' More ThisGame ->
					ArchiveIntro
				* DoCan Anybody GetFind HelpName -> OutsideHelp
				* DoCan Anybody GetFind HelpName 'with'/'for'/'on' ThisGame -> OutsideHelp ;
Verb meta 'wheres'  
				* 'here'			-> MustLook
				* IsAmAre ThePoint ThisGame 					-> Help;
Verb meta 'what' 
				* NextThing						-> Help 
				* IsAmAre 'I'/'I?'							-> MustXMe
				* IsAmAre 'you'/'you?'							-> ParserHelp
 				* IsAmAre 'here'/'here?'					-> MustLook
				* IsAmAre 'this'/'this?'					-> Help
				* IsAmAre Anybody Meant 'to' PlayUse -> Help
				* IsAmAre Anybody Meant 'to' PlayUse 
					NextThing -> Help
				* IsAmAre Anybody Meant 'to' PlayUse
					'in'/'on'/'for'/'with' ThisGame 		-> Help
				* IsAmAre noun								-> Examine
				* IsAmAre ThePoint								-> BoredHelp
				* IsAmAre ThePoint ThisGame 					-> BoredHelp
				* IsAmAre ThisGame 'about'/'about?'/'for?'/'for'		-> BoredHelp
				* IsAmAre ThisGame 'about'/'about?'/'for?'/'for'		-> BoredHelp
				* IsAmAre 'I' topic 						-> Help
				* IsThisGame					-> IntroHelp
				* DoCan Anybody PlayUse	-> VerbHelp
				* DoCan Anybody PlayUse NextThing	-> VerbHelp
				* DoCan Anybody PlayUse
					'in'/'on'/'for'/'with' ThisGame 		-> VerbHelp
				* IsAmAre ThisGame							-> IntroHelp;
Verb meta 'whats'
				* 'here'			-> MustLook
				* noun				-> Examine
				* ThePoint ThisGame 					-> BoredHelp;
Verb meta 'how'
				* DoCan Anybody PlayUse -> Help
				* IsAmAre Anybody Meant 'to' PlayUse -> Help
				* IsAmAre Anybody Meant 'to' PlayUse ThisGame -> Help
				* DoCan Anybody PlayUse ThisGame -> Help
				* DoCan Anybody GetFind ThisGame -> ArchiveIntro
				* DoCan Anybody GetFind More ThisGame -> ArchiveIntro
				* DoCan Anybody GetFind HelpName -> OutsideHelp
				* DoCan Anybody GetFind HelpName 'with'/'for'/'on' ThisGame -> OutsideHelp
				* DoCan ThisGame 'work'/'work?' -> OutsideHelp;
				
#ifndef NO_HELP_GRAMMAR;   !!! added
Verb 'help'		* 	-> Help
				* topic	-> Help;
#endif;   !!! added

Verb 'please' 'kindly'	* topic -> TooPolite;


Extend only 'i//' 
                * topic							 -> NewbieGrammar;
Extend 	'walk'		* SomeDirection 			-> NoReturn
					* 'back' noun=ADirection	 -> Go
					* 'around'/'about'/'away'
											-> VagueGo
					* 'on'/'over'/'across' noun		-> Enter
					* 'to'/'towards'/'around'/'past'/'under' noun
														-> LocalGo 
					* 'over'/'up'/'down' 'to'/'towards' noun
														-> LocalGo;
Extend 'turn'		* SomeDirection				 -> NoReturn;
Extend 	'climb'		* SomeDirection					 -> NoReturn
					* 'back' noun=ADirection	 -> Go;
Verb 	'keep'		* 'going'/'walking'/'heading'/'running'
									 noun=ADirection -> NoReturn
					* 'going'/'walking'/'heading'/'running'
									 noun=ADirection -> MustGo;
Verb 	'continue'	*  noun=ADirection -> MustGo
					* 'going'/'walking'/'heading'/'running' 
									noun=ADirection -> MustGo;
Verb 	'return' 'back'	*							 -> NoReturn
					* 'to' topic				 -> NoReturn;
					
Extend 	'check'		* 'for' noun				 -> Examine
					* 'for' topic				 -> SpecificSearch;
Extend 	'look'		* 'for' noun				 -> Examine
					* 'for' topic				 -> SpecificSearch;
Extend 	'search'	* 'for' noun				 -> RightHere
					* 'for' topic				 -> SpecificSearch;
Verb 	'find'		* noun						 -> RightHere
					* topic						 -> SpecificSearch;
Extend 	'wear'		* noun 'on' noun			 -> Wear;
Extend  'sing'		* InternalNouns				 -> InternalAccusative;
Extend 	'think'		* InternalNouns				 -> InternalAccusative;

[ TooPoliteSub;
	"The parser does not understand polite formulations such as 
	PLEASE LOOK AROUND NOW or KINDLY OPEN THE BOX.  Start your commands
	with an imperative verb and they will work better.";
];

[ InternalAccusativeSub;
	"In constructions like SING A SONG, the ~A SONG~ part is what is known
	as an internal accusative -- a direct object that is not actually necessary.
	In short: SING will do just as well.  The game has difficulty with
	parsing abstracts like ~a song~.";
];

[ MustGoSub;
	print "[Generally, it is necessary to phrase commands like this as a simple
		direction: GO NORTH, NORTH, etc., rather than KEEP GOING NORTH, HEAD BACK NORTH,
		etc.]^^";
	<<Go noun>>;
];

[ RightHereSub;
	print "", (The) noun, " is ";
	if (IndirectlyContains(player, noun)) "already in your possession!";
	"in plain sight!";
];

[ SpecificSearchSub;
	"If you want to look for something, try LOOK (to see
	the room as a whole); LOOK IN containers; LOOK UNDER large items;
	and SEARCH such items as piles and heaps.^^If you still can't
	find whatever it is, you're probably out of luck...";
];

[ NoReturnSub;
	"The game does not keep track of your path through the rooms, nor 
	your orientation within them.  Instead, you should rely on absolute
	compass directions.  If you
	wish to visit a new location or return to a previous one, you will have to
	type the directions to take you there -- NORTH, UP, etc.";
];

[ ParserHelpSub;
	"The voice with which you are communicating is the narrator of the
	game.^^[If you are having trouble with the game, try HELP.]";
];
[ NewbieGrammarSub;
	"If the game is not understanding you, try issuing your commands in the imperative:
	e.g., >THROW THE KNIFE, but not >I WOULD REALLY LIKE TO THROW THE KNIFE.  Chatty sentences
	such as >YOU ARE A VERY STUPID GAME will only prove themselves true.^^If you really
	feel that the game is looking for a word that is not a verb (as the solution
	to a riddle, eg.) try some variations, such as SAY FLOOBLE.";
];
[ MustLookSub;
	print "[You can do this in the future by typing LOOK, which is quicker and 
		more standard.]^";
	<<Look>>;
];
[ MustXMeSub;
	print "[You're the main character of the game.  Of course, the game author may
		have given you a description.  You can see this description in the future 
		by typing EXAMINE ME, which is quicker and more standard.]^^";
	<<Examine player>>;
];

Verb 'intro' * -> TotalIntro;
[ TotalIntroSub;
#ifdef BasicBrief;
	BasicBrief(); new_line;
#endif;
#ifdef StartingInstructions;
	StartingInstructions(); rtrue;
#endif;
	"This is a work of interactive fiction.  You should explore and try to do things.";
];
[ BoredHelpSub;
#ifdef StartingInstructions;
	StartingInstructions(); rtrue;
#ifnot;
	<<Help>>;
#endif;
];

[ OutsideHelpSub;
#ifdef OnlineHelp;
	OnlineHelp(); rtrue;
#ifnot;
	<<Help>>;
#endif;
];

Verb 'verbs' *	-> VerbHelp;
[ VerbHelpSub;
#ifdef StandardVerbs;
	StandardVerbs(); rtrue;
#ifnot;
	<<Help>>;
#endif;
];

Verb 'hint' 'hints' *	-> HintHelp;
[ HintHelpSub;
#ifdef StuckInstructions;
	StuckInstructions(); rtrue;
#ifnot;
	<<Help>>;
#endif;
];

[ ArchiveIntroSub;
#ifdef MoreGames;
	MoreGames(); rtrue;
#ifndef MoreGames;
	"You can find more games like this at the Interactive Fiction archive,
	http://www.ifarchive.org, and a guide to the archive at
	http://www.wurb.com/if/.";
#endif;
];

[ HelloThereSub;
	"Hi!^^If you are new to Interactive Fiction, you may want to type HELP.";
];

[ ExitsSub;
	"If you LOOK, you may notice some compass directions you can use to 
	move to new rooms.";
];

[ LocalGoSub;
	if (noun has door) <<enter noun>>;
	"There's no need to walk towards items that are already
	in your vicinity.";
]; 

Verb 'demo' * -> Demo;
[ DemoSub;
#ifdef CheeseSample;
	CheeseSample();
#ifnot;
	"No demo is available.";
#endif;
];

[ HelpSub;
	"If you would like a list of verbs you could try, type VERBS.^^If the problem is more
	that you don't know how to get going with the game, type INTRO.^^If you'd like a sample
	of a game being played, type DEMO.^^Finally, if you know what you want to achieve
	but can't figure out how to achieve it, type HINT.
	^^If you are having further difficulty with IF, try looking at some of the online help sites
	such as http://www.brasslantern.org.";
];

[ IntroHelpSub;
#ifdef BasicBrief;
	BasicBrief(); rtrue;
#ifnot;
	"This is a work of interactive fiction, in which you play the role of the 
	main character.  You interact by typing text at the prompt.";
#endif;
];
