!====================================================================================
! TIMESYS 1.0 	INFORM TIME-ORIENTED GAME EXTENSION
!
!			An Inform library to provide an understanding of English time
!                	strings, and give an enhanced Wait command.
! 
!             (c) Kevin Forchione, 1998, but freely usable.
!             Compatible with Inform 6.15, library 6/7.
!                Based on WaitTime.h by L. Ross Raszewski, 1996 
!                     and TimeWait.h by Andrew Clover, 1995
!			with much appreciation for their efforts.
!
! IMPLEMENTATION REQUIREMENTS:
!
!     Time-oriented games only
!	Replace DrawStatus; and set constant to STYLE you wish (see STYLES below)	
!     Replace WaitSub(); (the replacement routine is already provided in the
!		extension.
!     Change WAIT verb to META in GRAMMAR.H
!     #include TimeSys.h somewhere after #include "Grammar";
!	Set the day-of-the-week (if desired) in Initialise() with 
!		TimeSys('day'); (see below)
!	Day-of-week advancement (if desired) At the top of TimePasses(); 
!		call TimeSys.NextDay() for automatic handling of day-of-week advancement.
!
! COMMENTS:
! 
!	This is hardly an ideal solution (That being an overhaul of the 
!	InformLibrary itself to extend its time-keeping capabilities.)
!
!	Nonetheless, by making the WAIT command META we take control out of the 
!	InformLibrary's Turn__end process and allow the extension to process
! 	Waiting() and StopWaiting() requests without falling back into final execution 
!	of InformLibrary.End_Turn_Sequence, which causes problems with StopWaiting() 
!	requests. (i.e. the request stops the extension at the desired time, but 
!	InformLibrary.Turn__end gets the final word and bumps the time up one more step.) 
!
!	DRAWBACKS: However, there are some drawbacks...
!
!	Unfortunately the functions of the TimeSys object are completely dependent 
!	upon being called through the wait subroutines as were those of it's predecessor 
!	tw_waiting (It's never in scope). 
!
!	Making the WAIT verb META compounds the difficulty in generation of actions 
!	to an object's before() and Orders(). 
!
!	ENHANCEMENTS: The TimeSys object has been significantly enhanced over 
!			  the old tw_waiting object. 
!
!	The variables in TimeSys have all been made private. Access methods 
!	It now contains coding for the advancement of the day-of-week, which should
!	be called from TimePasses() with TimeSys.NextDay().
!
!	The redundant call to TimePasses() in the Waiting process that occurred in 
!	WaitTime.h has been removed and this process moved to the TimeSys object 
!	instead of being handled in a standalone routine. In addition this means 
!	that the wait process is effected only once each cycle by  the 
!	InformLibrary.End_Turn_Sequence(). 
!
!    	You can set a wait default (independent of time_step) which will allow you
!	to wait a default number of minutes (such as in Infocom's "Sherlock: The Riddle
!	of the Crown Jewels"). 
!
!	The non-waiting time_rate is kept independent of the waiting process, so 
!	that you can SetTime() for any rate you wish for the normal (non-waiting)
!	passage of time. Waiting converts minutes to turns (as WaitTime.h did), 
!	then when the waiting process is finished the original non-waiting time_rate
!	is restored.
!
!	Other features are documented below.
!          
!====================================================================================

Object TimeSys "TimeSys"
   has proper
   private 
   TS_day				0		! Day-of-the-week numeric value			
   ,TS_WaitDefault 		10		! Default value for Wait with no operand
   ,TS_WaitMax	 		1440		! Maximum number of minutes allowed in a Wait
   ,TS_wait_time_initial 	0		! Number of minutes to be waited
   ,TS_wait_time_remaining 	0		! Number of minutes for waiting remaining
   ,TS_normal_time_rate 	0		! Saved value of non-wait game time-rate
   ,TS_prev_time_save		0		! Saved value used to determine new day
   with name "TimeSys"

   ,Waiting
	[ i;
  		if (noun > self.TS_WaitMax) {give self locked; "That's too long to wait.";};
  		if (noun==0) {give self locked; "Time doesn't pass.^";};
  		print "Time passes...^";
  		give self on;
  		self.TS_wait_time_initial=noun;
  		self.TS_wait_time_remaining=noun;
  		self.TS_normal_time_rate=time_rate;
  		SetTime(the_time,1);
 
  		for (i=noun : (i>0)&&(deadflag==0)&&(self has on) : i--)
    		{

			self.TS_wait_time_remaining--;

		#ifdef InformLibrary;
    			InformLibrary.End_Turn_Sequence();
		#ifnot;
 		#ifdef EndTurnSequence;
    			EndTurnSequence();
 		#ifnot;
    		Message fatalerror "waittime.h requires \
                        InformLibrary.End_Turn_Sequence() or \
                        EndTurnSequence() to be defined (this should be done \
                        by the Inform Library)";
 		#endif;
		#endif;
    			DisplayStatus();
    		#IFV5;
    			DrawStatusLine();
    		#ENDIF;
    		}
  		SetTime(the_time,self.TS_normal_time_rate);
  		if ((self hasnt on)&&(self.TS_wait_time_remaining>0)&&(deadflag==0))
    		   print "^(waiting stopped)^";
	]

!--------------------------------------------------------------------------------------
! This routine allows the author to code StopWaiting requests for any event s/he chooses.
! It also give the player the choice of continued waiting if they wish.
!----------------------------------------------------------------------------------------
   ,StopWaiting
	[; 
	   if (self.TS_wait_time_remaining==0) rfalse;

	   DisplayStatus();
    	   #IFV5;
    	   DrawStatusLine();
    	   #ENDIF;

	   print "^Do you want to continue waiting?";
  	   if (YesOrNo()==0) give self ~on;
	]

   ,SetDefault
	[ d; self.TS_WaitDefault=d;
	]

   ,GetDefault
	[; return self.TS_WaitDefault;
	]

!-----------------------------------------------------------------------------------------
! This function allows the author to set the game day using the name for the
! day-of-the-week, instead of having to use its numeric value. This allows TimeSys
! to keep this variable private.
!-----------------------------------------------------------------------------------------
   ,SetDay
	[ d;
		switch(d)
		{ 'Sunday','sunday','Sun','sun',"Sunday","sunday","Sun","sun": 
			self.TS_day=0;
		  'Monday','monday','Mon','mon',"Monday","monday","Mon","mon": 
			self.TS_day=1;
		  'Tuesday','tuesday','Tue','tue',"Tuesday","tuesday","Tue","tue": 
			self.TS_day=2;
		  'Wednesday','wednesday','Wed','wed',"Wednesday","wednesday","Wed","wed": 
			self.TS_day=3;
		  'Thursday','thursday','Thu','thu',"Thursday","thursday","Thur","thur": 
			self.TS_day=4;
		  'Friday','friday','Fri','fri',"Friday","friday","Fri","fri": 
			self.TS_day=5;
		  'Saturday','saturday','Sat','sat',"Saturday","saturday","Sat","sat": 
			self.TS_day=6;
		  default: self.TS_day=d;
		};
	]

!-----------------------------------------------------------------------------------------
! This function allows time-oriented games to have the day-of-the-week pushed forward with
! the passage of time. Although it goes beyond the scope of purely 'waiting' it aspires to
! a more mature time-keeping system we can only now dream of.
!
! This should be called from TimePasses() as TimeSys.NextDay() for automatic advancement.
!------------------------------------------------------------------------------------------
   ,NextDay
	[;
         if (the_time < self.TS_prev_time_save) {self.TS_day++;};
         if (self.TS_day > 6) {self.TS_day=0;};
         self.TS_prev_time_save=the_time;
	]
  
!------------------------------------------------------------------------------------------
! This routine returns either the number value or the associated name for the 
! day_of_the_week, and can be included in any StatusLine routine that needs to know the 
! this information
!------------------------------------------------------------------------------------------
   ,GetDay
	[ f i; 
	   if (f==0) return self.TS_day;
	   if (f==1)
	   switch(self.TS_day)
		{ 0: i="Sunday   ";
		  1: i="Monday   ";
		  2: i="Tuesday  ";
		  3: i="Wednesday";
		  4: i="Thursday ";
		  5: i="Friday   ";
		  6: i="Saturday ";
		}
	   else
	   switch(self.TS_day)
		{ 0: i="Sun";
		  1: i="Mon";
		  2: i="Tue";
		  3: i="Wed";
		  4: i="Thu";
		  5: i="Fri";
		  6: i="Sat";
		};
	   return i;
	];

!------------------------------------------------------------------------------------------
! STATUSLINE STYLES
!
!	Some of the styles listed below mimic styles from classic Infocom games 
!	(versions may differ). To select a style of statusline simply define the 
!	constant listed below as one of the following:
!
!	BASIC_STYLE: Mimics statuslines of the following:
!		Cutthroats 						Release 23 / Serial number 840809
!		Deadline: An Interlogic Mystery		Release 27 / Serial Number 831005
!		MoonMist						Release  9 / Serial Number 861022
!		Suspect: An Interactive Mystery		Release 14 / Serial Number 841005
!		The_Witness: An Interlogic Mystery		Release 22 / Serial Number 840924
!		Wishbringer: The Magick Stone of Dreams	Release 69 / Serial Number 850920
!
!	STANDARD_STYLE: Displays day/time information
!
!	FULL_STYLE: Displays day/time/score information 
!
!	SHERLOCK_STYLE: Mimics statuslines of Infocom's Sherlock game, which used
!		an hh:mm:ss format, even though the game didn't handle seconds:
!		Sherlock: The Riddle of the Crown Jewels	Release 21 / Interpreter 6 / 
!									Version j / Serial Number 871214 	
!			
!===========================================================================================

Constant FULL_STYLE;

#IFDEF BASIC_STYLE;
[ DrawStatusLine i width pos;
   @split_window 1; @set_window 1; @set_cursor 1 1; style reverse;
   width = 0->33;
   if (width == 0) width = 80;
   spaces (width-1);
   @set_cursor 1 2;  PrintShortName(location);
   pos = width-19;
   @set_cursor 1 pos;
   print "Time: ";
   i=sline1%12; if (i<10) print " ";
   if (i==0) i=12;
   print i, ":";
   if (sline2<10) print "0";
   print sline2;
   if ((sline1/12) > 0) print " pm"; else print " am";
   @set_cursor 1 1; style roman; @set_window 0;
];
#ENDIF; ! BASIC_STYLE

#IFDEF STANDARD_STYLE;
[ DrawStatusLine i width pos;
   @split_window 1; @set_window 1; @set_cursor 1 1; style reverse;
   width = 0->33;
   if (width == 0) width = 80;
   pos = width-22;
   spaces (width-1);
   @set_cursor 1 2;  PrintShortName(location);
   @set_cursor 1 pos;
   print (string) TimeSys.GetDay(1);
   i=sline1%12; if (i<10) print " ";
   if (i==0) i=12;
   print i, ":";
   if (sline2<10) print "0";
   print sline2;
   if ((sline1/12) > 0) print " pm"; else print " am";
   @set_cursor 1 1; style roman; @set_window 0;
];
#ENDIF; !STANDARD_STYLE

#IFDEF FULL_STYLE;
[ DrawStatusLine i width pos;
   @split_window 1; @set_window 1; @set_cursor 1 1; style reverse;
   width = 0->33;
   if (width == 0) width = 80;
   spaces (width-1);
   @set_cursor 1 2;  PrintShortName(location);
   pos = width-41;
   @set_cursor 1 pos;
   print (string) TimeSys.GetDay(1); 
   i=sline1%12; if (i<10) print " ";
   if (i==0) i=12;
   print i, ":";
   if (sline2<10) print "0";
   print sline2;
   if ((sline1/12) > 0) print " pm"; else print " am";
   pos = width-10;
   @set_cursor 1 pos; print "Score: ", score;
   @set_cursor 1 1; style roman; @set_window 0;
];
#ENDIF; ! FULL_STYLE

#IFDEF SHERLOCK_STYLE;
[ DrawStatusLine i width pos;
   @split_window 1; @set_window 1; @set_cursor 1 1; style reverse;
   width = 0->33;
   if (width == 0) width = 80;
   spaces (width-1);
   @set_cursor 1 2;  PrintShortName(location);
   pos = width-41;
   @set_cursor 1 pos;
   print (string) TimeSys.GetDay(1);
   i=sline1%12; if (i<10) print " ";
   if (i==0) i=12;
   print i, ":";
   if (sline2<10) print "0";
   print sline2;
   if ((sline1/12) > 0) print ":00 p.m."; else print ":00 a.m.";
   pos = width-10;
   @set_cursor 1 pos; print "Score: ", score;
   @set_cursor 1 1; style roman; @set_window 0;
];
#ENDIF; ! SHERLOCK_STYLE

!------------------------------------------------------------------------------------------
!Action routines for Wait command...
!------------------------------------------------------------------------------------------

[WaitSub;
  noun=TimeSys.GetDefault();
  TimeSys.Waiting();
];

[ WaitMovesSub; TimeSys.Waiting(); ];

[ WaitHoursSub;
  noun=noun*60+second;
  TimeSys.Waiting();
];

[ WaitUntilSub;
  if (parsed_number>=the_time) noun=parsed_number-the_time;
  if (parsed_number<the_time) 
      {parsed_number=the_time-parsed_number;
      noun=1440-parsed_number;
      }
  if (noun==0) {noun=1440; print "(tomorrow)^^";};

  TimeSys.Waiting();   
];

!-----------------------------------------------------------------------------------
! ParseTime takes data from the next words (using wn) and returns a
! the_time format time number, or -1 for unrecognisable. It can recognise
! time expressed in any of the following formats:
!
! a. <"0"-"59">|<"one"-"twenty">|"half"|"quarter" ["minutes"|"minute"]
!    "past"|"to" <"1"-"12">|<"one"-"twelve"> ["am"|"pm"]
! b. <"1"-"12">|<"one"-"twelve"> ["o'clock"] ["am"|"pm"]
! c. <"1"-"12">|<"one"-"twelve"> <"0"-"59">|<"one"-"twenty"> ["am"|"pm"]
! d. <"1"-"12">":"<"0"-"59"> ["am"|"pm"]
! e. "midnight"|"midday"|"noon"
!    If no am/pm is specified, the next time likely to come up is chosen; that
!    is, the one that's just ahead of the current time. However, if this
!    happens, the TimeSys object is given the 'general' attribute. Thus you
!    can change the time returned by twelve hours in an action like SetClock
!    if necessary.
!
!    The next dictionary command is there to allow us to compare a typed
!    string with "o'clock", something we can't normally do as it is bounded by
!    single quotes.
!-----------------------------------------------------------------------------------

constant TS_OCLOCK 'o^clock';

[ ParseTime i j k flg loop dig hr mn;
  give TimeSys ~general;
  i=NextWord();
  if (i=='midday' or 'noon' or 'midnight')           ! then case (e) applies
    {
    if (i=='midnight')
      hr=0;
    else
      hr=12;
    mn=0;
    parsed_number=(hr*60+mn);
    }
  else
    {
    k=(wn-1)*4+1;                                    ! test for case (d)
    j=parse->k;
    j=j+buffer;
    k=parse->(k-1);
    flg=0;
    for (loop=0:loop<k:loop++)
      {
      dig=j->loop;
      if (dig==':')
        flg=1;
      }
    if ((k>2)&&(k<6)&&(flg==1))                      ! then case (d) applies
      {
      hr=0;
      mn=0;
      loop=0;
      .tw_diglph;
      dig=j->loop;
      loop++;
      if (dig~=':')
        hr=hr*10;
      if (dig=='0') { hr=hr+0; jump tw_diglph; }
      if (dig=='1') { hr=hr+1; jump tw_diglph; }
      if (dig=='2') { hr=hr+2; jump tw_diglph; }
      if (dig=='3') { hr=hr+3; jump tw_diglph; }
      if (dig=='4') { hr=hr+4; jump tw_diglph; }
      if (dig=='5') { hr=hr+5; jump tw_diglph; }
      if (dig=='6') { hr=hr+6; jump tw_diglph; }
      if (dig=='7') { hr=hr+7; jump tw_diglph; }
      if (dig=='8') { hr=hr+8; jump tw_diglph; }
      if (dig=='9') { hr=hr+9; jump tw_diglph; }
      if (dig~=':') return -1;
      while (loop<k)
        {
        dig=j->loop;
        mn=mn*10;
        if (dig=='0') { mn=mn+0; jump tw_digokm; }
        if (dig=='1') { mn=mn+1; jump tw_digokm; }
        if (dig=='2') { mn=mn+2; jump tw_digokm; }
        if (dig=='3') { mn=mn+3; jump tw_digokm; }
        if (dig=='4') { mn=mn+4; jump tw_digokm; }
        if (dig=='5') { mn=mn+5; jump tw_digokm; }
        if (dig=='6') { mn=mn+6; jump tw_digokm; }
        if (dig=='7') { mn=mn+7; jump tw_digokm; }
        if (dig=='8') { mn=mn+8; jump tw_digokm; }
        if (dig=='9') { mn=mn+9; jump tw_digokm; }
        return -1;
        .tw_digokm;
        loop++;
        }                                            ! decode digital time
      }
    else
      {
      j=NextWordStopped();
      if ((j==TS_OCLOCK or -1)||(j=='am' or 'pm'))   ! then case (c) applies
        {
        hr=TryNumber(wn-2);
        mn=0;
        if (j~=TS_OCLOCK)
          wn--;
        }
      else
        {
        k=TryNumber(wn-1);
        if (k~=-1000)                                ! then case (b) applies
          {
          mn=k;
          hr=TryNumber(wn-2);
          }
        else                                         ! well, must be case (a)
          {
          mn=TryNumber(wn-2);
          if (i=='quarter')
            mn=15;
          if (i=='twenty-five')
            mn=25;
          if (i=='half' or 'thirty')
            mn=30;
          if (j=='minute' or 'minutes')
            j=NextWord();                            ! ignore 'minutes'
          hr=TryNumber(wn);
          wn++;
          if (j~='past' or 'to')
            hr=-1;
          if (j=='to')
            {
            hr--;
            mn=60-mn;
            if (hr==0)
              hr=12;
            }
          }
        }
      }
    if ((hr>12)||(hr<1)||(mn>59)||(mn<0))
      parsed_number=-1;
    else
      {
      if (hr==12)                                    ! now sort out am/pm
        hr=0;
      i=NextWord();
      if (i=='pm')
        hr=hr+12;
      else
        if (i~='am')                                 ! am or pm implied, then?
          {
          give TimeSys general;
          wn--;
          i=(hr*60+mn);
          j=((hr+12)*60+mn);
          i=i-the_time;
          j=j-the_time;
          if (i<0)
            i=i+(24*60);
          if (j<0)
            j=j+(24*60);
          if (i>j)
            hr=hr+12;
          }
      parsed_number=(hr*60+mn);
      }
    }
  if (parsed_number==-1)
    return -1;
  else
    return 1;
];

!----------------------------------------------------------------------------
! Now the grammar for the new Wait actions.
! You can use parsetime in other new actions. For example, you could perhaps
! allow setting of watches, etc. with grammar like:
! extend "set" first
!     * is_timepiece "to" parsetime -> SetClock;
!----------------------------------------------------------------------------

extend 'wait' replace
    *					    						-> Wait
    * 'until'/'til'/'till'/'for' parsetime           			-> WaitUntil
    * 'for' number 'hour'/'hours'						-> WaitHours
    * 'for' number 'minute'/'minutes'					-> WaitMoves
    * 'for' number 'hour'/'hours' number 'minute'/'minutes'		-> WaitHours
    * 'for' number 'hour'/'hours' 'and' number 'minute'/'minutes' -> WaitHours
    * number 'minute'/'minutes'             				-> WaitMoves
    * number 'hour'/'hours'                           		-> WaitHours
    * number                      						-> WaitMoves
    * number 'hour'/'hours' number 'minute'/'minutes' 		-> WaitHours
    * number 'hour'/'hours' 'and' number 'minute'/'minutes' 	-> WaitHours
    * parsetime                   						-> WaitUntil;
