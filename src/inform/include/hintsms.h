! --------------------------------------------------------------------
!
! RUNTIME-HINTS-SYSTEM
! for the INFORM 6.x IF-compiler, (c) 1999 Graham Nelson
!
! Provides a Magnetic Scrolls(R)-like hints-system that decodes code-
! sequences into plain text.
!
! Written & copyright (c) 2001 by Peer Schaefer.
! Unaltered distribution of this sourcecode without profit allowed.
! Use it freely within your own games, commercial or otherwise.
! It would be nice if you give me some credit, but it's not required.
!
! NO WARRANTY, NO LIABILITY. PROVIDED "AS IS". USE ON YOUR OWN RISK!
!
! Bug reports, comments and suggestions to: peerschaefer@gmx.net
!
! --------------------------------------------------------------------



! DESCRIPTION
! ===========
! Anybody out there remembering the good old hints-system in the
! text-adventures of "Magnetic Scrolls"(R), like The Pawn and The Guild of
! Thieves? You had to type in silly things like <5x rf mz g0 5b rx oo po>
! (that are provided in the manual) on the command-prompt, and the program
! decoded that into a short hint to get you unstuck.
!
! This module provides a very similar hints-system. To encode a hint into
! a code-sequence you can use the new verb "encode", that is only available
! if you compile the game with debug-mode on. In the manual/documentation of
! your game you can provide several code-sequences that you have created in
! this way.
!
! EXAMPLE:
! You can include the following text in the manual of your game:
!    "How can I open the small chest in the throne room?
!    42 17 60 49 26 77 6c e1 e2 f7 88 55 24"
!
! To decode such code-sequence your players can use the new verb "hint".
! Typing in
! >HINT 4217604926776ce1e2f7885524
! on the prompt will result in "use a hammer".
!
! The great advantage of such hints-system is that the hints are not
! included in the game itself; the game only contains a decoder for the
! hint-code-sequences. That allows you to publish new hints for your
! games without changing the game itself.



! INSTRUCTIONS
! ============
! Include this file after parser.h
!
! This file creates two new verbs: ENCODE and HINT. The ENCODE verb is
! only available if you compile your game in debug-mode. To encode a hint
! just type ENCODE followed by your hint.
!
! EXAMPLE:
!    If you type
!    >ENCODE Hello_world!
!    on the prompt (please use the underscore _ instead of a space to
!    avoid confusing the parser), the computer will output this code-
!    sequence:
!    fa 7f f0 85 56 77 94 91 1a 77 08 9f 58
!
! To decode such code-sequence use the HINT verb.
!
! EXAMPLE:
!    If you type
!    >HINT fa7ff085567794911a77089f58
!    the result will be "hello world!". Please don't use any spaces
!    within the code-sequence to avoid confusing the parser.
!
! That's all.
!
! Bug reports, comments and suggestions to: peerschaefer@gmx.net



! HOW IT WORKS
! ============
! The encoding-engine is very primitive, but hey, we're not encoding state
! secrets here - only hints. In a first step, every character is converted
! into a numerical value. This value is scrambled by calling the ENCODE
! routine. The result is converted into a two-digit-hex-number and printed
! out. When all characters are converted, a checksum is calculated, encoded
! and printed out.
!
! EXAMPLE:
! >ENCODE test
! results in c2 7f 08 9d 70, which means:
! c2 -> a scrambled 't'
! 7f -> a scrambled 'e'
! 08 -> a scrambled 's'
! 9d -> a scrambled 't'
! 70 -> the scrambled checksum
!
! The decoding-engine reverts this process. Each hex-number is converted
! into a numerical value and this value is un-scrambled. A new checksum is
! calculated and compared with the checksum at the end of the code-sequence.
! That's it.



! And here's the code. I apologize for the mess.
! I am neither a good programmer, nor am I very familiar with Inform (nor
! is my english good). If there is a real programmer out there who can
! tidie up a bit here, I would welcome that.
! Please eMail to: peerschaefer@gmx.net




Constant codemask_increment = 19;      ! This constant is used during the
                                       ! encoding and decoding process as
                                       ! a "key". You can change it to
                                       ! create your very own hint-codes.
                                       ! It must be a constant from 1 to
                                       ! 254. I recommend primes.
Global   codemask           = codemask_increment;
                                       ! This is a global counter, used by
                                       ! the encoding and decoding routines.


! The following routine converts a two-character-hex-number into a
! numerical value. Syntax is readhex(d16,d1). d16 is the first character
! of the hex-number, d1 is the second character.
[ readhex d16 d1 hi lo;
	if ((d16>='a')&&(d16<='f')) hi=d16-'a'+10; else hi=d16-'0';
	if ((d1 >='a')&&(d1 <='f')) lo=d1 -'a'+10; else lo=d1 -'0';
	return (hi*16)+lo;
];

! Inform provides no xor-operator. I hacked this out, but there is surely
! a more elegant way. Any suggestions?
[ xor n mask a y i;
	a = 1;
	y = 0;
	for (i=1:i<=8:i++)
	{
		if ((mask&a)==0)
		{
			if ((n&a)==a) y=y|a;
		}
		else
		{
			if ((n&a)==0) y=y|a;
		};
		a=a*2;
	};
	return y;
];


