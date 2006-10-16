!! --Copyright Notice--				-*- Inform -*-
!! timepiece.h 30 Sept 1996 Copyright 1996 Erik Hetzner <egh@raisin.bagel.org>
!! This program (can it be called that?) is released in the hopes
!! that it will be useful, but without a warranty of any kind.
!! It is distributed under the GNU General Public License.
!! This license is available at `ftp://prep.ai.mit.edu/pub/gnu/COPYING'
!! or by writing to:
!!   Free Software Foundation, Inc.
!!   675 Mass. Ave.
!!   Cambridge, MA 02139, USA
!!
!! --Intro--
!! timepiece.h is a library that provides a number of class for
!! implementing timepieces; that is, watches, clocks, & all the
!! like. It uses my very own printtime.h library, and L. Ross
!! Raszewski's timewait.h library. It has the following features:
!!  * provides for clocks that can display time in military, analog
!!    or twelve hour format. This is performed by an after routine
!!    for examine, so that sfter your desciption, the time is printed.
!!  * timepieces can be set to any time, even a time different from the
!!    games time. If the tp_canset attribute is set, the piece can
!!    be set to any time, and the time still advances at the same
!!    time as game time.
!!  * timepieces can run fast or slow; this is controlled by the
!!    tp_mn and tp_mn_per properties--the time piece adds tp_mn
!!    to its time every tp_mn_per minutes. If tp_mn is negative, the
!!    timepiece loses time, and vice-versa.
!!  * timepieces can be syncronized (set to each others values).
!!  * certain timepieces can be used to change the game time; ie:
!!    when one sets them, one sets the time for the game. Also, if
!!    these pieces run fast or slows, the games time does to. (Kind
!!    of silly & pointless, but at least it's complete! :)
!!
!! A number of classes are provided, mostly examples. TimepieceClass
!! is the basic one, and PocketwatchClass, WristwatchClass,
!! DigtialWatchClass and ClockClass are also provided. WristwatchClass
!! is just a basic wristwatch gains a minute every hour,
!! DigitalWatchClass is a wristwatch that can be switched from
!! military to digital time, and ClockClass is a game time clock that
!! can be set, altering the game time.
!!
!! --Usage--
!! Examine the (commented) class definitions to learn how to create
!! stunning timepieces. :) Don't forget to call
!! StartDaemon(some_timepiece) when you initialize if you want the
!! clock to lose or gain time.
!!  with tp_type [ military | analog | digital ]
!!       tp_dev (deviation from game time--this is an internal variable,
!!               you probably ought not to set it yourself.)
!!       tp_mn (minutes to change)
!!       tp_mm_per (per these minutes)
!!  has tp_canset   ! this timepiece can be set
!!      tp_gametime ! this timepiece keeps game time
!!      timepiece   ! this is a timepiece
!! StartDaemon(some_timepiece) ! MUST be called if you want tp_mn and
!!                             ! tp_mn_per to work.
!! Constant GAMESPEED number;  ! sets the pace of the game; used
!!                             ! when calling SetTime(). See inform
!!                             ! designer's manual for more
!!                             ! details.
!!
!! --Todo--
!! * Fix any bugs (it hasn't been tested that well).
!! * Add fine tuning for clocks: ie, set them an hour ahead, an hour
!!   behind, a minute forward, &c.
!! * Allow clocks rate to be adjusted--ie allow the player to change the
!!   tp_mn and tp_mn_per properties.
!! * Find a useful application for this... :)
!! * If you have any suggestions or bug reports, please please please
!!   (write to me) at egh@raisin.bagel.org. This is my first inform
!!   project, so the code's probably a little funky.

Include "waittime";
Include "printtime";

Ifndef GAMESPEED;
Constant GAMESPEED 1;
Endif;

Constant Military 0;
Constant Analog 1;
Constant Digital 2;

Property tp_type;
Property tp_dev;
Property tp_mn; ! minutes to change (+/-)
Property tp_mn_per; ! per minutes.

Attribute timepiece;
Attribute tp_gametime;
Attribute tp_canset;

Class 	TimepieceClass
 with	before
	[;
	 SetTo:
	    if (self has tp_canset)
    	    {
		if (second has timepiece)
		{
		    parsed_number = (second.tp_dev + the_time);
		}
	    	if (self has tp_gametime)
	    	{
	      	    SetTime((parsed_number), GAMESPEED);
		    print "The game's time has been set to ", (AnalogTime) the_time, ".^";
		    self.tp_dev = 0;
		    rtrue;
	    	}
	    	else
		{
    		    self.tp_dev = (parsed_number - the_time);  
		    print (The) self, " has been set to ",
			(AnalogTime) ((self.tp_dev + the_time)%(60*24));
		    rtrue;
		}
	    }   
	    else
	    {		
		print (The) self, " is set right; if you do that, \
		    you'll just lose all sense of time.^";
	      	rtrue;
	    }
	],
	after
	[;
 	 Examine:
	    switch (self.tp_type)
	    {
 	     military:
   	    	print (The) self, " reads ", (MilitaryTime)
		    ((self.tp_dev + the_time)%(60*24)), ".^";
 	     analog:
	    	print (The) self, " reads ", (AnalogTime)
		    ((self.tp_dev + the_time)%(60*24)), ".^";
 	     digital:    
		print (The) self, " reads ", (DigitalTime)
		    ((self.tp_dev + the_time)%(60*24)), ".^";
	    }
	],
	daemon
	[;
	  if ((the_time%60)%self.tp_mn_per == 0) 
	      self.tp_dev = self.tp_dev + self.tp_mn;
	    if (self has tp_gametime)
    		SetTime((self.tp_dev + the_time, GAMESPEED)); 
	],
	tp_dev 0,
	tp_mn 0,
	tp_mn_per 0,
 has 	timepiece;

! This pocketwatch can be opened and closed.
Class 	PocketwatchClass
class timepieceClass
 with 	after
	[;
	 Examine:
	    if (self has open)
	    {}
	    else
	    {
		print (The) self, " is closed.^";
	    	rtrue; 
	    }
	],
	tp_type analog,
 has	openable;

! This is an inaccurate wristwatch.
Class 	WristwatchClass
 class TimepieceClass
 with 	tp_type analog,
	tp_mn 1,      ! gains one minute
    	tp_mn_per 60, ! per hour.
 has 	tp_canset       ! we can set this watch right.
	    clothing;   ! we can wear it.

! This clock keeps the game time.
Class 	ClockClass
 class 	TimepieceClass
 with 	tp_type analog,
 has 	static
	    tp_gametime        ! This is the master clock; it keeps the game time.
	    tp_canset;         ! A magic clock; it can change the game time.


! This watch can switch from military to digital time at the press of
! a button.
Class 	DigitalWatchClass
 class 	TimepieceClass
 with 	before [;
	 Push:
	    if (self.tp_type == digital)
	    {
		self.tp_type = military;
		print "The watch switches to military time.^";
	    }
	    else
	    {
		self.tp_type = digital;
		print "The watch switches to 12 hour time.^";
	    }
	    rtrue;
	],
	tp_type digital,
 has 	clothing; ! can be worn

Extend "set" first
    * timepiece "to" ParseTime		-> SetTo
    * timepiece "to" timepiece 		-> SetTo;

Verb "syncronize" "sync"
    * timepiece "to" timepiece 		-> SetTo;

