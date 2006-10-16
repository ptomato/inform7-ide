!   FOOTNOTE.H          An Inform Library Extention for Inform 5.5 or greater
!                       To produce sequentially numbered footnotes.
!                       L. Ross Raszewski
!       VERSION 2.0
! New features: *Define Redisplay_Footnotes to 1 before inclusion to prevent
!               the player from seeing the footnote indicator more than once
!               *Define footnote1__tx to the text to be printed before the 
!               number and footnote2__tx to the text to be printed after it
!               in order to change the default [footnote #] message
!               (multi-lingual support)
!
!  
!
!
! This library, based on the excercise in the manual, allows you to integrate 
! footnotes into a game, which are numbered IN THE ORDER THEY APPEAR.  It has
! two advantages over the simple, obvious system (ie. the one used in 
! Hitchhiker's Guide):  First, footnotes which have not been mentioned cannot
! be accessed.  Second, the footnotes will always count upward in series, no 
! matter what order they're called in.  The example in the manual, I found, 
! sometimes numbers the same footnote twice.  I've avoided this, so that the 
! same footnote will have the same number, regardless of how it is called 
! (sort of like Achieved(#);)  
! Include AFTER Grammar.h
! define the constant MAX_FOOTNOTES to the number of footnotes in the game
! before inclusion.
! To print a footnote in running text, call Note() with the number of the 
! footnote (THe number is internal, and may not be the one that apears on the 
! screen.  Ex: If two footnotes have already been seen, then the statement:
!                  Print "All the world's a stage.", (note) 1;
! will print "All the world's a stage. [Footnote 3]", with the [Footnote 3]
! in underline print.  Note() prints the text contained in Footnote1__TX
! and Footnote2__TX, which defaults to having a leading space, but no trailing 
! one.
! The footnote is read by issuing the command FOOTNOTE 3 (or NOTE 3, or 
! READ FOOTNOTE 3).  FOOTNOTE alone will instruct the player on the use of 
! Footnotes.
!
! You must define the function Printnote(n); before inclusion.  This function 
! actually prints the footnote text after FootnoteSub has printed the words
! [Footnote #]^ and has converted the number the player typed into its 
! original number in the footnote list  A sample PrintNote sub might read:
!       [ PrintNote n;
!         switch(n){
!                 1: "William Shakespeare";
!                 2: "Sloathes have no taste";
!                 3: {Style bold; print "Sesame Street"; style roman; " was \
!                       brought to you by the letter ~K~ and the number 4.";};
!                 };
!              ];
!
!  You can have a footnote do anything you like, even call another footnote.
!  (I've tried it, it seems to work).
!  Word of warning: The footnotes_seen array is a bit array, so I think you 
!  can't have more than 256 footnotes.
!
!  
!

System_file;
Array footnotes_seen -> MAX_FOOTNOTES;
Global footnote_count;
#IFNDEF Redisplay_Footnotes;
Constant Redisplay_Footnotes 0;
#ENDIF;

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! Multi-lingual block.  Define the following 
! contants before inclusion to use alternate 
! Messages
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#IFNDEF Footnote1__TX;
Constant Footnote1__TX "[Footnote ";
#ENDIF;
#IFNDEF Footnote2__TX;
Constant Footnote2__TX "]";
#ENDIF;
#IFNDEF Footnote3__TX;
Constant Footnote3__TX "Footnotes count upward from 1.";
#ENDIF;
#IFNDEF Footnote4__TX;
Constant Footnote4__TX "That footnote [";
#ENDIF;
#IFNDEF Footnote5__TX;
Constant Footnote5__TX "] has not been mentioned.";
#ENDIF;
#IFNDEF Footnote6__TX;
Constant Footnote6__TX "To view a footnote, type ~FOOTNOTE #~, where # is 
                        the number of the footnote you wish to read.";
#ENDIF;
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! End of Multi-lingual block
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

[ FNWarnSub;
        print_ret (string) Footnote6__TX;
        ];
[ Note n;
     if (footnotes_seen->n~=0 && Redisplay_Footnotes==1) rfalse;
     if (footnotes_seen->n==0) {
     footnote_count++; footnotes_seen->n=footnote_count;};
     print " ";
     style underline;
     print (string) Footnote1__TX,footnotes_seen->n, (string) Footnote2__TX;
     style roman;
];
[ FootnoteSub n;
     if (noun>footnote_count)
     {   print (string) Footnote4__TX,noun, (string) Footnote5__TX, "^"; rtrue; }
     if (noun==0) print_ret (string) Footnote3__tx;
     for(n=1:(footnotes_seen->n~=noun && n<=(MAX_FOOTNOTES-1)):n++);
     style underline;
     print (string) Footnote1__TX,noun,(string) Footnote2__tx, "^";
     style roman;
     PrintNote(n);
 ];

Verb meta "footnote" "note" *                           ->FNWarn
           * number                                     ->Footnote;
Extend "read"
           * "footnote" number                          ->Footnote
           * "note" number                              ->Footnote
           * "footnote"                                 ->FNWarn
           * "footnotes"                                ->FNWarn;

#Stub PrintNote 1;