! This is the decode-routine. The routine receives one numerical parameter
! and returns a numerical value. Both are in range of 1...255.
[ Decode x y a b i;
        x = xor (x, codemask);          ! First xor with codemask
        a = 1;                          ! Start with bit no. 1
        b = 128;                        ! Bit no. 8
	y = 0;

        for (i=1:i<=8:i++)              ! This loop mirrors the whole byte,
        {                               ! making bit 1 to 8, bit 2 to bit 7,
                if ((x&b)==b) y=y|a;    ! bit 3 to bit 6 etc.
                a=a*2;                  ! Next bit
                b=b/2;                  ! Previous bit
	};

        codemask = (codemask+codemask_increment)%$ff;   ! Increments codemask
        return ($ff-y);                 ! Negate and return
];


[ HintSub hintcode i checksum1 checksum2 c;
        if (WordLength(2)<4)
                "Too short hint-code.";         ! At least four characters

        hintcode = WordAddress(2);              ! Here it starts

        for (i=1 : i<=WordLength(2) : i++)      ! Only hex-digits allowed
	{
		c = hintcode->(i-1);
		if (~~( (c>='0') && (c<='9') ||
                        (c>='a') && (c<='f') ) )
			"Illegal character in hint-code (maybe a typo?)";
	};

        checksum1 = 73;                         ! Base-value of the checksum
                                                ! (This is a arbitrary value.
                                                ! You can change it to any
                                                ! value from 0 to 255.)
        codemask  = codemask_increment;         ! Reset codemask

        for (i=1:i<=(WordLength(2)-2):i++)      ! Calculate checksum
		if ((i%2)==0)
                        checksum1 = checksum1
                                    + Decode (readhex ( hintcode->(i-2),
                                                        hintcode->(i-1) ) );

        checksum2 = Decode ( readhex (                  ! Original checksum
                        hintcode->(WordLength(2)-2),    
                        hintcode->(WordLength(2)-1)
                    ) );

        if ((checksum1%$ff)~=(checksum2%$ff))   ! Checksum OK?
                "Sorry, invalid hint-code (maybe a typo?)";

        codemask = codemask_increment;          ! Reset codemask

        for (i=1 : i<=(WordLength(2)-2) : i++)  ! Main-decoding-loop
		if ((i%2)==0)
		{
                        c = Decode ( readhex (hintcode->(i-2),
                                              hintcode->(i-1) ) );
                        if (c=='_') c=' ';      ! Convert _ to space
                        print (char) c;         ! That's it!
		};
        print "^";                              ! CR
	rtrue;
];

! The following parse-routine grabs at much text as possible.
[ ParseAll;
	if (NextWordStopped()==-1)		! No text at all.
	{
#ifdef DEBUG;
                if (action_to_be==##Encode)
                        print "(Missing text for encoding.)^";
                else    print "(Missing hint-code.)^";
#endif;
#ifndef DEBUG;
		print "(Missing hint-code.)^";
#endif;
		return GPR_FAIL;
	};
	if (NextWordStopped()~=-1)		! More than one word
	{
#ifdef DEBUG;
                if (action_to_be==##Encode)
                        print "(Please use no spaces. Use underscores _ instead.)^";
                else    print "(Please use no spaces. If the code-sequence contains any spaces, just omit them!)^";
#endif;
#ifndef DEBUG;
                print "(Please use no spaces. If the code-sequence contains any spaces, just omit them!)^";
#endif;
		return GPR_FAIL;
	};
	return GPR_PREPOSITION;			! O.k.!
];

Verb meta 'hint'	* ParseAll	-> Hint;



#ifdef DEBUG;

! The following routine prints out a given 1-byte-value as a 2-digit-hex-
! number. Example: "print (hex) 255;" results in "ff".
[ hex n;
        if ((n/$10)<10) print (n/$10);
                else print (char) 'a'+(n/$10)-10;
        if ((n%$10)<10) print (n%$10);
                else print (char) 'a'+(n%$10)-10;
];

! This is the encoding-routine. The routine receives one numerical parameter
! and returns a numerical value. Both are in range of 1...255.
[ Encode x y a b i;
        a = 1;                          ! Start with bit no. 1
        b = 128;                        ! Bit no. 8
	y = 0;
        for (i=1 : i<=8 : i++)          ! This loop mirrors the whole byte,
        {                               ! making bit 1 to 8, bit 2 to bit 7,
                if ((x&b)==b) y=y|a;    ! bit 3 to bit 6 etc.
                a=a*2;                  ! Next bit
                b=b/2;                  ! Previous bit
	};
        y = xor($ff-y, codemask);       ! xor with codemask
        codemask = (codemask+codemask_increment)%$ff;   ! Increment codemask
        return y;                       ! That's it!
];

[ EncodeSub hintcode i checksum res;
	hintcode = WordAddress(2);
        checksum = 73;                          ! Base-value of the checksum.
                                                ! (This is a arbitrary value.
                                                ! You can change it to any
                                                ! value from 0 to 255.)
        codemask = codemask_increment;          ! Reset codemask
        for (i=1 : i<=WordLength(2) : i++)
	{
                res = (hintcode->(i-1));        ! Get character
                checksum = checksum + res;      ! Create checksum
                print (hex) Encode(res), " ";   ! Encode!
	};
        print_ret (hex) Encode((checksum%$ff)); ! Encode the checksum
];

Verb meta 'encode'	* ParseAll	-> Encode;

#endif;


! End.
