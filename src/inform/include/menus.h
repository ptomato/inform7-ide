! -------------------------------------------------------------------------
!   Menus.h           A library extension providing easier and better menus
!                                                      Graham Nelson 961113
!                                                       Kevin Bracey 011128
!
!   A menu is a tree of objects of class Option.  A Menu is an Option which
!   launches a fresh menu when chosen.  To choose option O, send the
!   message:
!
!       O.select();
!
!   So to start off a menu session, send this message to the top menu.
!
!   Here's a simple menu structure:
!
!       Menu "Instructions for playing Mordred";
!       Menu   -> "How to play adventure games";
!       Option -> -> "Looking around"
!                     with description "I am your eyes and ears ...";
!       Option -> -> "Taking and Dropping"
!                     with description "When you find items ...";
!       Option -> "About the author"
!                  with description "The author was born in ...";
!
!   Menus produced in this code are automatically divided into pages
!   so that they'll always fit on the screen, whatever size the screen is
!   and however many options there are.
!
!   Note that since objects can always be moved about in play, it's easy
!   to create new menu structures to fit the circumstances of the moment.
!   (For example, a hints menu which gives hints only on currently open
!   puzzles.)
!
!   You can instead write a routine to receive the "description" message.
!   Then the text printed when an option is chosen can also vary.
!
!   If you return 2 from such a routine, then the game does not prompt
!   the player to press a key before going back into the menu.
!   If you return 3, then the whole menu is closed up immediately.
!
!   Finally, you can always give your own "select" routine for an Option.
!   The rules for return values are the same; what's different is that
!   this way the screen will not be cleared and given a nice banner when
!   the option is chosen.  (Nothing visible will happen unless you do
!   it yourself.)  The SwitchOption class is an example of the kind of
!   gadget you might want this for:
!
!       Menu "Game settings";
!       SwitchOption -> FullRoomD   "full room descriptions" has on;
!       SwitchOption -> WordyP      "wordier prompts";
!       SwitchOption -> AllowSavedG "allow saved games" has on;
!
!   Choosing any of these switch-options flips them between on and off.
!   In your program, you can test
!
!       if (WordyP has on) ...
!
!   and so forth to check the current state.
! -------------------------------------------------------------------------

Ifndef PKEY__TX;

Constant LIB_PRE_63;

!   Then we are using library 6/1 or 6/2, which won't have defined these:

Constant NKEY__TX     = "  N = next option";
Constant PKEY__TX     = "P = previous";
Constant QKEY1__TX    = "  Q = resume game";
Constant QKEY2__TX    = "Q = previous menu";
Constant RKEY__TX     = "RETURN = select option";

Constant NKEY1__KY    = 'N';
Constant NKEY2__KY    = 'n';
Constant PKEY1__KY    = 'P';
Constant PKEY2__KY    = 'p';
Constant QKEY1__KY    = 'Q';
Constant QKEY2__KY    = 'q';

Endif;

Global screen_width;
Global screen_height;
Global char_width;
Global char_height;

Array ForUseByOptions -> 129;

#Iftrue #version_number==6;
[ Menus_Measure s;
    @output_stream 3 ForUseByOptions;
    print (string) s;
    @output_stream -3;
    return 0-->24;
];
#Endif;

