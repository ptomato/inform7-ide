! ---------------------------------------------------------------------------
! calyx_adjectives - This Inform library extension is heavily based on Andrew
!       Clover's adname.h (which in turn is based on an example in the Inform
!       Designers Manual).
!       Differences: * The adname property is additive.
!                    * The main Class is called Ad_Item instead of adnc.
!                    * Objects can be referred-to by an adname alone. The
!                      price to pay is that you have to take care of the
!                      actual difference yourself (see example below).
!       Release 1.
!
!       Include this library before you define your objects.
!       Written in 1997 by Miron Schmidt / Calyx. Placed in the
!       Public Domain. Actually, I've only given it a new name at all because
!       it is incompatible to Andrew's library.
!
!       Example:
!       First, define an object like this:
!               Ad_Item cigarette_lighter "cigarette lighter"
!                 with name 'lighter',
!                      adname 'cigarette',
!                      ...
!                 has ...;
!       Then, in ChooseObjects(), put in the following code:
!               [ ChooseObjects obj code prio;
!                       if (code <2) { Your code }
!                       prio=2;
!                       ...
!                       if (obj has adj_chosen)  return prio;
!                       return prio+10;
!               ];
!       You can define your own ChooseObjects() priorities, but an object
!       that was referred-to by its (noun) name should always have a higher
!       priority than an object that was referred-to by an adname only.
!
!       adj_chosen is an attribute that is set if the player referred to an
!       object by adnames only (e.g. 'cigarette' in the example).
!       Thus, a cigarette would be automatically chosen if it were in the
!       same room as the lighter, but in other circumstances, 'cigarette'
!       could also refer to the lighter.
! ---------------------------------------------------------------------------

System_file;

Property additive adname;
Attribute adj_chosen;                   ! This is set if the object was
                                        ! chosen because of an adname.
                                        ! You may use this knowledge in a
                                        ! ChooseObjects routine.

Class Ad_Item
  with
    adname 'the',                       ! If adname is 0, program crashes!
    parse_name [;
      return AdnameParser(self);        ! Calling another routine like this
                                        ! avoids excessive code duplication,
                                        ! and allows you to extend an
                                        ! object's parse_name routine whilst
                                        ! still calling the adname parser.
    ]
  has adj_chosen;

[ AdnameParser adobj w n i j a b succ fail;
  if (parser_action==##TheSame) {
    w=parser_one.&adname;
    n=(parser_one.#adname)/2;
    i=parser_two.&adname;
    j=(parser_two.#adname)/2;
    for (a=0:a<n:a++) {
      fail=1;
      for (b=0:b<j:b++) {
        if ((w-->a)==(i-->b))
          fail=0;
      }
      if (fail==1)  return -2;
    }
    for (a=0:a<j:a++) {
      fail=1;
      for (b=0:b<n:b++) {
        if ((w-->b)==(i-->a))
          fail= 0;
      }
      if (fail==1)  return -2;
    }
    return 0;   ! This bit adapted from the parser - check all words in
                ! adname of parser_one to see if they occur in adname of
                ! parser_two, then vice-versa. If either case is false, we
                ! say the objects are not the same (the player may type a
                ! word that distiguishes then); if both are true, we give the
                ! parser a crack at seeing if they're different (by looking
                ! at the name property).
  }
  else {
    n=0;
    succ=0;
    fail=0;
    while (fail==0) {
      fail=1;
      w=NextWord();
      for (i=0:i<(adobj.#adname)/2:i++) {
        if (w==adobj.&adname-->i) {
          fail=0;
          succ=1;
          give adobj adj_chosen;
        }
      }
      for (i=0:i<(adobj.#name)/2:i++) {
        if (w==adobj.&name-->i) {
          fail=0;
          succ=1;
          give adobj ~adj_chosen;
        }
      }
      n++;
    }
    if (succ==1)  return n-1;
    return 0;
                ! This is the bit of code executed normally (when the parser
                ! isn't trying to resolve identical objects). We just check
                ! that every word typed is in the adname or name property,
                ! and say that the phrase matches the object if any words are
                ! in the name.
  }
];
