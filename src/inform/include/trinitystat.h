! Trinitystat.h
! This library gives the ability to switch between the normal statusline and a
! Trinity style statusline. It works for both the Z-Machine and Glulx, and
! should work on all three compilers. (G.N.'s Z-Machine compiler, A.P.'s Glulx
! compiler, and A.P.'s biplatform compiler.) It has only been tested on the
! biplatform compiler, though.

! You must, of course, Replace DrawStatusLine; for this to work.

! An object is supplied, statline, with a property trinity. If trinity is true,
! then you will have a Trinity-style line. Guess what happens if trinity is
! false?

! If you have defined Constant DEBUG;, then you get an extra debugging verb.
! Typing "trinity" will switch statusline styles.

! Copyright (C) 2000 by Jonathan Rosebaugh. Released under GPL version 2.
! see http://www.gnu.org/copyleft/gpl.html

! This is necessary to compile with Graham's current Inform 6.21 compiler.
#ifndef WORDSIZE;
Constant TARGET_ZCODE;
Constant WORDSIZE 2;
#endif;

! Debugging verb
#ifdef DEBUG;
Object statline with trinity 0;
Verb meta "trinity"
* -> Trinity;
[TrinitySub;
statline.trinity = (~~(statline.trinity));
];
#endif;

! Z-Code version
#ifdef TARGET_ZCODE;
Array printed_text table 64;

#IFV5;
[ DrawStatusLine width posa posb i j;
if (statline.trinity)
{
  i = 0->33; if (i==0) i=80;
  font off;
  @split_window 1; @buffer_mode 0; @set_window 1;
  style reverse; @set_cursor 1 1; spaces(i);
  printed_text-->0 = 64;
  @output_stream 3 printed_text;
   if (location == thedark) print (name) location;
   else
   {   FindVisibilityLevels();
       if (visibility_ceiling == location)
           print (name) location;
       else print (The) visibility_ceiling;
   }
  @output_stream -3;
  j=(i-(printed_text-->0))/2;
  @set_cursor 1 j; print (name) location; spaces(j-1);
  style roman;
  @buffer_mode 1; @set_window 0; font on;
}
else {   @split_window 1; @set_window 1; @set_cursor 1 1; style reverse;
   width = 0->33; posa = width-26; posb = width-13;
   spaces width;
   @set_cursor 1 2;
   if (location == thedark) print (name) location;
   else
   {   FindVisibilityLevels();
       if (visibility_ceiling == location)
           print (name) location;
       else print (The) visibility_ceiling;
   }
   if ((0->1)&2 == 0)
   {   if (width > 76)
       {   @set_cursor 1 posa; print (string) SCORE__TX, sline1;
           @set_cursor 1 posb; print (string) MOVES__TX, sline2;
       }
       if (width > 63 && width <= 76)
       {   @set_cursor 1 posb; print sline1, "/", sline2;
       }
   }
   else
   {   @set_cursor 1 posa;
       print (string) TIME__TX;
       LanguageTimeOfDay(sline1, sline2);
   }
   @set_cursor 1 1; style roman; @set_window 0;
}
];
#ENDIF;

#endif; ! TARGET_ZCODE

! Glulx version
#ifdef TARGET_GLULX;
Array bluelalablankage-->1;


    [ DrawStatusLine width height posa posb centerarea;
        ! If we have no status window, we must not try to redraw it.
        if (gg_statuswin == 0)
            return;

        ! If there is no player location, we shouldn't try either.
        if (location == nothing || parent(player) == nothing)
            return;

        glk($002F, gg_statuswin); ! set_window
        StatusLineHeight(gg_statuswin_size);

        glk($0025, gg_statuswin, gg_arguments, gg_arguments+4);
        ! window_get_size
        width = gg_arguments-->0;
        height = gg_arguments-->1;
        posa = width-26; posb = width-13;

        glk($002A, gg_statuswin); ! window_clear

if (statline.trinity)
{
        if (location == thedark) {
            centerarea = (width/2) - (PrintAnyToArray(bluelalablankage,0, location)/2);
        }
        else {
            FindVisibilityLevels();
            if (visibility_ceiling == location)
                centerarea = (width/2) - ((PrintAnyToArray(bluelalablankage,0,
location)) / 2);
            else
                centerarea = (width/2) - (PrintAnyToArray(bluelalablankage,0,
visibility_ceiling)/2);
        }
        glk_window_move_cursor( gg_statuswin, centerarea, 0);
        if (visibility_ceiling == location)
            print (name) location;
        else
            print (The) visibility_ceiling;
}
else {

        glk($002B, gg_statuswin, 1, 0); ! window_move_cursor
        if (location == thedark) {
            print (name) location;
        }
        else {
            FindVisibilityLevels();
            if (visibility_ceiling == location)
                print (name) location;
            else
                print (The) visibility_ceiling;
        }

        if (width > 66) {
            glk($002B, gg_statuswin, posa-1, 0); ! window_move_cursor
            print (string) SCORE__TX, sline1;
            glk($002B, gg_statuswin, posb-1, 0); ! window_move_cursor
            print (string) MOVES__TX, sline2;
        }
        if (width > 53 && width <= 66) {
            glk($002B, gg_statuswin, posb-1, 0); ! window_move_cursor
            print sline1, "/", sline2;
        }
}
        glk($002F, gg_mainwin); ! set_window
    ];
#endif; ! TARGET_GLULX


