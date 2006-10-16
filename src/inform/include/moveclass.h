! -                                                                         
! MOVECLASS, a library file to provide random, directed and 'intelligent'
!            movement for NPCs
!
! Version 8.02, written by Neil Brown          neil@highmount.demon.co.uk
!                      and Alan Trewartha      alan@alant.demon.co.uk
!
! Last altered 18th April 1999.
!
! Developed on an Acorn. Some weaker PC text editors (eg Notepad) don't like
! the fact that lines aren't terminated in exactly the same way as on PCs.
!
! The functions of this library are too complex to go into here, so please
! refer to the brief manual which should be near to where you found this
! file, and is named 'moveman.txt'.
!
! If you are including the library file FOLLOWER.H in your game code, please
! include this file AFTERWARDS and not before, otherwise errors will occur.
! -                                                                         

System_file;

Message "!! Compiling library extension MoveClass 8.01 !!";

Attribute en_route;
Property  npc_open;
Property  after_action;
Property  before_action;
Property  caprice  alias  time_left;
Global    path_size_limit = 10;

Constant   RANDOM_MOVE = 0; ! The different move_types
Constant    AIMED_MOVE = 1;
Constant       NO_MOVE = 2;
Constant   PRESET_MOVE = 3;

Constant      ANY_PATH = $$00000000; ! The different types of AIMED_MOVEs
Constant UNLOCKED_PATH = $$00001000; ! Bitmaps so they can be combined
Constant     OPEN_PATH = $$00010000; ! in principle
Constant DOORLESS_PATH = $$00100000;


Ifndef Room;
Class Room
  with number;
EndIf;


Class moveclass
  with move_type
           RANDOM_MOVE, ! The default move_type is to move randomly
       caprice      20, ! Chance the NPC will move each turn when RANDOM_MOVE
       npc_dirs 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0,
                        ! The calculated directions that the NPC takes.
                        ! Note this is a word array, but the dirs are held as
                        ! single bytes, so a path of 64 moves is possible.
       prepath_name  0, ! The name of the predetermined path array
       prepath_size  0, ! The length of the set path
       npc_stage     0, ! Position along set path array
       npc_target,      ! The target destination
       npc_blocked [; NPC_Path(self,RANDOM_MOVE); ],
                        ! Alternatively do nothing and wait for the path to
                        ! unblock. Or, more intelligently look for a less
                        ! restrictive path.
       npc_ifblocked 0, ! Free for use by npc_blocked
       npc_arrived [; NPC_Path(self,RANDOM_MOVE); ],
                        ! Redefine this within the actual NPC object
                        ! for more sophisticated results. Deals with what
                        ! happens when an NPC arrives at its destination. In
                        ! this case, it returns to random movement.
       walkoff "walks off",
       walkon  "arrives",
       follow_action,   ! } In case Follower has been included but the NPC
       follow_object,   ! } isn't of FollowClass.
       daemon [ i n k;
           if (RunRoutines(self,before_action)) rtrue;
                        ! Return true from before_action to overide
                        ! moveclass motion
                          
           switch(self.move_type)
           { 0, RANDOM_MOVE:                          ! Random movement
#ifdef DEBUG;
if (parser_trace>1)
print "[RANDOM_MOVE daemon for ", (the) self ,"]^";
#endif;
              if (random(100)>=self.caprice) rfalse;
              objectloop (i in compass)
                if (LeadsTo(i,parent(self),ANY_PATH))
                {  n++;
#ifdef DEBUG;
if (parser_trace>1)
print "[Choice ",n, ": ",(GiveDir) i ,"]^";
#endif;
                }                
              if (n==0) rfalse;
              k=random(n); n=0;                       ! Choose one direction
#ifdef DEBUG;
if (parser_trace>1)
print "[Choosing ",k, "]^";
#endif;
              objectloop (i in compass)
              { if (LeadsTo(i,parent(self),ANY_PATH)) n++;
                if (n==k)
                {  MoveNPCDir(self,i);
                   break;
                }
              }

             1, AIMED_MOVE :                   ! Moving on a calculated path
              i=self.&npc_dirs->self.npc_stage;
#ifdef DEBUG;
if (parser_trace>1)
print "[AIMED_MOVE daemon moving ", (the) self, " ", (GiveDir) i,"]^";
#endif;
              if (i==0  || MoveNPCDir(self,i)) ! Routine only called if i~=0
                  self.npc_stage++;
              if (parent(self)==self.npc_target)
                  self.npc_arrived();
              
             2, NO_MOVE :                                ! Not moving at all
#ifdef DEBUG;
if (parser_trace>1)
print "[NO_MOVE daemon for ", (the) self, " doing nothing]^";
#endif;
            
             3, PRESET_MOVE :               ! Moving on a predetermined path
              i=(self.prepath_name)->self.npc_stage;
#ifdef DEBUG;
if (parser_trace>1)
print "[PRESET_MOVE daemon moving ", (the) self," ", (GiveDir) i, "]^";
#endif;
              if (i==0 ||MoveNPCDir(self,i))   ! Routine only called if i~=0
                  self.npc_stage++;
              if (self.npc_stage>=self.prepath_size)
                  self.npc_arrived();
            
       default: "** MoveClass Error: move_type set to an unacceptable
                                     value for ", (the) self, " **";
          }
        ];



     
