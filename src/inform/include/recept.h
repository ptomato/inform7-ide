! --------------------------------------------------------------------
!
! RECEPTACLES
! ===========
! (containers with defined capacities)
! for use with INFORM 6.x, (c) 1999 Graham Nelson
! Version 1.0 (2002-July-06)
!
! Written and copyright (c) 2002 Peer Schaefer. All rights reserved.
! NO WARRANTY! PROVIDED "AS IS". USE ON YOUR OWN RISK!
! You may distribute this file unaltered and without profit. You may
! use this file for your own programs/stories (even commercial
! products) freely. It would be nice if you give me some credits, but
! that's not required.
!
! Bug reports, suggestions, comments, questions or congratulations to:
! peerschaefer@gmx.net
! (or, if that fails, to: peer@wolldingwacht.de)
!
! The latest version is available at:
! http://www.wolldingwacht.de/if/recept.html
!
! --------------------------------------------------------------------
!
!
! REFERENCE
! =========
! This library contribution implements a new class of objects called
! "receptacles". Receptacles are containers (or supporters) that are
! aware of weight, volume and size. Every time the player tries to put
! an object into such a receptacle the receptacle checks whether its
! remaining storage-capacity is sufficient or not.
!
! To define its weight, volume and size every object in the game can
! (but hasn't to) provide the properties
!   - weight
!   - volume
!   - size   ("size" represents the maximum length on any axis, e.g.
!             the length of an arrow or of a staff. It's obvious that
!             the volume of a quiver is "used up" when you store many
!             arrows in it, but the length of the quiver is not "used
!             up" by the length of the stored arrows: it only
!             indicates whether an arrow is too long for the quiver
!             or not.)
!
! Each of these three properties can be a numerical value or a
! routine that returns a numerical value. If one of these properties
! is missing or has the value 0 (zero), the respective dimension of
! the object is 0 (that means that the object has no or only
! neglectable weight, volume or size).
!
! Please notice: Weight, volume and size are measured in abstract
!                "units" (plain numbers). Whether one "unit" of e.g.
!                weight is a grain, a british pound, a kilogram, an
!                US-ton or 1 sun-mass is completely up to you and
!                your program.
!
! To create a container which automatically checks weight, volume and
! size use the new "receptacle"-class. Containers of this class can
! provide three new properties:
!   - capacity_weight
!   - capacity_volume
!   - capacity_size
! Each of these three properties can be a numerical value or a routine
! that returns a numerical value. If one of these properties is missing
! or has the value INFINITE_CAPACITY (a predefined constant), the
! respective capacity of the receptacle is infinite.
!
! Of course containers/receptacles can also provide weight, volume and
! size (since they are not only containers/receptacles but also objects
! that can be stored inside other containers/receptacles).
!
! For clarification:
!  * The player can store multiple objects in a receptacle, as long as
!    their total-weight does not exceed the capacity_weight of the
!    receptacle.
!  * The player can store multiple objects in a receptacle, as long as
!    their total-volume does not exceed the capacity_volume of the
!    receptacle.
!  * In other words: capacity_weight and capacity_volume are "used up"
!    by objects stored inside the receptacle.
!  * capacity_size is a different story: it's not "used up" but
!    checked seperately for each object.
!
! As a little extra a receptacle can also provide the new property
! capacity_number. That property can be a number or a routine that
! returns a number. It indicates how many objects (maximum) can be
! stored inside that receptacle. Any objects beyond this maximum are
! rejected.
!
!
! Some technical details:
! -----------------------
!  * Calculating the weight of a container (or supporter) is a
!    recursive process: The weights of all (immediate) child-objects
!    are added to the weight of the container itself; the total-
!    weights of these child-objects are calculated by adding up the
!    weights of THEIR child-objects; and so forth.
!  * If the weight of an object is provided as a routine, this routine
!    should return the TOTAL weight of the object (including the
!    weight of all child-objects inside, grand-child-objects etc).
!    The weights of child-objects are NOT added automatically in this
!    case. That gives you more flexibility in the creation of special
!    containers (e.g. magical bags that weigh less than their
!    contents).
!  * Volume and size of childrens are NOT added to the objects
!    volume/size (assuming that standard-containers are not flexible
!    and have fixed proportions). To override this you can provide
!    your own VOLUME and SIZE properties, which calculate volume resp.
!    size at run-time (see example below).
!  * It's totaly legal to create a container whose volume_capacity is
!    greater than it's volume (it's pretty inplausible, but this way
!    you can create magical bags, black holes and other weird things).
!
!
! DEBUGGING VERBS:
! ----------------
! When compiled in debug-mode recept.h provides three meta-verbs:
!   $weigh OBJECT           - prints the weight of the object
!   $measure OBJECT         - prints all dimensions of an object
!   $capacity OBJECT        - prints the capacities of a receptacle
!
!
!
! EXAMPLE 1:
! ----------
! Receptacle box "box" with name "box",
!        volume 10,                ! The box itself has a volume of 10
!        capacity_volume 9,        ! And it can store a volume of 9
!        has container;
!
! Object stone "stone" with name "stone",
!        volume 2;                 ! This stone has a volume of 2
!
! The lines above create a box with a volume of 10 and a capacity of 9,
! and a stone with a volume of 2. The player can put up to 4 stones in
! the box (with a total volume of 4x2=8). A fifth stone can't be stored
! in the box, since the capacity of the box is 9 and the total volume
! of 5 stones would be 10.
!
!
! EXAMPLE 2:
! ----------
! The following example creates a wooden box and a steel box:
!
! Receptacle wooden_box "wooden box"
!        with name "wooden" "box",
!        volume 10,              ! The box itself has a volume of 10
!                                ! units
!        capacity_volume 9,      ! And it can store objects up to a
!                                ! volume of 9 units
!        has container;
!
! Receptacle steel_box "steel box"
!        with name "steel" "box",
!        volume 8,               ! The box itself has a volume of 8
!                                ! units
!        capacity_volume 7,      ! And it can store objects up to a
!                                ! volume of 7 units
!        has container;
!
! You can put the steel box into the wooden box (volume 8 fits in
! capacity 9) but not the wooden box into the steel box (volume 10
! doesn't fit in capacity 7). If you put something with a volume of 2
! or greater into the wooden box, you can't put the steel box into
! it because that would require a free volume of 8 or more.
!
!
! EXAMPLE 3:
! ----------
! Volume and size of childrens are NOT added to the objects volume or
! size (assuming that containers are not flexible and have fixed
! proportions), so if you want to create a flexible bag whose volume
! grows if objects are stored inside, you should code an appropriate
! routine as the volume property:
!
! Receptacle -> bag "flexible bag"
!        with name "flexible" "bag",
!        capacity_volume 20,
!        capacity_size 5,
!        volume [ v i;
!                v = 1;                         ! Minimal volume.
!                objectloop (i in bag)          ! Add volumes of all
!                        v = v + VolumeOf (i);  ! immediate childs.
!                return v;
!        ],
!        size [ s i;
!                s = 1;                  ! Minimal size when empty.
!                objectloop (i in bag)   ! Find the largest child:
!                        if (SizeOf (i) > s) s = SizeOf (i);
!                return s;               ! (The size of the largest
!        ],                              ! immediate child determines
!        has container;                  ! the size of the bag.)
!
!
! EXAMPLE 4:
! ----------
! The weights of childrens are added automatically to the objects
! weight. You can override this by providing an own weight-routine.
! The following example creates a wonder-bag, whose total-weight is
! only half the total-weight of it's contents:
!
! Receptacle -> wonder_bag "wonder bag"
!        with name "wonder" "magic" "bag",
!        capacity_volume 100,
!        weight [ w i;
!                w = 1;                        ! Base weight of bag is 1
!                objectloop (i in wonder_bag)
!                        w = w + WeightOf (i); ! Add up weights...
!                return (w/2);                 ! ...and return 50%
!        ],
!        has container;
!
!
! CONTACT:
! --------
! Bug reports, suggestions, questions, comments and congratulations to:
! peerschaefer@gmx.net
! (or, if that fails, to: peer@wolldingwacht.de)




System_file;                      ! to avoid a warning for not using the
                                  ! __DUMMY_RECEPT class


Class __DUMMY_RECEPT              ! Defines the seven new properties without
        with                      ! using up any program-space or
                weight 0,         ! runtime-memory
                size   0,
                volume 0,
                capacity_weight 0,
                capacity_size   0,
                capacity_volume 0,
		capacity_number 0;
 

Constant INFINITE_CAPACITY -1;    ! (-1) symbolizes an infinite capacity


! --------------------------------------------------------

! The following functions calculate the weight, volume and size
! of any given object.

[ WeightOf obj w i;
        if (obj provides weight)
        {
                if (metaclass(obj.weight) == Routine)
                        return indirect (obj.weight);
                w = obj.weight;
        } else  w = 0;
        if ((obj has container) || (obj has supporter))
                objectloop (i in obj)
                        w = w + WeightOf (i);   ! Add weight of child-
                                                ! objects
        return w;
];

[ SizeOf obj;
        if (obj provides size)
        {
                if (metaclass(obj.size) == Routine)
                        return indirect (obj.size);
                return obj.size;
        };
        return 0;
];

[ VolumeOf obj;
        if (obj provides volume)
        {
                if (metaclass(obj.volume) == Routine)
                        return indirect (obj.volume);
                return obj.volume;
        };
        return 0;
];


! --------------------------------------------------------

! The following functions calculate the capacity of any given
! container or supporter.

[ CapacityWeightOf obj;
        if (obj provides capacity_weight)
        {
                if (metaclass(obj.capacity_weight) == Routine)
                        return indirect (obj.capacity_weight);
                return obj.capacity_weight;
        }
        return INFINITE_CAPACITY;        ! Unlimited weight capacity
];

[ CapacityVolumeOf obj;
        if (obj provides capacity_volume)
        {
                if (metaclass(obj.capacity_volume) == Routine)
                        return indirect (obj.capacity_volume);
                return obj.capacity_volume;
        }
        return INFINITE_CAPACITY;        ! Unlimited volume capacity
];

[ CapacitySizeOf obj;
        if (obj provides capacity_size)
        {
                if (metaclass(obj.capacity_size) == Routine)
                        return indirect (obj.capacity_size);
                return obj.capacity_size;
        }
        return INFINITE_CAPACITY;        ! Unlimited size capacity
];

[ CapacityNumberOf obj;
        if (obj provides capacity_number)
        {
                if (metaclass(obj.capacity_number) == Routine)
                        return indirect (obj.capacity_number);
                return obj.capacity_number;
        }
        return INFINITE_CAPACITY;        ! Unlimited number of objects
];


! --------------------------------------------------------


Class Receptacle
  with
        before [ s i;

        Receive:
        ! Check weight:
                if ( CapacityWeightOf (self) ~= INFINITE_CAPACITY )
                                                       ! not unlimited
                                                       ! capacity
                {
                        if ( WeightOf(noun) > CapacityWeightOf (self) )
                                print_ret (The) noun, " is too heavy for ", (the) self, ".";
                        s = 0;
                        objectloop (i in self)         ! calculate the
                                s = s + WeightOf(i);   ! weight of all
                                                       ! contents
                        if ( (s + WeightOf(noun)) > CapacityWeightOf (self) )
                                print_ret (The) self, " has reached it's capacity.";
                };

        ! Check volume:
                if ( CapacityVolumeOf (self) ~= INFINITE_CAPACITY )
                                                       ! not unlimited
                                                       ! capacity
                {
                        if ( VolumeOf(noun) > CapacityVolumeOf (self) )
                                print_ret (The) noun, " does not fit into ", (the) self, ".";
                        s = 0;
                        objectloop (i in self)         ! calculate the
                                s = s + VolumeOf(i);   ! volume of all
                                                       ! contents
                        if ( (s + VolumeOf(noun)) > CapacityVolumeOf (self) )
                                print_ret (The) self, " has reached it's capacity.";
                };

        ! Check size:
                if ( CapacitySizeOf (self) ~= INFINITE_CAPACITY )
                                                       ! not unlimited
                                                       ! capacity
                {
                        if ( SizeOf(noun) > CapacitySizeOf (self) )
                                print_ret (The) noun, " is too large for ", (the) self, ".";
                };

	! Check number:
                if ( CapacityNumberOf (self) ~= INFINITE_CAPACITY )
                                                       ! not unlimited
                                                       ! capacity
                {
                        s = 0;
			objectloop (i in self)
				s++;
			if ( (s+1) > CapacityNumberOf (self) )
                                print_ret (The) self, " contains too many objects.";
                };

	        rfalse;
        ];



#ifdef DEBUG;

Verb meta       '$weigh'          * noun                -> MetaWeigh;
Verb meta       '$measure'        * noun                -> MetaMeasure;
Verb meta       '$capacity'       * noun                -> MetaCapacity;

[ MetaWeighSub;
                print_ret (The) noun, " weighs ",
                WeightOf (noun), " units.";
];
[ MetaMeasureSub;
                print (The) noun, ":^";
                print "Weight: ", WeightOf(noun), " units^";
                print "Size: ",   SizeOf(noun),   " units^";
                print "Volume: ", VolumeOf(noun), " units^";
                rtrue;
];
[ MetaCapacitySub;
                print (The) noun, ":^";

                print "Capacity (weight): ";
                if (CapacityWeightOf(noun) == INFINITE_CAPACITY)
                        print "infinite^";
                else
                        print CapacityWeightOf(noun), " units^";

                print "Capacity (size): ";
                if (CapacitySizeOf(noun) == INFINITE_CAPACITY)
                        print "infinite^";
                else
                        print CapacitySizeOf(noun), " units^";

                print "Capacity (volume): ";
                if (CapacityVolumeOf(noun) == INFINITE_CAPACITY)
                        print "infinite^";
                else
                        print CapacityVolumeOf(noun), " units^";

                print "Capacity (number of objects): ";
                if (CapacityNumberOf(noun) == INFINITE_CAPACITY)
                        print "infinite^";
                else
                        print CapacityNumberOf(noun), "^";

                rtrue;
];

#endif;

! End
