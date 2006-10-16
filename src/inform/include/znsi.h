!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!                                    A control-code add on system for 
!   ZNSI                             istring.h
! Version 1                          by L. Ross Raszewski
!
! A brief explanation:
!  I was coding up an array of stings to be printed by a function in a game I
!  was writng, when it occured to me that I wanted to used colored text, and, 
!  unfortunately, Inform requires the unsightly kludge of doing this:
!  print "The last word in this sentence should be in ";
!  @set_colour 3 0;
!  print "red";
!  @set_colour 1 0;
!  print "."
!  So, I decided to write a library that would let me abbreviate control codes
!  in the text of a string.  Hence ZNSI.
!  ZNSI is a set of control sequences which may be inserted into a string and
!  and run at print-time.  It uses the string-printing facilities of the
!  Istring library.
!  In order to print a sequence of control characters, use the function 
!  LPrintString (or Printstring, for string arrays). The following 
!  control sequences have been defined: (all sequences start with the
!  character '[' and are case sensitive)
!  Sequence:    Effect:                 Sequence:  Effect:
!  "[N"         default font and color  "[7"       Foreground: Magenta
!  "[B"         Bold font               "[8"       Foreground: Cyan
!  "[U"         Underline font          "[9"       Foreground: White
!  "[R"         Reverse font            "[d"       Background: Default
!  "[F"         Monospace font          "[r"       Background: red
!  "[f"         Proportional font       "[g"       Background: green
!  "[1"         Foreground: default     "[y"       Background: Yellow
!  "[2"         Foreground: black       "[l"       Background: Blue
!  "[3"         Foreground: Red         "[m"       Background: Magenta
!  "[4"         Foreground: Green       "[c"       Background: Cyan
!  "[5"         Foreground: Yellow      "[w"       Background: White
!  "[6"         Foreground: Blue        "[b"       Background: Black
!  "[["         Print the character '[' 
!
! So, the above example would become:
! LPrintString("The last word of this sentence should be in [3red[1.");
!
! Other examples:
! To print:
!   Some words are *bold*, while others are [bracketed].
! do this:
! LPrintString("Some words are [Bbold[N, while others are [[bracketed].");
!
! Any invalid control sequence will be printed without the '[' delimiter,
! so LPrintString("[A New Day]"); would print:
!   A New Day]
!
! StrLen and Lstrlen will return the number of non-control characters in
! a string.
!
! This file must be included BEFORE  Istring.h in order to work properly.
! If you are including utility.h, include it BEFORE this library,
! otherwise ZNSI will redefine the Emphasis function.
!
! Let me know what you think.  I'm also accepting suggestions for new
! control codes...
! rraszews@acm.org
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
system_file;
Default NORMAL 0;
Default BOLD   1;
Default UNDERLINE 2;
Default REVERSE 4;


Constant ZNSI_Symbols 22;
Array ZNSI --> 'N' Emphasis 0
               'B' Emphasis BOLD
               'U' Emphasis UNDERLINE
               'R' Emphasis REVERSE
               'F' SetFont  1
               'f' SetFont  0
               '1' SetCol   1
               '2' SetCol   2
               '3' SetCol   3
               '4' SetCol   4
               '5' SetCol   5
               '7' SetCol   7
               '8' SetCol   8
               '6' SetCol   6
               '9' SetCol   9
               'd' SetBCol  1
               'b' SetBCol  2
               'r' SetBCol  3
               'g' SetBCol  4
               'y' SetBCol  5
               'l' SetBCol  6
               'm' SetBCol  7
               'c' SetBCol  8
               'w' SetBCol  9;

[ ZNSI_Lookup char j;
  for(j=0:j<ZNSI_Symbols:j++)
   if (char==ZNSI-->(3*j))
     return (3*j)+1;
  rfalse;
];

Ifndef Emphasis;
[ Emphasis n;
switch(n){
0: style roman; 
   @set_colour 1 1;
1: style bold;
2: style underline;
4: style reverse;
}
];
Endif;


[ PrintString str i j;
   for(:i<(str-->0):i++)
   {
    if (str->(i+2)=='['){
     @sound_effect 3;
     i++;
     j=ZNSI_Lookup(str->(i+2));
     if (j>0) indirect(ZNSI-->j,ZNSI-->(j+1));
     else print (char) str->(i+2);
    } else
    print (char) str->(i+2);
   }
   return i-2;
];

[ StrLen Str i k;
   for(:i<(str-->0):i++)
   {
    if (str->(i+2)=='['){
     i++;
     if (ZNSI_Lookup(str->(i+2))<=0) k++;
    } else k++;
   }
  return k;
];
         
[ Justify str width align pad_chr i;
   EmptyString(StringBuffer2,width+Zchars(str),pad_chr);
   i=StrLen(str);
   if (align==LEFT or 0) i=0;
   else if (align==RIGHT) i=width-i;
   else if (align==CENTERED) i=(width-i)/2;
   StrCpy(StringBuffer2,str,i);
   StringBuffer2-->0=width+Zchars(str);
   PrintString(StringBuffer2);
];
[ ZChars Str i k;
   for(:i<(str-->0):i++)
    if (str->(i+2)=='['){
     k++; i++;
     if (ZNSI_Lookup(str->(i+2))>0) k++;
   }
  return k;
];
[ SetFont i;
 if (i) font off;
 else font on;
];

[ SetCol i;
  if ((0->1)&1)  @set_colour i 0;
];
[ SetBCol i;
  if ((0->1)&1)  @set_colour 0 i;
];