[ NPC_path npc movement_type targetroom path_type steps i j k found;

  if (metaclass(movement_type)==Object && movement_type ofclass Room)
  {   path_type=targetroom;
      targetroom=movement_type;
      movement_type=AIMED_MOVE;            ! To stay compatible with old code
  }
#ifdef DEBUG;
if (parser_trace>1)
{print "[NPC_Path setting ", (the) self," to ";
 switch (movement_type)
 {     NO_MOVE: print "NO_MOVE";
   RANDOM_MOVE: print "RANDOM_MOVE";
   PRESET_MOVE: print "PRESET_MOVE";
    AIMED_MOVE: print "AIMED_MOVE";
       default: print "**UNDEFINED**";
 }
 print "]^";
}
#endif;
  if (movement_type==NO_MOVE)              ! Call to set NO_MOVE
  {  npc.move_type=NO_MOVE;
     rtrue;
  }

  if (movement_type==RANDOM_MOVE)          ! Call to set RANDOM_MOVE
  {  npc.move_type=RANDOM_MOVE;
     if (path_type~=0)
         npc.caprice=path_type;
     rtrue;
  }

  if (movement_type==PRESET_MOVE)          ! Call to set PRESET_MOVE
     return NPCprepath(npc,targetroom,path_type);

  if (movement_type~=AIMED_MOVE)
     rfalse;
     
                            ! Can only calculate paths from Room class to Room
                            ! class objects, so...
  if (~~(parent(npc) ofclass Room) || parent(npc)==targetroom)
      rfalse;
  objectloop (i ofclass Room)
  {  i.number=0;         
     give i ~en_route;      ! Reset all 'Rooms'
  }
  parent(npc).number=1;     ! Move out from the starting room, labelling each 
  give parent(npc) en_route;! room as en_route and with the number of steps
                            ! until you find the target room.
  for (steps=1: steps<path_size_limit:steps++)
  { objectloop (i has en_route)
    { if (i.number==steps)
      { objectloop (j in Compass)
        { k=LeadsTo(j,i,path_type);
          if (k ofclass Room)
          { give k en_route;
            if (k.number==0)
            {  k.number=steps+1;
#ifdef DEBUG;
if (parser_trace>1)
print "[",(name) k, " is ", steps+1, "]^";
#endif;
            }
            if (k==targetroom)  found=true;
          }
          if (found) break;
        }
      }
      if (found) break;    
    }
    if (found) break;
  }
  if (found==false) rfalse;            ! Must have reached the path_size_limit
      
  objectloop(i has en_route)
    if (i.number>steps && i~=targetroom)
    { i.number=0;                      ! Mark rooms at same distance as the
      give i ~en_route;                ! target room as uninteresting
    }
  
  npc.npc_stage=0;
  npc.move_type=AIMED_MOVE;
  npc.npc_target=targetroom;
  npc.prepath_size=steps;

#ifdef DEBUG;
if (parser_trace>1)
print "[Found path with ",steps, " steps. Now working backwards...
        ^", (name) targetroom;
#endif;

  for ( :steps>0:steps--)              ! Going back one 'step' each time, find
  { found=false;                       ! an interesting room that leads to
    objectloop(i has en_route)         ! the 'step+1' room that we've left
    { if (i.number==steps)
      { objectloop (j in Compass)
        { k=LeadsTo(j,i,path_type);
          if (k has en_route && k.number==steps+1)  found=true;
          if (found) break;
        }
      }
      if (found) break;
    }

#ifdef DEBUG;
if (parser_trace>1)
print " is...^", (GiveDir) j, " of ", (name) i, " which";
#endif;

    npc.&npc_dirs->(k.number-2)=j;     ! Note direction in the npc_dirs array
    objectloop(k has en_route)         ! Mark other rooms with the same number
      if (k.number==steps && i~=k)     ! as uninteresting
      { k.number=0;
        give k ~en_route;
      }
  }

#ifdef DEBUG;
if (parser_trace>1)
print " is where we started!]^";
#endif;

  rtrue;
];



