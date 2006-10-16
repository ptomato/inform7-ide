!---------------------------------------------------------------------------------
!
! style.h
!
! a library extension for embedding styles in print statements
!
! by Chris Klimas
!
! you can use this extension for free, but give me credit
!
! designed for Inform 6.15
!
! an overview:
!
! print (b) "whatever"; prints whatever in bold
! print (i) "whatever"; prints whatever in underline/italic
! print (r) "whatever"; prints whatever in reverse
!
! variants:
!
! print (boldface) "whatever";
! print (underlined) "whatever";
! print (u) "whatever";
! print (italics) "whatever";
! print (inverse) "whatever";
!
! bold, underline, and reverse will not work
! they're reserved words
!
! note that this will restore roman style
! 
! include at will
!
!---------------------------------------------------------------------------------

[ b text;
  style bold;
  print (string) text;
  style roman;
];

[ boldface text;
  print (b) text;
];

[ i text;
  style underline;
  print (string) text;
  style roman;
];

[ u text;
  print (i) text;
];

[ italics text;
  print (i) text;
];

[ r text;
  style reverse;
  print (string) text;
  style roman;
];

[ inverse text;
  print (r) text;
];
