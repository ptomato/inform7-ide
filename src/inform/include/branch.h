!---------------------------------------------------------------------------------
!
! branch.h
!
! a class for branching menus
!
! by Chris Klimas
!
! designed for Inform 6.15
! you can use this extension for free, but give me credit
!
!
! an overview:
!
! this a class definition.
!
! num_branches 			how many branches are in the set
!
! branch_1, etc			the text of each branch
!
! chosen_1, etc			a string of function, printed or run if
!						the respective branch is chosen
!
! display				call this function to display the branch
!
!
! how you should really use this library:
!
! step 1. draw all the branches out on paper. figure out what's going on,
!         because once you start coding, everything gets more confusing.
! step 2. design an object for each branch point.
! step 3. code chosen_1, etc., appropriately to link to other branch_sets
!		  (by calling the next branch_set's display).
! step 4. test.
! step 5. repeat step 4.
!
! include this library before any branch_set definitions.
!
!---------------------------------------------------------------------------------
 
! This function is a low-level input hook.

[ Inkey key;
  @read_char 1 key;
  return key;
];

class branch_set
	with
		num_branches
			1,
		branch_1
			[;
			 print "** Error: branch_1 undefined for ",(name) self;
			],
		branch_2
			[;
			 print "** Error: branch_2 undefined for ",(name) self;
			],
		branch_3
			[;
			 print "** Error: branch_3 undefined for ",(name) self;
			],
		branch_4
			[;
			 print "** Error: branch_4 undefined for ",(name) self;
			],
		chosen_1
			[;
			 print "** Error: chosen_1 undefined for ",(name) self;
			 InKey();
			 quit;
			],
		chosen_2
			[;
			 print "** Error: chosen_2 undefined for ",(name) self;
			 InKey();
			 quit;
			],
		chosen_3
			[;
			 print "** Error: chosen_3 undefined for ",(name) self;
			 InKey();
			 quit;
			],
		chosen_4
			[;
			 print "** Error: chosen_4 undefined for ",(name) self;
			 InKey();
			 quit;
			],
		display
			[ currentChoice choiceMade x;
  
  			  ! First, erase the upper window and print the menu items.
  
  			  @erase_window 1; 						! Erase the top window.
  			  x = self.num_branches + 1;
  			  @split_window x;				 		! Set it to the needed size.
  			  @set_window 1;
  			  style reverse;						! Clear the window to reverse
  			  font off;
  			  @set_cursor 1 1;
  			  spaces((0->33)-1);
  			  @set_cursor 2 1;
  			  spaces((0->33)-1);
  			  @set_cursor 1 2;						! Print each item.
  			  print (string) self.branch_1;
  			  if (self.num_branches > 1) {
  					@set_cursor 3 1;
  					spaces((0->33)-1);
  					@set_cursor 2 2;
  					print (string) self.branch_2;
  					};
  			  if (self.num_branches > 2) {
  					@set_cursor 4 1;
  					spaces((0->33)-1);
  					@set_cursor 3 2;
  					print (string) self.branch_3;
  					};
  			if (self.num_branches > 3) {
  					@set_cursor 5 1;
  					spaces((0->33)-1);
  					@set_cursor 4 2;
  					print (string) self.branch_4;
  					};
  
  			! Now, go into a simple event loop that first prints an indicator of
  			! the currently selected item, then waits for a keypress, then
  			! adjusts currentChoice or sets choiceMade to true.
  
  			currentChoice = 1;
  			do {
  	 			font off;							! Clear the window to reverse
  	 			style reverse;
	 			for(x=1:x==5:x++) {
  	 	 			 @set_cursor x 1;
  	 	 			 spaces((0->33)-1);
  				};
  	 			@set_cursor 1 2;
  	 			if (currentChoice == 1) style roman;
  	 			print (string) self.branch_1;
  	 			if (self.num_branches > 1) {
  	 	 			@set_cursor 2 2;
  	 	 			style reverse;
  	 	 			if (currentChoice == 2) style roman;
  	 	 			print (string) self.branch_2;
  	 				};
  	 			if (self.num_branches > 2) {
  	 	 			@set_cursor 3 2;
  	 	 			style reverse;
  	 	 			if (currentChoice == 3) style roman;
  	 	 			print (string) self.branch_3;
  	 				};
  	 			if (self.num_branches > 3) {
  	 	 			@set_cursor 4 2;
  	 	 			style reverse;
  	 	 			if (currentChoice == 4) style roman;
  	 	 			print (string) self.branch_4;
  	 				};
		
		 		! Wait for a keypress and act on it.
	 
	 			switch(InKey()) {
	 				
	 				! N,n,down arrow
	 	 			
	 	 			78,110,130:
	 	 			if (currentChoice < self.num_branches) 
 	 					currentChoice++;
	 	 			else
	 	 				@sound_effect 2;
	 	
	 				! P,p,up arrow
	 	 			
	 	 			80,112,129:
	 	 			if (currentChoice > 1)
	 	 				currentChoice--;
	 	 			else
	 	 				@sound_effect 2;
	 				
	 				! Return,right arrow
	 	 			
	 	 			13,29:
	 	 			choiceMade = true;
	 				
	 				! Q,q
	 	  			
	 	  			81,113:			@sound_effect 2;
	 				
	 				};
				}
  			until (choiceMade);
  			@erase_window 1;
  			@set_window 0;
  			style roman;
  			font on;
 	 		switch(currentChoice) {
 	 			
 	 			1:
 	 			self.chosen_1();
 	 			
 	 			2:
 	 			self.chosen_2();
 	 			
 	 			3:
 	 			self.chosen_3();
 	 			
 	 			4:
 	 			self.chosen_4();
 	 			
 	 			};
			];
