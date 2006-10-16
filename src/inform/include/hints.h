!       Hints           An -*- Inform -*- 6 library by L. Ross Raszewski
!                       To produce nicely formatted hints.
!                       Upgraded and made easier to use
!
! REQUIRES DoMenu.H, Center.H, and AltMenu.H
!
! Upon reflection, the older version of this library was clumsy and inflexible
! This system is well suited to adaptive hints and other wonderful things.
! Grammar -
! The verb "Hint" ("Help") calls up the hint menu, which should begin with an
! AltMenu Menu object called hint_menu.  (The first time, you are given the 
! option to disable the hints.)
! The children of hint_menu should be the top level hints or categories, and 
! so on etc until you reach the actual hints.  The hint categories are 
! objects of the class "HintTopic", whose short names are the names of the 
! hints.  The children of a HintTopic are of class Hint.  Their descriptions
! contain the actual hints, which are displayed in order.
! Example hierarchy (This is NOT code, it's more a sort of tree):
!       Menu hint_menu
!           Menu "Prologue"
!               HintTopic "What do I do here?"
!                       Hint with desc. "Look around."
!                       Hint w/d "Move around."
!           Menu "Part 1"
!               HintTopic "Who am I?"
!                       Hint w/d "You are Perry Simm"
!               HintTopic "What Do I do?"
!                       Hint w/d "Not a lot"
!                       Hint w/d "Nothing at all"
!                       Hint w/d "because this ain't a real game"
!
! The player sees the menu titled whatever you choose to call the hint_menu,
! on which an option is Prologue.  Under prologue is the option 
! "What do I do here?"  Selecting that will display "Look Around".  If the 
! player asks for another hint, he gets it.  if not, revealed hints are 
! re-reveled when he reenters the menu.  That is, if the player had read 
! the first two hints under "What do I do", then went back to the game, he 
! could at any time go back to the hint menu, navigate to "What do I do"
! and would automatically have the first two hints displayed.
! HINT_STYLE must be defined before inclusion (or else it defaults to 0) 
!  if it's 0, then the hint is displayed thus:
!       Not a lot (2 left)
!  if it's 1, you get:
!       Not a lot [1/4]
!  if it's 2, you get:
!     [3 hints left]-> Not a lot
!     [2 hints left]-> 
! (Note that when it's 2, you don't get the first hint when you first 
! enter a topic; you get [x hints left]-> )
!
! You can move the hintTopics about, or even do wretched things to 
! their children. (I wouldn't, but you can)  In an adaptive hint system, 
! you could nove the appropriate HintTopic (with its children already inside)
! into the hint_menu when you want the hint available.
!
! The title_bar property of a HintTopic specifies the top line of the hint 
! status line.  It defaults to "InvisiHints"  I've included WaitForKey();
! in case you don't have it already.
!
! Questions, comments, rude suggestions?  I'm at rraszews@skipjack.bluecrab.org
System_file;
#IFNDEF WaitForKey;
[ WaitForKey str i;
if (str==0) str="(Please Press Any Key)";
print (string) str;
@read_char 1 0 0 i;
];
#EndIf;


[ HelpSub;
        if (hint_menu hasnt general)
        {print "[Warning: It is recognized that the temptation for help \
        may at times be so exceedingly strong that you might fetch hints \
        prematurely. Therefore, you may at any time during the story type \
        HINTS OFF, and this will disallow the seeking out of help for the \
        present session of the story. If you still want a hint now, indicate \
        HINT.]"; give hint_menu general;
         rtrue;}
        if (hint_menu has locked)
                print_ret "Hints have been disabled.";
hint_menu.select();
];
[ HelpOffSub;
        print "HINTS disabled.";
        give hint_menu locked;
        rtrue;
        ];
#ifndef HINT_STYLE;
Constant HINT_STYLE 0;
#ENDIF;
Class HintTopic 
        class Option,
        with    printsn [; print (name) self;],
                number 0,
                title_bar "InvisiHints",
                select [ o i j; 
                         if (pretty_flag==0) { 
                                self.title_bar();
                                self.printsn();
                                print "^Press ENTER for another hint, Q to 
                                        return to the menu.";
                                jump disph;
                                }
                         @erase_window -1;
                         @split_window 3;
                         @set_window 1;
                         i=0->33; if (i==0) i=80;
                         @set_cursor 1 1;
                         style reverse;
                         spaces(i);
                         CenterU(self.title_bar,1);
                         @set_cursor 2 1; spaces (i);
                         style bold;
                         CenterU(self.printsn,2);
                style roman; style reverse;
                @set_cursor 3 1; spaces(i);
                @set_cursor 3 2; print "RETURN = read hint"; 
                j=i-17; @set_cursor 3 j;
                print "Q = previous menu";
                style roman; font off;
                @set_window 0;
                .disph;
                print "^^^^^";
                i=0;
                objectloop (o in self) {
                        if (o has general) { o.GiveHint(); new_line; i++;}
                        }
                i=children(self)-i;                
                if (i==0) { WaitForKey("^^^[That's All Folks!]^"); return 2;}
                for (o=child(self):o has general:o=sibling(o));
                do {
                if (HINT_STYLE==2) {
                        print "[",i, " hint";
                        if (i>1) print "s";
                        print " left]-> ";
                do { @read_char 1 0 0 j; } until (j == 'Q' or 'q' or 27 
                                                 or 10 or 13 or 132);
                if (j== 'Q' or 'q' or 27) return 2;
                if (j== 10 or 13 or 132) o.GiveHint();
                i--;
                jump loopover;
                }
                o.GiveHint();
                i--;
                if (i==0) jump loopover;
                if (HINT_STYLE==1) {
                        j=children(self)-i;
                        print " [",j,"/",j+i,"]";
                        };
                if (HINT_STYLE==0) {
                        print " (", i, " hint";
                        if (i>1) print "s";
                        print " left.)";
                        };
                do { @read_char 1 0 0 j; } until (j == 'Q' or 'q' or 27 
                                                 or 10 or 13 or 132);
                if (j== 'Q' or 'q' or 27) { give o ~general; return 2;}
                .loopover;
                o=sibling(o);
                new_line;
                } until (i==0);
                WaitForKey("^^^[That's All Folks!]^"); return 2;
                ];
Class Hint
        with GiveHint [; PrintOrRun(self,description,1);
                         give self general;];





Verb meta "Help" "Hint" "hints" "clues"  "clue"
                                * "off"                         ->HelpOff
                                *                               ->Help;   
