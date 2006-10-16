! Compass rose (with compass on/off commands), in -*- Inform -*- mode.
!
! To use:
!    1) put 'Replace DrawStatusLine' before including standard libraries.
!    2) put 'Include "compass"' after including them.

Global use_compass = 0;

! The following constants describe the positions of the different parts of
! the windrose.
Constant U_POS 28;
Constant W_POS 30;
Constant C_POS 31;
Constant E_POS 32;
Constant IN_POS 34;

[ DrawStatusLine i;
    ! Switch to status window and reverse it.
    if (use_compass == 0) @split_window 1; else @split_window 3;
    @set_window 1; style reverse; font off;
    @set_cursor 1 1; spaces (0->33)-1;

    if (use_compass ~= 0) {
        @set_cursor 2 1; spaces (0->33)-1;
        @set_cursor 3 1; spaces (0->33)-1;
    }
    
    ! Now the original DrawStatusLine routine.
    @set_cursor 1 2;  print (name) location;
    if ((0->1)&2 == 0) {   
        @set_cursor 1 51; print "Score: ", sline1;
        @set_cursor 1 64; print "Moves: ", sline2;
    } else {   
        @set_cursor 1 51; print "Time: ";
        i = sline1 % 12; if (i < 10) print " ";
        if (i == 0) i = 12;
        print i, ":";
        if (sline2 < 10) print "0";
        print sline2;
        if ((sline1/12) > 0) print " pm"; else print " am";
    }

    ! And now print the directions of the location (if not dark).  This is
    ! the interesting part (for me at least).
    if (location ~= thedark && use_compass ~= 0) {
        ! First line
        if (location.u_to ~= 0)  { @set_cursor 1 U_POS; print "U"; }
        if (location.nw_to ~= 0) { @set_cursor 1 W_POS; print "@@92"; }
        if (location.n_to ~= 0)  { @set_cursor 1 C_POS; print "|"; }
        if (location.ne_to ~= 0) { @set_cursor 1 E_POS; print "/"; }
        if (location.in_to ~= 0) { @set_cursor 1 IN_POS; print "I"; }

        ! Second line
        if (location.w_to ~= 0)  { @set_cursor 2 W_POS; print "-"; }
                                   @set_cursor 2 C_POS; print "o";
        if (location.e_to ~= 0)  { @set_cursor 2 E_POS; print "-"; }

        ! Third line
        if (location.d_to ~= 0)  { @set_cursor 3 U_POS; print "D"; }
        if (location.sw_to ~= 0) { @set_cursor 3 W_POS; print "/"; }
        if (location.s_to ~= 0)  { @set_cursor 3 C_POS; print "|"; }
        if (location.se_to ~= 0) { @set_cursor 3 E_POS; print "@@92"; }
        if (location.out_to ~= 0){ @set_cursor 3 IN_POS; print "O"; }
    }

    ! switch to main window 
    @set_cursor 1 1; style roman; @set_window 0; font on;
];

[ CompassOffSub;
    use_compass = 0; "[Compass is now off.]";
];

[ CompassOnSub;
    use_compass = 1; "[Compass is now on.]";
];

Verb meta "compass"
		*				-> CompassOn
		* "off"				-> CompassOff
		* "on"				-> CompassOn;
