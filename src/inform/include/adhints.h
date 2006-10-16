! -------------------------------------------------------------------------
! Adaptive Hints for Inform
! (c) 1995 Michael S. Phillips
! -------------------------------------------------------------------------
! Hints Library:  Mar 25, 1996
! Adaptive Hints library, release 0.92a (960325)
! -------------------------------------------------------------------------
! This module is (c) 1996 Michael S. Phillips, but it is freely usable.
! The author may be reached at: mike@lawlib.wm.edu  as of this release.
! -------------------------------------------------------------------------

! -------------------------------------------------------------------------
! A note about customizing the behavior of the hints system:
!     A large number of '#ifdef xxx;' '#endif;' pairs are in the file, in
! order for the author using this hints system to be able to dictate the 
! exact behavior of the hints system.
!     Ideally, the definition of these variables would be done by the 
! declaration of an appropriate Constant (e.g. 'Constant GIVEHINTSONCE;')
! before including AdHints.h into the game.  This way, the library can be
! swapped if upgraded in such a manner that it does not require editing
! in order to preserve the desired behavior.  Each meaningful definition
! (and its effect) is detailed below.
! -------------------------------------------------------------------------
! GIVEHINTSONCE: If defined, this will cause a set of hints to be given by
! the 'HINT' command once AND ONLY ONCE, and only the first time.
! Otherwise, the hint(s) will disappear and need to be viewed with 
! 'REVIEW'.  This is not particularly kind behavior, but it does match
! the behavior of adhint.t for TADS.  If this and NOHINTREVIEW are both
! defined, then a given set of hints will appear ONE TIME ONLY and be
! utterly unretrievable afterwards.
!Constant GIVEHINTSONCE;
! -------------------------------------------------------------------------
! NOHINTREVIEW: If defined, this disables the 'REVIEW' command.  Note that 
! this only disables the grammar for 'REVIEW', and that ReviewSub() can 
! still be called by the game if the author so desires (for instance, after 
! the game is over).
!Constant NOHINTREVIEW;
! -------------------------------------------------------------------------
! REVIEWGIVENONLY: If defined, this causes ONLY those hints which were 
! actually given to be shown by the 'REVIEW' command.
!Constant REVIEWGIVENONLY;
! -------------------------------------------------------------------------
! SHOWSOLVEDTAG: If defined, the string '(solved)' will appear after each
! puzzle when 'REVIEW'ing hints if the puzzle has been solved.
!Constant SHOWSOLVEDTAG;
! -------------------------------------------------------------------------
! HINTDEBUG: If defined, this will give extra internal information about
! what is going on at certain key points of the hints code.  You probably
! don't want to define this unless you're debugging this particular file.
!Constant HINTDEBUG; 
! -------------------------------------------------------------------------
! HINTDEBUGVERBS: If defined, this allows the various hint debugging verbs
! (allhints, allpuzzles) to be used.  If HINTDEBUG is defined, these will
! be available.
!Constant HINTDEBUGVERBS;
! -------------------------------------------------------------------------

#ifdef HINTDEBUG;
#ifndef HINTDEBUGVERBS;
Constant HINTDEBUGVERBS;
#endif;
#endif;

Attribute given alias visited;
Attribute solved alias open;
Attribute in_menu alias locked;

Property hint_check;
Property additive the_hints;

Global AH_hints_available = 1;

Global AH_num_pages = 0;
Global AH_current_page = 0;
Global AH_hints_per_page = 0;

! First, we define the 'hint' class
! When constructing a hint, make certain that it 'does the right thing' and
! is declared as such a class for appropriate defaults.
! A hint will have the 'general' attribute set if it is available, and the
! 'given' attribute (which is aliased to 'visited') if the hint has been
! used.  Only if it is 'given' or 'solved' (which is aliased to 'open') will 
! it appear with the REVIEW command.

Class HintClass
 has  proper
 with name "hint";

Object Hints "hints" !selfobj
 has   concealed
 with  name "h,";

! If you need an AfterPrompt() routine yourself, then call it AfterPrompt2, 
! and this will automatically call it after updating the hints.
[ AfterPrompt;
    AH_UpdateHints();
#ifdef AfterPrompt2;
    AfterPrompt2();
#endif;
];

! Okay, a meta-routine which is passed a hint object (well, hopefully :-) )
! and cycles through the hints.
[ AH_ShowHints  hintobj i j k die_now hint_number;

    give hintobj given;

    print "^Hints for: ", (name) hintobj, "^";

    i = hintobj.#the_hints / 2;

    j = 1;
    hint_number = 1;

    die_now = 0;

    print "(Press Q to quit receiving hints, or any other key to continue)^";

    while (j <= i && die_now==0) {
        print "^^(", j, "/", i, ") ";
        print_paddr (hintobj.&the_hints)-->(j-1);

        if (j ~= i) {
            @read_char 1 0 0 k;
            if (k=='Q' or 'q') die_now = 1;
        }
        j++;
        hint_number++;
    }

    print "^";

    if (die_now == 1) return 2;

    rtrue;

];

