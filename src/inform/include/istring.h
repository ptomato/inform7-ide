!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!    IString                    The Inform String Library: A reconstruction
!                               of the ansi C string functions,
!   Version 2.1                 By L. Ross Raszewski
!                               rraszews@acm.org
!
! New in this version: Drop-in support for ZNSI.h
! New in version 2.0: support for routines printing strings.  Most places
!                      where the documentation says "literal string",
!                      you can add "or a routine to print one"
!                      Emptystring and Justify functions.
!
!
!       This documentation is intended for users unfamiliar with the
!       ANSI C string.h library.  The ZMachine does not do dynamic storage,
!       so this does not return pointers the way that C does, nor does it
!       reallocate space.
!       I have also added several additional functions not in C, which
!       emulate some of the functionality of printf();
!       No, I'm not entirely sure why anyone would need some of these,
!       but I figured that as long as I was doing some of them, I might as
!       well do the rest.  And, after all, I hate it when people say
!       Why would you ever need to *** in a text game?
!
!  First:  There are two types of strings in inform: literal strings
!          and string arrays.  This library is for dealing with String
!          arrays.  Literal strings cannot be transformed by the Z-machine
!          However, utilities to convert a string from literal to array
!          are included.  Conversion from array to literal is impossible,
!          as literal strings must be known at compile time.
!          A string array is a byte array, whose first two bytes contain
!          the integer length of the string
!
!         A string array must have a length of 2+the maxumum number of
!         characters.
!  The Functions:
!  WriteString: Converts a literal string into a string array.
!               can also convert a routine to print a literal string.
!               note: the array MUST have enough space to hold the literal.
!          usage: WriteString(array,literal);
!          ex: WriteString(BobStr,"Bob");
!              
!
!  PrintString: Prints a string array. Returns the number of character read
!         usage: PrintString(array[,offset]);
!                print (StringArray) array;
!         ex: PrintString(BobStr);
!             PrintString(BobStr,1); prints from the second character on.
!  LPrintString: Does the same, but accepts a literal string (for ZNSI)
!
! StrCpy: Copies one string to another. --target must be large enough
!          to hold the source.  Returns the number of characters copied
!       usage: StrCpy(target_array, source_array);
!          ex: StrCpy(NewStr,BobStr) copies BobStr into NewStr
!
! StrCat: Concatenates two strings into one.  The result is stored in the
!          first argument --MUST BE LARGE ENOUGH.  Returns the new size of
!          the string.
!       usage: StrCat(string1, string2);
!          ex:  if HiStr contains "Hello " and WrldStr contains "World",
!               StrCat(HiStr, WrldStr);
!               stores "Hello World" in HiStr.  HiStr must be of length at
!               least 13.
!
! LStrCat: As StrCat, but the second argument must be a literal string:
!          (can also use a routine to print a literal string)           
!       ex: LStrCat(HiStr,"World"); yields as above
!
! StrCmp: Compares two strings. Returns 0 if they are the same, positive if
!         the first comes earlier alphabetically, negative otherwise
!
! LStrCmp: As StrCmp, but with TWO literal strings (or routines to print them).
!      ex: LStrCmp("Hi","Hi"); returns 0
!          LStrCmp("Hi","Bye"); returns a negative number
!
! StrLen: Returns the length of a string (which is also stored in str-->0)
!       usage: StrLen(string);
!
! LStrLen: (more useful) Returns the length of a literal string
!       usage :LStrLen("Hi there");
!
! StrChr: Matches the first occurance of a character in a string
!       ex: if AStr contains the string "Hi, how are you"
!           StrChr(AStr,','); returns 3
!           therefore Astr->(3+2)==',' (there is an offset of 2 in string
!                                       arrays)
! LStrChr: As StrChr but with a literal string or routine to print one
!
! StrStr: Matches the first occurance of a substring in a string
!      usage: StrStr(string,substring);
!       ex: if AStr is as above, and Bstr contains "how",
!           StrStr(AStr, Bstr) returns 5;
! LStrStr: As above, except the SUBSTRING ONLY must be a literal string
!       ex: LStrStr(Astr,"how"); is the same as the above example.
!
! --NON-C-BASED FUNCTIONS--
!
! EmptyString: Generates a string of blanks.  
!        usage: EmptyString(string,number[, character]);
!           ex: EmptyString(Bob,5); makes Bob a string of 5 space characters
!               EmptyString(Bob,5,'.'); as above, with periods instead of spaces
!
! Justify: Justifies a string in a window of given width.  Alignment defaults
!          to LEFT, but may also be RIGHT of CENTERED.  Optional character
!          parameter specifies character to pad the empty spaces with.
!       usage: Justify(String, width[, alignment, character])
!       ex:    Justify(Bob,10,RIGHT); Right-justifies Bob in a 10-space field 
! LJustify: As Justify, but with a literal string or routine to print one.
!
! Questions, Comments, Suggestions, PLEASE e-mail me
! Also, if you use this file in a game, let me know, put me in the credits,
! toot my praises.  Or just drop me a line so I can see what use you've put
! it to