[ NPCPrePath npc path_array path_length fakevar;
  fakevar=fakevar;            ! In case code tries passing a room name too
  if (npc ofclass moveclass)
  { npc.npc_stage=0;
    npc.move_type=PRESET_MOVE;
    npc.prepath_name=path_array;
    npc.prepath_size=path_length;
  }
  else
  { "*** MoveClass Error: NPCPrePath called for non-moveclass object '",
     (the) npc, "' ***";
  }
];



[ LeadsTo direction thisroom path_type k tmp tmp2;
   if (~~(direction provides door_dir)) rfalse;
   if (~~(thisroom provides direction.door_dir)) rfalse;
   k=thisroom.(direction.door_dir);
   if (ZRegion(k)==2)
       k=k();
   if (ZRegion(k)~=1) rfalse;
   if (k has door)
   { if (path_type & DOORLESS_PATH) rfalse;
     if ((path_type & OPEN_PATH) && k hasnt open) rfalse;
     if ((path_type & UNLOCKED_PATH) && k has locked) rfalse;
     tmp=parent(k);
     move k to thisroom;
     tmp2=k.door_to();
     move k to tmp;
     k=tmp2;
   }
   if (~~(k ofclass Room)) rfalse;
   return k;
];



[ MoveNpcDir tomove direction i j p message;
  message=2;
  p=parent(tomove);
  i=LeadsTo(direction,p, ANY_PATH);
  if (i==0)
  { tomove.npc_blocked();
#ifdef DEBUG;
if (parser_trace>1)
print "[MoveNPCDir blocked: Direction leads nowhere]^";
#endif;
    rfalse;
  }
  
  j=p.(direction.door_dir);
  if (ZRegion(j)==2) j=j();
  if (j has door)
  { if (j provides npc_open)          ! npc_open returns
    { message=j.npc_open(self);       ! 2 to go through door as normal
      if (message==false)             ! 1 to go through door and prevent
      { tomove.npc_blocked();         !      walkon/walkoff run/printing
#ifdef DEBUG;                         ! 0 to stop npc using door
if (parser_trace>1)
print "[MoveNPCDir blocked: ", (the) j, "'s npc_open returned false]^";
#endif;
        rfalse;
      }
    }
    else
      if (j hasnt open)
      {   tomove.npc_blocked();
#ifdef DEBUG;
if (parser_trace>1)
print "[MoveNPCDir blocked: ", (the) j, " is closed with no npc_open]^";
#endif;
          rfalse;
      }
  }
          
  MoveNPC(tomove, i, ##Go, direction);
  
  if (p==location && message==2)      ! If npc_open used then it must return 2
  { if (ZRegion(self.walkoff)==3)     ! if it wants walkon/walkoff to execute
        print "^", (The) self, " ", (string) self.walkoff,
              " ", (GiveDir) direction, ".^";
    else
        self.walkoff(direction);
  }
  
  if (parent(self)==location && message==2)
  { if (ZRegion(self.walkon)==3)
      print "^", (The) self, " ", (string) self.walkon, ".^";
    else
      self.walkon(direction);
  }
  
  if (self provides after_action) self.after_action();
  rtrue;
];



Ifndef MoveNPC; ! Provides MoveNPC if program isn't including 'Follower'
[ MoveNPC tomove dest actn objn;
  move tomove to dest;
  actn=actn;
  objn=objn;
];
Endif;



[ GiveDir i;
  switch(i)
  { n_obj: print "to the north";
    s_obj: print "to the south";
    e_obj: print "to the east";
    w_obj: print "to the west";
   ne_obj: print "to the northeast";
   nw_obj: print "to the northwest";
   se_obj: print "to the southeast";
   sw_obj: print "to the southwest";
    u_obj: print "upwards";
    d_obj: print "downwards";
   in_obj: print "inside";
  out_obj: print "outside";
  }
];

