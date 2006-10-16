! ----------------------------------------------------------------------------
! PluralObj - an Inform library to allow plural nouns. For example, a pair of
!             gloves, some adverts, or a pile of magazines. Give an object the
!             attribute 'pluralobj' and the correct messages will appear.
!
!             #include it just before VerbLib.
!             (c) Andrew Clover, 1995, but freely usable. Release 2.
!             Compatible with Inform 5.5, library 5/12
! ----------------------------------------------------------------------------

! NB. This library provides LibraryMessages in order to change the library
!     responses to plural-aware ones. If you want to provide your own library
!     messages, call the LibraryMessages object LibraryMessages2.

! This library gives you some commands to use instead of just printing 'it' or
! such like. They are itthey, itthem, doesntdont, isntarent, thatthose,
! thisthese (lower case initials); ittheyc, itstheyrec, thatthosec,
! thisthesec, thatstheyre, thattheyc (upper case). Also verbsuf, which just
! prints 's' for singular-form verbs. The commands also print the correct
! pronoun for animates. One parameter is necessary, an object number to check
! for pluralobj. With anything else, your printing commands will have to check
! for the pluralobj flag themselves.

attribute pluralobj;

object LibraryMessages "lm"
  with before [ i;
         #IFDEF LibraryMessages2;
           i=runroutines(LibraryMessages2,before);
           if (i~=0)
             return i;
         #ENDIF;
         i=action;            ! pointless statements to stop a compiler warning
         i=action;            ! 'unused variable' appearing if lm2 object absent
         Take:
           if (lm_n==5)
             {
             print "You already have ";
             thatthose(noun);
             ".";
             }
           if (lm_n==6)
             {
             thatthosec(noun);
             print " seem";
             verbsuf(noun);
             " to belong to ";
             defart(lm_o);
             ".";
             }
           if (lm_n==7)
             {
             thatthosec(noun);
             print " seem";
             verbsuf(noun);
             " to be a part of ";
             defart(lm_o);
             ".";
             }
           if (lm_n==8)
             {
             thatthosec(noun);
             print " ";
             isntarent(noun);
             " available.";
             }
           if (lm_n==10)
             {
             thatstheyrec(noun);
             " hardly portable.";
             }
           if (lm_n==11)
             {
             thatstheyrec(noun);
             " fixed in place.";
             }
         Drop:
           if (lm_n==2)
             {
             print "You haven't got ";
             thatthose(noun);
             ".";
             }
         Remove:
           if (lm_n==1)
             {
             ittheyc(second);
             print " ";
             if (second has pluralobj)
               print " are";
             else
               print " is";
             " unfortunately closed.";
             }
           if (lm_n==2)
             {
             print "But ";
             itthey(noun);
             print " ";
             isntarent(noun);
             " there now.";
             }
         PutOn:
           if (lm_n==5)
             {
             print "(first taking ";
             itthem(noun);
             " off)^";
             }
         Insert:
           if (lm_n==1)
             {
             print "You need to be holding ";
             itthem(noun);
             print " before you can put ";
             itthem(noun);
             " into something else.";
             }
           if (lm_n==2)
             {
             thattheyc(second);
             " can't contain things.";
             }
           if (lm_n==3)
             {
             print "Alas, ";
             itthey(second);
             if (second has pluralobj)
               print " are";
             else
               print " is";
             " closed.";
             }
           if (lm_n==4)
             {
             print "You'll need to take ";
             itthem(second);
             " off first.";
             }
           if (lm_n==6)
             {
             print "(first taking ";
             itthem(noun);
             " off)^";
             }
         Transfer:
           if (lm_n==1)
             {
             thatthose(noun);
             print " ";
             isntarent(noun);
             " in your possession.";
             }
           if (lm_n==2)
             {
             print "First pick ";
             itthem(noun);
             " up.";
             }
         Enter:
           if (lm_n==2)
             {
             thatstheyrec(noun);
             " not something you can enter.";
             }
         Search:
           if (lm_n==5)
             {
             print "You can't see inside, since ";
             itthey(noun);
             if (noun has pluralobj)
               print " are";
             else
               print " is";
             " closed.";
             }
           if (lm_n==6)
             {
             cdefart(lm_o);
             if (second has pluralobj)
               print " are";
             else
               print " is";
             " closed.";
             }
         Unlock:
           if (lm_n==1)
             {
             thattheyc(noun);
             print " ";
             doesntdont(noun);
             " seem to be something you can unlock.";
             }
           if (lm_n==2)
             {
             itstheyrec(noun);
             " unlocked at the moment.";
             }
           if (lm_n==3)
             {
             thatthosec(second);
             print " ";
             doesntdont(second);
             " seem to fit the lock.";
             }
         Lock:
           if (lm_n==1)
             {
             thattheyc(noun);
             print " ";
             doesntdont(noun);
             " seem to be something you can lock.";
             }
           if (lm_n==2)
             {
             itstheyrec(noun);
             " locked at the moment.";
             }
           if (lm_n==3)
             {
             print "First you'll have to close ";
             itthem(noun);
             ".";
             }
           if (lm_n==4)
             {
             thatthose(second);
             print " ";
             doesntdont(second);
             " seem to fit the lock.";
             }
         SwitchOn:
           if (lm_n==1)
             {
             thatstheyrec(noun);
             " not something you can switch.";
             }
           if (lm_n==2)
             {
             thatstheyrec(noun);
             " already on.";
             }
         SwitchOff:
           if (lm_n==1)
             {
             thatstheyrec(noun);
             " not something you can switch.";
             }
           if (lm_n==2)
             {
             thatstheyrec(noun);
             " already off.";
             }
         Open:
           if (lm_n==1)
             {
             thatstheyrec(noun);
             " not something you can open.";
             }
           if (lm_n==2)
             {
             ittheyc(noun);
             print " seem";
             verbsuf(noun);
             " to be locked.";
             }
           if (lm_n==3)
             {
             itstheyrec(noun);
             " already open.";
             }
         Close:
           if (lm_n==1)
             {
             thatstheyrec(noun);
             " not something you can close.";
             }
           if (lm_n==2)
             {
             itstheyrec(noun);
             " already closed.";
             }
         Disrobe:
           if (lm_n==1)
             {
             print "You're not wearing ";
             thatthose(noun);
             ".";
             }
         Wear:
           if (lm_n==1)
             {
             print "You can't wear ";
             thatthose(noun);
             "!";
             }
           if (lm_n==2)
             {
             print "You're not holding ";
             itthem(noun);
             ".";
             }
           if (lm_n==3)
             {
             print "You're already wearing ";
             thatthose(noun);
             "!";
             }
         Eat:
           if (lm_n==1);
             {
             thatstheyrec(noun);
             " plainly inedible.";
             }
         Cut:
           print "Cutting ";
           thatthose(noun);
           " would achieve little.";
         Blow:
           print "You can't usefully blow ";
           thatthose(noun);
           ".";
         Set:
           print "No, you can't set ";
           thatthose(noun);
           ".";
         SetTo:
           print "No, you can't set ";
           thatthose(noun);
           " to anything.";
         Wave:
           if (lm_n==1)
             {
             print "But you aren't holding ";
             thatthose(noun);
             ".";
             }
         Push, Pull, Turn:
           if (lm_n==1)
             {
             itstheyrec(noun);
             " fixed in place.";
             }
       ];

