! ---------------------------------------------------------------------------- !
!   Shuffle.h
!       original 1.0  Sep00 by Roger Firth (roger.firth@tesco.net) for Inform 6
!
! ---------------------------------------------------------------------------- !
!   Installation: add the line:
!
!       Include "Shuffle";
!
!   anywhere in your game AFTER the Include "Parser" statement.
!
! ---------------------------------------------------------------------------- !
!   Shuffle() is like random(), in that it returns a random number in the
!   range 1..N. Unlike random, it returns a different value on each call,
!   until all N possible numbers have been used, at which point it starts
!   a new cycle. It's like dealing a shuffled pack of cards: the Two of Clubs
!   might show up at any time, but can appear only once per deal.
!
!   Shuffle() uses arrays of bit flags to track the numbers which have been
!   used within a cycle. A default set of 56 flags is supplied, so that if you
!   use the routine only once within your program, and you need shuffled numbers
!   only in the range 1..56, then all you need do is to call the routine.
!   For example, to simulate dealing a pack of cards, you might write:
!
!       for (i=0 : i<52 : i++) {
!           j = Shuffle(52) - 1;
!           suit = j/13;        ! 0,1,2,3
!           rank = 1 + j%13;    ! 1,2, ... 13
!           ...
!           }
!
!   More commonly, however, you'll need to use Shuffle() for several different
!   purposes, and here you are advised to create separate flag arrays for each.
!   Declare the arrays as in these examples:
!
!       Array mySmallFlags  -> 2;   ! for up to 8 flags
!       Array myBiggerFlags -> 3;   ! for up to 16 flags
!       Array myMediumFlags -> 5;   ! for up to 32 flags
!       Array myLargeFlags  -> 33;  ! for up to 256 flags
!
!   (That is, the data structure is a byte array in which the first byte holds
!   a count 0..255, and the remaining bytes each supply eight flag bits.
!   Note that if you need to generate shuffled numbers in a range greater than
!   1..256, you must modify the routine so that the count is held as a word.)
!   Then, supply the array name as the second parameter to Shuffle():
!
!       j = Shuffle(7, mySmallFlags);
!       j = Shuffle(20, myMediumFlags);
!
!   The application for which Shuffle() was written was to randomise NPC
!   responses in a number of default situations. For example:
!
!       Array   old_man_give -> 2;
!       Array   old_man_show -> 2;
!       Array   old_man_tell -> 2;
!
!       Object  old_man "old man"
!         has   animate
!         with  ...
!               life [;
!                   ...
!                   Give: switch (Shuffle(2, old_man_give)) {
!                       1: print_ret (The) self, " doesn't want ", (the) noun, ".";
!                       2: print_ret (The) self, " ignores your offering.";
!                       }
!                   Show: switch (Shuffle(3, old_man_show)) {
!                       1: print_ret (The) self, " isn't interested in ", (the) noun, ".";
!                       2: print_ret (The) self, " peers back distainfully.";
!                       3: "~I've seen hundreds of them in my time.~";
!                       }
!                   Tell: switch (Shuffle(5, old_man_tell)) {
!                       1: print_ret "There's no sign that ", (the) self, " has heard you.";
!                       2: print_ret (The) self, " points to his broken hearing-aid.";
!                       3: "~YOU'LL HAVE TO SPEAK UP - I'M A BIT DEAF.~";
!                       4: "~I don't know any more than you do.~";
!                       5: "Fancy that, then.";
!                       }
!                   ];
!
! ---------------------------------------------------------------------------- !
#ifdef DEBUG; message "Compiling Shuffle.h"; #endif;

Array   flagBit     ->  $01 $02 $04 $08 $10 $20 $40 $80;
Array   flagDefault -> 8;           ! default of count byte + 56 flag bits

[ Shuffle range flags n x y z;
        if (range < 2) return 1;
        if (flags == 0) flags = flagDefault;
        while (true) {
            n = random(range);      ! possible return value
            x = (n-1)%8;            ! bit 0..7 within flag byte
            y = 1+(n-1)/8;          ! byte 1.. within flag array
            if ((flags->y & flagBit->x) == 0) break;
            }                       ! loop until unused flag found
        if (flags->0 == range-1)    ! last unused flag?
            for (z = 1+(range-1)/8 : z>=0 : z--)
                flags->z = 0;       ! clear count and all flags
        flags->0 = flags->0 + 1;    ! increment count, set flag
        flags->y = (flags->y | flagBit->x);
        return n;
        ];

! ---------------------------------------------------------------------------- !
