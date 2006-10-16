! mistype.h for Inform 6 with library 6/10
!
! A library to automatically correct minor typing errors by the player, in a similar way
! to nitfol but on other interpreters.
!
! Written by C Knight.  Not copyrighted.
! Comments and bug reports welcomed: please see www.metebelis3.free-online.co.uk to email.
!
! To use, include the file after including 'parser', and put
! 'Replace KeyboardPrimitive;' before 'parser'.
! So your source might look like:
!     Replace KeyboardPrimitive;
!     Include "Parser";
!     Include "Mistype";
!
! (Defining the constant 'QUICKCHECK' before including this file only looks
! for inserted or transposed letters, not omissions or most substitutions,
! and so may be useful if targetting slow platforms.  Defining the constant
! 'NAMEABLES' allows objects with their own parse_names and general parsing routines,
! like the cubes in 'Balances', to pass unchecked.)
!
! You can disable the whole correction feature for a while by setting mistype_off
! to 1.  The user has separate controls - both must be on for correction to happen.
! 'mistype off'; 'mistype low' (only corrects single-letter errors); 'mistype on' (default)
! You are also reminded of DM4 p232: 'If you want objects to be unreferrable-to, put
! them somewhere else!'
!
! Adds about 2.5K to output Z-code file. (v 1.11 was 580 bytes - still available
! from website above)
!
! Version history
! v 1.0, (beta) 2 May 03  First version
! v 1.1, 6 May 03.  Added Glulx optimisations, 'off switch' (Gunther's idea)
!                   Also buffer length checks and garbage detector.
! v 1.11 8 May 03   Nitfol and Zip extreme issues addressed.  No deletions from 2-letter words (RF).
! v 1.2  12 May 03  Keyboard, grammar and scope sensitivity added.
!         (beta)    Now deals with multiple misspellings in last resort case 
!                   Fixed couple of retained zeroes in Glulx buffer (hacky)
!                   Changed criteria for transposing space (now only if also corrects next word)
!                   User command added
! v 1.21 16 May 03  Added 'mistype single', and separated user from programmer controls
!                   Superfluous spaces not printed. 'then' and stops accounted for.
!                   Last resort tweaks: optimised, prepositions searched, works with Nitfol,
!                   reduced distance for doubled letters.
!                   Made compatible with nameable objects and to ignore quoted words.
!
! (TODO: structure better while retaining speed. fix code indentation and comments)
!
! The entry point chosen (KeyboardPrimitive) should not conflict with many other
! contributed libraries, or non-English versions of Inform.  You could instead
! trap LanguagetoInformese(), BeforeParsing(), or Tokenise__().
!

! Can override these constants by defining them before including the file:
Default MISTYPE_MSG1 "[The story read that as ~";
Default MISTYPE_MSG2 "~].^";
Default MISTYPE_FUZZ 22;   ! lower means fewer corrections
! 10 is quite selective - about 2 errors or twisted; 30 can replace word completely;

! For use by main file if needed, and separate from user control:
Global mistype_off = false;

Global mistype_level = 2;  ! player control - 1 turns off 'last-resort' check
Global mistype_close;  ! for passing info between Fits() & main routine
Global mistype_loose;

!---------------------------------------------------

Array KeyNear -> "qldsvnxvfswrgdhfjguokhjlkpknbmpilowatedayryibcqezcutxalpa p lmpe";
!                 @`A B C D E F G H I J K L M N O P Q R S T U V W X Y Z [;\#] ~ ? 

#ifndef QUICKCHECK;
Array NounPhraseConnectives table  ! additional words not mentioned elsewhere
ALL1__WD ALL2__WD ALL4__WD AND1__WD BUT1__WD BUT2__WD ME1__WD OF1__WD OTHER2__WD THEN1__WD;
#endif;

[ WordInScope w r;
  ! approximate scope rules to help choose the preferable object
  ! doesn't prevent player from checking if a word is recognised
  ! called from Fits()
  while (r) {
    if (WordInProperty(w,r,name)) rtrue;
#ifdef pname;
    if (r provides pname && WordInProperty(w,r,pname)) rtrue;
#endif;
#ifdef scenic;
    if (r provides scenic && WordInProperty(w,r,scenic)) rtrue;
#endif;

    ! find the next object
    if (child(r) && (r hasnt container || r has transparent or open or supporter or visited) )
      r=child(r);  ! ?? if (r==Darkness) r=player;
    else {
       while (r && sibling(r)==0) r=parent(r);
       if (r) (r=sibling(r));
       }
    }
  ! could also check articles etc at this point when nouns are expected
  ! for (r=1: r<=LanguageDescriptors-->0: r=r+4) if (w == LanguageDescriptors-->r or LanguageDescriptors-->(r+3)) rtrue;
  
  rfalse;
  ];

#ifdef the_named_word;
Constant NAMEABLES;
#endif;

#ifdef NAMEABLES;
! optional check for non-dictionary words
Global mistype_parse;
Global mistype_named;

[ NamedParsesO obj  t;
  if (mistype_named<=0 && obj provides parse_name ) {
    t=wn; ! just in case
    wn=mistype_parse;
    mistype_named=obj.parse_name();
    wn=t;
    }
  ];
#message "Compiling spell-check to ignore nameable objects.";
#endif;  ! NAMEABLES

#ifndef TARGET_GLULX;

Replace KeyboardPrimitive;

!--------------------------------------------
! Last resort matching for Zcode
!--------------------------------------------

#ifndef QUICKCHECK;
Global mistype_bestv;  ! for 'last resort' matching
Global mistype_bestw;
Global mistype_start;
Global mistype_final;

Array mistype_names -> 16;

[ CheckWordDist w   offs oldoff i j scr inc;
  ! create a score scr, which is lower the more similar word w is to
  ! the string in the buffer between mistype_start and _final
  @output_stream 3 mistype_names;
  print (address) w;
  @output_stream -3;
  for (i=0: scr<mistype_bestv && i<=mistype_final-mistype_start && i<=9-offs: i++) {
    inc=5;
    if (i>0 && mistype_start->i==mistype_start->(i-1))
       inc=2; ! doubled letters count for little
    scr=scr+inc; ! if not found
    for (j=2: j<2+mistype_names->1: j++) {
      if (mistype_start->i==mistype_names->j) {
        offs=(j-2 - i);
        if (offs<oldoff)
           scr=scr-inc+oldoff-offs;  !+offs/2
        else
           scr=scr-inc+offs-oldoff;
        oldoff=offs;
        mistype_names->j = '?';
        break;
        }
      }
    } ! for i
  for (j=2: j<2+mistype_names->1: j++)
     if (mistype_names->j ~= '?') scr=scr+6;
  if (scr<mistype_bestv) {
     mistype_bestv=scr;
     mistype_bestw=w;
     }
  ];

[ CheckWordDistO obj  i;
  for (i=0: i<obj.#name/2: i++) CheckWordDist(obj.&name-->i);
  ];
#endif;

!--------------------------------------------
! Replacement KeyboardPrimitive for Zcode
!--------------------------------------------

[ Fits p i  flag gram lineleft j tok;
  ! Does word 'i' make grammatical sense in parse-buffer context 'p'?

  if (mistype_loose) rtrue; ! accept it on second pass
  mistype_close=1;  ! worth a second pass if there's something this close
  ! yes, I know you shouldn't set globals in functions, but it's economical

  flag=(p-->(i*2+1))->#dict_par1;
  ! +128 if given a noun, +8 if a preposition, +4 if plural, +1 if a verb
  if ((i==0 || p-->(i*2-1)==',//' or './/' or THEN1__WD) && p~=parse2)
    return (flag&1 || WordInScope(p-->(i*2+1), Compass) );  ! verb
  gram=(p-->1)->#dict_par2;
  if ((~~p-->1) || (~~gram)) return (p-->(i*2+1)~=0); ! unrec verb or command - no preference.
  ! and return 0 (don't suppress) if checking for GPR

  ! a rough and ready pseudo-parse searching for prepositions
  gram=(0-->7)-->(255-gram);
  lineleft = (gram)->0;
  j=1;
  while (lineleft && j<=i) {
    gram=gram+3;
    if ((tok=gram->0 &$F) ==ENDIT_TOKEN) { ! end of grammar line
      j=1;
      lineleft--;
      }
    else if (tok==PREPOSITION_TT) {  ! preposition
      if ((gram+1)-->0 == p-->(j*2+1)) {
        if (i==j) rtrue; ! perfect match
        j++;
        }
      }
    else { ! looking for noun
      if (tok==GPR_TT) rtrue; ! could be anything - even matches unrecognised (0)
      while ( j<p->1 && (p-->(j*2+1))->#dict_par1 & 136 == 128) { ! noun, not prep
        ! could add LanguageDescriptor check here as in line c330.
        if (j==i && WordInScope (p-->(j*2+1), location) )  ! a noun where a noun should be
          rtrue;
        j++;
        }
      }
    };

  rfalse; ! not a particularly good match
    
  ];

[ KeyboardPrimitive  buffer parse  i start len j c1 c2 t n alt fail quot;
  read buffer parse;           ! variables renamed
#iftrue #version_number == 6;  ! part of patch L61022 by Kevin Bracey
  @output_stream -1;
  @loadb buffer 1 -> sp;
  @add buffer 2 -> sp;
  @print_table sp sp;
  new_line;
  @output_stream 1;
#endif;

  if (mistype_off || ~~mistype_level) rtrue;  ! programmer and player control respectively

  ! JZIP has problems with overflow - can't CR if buffer is full
  ! if it's too many *words*, suppress the xzip/jzip warning
  if (parse->1 > parse->0) { 
    ! print "[Suppressing further 'too many words' warnings.]^";
    buffer->1 = parse->(1+4*parse->0)+parse->(4*parse->0)-2; ! truncate to 15 words
    buffer->(2+buffer->1)=0;
    }

! check for misspelled words

  if (buffer->1 < buffer->0)
    buffer->(2 + buffer->1) = 0; ! see library tokenise__

  for (i=0: i<parse->1 && fail < alt+4: i++)
    if (parse-->(n=i*2+1) == '~//') {quot = ~~quot; } else   ! ignore anything in quotes
    if (~~ (parse-->(n=i*2+1) || quot)) {  ! unrecognised word

      ! try fixing word
      start=parse->(i*4+5); 
      len=parse->(i*4+4);
      t = start+len-1;
      if (buffer->start <= '9' && buffer->t <='9') jump NxtChk;  ! numbers can't be tokenised

      ! oops_from=saved_oops=i+1; & 'oops_word=saved_oops;' in ParserError()
      ! it would be nice to fiddle this in case of a miscorrection 

#ifdef NAMEABLES;  ! check for valid non-dictionary words
      mistype_loose=0; ! ensure Fits checks against GPR
      if (i>0 && Fits(parse, i)) jump NxtChk; ! may match a GPR (general parse routine)
      parser_action=0;
      mistype_parse=i+1; ! try parsing from this word number
      mistype_named=0;
      if (parse~=parse2) ! don't bother if disambiguating
         LoopOverScope (NamedParsesO);
      if (mistype_named>0) {
         i=i+mistype_named-1; ! if matched multiple words
         continue; ! check anything after match
         }
#endif;

      mistype_close=0;
      for (mistype_loose=0->31=='N': mistype_loose<=mistype_close: mistype_loose++) {
      ! try a second pass if only found poor fits
      ! ignore whole loop for nitfol

      !first try transpositions
      for (j=start+len +1 - 2*(i+1==parse->1 || len<3): j>start: j--) {
        c1=buffer->j; 
        buffer->j=buffer->(j-1);
        buffer->(j-1)=c1;
        @tokenise buffer parse;
        if (parse-->n && Fits (parse,i) && (j<=t || parse-->(i*2+3)) ) jump NxtChk;  ! fixed!
                !  - only if next word is fixed
        buffer->(j-1)=buffer->j; ! undo transposition
        buffer->j=c1;
        } ! end of for loop

      ! now common substitutions - such as nearby keys
      for (j=t: j>=start: j--) {
        c2=buffer->j;
        if (j<t && j>start && c2 == buffer->(j-1) or buffer->(j+1) ) { ! repeated wrong letter?
            buffer->j = buffer->(j-1) + buffer->(j+1) - c2;
            @tokenise buffer parse;
            if (parse-->n && Fits (parse,i)) jump NxtChk;  ! fixed!
        	}
        if (c2=='#')
            c1=28*2;  ! UK keyboard, near Ret key
        else
            c1 = (c2 & $1F)*2;
        do {
            buffer->j = KeyNear->c1;
            @tokenise buffer parse;
            if (parse-->n && Fits (parse,i) ) jump NxtChk;  ! fixed!
            }
        until (c1++ & 1);   ! do two choices

        buffer->j=c2; ! restore
        }

      !now try deletions
      !  could impose a (len>2) test so don't turn words into single letters
      c1 = ' ';
      for (j=t: j>=start: j--) { ! j represents the one that's missed out
        c2=buffer->j;
        buffer->j=c1;
        c1=c2;
        @tokenise buffer parse;
        if (parse-->n && Fits (parse,i) ) jump NxtChk;  ! fixed!
        }
      ! and restore
      for (j=t: j>start: j--)
        buffer->j=buffer->(j-1);
      buffer->start=c2;

#ifndef QUICKCHECK;

      ! additions
      if (buffer->1 < buffer->0) {  ! not if could overflow
        (buffer->1)++;
        buffer->(2 + buffer->1) = 0; ! see library tokenise__

        for (j=buffer->1+1: j>t: j--)
          buffer->(j+1)=buffer->j;
        for (j=t+1: j>=start: j--) {
          for (c1='a': c1<='z': c1++) {
            buffer->j=c1;
            @tokenise buffer parse;
            if (parse-->n && Fits (parse,i) ) jump NxtChk;  ! fixed!
            }
          buffer->j=buffer->(j-1);
          }
        ! restore
        for (j=start: j<=buffer->1: j++)
          buffer->j=buffer->(j+1);
        (buffer->1)--;
        }

      ! all substitutions, a-z
      for (j=t: j>=start: j--) {
        c2=buffer->j;
        for (c1='a': c1<='z': c1++) {
          buffer->j=c1;
          @tokenise buffer parse;
          if (parse-->n && Fits (parse,i) ) jump NxtChk;  ! fixed!
          }
        buffer->j=c2;
        }

#endif;

      }

#ifndef QUICKCHECK;
      if (mistype_level>1) {
      ! try something more drastic - compare to relevant words
      mistype_bestv=1000; mistype_bestw=0;
      mistype_start=buffer+start; mistype_final=buffer+t;
      if (i==0 || parse-->(i*2-1)==',//' or './/' or THEN1__WD) { ! verb?
         ! call CheckWordDist for all verbs
         for (j=0-->4 +7:j < 0-->2 -5: j=j+9) 
            if (j->#dict_par1 &1) 
               CheckWordDist (j);
         }
      else { ! consider all likely words
         ! first, nouns
         LoopOverScope(CheckWordDistO);

         ! now articles and other determiners
         for (j=1: j<=LanguageDescriptors-->0: j=j+4) {
            CheckWordDist(LanguageDescriptors-->j);
            if (LanguageDescriptors-->(j+3)+2 > 0-->4)
               CheckWordDist(LanguageDescriptors-->(j+3));
            }  ! end loop over LanguageDescriptors

         ! check against all prepositions for verb, regardless of position
         c1=(parse-->1)->#dict_par2;
         if (parse-->1 && c1) { ! known verb or command
            c1=(0-->7)-->(255-c1); ! address of grammar for verb
            j = (c1)->0; ! number of grammar lines
            while (j) {
               c1=c1+3;
               if ((c1->0 &$F) > 9)  ! end of grammar line
                  j--;
               else if ((c1->0 &$F)==2) ! preposition
                  CheckWordDist((c1+1)-->0);
               } ! while any grammar left
            } ! if (c1)

         ! miscellanous words from our array
         for (j=1: j<=NounPhraseConnectives-->0: j++) {
            CheckWordDist(NounPhraseConnectives-->j);
            }
         }

      if (mistype_bestv < MISTYPE_FUZZ) {  ! found something vaguely similar
         @output_stream 3 mistype_names;
         print (address) mistype_bestw;
         @output_stream -3;
         if (mistype_names->1 > len) { ! move up
            buffer->1 = buffer->1 +mistype_names->1-len;
            if (buffer->1 > buffer->0) buffer->1=buffer->0;
            for (j=buffer->0 +1: j>=start+mistype_names->1: j--) {
               buffer->j=buffer->(j+len-mistype_names->1);
               }
            }
         for (j=0: j<mistype_names->1: j++) ! and copy into buffer
            buffer->(start+j) = mistype_names->(2+j);
         for (:j<len:j++) buffer->(start+j)=' '; ! blank any overlap
         @tokenise buffer parse;
         }
      }
#endif;

      .NxtChk; !outer loop

      if (parse-->n) alt++;
      else fail++;
      } ! end of 'if' each word needs fixing

  if (alt && fail<=alt+2) {
    print (string) MISTYPE_MSG1;  ! [The story read that as "
    for (i=2: i<=1+buffer->1: i++)
      if (buffer->i > ' ' || (i<1+buffer->1 && buffer->(i+1) > ' '))  ! ignore repeated spaces
        print (char) buffer->i;
    print (string) MISTYPE_MSG2;  ! "]
    }

  ]; ! end of KeyboardPrimitive
  
#ifnot;

!--------------------------------------------
! Last resort matching for Glulx
!--------------------------------------------

#ifndef QUICKCHECK;

Global mistype_bestv;  ! for 'last resort' matching
Global mistype_bestw;
Global mistype_start;
Global mistype_final;

Array mistype_names -> DICT_WORD_SIZE+4;

[ CheckWordDist w   offs oldoff i j scr len inc;
  ! create a score scr, which is lower the more similar word w is to
  ! the string in the buffer between mistype_start and _final
  len=PrintAnyToArray(mistype_names, DICT_WORD_SIZE, w);  ! print word

  for (i=0: scr<mistype_bestv && i<=mistype_final-mistype_start && i<=DICT_WORD_SIZE-offs: i++) {
    inc=5;
    if (i>0 && mistype_start->i==mistype_start->(i-1))
       inc=2; ! doubled letters count for little
    scr=scr+inc; ! if not found
    for (j=0: j<len: j++) {
      if (mistype_start->i==mistype_names->j) {
        offs=(j - i);
        if (offs<oldoff)
           scr=scr-inc+oldoff-offs;  !+offs/2
        else
           scr=scr-inc+offs-oldoff;
        oldoff=offs;
        mistype_names->j = '?';
        break;
        }
      }
    } ! for i
  for (j=0: j<len: j++)
     if (mistype_names->j ~= '?') scr=scr+6;
  if (scr<mistype_bestv) {
     mistype_bestv=scr;
     mistype_bestw=w;
     }
  ];

[ CheckWordDistO obj  i;
  for (i=0: i<obj.#name/WORDSIZE: i++) CheckWordDist(obj.&name-->i);
  ];

#endif;

!--------------------------------------------
! Replacement KeyboardPrimitive for Glulx
!--------------------------------------------

[ Fits p i w   flag gram lineleft j tok;
  ! Does word 'i' make grammatical sense in parse-buffer context 'p'?
 
  if (mistype_loose) rtrue; ! accept it on second pass
  mistype_close=1;  ! worth a second pass if there's something this close

  if (w==0) w=p-->(i*3+1);
  else p-->(i*3+1)=w;   ! synchronise with parse
  flag=w->#dict_par1;
  ! +128 if given a noun, +8 if a preposition, +4 if plural, +1 if a verb
  if ((i==0  || p-->(i*3-2)==',//' or './/' or THEN1__WD) && p~=parse2)
     return (flag&1 || WordInScope(w, Compass) );  ! verb
  gram=(p-->1)->#dict_par2;
  if ((~~p-->1) || (~~gram)) return (w~=0); ! unrec verb or command - no preference.
  ! and return 0 (don't suppress) if checking for GPR

  ! a rough and ready pseudo-parse searching for prepositions
  gram=(#grammar_table+WORDSIZE)-->(255-gram);

  lineleft = (gram)->0;
  j=1; gram--;
  while (lineleft && j<=i) {
     gram=gram+5;
     if ((tok=gram->0 &$F) ==$F) { ! end of grammar line
        j=1; gram--;
        lineleft--;
        }
     else if (tok==2) {  ! preposition
        if ((gram+1)-->0 == p-->(j*3+1)) {
           if (i==j) rtrue; ! perfect match
           j++;
           }
        }
     else { ! looking for noun
        if (tok==GPR_TT) rtrue; ! could be anything - even matches unrecognised (0)
        while (j<p-->0 && (p-->(j*3+1))->#dict_par1 & 136 == 128) { ! noun, not prep
           if (j==i && WordInScope (w, location) )  ! a noun where a noun should be
              rtrue;
           j++;
           }
        }
     };

  rfalse; ! not a particularly good match
    
  ];

[ KeyboardPrimitive  buffer parse  i start len j c1 c2 t n alt fail quot
            done dictstart wordstart dictlen entrylen keylen res c3;

  done = false;
  ! this bit lifted from Andrew Plotkin's biplatform parserm.h
  glk($00D0, gg_mainwin, buffer+WORDSIZE, INPUT_BUFFER_LEN-WORDSIZE, 
    0); ! request_line_event
  while (~~done) {
    glk($00C0, gg_event); ! select
    switch (gg_event-->0) {
      5: ! evtype_Arrange
        DrawStatusLine();
      3: ! evtype_LineInput
        if (gg_event-->1 == gg_mainwin) {
          buffer-->0 = gg_event-->2;
          done = true;
        }
    }
    t = HandleGlkEvent(gg_event, 0, buffer);
    if (t == 2) {
      done = true;
    }
    else if (t == -1) {
      done = false;
    }
  }

  ! Close any quote window
  if (gg_quotewin) {
    glk($0024, gg_quotewin, 0);
    gg_quotewin = 0;
  }

  if (mistype_off || ~~mistype_level) rtrue;  ! programmer and player control respectively

! check for misspelled words

  Tokenise__(buffer, parse);
  dictlen = #dictionary_table-->0;             ! for use in later tokenisation
  entrylen = DICT_WORD_SIZE + 7;
  dictstart = #dictionary_table + WORDSIZE;

  for (i=0: i<parse-->0 && fail < alt+4: i++)   ! loop over each word, with garbage escape
    if (parse-->(n=i*3+1) == '"//' or '~//') quot = ~~quot; else   ! ignore anything in quotes
    if (~~ (parse-->(n=i*3+1) || quot)) {  ! unrecognised word

      ! try fixing word

      start=parse-->(i*3+3); 
      len=parse-->(i*3+2);
      keylen=len+1; if (keylen>DICT_WORD_SIZE) keylen=DICT_WORD_SIZE;
      res=0; 
      wordstart=buffer+start;
      t = start+len-1;
      c3=buffer->(start+len); ! keep the original word separator, while it is zeroed
      if (buffer->start <= '9' && buffer->t <='9') jump NxtChk;  ! numbers can't be tokenised

      ! saved_oops=i+1;
      ! it'd be nice to do this in case of a miscorrection - need more work
      ! suggest putting 'oops_word=saved_oops;' in ParserError()

#ifdef NAMEABLES;  ! check for valid non-dictionary words
      mistype_loose=0; ! ensure Fits checks against GPR
      if (i>0 && Fits(parse, i)) jump NxtChk; ! may match a GPR (general parse routine)
      parser_action=0;
      mistype_parse=i+1; ! try parsing from this word number
      mistype_named=0;
      if (parse~=parse2) ! don't bother if disambiguating
         LoopOverScope (NamedParsesO);
      if (mistype_named>0) {
         i=i+mistype_named-1; ! if matched multiple words
         continue; ! check anything after match
         }
#endif;

      ! convert to lower case
      for (j=start: j<start+len: j++) {
         c1=buffer->j;
         if (c1>='A' && c1<='Z') buffer->j=c1+32;
         }

      mistype_close=0;
      for (mistype_loose=0: mistype_loose<=mistype_close: mistype_loose++) {
      ! try a second pass if only found poor fits

      !first try transpositions
      for (j=start+len +1 - 2*(i+1==parse-->0 || len<3): j>start: j--) {
         c1=buffer->j; 
         buffer->j=buffer->(j-1);
         buffer->(j-1)=c1;

         if (j>=start+len-1) { ! first pass may move spaces, second moves back.
            Tokenise__(buffer, parse);
            c3=buffer->(start+len); ! keep the original word separator, while it is zeroed
            if (parse-->n && Fits (parse,i) && (j<=t || parse-->(i*3+4)) ) jump NxtChk;  ! fixed!
            }
         else {
            if (start+len<INPUT_BUFFER_LEN)
               buffer->(start+len)=0; ! end marker so search is accurate
            @binarysearch wordstart keylen dictstart entrylen dictlen 1 1 res;
            if (res && Fits (parse,i,res)) jump NxtChk;  ! fixed
            }
         
         buffer->(j-1)=buffer->j; ! undo transposition
         buffer->j=c1;
         } ! end of for loop
      if (start+len<INPUT_BUFFER_LEN)
         buffer->(start+len)=0; ! end marker so search is accurate

      ! now common substitutions - nearby keys and false repetitions
      for (j=t: j>=start: j--) {
         c2=buffer->j;
         if (j<t && j>start && c2 == buffer->(j-1) or buffer->(j+1) ) { ! repeated wrong letter?
            buffer->j = buffer->(j-1) + buffer->(j+1) - c2;
            @binarysearch wordstart keylen dictstart entrylen dictlen 1 1 res;
            if (res && Fits (parse,i,res)) jump NxtChk;  ! fixed!
        	}
         if (c2=='#')
            c1=28*2;  ! UK keyboard, near Ret key
         else
            c1 = (c2 & $1F)*2;
         do {
            buffer->j = KeyNear->c1;
            @binarysearch wordstart keylen dictstart entrylen dictlen 1 1 res;
            if (res && Fits (parse,i,res)) jump NxtChk;  ! fixed!
            }
         until (c1++ & 1);   ! do two choices

         buffer->j=c2; ! restore
         }

      !now try deletions
      c1 = 0;
      for (j=t: j>=start: j--) { ! j represents the one that's missed out
         c2=buffer->j;
         buffer->j=c1;
         c1=c2;
         @binarysearch wordstart keylen dictstart entrylen dictlen 1 1 res;
         if (res && Fits (parse,i,res)) { ! fixed
            buffer->t = ' '; ! replace the extra zero
            jump NxtChk;  
            }
         }
      ! and restore
      for (j=t: j>start: j--)
         buffer->j=buffer->(j-1);
      buffer->start=c2;

#ifndef QUICKCHECK;

      ! additions
      if (buffer-->0 >= INPUT_BUFFER_LEN-WORDSIZE) jump NxtChk;  ! not if could overflow
      (buffer-->0)++;
      for (j=buffer-->0+WORDSIZE-1: j>t: j--)
         buffer->(j+1)=buffer->j;
      keylen++; if (keylen>DICT_WORD_SIZE) keylen=DICT_WORD_SIZE;
      len++;
      if (start+len<INPUT_BUFFER_LEN)
         buffer->(start+len)=0; ! end marker so search is accurate

      for (j=t+1: j>=start: j--) {
         for (c1='a': c1<='z': c1++) {
            buffer->j=c1;
            @binarysearch wordstart keylen dictstart entrylen dictlen 1 1 res;
            if (res && Fits (parse,i,res)) jump NxtChk;  ! fixed
            }
         buffer->j=buffer->(j-1);
         }
      ! restore
      for (j=start: j<buffer-->0+WORDSIZE-1: j++)
         buffer->j=buffer->(j+1);
      (buffer-->0)--; len--;
      keylen=len+1; if (keylen>DICT_WORD_SIZE) keylen=DICT_WORD_SIZE;
      if (start+len<INPUT_BUFFER_LEN)
         buffer->(start+len)=0; ! end marker so search is accurate

      ! all substitutions, a-z
      for (j=t: j>=start: j--) {
         c2=buffer->j;
         for (c1='a': c1<='z': c1++) {
            buffer->j=c1;
            @binarysearch wordstart keylen dictstart entrylen dictlen 1 1 res;
            if (res && Fits (parse,i,res)) jump NxtChk;  ! fixed
            }
         buffer->j=c2;
         }

#endif;
      buffer->(start+len)=c3;
      }

#ifndef QUICKCHECK;
      if (mistype_level>1) {
      ! try something more drastic - compare to relevant words
      mistype_bestv=1000; mistype_bestw=0;
      mistype_start=buffer+start; mistype_final=buffer+t;
      if (i==0 || parse-->(i*3-2)==',//' or './/' or THEN1__WD) { ! verb?
         for (j=dictstart: j < dictstart+dictlen*entrylen: j=j+entrylen) 
            if (j->#dict_par1 &1) 
               CheckWordDist (j);
         }
      else { ! consider all likely words
         ! first, nouns
         LoopOverScope(CheckWordDistO);

         ! now articles and other determiners
         for (j=1: j<=LanguageDescriptors-->0: j=j+4) {
            CheckWordDist(LanguageDescriptors-->j);
            if (LanguageDescriptors-->(j+3) > dictstart -2)
               CheckWordDist(LanguageDescriptors-->(j+3));
            }  ! end loop over LanguageDescriptors

         ! check against all prepositions for verb, regardless of position
         c1=(parse-->1)->#dict_par2;
         if (parse-->1 && c1) { ! known verb or command
            c1=(#grammar_table+WORDSIZE)-->(255-c1); ! address of grammar for verb
            j = (c1)->0; ! number of grammar lines
            c1--;
            while (j) {
               c1=c1+5;
               if ((c1->0 &$F) > 9)  ! end of grammar line - ENDIT_TOKEN
                  {j--; c1--;}
               else if ((c1->0 &$F)==2) ! preposition
                  {
                  CheckWordDist((c1+1)-->0);
                  }
               } ! while any grammar left
            } ! if (c1)

         ! miscellaneous words from array
         for (j=1: j<=NounPhraseConnectives-->0: j++) {
            CheckWordDist(NounPhraseConnectives-->j);
            }
         }

      if (mistype_bestv < MISTYPE_FUZZ) {  ! found something vaguely similar
         c1 = PrintAnyToArray (mistype_names, DICT_WORD_SIZE, mistype_bestw);
         ! c1 is now length of new word
         if (c1 > len) { ! move up
            buffer-->0 = buffer-->0 +c1-len;
            if (buffer-->0 > INPUT_BUFFER_LEN-WORDSIZE)
               buffer-->0 = INPUT_BUFFER_LEN-WORDSIZE;

            for (j=INPUT_BUFFER_LEN-1: j>=start+c1: j--) {
               buffer->j=buffer->(j+len-c1);
               }
            }
         for (j=0: j<c1: j++) ! and copy into buffer
            buffer->(start+j) = mistype_names->j;
         for (:j<len:j++) buffer->(start+j)=' '; ! blank any overlap
         res=mistype_bestw;
         if (len<c1)  ! for benefit of c3 restoration
            len=c1;
         }
      } ! end of last-resort checking
#endif;

      .NxtChk; !outer loop

      ! remove any temporary zeroes from input
      buffer->(start+len)=c3;
      if (parse-->n || res) {
         alt++;
         Tokenise__(buffer, parse);  ! update ready for next word
         }
      else fail++;
      } ! end of 'if' word needs fixing

  if (alt && fail<=alt+2) {
     print (string) MISTYPE_MSG1;  ! [The story read that as "
     for (i=WORDSIZE: i<WORDSIZE+buffer-->0: i++)
        if (buffer->i > ' ' || (i+1<WORDSIZE+buffer-->0 && buffer->(i+1) > ' '))  ! ignore repeated spaces
           print (char) buffer->i;
     print (string) MISTYPE_MSG2;  ! "]
     }

  ]; ! end of KeyboardPrimitive

#endif;

!--------------------------------------------
! End user commands
!--------------------------------------------

[ MistypeOffSub;
  mistype_level=0;
  "[Automatic typing check is now off.]";
  ];

[ MistypeOnSub;
  mistype_level=2;
  "[Automatic typing check is now fully on.]";
  ];

[ MistypeSingleSub;
  mistype_level=1;
  "[Automatic typing check is now making only single-letter corrections.]";
  ];

Verb meta 'mistype' 'spellcheck' 'correction' 'typos' 'typing' 'atc'
  * 'off' -> MistypeOff 
  * 'on'/'high'/'full' -> MistypeOn
  * 'half'/'semi'/'single'/'low' -> MistypeSingle
  * -> MistypeOff;

! End of file: mistype.h