system_file;
Constant ISTRING_LIBRARY 21;
Default RIGHT 0;
Default LEFT  1;
Default CENTERED 2;
Constant MAX_STR_LEN 102;       ! Maximum size of a literal string.  Adjust
                                ! As needed.  This is very large, and allows
                                ! for a 100 character literal string
                                ! This is also the maximum width of a justify
                                ! window.
Array StringBuffer1->MAX_STR_LEN; ! Temporary string buffers
Array StringBuffer2->MAX_STR_LEN;
[ ReadString str i j;
  @read_char 1 0 0 j;
   while(j~=13)
   {
    str->(i+2)=j;
    i++;
    @read_char 1 0 0 j;
   }
  str-->0=i;
  return i;
];

[ WriteString str litstr;
   if (litstr ofclass string)
    litstr.print_to_array(str);
   else
   {
    @output_stream 3 str;
    litstr();
    @output_stream -3;
   }
   return str-->0;
];
Ifndef Printstring;
[ PrintString str i;
   for(:i<(str-->0):i++)
    print (char) str->(i+2);
   return i-2;
];
Endif;
[ StringArray str; PrintString(str);];
[ StrCpy target source j i;
   target-->0=source-->0+j;
   for (i=2:i<(target-->0)+2-j:i++)
    target->(i+j)=source->i;
   return i-2;
];

[ StrCat first second i;
   for(i=2:i<2+second-->0:i++)
    first->(i+first-->0)=second->i;
   first-->0=first-->0+second-->0;
   return first-->0;
];
[ LStrCat target source;
   WriteString(StringBuffer1,source);
   return StrCat(target,StringBuffer1);
];
[ StrCmp first second i j k;
   j=0;
   if (first-->0<second-->0) k=first-->0;
   else k=second-->0;
   for(i=i+2:i<k+2&&j==0:i++)
    j=((second->i)-(first->i));
   return j;
];
[ LStrCmp First Second i;
   WriteString(StringBuffer1,First);
   WriteString(StringBuffer2,Second);
   return StrCmp(StringBuffer1, StringBuffer2,i);
];
Ifndef StrLen;
[ StrLen Str;
   return Str-->0;
];
Endif;
[ LStrLen str;
   WriteString(StringBuffer1,str);
   return StrLen(StringBuffer1);
];
[ StrChr str char i j k;
   j=i+2;
   k=0;
   while (k==0 && j<=str-->0+2)
   { if (str->j==char) k=1;
     j++;
   }
   if (k==0) j=2+i;
   return j-2-i;
];
[ LStrChr str chr i;
   WriteString(StringBuffer1,str);
   return StrChr(StringBuffer1,chr,i);
];
[ StrStr str substr i j k l;
   l=1;
   while (i<(str-->0) && l==1 && i~=-1)
   {
    l=0;
    j=StrChr(str, substr->2,i);
    if (j~=0)
     for(k=3:k<(substr-->0)+2 && l==0:k++)
      if (StrChr(str, substr->k,j)~=k-2) l=1;
    if (j==0) i=-1;
    else i=j+1;
   }
   return j;
];
[ LStrStr str substr i;
   WriteString(StringBuffer1,substr);
   return StrStr(str, StringBuffer1, i);
];
[ EmptyString str len chr i;
   if (chr==0) chr=' ';
   str-->0=len;
   for(i=2:i<len+2:i++)
    str->i=chr;
];
IfNdef Justify;
[ Justify str width align pad_chr i;
   EmptyString(StringBuffer2,width,pad_chr);
   i=StrLen(str);
   if (align==LEFT) i=0;
   else if (align==RIGHT) i=width-i;
   else if (align==CENTERED) i=(width-i)/2;
   StrCpy(StringBuffer2,str,i);
   StringBuffer2-->0=width;
   PrintString(StringBuffer2);
];
Endif;
[ LJustify str width align pad_chr;
   WriteString(StringBuffer1,str);
   return Justify(StringBuffer1,width,align, pad_chr);
];


[ LPrintString str i;
   WriteString(StringBuffer1,str);
   return PrintString(StringBuffer1,i);
];
