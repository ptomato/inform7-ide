!==============================================================================!
!   SMARTCANTGO.H
!       David Wagner
!           Version 3
!           8-Nov-1995
!       Revisited for Inform 6 by Roger Firth
!           Version 5
!           3-Feb-1999
!   All rights given away.
!
!   Defines a routine that can be used for the cant_go property of rooms to
!   helpfully list the exits, instead of saying just "You can't go that way."
!   Store the code in a file "smartcantgo.h" and include it into games with the
!   following line (placed anywhere after the standard Include "Parser";):
!
!       Include "smartcantgo";
!
!   Once the routine has been Included, the following code:
!
!       Object  Crystal_Cave "Crystal Cave"
!         with  s_to Narrow_Passage,
!               cant_go [; SmartCantGo(); ]
!               ...;
!
!   will produce the message "You can go only south." if the player goes
!   the wrong way.
!
!   Rather than explicitly add the cant_go routine to each room, you can
!   add it to a Room class from which your actual rooms are derived:
!
!       Class   Room
!         with  cant_go [; SmartCantGo(); ],
!         has   light;
!
!   In theory, you can instead change the default behaviour of the cant_go
!   routine by adding the following line to your Initialise routine, but in
!   practise there's a library bug/feature which prevents this working properly:
!
!       ChangeDefault(cant_go, SmartCantGo);
!
!==============================================================================!
!   NOTES
!
!   1.  If the room is dark, it prints just "You can't go that way."
!       How could the player know where the exits are?
!
!   2.  This routine ignores direction properties which point to concealed
!       doors or are strings. Therefore this code won't work quite as intended
!       in the following room:
!
!       Object  Library "Library"
!         with  description
!                   "A small wood-panelled library. An open window to the west
!                    affords a stunning view of the Tuscan coastline.",
!               w_to
!                   "Ouch! You discover that the ~window~ is really
!                    an incredibly lifelike mural painted on the wall.",
!               ...;
!
!   3.  This routine does include direction properties that are routines
!       on the assumption that the routine will return something sensible.
!       If your routine returns a concealed door, you will have to do
!       something different.
!
!   4.  This routine is adequate for the rare occasions when the player types
!       the wrong direction. One might be tempted to use this routine to
!       simplify room descriptions, as in:
!
!           Object  Boring_Room "Boring Room"
!             with  description [;
!                       print "A small room with white walls. ";
!                       SmartCantGo();
!                       ],
!                   ...;
!
!       Don't do this! Part of the interest of a room is a good description
!       of the exits.
!
!==============================================================================!
[ SmartCantGo room i dest desttype ndir;

    if (location == thedark) "You can't go that way.";

!  Find what room the player is in.

    room = location;
    while (parent(room)) room = parent(room);

!  Count the number of exits -- if a direction property is a string or
!  a concealed door, don't count it; if it is a routine, count it.

    ndir = 0;
    objectloop (i in compass) {
        dest = room.(i.door_dir);
        if (dest) {
            desttype = ZRegion(dest);
            if ((desttype ~= 3) &&
                (desttype == 2 || dest hasnt door || dest hasnt concealed))
                ndir++;
            }
        }
    if (ndir == 0) "There are no exits.";

!  Print the exits.

    print "You can go only ";
    objectloop (i in compass) {
        dest = room.(i.door_dir);
        if (dest) {
            desttype = ZRegion(dest);
            if ((desttype ~= 3) &&
                (desttype == 2 || dest hasnt door || dest hasnt concealed)) {
                PrintDirectionName(i);
                switch (--ndir) {
                    0:       ".";
                    1:       print " or ";
                    default: print ", ";
                    }
                }
            }
        }
    ];

!==============================================================================!
[ PrintDirectionName obj;

    switch (obj) {
        n_obj:   print "north";
        s_obj:   print "south";
        e_obj:   print "east";
        w_obj:   print "west";
        ne_obj:  print "northeast";
        se_obj:  print "southeast";
        nw_obj:  print "northwest";
        sw_obj:  print "southwest";
        u_obj:   print "up";
        d_obj:   print "down";
        in_obj:  print "in";
        out_obj: print "out";
        default: print "some unspecified direction";
        }
    ];

!==============================================================================!
