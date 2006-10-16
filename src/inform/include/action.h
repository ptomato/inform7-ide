!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!  action.h              A complete Library for the implementation for       !
!  by Adam Stark         Action Menu type control for Interactive Fiction.   !
!                        It isn't as efficient as it could be and It only    !
!                        looks good at 80 width screen mode, but it works for!
!                        me.                                                 !
!                        Usage:  If you want a command, write it as an object!
!                        of the Class MENU_ITEM.  To put it on the Menu in   !
!                        the beggining, the object needs to be put in        !
!                        MASTER_MENU.  The Title of MASTER_MENU reflects the !
!                        headline in the statusline.  Use the Route property !
!                        to give a function to an item.  The Title property  !
!                        is the menu title.  The Letters property should be  !
!                        the number of letters in the title.  The Blurb is a !
!                        quick description of the function.  The variable    !
!                        BLURBED controls whether the Blurb is printed.      !
!                        By declaring the variable REAL_TIME_GAME, the game  !
!                        can reflect a real time.                            !
!  History:                                                                  !
!	V0.8:  This version.  The first working beta version.                !
!                                                                            !
!Report bugs to: bstark@sprynet.com                                          !
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

Message "Inserting action.h";
Statusline time;
Property Select;
Property Route;
Property Start_Line;
Property KeyMenu;
Global Score;
Global Moves;
Global Rate=1;
Global PTime;
Global Cursor_Line = 1;
Global Levels_Displayed;
Global ClockTicks;
Global NewTime;
Property Title;
Property Blurb;
Property Letters;
Property Escape_Routine;
Global BLURBED=1;

Class MENU_ITEM
   with Select [;Levels_Displayed++; self.Start_Line=1; 
                 Cursor_Line=1;PMove(Placeholder, self); REDRAW_TOP();],
        Start_Line 1;

MENU_ITEM MASTER_MENU,
   with Title "            ",
        Letters 19;

Object Placeholder
    with Title " ",
         Letters 1;

[REDRAW_TOP x obj n other; @buffer_mode 0; @set_window 1; font off;
!!!!!!!!!!!!!!!!!!!!Routine for printing the far right side!!!!!!!!!!!!!!!!!!!
	style reverse;
	for(x=0:x<4:x++)
		{n=(x+1);
		obj=AgeNum(parent(Placeholder),x+((parent(Placeholder)).Start_Line)-1); @set_cursor n 61; spaces 1;
		if (obj provides Title)
			{if (Cursor_Line==n) style roman;
			print (string) obj.Title; spaces (19-obj.Letters);}
			else spaces 19; style reverse;
		};
	style roman;
	for(x=0:x<4:x++)
		{n=(x+1);
		obj=AgeNum(parent(parent(Placeholder)),x+((parent(parent(Placeholder))).Start_Line)-1); @set_cursor n 41; spaces 1;
		if (obj provides Title)
			{if (Placeholder in obj) style reverse;
			print (string) obj.Title; spaces (19-obj.Letters);}
			else spaces 19; style roman;
		};
	style reverse;
	for(x=0:x<4:x++)
		{n=(x+1);
		obj=AgeNum(parent(parent(parent(Placeholder))),x+((parent(parent(parent(Placeholder)))).Start_Line)-1); @set_cursor n 21; spaces 1;
		if (obj provides Title)
			{if (parent(Placeholder) in obj) style roman;
			print (string) obj.Title; spaces (19-obj.Letters);}
			else spaces 19; style reverse;
		};
	style roman;
	for(x=0:x<4:x++)
		{n=(x+1);
		obj=AgeNum(parent(parent(parent(parent(Placeholder)))),x+((parent(parent(parent(parent(Placeholder))))).Start_Line)-1);
		@set_cursor n 1; spaces 1;
		if (obj provides Title)
			{if (parent(parent(Placeholder)) in obj) style reverse;
			print (string) obj.Title; spaces (19-obj.Letters);}
			else spaces 19; style roman;
		};
	if (BLURBED==1)
		{new_line; @buffer_mode 1;
		style reverse; spaces 35; print "Explanation"; spaces 34;
		style roman; new_line;
		spaces 80; new_line; spaces 80;
		@set_cursor 6 1;
		other = AgeNum(parent(Placeholder), (parent(Placeholder)).Start_Line+Cursor_Line-2);;
		if (ZRegion(other.Blurb)==2) other.Blurb();
		if (ZRegion(other.Blurb)==3) print (string) other.Blurb;};

	DrawStatusLine();
	@set_window 0; font on;
	];

[AgeNum obj x nobj;
	if (obj==nothing) return nothing;
	if (children(obj)<(x-1)) return nothing ;nobj=child(obj);
	for(:x>0:x--)
		{nobj=sibling(nobj);}
	if (nobj==Placeholder) nobj=sibling(nobj);
	return nobj;];

