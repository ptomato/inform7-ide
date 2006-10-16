!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!
!     FKeys.h                         An Inform 6 library extension
!                                     by David Fillmore
!     Release 1.0                
!
! This library file should enable the use of Function Keys in your games.
! To make it work, Replace Keyboard and KeyboardPrimitive before inclusion of
! Parser.h, include this file after Parser.h, and put the line 'SetupFKeys();' somewhere
! in the initialise routine.
!
! If you do use this file in your IF, it'd be nice if you gave me credit.
!
! If you have any problems, questions, or suggesctions, please e-mail me at
! DCAFillmore@Hotmail.com
!
!


#ifndef fkeys;
Constant fkeys;
#endif;

#ifndef WORDSIZE;
Constant TARGET_ZCODE;
Constant WORDSIZE 2;
#endif;

#ifdef beyond_status;
ZCharacter terminating 129 130 133 134 135 136 137 138 139 140 141 142 143 144 146 147 148 149 150 151 152 153 154;
#ifnot;
ZCharacter terminating 133 134 135 136 137 138 139 140 141 142 143 144 146 147 148 149 150 151 152 153 154;
#endif;

Global termchar;

#ifndef MyFKeys;
[ SetupFKeys;
  style roman;
  F1->0=4;
  F1->1='L';
  F1->2='O';
  F1->3='O';
  F1->4='K';
  F1->5=124;

  F2->0=9;
  F2->1='I';
  F2->2='N';
  F2->3='V';
  F2->4='E';
  F2->5='N';
  F2->6='T';
  F2->7='O';
  F2->8='R';
  F2->9='Y';
  F2->10=124;

  #ifdef RPG;
  F3->0=5;
  F3->1='S';
  F3->2='T';
  F3->3='A';
  F3->4='T';
  F3->5='S';
  F3->6=124;
  #ifnot;
  F3->0=5;
  F3->1='O';
  F3->2='P';
  F3->3='E';
  F3->4='N';
  F3->5=' ';
  #endif;

  F4->0=8;
  F4->1='E';
  F4->2='X';
  F4->3='A';
  F4->4='M';
  F4->5='I';
  F4->6='N';
  F4->7='E';
  F4->8=' ';

  F5->0=5;
  F5->1='T';
  F5->2='A';
  F5->3='K';
  F5->4='E';
  F5->5=' ';

  F6->0=5;
  F6->1='D';
  F6->2='R';
  F6->3='O';
  F6->4='P';
  F6->5=' ';

  F7->0=7;
  F7->1='A';
  F7->2='T';
  F7->3='T';
  F7->4='A';
  F7->5='C';
  F7->6='K';
  F7->8=' ';

  F8->0=5;
  F8->1='A';
  F8->2='G';
  F8->3='A';
  F8->4='I';
  F8->5='N';
  F8->6=124;

  F9->0=4;
  F9->1='U';
  F9->2='N';
  F9->3='D';
  F9->4='O';
  F9->5=124;

  F10->0=5;
  F10->1='O';
  F10->2='O';
  F10->3='P';
  F10->4='S';
  F10->5=' ';

  F11->0=4;
  F11->1='A';
  F11->2='S';
  F11->3='K';
  F11->4=' ';

  F12->0=5;
  F12->1='T';
  F12->2='E';
  F12->3='L';
  F12->4='L';
  F12->5=' ';
];
#endif;