! Okay, another meta routine, this one blips through all the hints and 
! calls the hint_check routine for all of them, to reset the solved and
! available flags.  Note that once the puzzle is solved, it is no longer 
! run (to speed things up).
[ AH_UpdateHints i j;

    objectloop (i in Hints) {
#ifdef HINTDEBUG;
        print "Running hint_check for: ", (name) i, "^";
#endif;
        if (i hasnt solved) {
            j = ZRegion(i.hint_check);     ! only run if hint_check routine 
            if (j==2) PrintOrRun(i,hint_check,2);       ! exists for hint i
        }
        if (i has solved) give i ~general;
    }
];

! Okay, some debugging routines (useful stuff for me, but useless for most
! other people, I suspect).
#ifdef HINTDEBUGVERBS;
[ AllPuzzlesSub i;
    print "All Puzzles with Hints:^";
    objectloop (i in Hints) {
        print "    ", (name) i;
#ifdef SHOWSOLVEDTAG;
        if (i has solved) print " (solved)";
#endif;
        print "^";
    }
];

[ AllHintsSub i ;

    objectloop (i in Hints) {
        give i in_menu;
    }

    AH_Menu();

    objectloop (i in Hints) {
        give i ~in_menu;
    }

    rtrue;

];
#endif;

! General use function -- calculates the width of a string
Array width_calc table 64;
[ AH_CalcWidth s i j;
    i = 0->33; if (i==0) i = 80;
    @output_stream 3 width_calc;
    print (string) s;
    @output_stream -3;
    j = (width_calc-->0)/2;
    return j;
];

! Menu support routine for HintSub

! display hints
[ AH_HintPrint i count start stop;
    print "Hints Available:^";
    count = 0;
    if (pretty_flag == 0) {
        objectloop(i in Hints) {
            if (i has in_menu) {
                count++;
                if (count < 10) { print "^  (", count, ") "; }
                    else print "^ (", count, ") ";
                print (name) i;
#ifdef SHOWSOLVEDTAG;
                if (i has solved) print " (solved)";
#endif;
            } ! in_menu
        } ! objectloop
    } ! pretty_flag
    else {
        start = AH_hints_per_page * (AH_current_page - 1);
        stop = start + AH_hints_per_page;
        if (AH_current_page > 1) print "^     (previous page)";
        objectloop(i in Hints) {
            if (i has in_menu) {
                count++;
                if (count > start && count <= stop) {
                    print "^     ", (name) i;
#ifdef SHOWSOLVEDTAG;
                    if (i has solved) print " (solved)";
#endif;
                } ! start < count <= stop
            } ! in_menu
        } ! objectloop
        if (AH_current_page < AH_num_pages) print "^     (next page)";
    } ! elseif
];

! return titles, widths, and stuff
[ AH_HintInfo i j count target;

    count = 0;
    if (pretty_flag == 0) {
        objectloop(i in Hints) {
            if (i has in_menu) {
                count++;
                if (count == menu_item) j = i;
            }
        }
    } ! plain
    else {
        if (AH_current_page == 1) { target = menu_item; }
        else {target = ((AH_current_page - 1)*AH_hints_per_page) + menu_item;}
        objectloop (i in Hints) {
            if (i has in_menu) {
                count++;
                if (count == target) j = i;
            }
        }
        ! take care of setting count for paging
        if (AH_current_page == 1 && AH_num_pages ~= 1)
            { count = AH_hints_per_page + 1; }
        else { 
            if (AH_current_page == AH_num_pages && AH_num_pages ~= 1)
                { count = count - ((AH_num_pages-1)*AH_hints_per_page) + 1; }
            else {
                if (AH_current_page == 1 && AH_num_pages == 1)
                    { count = count; }  ! null assignment
                else { count = AH_hints_per_page + 2; }
            } ! elseif
        } ! convoluted elseif
    } ! pretty

    if (menu_item == 0) {
        item_name = "Hints";
        item_width = AH_CalcWidth(item_name);
        return count;
    }

    item_name = j.short_name;
    item_width = AH_CalcWidth(j.short_name);
    rtrue;
];

! call appropriate routine for menu
[ AH_HintMenu i j count target;
    count = 0;
    if (pretty_flag == 0) {
        objectloop (i in Hints) {
            if (i has in_menu) {
                count++;
                if (count == menu_item) j = i;
            } ! in_menu
        } ! objectloop
    } ! pretty_flag
    else {
! take care of special cases first:
        if (AH_current_page == 1) {
            if (menu_item == (AH_hints_per_page + 1)) {
                AH_current_page++;
                DoM_cl = 7;
                return 2;  ! redraw menu screen
            }
        } ! current page
        else {
            if (menu_item == 1) {
                AH_current_page--;
                DoM_cl = 7;
                return 2;  ! redraw menu screen
            }
            if (menu_item == (AH_hints_per_page + 2)) {
                AH_current_page++;
                DoM_cl = 7;
                return 2;  ! redraw menu screen
            }
        } ! elseif
        j = NULL;
        if (AH_current_page == 1) { target = menu_item; }
        else {
            target = ((AH_current_page - 1) * AH_hints_per_page) 
                     + menu_item - 1;  ! account for (previous page) option
        } ! elseif
        objectloop (i in Hints) {
            if (i has in_menu) {
                count++;
                if (count == target) j = i;
            } ! in_menu
        } ! objectloop
    } ! elseif
    AH_ShowHints(j);
    rtrue;
];

