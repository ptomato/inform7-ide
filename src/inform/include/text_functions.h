!------------------------------------------------------------------------------!
!  text_functions.h  -  A collection of functions for dealing with text in     !
!                       Inform.                                                !
!  Release: 1.0                                                                !
!  Date: January 5th, 1999                                                     !
!  Author: Patrick Kellum <pkellum@pagans.org>                                 !
!------------------------------------------------------------------------------!
!  About:                                                                      !
!     These functions make changing text colours and styles much easer.  For   !
!     example, compare the following to pieces of example code:                !
!                                                                              !
!     Before:                                                                  !
!        print "This text is in ";                                             !
!        bold;                                                                 !
!        print "bold";                                                         !
!        roman;                                                                !
!        print " text.^";                                                      !
!                                                                              !
!     After:                                                                   !
!        print "This text is in ", (TextStyle) bold, "bold",                   !
!        (TextStyle) roman, " text.^";                                         !
!                                                                              !
!     Before:                                                                  !
!        print "This text is ";                                                !
!        @set_colour 3 0;                                                      !
!        print "red";                                                          !
!        @set_colour 1 0;                                                      !
!        print ".^";                                                           !
!                                                                              !
!     After:                                                                   !
!        print "This text is ", (FGColour) red, "red",                         !
!        (FGColour) default_colour, ".";                                       !
!                                                                              !
!     And not only do they make working with colours and styles easer but they !
!     also have the following additional features:                             !
!                                                                              !
!        o  Checks to make sure the interpreter supports the feature before    !
!           continuing.  For instance, if the interpreter doesn' support       !
!           colours then the function will return without trying to change the !
!           colour.                                                            !
!                                                                              !
!        o  Checks the z-code version before allowing the three higher colour  !
!           numbers to be used.                                                !
!                                                                              !
!        o  Written in pure z-machine assemble.  This will result in much more !
!           optimized code then Inform would write if it had to compile it     !
!           first. This can cut the resulting z-code for the function down to  !
!           nearly half and speed up compiliation time.                        !
!------------------------------------------------------------------------------!
!  Instructions:                                                               !
!     TextStyle(style)                                                         !
!        This is used to change the style of the text.  The style can be one   !
!        of the following values:                                              !
!           o  roman    -  resets the text style to the default                !
!                          non-fixed-width font.                               !
!           o  reverse  -  turns on reverse text.                              !
!           o  bold     -  turns on bold text.                                 !
!           o  italic   -  turns on italic text.                               !
!           o  fixed    -  turns on the fixed-width font.                      !
!        You can also use the style numbers or make your own names (for        !
!        example, you can use 'underline' insted of 'italics' to get the same  !
!        effect).  See the code below for examples of how to create your own   !
!        names.                                                                !
!                                                                              !
!     TextColour(foreground,background)                                        !
!        This will change the foreground and background of the text if allowed !
!        by the interpreter.  Note that this function can't be used inline a   !
!        print statement due to a limitation in Inform.  Use FGColour and      !
!        BGColour for that purpose.  The foreground and background can be the  !
!        following values:                                                     !
!           o  current_colour    -  don't change the colour.                   !
!           o  default_colour    -  change the colour to the default colour as !
!                                   specified by the interpreter.              !
!           o  black             -  black                                      !
!           o  red               -  red                                        !
!           o  green             -  green                                      !
!           o  yellow            -  yellow                                     !
!           o  blue              -  blue                                       !
!           o  magenta           -  magenta, or you could use purple.          !
!           o  cyan              -  cyan                                       !
!           o  white             -  white                                      !
!           o  light_grey        -  only in version 6 games and then only on   !
!                                   interpreters that claim to be running on   !
!                                   either a PC or an Amiga.                   !
!           o  medium_grey       -  only in version 6 games and then only on   !
!                                   interpreters that claim to be running on   !
!                                   an Amiga.                                  !
!           o  dark_grey         -  only in version 6 games and then only on   !
!                                   interpreters that claim to be running on   !
!                                   an Amiga.                                  !
!                                                                              !
!     FGColour(colour)                                                         !
!        Changes the colour of the text if allowed by the interpreter.  See    !
!        TextColour above for possable colour values.                          !
!                                                                              !
!     BGColour(colour)                                                         !
!        Changes the colour of the text backbround if allowed by the           !
!        interpreter.  See TextColour above for possable colour values.        !
!------------------------------------------------------------------------------!
!  History:                                                                    !
!     1.0:  First release.                                                     !
!------------------------------------------------------------------------------!
System_file;

Message "^Including the following library:";
Message "   text_functions.h  -  A collection of functions for dealing with text in Inform.";
Message "   Release: 1.0";
Message "   Date   : January 5th, 1999";
Message "   Author : Patrick Kellum <pkellum@pagans.org>^";

#IfV3;
   Message fatalerror "Sorry, this library only works on version 5 or greater games.";
#Endif;

Constant roman 0;
Constant clear 0;
Constant reverse 1;
Constant bold 2;
Constant italic 4;
Constant underline 4;
Constant fixed 8;

Constant current_colour 0;
Constant default_colour 1;
Constant black 2;
Constant red 3;
Constant green 4;
Constant yellow 5;
Constant blue 6;
Constant magenta 7;
Constant purple 7;
Constant cyan 8;
Constant white 9;
!  only version 6 games
Constant grey 10;          !  grey in pc interpreters, light grey in Amiga interpreters
Constant light_grey 10;    !  grey in pc interpreters, light grey in Amiga interpreters
Constant medium_grey 11;   !  Amiga interpreters only
Constant dark_grey 12;     !  Amiga interpreters only

[ TextStyle style;
   @je style 2 ?~_italic;
   @call_2s CheckTextFlag 4 ->sp;
   @jz sp ?rfalse;
   @set_text_style 2;
   @rfalse;
   ._italic;
   @je style 4 ?~_reverse;
   @call_2s CheckTextFlag 8 ->sp;
   @jz sp ?rfalse;
   @set_text_style 4;
   @rfalse;
   ._reverse;
   @je style 1 ?~_fixed;
   @set_text_style 1;
   @rfalse;
   ._fixed;
   @je style 8 ?~_roman;
   @call_2s CheckTextFlag 16 ->sp;
   @jz sp ?rfalse;
   @set_text_style 8;
   @rfalse;
   ._roman;
   @jz style ?~rfalse;
   @set_text_style 0;
   @rfalse;
];

[ FGColour colour;
   @call_2s CheckTextFlag 1 ->sp;
   @jz sp ?rfalse;
   @loadb $00 $00 ->sp;
   @je sp $06 ?skip;
   @jg colour 9 ?rfalse;
   .skip;
   @set_colour colour 0;
   @rfalse;
];

[ BGColour colour;
   @call_2s CheckTextFlag 1 ->sp;
   @jz sp ?rfalse;
   @loadb $00 $00 ->sp;
   @je sp $06 ?skip;
   @jg colour 9 ?rfalse;
   .skip;
   @set_colour 0 colour;
   @rfalse;
];

[ TextColour fg bg;
   @call_2s CheckTextFlag 1 ->sp;
   @jz sp ?rfalse;
   @loadb $00 $00 ->sp;
   @je sp $06 ?skip;
   @jg fg 9 ?rfalse;
   @jg bg 9 ?rfalse;
   .skip;
   @set_colour fg bg;
   @rfalse;
];

[ CheckTextFlag flag;
   @loadb $01 $00 ->sp;
   @and sp flag ->sp;
   @jz sp ?rfalse;
   @rtrue;
];

!------------------------------------------------------------------------------!
!  End Of File                                                                 !
!------------------------------------------------------------------------------!
