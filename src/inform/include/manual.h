!-----------------------------------------------------------------------------
!  Manual.H                     An Inform _6_ library (Backward compatable)
!                               to generate nicely-formatted browseable text
!                               for Z-Machine books and built-in manuals. 
!                               By L. Ross Raszewski
! 
! Yes, that's right.  The library-writer who brought you 'TimeWait', 'Date',
! 'PluralF', 'Footnotes', and all of those other useless #include files has 
! finally perfected (I hope) his manual-writing library in a form that allows
! you, yes you, to create an "Inform book".
! (I'm not being pompous, I'm just glad to have gotten this working, I've 
! been working on it for a while, and it's never been easy to modify.)
!
! So, how does it work?
! Simple:  you provide a call to the routine WriteManual with the 
! following arguments:
!
! WriteManual(DoFunc,Title,Titlelength,PageNames,PageNameLength,
!             NumberOfPages,[StartPageNumber]);
!
! DoFunc:  A function which can be called with 1 argument, and which prints
!               a page of text based on that argument (probably with a switch 
!               directive
! Title:   The Title you want to appear on the first line of the header 
!          (a string)
! Titlelength: The number of characters in the title (used for centering)
! PageNames: An array containing the titles of the pages to appear on 
!            the second line of the header
! PageNameLength: An Array containing the lengths of the PageNames
! NumberOfPages: Just what it says
! StartPageNumber: For indexing (I'll write that later), you can call the 
!                  manual to any page.  By default, it will start on page 0
!                  which should be a title page.
!
! Toss something like this into your code:
! [ ManualSub;
!   WriteManual(DoPage,"Zork - Game Manual",18,pspage,pspn,3);
! ];
! Verb meta "manual" "instructions" *                     ->Manual;
!
! and "Manual" will bring up a manual.  Your DoPage(mpage); should run 
! something like:
!
! [ DoPage i;
!     switch(i){
!               0: @erase_window -1; box "Zork Manual";
!               1: "This is page 1";
!               2: "This is Page 2";
!               ...
!               };
! ];
! pspage --> "Title" "Page 1" "Page 2" "Hey, Page 3";
! pspn -> 5 6 6 11;
!
! Note that the banner will only appear on the title page if the user 
! holds down on the "p" key (It will sort of flicker as it is drawn by 
! ManualPage and erased by DoPage).
!
! Up and P work as page up, Down and N work as page down, Esc and Q work as 
! exit, and [Space] will advance from page 0.
!
! This also checks pretty_flag, and produces a lowkey_Menu, which simply 
! writes to the screen and asks for an N, P or Q.  Lowkey menus will not 
! start on page 0, but the player can pageup to them.
!
! If you have any trouble, or comments on this or any of my libraries,
! _please_ write me at rraszews@skipjack.bluecrab.org
! I'm always glad to hear from people using my libraries.
!
! Thanks to Graham Nelson for Inform, The Guys on R.A.I-F, and whoever 
! told me that Inform 6.05 would actually _work_ on my story files.
!


[ WriteManual doman title titlel pagen pagel npages startp pnum flag;
  if (pretty_flag==0) {LowKeyManual(doman,title,pagen,npages,startp); rtrue;};
   pnum=startp;
   do {
        switch(ManualPage(pnum,pagen-->pnum,pagel->pnum,title,titlel,doman))
                {0: pnum--;
                 1: pnum++;
                 2: flag=1;};
        if (pnum<0) pnum=0;
        if (pnum>npages) pnum=pnum--;}
   until (flag==1);
   print "^^Back to story...^^"; rtrue;
];

[ ManualPage mpage header l title titlel doman i j keypress;
i=0->33; if (i==0) i=80;
l=l/2;
style roman;
@split_window 3; @set_window 1; style reverse;
@set_cursor 1 1; spaces(i); @set_cursor 2 1; spaces(i); @set_cursor 3 1; spaces(i);
j=(i/2)-(titlel/2); @set_cursor 1 j;print (string) title;style bold;style reverse;
j=(i/2)-l;@set_cursor 2 j; print (string) header; style roman; style reverse;
@set_cursor 2 2; print "P = previous page";
j=i-13 ; @set_cursor 2 j; print "N = next page";
@set_cursor 3 2; print "Q = resume story";
@set_cursor 3 j; print "Page ", mpage;
@set_window 0; style roman; font on; indirect(doman,mpage); new_line;
do { @read_char 1 0 0 keypress;} until ((keypress==129 or 'P' or 'p' or 130 
                                        or 'N' or 'n' or 'q' or 'Q' or 27) 
                                        || (keypress==32 or 10 or 13 && mpage==0));
@erase_window -1;
if (keypress==129 or 'P' or 'p') return 0;
if (keypress==32 or 10 or 13 && mpage==0) return 1;
if (keypress==130 or 'N' or 'n') return 1;
if (keypress==27 or 'Q' or 'q') return 2;
];

[ LowKeyManual doman title pagen npages startp i j k;
        if (startp==0) startp=1;
        print "^^^"; print (string) title;
        j=startp;
        do        
        { print (string) pagen-->j; print "^";
        indirect(doman,j);
        print "^^[Press ~N~ for the next page, ~P~ for the previous, 
                 or ~Q~ to quit]^^";
        do {@read_char 1 0 0 i;} until (i=='N' or 'n' or 'P' or 'p' or 'Q' or 'q');
        if (i=='N' or 'n') j++;
        if (i=='P' or 'p') j--;
        if (i=='Q' or 'q') k=1;
        if (j<0) j==0;
        if (j>npages) j--;
        #IFV5;
        DrawStatusLine();
        #ENDIF;
        } until (k==1);
];
