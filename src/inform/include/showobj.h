!--------------------------------------------------------------------------
! SHOWOBJ.H
! David Wagner
! Version 3
!  7-Nov-1995
! All rights given away.
! 
! Defines the routines and grammar to create a debugging SHOWOBJ command,
! which will (to the best of its ability) reconstruct the definition of an
! object as it would appear in the Inform source code.  This file is
! self-contained, so one could save this as a separate file and 'Include
! "showobj"' sometime after the parser has been Include'd.
! 
! This requires that the DEBUG flag be set.
! 
! Restrictions: Of course, it can't print out routine definitions, so it
! simply indicates that they exist by printing "[ ... ]". Also, it has to
! make guesses about whether objects are rooms, or daemons rather than
! timers, etc. and so it may get these wrong on occasion.  Also, it only
! knows about those properties and attributes defined in the library
! (version 5/12), so you will have to modify it if you want your own
! properties / attributes printed (or if you using an earlier version of
! the libraries).  Also, it doesn't know whether and object was originally
! defined using a "class" keyword.
! 
! For the describe, description and short_name properties, this will
! actually call the routines to see what they print out; since calling
! routines can have unwanted side effects, this feature can be turned off
! by defining a constant SHOWOBJNOCALL before including this file.
!
! By default, it prints all the properties followed by all the attributes;
! this order can be reversed by defining a constant GARETH before including
! this file.
!--------------------------------------------------------------------------

#IFDEF DEBUG;

! Use up one global here.  Sorry.  This initialized to 0; it is set to 1 if
! the keyword 'with' has been printed and 2 if the keyword 'has' has been
! printed and 3 if both have been printed

Global ShowObjFlag;

