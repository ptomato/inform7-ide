!  Center       A handy utility for centering printed text in the lower 
!               or upper window for Inform 6 by L. Ross Raszewski
!               (rraszews@skipjack.bleucrab.org)
! Ever wanted something like a status line centered but not known how to do 
! it without storing the width of the text somewhere?  I know I have.
! Yes, I'm sure everyone knows how to _now_, but do you have a library to do 
! it for you?  Are you a casual programmer who doesn't want to play with 
! @output_stream?  
! Center(x);
! x is either a string, or a routine to print one.  The result is centered 
! on the current line.  
! More useful is:
! CenterU(x,i)
! x is as above, i is the number of the line IN THE UPPER WINDOW you want to 
! print to.
! The catch?  It will only center a number of characters less than your 
! screen width.  Moreover, if your screen width is greater than 128, you can
! only center 128 characters.  And I think new lines will make it blow up.
!
! Still, it's better than the alternative, innit?
!

Array CenterText string 128;

[ Center instring i;
  CenterText-->0=128;
  @output_stream 3 CenterText;
  if (instring ofclass string)
  print (string) instring;
  if (instring ofclass Routine)
  indirect(instring);
  @output_stream -3;
  i=0->33;
  i=i-CenterText-->0;
  i=i/2;
  spaces(i);
  if (instring ofclass string)
  print (string) instring;
  if (instring ofclass Routine)
  indirect(instring);
];
[ CenterU instring j i;
  CenterText-->0=128;
  @output_stream 3 CenterText;
  if (instring ofclass string)
  print (string) instring;
  if (instring ofclass Routine)
  indirect(instring);
  @output_stream -3;
  i=0->33;
  i=i-CenterText-->0;
  i=i/2;
  @set_cursor j i;
  if (instring ofclass string)
  print (string) instring;
  if (instring ofclass Routine)
  indirect(instring);
];
