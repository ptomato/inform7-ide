! EasyDoors.h (Easy handling of doors)
! copyright (c) 2000-2001 Andrew MacKinnon
! May be distributed freely, proprietary incorporation allowed
! May be embedded in any game file created with Inform
! Credit for use is appreciated but not required
!
! Version 3.0 -- 26 May 2001
!
! Example:
!
! Room Bedroom "Bedroom"
!   with description "This is a bedroom.",
!   n_to bedroom_door;
!
! Room Hall "Hall"
!   with description "This is a hall.",
!   s_to bedroom_door;
!
! Doorway bedroom_door "bedroom door"
!   with name 'door',
!   side1_to Bedroom,  ! Room on side 1
!   side1_dir s_to,    ! Direction to get there (s_to, not s_obj)
!   side2_to Hall,     ! ditto for side 2
!   side2_dir n_to,    ! direction
!   found_in Bedroom Hall, ! required to show which two rooms the door is in
!  ! OPTIONAL BELOW
!   isconcealed {0 | 1}, ! 0-as a seperate paragraph in room desc
!                        ! 1-must add to room desc
!                        ! (default 1)
!   autoopen {0 | 1}     ! 0-must open explicitly
!                        ! 1-automaticaly opens when go through when not locked
!                        ! (default 1)
!   opendesc "open",     ! open description (default "open")
!   closeddesc "closed"; ! closed description (default "closed")
!  ! METHODS
!
!   statedesc()          ! prints state (open or closed) for use in room descriptions,
!                        ! recommended in room description bodies
!                        ! REMEMBER to set lookmode=2 in Initialise() or this
!                        ! is a bad idea
!
!
! Note that the class Room is created, this just gives the room light by default;
! I find it tons easier with such a class distinguishing rooms and objects.
!
! DISACTIVATE THIS BY DEFINING "NO_ROOM_CLASS"

#IFNDEF NO_ROOM_CLASS;
Class Room
  has light;
#ENDIF;

Class Doorway
  with door_to
  [; if (location==self.side1_to) return self.side2_to; return self.side1_to;
  ],
  door_dir
  [; if (location==self.side1_to) return self.side2_dir; return self.side1_dir;
  ],
  side1_to 0,
  side2_to 0,
  side1_dir 0,
  side2_dir 0,
  opendesc "open",
  closeddesc "closed",
  statedesc [; if (self has open)   print (string) self.opendesc; 
               if (self hasnt open) print (string) self.closeddesc; ],
  isconcealed 1, ! concealed by default
  describe
  [; if (self.isconcealed==1) rtrue;
     print "^";
     print (The) self, " is "; self.statedesc(); ".";
  ],
  autoopen 1, ! automaticaly open by default
  before
  [; Enter: self.automat();
  ],
  react_before
  [; Go: if (noun notin compass) rfalse;
         if ((noun.door_dir==self.side1_dir && location==self.side2_to)
          || (noun.door_dir==self.side2_dir && location==self.side1_to)
          || (location.(noun.door_dir)==self))
               self.automat();
  ],
  automat
  [; if (self.autoopen==0) return;
     if (self has locked) return; ! can't open if locked
     if (self has open) return;   ! nothing to do if open
     if (self hasnt open && self hasnt locked)
       {print "(opening ", (the) self, ")^"; give self open;}
  ],
  has door static openable;