[ PressedKey a_buffer charnum x y z fkey a;
  style roman;
  !print termchar;
  switch(termchar)
  { 133 to 144: switch(termchar)
	            { 133: fkey=F1;
				  134: fkey=F2;
				  135: fkey=F3;
				  136: fkey=F4;
				  137: fkey=F5;
				  138: fkey=F6;
				  139: fkey=F7;
				  140: fkey=F8;
				  141: fkey=F9;
				  142: fkey=F10;
				  143: fkey=F11;
				  144: fkey=F12;
				}
				a=buffer->1;
				buffer->1=fkey->0+a;

	            x=(fkey->0)+1;
				for (y=1:y<x:y++)
                { buffer->(y+1+a)=(fkey->y);
				  print (char) buffer->(y+1+a);
		          if (buffer->(y+1+a) > $40 && buffer->(y+1+a) < $5b)
		          { buffer->(y+1+a)=(buffer->(y+1+a)+$20);
		          }
		        }

		        if (fkey->y==124)
		        { print "^";
		          z=0;
                }
		        else
		        z=1;

    #ifdef beyond_status;
     129: switch(scrollup)
         { 0: @sound_effect 2;
           1: descpart--;
              @erase_window 1;
              currentlinenum=0;
         }
		 z=1;

     130: switch(scrolldown)
         { 0: @sound_effect 2;
           1: descpart++;
              @erase_window 1;
              currentlinenum=0;
         }
		 z=1;
  #endif;


	146: buffer->1=9;
	     buffer->2='s';
		 buffer->3='o';
		 buffer->4='u';
		 buffer->5='t';
		 buffer->6='h';
		 buffer->7='w';
		 buffer->8='e';
		 buffer->9='s';
		 buffer->10='t';
		 print "SOUTHWEST^";
         z=0;
    147: buffer->1=7;
	     buffer->2='s';
		 buffer->3='o';
		 buffer->4='u';
		 buffer->5='t';
		 buffer->6='h';
		 print "SOUTH^";
         z=0;
    148: buffer->1=9;
	     buffer->2='s';
		 buffer->3='o';
		 buffer->4='u';
		 buffer->5='t';
		 buffer->6='h';
		 buffer->7='e';
		 buffer->8='a';
		 buffer->9='s';
		 buffer->10='t';
		 print "SOUTHEAST^";
         z=0;
	149: buffer->1=9;
	     buffer->2='w';
		 buffer->3='e';
		 buffer->4='s';
		 buffer->5='t';
		 print "WEST^";
         z=0;
	150: buffer->1=4;
	     buffer->2='w';
		 buffer->3='a';
		 buffer->4='l';
		 buffer->5='k';
		 print "WALK^";
         z=0;
    151: buffer->1=4;
	     buffer->2='e';
		 buffer->3='a';
		 buffer->4='s';
		 buffer->5='t';
		 print "EAST^";
         z=0;
    152: buffer->1=9;
	     buffer->2='n';
		 buffer->3='o';
		 buffer->4='r';
		 buffer->5='t';
		 buffer->6='h';
		 buffer->7='w';
		 buffer->8='e';
		 buffer->9='s';
		 buffer->10='t';
		 print "NORTHWEST^";
         z=0;
	153: buffer->1=5;
	     buffer->2='n';
		 buffer->3='o';
		 buffer->4='r';
		 buffer->5='t';
		 buffer->6='h';
		 print "NORTH^";
         z=0;
	154: buffer->1=9;
	     buffer->2='n';
		 buffer->3='o';
		 buffer->4='r';
		 buffer->5='t';
		 buffer->6='h';
		 buffer->7='e';
		 buffer->8='a';
		 buffer->9='s';
		 buffer->10='t';
		 print "NORTHEAST^";
         z=0;
    !#endif;
  }
  !145-154  keypad 0 to 9
  !146:
  font on;
  charnum=a_buffer->1;
  return z;


];

[ KeyboardPrimitive a_buffer a_table;
  #ifdef USECOLOURS;
  if (coloursavailable==1)
  { SetColours(4);
    @set_colour foreground background;
  }
  #endif;
  a_buffer->0 = 120;
  @aread a_buffer a_table -> termchar;
  
];

