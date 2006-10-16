!---------------------------------------------------------------------------
!     YesNo                     An Inform 6 library to ask a semi-rhetorical
!                               Yes or no question, by L. Ross Raszewski
!                               (rraszews@skipjack.bluecrab.org)
!
! Several infocom games have asked the player yes or no questions which are 
! partially rhetorical (Take Hitchhiker's for example)  While Inform's library
! provides for askinga question and demanding an answer, it makes no such 
! allowance for non-manditory yes or no questions.  This does exactly that.
! send the message YesNo.Ask(ifyes,ifno,ifneither);, where IfYes and IfNo
! are routines to run or messages to print if the player answers yes or no.
! Ifneither is the string or routine that is printed if the player says 
! neither yes nor no in the next move.
! 
! This is my first fully object-oriented library extension, but it seems to 
! work anyway.
!
! You can omit any of the three arguments (actually, you have to put a zero in 
! to hold the place if you want to assign an action to ifno, but not ifyes.
!
! The answer is only accepted druing the next turn after the question is asked,
! and uses a timer, so if you're pushing the limit, sorry.
!
! If you like it, or if you don't, e-mail me and say so!

Object YesNo
        with react_before [;  Yes: if (self has on) return self.ifAff();
                              No: if (self has on) return self.ifNeg();
                              default: self.ifNeither();
                          ],
             ifAff 0,
             ifNeg 0,
             ifNeither 0,
             Ask [ ifyes ifno ifnone; 
                        self.ifAff=ifyes;
                        self.ifNeg=ifno;
                        self.ifNeither=ifnone;
                        StartTimer(self,1);
                        give self on;
                 ],
             time_left 1,
             time_out [; self.ifAff=0; self.ifNeg=0;
                         self.ifNeither=0;
                        give self ~on;],
             found_in [; rtrue;],
         has concealed;
