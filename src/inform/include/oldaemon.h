!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!  oldaemon                     The Objectloop Daemon
!  Version 1.0                  By L. Ross Raszewski
!  12.2.2002                    (rraszews@hotmail.com)
!  For Z-code and Glulx
!  No external requirements
!
! It is a well known fact that one of the major causes of game sluggishness
! is use of the general-form objectloop.
! The general form objectloop is any objectloop construct other than
! (x in y), (x from y), or (x near y).
!
! If a game uses the general-form objectloop within a daemon or each_turn
! construct, the latency between turn cycles is substantially increased.
! Inform's standard library already performs one such objectloop
! (In 'MoveFloatingObjects')
!
! The purpose of the Objectloop Daemon is to reduce the number of times
! that an objectloop must be executed by combining a number of tasks into
! a single loop.
!
! To use the objectloop daemon, simply insert the line:
! StartDaemon(loop_daemon);
! In your Initialise function. This will activate the daemon.
! Now, instead of performing an objectloop in the context of a daemon
! or each_turn, place an object inside the loop_daemon object to
! perform the task.  objectloop daemon events have the following structure:
!
! Object loop_task
! with
!   exec [ o;
!        ! Required. When the daemon is run, the exec function will be
!        !   executed for every object in the game
!        !   The exec method corresponds to the body of the original
!        !   objectloop
!       ],
!   init [;
!       ! Optional. If provided, this method is called before looping begins
!       ],
!   fini [;
!       ! Optional. If provided, this method is called after looping ends
!       ],
!   pred [ o;
!       ! Optional. If provided, this is called for every object immediately
!       ! before exec. It should return false to exclude the object, true to
!       ! include it. The pred method corresponds to the loop condition.
!       ! If no pred is included, the loop is equivalent to "loop over
!       ! all objects"
!       ];
!
! Here is an example of a loop task:
!  This code shows a daemon which sets the 'general' attribute on every
!  object of class 'Foo' once per turn. If several such daemons exist,
!  they might produce a substantial lag. The objectloop daemon version
!  will perform only one objectloop, regardless of how many tasks are
!  assigned.
!  ...
!  daemon [o;
!            objectloop(o ofclass Foo)
!               give o general;
!       ],
! ...
!
! And now the objectloop daemon version:
!
! object flag_set loop_daemon
!  with pred [o; return (o ofclass Foo); ],
!       exec [o; give o general; ];
!
! Placing a task inside the loop_daemon enables it -- the task will be run
! the next time daemons are executed. Removing a task disables it.
!
! Caveats:
! 1. Removing a task
!    It is safe to remove a task from the objectloop daemon at any time.
!    The task will stop immediately. Of course, disabling a task does not
!    undo work already done. For reference, here is the effect of removing
!    a task at various times:
!    When the daemon is not currently executing:
!                         The task will not be executed when the daemon starts
!    Within the task's 'init':
!                         The task will not be executed on this turn
!    Within the task's 'fini':
!                         The task has already been done for this turn, but
!                         will not be done on subsequent turns
!    Within an 'exec':  The task will not be applied to any more objects.
!                       This is roughly equivalent to performing a 'break'
!                       within a normal objectloop.
!                       To allow a task to break in the middle, but to make it
!                       continue to run on the next turn, it is
!                       necessary to re-insert it (via a daemon of its own).
!                       'fini' will not be run for this task.
!   Within a 'pred': If the pred function returns true, the task will
!                       *still be run* on the current object, but will not
!                       be run on any other objects.
!
! 2. Interlacing. In a traditional demon-based objectloop, the body of each
!                 loop is executed once for every object consecutively.
!                 If there are two such loops, the first loop is executed for
!                 every object, then the second loop is executed for every
!                 object. Within the objectloop daemon, the order is changed
!                 slightly. Tasks are interlaced. That is, every task is run
!                 for the first object, then every task is run for the second
!                 object. This may be important if the body of the loop makes
!                 use of global variables.
! 3. Scope. Since 'exec' is excecuted fresh for each object, you cannot use
!           local variables to collect information over the course of an
!           objectloop. The recommended solution to this is to attach
!           properties to the task which are used in place fo local variables.
!           You must then also use the .init() routine to reset these
!           properties each time
! 4. Context. The loop_daemon is run in the context of a daemon. Consult the
!        inform designer's manual for a discussion of where exactly daemons
!        are executed during the course of a turn cycle.  Individual tasks are
!        executed in lineage order. Using 'move x to loop_daemon' will make
!        x the first task to be executed. For finer control, use pmove and
!        rmove (in Utility.h)
! 5. Speed. The loop daemon imposes some overhead over a standard objectloop.
!           It is not sensible to use it to replace a single objectloop.
!           However, it will perform at most one objectloop per turn, regardless
!           of how many tasks are active. Therefore, if there is more than one
!           such loop, the daemon will save time. The amount of time it saves
!           is proportional to the total number of objects in the game.
!           If no tasks are active, the daemon does not perform the objectloop.
!
! The author would appreciate any and all commentary. This library extention
! is free for use in all non-commercial and competition games.
! Permission is required (and will almost certainly be forthcoming) for
! commercial releases.
!
ifndef OLDAEMON_LIBRARY;
Constant OLDAEMON_LIBRARY 32;

Object loop_daemon "Objectloop Daemon"
with
 init 0, fini 0, exec 0, pred OLDAEMON_LIBRARY,
 daemon
 [x o y;
       for(x=child(self):x:x=y)
       {
        y=sibling(x);
        if (x provides init)
         x.init();
       }
       if (~~child(self)) return;
       objectloop(o)
       if (~~child(self)) return; else
       for(x=child(self):x:x=y)
       {
        y=sibling(x);
        if (~~(x provides pred) || x.pred(o))
        x.exec(o);
       }
       for(x=child(self):x:x=y)
       {
        y=sibling(x);
        if (x provides fini)
        x.fini();
       }
    ];

endif;