[ itthey i;
  if (i has pluralobj)
    print "they";
  else
    {
    if (i has animate)
      {
      if (i has female)
        print "she";
      else
        print "he";
      }
    else
      print "it";
    }
];

[ thattheyc i;
  if (i has pluralobj)
    print "They";
  else
    {
    if (i has animate)
      {
      if (i has female)
        print "She";
      else
        print "He";
      }
    else
      print "That";
    }
];

[ itthem i;
  if (i has pluralobj)
    print "them";
  else
    {
    if (i has animate)
      {
      if (i has female)
        print "her";
      else
        print "him";
      }
    else
      print "it";
    }
];

[ ittheyc i;
  if (i has pluralobj)
    print "They";
  else
    {
    if (i has animate)
      {
      if (i has female)
        print "She";
      else
        print "He";
      }
    else
      print "It";
    }
];

[ itstheyrec i;
  if (i has pluralobj)
    print "They're";
  else
    {
    if (i has animate)
      {
      if (i has female)
        print "She's";
      else
        print "He's";
      }
    else
      print "It's";
    }
];

[ thatstheyrec i;
  if (i has pluralobj)
    print "They're";
  else
    {
    if (i has animate)
      {
      if (i has female)
        print "She's";
      else
        print "He's";
      }
    else
      print "That's";
    }
];

[ doesntdont i;
  if (i hasnt pluralobj)
    print "doesn't";
  else
    print "don't";
];

[ isntarent i;
  if (i hasnt pluralobj)
    print "isn't";
  else
    print "aren't";
];

[ thatthose i;
  if (i hasnt pluralobj)
    print "that";
  else
    print "those";
];

[ thatthosec i;
  if (i hasnt pluralobj)
    print "That";
  else
    print "Those";
];

[ thisthese i;
  if (i hasnt pluralobj)
    print "this";
  else
    print "these";
];

[ thisthesec i;
  if (i hasnt pluralobj)
    print "This";
  else
    print "These";
];

[ verbsuf i;
  if (i hasnt pluralobj)
    print "s";
];
