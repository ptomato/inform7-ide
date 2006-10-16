!---------------------------------------------------------------------------
!	ExpertGrammar.h, by Emily Short (emshort@mindspring.com) 
!
!	Being grammar statements and a couple of verbs to extend the standard
!	library with some of the most-requested improvements.  The most useful
!	of these are equivalences that the standard library doesn't understand --
!	GET DOWN for EXIT, DIG IN for DIG.  The rest are just spare verb stubs,
!	provided (in many cases) because some other standard authoring language
! 	has these verbs and yours truly has had a fit of jealousy.  Others
!	result from an inspection of the command logs on phpZork.  Bizarre though
!	some of these may seem to a seasoned adventurer ('but that's not an
!	IF word!!'), they are attested in actual play scripts.  Someone out
!	there has tried to use them...
!
!
!	Version 0.6 -- still in progress.  Let me know if you have 
!		more suggestions.
!
!	5/21/03
!
!	Thanks to Daniel Lackey for his corrections.
!
!	RIGHTS:
!	
!	This library file may be treated as public domain.  It may be
!	included with or without credit to the original author.  It may be
!	modified at the user's discretion.  It may be freely redistributed. 
!	Parts may be extracted. 
!
!	CONTENTS:
!
!	I would summarize, but they're pretty self-explanatory.  Page through
!	and have a look.  It's quite likely that you'll want to replace some
!	stubs and/or fiddle with the grammar lines.
!
!	INSTALLATION: 
!
!	Include "ExpertGrammar" after Grammar in your gamefile.  
!---------------------------------------------------------------------------


Verb 	'hand' 'deliver' 	= 'give';
Verb 	'depart' 			= 'leave'; 
Verb 	'excavate'			= 'dig';
Verb	'carve'				= 'cut';
Verb 	'steal' 'grab' 'acquire' 'snatch'
							= 'take';
Verb	'place' 'stick'	'shove'	'stuff'
							= 'put';
Verb 	'yell'				= 'shout';
Verb 	'strike'			= 'hit';
Verb 	'board'				= 'enter';
Verb 	'toss' 'fling' 'hurl' 
							= 'drop';
Verb 	'melt' 'ignite' 'incinerate' 'kindle' 'parch' 'bake' 'toast'
							= 'burn';
Verb 	'see' 'view' 'observe'				
							= 'examine';
Verb 	'proceed'			= 'walk';
Verb 	'whistle' 'hum'		= 'sing';
Verb 	'injure'			= 'attack';
Verb 	'wander'			= 'walk';
							
							

Verb 'use' 'utilise' 'utilize' 'employ' 'try'
				* clothing 						 -> Wear 
				* edible 						 -> Eat
				* enterable 					 -> Enter
				* door							 -> Enter
				* switchable					 -> SwitchOn
				* openable						 -> Open
				* noun 							 -> Use
				* noun 'on'/'in'/'with' lockable		 -> TryKey
				* noun 'to'/'for' topic			 -> RephraseUse;  

Verb 	'bite'		* edible				-> Eat
					* noun					-> Attack;
Verb 	'descend'	*						-> GoDown
					* noun					-> Enter;
Verb 	'ascend'	* 						-> GoUp
					* noun					-> Enter;	
Verb 	'knock' 'bang' 'pound'
					* 'on' noun				-> KnockOn; 
Verb 	'kick'		* noun					-> Attack
					* 'in'/'down'/'through' noun		-> Attack; 
 
Verb 	'shake' 'rattle'
					* noun					-> Shake;
Verb 	'dance' 'foxtrot' 'tango' 'waltz' 
					* 						-> Dance;
Verb 	'xyzzy' 'plugh' 'plover'
					*						-> Xyzzy; 
Verb 	'extinguish'
					* noun					-> Extinguish;
Verb 	'untie' 'loosen' 'free' 'loose'
					* noun					-> Untie; 
Verb	'activate' 'start'	
					* 'over'/'again'				 -> Restart
					* noun					-> SwitchOn;
Verb	'stop' 'deactivate'
					* noun					-> SwitchOff;
Verb	'suicide' 	*						-> Quit;
Verb	'commit'	* 'suicide'				-> Quit;
Verb 	'reach'		* 'for'/'towards'/'to' noun -> Take;

Extend 'kill' first * 'me'/'myself'			-> Quit;
Extend 	'throw'		* 'away'				-> Drop;					
Extend  'yell' first *						-> Shout;
Extend 	'jump'		* 'on'/'onto' noun		-> Enter;
Extend 	'get' 	first	* 'down'				-> Exit;
Extend 'get' last
					* 'out' multi			-> Take
					* multiinside 'out' 'of'/'from' noun
											-> Remove;
Extend 	'look' 		* noun=ADirection       -> LookDir
					* 'around'/'about'		-> Look; 

Extend 	'dig'	first	
					* 						-> DigGround
					* 'with' noun			-> DigGround
					* 'in' noun				-> Dig
					* 'in' noun 'with' noun -> Dig;
Extend 'put'		* 'out' noun 			-> Extinguish;

Extend 	'go'		* 'down'/'up' noun		-> Enter
					* 'upstairs'			-> GoUp
					* 'downstairs'			-> GoDown;
Extend 'break'		* 'in'/'down'/'through' noun -> Attack
					* noun 'in'/'down'			-> Attack
					* 'in'/'down'/'through' noun 'with' noun -> Attack
					* noun 'in'/'down'	'with' noun -> Attack;
Extend 'blow'		* 'on'/'in'/'through' noun -> BlowThrough
					* 'out' noun			-> Extinguish;
Extend 'pray'		* 'for'/'to' topic			-> Pray;
Extend 'rub'		* noun 'on'/'onto'/'over' noun
											-> RubOn;
Extend 'show'		* 'inventory'/'invent'/'inv'/'possessions'
											-> Inv;
Extend 'set'		* multiheld 'down'			-> Drop
					* multiheld 'on' noun		-> PutOn;

[ DigGroundSub;
	if (noun) <<Dig d_obj noun>>;
	else <<Dig d_obj>>;
];

[ RubOnSub;
	"There's not much point in that.";
];

[ RephraseUseSub;
	"Try <VERB> with ", (the) noun, " or <VERB> <THING> with ", 
		(the) noun, ".";
];

[ GoDownSub;
	<<Go d_obj>>;
];

[ GoUpSub;
	<<Go u_obj>>;
];
[ UseSub;
	"It's not entirely clear what you intend.";
];

[ UntieSub;
	"It's not entirely clear what you intend.";
];

[ ExtinguishSub;
	if (noun has on && noun has switchable) 
		<<switchoff noun>>;
	"This has no effect.";
];

[ BlowThroughSub;
	"This has no effect.";
];

[ TryKeySub;
	if (second has locked) <<Unlock second noun>>;
	<<Lock second noun>>;
];

[ LookDirSub;
	"You see nothing unexpected in that direction.";	
];
  
[ DanceSub;
	"You've never been much of a dancer.";
];

[ ShakeSub;
	"Nothing happens.";
];

[ XyzzySub;
	"There's a listening sort of silence.";
];

[ ShoutSub;
	"~Yeeagh!~";
];

[ KnockOnSub;
	if (noun has door) "No one answers.";
	"Nothing happens.";
];
