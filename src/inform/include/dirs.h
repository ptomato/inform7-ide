!----------------------------------------------------------------------------
! Dirs.h
!----------------------------------------------------------------------------
! Author:  Nicholas Daley <daleys@ihug.co.nz>
! Version: 2.1
! Purpose: Provides a 'dirs' meta-verb that tells you which directions you
!          can go in.
!
! It just checks if a value was given for the <dir>_to properties, so if one
! stored a string or a routine, then it will be included.  If cant_go moves
! the player, then this is _not_ included.
! Location objects can provide an allow_directions property which may tell
! dirs.h:a)(return false)Print if the location provides the corresponding
!                        *_to property
!        b)("    " true)The direction has already been printed
!        c)("    " 2)Ignore this direction - don't print it
!        d)("    " default)Definitely print this direction
!----------------------------------------------------------------------------
!Revision History:1.0=Original version (doesn't work with library 6/3 or
!                     later)
!                 2.0=Works with libraries before and after library 6/3
!                     Added the 'allow_directions' property
!                 2.1=can use verbs 'exits' or 'list exits' too.
!----------------------------------------------------------------------------
#ifndef DirectionName; #ifndef LanguageDirection;
  Message fatalerror "dirs.h requires DirectionName() or \
                      LanguageDirection() to be declared (this should be \
                      done by the library)";
#endif; #endif;

Property allow_directions;

[ DirsSub i flag flag2 j loc;
  print "You can go:^";
  if(location==thedark) loc=real_location;
  else loc=location;
  j=0;
  objectloop(i in Compass) {
   if(loc provides allow_directions)
    switch(loc.allow_directions(i))
     {
      false:flag=false;flag2=false;  !Print if the corresponding *_to property
                                     !is provided
      true:j++;flag2=true;           !The direction has already been printed.
      2:flag2=true;                  !Ignore this direction (do not print it)
      default:flag=true;             !Definitely print this direction
     };

   if(loc provides (i.door_dir) && metaclass(loc.(i.door_dir))~=nothing ||
      flag==true && flag2==false)
    {
      print " ";
#ifdef DirectionName;
      DirectionName(i.door_dir);
#ifnot;
 #ifdef LanguageDirection;     
      LanguageDirection(i.door_dir);
 #endif;
#endif;
     new_line;
     j++;
    }
  }
 if(j==0) " nowhere";
];

 Verb meta "dirs" "directions" * -> Dirs;
 Verb meta "list" * "exits" -> Dirs;
 Verb meta "exits" * -> Dirs;
