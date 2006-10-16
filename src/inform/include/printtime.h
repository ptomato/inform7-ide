!! --Copyright Notice--				-*- Inform -*-
!! printtime.h version 30 Sept 1996
!! Copyright 1996 Erik Hetzner
!! This program (can it be called that?) is released in the hopes
!! that it will be useful, but without a warranty of any kind.
!! It is distributed under the GNU General Public License.
!! This license is available at `ftp://prep.ai.mit.edu/pub/gnu/COPYING'
!! or by writing to:
!!   Free Software Foundation, Inc.
!!   675 Mass. Ave.
!!   Cambridge, MA 02139, USA
!!
!! --Usage--
!! This library provides three functions: `AnalogTime', `DigitalTime',
!! and `MilitaryTime' which can be called in print statements like:
!!
!! print "The time is: " (DigitalTime) the_time ".^";
!!
!! This will print the time stored in the variable the_time.  This
!! number can be the game time, which is stored in the variable
!! the_time, or any other number or variable. For it to make sense,
!! though, the number must be in the form: (Hours * 60 + Minutes).
!! Enjoy!
!! -Erik Hetzner <egh@raisin.bagel.org> 30 Sept 1996

Constant midnight 0;
Constant noon 12;

[Hour hr;
    if (hr == midnight)
	print (number) 12;
    else if (hr == noon)
	print (number) 12;
    else
    	print (number) hr%12;
];

[SupHour hr;
    if (hr == midnight)
	print "midnight";
    else if (hr == noon)
	print "noon";
    else
	print (number) hr%12;
];

[AnalogTime time hr mn suffix past til;
    hr = (time/60); ! set the hour
    mn = (time%60); ! set the minute
    if ((mn == 15) || (mn == 30)) ! Will the time be
	past = true;                        ! displayed using `past'?
    if ((mn == 45) || (mn == 50)) ! Will it use
	til = true;                         ! `'til'?	      
    if (til) ! if the time is 'til something, what is is til
	hr = (hr+1)%24;  ! 'til is one hour ahead--compensate for going
                         ! past midnight by remaindering by 24.
    
    if (hr == midnight && mn == 0) ! is it midnight?
	{
    	    print "midnight"; ! the print it
    	    rtrue;            ! and exit
	}
    
    if (hr == noon && mn == 0) ! is it noon?
    {
	print "noon"; ! then print it
	rtrue;        ! and exit
    }

    if (hr > midnight && hr < noon )                   ! Is it morning?
	suffix = " in the morning";          ! then make it the suffix.
    else if (hr > noon && hr < 18)             ! Perhaps the afternoon?
	suffix = " in the afternoon";
    else if (((hr == midnight || hr == noon) && past) || ! if it's midnight
	     ((hr == midnight || hr == noon) && til))    ! or noon we don't
	suffix = "";                         ! use a suffix.
    else
	suffix = " at night";	   
    
    if (til)
    {
	if (mn == 45)
	    print "quarter 'til ", (SupHour) hr, (string) suffix;
    	else if (mn == 50)
	    print "ten 'til ", (SupHour) hr, (string) suffix;
    }
    else if (past)
    {
	if (mn == 15)
	    print "quarter past ", (SupHour) hr, (string) suffix;
    	else if (mn == 30)
	    print "half past ", (SupHour) hr, (string) suffix;
    }
    else
    {
	if (mn == 0)
	    print (Hour) hr, " o'clock", (string) suffix;
    	else if (mn > 0 && mn < 10)
	    print (Hour) hr, " oh ", (number) mn, (string) suffix;
    	else if (mn >= 10)
	    print (Hour) hr, " ", (number) mn, (string) suffix;
    }
];    

[MilitaryTime time;
    if (time/60 < 10)   ! Do we need a hr preceeding `0'?
    	print "0", time/60; ! then print one
    else
	print time/60;  ! or don't
    if (time%60 < 10)   !do we need a mn preceeding `0'?
    	print "0", time%60; ! then print one
    else
    	print time%60;  ! or don't
];

[DigitalTime time sep hr mn;
    if (time/60 == midnight || time/60 == noon) ! is it 12?
	hr = 12;  ! then let it be
    else
	hr = (time/60%12); ! or just set the hour...
    mn = (time%60);        ! and set the minute
    if (mn < 10)           ! do we need a mn preceeding `0'?
	sep = ":0";        ! then add one
    else
	sep = ":";         ! or don't
    if ((the_time/60)/12 == 0) ! is it AM?
	print hr, (string) sep, mn, " AM"; 
    else ! or pm?
	print hr, (string) sep, mn, " PM";
];
