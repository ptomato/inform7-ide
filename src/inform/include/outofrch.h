!----------------------------------------------------------------------------
!  OutOfRch.h  version 2.00                                            4/7/99
!  Marnie Parker   aka Doe aka FemaleDeer                   doeadeer3@aol.com
!----------------------------------------------------------------------------
! HISTORY
!
! Version 2.00 fixes an add_to_scope bug (when the scoped object is within the
! player) found by David Ledgard. Additionally, it adds a routine ability to
! reach_zones and allows the changing outofreach's default (take all) when no
! reach_zones are defined. Also included are new comments, including a warning
! note about classes and additive/non-additive properties.

! Version 1.01 fixes a slight bug in version 1.00.

!----------------------------------------------------------------------------
! PURPOSE (SUMMARY)
!
! A simple way (without scoping) to set limits on reaching when the player
! is in an enterable supporter or container, so they can take what is nearby
! but not what is in the rest of the room. The property, reach_zones, is used
! to define what other "areas" (stationary supporters/containers) are in reach
! (in addition to the player and enterable object, which are always in reach).
!
! Example: If the player is on a chair in front of a desk, the desk and desk
! drawer could be defined as reach_zones, within reach, and everything else
! as out of reach. If no reach_zones are defined, the property, limittake, can
! be set true to define everything in the location (except the player and
! enterable object) as out of reach.
!
! Can also include objects "on the floor" (when includefloor is true). Note
! that with the addition of the property, limittake, this routine has become a
! bit more complex than I would like, but due to user requests/misunderstandings
! it appears it have become necessary.
!
! Additionally lets the player exit a supporter or container without a new
! <Look> and, if the player is on a supporter, can drop items to the floor
! instead of onto the supporter. Usually I don't like dropped items ending up
! on the same thing that the player is sitting on (like that chair).
!
! Of course, when exiting, if the player is on a supporter on a supporter (or
! a container in a container), this will only exit the player to the next
! level. This can be easily be changed by changing the line, move player to
! parent(self), to, move player to location. Dropping can also be easily
! changed to drop the object to the next level, by changing the line, move
! noun to location, to, move noun to parent(self). However, the latter is
! the Inform default, anyway.
!
! The above is accomplished with a new class, InsideOrOn, for enterable
! supporters/containers. The react_before of this class does the in reach
! checking, exiting and dropping and is only called when the player is in
! the enterable object (the member of the InsideOrOn class).

! Include this file after parser.h.

!----------------------------------------------------------------------------
! NOTE ON CLASSES
!
! react_before is a non-additive class property. This means that if you create
! an object that is a member of the InsideOrOn class and it also has a
! react_before routine, the object's react_before completely "overwrites" the
! class' react_before. I.E., The class' react_before will NEVER BE called. You
! can, however, explicitly call it by inserting this:
!
! object.InsideOrOn::react_before();
!
! in the object's react_before. Note also that this is the way to call all
! non-additive class properties. Of course, if the object's property returns
! true, the class' property will still not be called. Consider yourself warned.

!----------------------------------------------------------------------------
! USAGE
!
! Note:  Objects in the player and enterable object are always within reach.
!
!----------------------------------------------------------------------------
!
! PROPERTIES    (top setting in each is the default):
!
! reach_zones   (only for other stationary containers/supporters - "furniture")
!               0             = undefined
!               array/routine = the area(s) within reach
! includefloor  false = only objects in reach_zones (if defined), player
!                       and enterable object are in reach
!               true  = also include "takeable" objects "on the floor"
! limittake     (applies only when reach_zones are undefined - 0)
!               false = everything is within reach
!               true  = limit take (and other actions) to only objects in the
!                       player and enterable object (only those are in reach)
! droponfloor   (applies only to supporters)
!               true  = dropped objects end up on the floor
!               false = they end up on the supporter
!
!----------------------------------------------------------------------------
! DEFINE:  reach_zones similarly to found_in. It can be set to 0 or defined
! with an array of objects or a routine (no strings). However, as with
! found_in, a routine can return only one object, not several. Also, as with
! found_in, the two cannot be combined to include both objects and a routine.
!
! Using the chair mentioned above as an example:
!
! reach_zones 0;
! reach_zones desk;
! reach_zones desk desk_drawer;
! reach_zones [; if (self has general) return desk; return 0; ];
! (in this case, general is a flag for when the chair is in front of the desk)
!
! NOT
!
! reach_zones desk [; if (self has general) return desk_drawer; return 0; ];
!
! This will not work with found_in and it won't work with reach_zones.
!
! Note:  If you have a string in your routine, it will be printed twice,
! once when tested by the class InsideOrOn and once by the program. Since
! this is undesirable behavior, do not include a string in your routine.
!
!----------------------------------------------------------------------------
! COMBINATIONS:
!
! reach_zones defined       (not 0)
!                         
! includefloor false        only objects in reach_zones, the player and
!                           enterable object are in reach
!     
! includefloor true         only objects in reach_zones, "on the floor", in the
!                           player and enterable object are in reach
!----------------------------------------------------------------------------
! Limittake is only checked when reach_zones are undefined (0).
!
! reach_zones undefined     (0)
!
! limittake true            only objects in the player and enterable object
! includefloor false        are in reach
!   
! limittake true            only objects "on the floor", in the player and
! includefloor true         enterable object are in reach
!                         
! limittake false           all objects are in reach when limittake is false
! includefloor false/true   regardless of how includefloor is set, this is
!                           the Inform default
!----------------------------------------------------------------------------
! Droponfloor only applies to supporters, not containers, and is unaffected by
! the other properties.
!
! droponfloor true          drop objects to the floor
!             false         drop objects to the supporter
!
!
! Note:  If the player is on a supporter and they enter, "put x on floor", and
! drop on floor is not set true, the object will be dropped to what the player
! is on. This is normal Inform behavior, regardless of the fact that the word,
! "floor" or "ground", was used. Put noun on floor (d_obj) becomes a drop and,
! as mentioned, Inform normally drops objects to whatever the player is on.
!
!----------------------------------------------------------------------------
! EXAMPLES
!
! Object chair "chair"
! class InsideOrOn
! has supporter
! with name "chair",
! initial "There is a chair in front of the desk.",
! description "It is an ordinary chair.",
! includefloor true,
! reach_zones desk desk_drawer;

