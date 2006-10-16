! ---------------------------------------------------------------------------- !
!   Scenic.h
!       corrected 3.1 Jul00 by Roger Firth and Stefano Gaburri
!       extended  3.0 Jul00 by Roger Firth and Stefano Gaburri (EXAMINE room)
!       extended  2.5 May00 by Roger Firth and Stefano Gaburri
!       extended  2.4 May00 by Roger Firth and Stefano Gaburri
!       extended  2.3 May00 by Roger Firth (compatibility with glulx)
!       extended  2.2 Mar00 by Roger Firth
!       extended  2.1 Mar00 by Roger Firth with input from Paul E Bell
!       reworked  2.0 Mar00 by Roger Firth for Inform 6
!       extended  1.1 Sep95 by Joe Mason
!       original  1.0 Jan95 by Richard Barnett for Inform 5
!
! ---------------------------------------------------------------------------- !
!   Installation: add the line:
!
!       Include "Scenic";
!
!   near the end of your game AFTER you've used the Scenic property,
!   but BEFORE the Include "Grammar" statement. Optionally, precede it with:
!
!       Global ScenicFlag 0 [+ 1] [+ 2] [+ 4];
!
!   to control end-of-turn processing and examination of rooms:
!       0 = both "EXAMINE swd" and "OTHERVERB swd" count as 'a turn'
!           (where 'swd' is a dictionary word defined in a Scenic property);
!           "EXAMINE room" is handled traditionally.
!      +1 = "OTHERVERB swd" doesn't count as 'a turn'.
!      +2 = "EXAMINE swd" doesn't count as 'a turn'.
!      +4 = "EXAMINE room" outputs the room's description.
!
!   Also optional is control over the "That's just scenery." message produced
!   when you do anything _other_ than EXAMINE|CONSULT|LOOK AT a bit of scenery.
!   You can precede the Include line with something like:
!
!       Constant ScenicError "Stop molesting the scenery!";
!
!   (to use your own message) or
!
!       [ myScenicError; switch(random(3)) {
!           1: "That's just scenery.";
!           2: "Stop molesting the scenery!";
!           default: "No way."; } ];
!       Constant ScenicError myScenicError;
!
!   (to add flexibility through your own routine) or simply
!
!       Constant ScenicError NULL;
!
!   (to output the traditional "That's not something you need to...".)
!
!   CAUTION: The package implements a version of the ParserError() Entry Point.
!   If you already have this defined, perhaps in another Included package, the
!   (misleading) compiler error is: Expected routine name but found ParserError
!
! ---------------------------------------------------------------------------- !
!   This package enables you to EXAMINE rooms and scenic non-objects.
!   These two features are independent (but closely related).
!
!   1. ROOM descriptions
!
!   A standard way of avoiding nonsensical "You can't see any such thing."
!   responses is by adding NAME properties to room objects. For example,
!   you might code a Forest room using this technique:
!
!       Object  forest "Forest"
!         with  description "The forest...",
!               name 'forest' 'wood',
!               etc... ;
!
!   Now, EXAMINE THE FOREST will elicit the friendlier response "That's not
!   something you need to refer to in the course of this game." This package
!   goes one better: once you enable the feature, EXAMINE THE FOREST will then
!   print the room's description (much as though you'd typed LOOK, though
!   without mentioning other objects in the room).
!
!   2. SCENIC descriptions
!
!   It's likely that the forest's description will mention trees, shrubs and
!   so on, and equally likely that the player will want to EXAMINE them as well.
!   Scenic non-objects provide a middle ground between the bare existence
!   of a word you can type and receive a "That's not something you need to..."
!   message, and the trouble of creating a separate scenery object whose only
!   interesting feature is that it has a description.
!
!   That is, rather than the very minimalistic:
!
!       Object  forest "Forest"
!         with  description "The forest...",
!               name 'forest' 'wood' 'tree' 'trees' 'shrub' 'shrubs' 'moss',
!               etc... ;
!
!   or the somewhat over-engineered:
!
!       Object  forest "Forest"
!         with  description "The forest...",
!               name 'forest' 'wood',
!               etc... ;
!       Object  -> "Trees"
!         with  description "The trees tower over you.",
!               name 'tree' 'trees',
!         has   scenery;
!       Object  -> "Shrubs"
!         with  description "The shrubs don't.",
!               name 'shrub' 'shrubs',
!         has   scenery;
!       Object  -> "Moss"
!         with  description "The moss cowers under everything.",
!               name 'moss',
!         has   scenery;
!
!   you can now include this package and then say:
!
!       Object  forest "Forest"
!         with  description "The forest...",
!               name 'forest' 'wood',
!               scenic  'tree' 'trees' 0 "The trees tower over you."
!                       'shrub' 'shrubs' 0 "The shrubs don't."
!                       'moss' 0 "The moss cowers under everything.",
!               etc... ;
!
!   or, to vary or randomize your scenic descriptions:
!
!       [ myDescribeTrees;  ... some code to print a description ...; ];
!       [ myDescribeShrubs; ... some code to print a description ...; ];
!       [ myDescribeMoss;   ... some code to print a description ...; ];
!       Object  forest "Forest"
!         with  description "The forest...",
!               scenic  'tree' 'trees' 0 myDescribeTrees
!                       'shrub' 'shrubs' 0 myDescribeShrubs
!                       'moss' 0 myDescribeMoss,
!               etc... ;
!
!   or, finally, to output the traditional "That's not something you need to..."
!
!       Object  forest "Forest"
!         with  description "The forest...",
!               name 'forest' 'wood',
!               scenic  'tree' 'trees' 'shrub' 'shrubs' 'moss' 0 NULL,
!               etc... ;
!
!   3. SUMMARY
!
!   Trust us: this stuff is simpler than it looks. We suggest that you might:
!   - set ScenicFlag to 4 to enable the EXAMINE room feature;
!   - use the NAME property of a room _only_ for that room's name/synonyms;
!   - use the SCENIC property with strings for secondary text about items
!     mentioned in the room's description;
!   - use the SCENIC property with NULL for tertiary text about items mentioned
!     in those strings. For example:
!
!       Object  forest "Forest"
!         with  description
!                   "The forest seems to stretch in every direction. Around you
!                   an almost impenetrable tangle of bush and shrub silently
!                   struggles for possession of the mossy ground, while overhead
!                   the trees rise gaunt and limbless through the dank sour air.",
!               name 'forest' 'wood',
!               scenic
!                   'tree' 'trees' 'gaunt' 'limbless' 0 "Streaked with lichen,
!                     gnarled and knotted, the trees offer no support below
!                     the branching canopy dozens of feet above you."
!                   'bush' 'bushes' 'shrub' 'shrubs' 'tangle' 0 "Thorn and
!                     bramble thrust spiteful barbs in angry coils."
!                   'moss' 'ground' 0 "The only bright colour in this dismal
!                     place, the dark emerald moss billows soft and sodden
!                     around your feet."
!                   'air' 'dank' 'sour' 0 "Almost dense enough to grasp, the
!                     heavy atmosphere piles oppressively down on you."
!                   'sky' 0 "No trace of the sky is visible through the leaves."
!                   'lichen' 'branch' 'branches' 'canopy' 'thorn' 'thorns'
!                   'bramble' 'brambles' 'barb' 'barbs' 'coil' 'coils'
!                   'colour' 'color' 'emerald' 'billows' 'leaf' 'leaves'
!                   'atmosphere' 0 NULL,
!               etc... ;
!
!
!
! ---------------------------------------------------------------------------- !
#ifdef SCENIC;
#ifdef DEBUG; message "Compiling Scenic.h"; #endif;

