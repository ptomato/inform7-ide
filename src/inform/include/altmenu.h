!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! -*- Inform -*- !!!!
!       AltMenu                 An Inform 6 Library extension to create 
!                               object-oriented menus.  By L. Ross Raszewski
!                               (rraszews@skipjack.bluecrab.org)
!  Requires DoMenu.h and Center.h
! First of all, let me say that this is inspired heavily by Graham Nelson's
! own alternative menu system.  While the code is all my own, I'm not the 
! sort of person who would shamelessly copy someone else's idea without at 
! least mentioning them.
! Graham's implemenatation is really a lot more impressive code-wise.  Mine 
! is really just a shell for DoMenu.  However, my library uses the extended
! functions of the DoMenu Library I've written.
! Usage:
! To activate a menu, send the select(); message to the appropriate 
! Menu-Class object.  
! Menu objectname "Title of the Menu"
!     with description "If given, this text will be displayed above the menu
!                       choices",
!          number (number of lines in the description),
!          title_bar "If given, this will be displayed on the top line and the
!                       object name will appear on the second line.";
!  The children of a menu object are the options on the menu.  All children of 
! a menu MUST be of class Option, or of an option-ovide a select and doname property, but that's all Class
! Option does)
! Option -> "Title of the option"
!    with description [; string or rouine to be printed when Option is selected
!                        returns 1 to prompt a waitforkey, 2 to return instanly
!                        to the menu, or 3 to return and quit the menu];
! option.select(); is called when an option is selected, so you could change 
! that if you wanted.
!
!
!  This is part of a series actually, these are inter-dependant libraries.
!  For this to work, you have to include:
!  center.h   -> Centers a line of text (or a routine to print one) in 
!  the upper or lower window.
!  DoMenu       -> A modified DoMenu routine to allow long headers and 2-line
!                       title bars.
! For maximum enjoyment, add on (not required)
!  Hints.H (UPDATED)  -> A system for making object-oriented hints which appear
!                      formatted in the same way as Infocom's InvisiClues
!


Class Option
        with select [; return self.description();],
              doname [; print (name) self;];


Class Menu
        class Option,
        with select [; DoMenu(self.emblazon,self.titles,self.execute); 
                        return 2;],
             emblazon [ o; if (self provides description) self.description();
                                else new_line;
                         objectloop (o in self) {
                                spaces(5);
                                print (name) o;
                                new_line;
                                }
                         ],
             titles [ i o; if (menu_item==-1) {
                                if (self provides title_bar) 
                                        item_name=self.doname;
                                if (self provides number)
                                        return self.number();
                                else return 1;
                                }
                        if (menu_item==0) {
                                if (self provides title_bar)
                                        item_name=self.title_bar;
                                else item_name=self.doname;
                                return children(self);
                                }
                        o=child(self);
                        for (i=1:o~=nothing:i++)
                                {if (menu_item==i) item_name=o.doname;
                                 o=sibling(o);}
                        ],
             execute [ i o; 
                        o=child(self);
                        for (i=1:o~=nothing:i++)
                                {if (menu_item==i) return o.select();
                                 o=sibling(o);}
                        return 2;
                        ];
