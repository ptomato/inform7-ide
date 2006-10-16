!------------------------------------------------------------------------------
!
!							N P C _ E N G I N E . h
!
!
!	Copyright (c) 1999 Volker Lanz.
!
!	See NPC_Engine.txt for documentation and copyright information.
!
!	serial 990531 - release 3
!
!------------------------------------------------------------------------------

System_file;

!------------------------------------------------------------------------------
!	Delete or comment this if you use timewait.h / waittime.h
!------------------------------------------------------------------------------
#ifndef tw_waiting;
	Object tw_waiting;
#endif;


!------------------------------------------------------------------------------
!	A T T R I B U T E S
!------------------------------------------------------------------------------

Attribute	npc_explored;


!------------------------------------------------------------------------------
!	F A K E   A C T I O N S
!------------------------------------------------------------------------------
fake_action	NPC_PrintVisible;


!------------------------------------------------------------------------------
!	P R O P E R T I E S
!------------------------------------------------------------------------------

Property	npc_open;
Property	looks_to;


!------------------------------------------------------------------------------
!	C O N S T A N T S
!------------------------------------------------------------------------------
Constant	JUMP_follow_type	1;
Constant	PATH_follow_type	2;


!------------------------------------------------------------------------------
!	G L O B A L S
!------------------------------------------------------------------------------
Global		npc_follow_type;
Global		npc_first_room;
Global		npc_first_npc;


