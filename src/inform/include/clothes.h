! Clothing.h
! Written 1998 by Denis Moskowitz (dmm@cs.hmc.edu)
!	- All Rites Reversed (k) Reuse what you like
! Defines Clothes class for easy clothing handling.
! Set area, level, and covering for pieces of clothing.
! Slightly mogrified from the original by Timothy Groves 
! (groves_ca@yahoo.co.uk)

Class Clothes
  has clothing
  with area 0,  ! area of body covered
  level 0,	! distance of clothing from body: e.g. underwear < shirt < coat
  covering true,	! whether this piece of clothing hides other pieces
  conceal [i c l;
      ! Hide clothing that is entirely covered by other clothing
      ! This is called when clothing is put on or taken off
    c = 0; l = -1;
    ! take a look at all worn clothing
    objectloop (i in parent(self) && i has worn && i ofclass Clothes) {
      if (i.area == self.area && i.level < self.level) {
        if (self.covering && self has worn) { ! hide covered clothing
          give i concealed; 
        }
        else if (i.level > l) {        ! find next visible clothing
          c = i;
          l = c.level;
        }
      }
    }
    if (c) {        !show clothing under this piece and recurse inward
      give c ~concealed;
      c.conceal();
    }
  ],
  before [c j i l;
    Wear, Disrobe:
        ! Don't let player take off clothing if it's under something
	  ! (WEAR SHIRT. WEAR JACKET. TAKE OFF SHIRT.)
        ! Don't let player put on clothing in wrong order
	  ! (WEAR JACKET. WEAR SHIRT.)
	! Don't let player wear two of the same thing at once
	  ! (WEAR BLUE SHIRT.  WEAR WHITE SHIRT.)
      c = 0;        ! Clothing that is at the same level
      j = 0;        ! Clothing that is at a higher level
      l = self.level;        ! Level of j
      objectloop(i in parent(self) && i has worn && i ofclass Clothes) {
        if (i.area == self.area) {
          if (i.level == self.level && i ~= self) c = i;
          else if (i.level > l) { j = i; l = j.level; }
        }
      }
      if (j) {        ! Clothing over self
        "You'll have to take off ", (the) j, " first.";
      }
      if (c && action == ##Wear) {        ! Clothing at the same level as self
        "You can't wear ", (the) self, " and ", (the) c, " at the same time.";
      }
  ],
  after [i count;
    Wear, Disrobe: self.conceal();
      print "You are now wearing ";
      count = 0;
      objectloop(i in parent(self))
      {
        if (i hasnt worn) give i ~workflag; else {give i workflag; count++;}
      }
      if (count == 0) print "nothing";
      else WriteListFrom(child(parent(self)), 
        ENGLISH_BIT + WORKFLAG_BIT + CONCEAL_BIT);
      ".";
  ];

!
! ListClothing(subject) - lists everything worn by the subject.  Concealed
! items are not revealed using this function.
!

[ ListClothing subject i count;
  print (the) subject, " is wearing ";
      count = 0;
      objectloop(i in subject)
      {
        if (i hasnt worn) give i ~workflag; else {give i workflag; count++;}
      }
      if (count == 0) print "nothing";
      else WriteListFrom(child(subject), 
        ENGLISH_BIT + WORKFLAG_BIT + CONCEAL_BIT);
      ".";  
];

!
! CheckArea(subject, test_area) - return the outermost clothing item worn in
! the selected area on the subject.  Great for testing for generic items of
! clothing at certain levels.
!

[ CheckArea subject test_area flag i;
	flag = nothing;
	objectloop (i in subject)
		{
		if (i ofclass Clothes)
			if (i has worn)
				if (i.area==test_area)
					if (flag==nothing)
					  flag = i;
					else
					  if (flag.level < i.level)
					    flag = i;
		} 
	return flag;
];

!
! CheckLevel(subject,test_level) - Return the first article of clothing worn
! at the selected level in the subject's inventory.
!

[ CheckLevel subject test_level flag i;
	flag = nothing;
	objectloop (i in subject)
		{
		if (i ofclass Clothes)
			if (i has worn)
				if (i.level==test_level)
					flag = i;
		}
	return flag;
];

