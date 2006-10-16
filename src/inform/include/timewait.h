! ------------------------------------------------------ -*- Inform -*- ----
!  TimeWait - an Inform library to provide understanding of English time
!             strings, and give an enhanced Wait command. Time games only!
!
!             Include it just after Grammar.
!             (c) Andrew Clover, 1995, but freely usable. Release 3.
!             Compatible with Inform 5.5, library 5/12.
! --------------------------------------------------------------------------

! The tw_waiting object is a dodgy way of having a global variable without
! using a global (which we can't because we're #included at an inconvenient
! time). If any importantish messages appear, you should give this object
! 'on', and any multiple waits will stop. This is intended for events such
! as "Bob threatens to kill you unless you give him a fish within the next
! three moves", but not each turn messages like "The radio blares out a
! Hammond organ rendition of the Mr. Blobby song". The only point at which
! it is essential you set this is if you move the clock backwards, or
! forwards 24 hours or more.

object tw_waiting "tw";

! Action routines for Wait command...

[ WaitMovesSub i;
  if (noun==0)
    "I don't understand how to wait that long.";
  L__M(##Wait);
  give tw_waiting ~on;
  for (i= noun: (i>1) && (deadflag==0) && (tw_waiting hasnt on): i--)
    {
    turns--;
! #ifdef InformLibrary.End_Turn_Sequence;
    InformLibrary.End_Turn_Sequence();
! #ifnot;
! #ifdef EndTurnSequence;
!    EndTurnSequence();
! #ifnot;
!    Message fatalerror "timewait.h requires \
!                        InformLibrary.End_Turn_Sequence() or \
!                        EndTurnSequence() to be defined (this should be done \
!                        by the Inform Library)";
! #endif;
!#endif;
    DisplayStatus();
    #IFV5;
    DrawStatusLine();
    #ENDIF;
    }
  if ((tw_waiting has on) && (i>1) && (deadflag==0))
    {
    print "^(waiting stopped)^";
    meta= 1;
    }
];

[ WaitMinsSub i;
  noun= (the_time+noun);
  i=noun / 1440;
  parsed_number=noun % 1440;
  WaitUntilSub(i+1);
];

[ WaitHoursSub;
  noun= noun*60;
  WaitMinsSub();
];

[ WaitUntilSub i j;
  parsed_number=(parsed_number-1);
  if (parsed_number<0)
    parsed_number=parsed_number+1440;
  parsed_number=parsed_number % 1440;
  if (i==0)
    {
    if (parsed_number<the_time)
      {
      print "(tomorrow)^^";
      i=1;
      }
    }
  else
    i=i-1;
  L__M(##Wait);
  give tw_waiting ~on;
  while (((the_time<parsed_number)||(i>0))&&(tw_waiting hasnt on)&&(deadflag==0))
    {
    j=the_time;
    turns--;
!#ifdef InformLibrary.End_Turn_Sequence;
    InformLibrary.End_Turn_Sequence();
!#ifnot;
! #ifdef EndTurnSequence;
!    EndTurnSequence();
! #ifnot;
!    Message fatalerror "timewait.h requires \
!                        InformLibrary.End_Turn_Sequence() or \
!                        EndTurnSequence() to be defined (this should be done \
!                        by the Inform Library)";
! #endif;
!#endif;
    if (the_time<j)
      {
      i=i-1;
      }
    DisplayStatus();
    #IFV5;
    DrawStatusLine();
    #ENDIF;
    }
  if ((tw_waiting has on)&&(deadflag==0))
    {
    print "^(waiting stopped)^";
    meta= 1;
    }
];

! ParseTime takes data from the next words (using wn) and returns a
! the_time format time number, or -1 for unrecognisable. It can recognise
! time expressed in any of the following formats:

! a. <"0"-"59">|<"one"-"twenty">|"half"|"quarter" ["minutes"|"minute"]
!    "past"|"to" <"1"-"12">|<"one"-"twelve"> ["am"|"pm"]
! b. <"1"-"12">|<"one"-"twelve"> ["o'clock"] ["am"|"pm"]
! c. <"1"-"12">|<"one"-"twelve"> <"0"-"59">|<"one"-"twenty"> ["am"|"pm"]
! d. <"1"-"12">":"<"0"-"59"> ["am"|"pm"]
! e. "midnight"|"midday"|"noon"

! If no am/pm is specified, the next time likely to come up is chosen; that
! is, the one that's just ahead of the current time. However, if this
! happens, the tw_waiting object is given the 'general' attribute. Thus you
! can change the time returned by twelve hours in an action like SetClock
! if necessary.

! The next dictionary command is there to allow us to compare a typed
! string with "o'clock", something we can't normally do as it is bounded by
! single quotes.

[ ParseTime i j k flg loop dig hr mn;
  give tw_waiting ~general;
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
      if ((j=='o^clock' or -1)||(j=='am' or 'pm'))   ! then case (c) applies
        {
        hr=TryNumber(wn-2);
        mn=0;
        if (j~='o^clock')
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
          give tw_waiting general;
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

[ ParseFor i;
  i= NextWord();
  if (i~='for')
    wn--;
  return 0;
];

! Now the grammar for the new Wait actions.  You can use parsetime in other
! new actions. For example, you could perhaps allow setting of watches,
! etc. with grammar like:

! extend "set" first
!     * is_timepiece "to" parsetime -> SetClock;

! You may also like to extend wait to allow "Wait for <something>" extend
! this "first" to make this take precedence over "Wait for <time>" (the
! parsefors in the grammar line represent 0 or 1 occurances of the word
! 'for').

extend "wait"
    * "until" parsetime           -> WaitUntil
    * "til" parsetime             -> WaitUntil
    * "till" parsetime            -> WaitUntil
    * number                      -> WaitMoves
    * parsefor number "minute"    -> WaitMins
    * parsefor number "minutes"   -> WaitMins
    * parsefor number "hour"      -> WaitHours
    * parsefor number "hours"     -> WaitHours;

