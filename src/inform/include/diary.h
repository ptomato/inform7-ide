!==============================================================================
! diary.h by Gunther Schmidl <gschmidl@gmx.at>                 Release 1.000226
!==============================================================================
!
! This class defines a diary item that can be filled in by the game author to
! provide background information or similar. It is derived from L. Ross
! Raszewski's manual.h, and requires his Utility.h and IString.h libraries.
!
!------------------------------------------------------------------------------
! CUSTOMIZATION
!------------------------------------------------------------------------------
!
! You can customize the strings this library will print by either overriding 
! the default values or setting the appropriate self.D_....__TX from within the 
! game. 
!
! D_AWAY__TX is used when the player types 'Q' to exit the diary
! D_UPDT__TX is printed when the diary is updated
! D_INFO__TX is printed when the diary is updated for the first time.
!
! You can use special formatting in D_UPDT__TX and D_INFO__TX: see the 
! FormatText routine for information. If you don't need this fancy stuff, 
! define NoFormat somewhere before inclusion of the library. 
!
! This library uses the colour patch by Adam Cadre that'll correctly restore
! colors upon UNDO and RESTORE; see http://www.adamcadre.ac/inform.html
! Basically, if you want to use colour, you'll have to define USES_COLOUR
! somewhere in the game and provide two Globals: 'fg' and 'bg'.
! If this isn't what you use, you'll have to change the 'Colour' routine
! below.
!
! The only things you'll have to change directly are the "Go to what page" box
! (since Inform can't handle 'box self.D_PAGE__TX') and the key handling, 
! should you change the default keys from P, N, Q and G.
!
!------------------------------------------------------------------------------
! HOW TO USE IT
!------------------------------------------------------------------------------
!
! Use "Diary objectname" to define the class. Whatever you give for a 
! description will be printed on the title page. The object's short name will
! be displayed on the first line of the screen. The game will automatically 
! wait for any key, then display the next page.
!
! Example:   
!
! Diary mydiary "Adventurer's Diary"
! with  
!  description
!  [;
!   @erase_window NULL;
!   box "The Adventurer's Diary"
!       "----------------------"
!       " THIS IS MINE SO KEEP "
!       "    YOUR HANDS OFF    ";
!  ];   
!
!------------------------------------------------------------------------------
!
! A diary page is a simple object. Its short name will be displayed on the 
! second line of the screen, and the description in the window itself. Be
! careful not to put too much text on one page.
! If you give the page the attribute 'on', it will be listed in the table of
! contents (see below)
!
! Example:
!
! Object p1 "24 Frobuary"
! with
!  description "^blah blah blah",
! has on;
!
!------------------------------------------------------------------------------
!
! If you want, you can use Class ContentsPage to provide a table of contents.
! The short name will be displayed on the second line of the screen, and you
! can provide the PageHeader property to display a short text before the 
! table begins. If you need several content pages, just define them; they will 
! be left empty if unneeded.
!
! Example:
!
! ContentsPage "Contents";
! 
!------------------------------------------------------------------------------
!
! DO NOT MOVE ANY PAGES INTO THE DIARY YOURSELF!    
!
! You must use diary.update(pagename, flag) for this reason, or the diary
! will not open on the correct page.
!  
! Use diary.update(pagename, 2) to insert content pages or if you want to 
! add a page without notifying the user.
!
! Use diary.update(pagename, 1) to insert the first of a number of pages (or
! one page only.) This will notify the player that the diary has been updated.
!
! Use diary.update(pagename) to insert the rest of the pages. The game will
! silently add them but not update the page counter, so the diary will open
! on the last page inserted with flag 1.       
!
!------------------------------------------------------------------------------
! WHAT ELSE YOU NEED
!------------------------------------------------------------------------------
!
! A verb 'diary' plus a subroutine 'DiarySub' (or whatever you want to call 
! them) that look similar to these:
!
! [ DiarySub;
!  mydiary.select(mydiary.last_page);
! ];
!
! Verb 'diary' * -> Diary;
!
!==============================================================================

Class Diary
 with    
  D_PKEY__TX "P = previous page",
  D_NKEY__TX "N = next page",
  D_QKEY__TX "Q = resume story",
  D_GKEY__TX "G = Go to page ",
  D_PAGE__TX "Page ",
  D_AWAY__TX "^^You close the diary and put it away.^",
  D_UPDT__TX "Your diary has been updated.",         
  D_INFO__TX "Type %BDIARY%P to read the latest entry.",
  update
  [ page flag;   
   Pmove(page, self);

   if (flag == 1)
   {
    self.last_page = children(self);
    print "^^^[";
    FormatText( self.D_UPDT__TX );
    if (self.message == 0)
    {
     self.message = 1;    
     print " ";
     FormatText( self.D_INFO__TX ); 
     print "]^";
    }
    else
     "]";
   } 

   if (flag == 2)
    self.last_page = children(self);
    
   rtrue;
  ],
  last_page 0,
  message 0,
  emblazon 
  [ i j;
   if (pretty_flag==0) 
    return self.Lowkey_emblazon();
   @erase_window -1;
   i = 0->33; if (i == 0) i = 80;
   style roman;
   @split_window 3; @set_window 1; style reverse;
   @set_cursor 1 1; spaces(i);
   @set_cursor 2 1; spaces(i);
   @set_cursor 3 1; spaces(i);
   @set_cursor 1 1; print (string) self.D_PAGE__TX, self.pagen;
   CenterU(self.doname,1);
   style bold; style reverse;
   CenterU(self.dopagename,2); style roman;
   style reverse;
   @set_cursor 2 2; print (string) self.D_PKEY__TX;
   j = i - 14 ;
   @set_cursor 2 j; print (string) self.D_NKEY__TX;
   @set_cursor 3 2; print (string) self.D_QKEY__TX;
   @set_cursor 3 j; print (string) self.D_GKEY__TX;
   @set_window 0; style roman; font on;
  ],
  Lowkey_emblazon 
  [;
   print "^^---"; self.doname(); print "---^";
   print (string) self.D_PAGE__TX, self.pagen, " ";
   self.dopagename(); print "^^";
   print (string) self.D_PKEY__TX;
   spaces(4);
   print (string) self.D_NKEY__TX;
   new_line;
   print (string) self.D_QKEY__TX;
   spaces(4);
   print (string) self.D_GKEY__TX;
   new_line; new_line;
  ],
  select 
  [ startpage;
   self.pagen=startpage;
   while (self.dopage() ~= 2);
   if (pretty_flag~=0) 
    @erase_window -1;
   print (string) self.D_AWAY__TX; rtrue;
  ],
  page 0, 
  pagen 0,
  doname 
  [; 
   print (name) self;
  ],
  dopagename 
  [; 
   print (name) self.page;
  ],
  dopage 
  [ j;
   self.page=Scion(self,self.pagen);
   self.emblazon();
   self.page.description();
   j=self.keyloop();
   switch (j)
   {
    0: self.pagen--;
    1: self.pagen++;
    2: return 2;
   }
   if (self.pagen<0) self.pagen=0;
   if (self.pagen>children(self))
    self.pagen=children(self);
   return 1;
  ],
  keyloop 
  [ keypress k;
   do { @read_char 1 0 0 keypress;}
    until ((keypress == 129 or 'P' or 'p' or 130 or 'N' or 'n' or 'q' or 'Q' or 
          27 or 'g' or 'G') || (keypress == 32 or 10 or 13 && self.pagen == 0));

   if (keypress==129 or 'P' or 'p') return 0;
   if (keypress==32 or 10 or 13 && self.pagen==0) return 1;
   if (keypress=='G' or 'g')
   {
    if (pretty_flag)
     box "Go to what page?";
    else print "Go to what page? >";
    read buffer parse;
    k=parse-->1;
    if (parse->1==0) return 4;
    k=TryNumber(1);
    self.pagen=k;
    return 3;
   }
   if (keypress==130 or 'N' or 'n') return 1;
   if (keypress==27 or 'Q' or 'q') return 2;
  ];

Class ContentsPage
 with 
  description 
  [ o i; new_line;
   self.PageHeader();
   objectloop(i in parent(self))
   {
    o++;
    if (i has on && o<ValueOrRun(self,ucutoff)
    && o>ValueOrRun(self,lcutoff))
    {
     self.listitem=i;
     self.ListEntry(o);
     new_line;
    }
   }
  ],
  ucutoff 
  [; 
   return (self.page) * (0->33 - 5);
  ],
  lcutoff
  [; 
   return (self.page - 1)*(0->33 - 5) + 1;
  ],
  page 1,
  PageHeader 
  [; 
   rtrue;
  ],
  ListItem 0,
  PrintItem 
  [;
   print (name) self.ListItem;
  ],
  ListEntry 
  [ num i;
   i=(0->33)-13;
   if (i<=0) i=(0->33)-3;
   spaces(5);
   LJustify(Self.PrintItem, i, LEFT, '.');
   print num;
  ];

!----------------------------------------------------------- format text nicely

#IFNDEF NoFormat;

 Array printed_text table 2000;
 
 [ Colour f b;
   @set_colour f b;
 ];             
 
 [ FormatText format j k;
  format.print_to_array(printed_text);
  j = printed_text-->0;
  for(k = 2 : k < j + 2 : k++)
  {
   if(printed_text->k == '%')
   {
    switch(printed_text->(++k))
    {
     '%': print "%";
     ' ': spaces 1;
     'B': style bold;
     'P': style roman;
     'R': style reverse;
     'U': style underline;
     'n': font on;
     'N': font off;
     '1': Colour(1, 0);
     '2': Colour(2, 0);
     '3': Colour(3, 0);
     '4': Colour(4, 0);
     '5': Colour(5, 0);
     '6': Colour(6, 0);
     '7': Colour(7, 0);
     '8': Colour(8, 0);
     '9': Colour(9, 0);
     'a': Colour(0, 1);
     'b': Colour(0, 2);
     'c': Colour(0, 3);
     'd': Colour(0, 4);
     'e': Colour(0, 5);
     'f': Colour(0, 6);
     'g': Colour(0, 7);
     'h': Colour(0, 8);
     'i': Colour(0, 9);
    }
     } else
       print (char) printed_text->k;
   }
 ];   

#ENDIF;
