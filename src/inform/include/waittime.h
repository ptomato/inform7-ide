!Modified to work with the 6/3 library by Nicholas Daley
!-------------------------------------------------------------------------
! WaitTime - an Inform library to provide understanding of English time
!            strings, and give an enhanced Wait command. Time games only!
!            #include it just after Grammar.
!            by L. Ross Raszewski, 1996. Based on TimeWait.h by Andrew 
!           Clover.
!The ParseTime and WaitMoves functions are taken from TimeWait with only 
!minimal modification.  I worked this out after I found that I couldn't get
!Andrew's library to work.  I updated some of the code which I believe was, 
!outdated, and changed all the Wait subs so that they call WaitMovesSub.
!tw_waiting is used both as a sort of pseudo-global as Mr. Clover intended,
!and as an indicator.  I set up my programs to update the time through the
!TimePass routine by featuring the line:
!       if (tw_waiting hasnt locked) SetTime(the_time+1);
!       give tw_waiting ~locked;
!If an attempt is made to "Wait 0", or a function is called that is not meant
!to take any time, tw_waiting is given "locked", and the clock is not advanced.
!As with Andrew's TimeWait, any message that should halt the waiting process
!can give tw_waiting the "on" attribute, and discontinue waiting.  TimeWait
!featured a call to Time(), and wouldn't compile until I changed it to 
!TimePasses().  Even when I changed it, the program was still prone to getting
!stuck in infinite loops, which is why I converted the time passage into moves.
!the command "Wait 10" or "Wait 10 minutes" should now be equivalent to the player
!Typing "Wait" ten times.  During testing, I realized that calling TimePasses()
!did not perform all the tasks that occur at the end of the turn, so I changed it 
!again to End_Turn_Sequence().  I think this will make it operate satisfactorily.
!Also, I have added a MAX_WAIT, which should be defined as a constant before 
!inclusion.  Attempts to wait more than that many (minutes) will generate a
!"That's too long" message.  Inspired by "A Mind Forever Voyaging", I added
!a response to "Wait 0".  I'm not sure, but I think having a time passage rate
!other than 0 will give the program problems.  If you want more than 1 minute 
!to pass for each turn, some modification of the code will be necessary (Don't
!ask me how, I'm surprised I even got this far!).
!-------------------------------------------------------------------------
object tw_waiting "tw";
#IFNDEF MAX_WAIT; Constant MAX_WAIT 1000; #ENDIF;
!Action routines for Wait command...

[ WaitMovesSub i;
  if (noun > MAX_WAIT) {give tw_waiting locked; "That's too long to wait.";};
  if (noun==0)
    {print "Time doesn't pass.^";give tw_waiting locked;rtrue;};
  if (noun==1)
    <<Wait>>;
  print "Time passes.^";
  give tw_waiting ~on;
  for (i=noun:(i>1)&&(deadflag==0)&&(tw_waiting hasnt on):i--)
    {
    TimePasses();
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
  if ((tw_waiting has on)&&(i>1)&&(deadflag==0))
    print "^(waiting stopped)^";
];

[ WaitHoursSub;
  noun=noun*60;
  WaitMovesSub();
];

[ WaitUntilSub;
  if (parsed_number>=the_time) noun=parsed_number-the_time;
  if (parsed_number<the_time) 
      {parsed_number=the_time-parsed_number;
      noun=1440-parsed_number;
      print "(tomorrow)^^";
      }
  WaitMovesSub();   
];

!ParseTime takes data from the next words (using wn) and returns a
!the_time format time number, or -1 for unrecognisable. It can recognise
!time expressed in any of the following formats:
!!a. <"0"-"59">|<"one"-"twenty">|"half"|"quarter" ["minutes"|"minute"]
!   "past"|"to" <"1"-"12">|<"one"-"twelve"> ["am"|"pm"]
!b. <"1"-"12">|<"one"-"twelve"> ["o'clock"] ["am"|"pm"]
!c. <"1"-"12">|<"one"-"twelve"> <"0"-"59">|<"one"-"twenty"> ["am"|"pm"]
!d. <"1"-"12">":"<"0"-"59"> ["am"|"pm"]
!e. "midnight"|"midday"|"noon"
!!If no am/pm is specified, the next time likely to come up is chosen; that
!is, the one that's just ahead of the current time. However, if this
!happens, the tw_waiting object is given the 'general' attribute. Thus you
!can change the time returned by twelve hours in an action like SetClock
!if necessary.
!!The next dictionary command is there to allow us to compare a typed
!string with "o'clock", something we can't normally do as it is bounded by
!single quotes.

constant tw_oclock 'o^clock';

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
      if ((j==tw_oclock or -1)||(j=='am' or 'pm'))   ! then case (c) applies
        {
        hr=TryNumber(wn-2);
        mn=0;
        if (j~=tw_oclock)
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
          i
