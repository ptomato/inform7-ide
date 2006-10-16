!--------------------------------------------------------------------------
! FOLLOWER: an Inform library by Andrew Clover, release 3
! Based on Gareth Rees's original code.
! Updated by Gareth Rees, and then modified for use with Inform 6 by
! Neil Brown.
! 
! An Inform library to allow things to be FOLLOWed. To use, `Include' it
! just after `verblib', then give any object that can be followed `class
! followclass'. To move a followable object, call MoveNPC(object,
! destination, actiontomove, objectnumber). Pass 0 for objectnumber to
! disallow following.  Pass 0 for destination to remove object.
! 
! When the player attempts to follow the object, the action <actiontomove
! objectnumber> will be generated.
! 
! In this release, the player can also attempt to follow any followable
! object contained within the moving followable object. For example, if
! Louise drives off in a car, the player may 'Follow Louise' as well as
! 'Follow car'.
!
! Examples: MoveNPC(bob, bobshouse, ##Go, n_obj);
!           MoveNPC(driver, car, ##Enter, car);
!--------------------------------------------------------------------------

Property follow_action 0;
Property follow_object 0;
Attribute followable;

Class   followclass
 with   follow_action 0,
        follow_object 0,
 has    followable;

[ NewRoom i;
  objectloop (i has followable)
  {  if (i~=followclass) i.follow_object=0;
  } 
    rfalse;
];

[ FollowScope i;
    if (scope_stage==1) rfalse;
    if (scope_stage==2)
    {
      objectloop (i ofclass followclass)
        { PlaceInScope(i);
        }
      rtrue;
    }
    "Go where?";
];

[ FollowSub oldnoun oldpos;
    if (parent(noun) == location or player)
        print_ret "You are already with ", (the) noun, ".";
    if (noun hasnt followable)
        "That's not something you can follow.";
    if (noun.follow_object == 0) {
        print "You don't know where ";
        if (noun has animate) {
            if (noun has female) print "she"; else print "he";
        } else print "that";
        " is.";
    }
    oldnoun = noun;
    oldpos = parent(player);
    <(noun.follow_action) noun.follow_object>;
    if (oldnoun notin parent(player) && parent(player) ~= oldpos)
        print "^(", (the) oldnoun, " doesn't seem to be here any more.)^";
    rtrue;
];

[ MoveNPC tomove dest actn objn;
    if (dest==parent(player))
        tomove.follow_action= 0;
    else
        if (parent(tomove)==parent(player))
            MoveNPC2(tomove, actn, objn);
    if (dest==0) remove tomove;
    else move tomove to dest;
];

[ MoveNPC2 tomove actn objn i;
    if (tomove has followable) {
        tomove.follow_action= actn;
        tomove.follow_object= objn;
        objectloop(i in tomove) {
            MoveNPC2(i, actn, objn);
        }
    }
];

Verb "follow" "chase" "pursue" "trail"
    * scope=FollowScope -> Follow;
