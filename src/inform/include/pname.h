! pname.h, version 1.1, 10 May 2001, copyright 2001 Neil Cerutti. See
! pname.txt for use, distribution, and modification rights that are
! granted.
!
! An Inform Library 6/10 extension to allow more precise name definitions
! for objects and reduce the number of parse_name routines in the world.
!
! You probably won't find reading this file to be very illuminating. If you
! are looking for documentation for pname.h, please read pname.txt which is
! distributed with this file.

#ifndef WORDSIZE;
Constant WORDSIZE 2;
Constant INPUT_BUFFER_LEN 120;
Constant TARGET_ZCODE;
#endif;

Global matches_in_match_list;

Attribute phrase_matched;
Property additive pname;

Constant PHRASE_OP      '.p';
Constant OR_OP          '.or';
Constant OPT_OP         '.x';

! Returns the dictionary word wd_num from the input stream.
[ _pn_inpWord wd_num;
  #ifdef TARGET_ZCODE;
    return parse-->(wd_num*2-1);
  #ifnot;
    return parse-->(wd_num*3-2);
  #endif;
];

! Return the number of words stored in parse_table.
[ _pn_inpLen parse_table;
  #ifdef TARGET_ZCODE;
    return parse_table->1;
  #ifnot;
    return parse_table-->0;
  #endif;
];
  
