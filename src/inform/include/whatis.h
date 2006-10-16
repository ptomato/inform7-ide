! WhatIs    - Code to add "what is <object>" type commands
!             Copyright 1996 A.C. Murie
!             Version 1.0 (Nov 19, 1996)
!
!             This code may be freely distributed and used in any
!             program, commercial or otherwise. It would be nice if you
!             give me some credit, but it is not required. Feel free to
!             modify it if you want.
!
!             If you want to contact me, sourcerer@starbug.southern.co.nz
!             will be valid, at least until late 1997.
!
!             "Bugs?  They aren't bugs! They're undocumented features!"  :)
!
! Purpose   - The use can type "what is an <object>" and be given
!             information about it. This is a simple way to make background
!             information to the player without unwanted explanatory text
!             in the game.
!
!             As an example, Zork has a "what is a grue" style of command.
!
!             Additionally, "who am I" can be used to show who the player
!             is. This prints the player's short_name. This is of
!             particular use in games where the player can change
!             character, or plays a pre-defined character.
!
! Usage     - Include this file after parser.h
!
!             Give an object the 'queryable' attribute and the 'whatisit'
!             property. 'whatisit' may be a string or a routine to print
!             a description of the object.
!
!             Any object that is in scope may be queryed
!
!             Additionally, objects that do not appear in the game may be
!             stored in the object 'QueryObjs'. They will be in scope
!             all the time, and may be used for general game concepts
!
!             As a final option, the action WhatIs may be used in an
!             objects 'before' code to print the whatis text.
!
!             Any query for an object that is not in scope, regardless of
!             wether it actually exists or not. Thus, a query will not show
!             if an object exists, or is merely not available at that time.
!
! In game   - A player can type text in this format :
!             ["what"|"who"]  ["is"|"am"|"are"] <object>
!
!             eg: "who am I"
!                 "what is a disintegrator"
!                 "what are bombs"
!                 "what is the big green door"
!
! Restriction - A question mark must have a space between it and the last
!               word of the sentence, or be part of the objects 'name'
!               property.

Attribute queryable;
property  whatisit;

[ IsAmAre w; w=NextWord(); if (w=='is' or 'am' or 'are') return 0; return -1; ];
[ WhatIsWhatSub; "What is what?"; ];
[ QueryTopic; while( NextWordStopped() ~= -1 ) ; return 1; ];
[ WhatisQSub; "Good question."; ];

[ WhoAmISub;
    print "You are ";
    if( ZRegion(player.short_name) == 2 or 3 )
    {
        PrintOrRun(player,short_name,1);
        rtrue;
    }
    "yourself.";
];

[ WhatIsSub;
    if( noun == player ) <<WhoAmI>>;
    if( noun hasnt queryable ) "Good question.";
    if( ZRegion(noun.whatisit) == 2 or 3 )
    {
        PrintOrRun(noun,whatisit);
        rtrue;
    }
    "Good question.";
];

[ QueryScope;
    if( scope_stage == 1 ) rfalse;
    if( scope_stage == 2 )
    {
        ScopeWithin( QueryObjs );
        rfalse;
    }
];

Object QueryObjs "Queryable objects";

verb meta "who"  "what" 
           * IsAmAre                      -> WhatIsWhat
           * IsAmAre "I"                  -> WhoAmI
           * IsAmAre "I?"                 -> WhoAmI
           * IsAmAre "I" "?"              -> WhoAmI
           * IsAmAre scope=QueryScope     -> WhatIs
           * IsAmAre scope=QueryScope "?" -> WhatIs
           * QueryTopic                   -> WhatisQ;
