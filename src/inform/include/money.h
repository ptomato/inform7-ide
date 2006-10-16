!! money.h version 970331
!! Copyright 1997 Erik Hetzner
!! This file may be copied, modified and distributed freely, so long
!! as this notice remains. If it is modified, I ask that you change
!! the name somehow to indicate that changes have been made, and give
!! both you and me credit. You may include this library in any game so
!! long as you give me credit somewhere.
!! -- What is this? --
!! This is `money.h', an Inform header file that simulates
!! money. It is reasonably simple (I hope!) from your point of view
!! (that is, the point of view of you, the person including this
!! file!). Nevertheless, it needs some clarification:
!!
!! * You need a number of objects of class MetaMoneyClass. I'm not
!!   sure what exactly to call these -- in America, they would be
!!   dollars and cents. The must have value, sing_name, plur_name,
!!   sing and plur properties.
!! * All coins and bills to be used must also be defined, of class
!!   MoneyClass. They too must have value, sing_name, plur_name, sing
!!   and plur properties.
!! * You must define a number of constants:
!!
!!   1. MONEY_TOTAL: Total the money if it's in the player's possession
!!   2. MONEY_TOTAL_ON_GROUND: Total the money even if it's not in the
!!      player's possession.
!!   3. MONEY_SHOW: If the money is being totaled, show
!!      the individual coins and bills.
!!   4. MONEY_SHOW_ON_GROUND: Same as above, but applies to the
!!      ground (not in the player's possession.)
!!   5. MONEY_TALL_LIST: Show a tall list of the money if it's
!!      appropriate.
!!
!! * Read the example file, `greenbacks.inf'. Hopefully this file
!!   should make everything absolutely clear. If not, please write me!
!! * By default, there are 25 dynamically creatable pieces of
!!   money. If this is not enough, change the definition of MoneyClass.
!! -- BUGS! BUGS! BUGS! --
!! When you find a bug in this code, please write to me so that I can
!! fix the thing! Squash squash, you know? And I'm sure this code is
!! full of it. Too damned complex not to be. :)
!!
!! -- Future Enhancements --
!! * The ability to use commands like `drop four dollars' and have the
!!   computer what bills and coins to drop.
!! * It would be nice to have a change function, and a buy verb that
!!   would allow things to actually be done with the money you
!!   collect. :)
!!
!! -- About the author --
!! give self ego;
!! Erik Hetzner (that's me) was born in San Francisco on 24 August
!! 1978. He has lived all over Northern California, and has been
!! playing Interactive Fiction since grade 4. He got re-involved with
!! IF about two years ago. His past header files are `timepieces.h'
!! and `printtime.h'. He is currently working on at least two pieces
!! of IF -- and entry for the 1997 IF contest, and a longer work for
!! release at sometime. He will soon release the interactive Inform
!! Game List, a neat bit of coding that simulates a library. He is
!! also fiddling with the possibilities of using literate programming
!! tools with Inform. He is currently an undergraduate at UC Berkeley,
!! and finds that he spends far too much time fiddling with Inform.
!! give self ~ego;
!!
!! -- Contacting me --
!! Write to me at egh@uclink4.berkeley.edu. If you can't do this,
!! write me at: 2601 Warring Street Box 297; Berkeley, CA
!! 94720-2288. Failing this, call me at [510] 664-3565. Failing this,
!! write me at: 127 Brookmead Court; San Anselmo, CA 94960-1471. If
!! all else fails, call me at [415] 456-2759 (though I won't be
!! there!). If this isn't enough to get ahold of me, I don't know what
!! could possibly be. :)

Property value;
Property quantity;

Attribute money;

Global money_defs;

Array MetaMoney --> 4;

Constant MONEY_TOTAL_BIT = 1;
Constant MONEY_TOTAL_ON_GROUND_BIT = 2;
Constant MONEY_SHOW_BIT = 4;
Constant MONEY_SHOW_ON_GROUND_BIT = 8;
Constant MONEY_TALL_LIST_BIT = 16;

Class 	MoneyClass(25)
 with	parse_name [ theWord quant x i;
	    if (action == ##TheSame)
    		return -1;
	    else {
		quant = self.quantity;
	    	x = TryNumber (wn);
	    	if (x ~= -1000) {
		    quant = x;
		    x = true;
		    i++;
		    theWord = NextWord();
	    	}
	    	else
		    x = false;
	    	theWord = NextWord();
	    	if (theWord == 'money') {
		    self.number = 0;
		    parser_action = ##PluralFound;
	    	}
!	   	else if (WordInProperty (theWord, Values-->1, plur)) {
!		    
!		    ;
!		}    
	   	else if (WordInProperty (theWord, self, plur)) {
		    self.number = quant;
	    	}
	    	else if (WordInProperty (theWord, self, sing)) {
		    if (x == true)
		    	self.number = quant;
		    else
		    	self.number = 1;
	    	}
	    	else 
		    return 0;
	    	i++;
	    	return i;
	    }
	],
    	list_together [;
	    switch (inventory_stage) {
	     1:
		if (((Descendant (self, player) == true) &&
		     ((money_defs & MONEY_TOTAL_BIT) ~= 0)) ||
 		    ((parent (self) ~= player) &&
	     	     ((money_defs & MONEY_TOTAL_ON_GROUND_BIT) ~= 0)))
		{
		    print (TotalMoney) parent (self);
		} else
		    print "some money";
		if (((money_defs & MONEY_SHOW_BIT) ~= 0) &&
		    (Descendant (self, player) == true)) {
		    if ((money_defs & MONEY_TALL_LIST_BIT) ~= 0) {
			if ((c_style & NEWLINE_BIT) ~= 0)
			    	print ":^";
			    else
		    	    	print " (";
			rfalse;
		    }
		    else {
			if ((c_style & NEWLINE_BIT) ~= 0) {
			    print " (";
			    c_style = ENGLISH_BIT + FULLINV_BIT +
				RECURSE_BIT;
			}
			else
			    print " (";
			rfalse;
		    }
		}
		if (((money_defs & MONEY_SHOW_ON_GROUND_BIT) ~= 0) &&
		    (Descendant (self, player) == false)) {
		    print " (";
		    c_style = ENGLISH_BIT + FULLINV_BIT +
			RECURSE_BIT;
		    rfalse;
		}
	     	if (Descendant (self, player) == true)
		    print "^";
		rtrue;
	     2:
		if (((c_style & NEWLINE_BIT) == 0) &&
		    ((Descendant (self, player) == true) &&
		     ((money_defs & MONEY_TOTAL_BIT) ~= 0)) ||
		    ((Descendant (self, player) == false) &&
		     ((money_defs & MONEY_TOTAL_ON_GROUND_BIT) ~= 0)))
		    print ")";
	    }
	],
  	article [;
	    print (MonQuant) self;
	],
	short_name [;
	    print (MonSingOrPlur) self;
	    rtrue;
	],
	create [;
	    StartDaemon (self);
	],
	destroy [;
	    remove self;
	    StopDaemon (self);
	],  
	daemon [;
	    CombineMoney (self);
	    self.number = 0;
	],
	before [ x;
	    if (self.number == 0)
		self.number = self.quantity;
	    else if (self.number > self.quantity) {
		print "(There aren't that many ", (MonPlur) self,
		    ", but I'll use as many as I can.)^";
		self.number = self.quantity;
	    }
	    else if (self.quantity ~= self.number) {
		if (SplitMoney (self, self.number) == false)
		    rtrue;
	    }
	 Take:
	    objectloop (x in player)
		if (x ofclass MoneyClass)
		    if (x.value == self.value) {
			x.quantity = x.quantity + self.quantity;
			self.destroy ();
	    		"Taken.";
		    }
	    move self to player;
	    "Taken.";
	 Drop:
	    if (noun notin player)
		"You're not carrying any ", (MonPlur) noun, ".";
	    objectloop (x in Location)
		if (x ofclass MoneyClass)
		    if (x.value == self.value) {
			x.quantity = x.quantity + self.quantity;
			self.destroy ();
			"Dropped.";
		    }
	    move self to Location;
	    "Dropped.";
	],
	sing_name,
	plur_name,
	sing,
	plur,
	value,
	quantity,
	number,
 has 	money;

Class 	MetaMoneyClass
 with	sing_name,
	plur_name,
	sing,
	plur,
	value;

[ RSortTableByProp table prop i temp;
    for (i = 1: (i - 2) ~= table-->0: i++) {
 	if ((table-->i).prop < (table-->(i+1)).prop) {
    	    temp = table-->i;
	    table-->i = table-->(i+1);
	    table-->(i+1) = temp;
	    i = 1;
	}
    }
];

[ CombineMoney the_object x;
    if (the_object.quantity == 0) {
	remove the_object;
	StopDaemon (the_object);
    }
    for (x = child (parent (the_object)): x ~= nothing :
	 x = sibling (x)) {
	if ((x ofclass MoneyClass) && (x ~= the_object) &&
	    (x.value == the_object.value)) {
	    x.quantity = (the_object.quantity + x.quantity);
	    the_object.destroy ();
	}
    }
];

[ MonQuant theObject;
    if (theObject.quantity == 1)
	print "a";
    else
	print (number) theObject.quantity;
];

[ MonPlur theObject;
    print (string) theObject.plur_name;
];

[ MonSing theObject;
    print (string) theObject.sing_name;
];

[ MonSingOrPlur theObject;
    if (theObject.quantity == 1) 
	print (MonSing) theObject;
    else if (theObject.quantity > 1)
	print (MonPlur) theObject;
];	   

![ MonIsOrAre theObject;
!    if (theObject.quantity > 1)
!	print "are";
!    else
!	print "is";
!];    

[ TotalMoney parentObj total x y z object;
    objectloop (x in parentObj)
	if (x ofclass MoneyClass)
    	    total = total + (x.value * x.quantity);
    y = total;
    for (x = 1 : x <= MetaMoney-->0 : x++) {
	object = (MetaMoney-->x);	
	z = (y/object.value);
	if (z == 0)
    	    ;
	else {
	    if (z > 1)
	    	print (number) z, " ", (MonPlur) object;
	    else
	    	print (number) z, " ", (MonSing) object;
	    y = y%(object.value);
	    if ((x) ~= (MetaMoney-->0)) {
	    	if ((x+1) == (MetaMoney-->0))
 		    print " and ";
	    	else
	    	    print ", ";
	    }
	}
    }
    return total;
];

![ ListMoney parentObj x y z;
!    y = 0;
!    z = 0;
!    objectloop (x in parentObj)
!	if (x ofclass MoneyClass)
!	    y++;
!    for (x = child (parentObj): x ~= nothing: x = sibling (x)) {
!	if (x ofclass MoneyClass) {
!	    z++;
!   	    print (MonQuant) x, " ", (MonSingOrPlur) x;
!    	    if (z ~= y) {
!	    	if (z+1 == y)
! 		    print " and ";
!	    	else
!    		    print ", ";
!	    }
!	}	
!    }
!];    

[ SplitMoney origObj quant x;
    objectloop (x ofclass MoneyClass) {
	if (parent (x) == nothing) {
	    move x to parent (origObj);
	    MoneyClass.copy (x, origObj);
	    x.quantity = origObj.quantity - quant;
	    origObj.quantity = quant;
	    x.create ();
	    if (x.quantity < 1)
		remove (x);
	    rtrue;
	}
    }
    if (MoneyClass.remaining () > 0) {  
	x = MoneyClass.create ();
	MoneyClass.copy (x, origObj);
	x.quantity = (origObj.quantity - quant);
	origObj.quantity = quant;
	move x to parent (origObj);
	if (x.quantity < 1)
	    remove (x);
	rtrue;
    }
    print "^money.h: A very bad thing has happened. There are not \
	enough separate money objects. Consolidate your money or \
	report this error to the game programmer.^";
    rfalse;
];

[ Descendant childObj parentObj tempObj;
    for (tempObj = parent (childObj):tempObj ~= nothing:
	 tempObj = parent (tempObj)) {
       	if (tempObj == parentObj)
	    rtrue;
    }
    rfalse;
];

[ InitMoney i x;
    money_defs = 0;
    #ifdef MONEY_TOTAL;
    money_defs = money_defs + MONEY_TOTAL_BIT;
    #endif;
    #ifdef MONEY_TOTAL_ON_GROUND;
    money_defs = money_defs + MONEY_TOTAL_ON_GROUND_BIT;
    #endif;
    #ifdef MONEY_TALL_LIST;
    money_defs = money_defs + MONEY_TALL_LIST_BIT;
    #endif;
    #ifdef MONEY_SHOW;
    money_defs = money_defs + MONEY_SHOW_BIT;
    #endif;
    #ifdef MONEY_SHOW_ON_GROUND;
    money_defs = money_defs + MONEY_SHOW_ON_GROUND_BIT;
    #endif;
    objectloop (x ofclass MetaMoneyClass) {
	i++;
	MetaMoney-->0 = i;
	MetaMoney-->i = x;
    }
    RSortTableByProp (MetaMoney, value);
    objectloop (x ofclass MoneyClass)
    {
	if (x.quantity > 0)
    	    x.create ();
	else
	    x.destroy ();
    }      
];


