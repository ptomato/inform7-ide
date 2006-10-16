!-------------------------------------------------------------------------------
!  dynobj.h - an Inform library to implement truly dynamic objects
!  Copyright (C) 1999 by John Cater
!  Send comments or suggestions to: katre@rice.edu
!-------------------------------------------------------------------------------
#Ifndef dynobj_h;
Constant dynobj_h;
Message "[Including <dynobj>]";
System_file;

!! dynobj.h is a very simple, easy to use library.  Simply Include the file in
!! your code, then use the five class properties as normal.  dynobj will
!! create and destroy objects when you call class.create or class.destroy.
!! The recreate and copy properties work as before.  The only major change is
!! the behaviour of class.remaining, which now returns 1 in all cases, as it
!! is impossible to determine how many more objects can truly be created.

!! A dynamic class should be declared as 'Class foo(0) with...;'.  By placing
!! 0 in the parentheses you let dynobj know that this is a class that can have
!! dynamic members, and you do not waste resources by having extraneous
!! objects lying around unused.

!! dynobj relies on the debuglib.h debugging library.  Therefore, you can use
!! the global variable 'debugging_level' to specifiy wether you want
!! 'NO_DEBUGGING' (no messages will be printed), 'DEBUG_CRITICAL' (only
!! critical errors will be reported), 'DEBUG_WARNING' (warning messages will
!! be issued for a number of problems), or 'DEBUG_TRACE' (which prints trace
!! messages for every function entered).  Also, since this is a variable, it
!! is possible to set the level only for a particular section of code, if that
!! section is giving lots of errors.

#Ifdef dynobj_h;  ! remove "Constant declared but not used" warnings
#Endif;

#Ifndef TARGET_GLULX;
#Message fatalerror "Please recompile under glulx.  Dynobj will not work under
the z-machine.";
#Endif;

#Include "debuglib.h";
#Include "dynmem.h";

! Constant declarations
Constant OBJ_LEN	34;
Constant PROP_LEN	10;

Constant ATTR_OFFSET    1;
Constant NEXT_OFFSET    2;
Constant NAME_OFFSET    3;
Constant PROP_OFFSET    4;
Constant PARENT_OFFSET  5;
Constant SIBLING_OFFSET 6;
Constant CHILD_OFFSET   7;

Constant DYNOBJ_DEBUG_STRING "dynobj";

! Global variables
MemController __dynobj_obj_mem with block_size OBJ_LEN;
MemController __dynobj_data_mem;
MemController __dynobj_local_mem;

! This is a linked list of classes we've made objects from
Global __dynobj_class_list_start;
Global __dynobj_class_list_end;

! This is a node in the list
Class __dynobj_class_list_node (1)
  with prev nothing,
       next nothing,
       class_t nothing,
       num 0,
  ;

! This is a node for the node class.  If it's not static, things get ugly.
__dynobj_class_list_node __dynobj_node_1
  with class_t __dynobj_class_list_node,
       num 1,
  ;

! Functions

