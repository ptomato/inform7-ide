! ---------------------------------------------------------------------------- !
!   BoxClever.h by Roger Firth (roger@firthworks.com)
!
!       1.0 Jul 2002
!           Original version.
!
! ---------------------------------------------------------------------------- !
!   Installation: add the line:
!
!       Include "BoxClever";
!
!   near the start of your game file.
!
! ---------------------------------------------------------------------------- !
!
!   BoxClever contains a replacement for Box__Routine, used by the compiler
!   to implement the 'box' statement. By defining your own version of
!   Box__Routine, you automatically override the standard definition (which
!   can be found in the compiler source file veneer.c).
!
!   The only purpose of the enhanced routine is to support the use of
!   emboldening and underlining (or italics) in boxed text. These are controlled
!   by surrounding the appropriate text by {...} and [..] respectively.
!   For example:
!
!       box "This display box" "uses both {bold}" "and [underlined] text";
!
!   You can use other controlling characters by defining the Constants boxB1,
!   boxB0 etc before Including this file. For example:
!
!       Constant boxB1 '<';
!       Constant boxB0 '>';
!       Include "BoxClever";

System_file;

Array   boxBuf -> 100;              ! enough to hold a line of characters
Default boxB1 '{';                  ! character which turns bold ON
Default boxB0 '}';                  ! character which turns bold OFF
Default boxU1 '[';                  ! character which turns underline ON
Default boxU0 ']';                  ! character which turns underline OFF

[ Box__Routine
    maxw                            ! length of longest line of text
    table                           ! no. of lines, text1, text2, ... textN
    n w w2 line lc t b c;           ! local variables
    n = table-->0;                  ! n = number of lines
    @add n 6 -> sp;
    @split_window sp;               ! create upper window of n+6 lines
    @set_window 1;                  ! select upper window
    w = 0->33;                      ! w = width of window
    if (w == 0) w = 80;
    w2 = (w - maxw)/2;              ! w2 = left margin

    style reverse;                  ! turn on reverse printing
    @sub w2 2 -> w;                 ! left/right border of two chars
    @set_cursor 4 w;                ! position cursor to top of box
    spaces maxw + 4;                ! print row of reverse spaces
    line = 5; lc = 1;               ! get ready for text1
    do {                            ! for each line of text...
        @set_cursor line w;         !   position cursor
        spaces maxw + 4;            !   print row of reverse spaces
!       style roman;                !   avoid reverse+bold|underline by...
!       @set_cursor line w2;        !   ...repositioning cursor, and...
!       spaces maxw;                !   ...printing row of normal spaces
        @set_cursor line w2;        !   reposition cursor
        t = table-->lc;             !   t = line of text
        @output_stream 3 boxBuf;
        print (string) t;           !   expand into characters in buffer
        @output_stream -3;
        for (b=0 : b<boxBuf-->0 : b++) {
            c = boxBuf->(b+2);
            switch (c) {
                nothing: ;
                boxB1:   style bold;
                boxB0:   style roman; style reverse;
                boxU1:   style underline;
                boxU0:   style roman; style reverse;
                default: print (char) c;
            }
        }
        line++; lc++;               !   done with this line
        style reverse;              !   get ready for next line
    } until (lc > n);               ! ...until all lines printed
    @set_cursor line w;             ! position cursor to bottom of box
    spaces maxw + 4;                ! print row of reverse spaces
    @buffer_mode true;              ! turn on word-breaking
    style roman;                    ! turn off reverse printing
    @set_window 0;                  ! select lower window
    @split_window 1;                ! create upper window of one line

    @output_stream -1;              ! turn off screen output
    print "[ ";                     ! start transcript of box
    lc = 1;                         ! get ready for text1
    do {
        t = table-->lc;             !   t = line of text
        @output_stream 3 boxBuf;
        print (string) t;           !   expand into characters in buffer
        @output_stream -3;
        for (b=0 : b<boxBuf-->0 : b++) {
            c = boxBuf->(b+2);
            if (c ~= nothing or boxB1 or boxB0 or boxU1 or boxU0)
                print (char) c;
        }
        lc++;                       !   done with this line
        if (lc > n) break;          !   done with all lines?
        print "^  ";                ! get ready for next line
    } until (false);
    print " ]^^";                   ! end of box transcript
    @output_stream 1;               ! turn on screen output
];

! ---------------------------------------------------------------------------- !