Class Option
 with emblazon
      [ bar_height page pages temp;

          screen_width = 0->33;

          !   Clear screen:

          @erase_window -1;
          #Iftrue #version_number==6;
          @set_cursor -1;
          @mouse_window -1;
          temp = bar_height * char_height;
          @split_window temp;
          #Ifnot;
          @split_window bar_height;
          #Endif;

          !   Black out top line in reverse video:
          @set_window 1;
          @set_cursor 1 1;
          #Iftrue #version_number==6;
          @set_font 4 -> temp;
          #Endif;
          style reverse; spaces(screen_width);
          #Iftrue #version_number==6;
          @set_font 1 -> temp;
          #Endif;

          if (#version_number ~= 6 && standard_interpreter == 0)
              @set_cursor 1 1;
          else
          {   ForUseByOptions-->0 = 128;
              @output_stream 3 ForUseByOptions;
              print (name) self;
              if (pages ~= 1) print " [", page, "/", pages, "]";
              @output_stream -3;
              #Iftrue #version_number==6;
              temp = 1 + (screen_width*char_width - 0-->24)/2;
              #Ifnot;
              temp = (screen_width - ForUseByOptions-->0)/2;
              #Endif;
              @set_cursor 1 temp;
          }

          print (name) self;
          if (pages ~= 1) print " [", page, "/", pages, "]";

          return ForUseByOptions-->0;
      ],
      select
      [;  self.emblazon(1, 1, 1);

          style roman; @set_window 0; font on; new_line; new_line;

          if (self provides description)
              return self.description();

          "[No text written for this option.]^";
      ];    

Class Menu class Option
 with select
      [ count j obj pkey  line oldline top_line bottom_line
                            page pages options top_option y x;

          screen_width = 0->33;
          screen_height = 0->32;
          #Iftrue #version_number==6;
          @set_font 4 -> x;
          char_width = 0->39;
          char_height = 0->38;
          @set_font x -> x;
          #Ifnot;
          char_width = 0->38;
          char_height = 0->39;
          #Endif;
          if (screen_height == 0 or 255) screen_height = 18;
          screen_height = screen_height - 7;

          options = 0;
          objectloop (obj in self && obj ofclass Option) options++;
          if (options == 0) return 2;

          pages = options/screen_height;
          if (options%screen_height ~= 0) pages++;

          top_line = 6;

          page = 1;

          line = top_line;

          .ReDisplay;

          top_option = (page - 1) * screen_height;

          self.emblazon(5 + options, page, pages);
          
          #Iftrue #version_number==6;
          x = 1 + char_width; y = 1 + char_height; @set_cursor y 1;
          @set_font 4->j; spaces(screen_width); @set_font 1->j;
          @set_cursor y x; print (string) NKEY__TX;
          j = 1+screen_width*char_width - Menus_Measure(PKEY__TX) - char_width;
          @set_cursor y j; print (string) PKEY__TX;
          
          y = y + char_height; @set_cursor y 1;
          @set_font 4->j; spaces(screen_width); @set_font 1->j;
          @set_cursor y x; print (string) RKEY__TX;
          if (sender ofclass Option) j = QKEY2__TX; else j = QKEY1__TX;
          j = 1+screen_width*char_width - Menus_Measure(j) - char_width;
          @set_cursor y j;
          #Ifnot;
          @set_cursor 2 1; spaces(screen_width);
          @set_cursor 2 2; print (string) NKEY__TX;
          j = screen_width-12; @set_cursor 2 j; print (string) PKEY__TX;

          @set_cursor 3 1; spaces(screen_width);
          @set_cursor 3 2; print (string) RKEY__TX;
          j = screen_width-17; @set_cursor 3 j;
          #Endif;

          if (sender ofclass Option)
              print (string) QKEY2__TX;
          else
              print (string) QKEY1__TX;
          style roman;

          count = top_line; j = 0;
          objectloop (obj in self && obj ofclass Option)
          {   if (j >= top_option && j < (top_option + screen_height))
              {   
                  #Iftrue #version_number==6;
                  y = 1 + (count-1)*char_height;
                  x = 1 + 4*char_width;
                  @set_cursor y x;
                  print (char) ' ';
                  #Ifnot;
                  @set_cursor count 6;
                  #Endif;
                  print (name) obj;
                  count++;
              }
              j++;
          }
          bottom_line = count - 1;
          oldline = 0;

          for(::)
          {   !   Move or create the > cursor:

              if (line~=oldline)
              {   #Iftrue #version_number == 6;
                  x = 1 + 4*char_width;
                  count = top_line; j = 0;
                  objectloop (obj in self && obj ofclass Option)
                  {   if (j >= top_option && j < (top_option + screen_height))
                      {   if (j - top_option + top_line == oldline or line)
                          {   if (j - top_option + top_line == line)
                                  style reverse;
                              y = 1 + (count-1)*char_height;
                              @set_cursor y x;
                              print (char) ' ', (name) obj, (char) ' ';
                              style roman;
                          }
                          count++;
                      }
                      j++;
                  }
                  #Ifnot;
                  if (oldline~=0) { @set_cursor oldline 4; print " "; }
                  @set_cursor line 4; print ">";
                  #Endif;
              }
              oldline = line;

              @read_char 1 -> pkey;
              
              if (pkey == 253 or 254)
              {   !   Mouse click:
                  x = (0-->27-->1 - 1) / char_width + 1;
                  y = (0-->27-->2 - 1) / char_height + 1;
                  if (y >= top_line && y <= bottom_line)
                  {   line = y;
                      if (pkey == 253)
                         pkey = 13;
                      
                  }
                  else if (y == 2)
                  {   if (x <= screen_width / 2)
                          pkey = 130;
                      else
                          pkey = 129;
                  }
                  else if (y == 3)
                  {   if (x <= screen_width / 2)
                          pkey = 13;
                      else
                          pkey = 27;
                  }
                  else
                      @sound_effect 1;
              }

              if (pkey == NKEY1__KY or NKEY2__KY or 130)
              {   !   Cursor down:
                  line++;
                  if (line > bottom_line)
                  {   line = top_line;
                      if (pages > 1)
                      {   if (page == pages) page = 1; else page++;
                          jump ReDisplay;
                      }
                  }    
                  continue;
              }

              if (pkey == PKEY1__KY or PKEY2__KY or 129)
              {   !   Cursor up:
                  line--;
                  if (line < top_line)
                  {   line = bottom_line;
                      if (pages > 1)
                      {   if (page == 1)
                          {   page = pages;
                              line = top_line
                                     + (options % screen_height) - 1;
                          }
                          else
                          {   page--; line = top_line + screen_height - 1;
                          }
                          jump ReDisplay;
                      }
                  }    
                  continue;
              }

              if (pkey==QKEY1__KY or QKEY2__KY or 27 or 131) break;

              if (pkey==10 or 13 or 132)
              {   count = 0;
                  objectloop (obj in self && obj ofclass Option)
                  {   if (count == top_option + line - top_line) break;
                      count++;
                  }

                  switch(obj.select())
                  {   2: jump ReDisplay;
                      3: jump ExitMenu;
                  }

                  #ifdef LIB_PRE_63;
                  print "[Please press SPACE to continue.]^";
                  #ifnot;
                  L__M(##Miscellany, 53);
                  #endif;
                  @read_char 1 -> pkey;
                  jump ReDisplay;
              }
          }

          .ExitMenu;

          if (sender ofclass Option) return 2;

          #Iftrue #version_number==6;
          @set_font 1 -> x;
          #Ifnot;
          font on;
          #Endif;
          @set_cursor 1 1;
          @erase_window -1; @set_window 0;
          #Iftrue #version_number==6;
          @set_cursor -2;
          #Endif;
          new_line; new_line; new_line;
          if (deadflag==0) <<Look>>;
          return 2;
      ];

Class SwitchOption class Option
  with short_name
       [;  print (object) self, " ";
           if (self has on) print "(on)"; else print "(off)";
           rtrue;
       ],
       select
       [;  if (self has on) give self ~on; else give self on;
           return 2;
       ];

! ------------------------------------------------------------------------
