! Nameable.h
!	Inform Library extension by John Colagioia (JColagioia@csi.com)
!	based heavily on code by Graham Nelson for "Balances."  All errors
!	should be considered mine, rather than Graham's.
!
! To use:
!	Include "nameable.h" after the parser.  Create objects of type Nameable,
!	and store their "short_name" in the "original_short_name" property.
!	Since short_name does a significant amount of the work, here, do not
!	give the object a short_name, whether it's like this:
!		Nameable Thing_One "thing one";
!	or like this:
!		Nameable Thing_Two
!		  with	short_name "thing two";
!	as both of these will cause chaos of one sort or another.
!
! Customization:
!	There are three hooks that the game programmer is encouraged to
!	use to customize this library:
!		name_in_caps:	This global variable, if set nonzero, capitalizes
!				the name assigned to the Nameable object (the
!				default behavior).  If set to 0, the name
!				is left as lowercase.
!		name_as_proper:	Another global variable.  If set nonzero (the
!				default), the name is treated as a proper
!				name, meaning that articles are not used in
!				descriptions (i.e., "Bob is here" rather than
!				"The Bob is here").  Otherwise, do not set the
!				"proper" attribute.
!		naming_function:
!				This variable, if set, should hold a function
!				that will describe the naming process.  It gets
!				the object as a parameter.  An example of such
!				a function might be:
!
!				[ Namer obj i ;
!				 print "You decide that ", (the) obj, " looks like
!					a ~";
!				 for (i=0:i<8:i++)
!					print (char)(obj.&given_name)->i;
!				 print "~.";
!				 rtrue;
!				];
!
!				If unset, the variable is ignored and nothing
!				will be printed when naming occurs.
!
! Note:  With no licensing notice to the contrary in "Balances," it can be expected
! to be under copyright.  Therefore, the portions of this code should be considered
! as such until notified otherwise by Graham Nelson.  Any modifications to that code
! made by John Colagioia, however, are in the Public Domain, for whatever that's
! worth.

! Storage for the name
Array	name_text_buffer -> 8;
Array	temp_buffer -> 8;
! Internal directives and pseudo-pointers shared among routines
Global	from_buffer = false;
Global	the_named_word = 0;
Global	from_char;
Global	to_char;
! User configuration flags and hooks
Global	name_in_caps = 1;
Global	name_as_proper = 1;
Global	naming_function;

Class Nameable with
  original_short_name,	! The default name to display, until the player
			! comes up with something better.
  article "a",		! Easily changed, but anything nameable is likely
			! to be conceptually non-unique/one of a set, rather
			! than something which gets the article "the."
  given_name 0 0 0 0 0 0 0 0,
			! This is the array where the name will be stored.
  parse_name	! Check for the typical names as well as the name
		! given to the object by the player.
	[ i j flag ;
	 if (parser_action == ##TheSame)
		{
		 for (i=0:i<8:i++)
			if ((parser_one.&number)->i ~=
				(parser_two.&number)->i)
				return (-2);
			return (-1);
		}
	 for (::j++)
		{
		 i = NextWord ();
		 flag = false;
		 if (WordInProperty (i, self, name))
			flag = true;
		 else if (self provides optname && self.optname ~= 0)
			{
			if (WordInProperty (i, self, optname))
				flag = true;
			}
		 else if ((self.&given_name)->0 ~= 0)
			{
			 wn--;
			 if (TextReader (0) == 0)
				return (j);
			 for (i=0:i<8:i++)
				if ((self.&given_name)->i ~= name_text_buffer->i)
					return (j);
			 flag = true;
			}
		 if (flag == false)
			return (j);
		}
	],
  short_name	! Print out the name given by the player, if one exists.
		! Otherwise, use self.original_short_name.
	[ i ;
	 if ((self.&given_name)->1 ~= 0)
		{
		 if (((self.&given_name)->0) ~= 0)
			for (i=0:i<8:i++)
				print "", (char) (self.&given_name)->i;
		}
	 else	print "", (string) self.original_short_name, "";
	 rtrue;
	],
  baptise	! Collect the name from the player's naming command, and
		! place them into the given_name array.
	[ i ;
	 wn = the_named_word;
	 if (TextReader(1)==0) return i;
	 for (i=0: i<8: i++)
		(self.&given_name)->i = name_text_buffer->i;
	 if (name_in_caps)	! Configure capitalization using this flag.
		(self.&given_name)->0 = (self.&given_name)->0 - 32;
	 self.article="the";
	 if (name_as_proper)	! Configure "properness" using this flag.
		give self proper;
	 if (naming_function)	! Use this to print out message or do other
				! handling.
		naming_function (self);
	 rtrue;
	];

[ QuotedText i j f ;	! Accept only quoted strings as names.
	! This could also be done for non-quoted strings, but that causes
	! confusion if the player types:
	!	> name the troll Buddy then go west
	! If not quoted, "Buddy then go west" is interpreted as the name.
 i = WordAddress (wn++);
 i = i - buffer;
 if (buffer->i == '"')
	{
	 for (j=i+1:(j<=(buffer->1)+1)&&(buffer->j~='"'):j++)
		{
		 f = j - (i + 1);
		 temp_buffer->f = buffer->j;
		}
	 for (f=f+1:f<8:f++)
		temp_buffer->f = 0;
	 if (buffer->j == '"')
		f = j;
	 else	f = 0;
	 if (f == 0)
		return (-1);
	 from_char = i + 1;
	 to_char = f - 1;
	 if (from_char > to_char)
		return -1;
	 the_named_word = wn;
	 while ((buffer + f) > WordAddress (wn))
		wn++;
	 wn++;
	 return (player);
	}
 return (-1);
];

[ TextReader flag point i j len where_from start stop ;
 if (from_buffer == true)
	{
	 where_from = temp_buffer;
	 start = 0;
	 stop = 7;
	}
 else	{
	 where_from = buffer;
	 start = from_char;
	 stop = to_char;
	}
 for (i=0:i<8:i++)
	name_text_buffer->i = 0;
 if (flag==1 && start~=stop)
	{
	 for (i=start,j=0:i<=stop&&j<7:i++)
		{
		 name_text_buffer->j = where_from->i;
		 if (where_from->i ~= ' ' or ',' or '.')
			j++;
		}
	 from_char=0;
	 to_char=0;
	 rtrue;
	}
 if (wn > parse->1)
	{
	 wn++;
	 rfalse;
	}
 i = wn * 4 + 1;
 j = parse->i;
 point = j + buffer;
 len = parse->(i-1);
 for (i=0:i<len && i<8:i++)
	name_text_buffer->i = point->i;
 wn++;
 rtrue;
];

[ RenameSub ;
 from_buffer = true;
 if (noun provides baptise)
	{
	 noun.baptise ();
	 rtrue;
	}
 from_buffer = false;
 "", (The) noun, " surely already has a name!";
	! Note that this comment is only for objects of the wrong
	! class.  Players can currently rename the same object
	! as much as they see fit.
];

Verb "name" "call" "rename"
		* noun QuotedText		-> Rename;