!-----------------------------------------------------------------------
! PropertyType
! Determines the type of property.  Slightly more informative than ZRegion
! (also ZRegion(addr) has problems if it is called when addr is obj.prop
! where the property is an array of values).
!
!      Input: obj   = object
!             prop  = property
!             iprop = element number if the property is an array
!
!    Returns: 0 = property doesn't exist
!             1 = property is a valid object number
!             2 = property is routine
!             3 = property is a string
!             4 = property is some other number
!
!      Calls: UnsignedCompare
!
!      Local: addr  = address of the property in question
!-----------------------------------------------------------------------
[ PropertyType obj prop iprop addr;
    if (obj.&prop == 0) return 0;         ! property doesn't exist
    if (obj.#prop == 1) return 4;         ! one-byte long = pure number
    addr = (obj.&prop)-->iprop;
    if (addr == NULL) return 0;          
    if (addr == 0) return 4;
    if (addr >= 1 && addr <= top_object) return 1;
    if (UnsignedCompare(addr, #strings_offset)>=0) return 3;
    if (UnsignedCompare(addr, #code_offset)>=0) return 2;
    return 4;
];

!-----------------------------------------------------------------------
! PrintObjectProperty
! Prints one property for the noun object.
! Checks to see if the property exists first.
! Prints 'with' before printing the first property.
!
!      Input: prop       - the property to print
!             propstring - the name of the property
!
!  Called by: ShowObjSub
!
!      Calls: PropertyType, RunRoutines
!
!      Local: a     - type of property (returned by PropertyType())
!             nprop - size of the property
!             iprop - loop counter
!             addr  - number or address of the string / routine
!-----------------------------------------------------------------------
[ PrintObjectProperty prop propstring a nprop iprop addr itemp;

    ! Does the property exist at all?

    if (noun.&prop == 0) rfalse;
    if (noun.#prop == 2 && noun.prop == NULL) rfalse;

    ! The property exists, so print its name (and "with" if it is the
    ! first)

    if ((ShowObjFlag & 1) == 0) {
        print "^  with ";
        ShowObjFlag = ShowObjFlag + 1;
    }
    else
        print ",^       ";
    print (string) propstring, " ";

    ! If the property is only one byte, it's just a number

    if (noun.#prop == 1) {print noun.prop; return}

    ! Otherwise, loop over each word of the property

    nprop = noun.#prop / 2;
    for (iprop=0: iprop<nprop: iprop++) {
        if (iprop ~= 0) print " ";

        ! Get the address of the property and its type

        addr = (noun.&prop)-->iprop;
        a = PropertyType(noun,prop,iprop);
        if (a == 0) jump ENDLOOP;

        ! Handle the name property special

        if (prop == name) {
            print "~", (address) addr, "~";
            jump ENDLOOP
        }

        ! For found_in, we want to print the names of the room

        if (prop == found_in && a == 1) {
            PrintShortName(addr);
            jump ENDLOOP;
        }

        ! Some properties must always be routines:

        if (prop == before or after or life ||
            prop == react_before or react_after or orders ||
            prop == grammar or daemon) {
            print "[ ... ]";
            jump ENDLOOP;
        }
        
        ! Call the describe, description and short name routines to see
        ! what they say about the object; since routines can have
        ! side-effects, this can be compiled with SHOWOBJNOCALL defined to
        ! prevent this from happening.

        #IFNDEF SHOWOBJNOCALL;
        if ( prop == describe ||
           (prop == description && a == 2) ||
           (prop == short_name && a == 2)) {
            self = noun;
            print "[ ~"; 
            itemp=indirect(addr);
            print "~ ]";
            jump ENDLOOP;
        }
        #ENDIF;
        #IFDEF SHOWOBJNOCALL;
        itemp = 0;
        #ENDIF;

        ! Handle door_dir property specially

        if (prop == door_dir && a == 1) {
            if (noun.door_dir ==   n_to) print "n_to";
            if (noun.door_dir ==  ne_to) print "ne_to";
            if (noun.door_dir ==   e_to) print "e_to";
            if (noun.door_dir ==  se_to) print "se_to";
            if (noun.door_dir ==   s_to) print "s_to";
            if (noun.door_dir ==  sw_to) print "sw_to";
            if (noun.door_dir ==   w_to) print "w_to";
            if (noun.door_dir ==  nw_to) print "nw_to";
            if (noun.door_dir ==   u_to) print "u_to";
            if (noun.door_dir ==   d_to) print "d_to";
            if (noun.door_dir ==  in_to) print "in_to";
            if (noun.door_dir == out_to) print "out_to";
            jump ENDLOOP;
        }

        ! All other cases

        if (a == 1 || a == 4) print addr;
        if (a == 2) print "[ ... ]";
        if (a == 3) { print "~", (string) addr, "~"; }
        .ENDLOOP
    }
    rtrue;
];

!-----------------------------------------------------------------------
! PrintRoomProperty
! Prints one property for the noun object - assumes that noun is a room and
! prop is a direction; prints the name of the room that lies in that
! direction
!
!      Input: prop       - the property to print (must be a direction)
!             propstring - the name of the property
!
!  Called by: ShowObjSub
!
!      Calls: ZRegion, PrintShortName
!
!      Local: a - type of property (string or routine)
!-----------------------------------------------------------------------
[ PrintRoomProperty prop propstring a;
    if (noun.prop == NULL) rfalse;
    a = ZRegion(noun.prop);
    if (a == 0) rfalse;
    if ((ShowObjFlag & 1) == 0) {
        print "^ with   ";
        ShowObjFlag = ShowObjFlag + 1;
    }
    else
        print ",^        ";
    print (string) propstring, " ";

    if (a == 1) PrintShortName(noun.prop);
    if (a == 2) print "[ ... ]";
    if (a == 3) { print "~", (string) noun.prop, "~"; }
    rtrue;
];

!-----------------------------------------------------------------------
! PrintObjectAttribute
! Prints one attribute for the noun object.
! The caller has already checked that the attribute is set
!
!      Input: attr       - attribute to test for
!             text       - name of the attribute
!
!  Called by: ShowObjAttributes
!-----------------------------------------------------------------------
[ PrintObjectAttribute attr text;
    if (noun hasnt attr) rfalse;
    if ((ShowObjFlag & 2) == 0) {
        if ((ShowObjFlag & 1) == 1) print ",";
        print "^  has ";
        ShowObjFlag = ShowObjFlag + 2;
    }
    print " ", (string) text;
    rtrue;
];

!-----------------------------------------------------------------------
! ShowObjProperties
! Prints the properties of an object
!
!  Called by: ShowObjSub
!
!      Calls: PrintObjectProperty, PrintRoomProperty,
!-----------------------------------------------------------------------
[ ShowObjProperties;

    PrintObjectProperty(name,"name");
    PrintObjectProperty(article,"article");
    PrintObjectProperty(short_name,"short_name");
    PrintObjectProperty(parse_name,"parse_name");

    ! Since when_open == when_on == initial, guess which is meant

    if (noun has switchable || noun has on)
        PrintObjectProperty(when_on,"when_on");
    else {
        if (noun has open || noun has openable)
            PrintObjectProperty(when_open,"when_open");
        else
            PrintObjectProperty(initial,"initial");
    }
    PrintObjectProperty(description,"description");
    PrintObjectProperty(describe,"describe");

    ! Make the assumption that if the object isn't contained in something
    ! (or doesn't have a found_in property) then it is a room.  This won't
    ! always be correct, though. PrintRoomProperty prints the directions

    if (parent(noun) == 0 && noun hasnt moved && noun.&found_in == 0) {
        PrintObjectProperty(cant_go,"cant_go");
        PrintRoomProperty(n_to,"n_to");
        PrintRoomProperty(ne_to,"ne_to");
        PrintRoomProperty(e_to,"e_to");
        PrintRoomProperty(se_to,"se_to");
        PrintRoomProperty(s_to,"s_to");
        PrintRoomProperty(sw_to,"sw_to");
        PrintRoomProperty(w_to,"w_to");
        PrintRoomProperty(nw_to,"nw_to");
        PrintRoomProperty(u_to,"u_to");
        PrintRoomProperty(d_to,"d_to");
        PrintRoomProperty(in_to,"in_to");
        PrintRoomProperty(out_to,"out_to");
    }
    else {
        PrintObjectProperty(door_to,"door_to");

        ! Since s_to == when_off == when_closed, guess which is meant

        if (noun has switchable || noun has on)
            PrintObjectProperty(when_off,"when_off");
        else
            PrintObjectProperty(when_closed,"when_closed");
        PrintObjectProperty(with_key,"with_key");
        PrintObjectProperty(door_dir,"door_dir");
        PrintObjectProperty(add_to_scope,"add_to_scope");
        PrintObjectProperty(invent,"invent");
        PrintObjectProperty(plural,"plural");
        PrintObjectProperty(list_together,"list_together");
        PrintObjectProperty(react_before,"react_before");
        PrintObjectProperty(react_after,"react_after");
        PrintObjectProperty(grammar,"grammar");
        PrintObjectProperty(orders,"orders");
    }

    PrintObjectProperty(before,"before");
    PrintObjectProperty(after,"after");
    PrintObjectProperty(life,"life");
    PrintObjectProperty(number,"number");
    PrintObjectProperty(found_in,"found_in");
    PrintObjectProperty(capacity,"capacity");

    ! Check to see if the time_left property exists to distinguish between
    ! daemons and timers

    if (noun.&time_left ~= 0)
        PrintObjectProperty(time_out,"time_out");
    else
        PrintObjectProperty(daemon,"daemon");
    PrintObjectProperty(time_left,"time_left");
    PrintObjectProperty(each_turn,"each_turn");
];

!-----------------------------------------------------------------------
! ShowObjAttributes
! Prints all the attributes of an object
!
!  Called by: ShowObjSub
!
!      Calls: PrintObjectAttribute
!-----------------------------------------------------------------------
[ ShowObjAttributes;
    PrintObjectAttribute(animate,"animate");
    PrintObjectAttribute(clothing,"clothing");
    PrintObjectAttribute(concealed,"concealed");
    PrintObjectAttribute(container,"container");
    PrintObjectAttribute(door,"door");
    PrintObjectAttribute(edible,"edible");
    PrintObjectAttribute(enterable,"enterable");
    PrintObjectAttribute(absent,"female/absent");
    PrintObjectAttribute(general,"general");
    PrintObjectAttribute(light,"light");
    PrintObjectAttribute(lockable,"lockable");
    PrintObjectAttribute(locked,"locked");
    PrintObjectAttribute(moved,"moved");
    PrintObjectAttribute(on,"on");
    PrintObjectAttribute(open,"open");
    PrintObjectAttribute(openable,"openable");
    PrintObjectAttribute(proper,"proper");
    PrintObjectAttribute(scenery,"scenery");
    PrintObjectAttribute(scored,"scored");
    PrintObjectAttribute(static,"static");
    PrintObjectAttribute(supporter,"supporter");
    PrintObjectAttribute(switchable,"switchable");
    PrintObjectAttribute(talkable,"talkable");
    PrintObjectAttribute(transparent,"transparent");
    PrintObjectAttribute(visited,"visited");
    PrintObjectAttribute(workflag,"workflag");
    PrintObjectAttribute(worn,"worn");
];

!-----------------------------------------------------------------------
! ShowObjSub
! Provides the SHOWOBJ verb to print all properties and attributes of
! the noun object
!
!      Calls: ShowObjProperties ShowObjAttributes
!
!-----------------------------------------------------------------------
[ ShowObjSub;
    if (noun == 0) noun = inp1;
    if (noun <= 0 || noun > top_object) rfalse;
    ShowObjFlag = 0;

    ! Print the header - a rare case where we actually want to use
    ! "print object"

    print "Object  ",noun," ~", object noun, "~";

    ! Print everything
#IFNDEF GARETH;
    ShowObjProperties();
    ShowObjAttributes();
#IFNOT;
    ShowObjAttributes();
    ShowObjProperties();
#ENDIF;

    print ";^";
];

!  Define the verb SHOWOBJ here

Verb meta "showobj"
    * noun                           -> ShowObj
    * number                         -> ShowObj;
#ENDIF;
