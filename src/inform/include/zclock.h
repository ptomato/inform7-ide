! zclock.h
! A nearly exact port of the daemons used in Infocom games.
! Ported by Allen Garvin, December 6, 2003
! Use freely!

! Some useful constants. Their names come from the ZIL source for minizork
Constant C_ENABLED 0;
Constant C_TICK    1;
Constant C_RTN     2;
Constant C_INTLEN  6;

! Modify this to increase the number of possible daemons.
! It must be a multiple of C_INTLEN (by default,  there can be 30 daemons)
Constant C_TABLELEN 180;

Array	C_table	--> C_TABLELEN;
Global	C_ints = C_TABLELEN;

! Queue is the programmer's interface to the daemons. It takes 2 arguments.
!    rtn:    The routine to be queued as a timer or daemon
!    ticks:  If positive, the timer countdown length. After 'ticks' turns
!               rtn will be called.
!            If negative, then behave like an Inform daemon. 'rtn' will be
!               called every turn.
!	     If zero, the daemon or timer will be disabled.
! A queued event must be explicitly enabled. The return value of the
!   function is the interrupt array for the event. If the first byte is
!   set to 1, it will be enabled. If 0, it will be disabled.
!   Thus:
!          Queue(Foo, 5)-->C_ENABLED = true;      Call 'Foo' in 5 turns
!          Queue(Bar, -1)-->C_ENABLED = true;     Call 'Bar' every turn
!	   Queue(Biff, 20);	Place 'Biff' in the queue, but do not start
!                               the timer yet. The timer will not decrement.
!                               No real purpose I can see, but Infocom did
!                               this frequently in their Main routines
!	   Queue(Foo); or Queue(Foo, 0);          Stop the 'Foo' timer
[ Queue rtn ticks cint ;
  cint = QueueInterrupt(rtn);
  cint-->C_TICK = ticks;
  StartDaemon(zork_daemon);
  return cint;
];

! QueueInterrupt was rarely called directly in Infocom games
[ QueueInterrupt rtn end c int ;
  end = C_table + C_TABLELEN;
  c = C_table + C_ints;
  while( true ) {
    if( c ~= end ) {
      if( (c-->C_RTN) == rtn )
        return c;
      c = c + C_INTLEN;
    } else {
      C_ints = C_ints - C_INTLEN;
      int = C_table + C_ints;
      int-->C_RTN = rtn;
      return int;
    }
  }
];

! I decided to implement the queue by using an object with an Inform daemon.
Object 	zork_daemon "Daemon"
  with	name 'zdaemon',
	daemon [ c end tick flag;
	  c = C_table + C_ints;
	  end = C_table + C_TABLELEN;
	
	  while( true ) {
	    if( c == end ) {
	      return flag;
	    }
	    if( c-->C_ENABLED ) {
              tick = c-->C_TICK;
              if( tick ~= 0 ) {
                c-->C_TICK = (tick - 1);
                if( tick <= 1 && (c-->C_RTN)() )
                  flag = 1;
	      }
	    }
            c = c + C_INTLEN;
	  }
	];

! Notes:
! Once a routine is queued, there is no method to remove it from the
!   queue. Thus, there is an absolute number of queues in a game. But
!   Requeueing a routine will use the same slot, instead of using a new one.
! If you queue a routine with a negative value, there is a bug. When 32767
!   turns have passed, the number will overflow to positive, and the queue
!   will then be a countdown instead of a daemon event. Though easy to fix
!   I decided to leave it bug-compatible with Infocom games. In Zork I, for
!   instance, after 2^15 turns the thief will quit moving around the maze.
!   And neither the thief nor troll will defend themselves when attacked.
