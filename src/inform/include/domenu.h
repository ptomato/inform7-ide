!    DOMENU             A replacement for Inform's standard DoMenu
!                       built for Inform 6 by L. Ross Raszewski
!                       (rraszews@skipjack.bluecrab.org)
!       REQUIRES CENTER.H
! Basically, this new menu does two very simple things.  First, unlike 
! with the traditional DoMenu, you can have any number of lines at the top 
! for a description or whatnot.  That is to say
! " Information is available on a variety of subjects^
!   You can access this information by selecting one of the lines below^
!   Press q to quit the menu^
!   ^     Prologue
!   ^     Beginning
!   ^     Middle
!   ^     End"
! will now work correctly as a menu.
! "How?" you ask.  Simple.  In your EntryR() routine (That's the first one you 
!  name after the string), you add a line
!       if (menu_item==-1) return x;
!  where x is the number of lines of description (4 in the above example)
!  The only change you'll have to make to existing menus if you don't want to 
!  change anthing is to remove 1 line at the top of the menu_choices.
!  (incidentally, if you put in a ",1" after the third argument, the title bar 
!  will say "Q=quit game".  Something for a game of mine I left in because I 
!  didn't want to take it out)
!
! Second, you can now have two lines of title at the top.  Depending on 
! your interpreter, (I believe Frotz and InfoTaskForce do this) the second 
! line will appear in reverse-bold
! For this trick, set item_name to the second line when menu_item=-1
!
! if (menu_item==-1) { item_name="Second Line"; return 4;} in the example above
!
! One more thing, you no longer have to fiddle with item_width.  DoMenu 
! doesn't need it anymore.  Merry Christmas.
!
! Replace DoMenu(); 
! Replace LowKey_Menu();
!
! Some time before including this.
!
! Comments?  e-mail me!
!
!  This is part of a series actually, these are inter-dependant libraries.
!  For this to work, you have to include:
!  center.h   -> Centers a line of text (or a routine to print one) in 
!  the upper or lower window.
!
! For maximum enjoyment, add on (not required)
!  AltMenu.H   -> An alternative to menus, object oriented menu system
!                       (inspired by Graham Nelson's attempt to do the same
!                        thing.  Mine makes use of the nifty abilities of this
!                        library)
!  Hints.H (UPDATED)  -> A system for making object-oriented hints which appear
!                      formatted in the same way as Infocom's InvisiClues
!

[ LowKey_Menu menu_choices EntryR ChoiceR lines main_title i j;
  menu_nesting++;
 .LKRD;
  menu_item=0;
  lines=indirect(EntryR);
  main_title=item_name;
  item_name=" ";
  menu_item=-1;
  indirect(EntryR);
  if (item_name ofclass string) print (string) item_name;
  else indirect(item_name);
  print "^--- "; 
  if (main_title ofclass string) print (string) main_title;
  else indirect(main_title);
  print " ---^^^";
  if (ZRegion(menu_choices)==3) print (string) menu_choices;
  else indirect(menu_choices);

 .LKML;
  print "^Type a number from 1 to ", lines,
        ", 0 to redisplay or press ENTER.^> ";

   #IFV3; read buffer parse; #ENDIF;
   temp_global=0;
   #IFV5; read buffer parse DrawStatusLine; #ENDIF;
   i=parse-->1;
   if (i=='quit' or #n$q || parse->1==0)
   {   menu_nesting--; if (menu_nesting>0) rfalse;
       if (deadflag==0) <<Look>>;
       rfalse;
   }
   i=TryNumber(1);
   if (i==0) jump LKRD;
   if (i<1 || i>lines) jump LKML;
   menu_item=i;
   j=indirect(ChoiceR);
   if (j==2) jump LKRD;
   if (j==3) rfalse;
   jump LKML;
];

#IFV3;
[ DoMenu menu_choices EntryR ChoiceR;
  LowKey_Menu(menu_choices,EntryR,ChoiceR);
];
#ENDIF;


#IFNDEF MenuFlag;
Global MenuFlag;
#ENDIF;

[ DoMenu menu_choices EntryR ChoiceR inflag 
         lines ban_lines main_title sub_title cl i j oldcl pkey;
        if (pretty_flag==0) 
         { LowKey_Menu(menu_choices,EntryR,ChoiceR);
           rfalse;}
         menu_nesting++;
         menu_item=0;
         lines=indirect(EntryR);
         main_title=item_name;
         item_name=" ";
         menu_item=-1;
         ban_lines=indirect(EntryR);
         ban_lines=ban_lines+5;
         sub_title=item_name;
         cl=ban_lines;
         .ReDisplay;
                oldcl=0;
                @erase_window -1;
                i=ban_lines+lines;
                @split_window i;
                i=0->33; if (i==0) i=80;
                @set_window 1;
                @set_cursor 1 1;
                style reverse;
                spaces(i);
                CenterU(main_title,1);
                @set_cursor 2 1; spaces (i);
                style bold;
                CenterU(sub_title,2);
                style roman; style reverse;
                @set_cursor 2 2; print "N = next subject";
                j=i-12; @set_cursor 2 j; print "P = previous";
                @set_cursor 3 1; spaces(i);
                @set_cursor 3 2; print "RETURN = read subject"; 
                j=i-17; @set_cursor 3 j;
                if (inflag==1) print "    Q = quit game";
                else
                if (inflag==2) print "    Q = quit menu";
                else
                if (menu_nesting==1)
                        print "  Q = resume game";
                else
                        print "Q = previous menu";
                style roman; font off;
                @set_cursor 5 2;
                if (menu_choices ofclass String) print (string) menu_choices;
                else indirect(menu_choices);

       .KeyLoop;
                if (cl~=oldcl)
                 {  if (oldcl>0) { @set_cursor oldcl 4; print " ";}
                    @set_cursor cl 4; print ">";
                 }
                oldcl=cl;
      @read_char 1 0 0 pkey;
      if (pkey=='N' or 'n' or 130)
          { cl++; if (cl>=ban_lines+lines) cl=ban_lines; jump KeyLoop; }
      if (pkey=='P' or 'p' or 129)
          { cl--; if (cl==ban_lines-1)  cl=ban_lines+lines-1; jump KeyLoop; }
      if ((pkey=='Q' or 'q' or 27)||(pkey==131)) { jump QuitHelp; }
      if (pkey==10 or 13 or 132)
      {   @set_window 0; font on;
          new_line; new_line; new_line;
          menu_item=cl-ban_lines+1;
          indirect(EntryR);

          @erase_window $ffff;
          @split_window 1;
          i = 0->33; if (i==0) { i=80; }
          @set_window 1; @set_cursor 1 1; style reverse; spaces(i);
          CenterU(item_name,1);
          style roman; @set_window 0; new_line;

          i = indirect(ChoiceR);
       !   if (i==2 && Menuflag~=10000) jump ReDisplay;
          if (i==2) jump ReDisplay;
          if (i==3) jump QuitHelp;

          print "^[Please press SPACE.]^";
          @read_char 1 0 0 pkey; jump ReDisplay;
      }
      jump KeyLoop;
      .QuitHelp;
      if (Menuflag==10000) Menuflag=0;
      menu_nesting--; if (menu_nesting>0) rfalse;
      font on; @set_cursor 1 1;
      @erase_window $ffff; @set_window 0;
      new_line; new_line; new_line;
      if (deadflag==0 && inflag==0) <<Look>>;
];  