! Object table "table"
! class InsideOrOn
! has supporter static
! with name "table",
! initial "There is also a table here, between the desk and closet.",
! description "It is a very large and high.",
! limittake true,
! droponfloor false;

! When the player is on the chair, objects on the table or in the closet would
! be out of reach, while those in the player, on the chair (because the player
! and enterable object are always within reach), the desk, desk drawer and on
! the floor would be in reach (because desk and desk_drawer are defined as
! reach_zones and includefloor is set true). When the player is on the table,
! everything except objects in the player and table would be out of reach
! (because no reach_zones are defined and limittake is set true). Also,
! objects dropped from the chair will land on the floor (because the default
! for droponfloor is true), while those dropped from the table will land on
! the table (because droponfloor is set false).

! Object chair "chair"
! class InsideOrOn
! has supporter
! with name "chair",
! initial
! [; if (self hasnt general) "The chair is by the bed.";
!   "The chair is in front of the desk."; ],
! description "It is an ordinary chair.",
! includefloor true,
! reach_zones [; if (self hasnt general) return bed; return desk; ],
! before
! [; Push, Pull : if (player in self)
!                    "You can't move the chair while you are sitting on it.";
!                 if (self hasnt general)
!                 {  give self general;
!                    "You move the chair in front of the desk.";
!                 }
!                 else
!                 {  give self ~general;
!                    "You move the chair next to the bed.";
!                 }
! ];

! Same as the above chair (see next paragraph), but when the player is on the
! chair, objects on the desk are only reachable when the chair is in front of
! it and objects on the bed are only reachable when the chair is beside it.

! A reach zone with an added_to_scope object automatically includes that scoped
! object. So the desk_drawer would also be reachable when the desk is reachable
! if the desk has add_to_scope desk_drawer. Thus the desk_drawer would not need
! to be defined as a separate reach_zone.

! Object chair "chair"
! class InsideOrOn
! has supporter
! with name "chair",
! initial "There is a chair here.",
! description "It is an ordinary chair.",
! limittake true;

! In this instance, everything not in the player and chair would be out of
! reach (because reach_zones are not defined and limittake is set true).

! Object table "table"
! class InsideOrOn
! has supporter static
! with name "table",
! initial "There is also a table here, between the desk and closet.",
! description "It is a very low table.",
! droponfloor false;

! Now everything would be within reach from the table (because no reach_zones
! are defined and the default for limittake is false). But objects dropped from
! the table would still end up on the table (because droponfloor is set false).

! Also see the note above the class routine, outofreach, for a definition of
! what objects are considered to be "on the floor".

! Hopefully these examples illustrate the various ways that outofrch.h can be
! used. It was initially only intended to provide reach_zones for enterable
! supporters/containers. It has since been altered to also be able to restrict
! what is in reach when no reach_zones are defined.

!----------------------------------------------------------------------------

