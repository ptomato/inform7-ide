! ---------------------------------------------------------------------------- !
!   Daemons.h by Andrew Plotkin and Roger Firth (roger.firth@tesco.net,
!                                   to whom all problems should be notified).
!
!       2.0 Oct 2001
!           Converted to Inform 6 and Glulx by Roger Firth.
!
!       1.0 Oct 1995
!           Original version for Inform 5.5 by Andrew Plotkin.
!
!   This file is in the public domain.
! ---------------------------------------------------------------------------- !
!   Installation: add the lines:
!
!       Replace StartDaemon;
!       Replace StartTimer;
!
!   before your Include "Parser" line. Also, add the line:
!
!       Include "daemons";
!
!   near the end of your game AFTER you've defined an object which uses a
!   "daemon_priority" property.
!
! ---------------------------------------------------------------------------- !
!
!   This is a package which you can include to make the daemons and timers of
!   the Inform libraries behave predictably. They will execute in the order
!   defined by the optional "daemon_priority" property of each object in the
!   daemon/timer list. Higher priorities go first; if priorities are equal,
!   they execute in the order in which they were started up. The default
!   daemon_priority is zero, but you can assign any numerical value to the
!   property, including negative ones.
!
!   There are a few restrictions:
!
!   - daemon_priority must be a simple variable containing a number; it cannot
!     be an embedded routine which returns a number.
!   - you should assign a new value to a daemon_priority property only while
!     the object's daemon/timer is *not* running. This means that if an object
!     has both a daemon and a timer, they must run at the same priority.
!   - it is illegal to call StartDaemon() or StartTimer() from inside a
!     daemon or timer routine. It *is* legal to call StopDaemon() or
!     StopTimer() at any time.
!
!   A bonus utility routine IsDaemonActive(obj) returns 1 if obj currently has
!   an active daemon, 2 for an active timer, 3 for both, 0 for neither.
!
!   Here is an example daemon:
!
!   Object  myDaemon
!     with  daemon_priority 1000,
!           daemon [;
!               ...
!               ];
!
! ---------------------------------------------------------------------------- !

#ifndef WORD_HIGHBIT;               ! Is already defined in Glulx compiler.
Constant WORD_HIGHBIT $8000;        ! Topmost bit in Z-machine word, used by
#endif;                             ! the Library to flag daemons in the_timers.

!   GetPriority() is a utility routine which masks out the daemon-or-timer flag
!   and fetches the value of the daemon_priority property.
!
[ GetPriority obj;
    obj = obj & ~WORD_HIGHBIT;
    if (obj provides daemon_priority) return obj.daemon_priority;
    return 0;
    ];

!   StartDaemonOrTimer() is a utility routine which inserts the given object
!   in the_timers list, at the position defined by its daemon_priority. That is,
!   the object is inserted after all objects with a greater-or-equal priority,
!   and before all objects with a lesser priority.
!
[ StartDaemonOrTimer obj
    i j;

    ! If obj already in the list, do nothing
    for (i=0 : i<active_timers : i++)
        if (the_timers-->i == obj) rfalse;

    ! Compress the list, removing empty slots
    for (i=0,j=0 : i<active_timers : i++)
        if (the_timers-->i) {
            if (i > j) {
                the_timers-->j = the_timers-->i;
                the_timers-->i = 0;
                }
            j++;
            }

    ! Check for capacity to run another daemon/timer
    active_timers = j;
    if (active_timers == MAX_TIMERS) { RunTimeError(4); rfalse; }

    ! Find the first slot at a lower priority
    for (i=0 : i<active_timers : i++)
        if (GetPriority(the_timers-->i) <  GetPriority(obj)) break;

    ! Shift upwards that slot and those above it
    for ( : j>i : j--) the_timers-->j = the_timers-->(j-1);

    ! Insert the new object
    the_timers-->i = obj;
    return ++active_timers;
    ];

!    Replacement for the standard Library version of StartDaemon().
!
[ StartDaemon obj;
    StartDaemonOrTimer(obj+WORD_HIGHBIT);
    ];

!    Replacement for the standard Library version of StartTimer().
!
[ StartTimer obj timer;
    if (obj.&time_left == 0) { RunTimeError(5, obj); return; }
    if (StartDaemonOrTimer(obj)) obj.time_left = timer;
    ];

! ---------------------------------------------------------------------------- !

System_file;

!   IsDaemonActive() tests whether an object's daemon and/or timer is
!   currently scheduled to run.
!
[ IsDaemonActive obj
    i j;
    for (i=0,j=0 : i<active_timers : i++) {
        if (the_timers-->i == obj) j = 2;
        else if (the_timers-->i == obj + WORD_HIGHBIT) j++;
        }
    return j;
    ];

! ---------------------------------------------------------------------------- !

