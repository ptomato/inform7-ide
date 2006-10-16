! physics.h
! Version 1.1
! by Sam Hulick <shulick@indiana.edu>

! This is a slight revision (not by much) of the old physics.inf example
! I released quite awhile ago.  Please read through these comments here
! so you know exactly how it works.

! P.S.: I give  Graham Nelson complete permission to add this into
! his next library release.

! When creating a game using physics.h, you must define a new player
! object, or directly modify the one in parser.h so it includes
! a 'max_space 100', in it, preferably near the 'capacity 100'.
! So if you make your own player obj, be sure to do this as the
! first line in Initialise():
!
!    ChangePlayer(newself);
! followed by..
!    player.max_weight = max_weight_carried;
!    player.max_space = max_space_carried;
!
! Note that max_weight and max_space do NOT work on an object-by-object
! basis, but by weight #'s.  That's the whole point of physics.h.
! Inform's capacity handling routines aren't very detailed, in that you
! could put three large rocks in a bag, and the bag will be full, but
! if you put three small pebbles in the bag, it is strangely full as
! well.  This file will remedy that problem.  When you define objects,
! give it a 'weight' and 'size', i.e.
!
!   Object skull "human skull"
!    with  name "human" "skull",
!          weight 8,
!          size 6,
!          ...
!
! Just figure a decent weight/size scale for your game, and think of
! what size or weight you would give it.  I would give a shovel a
! weight of 10 or 13, perhaps, and a size of 15-20, maybe.  It's
! just estimating.  I give very small objects (pebbles, M&M's, etc.)
! a weight and size of 0, usually, because they're so small.  Or
! you could up-scale your system, making M&M's size/weight both 1,
! then your shovel would more likely have a weight of 30, since you've
! up-scaled your values to compensate for the M&M's.  There are no
! rights or wrongs: it's your game.  I'm just providing the medium
! to control these weights and sizes.

! Now, on to more technical things.  max_weight and max_space are
! used for containers and supporters.  They can be values or
! routines (maybe for magical containers that expand).  I think
! that is self-explanatory.

! too_big and too_heavy are a bit more complicated.  They can simply
! be a string to print to the user when an object is too heavy or
! big to carry, or it can be a routine if you wish.  But on a supporter
! or container, it really MUST be a routine.  When too_heavy or too_big
! is called, 'action' contains either ##Take, ##PutOn, or ##Insert.
! Let's give an example:

! Object mcont "magic container"
!  with  description "You can put things on or in it, ooh.",
!        name "magic" "container",
!        size 5,
!        weight 10,
!        max_weight 30,
!        max_size 30,
!        too_heavy [;
!           switch (action)
!           {
!              ##Take: "The container contains too many heavy things \
!                       for you to handle it.";
!              ##PutOn: "However magical it is, it can't support the \
!                        weight of that.";
!              ##Insert: "That object is too heavy, the magical \
!                         container would rip trying to carry that.";
!           }
!        ], 
!        .....
!
! And too_big works the same way, only it deals with the fact that
! the container can't handle the size of an object.  If  these things
! are unclear to you, you can skim through this file and read the
! source code.  If you do not provide a too_heavy/too_big routine
! or string for an object, default messages will be printed, and
! you don't have to handle examining the 'action' variable.  But
! you will most likely want to handle it yourself, since the default
! messages are.. well.. rather bland.

! IF YOU FIND ANY BUGS, or have any questions, e-mail me at
! shulick@indiana.edu.  Thanks!  Enjoy the code.  Hope it adds some
! interesting depth to your games.

! P.S.: Don't forget to REPLACE RTakeSub, InsertSub, and PutOnSub,
! and include this file after parser and verblib.

Property max_weight alias capacity;    ! max weight something can hold
Property max_space;                    ! max space.....
Property weight;   ! can be a routine.  so can max_weight/max_space
Property size;     ! same for this one
Property too_heavy "That's too heavy for you to carry right now.";
Property too_big "That's too big for you to carry right now.";

! can the carrier take on the weight of obj?  true/false return value
[ OkWeight carrier obj w;
   w = ValueOrRun(obj, weight);
   if ((WeightCarried(carrier) + w) > ValueOrRun(carrier, max_weight)) rfalse;
   rtrue;
];

! can the carrier take on the size of obj?
[ OkSize carrier obj s;
   s = ValueOrRun(obj, size);
   if ((SpaceCarried(carrier) + s) > ValueOrRun(carrier, max_space)) rfalse;
   rtrue;
];

[ WeightCarried obj o total;
   objectloop (o in obj)
   {
      total = total + ValueOrRun(o, weight);
      if (child(o) ~= 0)
         total = total + WeightCarried(o);
   }
   return total;
];

[ SpaceCarried obj o total;
   objectloop (o in obj)
      total = total + ValueOrRun(o, size);
   return total;
];

!! The reason SpaceCarried() isn't recursive is because it merely doesn't
! have to be.  WeightCarried() must be recursive, because if you put a
! huge heavy steel ball into a light sack and carry the sack, this doesn't
! mean the sack relieves your weight.  However, if you can manage to fit a
! very large object into a small sack, then your troubles are reduced. (a
! good example would be a huge Nerf(TM) beachball that is tough to carry
! around, but if you find a small, light box, you can open the box, stuff
! the Nerf(TM) ball into it, and close the box.  The weight carried is
! still the box plus the ball, but now your space carried has reduced.

[ AdviseMoveIt obj;
   print "Try moving some things off ", (the) obj; ".";
];

[ AdvisePullIt obj;
   print "Try removing some things from ", (the) obj; ".";
];

[ RTakeSub fromobj i j k postonobj a rval;
  if (noun==player) return L__M(##Take,2);

  if (noun has animate) return L__M(##Take,3,noun);

  if (parent(player)==noun) return L__M(##Take,4,noun);

  i=parent(noun);
  if (i==player) return L__M(##Take,5);

  if (i has container || i has supporter)
  {   postonobj=i;
      k=action; action=##LetGo;
      if (RunRoutines(i,before)~=0) { action=k; rtrue; }
      action=k;
  }

  while (i~=fromobj && i~=0)
  {   if (i hasnt container && i hasnt supporter)
      {   if (i has animate) return L__M(##Take,6,i);
          if (i has transparent) return L__M(##Take,7,i);
          return L__M(##Take,8);
      }
      if (i has container && i hasnt open)
          return L__M(##Take,9,i);
      i=parent(i);
      if (i==player) i=fromobj;
  }
  if (noun has scenery) return L__M(##Take,10);
  if (noun has static)  return L__M(##Take,11);

  ! If it's too heavy, don't even bother with SACK_OBJECT. just because
  ! we put something in a sack doesn't make it any lighter.
  if (OkWeight(player, noun) == 0)
  {
     a = action;
     action = ##Take;
     rval = PrintOrRun(noun, too_heavy, 0);
     action = a;
     return rval;
  }

  if (OkSize(player, noun) == 0)
  {
      if (SACK_OBJECT~=0)
      {   if (parent(SACK_OBJECT)~=player)
              return ObjTooBig(noun);
          j=0;
          objectloop (k in player) 
              if (k~=SACK_OBJECT && k hasnt worn && k hasnt light) j=k;

          if (j~=0)
          {   L__M(##Take,13,j);
              keep_silent = 1; <Insert j SACK_OBJECT>; keep_silent = 0;
              if (j notin SACK_OBJECT) rtrue;
          }
          else return ObjTooBig(noun);
      }     
      else return ObjTooBig(noun);
  }
  move noun to player;

  if (postonobj~=0)
  {   k=action; action=##LetGo;
      if (RunRoutines(postonobj,after)~=0) { action=k; rtrue; }
      action=k;
  }
  rfalse;
];

[ ObjTooBig obj a rval;
   a = action;
   action = ##Take;
   rval = PrintOrRun(obj, too_big, 0);
   action = a;
   return rval;
];

[ PutOnSub a;
  receive_action=##PutOn; 
  if (second==d_obj) { <Drop noun>; rfalse; }
  if (parent(noun)~=player) return L__M(##PutOn,1,noun);

  if (second>1)
  {   action=##Receive;
      if (RunRoutines(second,before)~=0) { action=##PutOn; rtrue; }
      action=##PutOn;
  }

  if (IndirectlyContains(noun,second)==1) return L__M(##PutOn,2);
  if (second hasnt supporter) return L__M(##PutOn,3,second);
  if (parent(second)==player) return L__M(##PutOn,4);
  if (noun has worn)
  {   L__M(##PutOn,5);
      <Disrobe noun>;
      if (noun has worn) rtrue;
  }
  if (OkSize(second, noun) == 0)
  {
     a = action;
     action = ##PutOn;
     if (RunRoutines(second, too_big) == 0)
     {
        ! default message
        print_ret (The) noun, " won't fit on ", (the) second, ".";
     }
     action = a;
     if (ValueOrRun(noun, size) <= ValueOrRun(second, max_space))
        return AdviseMoveIt(second);
     rtrue;
  }
  if (OkWeight(second, noun) == 0)
  {
     a = action;
     action = ##PutOn;
     if (RunRoutines(second, too_heavy) == 0)
     {
        print_ret "You feel ", (the) second, " begin to give as you start to put ",
              (the) noun, " on it.  Maybe this isn't such a good idea.";
     }
     action = a;
     if (ValueOrRun(noun, weight) <= ValueOrRun(second, max_weight))
        return AdviseMoveIt(second);
     rtrue;
  }

  move noun to second;

  if (AfterRoutines()==1) rtrue;

  if (second>1)
  {   action=##Receive;
      if (RunRoutines(second,after)~=0) { action=##PutOn; rtrue; }
      action=##PutOn;
  }

  if (keep_silent==1) rtrue;
  if (multiflag==1) return L__M(##PutOn,7);
  L__M(##PutOn,8,noun);
];

[ InsertSub a;
  receive_action = ##Insert;
  if (second==d_obj ) <<Drop noun>>;
  if (parent(noun)~=player) return L__M(##Insert,1);

  if (second>1)
  {   action=##Receive;
      if (RunRoutines(second,before)~=0) { action=##Insert; rtrue; }
      action=##Insert;
  }
  if (second hasnt container) return L__M(##Insert,2);
  if (second hasnt open)      return L__M(##Insert,3);
  if (IndirectlyContains(noun,second)==1) return L__M(##Insert,5);
  if (noun has worn)
  {   L__M(##Insert,6);
      <Disrobe noun>; if (noun has worn) rtrue;
  }

  if (OkSize(second, noun) == 0)
  {
     a = action;
     action = ##Insert;
     if (RunRoutines(second, too_big) == 0)
     {
        print_ret "There's not enough room inside ", (the) second, " for ",
              (the) noun, ".";
     }
     action = a;
     if (ValueOrRun(noun, size) <= ValueOrRun(second, max_space))
        return AdvisePullIt(second);
     rtrue;
  }
  if (OkWeight(second, noun) == 0)
  {
     a = action;
     action = ##Insert;
     if (RunRoutines(second, too_heavy) == 0)
     {
        print_ret (The) second, " won't hold anymore weight.";
     }
     action = a;
     if (ValueOrRun(noun, weight) <= ValueOrRun(second, max_weight))
        return AdvisePullIt(second);
     rtrue;
  }

  move noun to second;

  if (AfterRoutines()==1) rtrue;

  if (second>1)
  {   action=##Receive;
      if (RunRoutines(second,after)~=0) { action=##Insert; rtrue; }
      action=##Insert;
  }
  if (keep_silent==1) rtrue;
  if (multiflag==1) return L__M(##Insert,8);
  L__M(##Insert,9,noun);
];