Class InsideOrOn
 has enterable
 with reach_zones 0, ! reachable areas (other supporters & containers)
 includefloor false, ! (t/f) also include objects "on the floor"
 limittake false,    ! (t/f) reach_zones = 0, other objects are out of reach
 droponfloor true,   ! (t/f) drop objects from this supporter to the floor
 react_before
 [; if (player notin self) rfalse;
    Take, Remove, Search, Attack, Open, Close, Lock, Unlock, Push, Pull, Turn,
    SwitchOn, SwitchOff, Touch, Taste, Smell, Squeeze, LookUnder, Empty
         : if (self.outofreach(noun))
              "You can't reach ", (the) noun, " from where you are.";
           rfalse;
    Insert, Puton, Transfer, EmptyT
         : if (action==##EmptyT)
           { if (ObjectIsUntouchable(noun, 1)) rfalse; }
           else
           { if (action==##Transfer)
             { if (noun notin player) rfalse; }
             else if (parent(noun)~=player) rfalse;
           }
           if (action==##Insert or ##PutOn)
           { if ((second == d_obj) || (player in second)) <<Drop noun>>; }
           if (self.outofreach(second))
              "You can't reach ", (the) second, " from where you are.";
           rfalse;
    Exit : if ((self has container) && (self hasnt open)) rfalse;
           move player to parent(self);
           if (keep_silent == 0)
           {  print "You get ";
              if (self has supporter) print "off ";
              else print "out of ";
              print (the) self, ".^";
           }
           rtrue;
    Drop : if ((self has container) || (~~(self.droponfloor))) rfalse;
           if ((noun == player) || (noun notin player)) rfalse;
           move noun to location;
           if (keep_silent == 0)
              print "Dropped.^";
           rtrue;
 ],

! Copy of IndirectlyContains from verblibm.h, altered to include
! ObjectScopedBySomething to fix add_to_scope bug. Ditto with topholder,
! " added to fix add_to_scope bug.

! Find if the second object is in the first object.

 contains
 [ o1 o2 o3;

  while (o2~=0)
  {   if (o1==o2) rtrue;
      o3 = ObjectScopedBySomething(o2);
      if (o3 == 0) o2 = parent(o2);
      else o2 = o3;
  }
  rfalse;

 ],

! Find the top container (parent) of the object, short of the location.

 topholder
 [ o1 o2;

  while (o1 ~= location)
  {  o2 = ObjectScopedBySomething(o1);
     if (o2 == 0) o2 = parent(o1);
     if (o2 == location) break;
     o1 = o2;
  }
  return o1;

 ],

! This routine only returns true if the object is found to be definitely
! out of reach. It returns false if the object is within reach. If it is not
! in the location or is in a closed container it also returns false so that
! Inform can handle with its usual error messages.

! *** Note:  The only way to differentiate between a movable ("takeable")
! container on the floor that could be within reach if includefloor is true
! and a non-movable (large or piece of scenery) container that could be out
! of reach if not defined in reach_zones, was to define a non-movable container
! (or supporter) as having either of these attributes:  enterable, scenery,
! static or concealed. Bear this in mind. This was the bug in version 1.00.

! Examples:  A sack object on the floor (if include floor is set true) without
! enterable, scenery, static or concealed would be considered in reach. A
! closet would probably have static (or scenery) and could be defined as a
! reach_zone, within reach, or if not included as a reach_zone, could be out
! of reach (if limittake is set true).

 outofreach
 [ o c p i j;

! Is the object not in the location or is it in a closed container?

   if (ObjectIsUntouchable(o, 1)) rfalse;

! Next check if the object is in player and/or enterable (InsideOrOn) object.

   if (self.contains(player, o)) rfalse;
   if (self.contains(self, o)) rfalse;

! NO reach_zones are defined and limittake is false (meaning take all), so skip
! the rest of the checking.

   if (ZRegion(self.&reach_zones-->0) ~= 1 or 2)
   { if (~~(self.limittake)) rfalse; }

! Find the top "holder" (container/supporter) of the object.

   p = self.topholder(o);

! If reach_zones are defined, check them first. If the object (its top holder)
! is in a reach_zone (is the same as a reach zone), it is within reach.

   if (ZRegion(self.&reach_zones-->0) == 1 or 2)
   {  if (ZRegion(self.&reach_zones-->0) == 2)
      { j = self.reach_zones();
        if (j == p) rfalse;
      }
      else
      {  c = self.#reach_zones;
         for (i = 0: i < (c/2): i++)
         { j = self.&reach_zones-->i;
           if (j == p) rfalse;
         }
      }
   }

! Next, check includefloor for both defined/undefined reach_zones. If
! includefloor is true and the object is "on the floor", it is within reach.

! See *** note above for why these attributes are used.

   if (self.includefloor)
   {  if (p hasnt enterable && p hasnt scenery && p hasnt static &&
          p hasnt concealed) rfalse;
   }

! The object isn't in the player, enterable object, a reach zone (if defined)
! or "on the floor" (if includefloor is true), so it is definitely out of reach.

   rtrue;

 ];