[ _pn_parse obj
  i p_start p_end in_phrase pn_len n pn_addr inp_len
  match_total p_match_total;

  give obj ~phrase_matched;

  ! Count all input words that appear in pname and save in match_total. If
  ! match_total is zero, there's no point in checking for phrase matches so
  ! return 0.
  match_total = 0;
  inp_len = _pn_inpLen(parse);

  for (i = wn: i <= inp_len: i++)
  {
    if (WordInProperty(_pn_inpWord(i), obj, pname))
    {
      match_total++;
      if ((_pn_inpWord(i))->#dict_par1 & $$00000100)
        parser_action = ##PluralFound;
    }
    else
      break;
  }
  if (match_total == 0) return 0;

  pn_addr = obj.&pname; pn_len = obj.#pname/WORDSIZE;
  ! Find phrases in pname and parse for each one.
  if (obj provides pname)
  {
    i = 0;
    if (pn_addr-->0 == PHRASE_OP)
      i++;

    p_start = i;
    in_phrase = true;
    p_match_total = 0;
    for (: i < pn_len: i++)
    { 
      if (~~in_phrase)
      { 
        ! Find the beginning of a phrase.
        if (pn_addr-->i == PHRASE_OP)
        {
          if (pn_addr-->i == PHRASE_OP) 
            in_phrase = true;
          else
            print "ERROR in pname.h phrase parser^";
          p_start = i+1;
        }
      }
      else ! in_phrase
      {
        ! Find the end of a phrase
        if (i+1 == pn_len || pn_addr-->(i+1) == PHRASE_OP)
        {
          p_end = i;
          #ifdef DEBUG;
            if (parser_trace>=7)
            {
              print "     matching ";
              _pn_printPhrase(obj, p_start, p_end);
            }
          #endif;
          n = _pn_matchPhrase(obj, p_start, p_end);
          ! Remember only the longest phrase match
          if (n > p_match_total)
            p_match_total = n;
          in_phrase = false;
        }
      }
    }
  }
  #ifdef DEBUG;
    if (p_match_total && parser_trace>=5) print "    longest phrase match was ",
      p_match_total, "^";
  #endif;
  ! If the longest phrase match was greater than or equal to the total
  ! number of words matched, then a phrase match has occurred.
  if (p_match_total >= match_total) {
    give obj phrase_matched;
  }

  #ifdef DEBUG;
    if (parser_trace>=5) print "    pname_parser returned ",match_total,"^";
  #endif;
  
  return match_total;
];

#ifdef DEBUG;
! Prints consecutive words in a pname property, in the range 
! [p_start, p_end].
[ _pn_printPhrase obj p_start p_end;
  print "phrase: ";
  for (: p_start <= p_end: p_start++)
  {
    print "~",(address)obj.&pname-->p_start,"~ ";
  }
  new_line;
];
#endif;
   
#ifdef DEBUG;
! Prints the num word from the input stream, even if it was not in the
! dictionary.
[ _pn_printInpWord num
  i;
  if (_pn_inpWord(num) == 0)
  {
    ! Print non-dictionary word
    for (i = 0: i < WordLength(num): i++) 
      print (char) WordAddress(num)->i;
  }
  else
    print (address)_pn_inpWord(num);
];
#endif;

! Tries to match input (starting from wn) to the phrase from 
! range [ps, pe] of obj.pname.
!
! Return value: If the phrase did not match input, 0.
!               If the phrase did match input, it returns the number of
!                 words it matched.
[ _pn_matchPhrase obj ps pe
  i j k l m pa phrase_match match inp_len op_match;
  inp_len = _pn_inpLen(parse);
  ! Try to match a phrase starting from wn in the input stream.
  pa = obj.&pname;
  phrase_match = true;
  i = wn;
  for (j = ps: j <= pe: j++)
  {
      if ((pe - j) >= 2 && pa-->(j+1) == OR_OP) ! OR_OP processing
      {
        #ifdef DEBUG;
          if (parser_trace>=7) print "      OR_OP found in phrase^";
        #endif;
        ! Loop over the words in the OR_OP clause until one or none are
        ! found, incrementing j to the end of all OR_OPs.
        match = false;
        for (k = j: pa-->(j+1) == OR_OP && k <= pe: k = k+2)
        {
          if (i <= inp_len && pa-->k == _pn_inpWord(i))
          {
            match = true;
            break;
          }
        }
        if (match == true) 
        {
          #ifdef DEBUG;
            if (parser_trace>=7) print "      ~",(_pn_printInpWord)i,"~ in 
              input matches ~",(address)pa-->k,"~ in or clause^";
          #endif;
          i++;
          ! Increment j until it points off the end of the OR_OPs.
          for (: pa-->(j+1) == OR_OP && j <= pe: j = j+2)
            ;
        }
        ! See if the last optional word matched was in the OR clause.
        else if (op_match)
        {
          for (k = j: pa-->(j+1) == OR_OP && k <= pe: k=k+2)
          {
            if (op_match == pa-->k)
            {
              match = true;
              break;
            }
          }
          op_match = 0;
          if (match == true)
          {
            #ifdef DEBUG;
              if (parser_trace>=7) print "      previously matched optional word,
                ~",(address)pa-->k,"~, matched to or clause^";
            #endif;
            ! Increment j until it points off the end of the OR_OPs.
            for (: pa-->(j+1) == OR_OP && j <= pe: j = j+2)
              ;
          }
        }
        else
        {
          #ifdef DEBUG;
            if (parser_trace>=7) {
              if (i <= inp_len) print "      ~",(_pn_printInpWord)i,"~ in 
                input fails to match or clause^";
              else print "      input has run out and OR_OP unmatched^";
            }
          #endif;
          phrase_match = false;
          break;
        }
      } ! end OR_OP processing
      else if (j+1 <= pe && pa-->j == OPT_OP) ! OPT_OP processing
      {
        #ifdef DEBUG;
          if (parser_trace>=7) print "      found OPT_OP in phrase^";
        #endif;
        ! Loop over the words in the input, checking for each optional
        ! word. In this way, any list of optional words can appear in any
        ! order, and multiple times.
        match = 0;
        for (k = i: k <= inp_len: k++)
        {
          m = match;
          for (l = j: l < pe && pa-->l == OPT_OP: l = l+2)
          {
            if (_pn_inpWord(k) == pa-->(l+1))
            {
              ++match;
              op_match = _pn_inpWord(k);
              #ifdef DEBUG;
                if (parser_trace>=7) print "      optional word ~",
                  (_pn_printInpWord)k, "~ matched^";
              #endif;
            }
          }
          ! If word k didn't match any of the optional words, we're
          ! finished.
          if ((match - m) < 1)
          {
            #ifdef DEBUG;
              if (parser_trace>=7) print "      input word ~",
                (_pn_printInpWord)k,"~ does not match any of the optional
                words^";
            #endif;
            break;
          }
        }
        if (match == 0)
        {
          #ifdef DEBUG;
            if (parser_trace>=7) print "      failed to match any optional 
              words^";
          #endif;
        }
        i = i+match;
        ! Increment j to the end of the optional words
        for (: j < pe: j = j+2)
        {
          if (pa-->j ~= OPT_OP)
            break;
        }
        j = j-1;
      } ! end OPT_OP processing
      else ! Consecutive word processing
      {
        if (i <= inp_len && _pn_inpWord(i) == pa-->j)
        {
          #ifdef DEBUG;
            if (parser_trace>=7) print "      ~",(address)pa-->j,"~ in 
              phrase matches ~",(_pn_printInpWord)i,"~ in input^";
          #endif;
          i++;
        }
        else if (op_match && op_match == pa-->j)
        {
          #ifdef DEBUG;
            if (parser_trace>=7) print "      previously matched optional
              word, ~",(address)op_match,"~, matches ~",(_pn_printInpWord)i,
              "~ in phrase^";
          #endif;
          op_match = 0;
        }
        else
        {
          #ifdef DEBUG;
            if (parser_trace>=7) {
              if (i <= inp_len) print "      ~",(address)pa-->j,"~ 
                fails to match ~",(_pn_printInpWord)i,"~ in input^";
              else print "      ~",(address)pa-->j,"~
                unmatched and input has run out^";
            }
          #endif;
          phrase_match = false;
          break;
        }
      } ! end Consecutive word processing
  }
  if (phrase_match == true) 
  {
    #ifdef DEBUG;
      if (parser_trace>=5) print "      phrase matches input^";
    #endif;
    ! i must be left pointing off the end of everything we matched
    ! for the following line to work correctly.
    return i-wn;
  }
  else
  {
    #ifdef DEBUG;
      if (parser_trace>=7) print "      phrase fails to match input^";
    #endif;
    return 0;
  }
];

! The following is the standard MakeMatch routine from parserm.h, by Graham
! Nelson. It has been changed so that objects are only added to the match
! list if their _pn_score is equal to the highest score yet encountered. If a
! greater _pn_score is found, the match list is restarted.
! ----------------------------------------------------------------------------
!  MakeMatch looks at how good a match is.  If it's the best so far, then
!  wipe out all the previous matches and start a new list with this one.
!  If it's only as good as the best so far, add it to the list.
!  If it's worse, ignore it altogether.
!
!  The idea is that "red panic button" is better than "red button" or "panic".
!
!  number_matched (the number of words matched) is set to the current level
!  of quality.
!
!  We never match anything twice, and keep at most 64 equally good items.
! ----------------------------------------------------------------------------

[ MakeMatch obj quality 
   i p_match;
   if (quality > 0 && obj has phrase_matched)
     p_match = true;
#ifdef DEBUG;
   if (parser_trace>=6) {
     print "    Match with quality ", quality, " and ";
     if (p_match)
       print "a";
     else
       print "no";
     print " phrase match^";
     if (matches_in_match_list)
       print "    phrase-matched items in current match_list^";  
   }
#endif;
   if (token_filter~=0 && UserFilter(obj)==0)
   {   #ifdef DEBUG;
       if (parser_trace>=6) print "    Match filtered out: token filter ", 
         token_filter, "^";
       #endif;
       rtrue;
   }
   if ((quality < match_length) ||
       (quality == match_length && p_match == false && matches_in_match_list))
   {
      #ifdef DEBUG;
      if (parser_trace>=6) print "    Match discarded^";
      #endif;
      rtrue;
   }
   if (quality > match_length || 
      ((matches_in_match_list == false) && 
       (quality >= match_length) && 
       p_match)) 
   { 
#ifdef DEBUG;
      if (parser_trace>=6) print "    Creating new match_list^";
#endif;
      if (p_match) 
        matches_in_match_list = true;
      match_length=quality; 
      number_matched=0;
   }
   else
   {   if (WORDSIZE*number_matched>=WORDSIZE*64) rtrue;
       for (i=0:i<number_matched:i++)
           if (match_list-->i==obj) rtrue;
   }
   match_list-->number_matched++ = obj;
#ifdef DEBUG;
   if (parser_trace>=6) print "    Match added to list^";
#endif;
];
! ----------------------------------------------------------------------------
!  NounDomain does the most substantial part of parsing an object name.
!
!  It is given two "domains" - usually a location and then the actor who is
!  looking - and a context (i.e. token type), and returns:
!
!   0    if no match at all could be made,
!   1    if a multiple object was made,
!   k    if object k was the one decided upon,
!   REPARSE_CODE if it asked a question of the player and consequently rewrote
!        the player's input, so that the whole parser should start again
!        on the rewritten input.
!
!   In the case when it returns 1<k<REPARSE_CODE, it also sets the variable
!   length_of_noun to the number of words in the input text matched to the
!   noun.
!   In the case k=1, the multiple objects are added to multiple_object by
!   hand (not by MultiAdd, because we want to allow duplicates).
! ----------------------------------------------------------------------------

[ NounDomain domain1 domain2 context    first_word i j k l
                                        answer_words marker;

#ifdef DEBUG;
  if (parser_trace>=4)
  {   print "   [NounDomain called at word ", wn, "^";
      print "   ";
      if (indef_mode)
      {   print "seeking indefinite object: ";
          if (indef_type & OTHER_BIT)  print "other ";
          if (indef_type & MY_BIT)     print "my ";
          if (indef_type & THAT_BIT)   print "that ";
          if (indef_type & PLURAL_BIT) print "plural ";
          if (indef_type & LIT_BIT)    print "lit ";
          if (indef_type & UNLIT_BIT)  print "unlit ";
          if (indef_owner ~= 0) print "owner:", (name) indef_owner;
          new_line;
          print "   number wanted: ";
          if (indef_wanted == 100) print "all"; else print indef_wanted;
          new_line;
          print "   most likely GNAs of names: ", indef_cases, "^";
      }
      else print "seeking definite object^";
  }
#endif;

  match_length=0; number_matched=0; match_from=wn; placed_in_flag=0;

  ! ***
  matches_in_match_list = false;
  ! ***

  SearchScope(domain1, domain2, context);

#ifdef DEBUG;
  if (parser_trace>=4) print "   [ND made ", number_matched, " matches]^";
#endif;

  wn=match_from+match_length;

!  If nothing worked at all, leave with the word marker skipped past the
!  first unmatched word...

  if (number_matched==0) { wn++; rfalse; }

!  Suppose that there really were some words being parsed (i.e., we did
!  not just infer).  If so, and if there was only one match, it must be
!  right and we return it...

  if (match_from <= num_words)
  {   if (number_matched==1) { i=match_list-->0; return i; }

!  ...now suppose that there was more typing to come, i.e. suppose that
!  the user entered something beyond this noun.  If nothing ought to follow,
!  then there must be a mistake, (unless what does follow is just a full
!  stop, and or comma)

      if (wn<=num_words)
      {   i=NextWord(); wn--;
          if (i ~=  AND1__WD or AND2__WD or AND3__WD or comma_word
                 or THEN1__WD or THEN2__WD or THEN3__WD
                 or BUT1__WD or BUT2__WD or BUT3__WD)
          {   if (lookahead==ENDIT_TOKEN) rfalse;
          }
      }
  }

!  Now look for a good choice, if there's more than one choice...

  number_of_classes=0;
  
  if (number_matched==1) i=match_list-->0;
  if (number_matched>1)
  {   i=Adjudicate(context);
      if (i==-1) rfalse;
      if (i==1) rtrue;       !  Adjudicate has made a multiple
                             !  object, and we pass it on
  }

!  If i is non-zero here, one of two things is happening: either
!  (a) an inference has been successfully made that object i is
!      the intended one from the user's specification, or
!  (b) the user finished typing some time ago, but we've decided
!      on i because it's the only possible choice.
!  In either case we have to keep the pattern up to date,
!  note that an inference has been made and return.
!  (Except, we don't note which of a pile of identical objects.)

  if (i~=0)
  {   if (dont_infer) return i;
      if (inferfrom==0) inferfrom=pcount;
      pattern-->pcount = i;
      return i;
  }

!  If we get here, there was no obvious choice of object to make.  If in
!  fact we've already gone past the end of the player's typing (which
!  means the match list must contain every object in scope, regardless
!  of its name), then it's foolish to give an enormous list to choose
!  from - instead we go and ask a more suitable question...

  if (match_from > num_words) jump Incomplete;

!  Now we print up the question, using the equivalence classes as worked
!  out by Adjudicate() so as not to repeat ourselves on plural objects...

  if (context==CREATURE_TOKEN)
      L__M(##Miscellany, 45); else L__M(##Miscellany, 46);

  j=number_of_classes; marker=0;
  for (i=1:i<=number_of_classes:i++)
  {   
      while (((match_classes-->marker) ~= i)
             && ((match_classes-->marker) ~= -i)) marker++;
      k=match_list-->marker;

      if (match_classes-->marker > 0) print (the) k; else print (a) k;

      if (i<j-1)  print ", ";
      if (i==j-1) print (string) OR__TX;
  }
  print "?^";

!  ...and get an answer:

  .WhichOne;
#ifdef TARGET_ZCODE;
  for (i=2:i<INPUT_BUFFER_LEN:i++) buffer2->i=' ';
#endif; ! TARGET_ZCODE
  answer_words=Keyboard(buffer2, parse2);

!  Conveniently, parse2-->1 is the first word in both ZCODE and GLULX.
  first_word=(parse2-->1);

!  Take care of "all", because that does something too clever here to do
!  later on:

  if (first_word == ALL1__WD or ALL2__WD or ALL3__WD or ALL4__WD or ALL5__WD)
  {   
      if (context == MULTI_TOKEN or MULTIHELD_TOKEN or MULTIEXCEPT_TOKEN
                     or MULTIINSIDE_TOKEN)
      {   l=multiple_object-->0;
          for (i=0:i<number_matched && l+i<63:i++)
          {   k=match_list-->i;
              multiple_object-->(i+1+l) = k;
          }
          multiple_object-->0 = i+l;
          rtrue;
      }
      L__M(##Miscellany, 47);
      jump WhichOne;
  }

!  If the first word of the reply can be interpreted as a verb, then
!  assume that the player has ignored the question and given a new
!  command altogether.
!  (This is one time when it's convenient that the directions are
!  not themselves verbs - thus, "north" as a reply to "Which, the north
!  or south door" is not treated as a fresh command but as an answer.)

  #ifdef LanguageIsVerb;
  if (first_word==0)
  {   j = wn; first_word=LanguageIsVerb(buffer2, parse2, 1); wn = j;
  }
  #endif;
  if (first_word ~= 0)
  {   j=first_word->#dict_par1;
      if ((0~=j&1) && (first_word ~= 'long' or 'short' or 'normal'
                                     or 'brief' or 'full' or 'verbose'))
      {   CopyBuffer(buffer, buffer2);
          return REPARSE_CODE;
      }
  }

!  Now we insert the answer into the original typed command, as
!  words additionally describing the same object
!  (eg, > take red button
!       Which one, ...
!       > music
!  becomes "take music red button".  The parser will thus have three
!  words to work from next time, not two.)

#ifdef TARGET_ZCODE;

  k = WordAddress(match_from) - buffer; l=buffer2->1+1; 
  for (j=buffer + buffer->0 - 1: j>= buffer+k+l: j--)
      j->0 = 0->(j-l);
  for (i=0:i<l:i++) buffer->(k+i) = buffer2->(2+i);
  buffer->(k+l-1) = ' ';
  buffer->1 = buffer->1 + l;
  if (buffer->1 >= (buffer->0 - 1)) buffer->1 = buffer->0;

#ifnot; ! TARGET_GLULX

  k = WordAddress(match_from) - buffer;
  l = (buffer2-->0) + 1;
  for (j=buffer+INPUT_BUFFER_LEN-1 : j >= buffer+k+l : j--)
      j->0 = j->(-l);
  for (i=0:i<l:i++) 
      buffer->(k+i) = buffer2->(WORDSIZE+i);
  buffer->(k+l-1) = ' ';
  buffer-->0 = buffer-->0 + l;
  if (buffer-->0 > (INPUT_BUFFER_LEN-WORDSIZE)) 
      buffer-->0 = (INPUT_BUFFER_LEN-WORDSIZE);

#endif; ! TARGET_

!  Having reconstructed the input, we warn the parser accordingly
!  and get out.

  return REPARSE_CODE;

!  Now we come to the question asked when the input has run out
!  and can't easily be guessed (eg, the player typed "take" and there
!  were plenty of things which might have been meant).

  .Incomplete;

  if (context==CREATURE_TOKEN)
      L__M(##Miscellany, 48); else L__M(##Miscellany, 49);

#ifdef TARGET_ZCODE;
  for (i=2:i<INPUT_BUFFER_LEN:i++) buffer2->i=' ';
#endif; ! TARGET_ZCODE
  answer_words=Keyboard(buffer2, parse2);

  first_word=(parse2-->1);
  #ifdef LanguageIsVerb;
  if (first_word==0)
  {   j = wn; first_word=LanguageIsVerb(buffer2, parse2, 1); wn = j;
  }
  #endif;

!  Once again, if the reply looks like a command, give it to the
!  parser to get on with and forget about the question...

  if (first_word ~= 0)
  {   j=first_word->#dict_par1;
      if (0~=j&1)
      {   CopyBuffer(buffer, buffer2);
          return REPARSE_CODE;
      }
  }

!  ...but if we have a genuine answer, then:
!
!  (1) we must glue in text suitable for anything that's been inferred.

  if (inferfrom ~= 0)
  {   for (j = inferfrom: j<pcount: j++)
      {   if (pattern-->j == PATTERN_NULL) continue;
#ifdef TARGET_ZCODE;
          i=2+buffer->1; (buffer->1)++; buffer->(i++) = ' ';
#ifnot; ! TARGET_GLULX
          i = WORDSIZE + buffer-->0;
          (buffer-->0)++; buffer->(i++) = ' ';
#endif; ! TARGET_
    
          if (parser_trace >= 5)
          print "[Gluing in inference with pattern code ", pattern-->j, "]^";

! Conveniently, parse2-->1 is the first word in both ZCODE and GLULX.

          parse2-->1 = 0;

          ! An inferred object.  Best we can do is glue in a pronoun.
          ! (This is imperfect, but it's very seldom needed anyway.)
    
          if (pattern-->j >= 2 && pattern-->j < REPARSE_CODE)
          {   PronounNotice(pattern-->j);
              for (k=1: k<=LanguagePronouns-->0: k=k+3)
                  if (pattern-->j == LanguagePronouns-->(k+2))
                  {   parse2-->1 = LanguagePronouns-->k;
                      if (parser_trace >= 5)
                      print "[Using pronoun '", (address) parse2-->1, "']^";
                      break;
                  }
          }
          else
          {   ! An inferred preposition.
              parse2-->1 = No__Dword(pattern-->j - REPARSE_CODE);
              if (parser_trace >= 5)
                  print "[Using preposition '", (address) parse2-->1, "']^";
          }
    
          ! parse2-->1 now holds the dictionary address of the word to glue in.

          if (parse2-->1 ~= 0)
          {   k = buffer + i;
#ifdef TARGET_ZCODE;
              @output_stream 3 k;
              print (address) parse2-->1;
              @output_stream -3;
              k = k-->0;
              for (l=i:l<i+k:l++) buffer->l = buffer->(l+2);
              i = i + k; buffer->1 = i-2;
#ifnot; ! TARGET_GLULX
              k = PrintAnyToArray(buffer+i, INPUT_BUFFER_LEN-i, parse2-->1);
              i = i + k; buffer-->0 = i - WORDSIZE;
#endif; ! TARGET_
          }
      }
  }

!  (2) we must glue the newly-typed text onto the end.

#ifdef TARGET_ZCODE;
  i=2+buffer->1; (buffer->1)++; buffer->(i++) = ' ';
  for (j=0: j<buffer2->1: i++, j++)
  {   buffer->i = buffer2->(j+2);
      (buffer->1)++;
      if (buffer->1 == INPUT_BUFFER_LEN) break;
  }    
#ifnot; ! TARGET_GLULX
  i = WORDSIZE + buffer-->0;
  (buffer-->0)++; buffer->(i++) = ' ';
  for (j=0: j<buffer2-->0: i++, j++)
  {   buffer->i = buffer2->(j+WORDSIZE);
      (buffer-->0)++;
      if (buffer-->0 == INPUT_BUFFER_LEN) break;
  }    
#endif; ! TARGET_

#ifdef TARGET_ZCODE;

!  (3) we fill up the buffer with spaces, which is unnecessary, but may
!      help incorrectly-written interpreters to cope.

  for (:i<INPUT_BUFFER_LEN:i++) buffer->i = ' ';

#endif; ! TARGET_ZCODE
 
  return REPARSE_CODE;
];


!
!  TryGivenObject tries to match as many words as possible in what has been
!  typed to the given object, obj.  If it manages any words matched at all,
!  it calls MakeMatch to say so, then returns the number of words (or 1
!  if it was a match because of inadequate input).
!
[ TryGivenObject obj threshold k w j;

   ! ***
   if (obj provides pname) return _pn_TryGivenObject(obj, threshold);
   ! ***

#ifdef DEBUG;
   if (parser_trace>=5)
       print "    Trying ", (the) obj, " (", obj, ") at word ", wn, "^";
#endif;

   dict_flags_of_noun = 0;

!  If input has run out then always match, with only quality 0 (this saves
!  time).

   if (wn > num_words)
   {   if (indef_mode ~= 0)
           dict_flags_of_noun = $$01110000;  ! Reject "plural" bit
       MakeMatch(obj,0);
       #ifdef DEBUG;
       if (parser_trace>=5)
       print "    Matched (0)^";
       #endif;
       return 1;
   }

!  Ask the object to parse itself if necessary, sitting up and taking notice
!  if it says the plural was used:

   ! ***
   give obj ~phrase_matched;
   ! ***

   if (obj.parse_name~=0)
   {   parser_action = NULL; j=wn;
       k=RunRoutines(obj,parse_name);
       if (k>0)
       {   wn=j+k;
           .MMbyPN;

           if (parser_action == ##PluralFound)
               dict_flags_of_noun = dict_flags_of_noun | 4;

           if (dict_flags_of_noun & 4)
           {   if (~~allow_plurals) k=0;
               else
               {   if (indef_mode==0)
                   {   indef_mode=1; indef_type=0; indef_wanted=0; }
                   indef_type = indef_type | PLURAL_BIT;
                   if (indef_wanted==0) indef_wanted=100;
               }
           }

           #ifdef DEBUG;
               if (parser_trace>=5)
               {   print "    Matched (", k, ")^";
               }
           #endif;
           ! ***
           give obj phrase_matched;
           ! ***
           MakeMatch(obj,k);
           return k;
       }
       if (k==0) jump NoWordsMatch;
   }

!  The default algorithm is simply to count up how many words pass the
!  Refers test:

   parser_action = NULL;

   w = NounWord();

   if (w==1 && player==obj) { k=1; jump MMbyPN; }

   if (w>=2 && w<128 && (LanguagePronouns-->w == obj))
   {   k=1; jump MMbyPN; }

   j=--wn;
   threshold = ParseNoun(obj);
#ifdef DEBUG;
   if (threshold>=0 && parser_trace>=5)
       print "    ParseNoun returned ", threshold, "^";
#endif;
   if (threshold<0) wn++;
   if (threshold>0) { k=threshold; jump MMbyPN; }

   if (threshold==0 || Refers(obj,wn-1)==0)
   {   .NoWordsMatch;
       if (indef_mode~=0)
       {   k=0; parser_action=NULL; jump MMbyPN;
       }
       rfalse;
   }

   if (threshold<0)
   {   threshold=1;
       dict_flags_of_noun = (w->#dict_par1) & $$01110100;
       w = NextWord();
       while (Refers(obj, wn-1))
       {   threshold++;
           if (w)
               dict_flags_of_noun = dict_flags_of_noun
                                    | ((w->#dict_par1) & $$01110100);
           w = NextWord();
       }
   }

   k = threshold; jump MMbyPN;
];

[ _pn_TryGivenObject obj k j;

#ifdef DEBUG;
   if (parser_trace>=5)
       print "    Trying ", (the) obj, " (", obj, ") at word ", wn, "^";
#endif;

   dict_flags_of_noun = 0;

!  If input has run out then always match, with only quality 0 (this saves
!  time).

   if (wn > num_words)
   {   if (indef_mode ~= 0)
           dict_flags_of_noun = $$01110000;  ! Reject "plural" bit
       MakeMatch(obj,0);
       #ifdef DEBUG;
       if (parser_trace>=5)
       print "    Matched (0)^";
       #endif;
       return 1;
   }

   parser_action = NULL; j=wn;
   k=_pn_parse(obj);
   if (k>0) {   
       wn=j+k;
       .MatchMade;

       if (parser_action == ##PluralFound)
           dict_flags_of_noun = dict_flags_of_noun | 4;

       if (dict_flags_of_noun & 4) {   
           if (~~allow_plurals) 
             k=0;
           else {   
               if (indef_mode==0) {   
                   indef_mode=1; indef_type=0; indef_wanted=0; 
               }
               indef_type = indef_type | PLURAL_BIT;
               if (indef_wanted==0) 
                   indef_wanted=100;
           }
       }

       #ifdef DEBUG;
           if (parser_trace>=5) {   
               print "    Matched (", k, ")^";
           }
       #endif;
       MakeMatch(obj,k);
       return k;
   }
   else if (indef_mode~=0) {
     k=0; parser_action=NULL; jump MatchMade;
   }
   rfalse;
];


! ----------------------------------------------------------------------------
!  Identical decides whether or not two objects can be distinguished from
!  each other by anything the player can type.  If not, it returns true.
! ----------------------------------------------------------------------------

[ Identical o1 o2 p1 p2 n1 n2 i j flag;

  if (o1==o2) rtrue;  ! This should never happen, but to be on the safe side
  if (o1==0 || o2==0) rfalse;  ! Similarly
  if (parent(o1)==compass || parent(o2)==compass) rfalse; ! Saves time

  ! What complicates things is that o1 or o2 might have a parsing routine,
  ! so the parser can't know from here whether they are or aren't the same.
  ! If they have different parsing routines, we simply assume they're
  ! different.  If they have the same routine (which they probably got from
  ! a class definition) then the decision process is as follows:
  !
  ! The routine is called (with self being o1, not that it matters) with
  ! noun and second being set to o1 and o2, and action being set to the
  ! fake action TheSame.  If it returns -1, they are found identical; if
  ! -2, different; and if >=0, then the usual method is used instead.

  if (o1.parse_name~=0 || o2.parse_name~=0)
  {   if (o1.parse_name ~= o2.parse_name) rfalse;
      parser_action=##TheSame; parser_one=o1; parser_two=o2;
      j=wn; i=RunRoutines(o1,parse_name); wn=j;
      if (i==-1) rtrue; if (i==-2) rfalse;
  }

  !
  ! Compare name and pname properties for the two objects.
  !
  ! The objects are identical if the same words appear in their name
  ! properties and their pname properties are identical.
  !
  if ((o1 provides name) ~= (o2 provides name))
    rfalse;

  if ((o1 provides name) && (o2 provides name))
  {
    n1=(o1.#name)/WORDSIZE; n2=(o2.#name)/WORDSIZE;
    p1=o1.&name; p2=o2.&name;

    ! Set 'flag' to 0 if at any time a word from o1 is not found
    ! in o2.
    for (i=0: i < n1: i++) 
    { flag=0;
      for (j=0: j < n2: j++) if (p1-->i == p2-->j) flag=1;
      if (flag==0) rfalse;
    }

    ! repeat from the perspective of o2.
    for (j=0: j < n2: j++) 
    { flag=0;
      for (i=0: i < n1: i++) if (p1-->i == p2-->j) flag=1;
      if (flag == 0) rfalse;
    }
  }

  ! If one provides pname and the other doesn't they are obviously
  ! distinguishable.
  if ((o1 provides pname) ~= (o2 provides pname)) 
    rfalse;

  if (o1 provides pname && o2 provides pname) 
  { 
    n1=(o1.#pname)/WORDSIZE; n2=(o2.#pname)/WORDSIZE;

    ! If there are a different number of words in the pname
    ! properties, they are distinguishable.
    if (n1 ~= n2) 
      rfalse;

    p1=o1.&pname; p2=o2.&pname; 

    ! Compare pname properties, but this time all the words must be in
    ! the same order in both objects.
    for (i=0: i < n1: i++)
      if (p1-->i ~= p2-->i) rfalse;
  }

  rtrue;
];

#ifdef DEBUG;
[ pname_verify suppress
  obj pa plen i errflag;
  errflag = false;
  objectloop(obj)
  {
    if (~~ obj provides pname)
    {
      continue;
    }
    if (obj provides name && ~~suppress) {
      _pn_warning(obj);
      print " name and pname both delcared^";
    }
    if (obj provides parse_name && ~~suppress) {
      _pn_warning(obj);
      print " parse_name and pname both declared"; 
    }
    pa = obj.&pname; plen = obj.#pname/WORDSIZE;
    ! check for possible bogus operators and produce warnings
    ! unless warnings are suppressed
    for (i = 0: i < plen && ~~suppress: i++)
    {
      if (pa-->i == 'x')
      {
        _pn_warning(obj, i);
        print " 'x' in pname. You probably meant '.x'^";
      }
      else if (pa-->i == 'or')
      {
        _pn_warning(obj, i);
        print " 'or' in pname. You probably meant '.or'^";
      }
      else if (pa-->i == 'p')
      {
        print " 'p' in pname. You probably meant '.p'^";
      }
      else ;
    }
    for (i = 0: i < plen: i++)
    {
      if (pa-->i == PHRASE_OP && 
          (pa-->(i+1) == PHRASE_OP || (i+1) == plen))
      {
        if (i+1 ~= plen)
          _pn_error(obj, i+1); 
        else
          _pn_error(obj, i);
        print " illegal empty phrase^";
        errflag = true;
      }
      if (pa-->i == OR_OP && (i == 0 || _pn_isOp(pa-->(i+1)) || _pn_isOp(pa-->(i-1))
           || i+1 == plen))
      {
        _pn_error(obj, i);
        print " illegal OR_OP]^";
        errflag = true;
      }
      if (pa-->i == OPT_OP && (_pn_isOp(pa-->(i+1)) || i+1 == plen))
      {
        _pn_error(obj, i);
        print " illegal OPT_OP]^";
        errflag = true;
      }
    }
  }
  if (errflag == true)
    quit;
];

[ _pn_isOp i;
  if (i == PHRASE_OP or OR_OP or OPT_OP)
    return true;
  else
    return false;
];

[ _pn_error obj i;
  print (name) obj, ": pname error at word ", i+1, ": ";
];

[ _pn_warning obj i;
  print (name) obj, ": pname warning at word ", i+1, ": ";
];
#endif; ! #ifdef DEBUG
