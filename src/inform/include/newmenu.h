! NewMenu.h -- 960131   Mike Phillips (mike@lawlib.wm.edu)
! 
! for Inform 5.5, Library 5/12 (may work with earlier versions)
! 
! This implementation of LowKey_Menu fixes one bug (inability to 
! redisplay the menu) and adds a feature to the 'fancy' V5 menu:
! the DoM_cl global variable may be used as part of forcing a 
! redraw (a menu routine returning 2, for instance) to select what 
! the current 'line' of the pointer should be -- the first line is 
! 7, the second is 8, etc.  In addition, a re-draw of the menu will
! call the information again, so that the menu size can change and the 
! title can change (used by the author to allow multiple pages in 
! the AdHints.h library module without having multiple depths of menus).
!
! All rights released, use it how you will.  No warranty, of course.
!
! To make use of this, place the following lines:
! Replace LowKey_Menu;
! Replace DoMenu;
!
! before including Parser.h, and include NewMenu.h some time after that.
!


[ LowKey_Menu menu_choices EntryR ChoiceR lines main_title i j;
  menu_nesting++;
 .LKRD;
  menu_item=0;
  lines=indirect(EntryR);
  main_title=item_name;

  print "--- "; print_paddr main_title; print " ---^^";
  if (ZRegion(menu_choices)==3) print_paddr menu_choices;
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

#IFV5;
Global DoM_cl;
[ DoMenu menu_choices EntryR ChoiceR
         lines main_title main_wid cl i j oldcl pkey;
  if (pretty_flag==0)
  {   LowKey_Menu(menu_choices,EntryR,ChoiceR);
      rfalse;
  }
  menu_nesting++;
  DoM_cl=7;
  cl=DoM_cl;
  .ReDisplay;
      menu_item=0;
      lines=indirect(EntryR);
      main_title=item_name; main_wid=item_width;
      oldcl=0;
      @erase_window $ffff;
      i=lines+7;
      @split_window i;
      i = 0->33;
      if (i==0) i=80;
      @set_window 1;
      @set_cursor 1 1;
      style reverse;
      spaces(i); j=i/2-main_wid;
      @set_cursor 1 j;
      print_paddr main_title;
      @set_cursor 2 1; spaces(i);
      @set_cursor 2 2; print "N = next subject";
      j=i-12; @set_cursor 2 j; print "P = previous";
      @set_cursor 3 1; spaces(i);
      @set_cursor 3 2; print "RETURN = read subject";
      j=i-17; @set_cursor 3 j;
      if (menu_nesting==1)
          print "  Q = resume game";
      else
          print "Q = previous menu";
      style roman;
      @set_cursor 5 2; font off;

      if (ZRegion(menu_choices)==3) print_paddr menu_choices;
      else indirect(menu_choices);

      .KeyLoop;
      if (cl~=oldcl)
      {   if (oldcl>0) { @set_cursor oldcl 4; print " "; }
          @set_cursor cl 4; print ">";
      }
      oldcl=cl;
      @read_char 1 0 0 pkey;
      if (pkey=='N' or 'n' or 130)
          { cl++; if (cl==7+lines) cl=7; jump KeyLoop; }
      if (pkey=='P' or 'p' or 129)
          { cl--; if (cl==6)  cl=6+lines; jump KeyLoop; }
      if (pkey=='Q' or 'q' or 27) { jump QuitHelp; }
      if (pkey==10 or 13)
      {   @set_window 0; font on;
          new_line; new_line; new_line;

          menu_item=cl-6;
          indirect(EntryR);

          @erase_window $ffff;
          @split_window 1;
          i = 0->33; if (i==0) { i=80; }
          @set_window 1; @set_cursor 1 1; style reverse; spaces(i);
          j=i/2-item_width;
          @set_cursor 1 j;
          print_paddr item_name;
          style roman; @set_window 0; new_line;

          i = indirect(ChoiceR);
          if (i==2) { cl=DoM_cl; jump ReDisplay; }
          if (i==3) jump QuitHelp;

          print "^[Please press SPACE.]^";
          @read_char 1 0 0 pkey; jump ReDisplay;
      }
      jump KeyLoop;
      .QuitHelp;
      menu_nesting--; if (menu_nesting>0) rfalse;
      font on; @set_cursor 1 1;
      @erase_window $ffff; @set_window 0;
      new_line; new_line; new_line;
      if (deadflag==0) <<Look>>;
];  
#ENDIF;
