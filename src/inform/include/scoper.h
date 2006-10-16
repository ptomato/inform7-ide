! Written by Daniel Barkalow, v1.0, Sat Jan 29 2000.

! This file defines objects which will determine what rooms are
! visible from a given location. This is useful for having open spaces
! comprised of multiple game locations. It is not able to change
! dynamically, however. The simplest example is of a setup where the
! player has some way of seeing what's in adjacent locations (perhaps
! a LOOK <DIRECTION> command), and the game isn't responsible for
! mentioning objects in the other room all the time. To do this,
! simply create a Scoper with the adjacent locations in the "locs"
! property, and the directions to the locations in the "dirs"
! property.

! Note that objects found_in distant locations do not get added to
! scope with this mechanism. However, if an object which provides
! found_in is listed in the "locs" property of a Scoper, the object as
! well as its contents will be added to the scope.

! In order to have a single conceptual room with multiple locations,
! you want all of the objects, including the ones in different parts
! of the room to be listed when the room contents are listed. To
! handle this, use Locale() in the after Look method of each room to
! list the contents of other rooms with appropriate information.

! For instance:
!   Room1
!    ...
!   after [;
!    Look:
!     if (Locale(Room2, "You see", "You also see"))
!       print " on the other side of the room.^";
!    ]
!    ...
! Scoper ->
!  with locs Room2,
!       dirs "on the other side of the room";

! The player is permitted to Examine and Listen to objects in the
! distance, which prints a message saying 
! In order to permit further interaction with distant objects (beyond
! Examine and Listen), add the verbs to the list at the beginning of
! react_before.

! To find out where an object is, use the procedure
! Scoper_locate(obj), which returns the "dirs" entry corresponding to
! the "locs" entry that got the object added. This probably won't work
! with things added by the add_to_scope property of an object.

! TestScope(obj) reports true if the thing is in scope via a scoper;
! the ScopeCeiling(obj) does not get transferred, however, which can
! distinguish distant objects. Note that TestScope(actor1, actor2)
! might not be the same as TestScope(acter2, actor1) now, since
! Scopers aren't necessarily commutative.

Class 	Scoper	
  with 	locs 0,
 	dirs 0,
	add_to_scope
	[i j k l n;
	  n = self.#locs;
	  k = self.&locs;
	  l = self.&dirs;
	  for (j = 0 : (2 * j) < n : j++)
	   {
	    if ((k-->j) provides found_in)
	      AddToScope(k-->j);
	    else
	      objectloop (i in k-->j)
	        if (~~(i ofclass Scoper))
		  AddToScope(i);
	   }
	],
	react_before
	[i j k n;
	 Listen, Examine: ! Add other generally permissible actions here
	  i = Scoper_locate(noun);
	  if (i)
	    print "(", (the) noun, " ", (string) i, ")^";
	  rfalse;
	  ! This would be a good place to add traps for things that
	  ! the scoper should handle, like figuring out where a named
	  ! object actually is.
	 default:
	  if (inp1 ~= 1 && noun ~= 0) ! test for permissible actions
	    ! with noun.
	   {
	    i = Scoper_locate(noun);
	    if (i)
	      print_ret "You can't reach ", (the) noun, ", since it's ",
		(string) i, ". ";
	   }
	  if (inp2 == 1 || second == 0 || action == ##ThrowAt) ! add
	    ! permissible actions with second here.
	    rfalse;
	  i = Scoper_locate(second);
	  if (i)
	    print_ret "You can't reach ", (the) second, ", since it's ",
	      (string) i, ". ";
	],
  has 	concealed;

[Scoper_locate obj i j k l n;
  objectloop (i ofclass Scoper)
    if (parent(i) == real_location)
      l = i;
  if (~~l)
    rfalse; ! no scoper here.
  i = ScopeCeiling(obj);
  k = l.&locs;
  n = l.#locs;
  for (j = 0 : (2 * j) < n : j++)
   {
    if (i == k-->j)
      return l.&dirs-->j;
   }
  rfalse; ! not added via a scoper
];