!------------------------------------------------------------------------------
!	N P C _ S C O P E
!------------------------------------------------------------------------------
[ NPC_Scope i;
	if (scope_stage == 1) 
		rfalse;
    
	if (scope_stage == 2)
	{
		i = npc_first_npc;
		while (i)
		{
			PlaceInScope(i);
			i = i.npc_next;
		}
		rtrue;
	}

	L__M(##Miscellany, 30);
];


!------------------------------------------------------------------------------
!	V E R B S
!------------------------------------------------------------------------------

Verb 'follow' 'chase' 'pursue' 'trail'
	* noun									-> Follow
    * scope = NPC_Scope						-> Follow;

Verb 'find' 'where' 'where^s'
	*										-> Find
	* scope = NPC_Scope						-> Find
	* noun									-> Find
	* 'is'/'are' scope = NPC_Scope			-> Find
	* 'is'/'are' noun						-> Find;


!------------------------------------------------------------------------------
!	N P C _ R O O M   C L A S S
!------------------------------------------------------------------------------
Class NPC_Room
	with
	npc_label,
	npc_prev_room,
	npc_prev_dir,
	npc_number			0;


!------------------------------------------------------------------------------	
!	N P C _ V I S I B L E   R O O M
!------------------------------------------------------------------------------
NPC_Room NPC_Visible_Room "(npc_visible_room)"
	with
	n_to 0,
	ne_to 0,
	e_to 0,
	se_to 0,
	s_to 0,
	sw_to 0,
	w_to 0,
	nw_to 0,
	u_to 0,
	d_to 0;


!------------------------------------------------------------------------------
!	N P C _ E N G I N E   C L A S S
!------------------------------------------------------------------------------
Class NPC_Engine
	with
	npc_next,				! used to cascade the npcs in a list
	npc_number,				! number of the npc in our list
	npc_meeting 			0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0,
	time_left,	
	npc_met,				! time the player met the npc
	npc_old_loc,			! previous location of npc
	npc_old_dir,			! previous dir the npc went in
	npc_source,				! room the npc comes from
	npc_door_passed,		! door an npc has opened on its way (0 if none)
	npc_stopped,			! has this npc been stopped on its path by the player?
	npc_dest,
	npc_dirs				0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0,
	npc_target,				! The target destination
	npc_stage,				! position along movement array
	npc_move_type,			! 1 defined path, 2 stand still, 3 calculated path
	npc_follow_action,		! action used to follow an npc
	npc_follow_object,		! dir the npc went in
	npc_path_size,			! size of the path, calculated or predefined
	npc_win,
	npc_before_action
		[;
			rfalse;
		],
	npc_after_action
		[;
			rfalse;
		],
	npc_arrived
        [ room;
			room = room;
			rfalse;
		],
	time_out
		[;
			StartDaemon(self);
			NPC_Path(self, self.npc_dest, 2, 1);
			self.npc_dest = 0;
		],
	npc_meet
		[ npc;
			self.&npc_meeting-->(npc.npc_number) = turns;
			rtrue;
		],
	npc_last_saw
		[ npc;
			return (self.&npc_meeting-->(npc.npc_number));
		],
	npc_walkoff
		[i;
			print (The) self, " ";

			if(self.npc_door_passed)
				print "opens ", (the) self.npc_door_passed, " and ";

			if (i == u_obj or d_obj)
				print "is going";
			else
				print "heads off to";
		
			" ", (LeaveToDir) i, ".";
		],
	npc_walkon
		[i;
			print (The) self, " ";

			if(self.npc_door_passed)
				print "opens ", (the) self.npc_door_passed, " and ";

			if (ScopeCeiling(self) == self.npc_target)
				print "steps into the room from ", (ComeFromDir) i;
			else
				print "is walking past you";

			".";
		],
	orders
		[;
			Find:
				<< FIND noun >>;
		],
	daemon
		[ loc tmp new_dir new_loc tmp2;
			if (self.npc_before_action())
				give tw_waiting on;

			switch(self.npc_move_type)
			{
				1, 3:

					if (self.npc_stopped && TestScope(self, player))
					{
						self.npc_stopped--;
						
						if (self.npc_stopped == 0)
							return (NPC_Msg(##Miscellany, 1, self));
						
						rfalse;
					}
					
					self.npc_stopped = 0;

					new_dir = self.&npc_dirs->self.npc_stage;

					! Waiting for one turn
					if (new_dir == 0)
					{
						self.npc_stage--;
						if (self.npc_stage < 0)
						{
							self.npc_move_type = 2;
							give self ~concealed;
						
							if(self.npc_arrived(ScopeCeiling(self)))
								give tw_waiting on;
						}

						rfalse;
					}
					
					loc = ScopeCeiling(self);
					new_loc = ValueOrRun(loc, new_dir.door_dir);
					
					if (ZRegion(new_loc) ~= 1)
					{
						give self ~concealed;
						self.npc_move_type = 2;
						rfalse;
					}

					if (new_loc has door)
					{
						if ((~~(new_loc provides npc_open)) 
								&& (new_loc hasnt open || new_loc has locked))
						{
							if(~~NPC__Path(self, self.npc_target))
							{
								give self ~concealed;
								self.npc_move_type = 2;
							}
								
							rfalse;
						}

						tmp = parent(new_loc);
						
						move new_loc to ScopeCeiling(self);
						
						tmp2 = ValueOrRun (new_loc, door_to);
						
						if (tmp)
							move new_loc to tmp;
						else
							remove new_loc;
						
						if ((new_loc has locked || new_loc hasnt open) 
								&& (new_loc.npc_open(self) == false))
						{
							if (~~NPC__Path(self, self.npc_target))
							{
								give self ~concealed;
								self.npc_move_type = 2;
							}
									
							rfalse;
						}
						
						new_loc = tmp2;
					}

					NPC_Move(self, new_loc, ##Go, new_dir);
						
					self.npc_stage--;

					if (self.npc_after_action())
						give tw_waiting on;

					if (self.npc_move_type == 3)
					{
						if (self.npc_stage < 0)
						{
							give self ~concealed;
							self.npc_move_type = 2;
							
							if (self.npc_arrived(new_loc))
								give tw_waiting on;
						}
						
						rfalse;;
					}

					if (ScopeCeiling(self) == self.npc_target)
					{
						give self ~concealed;
						self.npc_move_type = 2;
						
						if (self.npc_arrived(new_loc))
							give tw_waiting on;
					}
					
					rfalse;

				2:	
					self.npc_old_loc = ScopeCeiling(self);
					if (self.npc_win)
						NPC_PrintWindowVisible(self, self.npc_win, 1);


				default:
					NPC_Error(13, self);
					rtrue;
			}
		],
	has					animate;



!------------------------------------------------------------------------------
!
!	N P C _ S C H E D U L E
!
!	Used to schedule an npc movement to happen after a given delay.
!------------------------------------------------------------------------------
[ NPC_Schedule npc target delay;
	StopTimer(npc);
	
	if (delay < 0)
	{
		NPC_Error(9, npc, target);
		rfalse;
	}

	npc.npc_move_type = 2;
	npc.npc_stopped = 0;
	npc.npc_dest = target;

	StartTimer(npc, delay);
];


!------------------------------------------------------------------------------
!
!	N P C _ M O V E
!
!	Moves an npc. Do not use "move npc to (whatever);" in your code if you
!	use the NPC_Engine. Use this routine instead.
!------------------------------------------------------------------------------
[ NPC_Move npc dest actn dir   visible_now visible_previous x;
    if (dest == location)
		npc.npc_follow_action = 0;
    else if (TestScope(npc, player))
	{
		npc.npc_follow_action = actn;
		npc.npc_follow_object = dir;
	}

	if (npc ofclass NPC_Engine)
	{
		npc.npc_old_loc = ScopeCeiling(npc);
		npc.npc_old_dir = dir;

		objectloop (x in ScopeCeiling(npc))
			if (x ofclass NPC_Engine && x ~= npc)
			{
				npc.npc_meet(x);
				x.npc_meet(npc);
			}	
	}
	
	if (dest == 0)
	{
		npc.npc_move_type = 2;
		remove npc;
	}
	else
	{
		move npc to dest;
		
		if (npc.npc_win)
			NPC_PrintWindowVisible(self, self.npc_win, 1);
		else
		{
			visible_now = NPC_CheckVisible(dest);
			visible_previous = NPC_CheckVisible(npc.npc_old_loc);

			NPC_PrintVisible(location, npc, visible_now, visible_previous);
		}
	}
];


!----------------------------------------------------------------------------	
!
!	N P C _ F I N D   P A T H
!
!	Finds the shortest path for the given npc from its current ScopeCeiling
!	to the destination given. Stores this path in the "npc_dirs" array of
!	the given npc in reverse order. Sets the npc.path_size to the size of
!	the array.
!----------------------------------------------------------------------------	

[ NPC_FindPath npc dest   src i count the_room dir neighbour q tmp lead_to;
	src = ScopeCeiling(npc);

	if (~~(src ofclass NPC_Room))
	{
		NPC_Error(7, npc);
		rfalse;
	}
	
	if (src ~= parent(npc))
	{
		NPC_Error(8, npc, src);
		rfalse;
	}

	#ifdef DEBUG;
		if (parser_trace >= 3)
			print "  [NPC_FindPath called with npc = ", (name) npc, " in ", (name) src, " and dest = ", (name) dest, ".]^";
	#ENDIF;

	i = npc_first_room;
	
	! Cleaning up rooms
	while (i)
	{
		i.npc_label = 32767;
		i.npc_prev_dir = 0;
		give i ~npc_explored;

		i = i.npc_number;
	}
	
	! Set label of source room to 0, so the algorithm knows where to start
	src.npc_label = 0;

	! Main loop begins
	do
	{
		! Increase counter by one
		count++;
		
		#ifdef DEBUG;
			if (parser_trace >= 4)
				print "    [Iteration #", count, ".]^";		
		#endif;

		the_room = 0;
		q = 0;
		
		i = npc_first_room;
		
		
		! Select unexplored room with minmum label
		while (i)
		{
			if (i hasnt npc_explored)
			{
				q++;
			
				if (the_room == 0)
					the_room = i;	
				else if (i.npc_label <= the_room.npc_label)
					the_room = i;
			}

			i = i.npc_number;
		}

		! Couldn't find another room to explore - this must be the end
		if (q == 0)
			jump end_of_exploration;

		! The room we chose now gets the explored attribute
		give the_room npc_explored;

		! And we go through all compass directions in that room		
		objectloop (i in Compass)
		{
			dir = i.door_dir;

			neighbour = ValueOrRun(the_room, dir);
			
			! There's a door in that direction! See if we can open it...
			if (neighbour && neighbour has door)
			{
				tmp = parent(neighbour);
				
				move neighbour to the_room;
				
				lead_to = ValueOrRun(neighbour, door_to);

				if (tmp)
					move neighbour to tmp;
				else
					remove neighbour;
				
				if (neighbour hasnt open)
				{
					if (neighbour provides npc_open && neighbour hasnt locked)
						neighbour = lead_to;
					else
						neighbour = 0;
				}
				else
					neighbour = lead_to;
			}

			! Okay, there is a room	and its label is less than that of the currently explored room
			if (neighbour && neighbour.npc_label > the_room.npc_label)
			{
				#ifdef DEBUG;
					if (parser_trace >= 4)
						print "    [Direction ", (name) i, " in ", (name) the_room, " leads to ", (name) neighbour, ".]^";
				#endif;
				
				neighbour.npc_label = the_room.npc_label + 1;
				neighbour.npc_prev_room = the_room;
				neighbour.npc_prev_dir = i;
					
				#ifdef DEBUG;
					if (parser_trace >= 4)
						print "      [Setting label of ", (name) neighbour, " to ", neighbour.npc_label, ".]^";
				#endif;
			}
			
			if (neighbour == dest)
				jump end_of_exploration;
		}
		
	} until (0);

	.end_of_exploration;
	
	i = dest;
	q = 0;
	
	if (i.npc_prev_dir)
	{
		#ifdef DEBUG;
			if (parser_trace >= 3)
				print "    [Path found for ", (name) npc, " from ", (name) src, " to ", (name) dest, ":";	
		#endif;
		
		! Copy the path found into the npc_dirs array of the npc given
		while (i.npc_prev_dir)
		{
			#ifdef DEBUG;
				if (parser_trace >= 3)
					print " ", (name) i.npc_prev_dir, " (from ", (name) i, ") ";
			#endif;
			
			if (q > 31)
			{
				NPC_Error(17, npc, dest);
				rfalse;
			}
			
			npc.&npc_dirs->q = i.npc_prev_dir;
			q++;
			i = i.npc_prev_room;
		}
	}
	else
		rfalse;
	
	! Set the correct path size for the npc
	npc.npc_path_size = q-1;
		
	#ifdef DEBUG;
		if (parser_trace >= 3)
			print ".]^";
	#endif;
		
	rtrue;
];

!----------------------------------------------------------------------------	
!
!	N P C _ _ P A T H
!
!	Tries to find a path for the given npc from its current ScopeCeiling to
!	the destination given. If the npc's daemon is active, the npc will then
!	start moving. If not, nothing will happen. This routine does not start
!	the daemon.
!----------------------------------------------------------------------------	
[ NPC__Path npc dest ifblocked use_best   i src;
	ifblocked = ifblocked;
	use_best = use_best;

	if (~~(npc ofclass NPC_Engine))
	{
		NPC_Error(15, npc);
		rfalse;
	}

	src = ScopeCeiling(npc);

	if (parent(npc) ~= src)
		move npc to src;

	if (src == dest)
	{
		#ifdef DEBUG;
			if (parser_trace >= 3)
				print "    [NPC__Path called for ", (name) npc, " with src and dest = ", (name) src, ".]^";
		#endif;
		
		npc.npc_arrived(src);
		
		rtrue;
	}
	
	npc.npc_target = dest;
	npc.npc_source = src;
	npc.npc_door_passed = 0;
	
	for (i=0: i < 32: i++)
		npc.&npc_dirs->i = 0;
	
	if (~~NPC_FindPath(npc, dest))
	{
		npc.npc_move_type = 2;
		NPC_Error(4, npc, dest);
	}
	else
	{
		npc.npc_stage = npc.npc_path_size;
		npc.npc_move_type = 1;
		
		! Now we say the npc is concealed, since descriptions for him will be
		! printed by the daemon rather than by the describe() property
		give npc concealed;
	}

	rtrue;
];


!------------------------------------------------------------------------------
!
!	N P C _ P A T H
!
!	This is just a stub which can be replaced within your story file.
!------------------------------------------------------------------------------
[ NPC_Path npc dest ifblocked use_best;
	NPC__Path(npc, dest, ifblocked, use_best);
];


!------------------------------------------------------------------------------
!	F I N D   S U B
!
!	Deals both with "where is npc?" and "npc, where is npc?" questions
!------------------------------------------------------------------------------
[ FindSub  m i;

	! No noun has been given.
	if (noun == 0)
	{
		NPC_Msg(##Find, 7);
		rtrue;
	}	

	! The given object is clearly visible to the actor (this one might cause trouble in certain
	! situations where something is visible to the actor who is being asked, but insivible to
	! the player)
	if (TestScope(noun, actor))
	{
		if (actor == player)
			NPC_Msg(##Find, 9);
		else
			NPC_Msg(##Find, 8);
	}
	else if (noun has animate)
	{
		if (actor == player)
		{
			m = noun.npc_met;
			
			i = NPC_CheckVisible(ScopeCeiling(noun));
			
			if (i)
			{
				noun.npc_met = turns;
				print_ret (The) noun, " ", (isorare) noun, " off to ", (LeaveToDir) i, ".";
			}
		}				
		else
			m = actor.npc_last_saw(noun);
		
		if (m == 0)
		{
			NPC_Msg (##Find, 5, actor);
			
			print " ", (itorthem) noun, " ";

			NPC_Msg (##Find, 6, actor);
		}
		else
		{
			i = turns - m;	! That's the time that's passed since actor met noun

#ifdef DEBUG;
			if (parser_trace >= 2)
				print "[time since last meeting: ", i, " minutes]^";
#endif;

			NPC_Msg(##Find, 2, actor);
				
			print " ", (itorthem) noun, " ";

			if (i > 120)		NPC_Msg(##Find, 1, 1);
			else if (i > 80)	NPC_Msg(##Find, 1, 2);
			else if (i > 45)	NPC_Msg(##Find, 1, 3);
			else if (i > 30)	NPC_Msg(##Find, 1, 4);
			else if (i > 10)	NPC_Msg(##Find, 1, 5);
			else if (i > 5)		NPC_Msg(##Find, 1, 6);
			else				NPC_Msg(##Find, 1, 7);
			
			if (actor ~= player)
			{
				print " ";
				NPC_Msg(##Find, 3, noun);
			}
		}
	}		
	else
		NPC_Msg(##Find, 4, actor);
	
	new_line;
	rtrue;
];

	
!------------------------------------------------------------------------------
!
!	N P C _ P R E P A T H
!
!	Used like in moveclass.h.
!------------------------------------------------------------------------------
[ NPC_PrePath npc arname arentries   i;
	if (npc ofclass NPC_Engine)
	{
		npc.npc_stage = arentries - 1;
		npc.npc_move_type = 3;
		
		for (i = 0: i < arentries: i++)
			npc.&npc_dirs->i = arname->(arentries - i - 1);

		npc.npc_path_size = arentries;
	}
	else
	{
		NPC_Error(12, npc);
		rfalse;
	}
	
	rtrue;
];


!------------------------------------------------------------------------------
!
!	N P C _ P R I N T   V I S I B L E
!
! This routine does the actual printing of visible NPCs depending on the given 
! arguments:
!
! 	loc				location where we print this from
!	npc				npc
!	new_visible		direction npc is visible to; this value has most certainly
!					been calculated by NPC_CheckVisible()
!	old_visible		previous direction npc was visible to; this value has
!					as well most certainly been calculated by NPC_CheckVisible()
!	flag			1 means: include characters who aren't moving
!
! The routine returns true if it printed a message, otherwise false
!------------------------------------------------------------------------------

[ NPC_PrintVisible loc npc new_visible old_visible flag   rval;
	#ifdef DEBUG;
		if (parser_trace >= 2)
		{
			print "^[NPC_PrintVisible with ", (the) npc, " from ", (name) loc, " - ";
			print "last dir of visibility: ", (name) old_visible, " - new dir: ", (name) new_visible, " - ";
			print "flag: ", flag, ".]^";
			
			print "    [npc is in: ", (name) ScopeCeiling(npc), " - previous location: ", (name) npc.npc_old_loc, " - ";
			print "previous_dir: ", (name) npc.npc_old_dir, ".]^";
		}
	#endif;
	
	! Take care of meeting between NPC and player
	if (new_visible || old_visible)
		npc.npc_met = turns;

	! We've got the following cases where we want to print a message:
	! (A) The npc just walked out of the room the player is in.
	! (B) The npc just walked into the room the player is in.
	! (C) The npc isn't moving, but flag is set to 1
	! (D) The npc wasn't visible the last move and comes into view
	! (E) The npc was visible the last move, is moving and still is visible
	! (F) The npc was visible the last move and disappears from sight
	
	! --- A ---
	! The NPC just walked out of the room where we print this from

	if (npc.npc_old_loc == loc && ScopeCeiling(npc) ~= loc)
	{
		npc.npc_met = turns;
		
		if (ZRegion(npc.npc_walkoff) == 3)
		{
			print (The) npc, " ", (string) npc.npc_walkoff, " to ";
			LeaveToDir(npc.npc_old_dir);
			print ".^";
		}
		else
			npc.npc_walkoff(npc.npc_old_dir);
		
		rval = true;
	}
	
	! --- B ---
	! The NPC just walked into the room where the player is
		
	else if (ScopeCeiling(npc) == loc && npc.npc_old_loc ~= loc)
	{
		npc.npc_met = turns;

		if (ZRegion(npc.npc_walkon) == 3)
		{
			print (The) npc, " ", (string) npc.npc_walkon, " from ";
			ComeFromDir(npc.npc_old_dir);
			print ".^";
		}
		else
			npc.npc_walkon(npc.npc_old_dir);
			
		rval = true;
	}
	
	! --- C ---
	! The NPC isn't moving, but this routine was called with flag == 1 (what probably 
	! means that it was called because the player looked through a window).

	else if (ScopeCeiling(npc) == npc.npc_old_loc && new_visible ~= 0)
	{
		if (flag == 1)
		{
			#ifdef DEBUG;
				if (parser_trace >= 3)
					print "    [NPC isn't moving, but flag is 1.]^";
			#endif;

			print (The) npc, " ";
			
			NPC_Msg(##NPC_PrintVisible, 1, new_visible);

			print " ", (LeaveToDir) new_visible, ".^";
			
			rval = true;
		}
	}
	else if (new_visible ~= 0)
	{
		! --- D ---
		! The NPC wasn't visible before, but now comes into sight
		
		if (old_visible == 0 && npc.npc_old_loc ~= loc)
		{
			#ifdef DEBUG;
				if (parser_trace >= 3)
					print "    [NPC was not visible before and now comes into view.]^";
			#endif;
			
			NPC_Msg(##NPC_PrintVisible, 2, new_visible);

			print " ", (the) npc, " ";
			
			if (npc.npc_door_passed)
			{
				NPC_Msg(##NPC_PrintVisible, 3);
				print " ";
			}
				
			NPC_Msg(##NPC_PrintVisible, 4, npc, loc);
			
			print ".^";
			rval = true;
		}
		
		! --- E ---
		! The NPC was visible the last move and still is
		
		else if (ScopeCeiling(npc) ~= npc.npc_old_loc)
		{
			#ifdef DEBUG;
				if (parser_trace >= 3)
					print "    [NPC was visible before and still is.]^";
			#endif;

			print (The) npc, " ";

			NPC_Msg(##NPC_PrintVisible, 5, npc, new_visible);

			print ".^";
			rval = true;
		}
	}

	! --- F ---
	! The NPC was visible the last move and now disappears from sight
	
	else if (new_visible == 0 && old_visible && ScopeCeiling(npc) ~= npc.npc_old_loc)
	{
		#ifdef DEBUG;
			if (parser_trace >= 3)
				print "    [NPC was visible before and now disappears from sight.]^";
		#endif;

		print (The) npc;
		
		NPC_Msg(##NPC_PrintVisible, 6, old_visible);
		
		print " ";
		
		if(npc.npc_door_passed)
		{
			NPC_Msg(##NPC_PrintVisible, 3);
			print " ";
		}

		NPC_Msg(##NPC_PrintVisible, 7, npc);

		print ".^";
		rval = true;
	}
	
	! Here come the game-specific things...
	else if (NPC_AfterPrintVisible(npc, loc))
	{
		#ifdef DEBUG;
			if (parser_trace >= 3)
				print "    [NPC_AfterPrintVisible returned true.]^";
		#endif;

		rval = true;
	}
	
	! Nothing happens at all: NPC wasn't visible before and still isn't.
	else
	{
		#ifdef DEBUG;
			if (parser_trace >= 3)
				print "    [NPC was not visible before and still isn't.]^";
		#endif;
	}

	! If this was set, we informed the player about it and must clear it now
	npc.npc_door_passed = 0;
	
	! Now let's see if we printed anything at all. If so, we set tw_waiting to on and clear
	! the npc's window property
	if (rval)
	{
		npc.npc_win = 0;
		give tw_waiting on;
	}

	#ifdef DEBUG;
		if (parser_trace >= 2)
			print "[NPC_PrintVisible returns ", rval, ".]^";
	#endif;
	
	! And return true if we did print something, otherwise false
	return(rval);
];


!------------------------------------------------------------------------------
!
!	N P C _ A F T E R   P R I N T   V I S I B L E
!
!	This is a stub for an entry point. A story may replace this to check if
!	it wants to print a message of its own.
!------------------------------------------------------------------------------
[ NPC_AfterPrintVisible npc loc;
	npc = npc;
	loc = loc;
	rfalse;
];


!------------------------------------------------------------------------------
!
!	N P C _ C H E C K V I S I B L E
!
! This routine checks if the location given ("loc") is visible from the 
! location of the player (if one argument is given) or from the location
! given (if two args are passed).
!------------------------------------------------------------------------------
[ NPC_CheckVisible test_loc current_room    current_compass check dir last_room tmp;

	! If no location was given as an argument, we take the player's current location
	if (~~current_room)
		current_room = location;

	tmp = current_room;

	#ifdef DEBUG;
		if (parser_trace >= 3)
			print "^[NPC_CheckVisible called for room '", (name) test_loc, "' from location '", (name) current_room, "'.]^";
	#endif;

	! Calling a story entry point to prevent certain rooms from being visible
	if (NPC_BeforeCheckVisible(current_room, test_loc))
	{

		#ifdef DEBUG;
			if (parser_trace >= 3)
				print "[NPC_CheckVisible returns false: NPC_BeforeCheckVisible returned true.]^";	
		#endif;

		rfalse;
	}

	! Now go through all the objects in the compass to see if we can see the test_loc in that 
	! direction
	objectloop (current_compass in Compass)
	{
		current_room = tmp;
		last_room = current_room;
		
		! Get the direction that compass objects points to
		dir = ValueOrRun(current_compass, door_dir);

		#ifdef DEBUG;
			if (parser_trace >= 4)
				print "    [Looking to '", (name) current_compass, "' in room '", (name) current_room, "']^";
		#endif;

		! Now we go in the current compass direction until we can't any longer
		while ((check = ValueOrRun(current_room, dir)) ~= 0)
		{
			! This is a door, but we can never see through doors, open or closed
			if (check has door)
				jump NextCompass;
					
			! We have found a compass direction in which the test_loc is visible from the location of the playe
			if (test_loc == check)
			{

				#ifdef DEBUG;
					if (parser_trace >= 3)
						print "[NPC_CheckVisible returns '", (name) current_compass, "'.]^";
				#endif;

				return(current_compass);
			}
			
			! If not, we go on in the direction
			last_room = current_room;
			current_room = check;
		}
		
		.NextCompass;
	}

	#ifdef DEBUG;
		if (parser_trace >= 3)
			print "[NPC_CheckVisible returns false.]^";	
	#endif;

	rfalse;
];

!------------------------------------------------------------------------------
!
!	N P C _ B E F O R E C H E C K V I S I B L E
!
!	A story entry point called before NPC_CheckVisible takes place. May be 
!	replaced by a story file to avoid going through NPC_CheckVisible by 
!	returning true.
!	Is passed current_room, where NPC_CheckVisible would start from, and
!	test_loc, the location the visibility of which is to be checked.
!------------------------------------------------------------------------------
[ NPC_BeforeCheckVisible current_room test_loc;
	current_room = current_room;
	test_loc = test_loc;
	rfalse;
];


!----------------------------------------------------------------------------	
!	N P C _ L O O K   T H R O U G H   W I N D O W
!----------------------------------------------------------------------------	
[ NPC_LookThroughWindow win   npc;
	npc = npc_first_npc;
	
	while (npc)
	{
		npc.npc_win = win;
		npc = npc.npc_next;
	}
];

!----------------------------------------------------------------------------	
!	N P C _ P R I N T  W I N D O W  V I S I B L E
!
!	This one prints npc's visible through the given window win.
!------------------------------------------------------------------------------
[ NPC_PrintWindowVisible npc win always    dto ddir new old msg_printed k l m address;
	#ifdef DEBUG;
		if (parser_trace >= 3)
			print "[NPC_PrintWindowVisible called for ", (name) win, " with always = ", always, ".]^";	
	#endif;

	if (win provides door_dir && win provides door_to && win provides looks_to)
	{
		dto = ValueOrRun(win, door_to);
		ddir = ValueOrRun(win, door_dir);
		
		NPC_Visible_Room.ddir = dto;
	
		#ifdef DEBUG;
			if (parser_trace >= 4)
				print "    [NPC_PrintWindowVisible: dto = ", (name) dto, " - dir = ", (name) ddir, ".]^";	
		#endif;

		address=win.&looks_to;
			
		if (address ~= 0)
		{
			if (ZRegion(address-->0) == 2)
			{
				NPC_Error(16, win);
				rfalse;
			}
			else
			{
				k=win.#looks_to;

				new = 0;
				old = 0;
				for (l=0: l < k/2: l++)
				{
					m = address-->l;
					if (ScopeCeiling(npc) == m)
						new = NPC_DirForTo(ddir);
					else if (npc.npc_old_loc == m)
						old = NPC_DirForTo(ddir);
				}
			}
				

			#ifdef DEBUG;
				if (parser_trace >= 3)
					print "    [Calling NPC_Print Visible with npc = ", (name) npc, " - new = ", (name) new, " - old = ", (name) old, "]^";
			#endif;

			if(NPC_PrintVisible (NPC_Visible_Room, npc, new, old, always))
				msg_printed = 1;
		}
			
		
		NPC_Visible_Room.ddir = 0;

		npc.npc_win = 0;

		if (msg_printed)
			rtrue;
	}

	rfalse;
];


!------------------------------------------------------------------------------
!	
!	F O L L O W S U B
!
!	Action routine for the follow verb.
!------------------------------------------------------------------------------
[ FollowSub savenoun savepos rval;
	if (NPC_BeforeFollowing(noun))
		rfalse;;

	savenoun = noun;
	savepos = location;
	
	if (npc_follow_type == JUMP_follow_type)
		rval = NPC_JumpFollow(noun);
	else if (npc_follow_type == PATH_follow_type)
		rval = NPC_PathFollow(noun);
	else
		rval = NPC_NormalFollow(noun);

	NPC_AfterFollowing(savenoun, savepos, rval);
	
	rtrue;
];

!------------------------------------------------------------------------------
!
!	N P C _ J U M P F O L L O W
!
!	Used for following in JUMP_FOLLOW_TYPE.
!------------------------------------------------------------------------------
[ NPC_JumpFollow item;
	PlayerTo(ScopeCeiling(item), 2);

    rtrue;
];

!------------------------------------------------------------------------------
!
!	N P C _ N O R M A L F O L L O W
!
!	Used for following in normal follow type.
!------------------------------------------------------------------------------
[ NPC_NormalFollow item;
    < (item.npc_follow_action) item.npc_follow_object >;

	rtrue;
];


!------------------------------------------------------------------------------
!	A dummy npc object used to calculate a path for PATH_following
!------------------------------------------------------------------------------
Object npc_temp_npc
	class NPC_Engine
	has concealed;


!------------------------------------------------------------------------------
!
!	N P C _ P A T H F O L L O W
!
!	Used for following in PATH_FOLLOW_TYPE.
!------------------------------------------------------------------------------
[ NPC_PathFollow item;
	move npc_temp_npc to location;
		
	NPC__path(npc_temp_npc, ScopeCeiling(item), 0, 1);

	< Go (npc_temp_npc.&npc_dirs->npc_temp_npc.npc_stage) >;
	
	remove npc_temp_npc;
	
	rtrue;
];

!------------------------------------------------------------------------------
!
!	N P C _ B E F O R E F O L L O W I N G
!
!	Default for a story entry point called before a following action will take
!	place. May cancel the following action by returning true.
!------------------------------------------------------------------------------
[ NPC_BeforeFollowing item;
	if (item == player)
		return(NPC_Msg(##Follow, 2));

    if (~~(item ofclass NPC_Engine))
		return(NPC_Msg(##Follow, 3, item));

    if (TestScope(item, player))
		return(NPC_Msg(##Follow, 4, item));

	if (item.npc_met == 0)
		return(NPC_Msg(##Follow, 5, item));

    if (turns - item.npc_met > 0 || (npc_follow_type == 0 && noun.npc_follow_object == 0))
		return(NPC_Msg(##Follow, 6, item));

	rfalse;
];

!------------------------------------------------------------------------------
!
!	N P C _ A F T E R F O L L O W I N G
!
!	Default for a story entry point called after a following action has taken 
!	place.
!------------------------------------------------------------------------------
[ NPC_AfterFollowing item oldpos succ;
	if (npc_follow_type ~= JUMP_follow_type or PATH_follow_type)
		if (TestScope(item, player) == 0 && location ~= oldpos || succ == 0)
		{
			NPC_Msg (##Follow, 1, item);
			rtrue;
		}

	rfalse;
];


!------------------------------------------------------------------------------
!
!	N E W   R O O M
!
!	Cleans up following variables everytime the player enters a new room.
!------------------------------------------------------------------------------
[ NewRoom i;
	if (npc_follow_type ~= 0)
		rfalse;
		
	i = npc_first_npc;
	
	while (i)
	{
		i.npc_follow_object = 0;
		i = i.npc_next;
	}

	rfalse;
];


!------------------------------------------------------------------------------
!
!	N P C _ I N I T I A L I S E
!
!	This is the initialisation function of the npc engine. Call this one in
!	your game's Initialise() function.
!	Takes the type of following you want to use as an argument.
!------------------------------------------------------------------------------
[ NPC_Initialise t   current prev c;
	if ((npc_follow_type = t) < 0 || t > 2)
	{
		NPC_Error(5, t);
		@quit;
	}
	
	objectloop (current ofclass NPC_Engine)
	{
		if (current ~= npc_temp_npc)
		{
			current.npc_number = c;

			current.npc_move_type = 2;
			current.npc_old_loc = ScopeCeiling(current);

			StartDaemon(current);

			if (prev)
				current.npc_next = prev;

			prev = current;

			if (++c > 32)
			{
				NPC_Error(6);
				@quit;
			}
		}
	}

	npc_first_npc = prev;
	
	prev = 0;
	
	objectloop (current ofclass NPC_Room)
	{
		if (prev)
			current.npc_number = prev;
			
		prev = current;
	}
	
	npc_first_room = prev;
];


!------------------------------------------------------------------------------
!
!	N P C _ M S G
!
!	All the messages the NPC Engine might print.
!------------------------------------------------------------------------------
[ NPC_Msg act n x1 x2;
	switch(act)
	{
		##Miscellany:
			switch(n)
			{
				1:	print_ret (The) x1, " starts to move about distractedly.";

				default:
					NPC_Error(14, n);
					rtrue;

			}
		
		##NPC_PrintVisible:
			switch(n)
			{
				1:
					print "is off to";
	
					if (x1 == u_obj or d_obj)
						print "ward";
					rtrue;
					
				2:
					print "To";
					if (x1 == u_obj or d_obj)
						print "ward";
					print " ", (LeaveToDir) x1, ",";
					rtrue;
					
				3:
					print "opens a door and";
					rtrue;

				4:
					print "comes into view from ", (ComeFromDir) x1.npc_old_dir;
					rtrue;

				5:
					print "is ";
					if (x1.npc_old_dir == d_obj)
						print "going downstairs";
					else if (x1.npc_old_dir == d_obj)
						print "going upstairs";
					else
					{
						print "to ", (LeaveToDir) x2, ", heading ";
						if ((x2 == n_obj && x1.npc_old_dir == s_obj) ||
								(x2 == ne_obj && x1.npc_old_dir == sw_obj) ||
								(x2 == e_obj && x1.npc_old_dir == w_obj) ||
								(x2 == se_obj && x1.npc_old_dir == nw_obj) ||
								(x2 == s_obj && x1.npc_old_dir == n_obj) ||
								(x2 == sw_obj && x1.npc_old_dir == ne_obj) ||
								(x2 == w_obj && x1.npc_old_dir == e_obj) ||
								(x2 == nw_obj && x1.npc_old_dir == se_obj) ||
								(x2 == u_obj && x1.npc_old_dir == d_obj) ||
								(x2 == d_obj && x1.npc_old_dir == u_obj))
							print "towards you";
						else
							print "toward ", (LeaveToDir) x1.npc_old_dir;
					}
					rtrue;
					
				6:
					print ", off to ", (LeaveToDir) x1, ",";
					rtrue;
					
				7:
					print "disappears from sight ";

					if (x1.npc_old_dir == u_obj)
						print "up the stairs";
					else if(x1.npc_old_dir == d_obj)
						print "down the stairs";
					else
						print "to ", (LeaveToDir) x1.npc_old_dir;
					rtrue;
					
				default:
					NPC_Error(10, n);
					rtrue;
			}
	
		##Follow:
			switch(n)
			{
				1:	print_ret (The) x1, " doesn't seem to be here any more.";
	
				2:	"You are always following yourself.";				

				3:	"One cannot follow ", (a) x1, ".";

				4:	"You are in the same place as ", (the) x1, "!";

				5:	"You haven't even seen ", (the) x1, " so far.";

				6:	"You seem to have lost track of ", (the) x1, ".";

				default:
					NPC_Error(11, n);
					rtrue;
			}
			
		##Find:
			switch(n)
			{
				1:
					switch(x1)
					{
						1:			print "a few hours";         
						2:			print "an hour or two";      
						3:			print "about an hour";       
						4:			print "about half an hour";  
						5:			print "about fifteen minutes";    
						6:			print "less than ten minutes";
						7:			print "just a minute";
						default:
							NPC_Error(3);
							rtrue;
					}
				
					print " ago.";
					rtrue;
			
				2:
					if (x1 == player)
						print "You";
					else
						print "~I";
					print " last saw";
					rtrue;
			
				3:
					print "I don't know where ";
					if (x1 has female)
						print "she";
					else
						print "he";
					print " went, though.~";
					rtrue;

				4:
					if (x1 == player)
						print "You'll have to find out yourself.";
					else
						print "~I don't know.~";
					rtrue;

				5:
					if (x1 == player)
						print "You";
					else
						print "~I";
					print " haven't seen";
					rtrue;
				
				6:
					if (x1 == player)
						print "yet.";
					else
						print "today.~";
					rtrue;
				
				7:
					"That question cannot be answered.";
				
				8:
					print "~Ahem...~";
					rtrue;
				
				9:
					print "Are you blind?";
					rtrue;
				
				default:
					NPC_Error(1, n);
					rtrue;
			}
		
		default:
			NPC_Error(2, action);
			rtrue;
	}
];


!------------------------------------------------------------------------------
!
!	N P C _ E R R O R
!
!	All the error messages the NPC Engine might print.
!------------------------------------------------------------------------------
[ NPC_Error n x1 x2;
	print "^[NPC Engine Runtime Error ", n, ": ";
	
	switch(n)
	{
		1:			print "NPC_Msg called with unknown value for FIND (", x1, ").";

		2:			print "NPC_Msg called with unknown action (", x1, ").";

		3:			print "NPC_Msg called for FIND, n = 1, unknown value for time passed since meeting.";

		4:			print "Couldn't find a path for ", (name) x1, " from ", (name) ScopeCeiling(x1), " to ", (name) x2, ".";

		5:			print "NPC_Initialise called with unknown npc_follow_type ", x1, ".";

		6:			print "Number of NPCs in the story is higher than 32.";
		
		7:			print "NPC_FindPath called with npc ", (name) x1, ": Location of npc isn't of NPC_Room class.";

		8:			print "NPC_FindPath called with npc ", (name) x1, ": npc isn't in a room (", (name) x2, ").";
		
		9:			print "NPC_Schedule for ", (name)  x1, " to ", (name) x2, " called with delay < 0.";

		10:			print "NPC_Msg called with unknown value for NPC_PRINTVISIBLE (", x1, ").";

		11:			print "NPC_Msg called with unknown value for FOLLOW (", x1, ").";
		
		12:			print "NPC_PrePath called for non-NPC_Engine object '", (the) x1, "'.";
		
		13:			print "npc_move_type value for ", (the) x1, " is unknown.";
		
		14:			print "NPC_Msg called with unknown value for MISCELLANY (", x1, ").";
		
		15:			print "NPC__Path called for non-NPC_Engine object '", (the) x1, "'.";
		
		16:			print "NPC_PrintWindowVisible: looks_to as routine not supported (window '", (name) x1, "').";
		
		17:			print "Path too long for ", (the) x1, " to ", (the) x2, ".";
		
		default:	print "Unknown error.";
	}
	
	"]";
];


!------------------------------------------------------------------------------
!	L I T T L E   H E L P E R S
!------------------------------------------------------------------------------

!------------------------------------------------------------------------------
[ DaemonActive daemon    x;
	for (x=0: x < active_timers: x++)
		if (the_timers-->x == $8000 + daemon)
			rtrue;
	
	rfalse;
];


!------------------------------------------------------------------------------
[ LeaveToDir i;
	switch (i)
	{
		n_obj:		print "the north";
		s_obj:		print "the south";
		e_obj:		print "the east";
		w_obj:		print "the west";
		ne_obj:		print "the northeast";
		nw_obj:		print "the northwest";
		se_obj:		print "the southeast";
		sw_obj:		print "the southwest";
		u_obj:		print "upstairs";
		d_obj:		print "downstairs";
		in_obj:		print "inside";
		out_obj:	print "outside";
	}
];


!------------------------------------------------------------------------------
[ ComeFromDir i;
	switch (i)
	{
		s_obj:		print "the north";
		n_obj:		print "the south";
		w_obj:		print "the east";
		e_obj:		print "the west";
		sw_obj:		print "the northeast";
		se_obj:		print "the northwest";
		nw_obj:		print "the southeast";
		ne_obj:		print "the southwest";
		d_obj:		print "upstairs";
		u_obj:		print "downstairs";
		out_obj:	print "inside";
		in_obj:		print "outside";
	}
];



!------------------------------------------------------------------------------
[ NPC_DirForTo i   x;
	objectloop (x in Compass)
		if (x.door_dir == i)
			return (x);

	return (0);
];


!------------------------------------------------------------------------------
!	E N D   O F   F I L E
!------------------------------------------------------------------------------