! Ensure compatibility with glulx, which uses four-byte words.
#ifndef WORDSIZE;
    Constant TARGET_ZCODE 0;
    Constant WORDSIZE 2+TARGET_ZCODE;   ! avoid compiler warning
#endif;

! Flags which control processing (described above).
#ifndef ScenicFlag;
Global  ScenicFlag = 0;
#endif;

! The message when you "OTHERVERB swd".
Default ScenicError "That's just scenery.";

[ GetScenicDesc obj swd x y z;
        ! If obj provides a 'scenic' property, check if it includes swd.
        ! If so, return the associated argument; if not; return false.
        if (~~(obj provides scenic && swd)) rfalse;
        x = obj.&scenic; y = x + obj.#scenic; z = 0;
        for ( : x<y : x=x+WORDSIZE) switch (z) {
            0:  if (0 == x-->0) z = z + 2; if (swd == x-->0) z++;
            1:  if (0 == x-->0) z = z + 2;
            2:  z = 0;
            3:  return x-->0;
            }
        rfalse; ];

Global  ScenicWord;
        ! A dictionary word which might be mentioned in a 'scenic' property.
        ! Once found, is reset to false to prevent further searching.

[ ScenicPrintOrRun x y;
        ! Print (if a string) or run (if a routine) the scenery/error message,
        ! or (if NULL) output the standard "That's not something you need to...".
        ! Perform normal Inform end-of-turn processing if required.
        ScenicWord = false;
        SetPronoun('it', NULL); SetPronoun('them', NULL);
        if (x == NULL) L__M(##Miscellany, 39);
        else { if (metaclass(x) == Routine) x(); else print (string) x, "^"; }
        if (~~(ScenicFlag & y)) InformLibrary.end_turn_sequence();
        ];

[ CheckScenicExamine obj x;
        ! If obj provides a 'scenic' property, check if it includes ScenicWord.
        ! If so, print/run the description.
        x = GetScenicDesc(obj, ScenicWord); if (x) {
            ScenicPrintOrRun(x, $0002);
            }
        ];

[ CheckScenicOther obj x;
        ! If obj provides a 'scenic' property, check if it includes ScenicWord.
        ! If so, print/run an appropriate response.
        x = GetScenicDesc(obj, ScenicWord); if (x) {
            if (x ~= NULL) x = ScenicError;
            ScenicPrintOrRun(x, $0001);
            }
        ];

[ Handle_CANTSEE_PE;
        ! For a CANTSEE error, sets ScenicWord to the last word on the line,
        ! then searches for that word in any 'scenic' properties of the
        ! location, and then of each object in scope. For an EXAMINE-like
        ! operation, outputs the scenic description.
        do ScenicWord = NextWordStopped(); until (ScenicWord == -1);
        wn = wn - 2; ScenicWord = NextWord();
        if (ScenicWord) {
            if (action_to_be == ##Examine or ##Search or ##Consult) {
                CheckScenicExamine(location);
                if (ScenicWord) LoopOverScope(CheckScenicExamine);
                }
            else {
                CheckScenicOther(location);
                if (ScenicWord) LoopOverScope(CheckScenicOther);
                }
            if (~~ScenicWord) rtrue;
            }
        rfalse;                     ! Error was NOT handled by this routine.
        ];

[ Handle_SCENERY_PE;
        ! For a SCENERY error (using a word from the NAME property of a room)
        ! due to an EXAMINE-like operation, outputs the room's description.
        if (ScenicFlag & $0004) {
            if (action_to_be == ##Examine or ##Search or ##Consult) {
                if (location.describe ~= NULL) RunRoutines(location, describe);
                else {
                    if (location.description == 0) RunTimeError(11, location);
                    else PrintOrRun(location, description);
                    }
                InformLibrary.end_turn_sequence();
                rtrue;
                }
            }
        rfalse;                     ! Error was NOT handled by this routine.
        ];

[ ParserError eType;
        ! Standard entry point, called by library routines after parse error.
        switch (eType) {
            CANTSEE_PE: return Handle_CANTSEE_PE();
            SCENERY_PE: return Handle_SCENERY_PE();
            default:    rfalse;     ! Error was NOT handled by this routine.
            }
        ];

! ---------------------------------------------------------------------------- !

[ RoomExOnSub;  ScenicFlag = ScenicFlag | $0004; "EXAMINE enabled for rooms."; ];
[ RoomExOffSub; ScenicFlag = ScenicFlag & $FFFB; "EXAMINE disabled for rooms."; ];
[ RoomExSub;    if (ScenicFlag & $0004) RoomExOffSub (); else RoomExOnSub (); ];

Verb meta 'roomex' *           -> RoomEx
                   * 'on'      -> RoomExOn
                   * 'off'     -> RoomExOff;

#endif;
! ---------------------------------------------------------------------------- !