[ Cl__Ms
    _vararg_count obj id a b x y;

  @copy sp obj;
  @copy sp id;
  _vararg_count = _vararg_count - 2;
  switch (id) {
    create:
      Print_Trace (DYNOBJ_DEBUG_STRING, "Ms__Cl called for create.");
      x = __dynobj_build_object (obj);
      if (x == nothing) 
      {
	 Print_Warn ("Ms__Cl create", "__dynobj_build_object returned
			 nothing.");
	 rfalse;
      }
      if (x provides create)
      {
        @copy create sp;
        @copy x sp;
        y = _vararg_count + 2;
        @call CA__Pr y 0;
      }
      return x;
    recreate:
      Print_Trace (DYNOBJ_DEBUG_STRING, "Ms__Cl called for recreate.");
      @copy sp a;
      _vararg_count--;
      if (~~(a ofclass obj))
      {
        RT__Err("recreate", a, -obj);
        rfalse;
      }
      if (a provides destroy)
      {
        a.destroy();
      }
      Copy__Primitive(a, __dynobj_child(obj));
      if (a provides create)
      {
        @copy create sp;
        @copy a sp;
        y = _vararg_count + 2;
        @call CA__Pr y 0;
      }
      rfalse;
    destroy:
      Print_Trace (DYNOBJ_DEBUG_STRING, "Ms__Cl called for destroy.");
      @copy sp a;
      _vararg_count--;
      if (~~(a ofclass obj))
      {
        RT__Err("destroy", a, -obj);
        rfalse;
      }
      if (~~(__dynobj_object_check (a)))
      {
	 Print_Warn ("Ms__Cl destroy", "Attempt to destroy a non-dynamic
			 object.");
	 rfalse;
      }
      if (a provides destroy)
      {
        a.destroy();
      }
      __dynobj_destroy_object (a);
      rfalse;
    remaining:
      Print_Trace (DYNOBJ_DEBUG_STRING, "Ms__Cl called for remaining.");
      return 1;
    copy:
      Print_Trace (DYNOBJ_DEBUG_STRING, "Ms__Cl called for copy.");
      @copy sp a;
      @copy sp b;
      _vararg_count = _vararg_count - 2;
      if (~~(a ofclass obj))
      {
        RT__Err("copy", a, -obj);
        rfalse;
      }
      if (~~(b ofclass obj))
      {
        RT__Err("copy", b, -obj);
        rfalse;
      }
      Copy__Primitive(a, b);
      rfalse;
  }
];

[ __dynobj_child parent
    child;

  @aload parent CHILD_OFFSET child;

  return child;
];

[ __dynobj_build_object class
    obj_inst i obj prop_source prop_addr name_base name_addr next_obj num;

  Print_Trace (DYNOBJ_DEBUG_STRING, "Building new object.");

  num = __dynobj_class_list_check (class);

  @aload class CHILD_OFFSET obj_inst;

  ! get next_obj
  @aload obj_inst NEXT_OFFSET next_obj;
  
  ! First, allocate some space for an object
  obj = __dynobj_obj_mem.alloc_ptr (32);
  if (obj == NULL)
  {
     Print_Warn ("__dynobj_build_object", "alloc_ptr returned NULL.");
     rfalse;
  }

  ! Now set the object fields
  ! glulx type
  obj->0 = $70;		! type $70 is a glulx object

  ! attributes - copy from class
  for (i = ATTR_OFFSET: i <= NUM_ATTR_BYTES: i++)
    obj->i = obj_inst->i;

  ! set the next object in the linked list
  @astore obj NEXT_OFFSET next_obj;

  ! set the name
  @aload obj_inst NAME_OFFSET name_base;
  name_addr = __dynobj_create_name (name_base, num);
  if (name_addr == NULL)
  {
     Print_Warn ("__dynobj_build_object", "alloc_ptr returned NULL.");
     rfalse;
  }
  @astore obj NAME_OFFSET name_addr;

  ! copy the property table, too.
  @aload obj_inst PROP_OFFSET prop_source;
  prop_addr = __dynobj_copy_prop_table (prop_source);
  if (name_addr == NULL)
  {
     Print_Warn ("__dynobj_build_object", "alloc_ptr returned NULL.");
     rfalse;
  }
  @astore obj PROP_OFFSET prop_addr;

  ! not part of the object tree
  @astore obj PARENT_OFFSET 0;
  @astore obj SIBLING_OFFSET 0;
  @astore obj CHILD_OFFSET 0;
  
  ! now add the new object to the list, right after prev_obj
  @astore obj_inst NEXT_OFFSET obj;

  return obj;
];

[ __dynobj_create_name base num
    addr len copy i num_2 num_len;

  Print_Trace (DYNOBJ_DEBUG_STRING, "Creating object name.");

  ! Calculating length
  len = base.print_to_array (addr, 4);
  num_2 = num;
  num_len = 1;
  do
  {
     num_2 = num_2 / 10;
     num_len++;
  } until (num_2 < 10);


  ! Making a copy in a buffer
  copy = __dynobj_local_mem.alloc_ptr (len + WORDSIZE);
  if (copy == NULL)
  {
     Print_Warn ("__dynobj_create_name", "alloc returned NULL.");
     return NULL;
  }
  base.print_to_array (copy, len + WORDSIZE);
  addr = __dynobj_data_mem.alloc_ptr (len + num_len + 4);
  if (addr == NULL)
  {
     Print_Warn ("__dynobj_create_name", "alloc returned NULL.");
     return NULL;
  }

  ! Creating string
  addr->0 = $E0;
  for (i = 0: i < len: i++)
    (addr + 1)->i = copy->(i + WORDSIZE);
  PrintAnyToArray (addr + len, num_len, DecimalNumber, num);
  (addr + 1)->(len + 4) = 0;

  ! Freeing copy
  __dynobj_local_mem.free_ptr (copy);

  return addr;
];

[ __dynobj_copy_prop_table source
    addr num i j s_tmp a_tmp a_data p_id p_len p_data p_flags;

  Print_Trace (DYNOBJ_DEBUG_STRING, "Creating object property table.");

  @aload source 0 num;
  addr = __dynobj_data_mem.alloc_ptr (WORDSIZE + PROP_LEN * num);
  if (addr == NULL)
  {
     Print_Warn ("__dynobj_copy_prop_table", "alloc returned NULL.");
     return NULL;
  }
  @astore addr 0 num;

  for (i = 0: i < num: i++)
  {
     s_tmp = source + i * PROP_LEN + WORDSIZE;
     a_tmp = addr + i * PROP_LEN + WORDSIZE;

     @aloads s_tmp 0 p_id;
     @astores a_tmp 0 p_id;
     s_tmp = s_tmp + 2;
     a_tmp = a_tmp + 2;

     @aloads s_tmp 0 p_len;
     @astores a_tmp 0 p_len;
     s_tmp = s_tmp + 2;
     a_tmp = a_tmp + 2;

     @aload s_tmp 0 p_data;
     a_data = __dynobj_data_mem.alloc_ptr (p_len * WORDSIZE);
     if (a_data == NULL)
     {
        Print_Warn ("__dynobj_copy_prop_table", "alloc returned NULL.");
        return NULL;
     }
     for (j = 0: j < p_len: j++)
     {
        a_data-->j = p_data-->j;
     }
     @astore a_tmp 0 a_data;
     s_tmp = s_tmp + 4;
     a_tmp = a_tmp + 4;

     @aloads s_tmp 0 p_flags;
     @astores a_tmp 0 p_flags;
  }

  return addr;
];

[ __dynobj_destroy_object obj
    name_addr prop_addr num_props i tmp p_data;

  Print_Trace (DYNOBJ_DEBUG_STRING, "Destroying object.");

  ! first, free the name
  @aload obj NAME_OFFSET name_addr;
  __dynobj_data_mem.free_ptr (name_addr);

  ! then, free the props
  @aload obj PROP_OFFSET prop_addr;
  num_props = prop_addr-->0;
  for (i = 0: i < num_props: i++)
  {
     tmp = prop_addr + i * PROP_LEN + WORDSIZE + 4;
     @aload tmp 0 p_data;
     __dynobj_data_mem.free_ptr (p_data);
  }
  
  ! set the object type to 0, just in case any accesses are attempted.
  obj->0 = $00;

  ! finally, free the object
  __dynobj_obj_mem.free_ptr (obj);

  rtrue;
];

[ __dynobj_class_list_init
    ;
    
  Print_Trace (DYNOBJ_DEBUG_STRING, "Initializing class list.");
  
  ! Set up first node, for __dynobj_class_list_node, to avoid infinite
  !	recursion
  
  __dynobj_class_list_start = __dynobj_node_1;
  __dynobj_class_list_end = __dynobj_node_1;

  rtrue;
];

[ __dynobj_class_list_show
    node;
  
  Print_Trace (DYNOBJ_DEBUG_STRING, "Displaying class list.");
  
  if (__dynobj_class_list_start == 0) __dynobj_class_list_init ();
    
  for (node = __dynobj_class_list_start: node ~= nothing: node = node.next)
  {
     print "Node ", (name) node, "^";
     print "  ", (name) node, ".prev = ", (name) (node.prev), "^";
     print "  ",(name) node, ".next = ", (name) (node.next), "^";
     print "  ",(name) node, ".class_t = ", (name) (node.class_t), "^";
     print "  ",(name) node, ".num = ", (name) (node.num), "^";
     new_line;
  }

  rtrue;
];

[ __dynobj_class_list_check class
    node found;

  Print_Trace (DYNOBJ_DEBUG_STRING, "Searching for class in class list.");
    
  if (__dynobj_class_list_start == 0) __dynobj_class_list_init ();
    
  ! check list for class
  for (node = __dynobj_class_list_start, found = false: 
       node ~= nothing: 
       node = node.next)
  {
     if (node.class_t == class) 
     {
	found = true;
	break;
     }
  }
  
  ! if not present, add class
  if (found ~= true)
     node = __dynobj_class_list_add (class);
  
  ! increment node.num
  node.num++;
  
  ! return num
  return node.num;
];

[ __dynobj_class_list_add class
    node;

  Print_Trace (DYNOBJ_DEBUG_STRING, "Adding a class to the class list.");

  node = __dynobj_class_list_node.create();
  node.prev = __dynobj_class_list_end;
  node.class_t = class;
  node.num = 0;
 
  if (__dynobj_class_list_end ~= nothing)
  {
     __dynobj_class_list_end.next = node;
     __dynobj_class_list_end = node;
  }

  return node;
];

[ __dynobj_object_check obj
    ;

  Print_Trace (DYNOBJ_DEBUG_STRING, "Checking wether an object is from dynobj.");

  if (~~(__dynobj_obj_mem.check_ptr (obj)))
  {
     Print_Warn ("__dynobj_object_check", "Object was not dynamically
		     allocated.");
     rfalse;
  }
  
  rtrue;
];

#Ifndef PrintAnything;
! This is copied straight from parserm.h

! This is a trivial function which just prints a number, in decimal
! digits. It may be useful as a stub to pass to PrintAnything.
[ DecimalNumber num;
    print num;
];

! This somewhat obfuscated function will print anything.
! It handles strings, functions (with optional arguments), objects,
! object properties (with optional arguments), and dictionary words.
! It does *not* handle plain integers, but you can use
! DecimalNumber or EnglishNumber to handle that case.
!
! Calling:                           Is equivalent to:
! -------                            ----------------
! PrintAnything()                    <nothing printed>
! PrintAnything(0)                   <nothing printed>
! PrintAnything("string");           print (string) "string";
! PrintAnything('word')              print (address) 'word';
! PrintAnything(obj)                 print (name) obj;
! PrintAnything(obj, prop)           obj.prop();
! PrintAnything(obj, prop, args...)  obj.prop(args...);
! PrintAnything(func)                func();
! PrintAnything(func, args...)       func(args...);
!
[ PrintAnything _vararg_count obj mclass;
    if (_vararg_count == 0)
        return;
    @copy sp obj;
    _vararg_count--;
    if (obj == 0)
        return;

    if (obj->0 == $60) {
        ! Dictionary word. Metaclass() can't catch this case, so we do
        ! it manually.
        print (address) obj;
        return;
    }

    mclass = metaclass(obj);
    switch (mclass) {
        nothing:
            return;
        String:
            print (string) obj;
            return;
        Routine:
            ! Call the function with all the arguments which are already
            ! on the stack.
            @call obj _vararg_count 0;
            return;
        Object:
            if (_vararg_count == 0) {
                print (name) obj;
            }
            else {
                ! Push the object back onto the stack, and call the
                ! veneer routine that handles obj.prop() calls.
                @copy obj sp;
                _vararg_count++;
                @call CA__Pr _vararg_count 0;
            }
            return;
    }
];

! This does the same as PrintAnything, but the output is sent to a
! byte array in memory. The first two arguments must be the array
! address and length; the following arguments are interpreted as
! for PrintAnything. The return value is the number of characters
! output.
! If the output is longer than the array length given, the extra
! characters are discarded, so the array does not overflow.
! (However, the return value is the total length of the output,
! including discarded characters.)

[ PrintAnyToArray _vararg_count arr arrlen str oldstr len;

   @copy sp arr;
   @copy sp arrlen;
   _vararg_count = _vararg_count - 2;

   oldstr = glk_stream_get_current ();
   str = glk_stream_open_memory (arr, arrlen, 1, 0);
   if (str == 0)
       return 0;

   glk_stream_set_current (str);

   @call PrintAnything _vararg_count 0;

   glk_stream_set_current (oldstr);
   @copy $ffffffff sp;
   @copy str sp;
   @glk $0044 2 0; ! stream_close
   @copy sp len;
   @copy sp 0;

   return len;
];
#Endif;

#Endif;