[ AH_CalcMenu i h count;
    count = 0;
    objectloop(i in Hints) {
        if (i has in_menu) count++;
    }
    h = 0->32;
#ifdef HINTDEBUG_PAGING;
    print "^height: ", h, "^";
#endif;
    if (h == 0) h = 25;
    h = h - 13;         ! adjust for administrative headaches
    AH_num_pages = (count / h) + 1;
    AH_hints_per_page = h;
#ifdef HINTDEBUG_PAGING;
    print "^num pages: ", AH_num_pages;
    print "^hints per page: ", AH_hints_per_page, "^";
#endif;
];

[ AH_Menu ;
    AH_CalcMenu();
    if (pretty_flag == 0) AH_num_pages = 1;
    AH_current_page = 1;
    DoMenu(#r$AH_HintPrint, #r$AH_HintInfo, #r$AH_HintMenu);
];

! Okay, the way the HintSub works is like so:
!   Check to make certain a hint is available and not yet solved.
!     If not, exit with a comment to that effect.
!   Find out how many puzzles are currently available and not already given
!     If none are left, comment that no new hints are available, and past
!       hints can be viewed using the REVIEW command.
!   If more than one puzzle is available at this point, pass it off to a 
!     menu routine.
!   Otherwise, do the normal hint for the only available puzzle.

[ HintSub i j numpuz some_given;

    AH_UpdateHints();

#ifdef HINTDEBUG;
    print "Hints available:^";
    objectloop (i in Hints) {
        if (i has general) {
            print (name) i, "^";
        }
    }
    print "^";
#endif;

    numpuz = 0;
    some_given = 0;
    objectloop (i in Hints) {         ! j = the first puzzle coming out 
        if (i has general) {          ! of this loop
#ifdef GIVEHINTSONCE;
            if (i hasnt given) {
                if (numpuz == 0) j = i;
                numpuz++;
            } else some_given++;
        }
#ifnot;
            if (numpuz == 0) j = i;
            numpuz++;
        }
        if (i has given || i has solved) some_given++;
#endif;
    }

    if (numpuz==0) {
        if (some_given ~= 0) {
            "No new hints are waiting.  Try using REVIEW to look at the hints \
            you have already seen.";
        }
        "You haven't found a puzzle yet to have a hint available!";
    }

    if (numpuz > 1) {
        objectloop (i in Hints) {
            if (i has general) {
#ifdef GIVEHINTSONCE;
               if (i hasnt given) {
#endif;
#ifdef HINTDEBUG;
                    print "Giving in_menu to puzzle: ", (name) i, "^";
#endif;
                    give i in_menu;
#ifdef GIVEHINTSONCE;
                }
#endif;
            }
        }
        AH_Menu();
        objectloop (i in Hints) {
            if (i has in_menu) {
#ifdef HINTDEBUG;
                print "Removing in_menu from puzzle: ", (name) i, "^";
#endif;
                give i ~in_menu;
            }
        }
        rtrue;
    }

    AH_ShowHints(j);

];

[ ReviewSub i count;

    AH_UpdateHints();

    count = 0;

    objectloop (i in Hints) {
#ifdef REVIEWGIVENONLY;
        if (i has given) {
#ifnot;
        if (i has solved || i has given) {
#endif;
#ifdef HINTDEBUG;
            print "Adding puzzle: ", (name) i, "^";
#endif;
            give i in_menu;
            count++;
        }
    }

    if (count == 0)
        "No hints are available to be reviewed.";

    AH_Menu();

    objectloop (i in Hints) {
        if (i has in_menu) {
#ifdef HINTDEBUG;
            print "Removing in_menu from ", (name) i, "^";
#endif;
            give i ~in_menu;
        }
    }

    rtrue;
];

! Disabling the hints system
[ HintsOffSub;
    if (AH_hints_available == 0) 
        "Hints are already disabled.";
    AH_hints_available = 0;
    "Hints are now disabled.";
];

[ HintsOnSub;
    if (AH_hints_available == 1)
        "Hints are already on!";
    "Hints cannot be re-enabled after being disabled.";
];

! And now we declare the grammar for HINT and REVIEW

#ifdef HINTDEBUGVERBS;
Verb "puzzles" "allpuzzles"
        *                                              -> AllPuzzlesSub;

Verb "allhints"
        *                                              -> AllHintsSub;

#endif;

Verb "hints"
        * "on"                                         -> HintsOnSub
        * "off"                                        -> HintsOffSub
        *                                              -> HintSub;

Verb "hint"
        *                                              -> HintSub;

#ifndef NOHINTREVIEW;
Verb "review"
        *                                              -> ReviewSub;
#endif;

