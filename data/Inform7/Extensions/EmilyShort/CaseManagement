Case Management by Emily Short begins here.

"'To say' rules to say the names of things and rooms, and passages of text, in uniform upper or lower case."

Use authorial modesty.

To say (item - a thing) in lower case:
	(- Decapitalize({item}); -)

To say (item - a room) in lower case:
	(- Decapitalize({item}); -)

To say (T - some text) in lower case:
	(- StringDecap({T}); -)


To say (item - a thing) in caps:
	(- Capitalize({item}); -)

To say (item - a room) in caps:
	(- Capitalize({item}); -)

To say (T - some text) in caps:
	(- StringCap({T}); -)
	
Include (-

#ifndef printed_text;
Array printed_text buffer 256;
#endif;

[ Decapitalize item depth i j ch; 
!	@output_stream 3 printed_text;
!	print (the) item;
!	@output_stream -3;
	j = VM_PrintToBuffer(printed_text, 256, DefArt, item);
!	j = printed_text-->0;
	for (i=WORDSIZE:i<(j+WORDSIZE):i++)
	{
		ch = VM_UpperToLowerCase(printed_text->i);
!		if (ch < 97 && ch > 64) ch = ch + 32;
		print (char) ch;
	}
];

[ StringDecap item depth i j ch; 
!	@output_stream 3 printed_text;
!	print (string) item;
!	@output_stream -3;
	j = VM_PrintToBuffer(printed_text, 256, item);
!	j = printed_text-->0;
	for (i=WORDSIZE:i<(j+WORDSIZE):i++)
	{
		ch = VM_UpperToLowerCase(printed_text->i);
!		if (ch < 97 && ch > 64) ch = ch + 32;
		print (char) ch;
	}
];


[ Capitalize item depth i j ch; 
!	@output_stream 3 printed_text;
!	print (the) item;
!	@output_stream -3;
!	j = printed_text-->0;
	j = VM_PrintToBuffer(printed_text, 256, DefArt, item);
	for (i=WORDSIZE:i<(j+WORDSIZE):i++)
	{
		ch = VM_LowerToUpperCase(printed_text->i);
!		if (ch > 96) ch = ch - 32;
		print (char) ch;
	}
];

[ StringCap item depth i j ch; 
!	@output_stream 3 printed_text;
!	print (string) item;
!	@output_stream -3;
!	j = printed_text-->0;
	j = VM_PrintToBuffer(printed_text, 256, item);
	for (i=WORDSIZE:i<(j+WORDSIZE):i++)
	{
		ch = VM_LowerToUpperCase(printed_text->i);
!		if (ch > 96) ch = ch - 32;
		print (char) ch;
	}
];
 
-)

Case Management ends here.

---- DOCUMENTATION ----

Case Management provides a couple of to say... phrases. 

	say "[item in caps]"

converts the text of the item's name to capital letters; similarly

	say "[item in lower case]" 

converts the whole text to lower case. This works for text, names of items, and names of rooms. It does assume that the name or text printed is not longer than 256 characters.