[ ZRegion addr;
  switch(metaclass(addr))       ! Left over from Inform 5
  {   nothing: return 0;
      Object, Class: return 1;
      Routine: return 2;
      String: return 3;
  }
];

Object temp_obj;
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! Pmove - moves obj1 into obj2 as the youngest child
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
[ Pmove obj1 obj2 o;
   for (o=child(obj2):o ofclass Object: o=child(obj2)) move o to temp_obj;
   move obj1 to obj2;
   for (o=child(temp_obj):o ofclass Object: o=child(temp_obj)) move o to obj2;
];

[ Loop key; 
	REDRAW_TOP();
	for (::)
	{@read_char 1 1 RE_LOOP key;
	switch (key)
		{'n', 'N', 130: Move_to_Next();
		 'p', 'P', 129: Move_to_Prev();
		 'q', 'Q', 27:  PerfectEscape();
		 10, 13, 132: Move_to_Son();
		 default: if(parent(Placeholder) provides KeyMenu)
			(parent(Placeholder)).KeyMenu(key);};
	REDRAW_TOP();};
	];

[ Move_to_Next;
	if ((children(parent(Placeholder))-1)<=(parent(Placeholder)).Start_Line+Cursor_Line-1) return;
	if (Cursor_Line == 4) (parent(Placeholder)).Start_Line++;
	else Cursor_Line++;];

[ Move_to_Prev;
	if (Cursor_Line ~= 1) Cursor_Line--;
	else {if (((parent(Placeholder)).Start_Line)>1) (parent(Placeholder)).Start_Line--;};];

[ Move_to_Son obj;
	obj = AgeNum(parent(Placeholder), (parent(Placeholder)).Start_Line+Cursor_Line-2);
	if (obj == nothing) return;
	if (obj provides Route)
		{@buffer_mode 1; Moves++;
		if (ZRegion(obj.Route)==2)
			obj.Route();
		if (ZRegion(obj.Route)==3)
			{new_line; print (string) obj.Route;};}
		else obj.Select()
	;];

[RE_LOOP; ERate(Moves,Rate);
#IFDEF REAL_TIME_GAME;
	ClockTicks++;
	EverySec();
#ENDIF;];

[ERate m r noe;
#IFNDEF REAL_TIME_GAME;	
	if (r<0) noe = ((m/r)+NewTime);
	if (r==0) noe = NewTime;
	if (r>0) noe = ((m*r)+NewTime);
#ENDIF;
#IFDEF REAL_TIME_GAME;
	noe=(ClockTicks/565);
	m=r;
#ENDIF;
	PTime=noe;];

[SetClock t r;
	Moves = 0;
	NewTime=t;
	Rate=r;];

[ DrawStatusLine width posa posb;
   if (BLURBED==1) @set_cursor 8 1;
   else @set_cursor 5 1;
   style reverse;
   width = 0->33; posa = width-26; posb = width-13;
   spaces width;
   if (BLURBED==1) @set_cursor 8 2;
   else @set_cursor 5 2;
   print (string) MASTER_MENU.Title;
   if ((0->1)&2 == 0)
   {   if (width > 76)
       {   
	if (BLURBED==1) @set_cursor 8 posa;
 	else @set_cursor 5 posa; print "Score: ", Score;
        if (BLURBED==1) @set_cursor 8 posb;
	else @set_cursor 5 posb; print "Moves: ", Moves;
       }
       if (width > 63 && width <= 76)
       {   if (BLURBED==1) @set_cursor 8 posb;
	   else @set_cursor 5 posb; print Score, "/", Moves;
       }
   }
   else
   {   if (BLURBED==1) @set_cursor 8 posa;
   else @set_cursor 5 posa;
       print "Time: ";
	if (((PTime/60)%12)==0) print "12";
	else print ((PTime/60)%12);
	print ":";
	if ((PTime%60)<10) print "0", (PTime%60);
	else print (PTime%60);
	if (((PTime/60)%24)==((PTime/60)%12)) print " am";
	else print " pm";
   }
   @set_cursor 1 1; style roman; @set_window 0;
];


[Main; if (BLURBED == 1) @split_window 8;
	else @split_window 5;
	Initialise(); Loop();];

[ PerfectEscape;if (parent(Placeholder) provides Escape_Routine)
			return (parent(Placeholder)).Escape_Routine();
		if (parent(Placeholder)~=MASTER_MENU)
		{Levels_Displayed--;PMove(Placeholder,parent(parent(Placeholder))); REDRAW_TOP();}
		else @quit;];

[ EverySec;];