[ Keyboard  a_buffer a_table  nw i w w2 x1 x2;

    DisplayStatus();
    .FreshInput;

!  Save the start of the buffer, in case "oops" needs to restore it
!  to the previous time's buffer

    for (i=0:i<64:i++) oops_workspace->i = a_buffer->i;

!  In case of an array entry corruption that shouldn't happen, but would be
!  disastrous if it did:

   a_buffer->0 = 120;
   a_table->0 = 15;  ! Allow to split input into this many words

!  Print the prompt, and read in the words and dictionary addresses

    L__M(##Prompt);
    AfterPrompt();
    a_buffer->1=0;
    .xyzzy;
    #IFV5; DrawStatusLine(); #ENDIF;
    KeyboardPrimitive(a_buffer, a_table);
    if (termchar~=13 or 10)
    { if (PressedKey()==1)
	  { a_buffer=buffer;
	    jump xyzzy;
	  }
	  
	  @tokenise buffer a_table;
    }
	
	nw=a_table->1;
    
    

!  If the line was blank, get a fresh line
    if (nw == 0)
    { L__M(##Miscellany,10); jump FreshInput; }

!  Unless the opening word was "oops", return

    w=a_table-->1;
    if (w == OOPS1__WD or OOPS2__WD or OOPS3__WD) jump DoOops;

#IFV5;
!  Undo handling

    if ((w == UNDO1__WD or UNDO2__WD or UNDO3__WD) && (parse->1==1))
    {   if (turns==1)
        {   L__M(##Miscellany,11); jump FreshInput;
        }
        if (undo_flag==0)
        {   L__M(##Miscellany,6); jump FreshInput;
        }
        if (undo_flag==1) jump UndoFailed;
        if (just_undone==1)
        {   L__M(##Miscellany,12); jump FreshInput;
        }
        @restore_undo i;
        if (i==0)
        {   .UndoFailed;
            L__M(##Miscellany,7);
        }
        jump FreshInput;
    }
    @save_undo i;
    just_undone=0;
    undo_flag=2;
    if (i==-1) undo_flag=0;
    if (i==0) undo_flag=1;
    if (i==2)
    {   style bold;
        print (name) location, "^";
        style roman;
        L__M(##Miscellany,13);
        just_undone=1;
        jump FreshInput;
    }
#ENDIF;
    
    return nw;

    .DoOops;
    if (oops_from == 0)
    {   L__M(##Miscellany,14); jump FreshInput; }
    if (nw == 1)
    {   L__M(##Miscellany,15); jump FreshInput; }
    if (nw > 2)
    {   L__M(##Miscellany,16); jump FreshInput; }

!  So now we know: there was a previous mistake, and the player has
!  attempted to correct a single word of it.

    for (i=0:i<=120:i++) buffer2->i = a_buffer->i;
    x1 = a_table->9; ! Start of word following "oops"
    x2 = a_table->8; ! Length of word following "oops"

!  Repair the buffer to the text that was in it before the "oops"
!  was typed:

    for (i=0:i<64:i++) a_buffer->i = oops_workspace->i;
    Tokenise__(a_buffer,a_table);

!  Work out the position in the buffer of the word to be corrected:

    w = a_table->(4*oops_from + 1); ! Start of word to go
    w2 = a_table->(4*oops_from);    ! Length of word to go

!  Write spaces over the word to be corrected:

    for (i=0:i<w2:i++) a_buffer->(i+w) = ' ';

    if (w2 < x2)
    {   ! If the replacement is longer than the original, move up...

        for (i=120:i>=w+x2:i--)
            a_buffer->i = a_buffer->(i-x2+w2);

        ! ...increasing buffer size accordingly.

        a_buffer->1 = (a_buffer->1) + (x2-w2);
    }

!  Write the correction in:

    for (i=0:i<x2:i++) a_buffer->(i+w) = buffer2->(i+x1);

    Tokenise__(a_buffer,a_table);
    nw=a_table->1;

    return nw;
];

Array F1 -> 36;
Array F2 -> 36;
Array F3 -> 36;
Array F4 -> 36;
Array F5 -> 36;
Array F6 -> 36;
Array F7 -> 36;
Array F8 -> 36;
Array F9 -> 36;
Array F10 -> 36;
Array F11 -> 36;
Array F12 -> 36;