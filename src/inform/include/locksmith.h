!-----------------------------------------------------
!	Models locks and keys
!
!	Verbs: 
!		New: DefaultLock, DefaultUnlock.  (From the player's point of view, no difference.)
!		Replaced: None
!	
!	Features:
!		Key class which makes it easier for the parser to choose objects for lock and
!			unlock, and which also remembers (once it's been used) what it opens.
!		Debugging verb UnlockAll unlocks everything in the game.
!
!	Resource cost: 
!		No attributes. 
!
!	Special Instructions:
!		After including grammar, insert the following two lines into your code:
!
!		Extend 'lock' first * noun=DefaultLockTest -> DefaultLock;
!		Extend 'unlock' first * noun=DefaultLockTest -> DefaultUnlock;  
!
!-----------------------------------------------------

Global assumed_instrument; 

Class Key,	 
	with invent [ ;
		if (inventory_stage == 2 && self.key_to ~= 0)
		{	print " (which opens ", (the) self.key_to, ")";
		}
	],
	react_after [;
		lock, unlock: if ((self == second) && (self.key_to==0))
			self.key_to = noun;
	],
	name "key" "keys//p", 
	list_together "keys",
	plural "keys",
	key_to 0;

[ DefaultLockSub leftplace;
	if (assumed_instrument == 0)
	{	leftplace = parent(noun.with_key);
		if (leftplace == nothing or 0) 
		{	print "You seem to have lost the key.^"; rtrue;
		}
		else
		{	print "But you left the key to ", (the) noun, 
			" in ", (the) leftplace, ".^";
			rtrue;
		}
	}
	print "(with ", (the) assumed_instrument, ")^"; 
	<<Lock noun assumed_instrument>>;
];
[ DefaultUnlockSub leftplace;
	if (assumed_instrument == 0)
	{	leftplace = parent(noun.with_key);
		if (leftplace == nothing or 0) 
		{	print "You seem to have lost the key.^"; rtrue;}
		else
		{	print "But you left the key to ", (the) noun, 
			" in ", (the) leftplace, ".^";
			rtrue;
		}
	}
	print "(with ", (the) assumed_instrument, ")^"; 
	<<Unlock noun assumed_instrument>>;
];

[ DefaultLockTest i count;
	if (noun hasnt lockable) rfalse;
	if (noun provides with_key && noun.with_key.key_to == noun) 
	{	if (noun.with_key notin player)
		{	assumed_instrument = 0; rtrue;
		}
		else 
		{	assumed_instrument = noun.with_key;
			rtrue;
		}
	}
	objectloop (i in player && i ofclass Key)
	{	if (i.key_to == 0) 
		{ count++; assumed_instrument = i; }
	}
	if (count==1) rtrue; rfalse;
];


#ifdef DEBUG;
Verb 'unlockall' * -> UnlockAll;
[ UnlockAllSub obj;
	objectloop(obj has locked) give obj ~locked;
	"All locks unlocked.";
];
#endif;
