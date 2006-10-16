! --------------------------------------------------------------------------
! GONEIGHBOUR
!
! Version 0.20, written by Alan Trewartha      alan@alant.demon.co.uk
! Last updated 3rd Feb 2001
!
! SUMMARY
! This extension makes extra use of room 'name' properties, which are
! usually only used to produce a "you do not need to refer to that" sort
! of message. It allows the objects in the Compass (n_obj, e_obj, etc)
! to take on the 'name's of the location that the player would get to by
! GOing in that direction. i.e. if the n_to of the current location leads
! to a location with the word 'farm' in its name property, then "GO FARM"
! will be the same as "GO NORTH" and "FARM" is the same as "NORTH".
!
! More precisely the compass object will take on the neighbouring
! locations 'name' property if that location has either the 'visited' or
! 'transparent' attribute but not if it is 'concealed'. The compass object
! also takes on words in a property called "external_name", regardless.
!
! --------------------------------------------------------------------------
! HOW TO USE GONEIGHBOUR
! 
! Before include "Parser" add the line "Replace Refers;"
! Include "GoNeighbour" any point after "Parser"
!
! If you have "noisy" *_to properties then add "Constant NOISY_DIR_TOS".
! This means that LeadsTo() won't call routines to see what they return.
!
! When writing location or "Room" objects, keep in mind
!    * transparent attribute  (to allow 'name' words even if not visited)
!    * concealed attribute (to disallow 'name' words even if visited)
!    * name property
!    * external_name property (YOU MUST USE SINGLE QUOTES!!)
!
! Optionally it's quite nice to add a little extra bit of grammar so that
! it accepts "GO TO FARM" "GO TOWARDS FARM":
!
!    Extend 'go' last
!    * 'to'/'toward'/'towards' noun -> Go;
!
! --------------------------------------------------------------------------
! EXAMPLE CODE
!
! Room_2 can be got to with "GO GREEN" before you've been there. You can see
! the green-ness from Room_1, so it's in the external_name property. You can
! only use "GO TRIANGLE" once you've been there.
!
! Object Room_1 "Red Circle Room"
!   with description
!           "The walls here are red. There is a circle on the ceiling.
!            To the north is a green room",
!        name "red" "circle",
!        external_name 'red',
!        n_to Room_2,
!   has  light;
!
! Object Room_2 "Green Triangle Room"
!   with description
!           "The walls here are green. There is a triangle on the ceiling.
!            To the south is a red room.",
!        name "green" "triangle",
!        external_name 'green',
!        s_to Room_1,
!   has  light;
!
! --------------------------------------------------------------------------


! A new common property
Property external_name;


! The refers patch
[ Refers obj wnum   wd i;
  if (obj==0) rfalse;

  i = wn; wn = wnum; wd = NextWordStopped(); wn = i;

  ! First the default
  if (WordInProperty(wd,obj,name)) rtrue;
  
  ! Then if obj has 'external_name' try that too
  if (obj provides external_name)
    if (WordInProperty(wd,obj,external_name)) rtrue;
    
  ! Now the trick.
  ! If obj is in compass, then try the object it leads to
  if (parent(obj)==Compass)
  {  i=location; if (i==thedark) i=real_location;
     i=LeadsTo(obj,i); if (i==0) rfalse;
     
     ! First the external_name
     if (WordInProperty(wd,i,external_name)) rtrue;
     
     ! If the room is concealed forget it...
     if (i has concealed) rfalse;
     
     ! If the room is unvisited and "opaque" forget it
     if (i hasnt visited && i hasnt transparent) rfalse;
     
     ! Otherwise check name
     if (WordInProperty(wd,i,name)) rtrue;
  }
    
  rfalse;
];



! LeadsTo returns either false (0), or the location that the player would
! come to by GOing in the direction given. This is a cut down version of
! the same routione provided in 'MoveClass' so the "Ifndef" makes sure that
! this is only compiled if it isn't already defined.

Ifndef LeadsTo;
Ifndef NOISY_DIR_TOS;
Message "** LeadsTo assuming quiet *_to (NOISY_DIR_TOS not defined) **";
Endif;
[ LeadsTo direction thisroom k tmp tmp2;
   if (~~(direction provides door_dir)) rfalse;
   if (~~(thisroom provides direction.door_dir)) rfalse;
   k=thisroom.(direction.door_dir);
   
   #ifdef NOISY_DIR_TOS;
     if (ZRegion(k)==2) rfalse;
   #endif;
   
   #ifndef NOISY_DIR_TOS;
     if (ZRegion(k)==2)
         k=k();
   #endif;
   
   if (ZRegion(k)~=1) rfalse;
   if (k has door)
   { tmp=parent(k);
     move k to thisroom;
     tmp2=k.door_to();
     if (tmp==0)
       remove k;
     else
       move k to tmp;
     k=tmp2;
   }
   return k;
];
Endif;




