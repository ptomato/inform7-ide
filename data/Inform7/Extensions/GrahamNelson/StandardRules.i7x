Version 2/080430 of the Standard Rules by Graham Nelson begins here.

"The Standard Rules, included in every project, define the basic framework
of kinds, actions and phrases which make Inform what it is."

Document ACTIONS at doc96.
Document ACTIVITIES at doc273.
Document ARSUMMARY at doc182.
Document EXTACTIVITIES at doc277.
Document HEADINGS at doc15.
Document KINDS at doc31.
Document KINDSVALUE at doc61.
Document LCARDS at doc368.
Document NEWRULEBOOKS at doc318.
Document PHRASES at doc160.
Document ROOMPLAYBEGINS at doc134.
Document SCENESINTRO at doc150.
Document act_all at doc306.
Document act_amuse at doc309.
Document act_banner at doc307.
Document act_clarify at doc300.
Document act_cnlo at doc297.
Document act_con at doc281.
Document act_csl at doc293.
Document act_darkdesc at doc292.
Document act_darkname at doc291.
Document act_details at doc287.
Document act_ds at doc299.
Document act_gt at doc286.
Document act_implicitly at doc304.
Document act_lc at doc285.
Document act_lni at doc295.
Document act_nowdark at doc289.
Document act_nowlight at doc290.
Document act_obit at doc308.
Document act_pan at doc284.
Document act_parsererror at doc305.
Document act_pld at doc296.
Document act_plp at doc298.
Document act_pn at doc282.
Document act_ppn at doc283.
Document act_reading at doc303.
Document act_smn at doc302.
Document act_startvm at doc310.
Document act_toodark at doc288.
Document act_which at doc301.
Document act_wpa at doc294.
Document kind_backdrop at doc35.
Document kind_device at doc40.
Document kind_door at doc38.
Document kind_person at doc43.
Document kind_player's at doc47.
Document kind_time at doc139.
Document kind_vehicle at doc42.
Document ph_abide at doc326.
Document ph_addlist at doc344.
Document ph_allow at doc196.
Document ph_awardp at doc135.
Document ph_beginact at doc279.
Document ph_bestroute at doc92.
Document ph_blankout at doc242.
Document ph_boxed at doc80.
Document ph_break at doc170.
Document ph_carryout at doc277.
Document ph_casing at doc334.
Document ph_change at doc115.
Document ph_changep at doc123.
Document ph_changev at doc119.
Document ph_changex at doc119.
Document ph_chooserow at doc237.
Document ph_consents at doc163.
Document ph_consider at doc326.
Document ph_curract at doc200.
Document ph_displayf at doc356.
Document ph_durations at doc141.
Document ph_end at doc137.
Document ph_exactm at doc335.
Document ph_extcr at doc403.
Document ph_filetables at doc362.
Document ph_follow at doc318.
Document ph_frontside at doc38.
Document ph_future at doc144.
Document ph_goingon at doc274.
Document ph_group at doc285.
Document ph_happening at doc153.
Document ph_holder at doc130.
Document ph_if at doc164.
Document ph_ignore at doc325.
Document ph_increase at doc218.
Document ph_indarkness at doc163.
Document ph_involves at doc200.
Document ph_islistedin at doc343.
Document ph_lbreak at doc74.
Document ph_let at doc174.
Document ph_list at doc173.
Document ph_listd at doc345.
Document ph_locof at doc51.
Document ph_minspart at doc143.
Document ph_mobjl at doc345.
Document ph_move at doc121.
Document ph_movebd at doc122.
Document ph_nearest at doc141.
Document ph_next at doc170.
Document ph_nextstep at doc212.
Document ph_nothing at doc161.
Document ph_now at doc125.
Document ph_numblank at doc242.
Document ph_numel at doc348.
Document ph_numof at doc82.
Document ph_numofc at doc333.
Document ph_numrows at doc234.
Document ph_omit at doc282.
Document ph_otherwise at doc167.
Document ph_outcomer at doc322.
Document ph_playsf at doc358.
Document ph_plus at doc218.
Document ph_randobj at doc132.
Document ph_random at doc131.
Document ph_readytr at doc364.
Document ph_regexp at doc336.
Document ph_remlist at doc344.
Document ph_remove at doc124.
Document ph_repeat at doc168.
Document ph_replace at doc338.
Document ph_requires at doc187.
Document ph_result at doc175.
Document ph_resultr at doc323.
Document ph_revl at doc347.
Document ph_roomdirof at doc92.
Document ph_rotl at doc347.
Document ph_runthrough at doc169.
Document ph_say at doc76.
Document ph_saya at doc69.
Document ph_sayif at doc72.
Document ph_saylv at doc342.
Document ph_sayn at doc70.
Document ph_sayoneof at doc73.
Document ph_scope at doc299.
Document ph_sort at doc243.
Document ph_sortl at doc347.
Document ph_stop at doc171.
Document ph_stopac at doc98.
Document ph_succeeds at doc321.
Document ph_surrounds at doc117.
Document ph_tabrepeat at doc238.
Document ph_testuo at doc417.
Document ph_thereis at doc239.
Document ph_timeshift at doc142.
Document ph_total at doc230.
Document ph_truncate at doc349.
Document ph_try at doc99.
Document ph_tsince at doc152.
Document ph_types at doc75.
Document ph_updatebp at doc122.
Document ph_while at doc165.
Document ph_wornot at doc163.
Document ph_writef at doc363.
Document ph_yes at doc175.
Document rules_after at doc100.
Document rules_before at doc98.
Document rules_dtpm at doc269.
Document rules_et at doc138.
Document rules_fail at doc185.
Document rules_instead at doc97.
Document rules_internal at doc328.
Document rules_per at doc184.
Document rules_proc at doc324.
Document rules_ri at doc196.
Document rules_wpb at doc134.
Document rules_wpe at doc137.
Document var_command at doc303.
Document var_location at doc64.
Document var_noun at doc105.
Document var_particular at doc281.
Document var_person_asked at doc183.
Document var_person_reaching at doc198.
Document var_prompt at doc116.
Document var_reason at doc185.
Document var_score at doc135.
Document var_sl at doc117.
Document var_time at doc139.
Document var_understood at doc260.
Document visibility at doc199.

Use MAX_ARRAYS of 1500.
Use MAX_CLASSES of 200.
Use MAX_VERBS of 300.
Use MAX_LABELS of 10000.
Use MAX_ZCODE_SIZE of 50000.
Use MAX_STATIC_DATA of 120000.
Use MAX_PROP_TABLE_SIZE of 200000.
Use MAX_INDIV_PROP_TABLE_SIZE of 20000.
Use MAX_STACK_SIZE of 65536.
Use MAX_SYMBOLS of 20000.
Use MAX_EXPRESSION_NODES of 256.

Use dynamic memory allocation of at least 8192 translates as
	(- Constant DynamicMemoryAllocation = {N}; -).
Use maximum indexed text length of at least 1024 translates as
	(- Constant IT_MemoryBufferSize = {N}+3; -).

Use dynamic memory allocation of at least 8192.

Use American dialect translates as (- Constant DIALECT_US; -).
Use the serial comma translates as (- Constant SERIAL_COMMA; -).
Use full-length room descriptions translates as (- Constant I7_LOOKMODE = 2; -).
Use abbreviated room descriptions translates as (- Constant I7_LOOKMODE = 3; -).
Use memory economy translates as (- Constant MEMORY_ECONOMY; -).
Use authorial modesty translates as (- Constant AUTHORIAL_MODESTY; -).
Use no scoring translates as (- Constant NO_SCORING; -).
Use command line echoing translates as (- Constant ECHO_COMMANDS; -).
Use undo prevention translates as (- Constant PREVENT_UNDO; -).
Use predictable randomisation translates as (- Constant FIX_RNG; -).
Use fast route-finding translates as (- Constant FAST_ROUTE_FINDING; -).
Use slow route-finding translates as (- Constant SLOW_ROUTE_FINDING; -).


Part SR1 - The Physical World Model

Section SR1/0 - Language

The verb to be in implies the reversed containment relation.
The verb to be inside implies the reversed containment relation.
The verb to be within implies the reversed containment relation.
The verb to be held in implies the reversed containment relation.
The verb to be held inside implies the reversed containment relation.

The verb to contain (he contains, they contain, he contained, it is contained,
he is containing) implies the containment relation.
The verb to be contained in implies the reversed containment relation.

The verb to be on implies the reversed support relation.
The verb to be on top of implies the reversed support relation.

The verb to support (he supports, they support, he supported, it is supported,
he is supporting) implies the support relation.
The verb to be supported on implies the reversed support relation.

The verb to incorporate (he incorporates, they incorporate, he incorporated,
it is incorporated, he is incorporating) implies the incorporation relation.
The verb to be part of implies the reversed incorporation relation.
The verb to be a part of implies the reversed incorporation relation.
The verb to be parts of implies the reversed incorporation relation.

The verb to enclose (he encloses, they enclose, he enclosed, it is enclosed,
he is enclosing) implies the enclosure relation.

The verb to carry (he carries, they carry, he carried, it is carried
(adjectival), he is carrying) implies the carrying relation.
The verb to hold (he holds, they hold, he held, it is held (adjectival),
he is holding) implies the holding relation.
The verb to wear (he wears, they wear, he wore, it is worn (adjectival),
he is wearing) implies the wearing relation.

The verb to be able to see (he is seen) implies the visibility relation.
The verb to be able to touch (he is touched) implies the touchability relation.

Definition: Something is visible rather than invisible if the player can see it.
Definition: Something is touchable rather than untouchable if the player can touch it.

The verb to conceal (he conceals, they conceal, he concealed, it is concealed,
he is concealing) implies the concealment relation.
Definition: Something is concealed rather than unconcealed if the holder of it conceals it.

Definition: Something is on-stage rather than off-stage if I6 routine "OnStage"
	says so (it is indirectly in one of the rooms).

The verb to be greater than implies the numerically-greater-than relation.
The verb to be less than implies the numerically-less-than relation.
The verb to be at least implies the numerically-greater-than-or-equal-to relation.
The verb to be at most implies the numerically-less-than-or-equal-to relation.

Section SR1/1 - Primitive Kinds

A room is a kind. [1]
A thing is a kind. [2]
A direction is a kind. [3]
A door is a kind of thing. [4]
A container is a kind of thing. [5]
A supporter is a kind of thing. [6]
A backdrop is a kind of thing. [7]
The plural of person is people. The plural of person is persons.
A person is a kind of thing. [8]
A region is a kind. [9]

Section SR1/2 - Rooms

The specification of room is "Represents geographical locations, both indoor
and outdoor, which are not necessarily areas in a building. A player in one
room is mostly unable to sense, or interact with, anything in a different room.
Rooms are arranged in a map."

A room can be privately-named or publically-named. A room is usually publically-named.
A room can be lighted or dark. A room is usually lighted.
A room can be visited or unvisited. A room is usually unvisited.

A room has a text called description.
A room has a text called printed name.

A room has an object called map region. The map region of a room is usually nothing.

The verb to be adjacent to implies the reversed adjacency relation.
Definition: A room is adjacent if it is adjacent to the location.

The verb to be regionally in implies the reversed regional-containment relation.

Include (-
!	with n_to 0, ne_to 0, e_to 0, se_to 0, s_to 0, sw_to 0, w_to 0, nw_to 0,
!	u_to 0, d_to 0, in_to 0, out_to 0,
-) when defining a room.

Section SR1/3 - Things

The specification of thing is "Represents anything interactive in the model
world that is not a room. People, pieces of scenery, furniture, doors and
mislaid umbrellas might all be examples, and so might more surprising things
like the sound of birdsong or a shaft of sunlight."

A thing can be lit or unlit. A thing is usually unlit.
A thing can be edible or inedible. A thing is usually inedible.
A thing can be fixed in place or portable. A thing is usually portable.
A thing can be scenery.
A thing can be wearable.
A thing can be pushable between rooms.

A thing can be handled.
A thing can be initially carried.

A thing can be privately-named or publically-named. A thing is usually publically-named.
A thing can be plural-named or singular-named. A thing is usually singular-named.
A thing can be proper-named or improper-named. A thing is usually improper-named.
A thing can be described or undescribed. A thing is usually described.
A thing can be marked for listing or unmarked for listing. A thing is usually
unmarked for listing.
A thing can be mentioned or unmentioned. A thing is usually mentioned.

A thing has a text called an indefinite article.
A thing has a text called a description.
A thing has a text called an initial appearance.
A thing has a text called printed name.
A thing has a text called a printed plural name.

Include (-
	with component_parent nothing, component_sibling nothing, component_child nothing,
-) when defining a thing.

Scenery is usually fixed in place. [An implication.]

Section SR1/4 - Directions

The specification of direction is "Represents a direction of movement, such
as northeast or down."

A direction can be privately-named or publically-named. A direction is usually
publically-named.
A direction can be marked for listing or unmarked for listing. A direction is
usually unmarked for listing.

A direction has a direction called an opposite.

Include (- class CompassDirection, -) when defining a direction.

The north is a direction.
The northeast is a direction.
The northwest is a direction.
The south is a direction.
The southeast is a direction.
The southwest is a direction.
The east is a direction.
The west is a direction.
The up is a direction.
The down is a direction.
The inside is a direction.
The outside is a direction.

The north has opposite south. Understand "n" as north.
The northeast has opposite southwest. Understand "ne" as northeast.
The northwest has opposite southeast. Understand "nw" as northwest.
The south has opposite north. Understand "s" as south.
The southeast has opposite northwest. Understand "se" as southeast.
The southwest has opposite northeast. Understand "sw" as southwest.
The east has opposite west. Understand "e" as east.
The west has opposite east. Understand "w" as west.
Up has opposite down. Understand "u" as up.
Down has opposite up. Understand "d" as down.
Inside has opposite outside. Understand "in" as inside.
Outside has opposite inside. Understand "out" as outside.

The inside object translates into I6 as "in_obj".
The outside object translates into I6 as "out_obj".

The verb to be above implies the mapping-up relation.
The verb to be mapped above implies the mapping-up relation.
The verb to be below implies the mapping-down relation.
The verb to be mapped below implies the mapping-down relation.

Section SR1/5 - Doors

The specification of door is "Represents a conduit joining two rooms, most
often a door or gate but sometimes a plank bridge, a slide or a hatchway.
Usually visible and operable from both sides (for instance if you write
'The blue door is east of the Ballroom and west of the Garden.'), but
sometimes only one-way (for instance if you write 'East of the Ballroom is
the long slide. Through the long slide is the cellar.')."

A door is always fixed in place.
A door is never pushable between rooms.
Include (- has door, -) when defining a door.

A door has an object called an other side.
Leading-through relates one room (called the other side) to various doors.
The verb to be through implies the leading-through relation.

Section SR1/6 - Containers

The specification of container is "Represents something into which portable
things can be put, such as a teachest or a handbag. Something with a really
large immobile interior, such as the Albert Hall, had better be a room
instead."

A container can be enterable.
A container can be opaque or transparent. A container is usually opaque.
A container has a number called carrying capacity.
The carrying capacity of a container is usually 100.

Include (- has container, -) when defining a container.

Section SR1/7 - Supporters

The specification of supporter is "Represents a surface on which things can be
placed, such as a table."

A supporter can be enterable.
A supporter has a number called carrying capacity.
The carrying capacity of a supporter is usually 100.

A supporter is usually fixed in place.

Include (-
	has transparent supporter
-) when defining a supporter.

Section SR1/8 - Openability

A door can be open or closed. A door is usually closed.
A door can be openable or unopenable. A door is usually openable.

A container can be open or closed. A container is usually open.
A container can be openable or unopenable. A container is usually unopenable.

Section SR1/9 - Lockability

A door can be lockable. A door is usually not lockable.
A door can be locked or unlocked. A door is usually unlocked.
A door has an object called a matching key.
A locked door is usually lockable. [An implication.]
A locked door is usually closed. [An implication.]
A lockable door is usually openable. [An implication.]

A container can be lockable. A container is usually not lockable.
A container can be locked or unlocked. A container is usually unlocked.
A container has an object called a matching key.
A locked container is usually lockable. [An implication.]
A locked container is usually closed. [An implication.]
A lockable container is usually openable. [An implication.]

Lock-fitting relates one thing (called the matching key) to various things.
The verb to unlock (it unlocks, they unlock, it unlocked, it is unlocked)
implies the lock-fitting relation.

Section SR1/10 - Backdrops

The specification of backdrop is "Represents an aspect of the landscape
or architecture which extends across more than one room: for instance,
a stream, the sky or a long carpet."

A backdrop is usually scenery.
A backdrop is always fixed in place.
A backdrop is never pushable between rooms.

Section SR1/11 - People

The specification of person is "Despite the name, not necessarily
a human being, but anything animate enough to envisage having a
conversation with, or bartering with."

A person can be female or male. A person is usually male.
A person can be neuter. A person is usually not neuter.

A person has a number called carrying capacity.
The carrying capacity of a person is usually 100.

Include (-
	has transparent animate
	with before NULL, ! number 0,
-) when defining a person.

The yourself is an undescribed person. The yourself is proper-named.

The description of yourself is usually "As good-looking as ever."

The yourself object translates into I6 as "selfobj".
Include (-
	has proper,
	with saved_short_name "yourself",
 -) when defining yourself.

Section SR1/12 - Animals, men and women

The plural of man is men. The plural of woman is women.

A man is a kind of person.
The specification of man is "Represents a man or boy."
A man is always male. A man is never neuter.

A woman is a kind of person.
The specification of woman is "Represents a woman or girl."
A woman is always female. A woman is never neuter.

An animal is a kind of person.

The specification of animal is "Represents an animal, or at any rate a
non-human living creature reasonably large and possible to interact with: a
giant Venus fly-trap might qualify, but not a patch of lichen."

Section SR1/13 - Devices

A device is a kind of thing.

A device can be switched on or switched off. A device is usually switched off.
Include (- has switchable, -) when defining a device.

The specification of device is "Represents a machine or contrivance of some
kind which can be switched on or off."

Section SR1/14 - Vehicles

A vehicle is a kind of container.

The specification of vehicle is "Represents a container large enough for
a person to enter, and which can then move between rooms at the driver's
instruction. (If a supporter is needed instead, try the extension
Rideable Vehicles by Graham Nelson.)"

A vehicle is always enterable.

A vehicle is usually not portable.

Section SR1/15 - Player's holdall

A player's holdall is a kind of container.

The specification of player's holdall is "Represents a container which the
player can carry around as a sort of rucksack, into which spare items are
automatically stowed away."

A player's holdall is always portable.
A player's holdall is usually openable.

Include (-
	Constant RUCKSACK_CLASS = (+ player's holdall +);
-) after "Definitions.i6t".

Section SR1/16 - Inform 6 equivalents

The wearable property translates into I6 as "clothing".
The undescribed property translates into I6 as "concealed".
The edible property translates into I6 as "edible".
The enterable property translates into I6 as "enterable".
The female property translates into I6 as "female".
The initially carried property translates into I6 as "initially_carried".
The mentioned property translates into I6 as "mentioned".
The lit property translates into I6 as "light".
The lighted property translates into I6 as "light".
The lockable property translates into I6 as "lockable".
The locked property translates into I6 as "locked".
The handled property translates into I6 as "moved".
The neuter property translates into I6 as "neuter".
The switched on property translates into I6 as "on".
The open property translates into I6 as "open".
The openable property translates into I6 as "openable".
The privately-named property translates into I6 as "privately_named".
The plural-named property translates into I6 as "pluralname".
The proper-named property translates into I6 as "proper".
The pushable between rooms property translates into I6 as "pushable".
The scenery property translates into I6 as "scenery".
The fixed in place property translates into I6 as "static".
The transparent property translates into I6 as "transparent".
The visited property translates into I6 as "visited".
The marked for listing property translates into I6 as "workflag".

The indefinite article property translates into I6 as "article".
The carrying capacity property translates into I6 as "capacity".
The description property translates into I6 as "description".
The other side property translates into I6 as "door_to".
The initial appearance property translates into I6 as "initial".
The map region property translates into I6 as "map_region".
The printed plural name property translates into I6 as "plural".
The printed name property translates into I6 as "short_name".
The matching key property translates into I6 as "with_key".

Part SR2 - Variables and Rulebooks

Section SR2/1 - Situation

The player is a person that varies. [*]
The player variable translates into I6 as "player".

The location -- documented at var_location -- is an object that varies. [*]
The score -- documented at var_score -- is a number that varies.
The last notified score is a number that varies.
The maximum score is a number that varies. [*]
The turn count is a number that varies.
The time of day -- documented at var_time -- is a time that varies. [*]
The darkness witnessed is a truth state that varies.

The location variable translates into I6 as "real_location".
The score variable translates into I6 as "score".
The last notified score variable translates into I6 as "last_score".
The maximum score variable translates into I6 as "MAX_SCORE".
The turn count variable translates into I6 as "turns".
The time of day variable translates into I6 as "the_time".

Section SR2/2 - Current action

The noun -- documented at var_noun -- is an object that varies. [*]
The second noun is an object that varies. [*]
The person asked -- documented at var_person_asked -- is an object that varies. [*]
The reason the action failed -- documented at var_reason -- is a rule that varies.
The item described is an object that varies.

The noun variable translates into I6 as "noun".
The second noun variable translates into I6 as "second".
The person asked variable translates into I6 as "actor".
The reason the action failed variable translates into I6 as "reason_the_action_failed".
The item described variable translates into I6 as "self".

Section SR2/3 - Used when ruling on accessibility

The person reaching -- documented at var_person_reaching -- is an object that varies.
The container in question is an object that varies.
The supporter in question is an object that varies.
The particular possession -- documented at var_particular -- is a thing that varies.

The person reaching variable translates into I6 as "actor".
The container in question variable translates into I6 as "parameter_object".
The supporter in question variable translates into I6 as "parameter_object".
The particular possession variable translates into I6 as "particular_possession".

Section SR2/4 - Used when understanding typed commands

The player's command -- documented at var_command -- is a snippet that varies.
The matched text is a snippet that varies.
The number understood -- documented at var_understood -- is a number that varies. [*]
The time understood is a time that varies. [*]
The topic understood is a snippet that varies. [*]
The truth state understood is a truth state that varies. [*]
The current item from the multiple object list is an object that varies.

The player's command variable translates into I6 as "players_command".
The matched text variable translates into I6 as "matched_text".
The topic understood variable translates into I6 as "parsed_number".
The truth state understood variable translates into I6 as "parsed_number".
The current item from the multiple object list variable translates into I6 as
	"multiple_object_item".

Section SR2/5 - Presentation on screen

The command prompt -- documented at var_prompt -- is a text that varies. [**]
The command prompt is ">".

The left hand status line -- documented at var_sl -- is a text that varies.
The right hand status line is a text that varies.

The left hand status line variable translates into I6 as "left_hand_status_line".
The right hand status line variable translates into I6 as "right_hand_status_line".

The listing group size is a number that varies.
The listing group size variable translates into I6 as "listing_size".

Section SR2/6 - Unindexed Standard Rules variables - Unindexed

The story title, the story author, the story headline, the story genre
and the story description are text variables. [*****]
The release number and the story creation year are number variables. [**]
Figure of cover is the file of cover art.

The story title variable translates into I6 as "Story".

The substitution-variable is an object that varies. [*]
The substitution-variable variable translates into I6 as "subst__v".

The I6-nothing-constant is an object that varies. [*]
The I6-nothing-constant variable translates into I6 as "nothing".

The I6-varying-global is an object that varies. [*]
The I6-varying-global variable translates into I6 as "nothing".

The item-pushed-between-rooms is an object that varies.
The item-pushed-between-rooms variable translates into I6 as "move_pushing".

The actor-location is an object that varies. [*]
The actor-location variable translates into I6 as "actor_location".

The parameter-object is an object that varies. [*]
The parameter-object variable translates into I6 as "parameter_object".

Section SR2/7 - The Standard Rulebooks

Procedural rules is a rulebook. [0]

Startup rules is a rulebook. [1]
Turn sequence rules is a rulebook. [2]
Shutdown rules is a rulebook. [3]

Scene changing rules is a rulebook. [4]
When play begins is a rulebook. [5]
When play ends is a rulebook. [6]
Every turn rules is a rulebook. [7]

Action-processing rules is a rulebook. [8]
The action-processing rulebook has a person called the actor.
Setting action variables is a rulebook. [9]
The specific action-processing rules is a rulebook. [10]
The specific action-processing rulebook has a truth state called action in world.
The specific action-processing rulebook has a truth state called action keeping silent.
The specific action-processing rulebook has a rulebook called specific check rulebook.
The specific action-processing rulebook has a rulebook called specific carry out rulebook.
The specific action-processing rulebook has a rulebook called specific report rulebook.
The specific action-processing rulebook has a truth state called within the player's sight.
The player's action awareness rules is a rulebook. [11]

Accessibility rules is a rulebook. [12]
Reaching inside rules is an object-based rulebook. [13]
Reaching inside rules have outcomes allow access (success) and deny access (failure).
Reaching outside rules is an object-based rulebook. [14]
Reaching outside rules have outcomes allow access (success) and deny access (failure).
Visibility rules is a rulebook. [15]
Visibility rules have outcomes there is sufficient light (failure) and there is
insufficient light (success).

Persuasion rules is a rulebook. [16]
Persuasion rules have outcomes persuasion succeeds (success) and persuasion fails (failure).
Unsuccessful attempt by is a rulebook. [17]

Before rules is a rulebook. [18]
Instead rules is a rulebook. [19]
Check rules is a rulebook. [20]
Carry out rules is a rulebook. [21]
After rules is a rulebook. [22]
Report rules is a rulebook. [23]

The does the player mean rules are a rulebook. [24]
The does the player mean rules have outcomes it is very likely, it is likely, it is possible,
it is unlikely and it is very unlikely.

Include (-
	[ CheckDPMR result sinp1 sinp2 rv;
		sinp1 = inp1; sinp2 = inp2; inp1 = noun; inp2 = second;
		rv = FollowRulebook( (+does the player mean rules+) );
		inp1 = sinp1; inp2 = sinp2;
		if ((rv) && RulebookSucceeded()) {
			result = ResultOfRule();
			if (result == (+ it is very likely outcome +) ) return 4;
			if (result == (+ it is likely outcome +) ) return 3;
			if (result == (+ it is possible outcome +) ) return 2;
			if (result == (+ it is unlikely outcome +) ) return 1;
			if (result == (+ it is very unlikely outcome +) ) return 0;
		}
		return 2;
	];
-);

Section SR2/8 - The Standard Rules

The little-used do nothing rule translates into I6 as "LITTLE_USED_DO_NOTHING_R".

Include (-
[ LITTLE_USED_DO_NOTHING_R; rfalse; ];
-);

The start in the correct scenes rule is listed first in the startup rulebook. [6th.]
The position player in model world rule is listed first in the startup rulebook. [5th.]
The update chronological records rule is listed first in the startup rulebook. [4th.]
The seed random number generator rule is listed first in the startup rulebook. [3rd.]
The initialise memory rule is listed first in the startup rulebook. [2nd.]
The virtual machine startup rule is listed first in the startup rulebook. [1st.]

The virtual machine startup rule translates into I6 as "VIRTUAL_MACHINE_STARTUP_R".
The initialise memory rule translates into I6 as "INITIALISE_MEMORY_R".
The seed random number generator rule translates into I6 as "SEED_RANDOM_NUMBER_GENERATOR_R".
The update chronological records rule translates into I6 as "UPDATE_CHRONOLOGICAL_RECORDS_R".
The position player in model world rule translates into I6 as "POSITION_PLAYER_IN_MODEL_R".

This is the start in the correct scenes rule: consider the scene changing rules.

The when play begins stage rule is listed in the startup rulebook.
The fix baseline scoring rule is listed in the startup rulebook.
The display banner rule is listed in the startup rulebook.
The initial room description rule is listed in the startup rulebook.

This is the when play begins stage rule: follow the when play begins rulebook.

This is the fix baseline scoring rule: now the last notified score is the score.

This is the display banner rule: say "[banner text]".

This is the initial room description rule: try looking.

A first turn sequence rule (this is the every turn stage rule): follow the every turn rules. [4th.]
A first turn sequence rule: consider the scene changing rules. [3rd.]
The generate action rule is listed first in the turn sequence rulebook. [2nd.]
The parse command rule is listed first in the turn sequence rulebook. [1st.]

The timed events rule is listed in the turn sequence rulebook.
The advance time rule is listed in the turn sequence rulebook.
The update chronological records rule is listed in the turn sequence rulebook.

A last turn sequence rule: consider the scene changing rules. [3rd from last.]
The adjust light rule is listed last in the turn sequence rulebook. [2nd from last.]
The note object acquisitions rule is listed last in the turn sequence rulebook. [Penultimate.]
The notify score changes rule is listed last in the turn sequence rulebook. [Last.]

This is the notify score changes rule:
	if the score is not the last notified score:
		issue score notification message;
		now the last notified score is the score;

The adjust light rule translates into I6 as "ADJUST_LIGHT_R".
The advance time rule translates into I6 as "ADVANCE_TIME_R".
The generate action rule translates into I6 as "GENERATE_ACTION_R".
The note object acquisitions rule translates into I6 as "NOTE_OBJECT_ACQUISITIONS_R".
The parse command rule translates into I6 as "PARSE_COMMAND_R".
The timed events rule translates into I6 as "TIMED_EVENTS_R".

The when play ends stage rule is listed first in the shutdown rulebook.
The resurrect player if asked rule is listed last in the shutdown rulebook.
The print player's obituary rule is listed last in the shutdown rulebook.
The ask the final question rule is listed last in the shutdown rulebook.

This is the when play ends stage rule: follow the when play ends rulebook.
This is the print player's obituary rule:
	carry out the printing the player's obituary activity.

The resurrect player if asked rule translates into I6 as "RESURRECT_PLAYER_IF_ASKED_R".
The ask the final question rule translates into I6 as "ASK_FINAL_QUESTION_R".

The scene change machinery rule is listed last in the scene changing rulebook.

The scene change machinery rule translates into I6 as "DetectSceneChange".

Section SR2/9 - The Entire Game scene

The Entire Game is a scene.
The Entire Game begins when the game is in progress.
The Entire Game ends when the game is over.

Section SR2/10 - Action processing

The before stage rule is listed first in the action-processing rules. [3rd.]
The set pronouns from items from multiple object lists rule is listed first in the
	action-processing rules. [2nd.]
The announce items from multiple object lists rule is listed first in the
	action-processing rules. [1st.]
The basic visibility rule is listed in the action-processing rules.
The basic accessibility rule is listed in the action-processing rules.
The carrying requirements rule is listed in the action-processing rules.
The instead stage rule is listed last in the action-processing rules. [4th from last.]
The requested actions require persuasion rule is listed last in the action-processing rules.
The carry out requested actions rule is listed last in the action-processing rules.
The descend to specific action-processing rule is listed last in the action-processing rules.
The end action-processing in success rule is listed last in the action-processing rules. [Last.]

This is the set pronouns from items from multiple object lists rule:
	if the current item from the multiple object list is not nothing,
		set pronouns from the current item from the multiple object list.

This is the announce items from multiple object lists rule:
	if the current item from the multiple object list is not nothing,
		say "[current item from the multiple object list]: [run paragraph on]".

This is the before stage rule: abide by the before rules.
This is the instead stage rule: abide by the instead rules.

This is the end action-processing in success rule: rule succeeds.

The basic accessibility rule translates into I6 as "BASIC_ACCESSIBILITY_R".
The basic visibility rule translates into I6 as "BASIC_VISIBILITY_R".
The carrying requirements rule translates into I6 as "CARRYING_REQUIREMENTS_R".
The requested actions require persuasion rule translates into I6 as "REQUESTED_ACTIONS_REQUIRE_R".
The carry out requested actions rule translates into I6 as "CARRY_OUT_REQUESTED_ACTIONS_R".
The descend to specific action-processing rule translates into I6 as
"DESCEND_TO_SPECIFIC_ACTION_R".

The work out details of specific action rule is listed first in the specific
action-processing rules.

A specific action-processing rule
	(this is the investigate player's awareness before action rule):
	consider the player's action awareness rules;
	if rule succeeded, change within the player's sight to true;
	otherwise change within the player's sight to false.

A specific action-processing rule (this is the check stage rule):
	anonymously abide by the specific check rulebook.

A specific action-processing rule (this is the carry out stage rule):
	consider the specific carry out rulebook.

A specific action-processing rule (this is the after stage rule):
	if action in world is true, abide by the after rules.

A specific action-processing rule
	(this is the investigate player's awareness after action rule):
	if within the player's sight is false:
		consider the player's action awareness rules;
		if rule succeeded, change within the player's sight to true;

A specific action-processing rule (this is the report stage rule):
	if within the player's sight is true and action keeping silent is false,
		consider the specific report rulebook;

The last specific action-processing rule: rule succeeds.

The work out details of specific action rule translates into I6 as
"WORK_OUT_DETAILS_OF_SPECIFIC_R".

A player's action awareness rule
	(this is the player aware of his own actions rule):
	if the player is the actor, rule succeeds.
A player's action awareness rule
	(this is the player aware of actions by visible actors rule):
	if the player is not the actor and the player can see the actor, rule succeeds.
A player's action awareness rule
	(this is the player aware of actions on visible nouns rule):
	if the noun is a thing and the player can see the noun, rule succeeds.
A player's action awareness rule
	(this is the player aware of actions on visible second nouns rule):
	if the second noun is a thing and the player can see the second noun, rule succeeds.

Section SR2/11 - Accessibility

The access through barriers rule is listed last in the accessibility rules.

The access through barriers rule translates into I6 as "ACCESS_THROUGH_BARRIERS_R".

The can't reach inside rooms rule is listed last in the reaching inside rules. [Penultimate.]
The can't reach inside closed containers rule is listed last in the reaching
inside rules. [Last.]

The can't reach inside closed containers rule translates into I6 as "CANT_REACH_INSIDE_CLOSED_R".
The can't reach inside rooms rule translates into I6 as "CANT_REACH_INSIDE_ROOMS_R".

The can't reach outside closed containers rule is listed last in the reaching outside rules.

The can't reach outside closed containers rule translates into I6 as "CANT_REACH_OUTSIDE_CLOSED_R".

The can't act in the dark rule is listed last in the visibility rules.

The last visibility rule (this is the can't act in the dark rule): if in darkness, rule succeeds.

Does the player mean taking something which is carried by the player
	(this is the very unlikely to mean taking what's already carried rule):
	it is very unlikely.

Definition: a number is even rather than odd if the remainder after dividing it by 2 is 0.
Definition: a number is positive if it is greater than zero.
Definition: a number is negative if it is less than zero.

Definition: a text is empty rather than non-empty if it is "".

Definition: an indexed text is empty rather than non-empty if I6 routine "INDEXED_TEXT_TY_Empty" says so
	(it contains no characters).

A scene can be recurring or non-recurring. A scene is usually non-recurring.
Definition: a scene is happening if I6 condition "scene_status-->(*1-1)==1"
	says so (it is currently taking place).

Definition: a table-name is empty rather than non-empty if the number of filled rows in it is 0.
Definition: a table-name is full rather than non-full if the number of blank rows in it is 0.

Definition: a rulebook is empty rather than non-empty if I6 routine "RulebookEmpty" says so (it
	contains no rules, so that following it does nothing and makes no decision).

Definition: an activity is empty rather than non-empty if I6 routine "ActivityEmpty" says so (its
	before, for and after rulebooks are all empty).
Definition: an activity is going on if I6 routine "TestActivity" says so (one
	of its three rulebooks is currently being run).

Definition: a list of values is empty rather than non-empty if I6 routine "LIST_OF_TY_Empty" says so
	(it contains no entries).

Part SR3 - Activities

Printing the name of something (documented at act_pn) is an activity. [0]

Before printing the name of a thing (called the item being printed)
	(this is the make named things mentioned rule):
	now the item being printed is mentioned.

The standard name printing rule is listed last in the for printing the name rulebook.
The standard name printing rule translates into I6 as "STANDARD_NAME_PRINTING_R".

Printing the plural name of something (documented at act_ppn) is an activity. [1]

Rule for printing the plural name of something (called the item) (this is the standard
	printing the plural name rule):
	say the printed plural name of the item.
The standard printing the plural name rule is listed last in the for printing
the plural name rulebook.

Printing a number of something (documented at act_pan) is an activity. [2]

Rule for printing a number of something (called the item) (this is the standard
	printing a number of something rule):
	say "[listing group size in words] ";
	carry out the printing the plural name activity with the item.
The standard printing a number of something rule is listed last in the for printing
a number rulebook.

Printing room description details of something (documented at act_details) is an activity. [3]

Listing contents of something (documented at act_lc) is an activity. [4]
The standard contents listing rule is listed last in the for listing contents rulebook.
The standard contents listing rule translates into I6 as "STANDARD_CONTENTS_LISTING_R".
Grouping together something (documented at act_gt) is an activity. [5]

Writing a paragraph about something (documented at act_wpa) is an activity. [6]

Listing nondescript items of something (documented at act_lni) is an activity. [7]

Printing the name of a dark room (documented at act_darkname) is an activity. [8]
Printing the description of a dark room (documented at act_darkdesc) is an activity. [9]
Printing the announcement of darkness (documented at act_nowdark) is an activity. [10]
Printing the announcement of light (documented at act_nowlight) is an activity. [11]
Printing a refusal to act in the dark (documented at act_toodark) is an activity. [12]

The look around once light available rule is listed last in for printing the
announcement of light.

This is the look around once light available rule:
	try looking.

Constructing the status line (documented at act_csl) is an activity. [13]
Printing the banner text (documented at act_banner) is an activity. [14]

Reading a command (documented at act_reading) is an activity. [15]
Deciding the scope of something (future action) (documented at act_ds) is an activity. [16]
Deciding the concealed possessions of something (documented at act_con) is an activity. [17]
Deciding whether all includes something (future action) (documented at act_all)
	is an activity. [18]
The for deciding whether all includes rules have outcomes it does not (failure) and
	it does (success).
Clarifying the parser's choice of something (future action) (documented at act_clarify)
	is an activity. [19]
Asking which do you mean (future action) (documented at act_which) is an activity. [20]
Printing a parser error (documented at act_parsererror) is an activity. [21]
Supplying a missing noun (documented at act_smn) is an activity. [22]
Supplying a missing second noun (documented at act_smn) is an activity. [23]
Implicitly taking something (documented at act_implicitly) is an activity. [24]

Rule for supplying a missing noun while an actor smelling (this is the ambient odour rule):
	change the noun to the location.

Rule for supplying a missing noun while an actor listening (this is the ambient sound rule):
	change the noun to the location.

Rule for supplying a missing noun while an actor going (this is the block vaguely going rule):
	issue library message going action number 7.

Starting the virtual machine (documented at act_startvm) is an activity. [25]

Amusing a victorious player (documented at act_amuse) is an activity. [26]

Printing the player's obituary (documented at act_obit) is an activity. [27]
The print obituary headline rule is listed last in for printing the player's obituary.
The print final score rule is listed last in for printing the player's obituary.
The display final status line rule is listed last in for printing the player's obituary.

The print obituary headline rule translates into I6 as "PRINT_OBITUARY_HEADLINE_R".
The print final score rule translates into I6 as "PRINT_FINAL_SCORE_R".
The display final status line rule translates into I6 as "DISPLAY_FINAL_STATUS_LINE_R".

Handling the final question is an activity. [28]

The immediately restart the VM rule translates into I6 as "IMMEDIATELY_RESTART_VM_R".
The immediately restore saved game rule translates into I6 as "IMMEDIATELY_RESTORE_SAVED_R".
The immediately quit rule translates into I6 as "IMMEDIATELY_QUIT_R".
The immediately undo rule translates into I6 as "IMMEDIATELY_UNDO_R".

The print the final question rule is listed in before handling the final question.
The print the final prompt rule is listed in before handling the final question.
The read the final answer rule is listed last in before handling the final question.
The standard respond to final question rule is listed last in for handling the final question.

This is the print the final prompt rule: say "> [run paragraph on]".

The read the final answer rule translates into I6 as "READ_FINAL_ANSWER_R".

This is the print the final question rule:
	let named options count be 0;
	repeat through the Table of Final Question Options:
		if the only if victorious entry is false or the game ended in victory:
			if there is a final response rule entry
				or the final response activity entry [activity] is not empty:
				if there is a final question wording entry, increase named options count by 1;
	if the named options count is less than 1, abide by the immediately quit rule;
	say "Would you like to ";
	repeat through the Table of Final Question Options:
		if the only if victorious entry is false or the game ended in victory:
			if there is a final response rule entry
				or the final response activity entry [activity] is not empty:
				if there is a final question wording entry:
					say final question wording entry;
					decrease named options count by 1;
					if the named options count is 0:
						say "?[line break]";
					otherwise if the named options count is 1:
						if using the serial comma option, say ",";
						say " or ";
					otherwise:
						say ", ";

This is the standard respond to final question rule:
	repeat through the Table of Final Question Options:
		if the only if victorious entry is false or the game ended in victory:
			if there is a final response rule entry
				or the final response activity entry [activity] is not empty:
				if the player's command matches the topic entry:
					if there is a final response rule entry, abide by final response rule entry;
					otherwise carry out the final response activity entry activity;
					rule succeeds;
	issue miscellaneous library message number 8.

Section SR2/12 - Final question options

Table of Final Question Options
final question wording	only if victorious	topic		final response rule		final response activity
"RESTART"				false				"restart"	immediately restart the VM rule	--
"RESTORE a saved game"	false				"restore"	immediately restore saved game rule	--
"see some suggestions for AMUSING things to do"	true	"amusing"	--	amusing a victorious player
"QUIT"					false				"quit"		immediately quit rule	--
--						false				"undo"		immediately undo rule	--

Section SR2/13 - Locale descriptions - Unindexed

Table of Locale Priorities
notable-object (an object)	locale description priority (a number)
--							--
with blank rows for each thing.

To describe locale for (O - object):
	carry out the printing the locale description activity with O.

To set the/-- locale priority of (O - an object) to (N - a number):
	if O is a thing:
		if N <= 0, now O is mentioned;
		if there is a notable-object of O in the Table of Locale Priorities:
			choose row with a notable-object of O in the Table of Locale Priorities;
			if N <= 0, blank out the whole row;
			otherwise change the locale description priority entry to N;
		otherwise:
			if N is greater than 0:
				choose a blank row in the Table of Locale Priorities;
				change the notable-object entry to O;
				change the locale description priority entry to N;

Printing the locale description of something (documented at act_pld) is an activity.

The locale paragraph count is a number that varies.

Before printing the locale description (this is the initialise locale description rule):
	now the locale paragraph count is 0;
	repeat with item running through things:
		now the item is not mentioned;
	repeat through the Table of Locale Priorities:
		blank out the whole row.

Before printing the locale description (this is the find notable locale objects rule):
	let the domain be the parameter-object;
	carry out the choosing notable locale objects activity with the domain;
	continue the activity.

For printing the locale description (this is the interesting locale paragraphs rule):
	let the domain be the parameter-object;
	sort the Table of Locale Priorities in locale description priority order;
	repeat through the Table of Locale Priorities:
		[say "[notable-object entry]...";]
		carry out the printing a locale paragraph about activity with the notable-object entry;
	continue the activity.

For printing the locale description (this is the you-can-also-see rule):
	let the domain be the parameter-object;
	let the mentionable count be 0;
	repeat with item running through things:
		now the item is not marked for listing;
	repeat through the Table of Locale Priorities:
		[say "[notable-object entry] - [locale description priority entry].";]
		if the locale description priority entry is greater than 0,
			now the notable-object entry is marked for listing;
		increase the mentionable count by 1;
	if the mentionable count is greater than 0:
		repeat with item running through things:
			if the item is mentioned:
				now the item is not marked for listing;
		begin the listing nondescript items activity;
		if the number of marked for listing things is 0:
			abandon the listing nondescript items activity;
		otherwise:
			if handling the listing nondescript items activity:
				if the domain is a room:
					if the domain is the location, say "You ";
					otherwise say "In [the domain] you ";
				otherwise if the domain is a supporter:
					say "On [the domain] you ";
				otherwise:
					say "In [the domain] you ";
				say "can [if the locale paragraph count is greater than 0]also [end if]see ";
				let the common holder be nothing;
				let contents form of list be true;
				repeat with list item running through marked for listing things:
					if the holder of the list item is not the common holder:
						if the common holder is nothing,
							now the common holder is the holder of the list item;
						otherwise now contents form of list is false;
					if the list item is mentioned, now the list item is not marked for listing;
				filter list recursion to unmentioned things;
				if contents form of list is true and the common holder is not nothing,
					list the contents of the common holder, as a sentence, including contents,
						giving brief inventory information, tersely, not listing
						concealed items, listing marked items only;
				otherwise say "[a list of marked for listing things including contents]";
				if the domain is the location, say " here";
				say ".[paragraph break]";
				unfilter list recursion;
			end the listing nondescript items activity;
	continue the activity.

Choosing notable locale objects of something (documented at act_cnlo) is an activity.

For choosing notable locale objects (this is the standard notable locale objects rule):
	let the domain be the parameter-object;
	let the held item be the first thing held by the domain;
	while the held item is a thing:
		set the locale priority of the held item to 5;
		now the held item is the next thing held after the held item;
	continue the activity.

Printing a locale paragraph about something (documented at act_plp) is an activity.

For printing a locale paragraph about a thing (called the item)
	(this is the don't mention player's supporter in room descriptions rule):
	if the item encloses the player, set the locale priority of the item to 0;
	continue the activity.

For printing a locale paragraph about a thing (called the item)
	(this is the don't mention scenery in room descriptions rule):
	if the item is scenery, set the locale priority of the item to 0;
	continue the activity.

For printing a locale paragraph about a thing (called the item)
	(this is the don't mention undescribed items in room descriptions rule):
	if the item is undescribed, set the locale priority of the item to 0;
	continue the activity.

For printing a locale paragraph about a thing (called the item)
	(this is the set pronouns from items in room descriptions rule):
	if the item is not mentioned, set pronouns from the item;
	continue the activity.

For printing a locale paragraph about a thing (called the item)
	(this is the offer items to writing a paragraph about rule):
	if the item is not mentioned:
		if a paragraph break is pending, say "[conditional paragraph break]";
		carry out the writing a paragraph about activity with the item;
		if a paragraph break is pending:
			increase the locale paragraph count by 1;
			now the item is mentioned;
			say "[command clarification break]";
	continue the activity.

For printing a locale paragraph about a thing (called the item)
	(this is the use initial appearance in room descriptions rule):
	if the item is not mentioned:
		if the item provides the property initial appearance and the
			item is not handled:
			increase the locale paragraph count by 1;
			say "[initial appearance of the item]";
			say "[paragraph break]";
			if a locale-supportable thing is on the item:
				repeat with possibility running through things on the item:
					now the possibility is marked for listing;
					if the possibility is mentioned:
						now the possibility is not marked for listing;
				say "On [the item] ";
				list the contents of the item, as a sentence, including contents,
					giving brief inventory information, tersely, not listing
					concealed items, prefacing with is/are, listing marked items only;
				say ".[paragraph break]";
			now the item is mentioned;
	continue the activity.

Definition: a thing (called the item) is locale-supportable if the item is not
scenery and the item is not mentioned.

For printing a locale paragraph about a thing (called the item)
	(this is the describe what's on scenery supporters in room descriptions rule):
	if the item is not undescribed and the item is scenery and
		the item does not enclose the player:
		set pronouns from the item;
		if a locale-supportable thing is on the item:
			repeat with possibility running through things on the item:
				now the possibility is marked for listing;
				if the possibility is mentioned:
					now the possibility is not marked for listing;
			increase the locale paragraph count by 1;
			say "On [the item] ";
			list the contents of the item, as a sentence, including contents,
				giving brief inventory information, tersely, not listing
				concealed items, prefacing with is/are, listing marked items only;
			say ".[paragraph break]";
	continue the activity.

Part SR4 - Actions

Section SR4/1 - Generic action patterns

Section SR4/2 - Actions concerning the actor's possessions

Taking inventory is an action with past participle taken, applying to nothing.
The taking inventory action translates into I6 as "Inv".

The specification of the taking inventory action is "Taking an inventory of
one's immediate possessions: the things being carried, either directly or in
any containers being carried. When the player performs this action, either
the inventory listing, or else a special message if nothing is being carried
or worn, is printed during the carry out rules: nothing happens at the report
stage. The opposite happens for other people performing the action: nothing
happens during carry out, but a report such as 'Mr X looks through his
possessions.' is produced (provided Mr X is visible).

The exotic 'use inventory to set pronouns rule' allows the inventory
listing to change the current meanings of IT, HIM, HER, and so on for the
player's future commands. Thus if the player is carrying a certain female
cat, say, taking inventory might print up the name 'Missee Lee', and this
would set HER to refer to Missee Lee. Some IF authors dislike this
convention: it can be abolished by writing 'The use inventory to set
pronouns rule is not listed in any rulebook.'"

Carry out taking inventory (this is the print empty inventory rule):
	if the first thing held by the player is nothing, stop the action with
		library message taking inventory action number 1.

Carry out taking inventory (this is the print standard inventory rule):
	issue library message taking inventory action number 2;
	say ":[line break]";
	list the contents of the player, with newlines, indented, including contents,
		giving inventory information, with extra indentation.

Carry out taking inventory (this is the use inventory to set pronouns rule):
	set pronouns from possessions of the player.

Report an actor taking inventory (this is the report other people taking
	inventory rule):
	if the actor is not the player,
		issue actor-based library message taking inventory action number 5 for the actor.

Taking is an action with past participle taken, applying to one thing.
The taking action translates into I6 as "Take".

The specification of the taking action is "The taking action is the only way
an action in the Standard Rules can cause something to be carried by an actor.
It is very simple in operation (the entire carry out stage consists only of
'now the actor carries the noun') but many checks must be performed before it
can be allowed to happen."

Check an actor taking (this is the can't take yourself rule):
	if the actor is the noun, stop the action with library message taking
		action number 2 for the noun.

Check an actor taking (this is the can't take other people rule):
	if the noun is a person, stop the action with library message taking
		action number 3 for the noun.

Check an actor taking (this is the can't take component parts rule):
	if the noun is part of something (called the whole), stop the action
		with library message taking action number 7 for the whole.

Check an actor taking (this is the can't take people's possessions rule):
	let the local ceiling be the common ancestor of the actor with the noun;
	let H be the not-counting-parts holder of the noun;
	while H is not nothing and H is not the local ceiling:
		if H is a person, stop the action with library message taking action
			number 6 for H;
		let H be the not-counting-parts holder of H;

Check an actor taking (this is the can't take items out of play rule):
	let H be the noun;
	while H is not nothing and H is not a room:
		let H be the not-counting-parts holder of H;
	if H is nothing, stop the action with library message taking action
		number 8 for the noun.

Check an actor taking (this is the can't take what you're inside rule):
	let the local ceiling be the common ancestor of the actor with the noun;
	if the local ceiling is the noun, stop the action with library message
		taking action number 4 for the noun.

Check an actor taking (this is the can't take what's already taken rule):
	if the actor is carrying the noun, stop the action with library message
		taking action number 5 for the noun;
	if the actor is wearing the noun, stop the action with library message
		taking action number 5 for the noun.

Check an actor taking (this is the can't take scenery rule):
	if the noun is scenery, stop the action with library message taking
		action number 10 for the noun.

Check an actor taking (this is the can only take things rule):
	if the noun is not a thing, stop the action with library message taking
		action number 15 for the noun.

Check an actor taking (this is the can't take what's fixed in place rule):
	if the noun is fixed in place, stop the action with library message taking
		action number 11 for the noun.

Check an actor taking (this is the use player's holdall to avoid exceeding
	carrying capacity rule):
	if the number of things carried by the actor is at least the
		carrying capacity of the actor:
		if the actor is holding a player's holdall (called the current working sack):
			let the transferred item be nothing;
			repeat with the possible item running through things carried by
				the actor:
				if the possible item is not lit and the possible item is not
					the current working sack, let the transferred item be the possible item;
			if the transferred item is not nothing:
				issue library message taking action number 13 for the
					transferred item;
				silently try the actor trying inserting the transferred item
					into the current working sack;
				if the transferred item is not in the current working sack, stop the action;

Check an actor taking (this is the can't exceed carrying capacity rule):
	if the number of things carried by the actor is at least the
		carrying capacity of the actor, stop the action with library
		message taking action number 12 for the actor.

Carry out an actor taking (this is the standard taking rule):
	now the actor carries the noun.

Report an actor taking (this is the standard report taking rule):
	if the actor is the player, issue library message taking action number 1
		for the noun;
	otherwise issue actor-based library message taking action number 16 for the noun.

Removing it from is an action applying to two things.
The removing it from action translates into I6 as "Remove".

The specification of the removing it from action is "Removing is not really
an action in its own right. Whereas there are many ways to put something down
(on the floor, on top of something, inside something else, giving it to
somebody else, and so on), Inform has only one way to take something: the
taking action. Removing exists only to provide some nicely worded replies
to impossible requests, and in all sensible cases is converted into taking.
Because of this, it's usually a bad idea to write rules about removing:
if you write a rule such as 'Instead of removing the key, ...' then it
won't apply if the player simply types TAKE KEY instead. The safe way to
do this is to write a rule about taking, which covers all possibilities."

Check an actor removing something from (this is the can't remove what's not inside rule):
	if the holder of the noun is not the second noun, stop the action with
		library message removing it from action number 2 for the noun.

Check an actor removing something from (this is the can't remove from people rule):
	if the holder of the noun is a person (called the owner), stop the action with
		library message taking action number 6 for the owner.

Check an actor removing something from (this is the convert remove to take rule):
	convert to the taking action on the noun.

The can't take component parts rule is listed before the can't remove what's not
inside rule in the check removing it from rules.

Dropping is an action applying to one thing.
The dropping action translates into I6 as "Drop".

The specification of the dropping action is "Dropping is one of five actions
by which an actor can get rid of something carried: the others are inserting
(into a container), putting (onto a supporter), giving (to someone else) and
eating. Dropping means dropping onto the actor's current floor, which is
usually the floor of a room - but might be the inside of a box if the actor
is also inside that box, and so on.

The can't drop clothes being worn rule silently tries the taking off action
on any clothing being dropped: unlisting this rule removes both this behaviour
and also the requirement that clothes cannot simply be dropped."

Check an actor dropping (this is the can't drop yourself rule):
	if the noun is the actor, stop the action with library message putting
		it on action number 4.

Check an actor dropping (this is the can't drop what's already dropped rule):
	if the noun is in the holder of the actor, stop the action with library
		message dropping action number 1 for the noun.

Check an actor dropping (this is the can't drop what's not held rule):
	if the actor is carrying the noun, continue the action;
	if the actor is wearing the noun, continue the action;
	stop the action with library message dropping action number 2 for the noun.

Check an actor dropping (this is the can't drop clothes being worn rule):
	if the actor is wearing the noun:
		issue library message dropping action number 3 for the noun;
		silently try the actor trying taking off the noun;
		if the actor is wearing the noun, stop the action;

Check an actor dropping (this is the can't drop if this exceeds carrying
	capacity rule):
	let H be the holder of the actor;
	if H is a room, continue the action; [room floors have infinite capacity]
	if H provides the property carrying capacity:
		if H is a supporter:
			if the number of things on H is at least the carrying capacity of H:
				if the actor is the player,
					issue library message dropping action number 5 for H;
				stop the action;
		otherwise if H is a container:
			if the number of things in H is at least the carrying capacity of H:
				if the actor is the player,
					issue library message dropping action number 6 for H;
				stop the action;

Carry out an actor dropping (this is the standard dropping rule):
	now the noun is in the holder of the actor.

Report an actor dropping (this is the standard report dropping rule):
	if the actor is the player, issue library message dropping action number 4
		for the noun;
	otherwise issue actor-based library message dropping action number 7 for the noun.

Putting it on is an action with past participle put, applying to two things.
The putting it on action translates into I6 as "PutOn".

The specification of the putting it on action is "By this action, an actor puts
something he is holding on top of a supporter: for instance, putting an apple
on a table."

Check an actor putting something on (this is the convert put to drop where possible rule):
	if the second noun is down or the actor is on the second noun,
		convert to the dropping action on the noun.

Check an actor putting something on (this is the can't put what's not held rule):
	if the actor is carrying the noun, continue the action;
	if the actor is wearing the noun, continue the action;
	stop the action with library message putting it on action number 1 for the noun.

Check an actor putting something on (this is the can't put something on itself rule):
	let the noun-CPC be the component parts core of the noun;
	let the second-CPC be the component parts core of the second noun;
	let the transfer ceiling be the common ancestor of the noun-CPC with the second-CPC;
	if the transfer ceiling is the noun-CPC,
		stop the action with library message putting it on action number 2 for
			the noun.

Check an actor putting something on (this is the can't put onto what's not a supporter rule):
	if the second noun is not a supporter,
		stop the action with library message putting it on action number 3 for
			the second noun.

Check an actor putting something on (this is the can't put onto something being carried rule):
	if the actor encloses the second noun,
		stop the action with library message putting it on action number 4 for
			the second noun.

Check an actor putting something on (this is the can't put clothes being worn rule):
	if the actor is wearing the noun:
		issue library message putting it on action number 5 for the noun;
		silently try the actor trying taking off the noun;
		if the actor is wearing the noun, stop the action;

Check an actor putting something on (this is the can't put if this exceeds
	carrying capacity rule):
	if the second noun provides the property carrying capacity:
		if the number of things on the second noun is at least the carrying capacity
			of the second noun,
			stop the action with library message putting it on action number 6 for
			the second noun;

Carry out an actor putting something on (this is the standard putting rule):
	now the noun is on the second noun.

Report an actor putting something on (this is the concise report putting rule):
	if the actor is the player and the I6 parser is running multiple actions,
		stop the action with library message putting it on action number 7
		for the noun;
	otherwise continue the action.

Report an actor putting something on (this is the standard report putting rule):
	if the actor is the player, issue library message putting it on action
		number 8 for the noun;
	otherwise issue actor-based library message putting it on action
		number 9 for the noun.

Inserting it into is an action applying to two things.
The inserting it into action translates into I6 as "Insert".

The specification of the inserting it into action is "By this action, an actor puts
something he is holding into a container: for instance, putting a coin into a
collection box."

Check an actor inserting something into (this is the convert insert to drop where
	possible rule):
	if the second noun is down or the actor is in the second noun,
		convert to the dropping action on the noun.

Check an actor inserting something into (this is the can't insert what's not held rule):
	if the actor is carrying the noun, continue the action;
	if the actor is wearing the noun, continue the action;
	stop the action with library message inserting it into action number 1 for
		the noun.

Check an actor inserting something into (this is the can't insert something into itself rule):
	let the noun-CPC be the component parts core of the noun;
	let the second-CPC be the component parts core of the second noun;
	let the transfer ceiling be the common ancestor of the noun-CPC with the second-CPC;
	if the transfer ceiling is the noun-CPC,
		stop the action with library message inserting it into action number 5 for
			the noun.

Check an actor inserting something into (this is the can't insert into closed containers rule):
	if the second noun is a closed container,
		stop the action with library message inserting it into action number 3 for
			the second noun.

Check an actor inserting something into (this is the can't insert into what's not a
	container rule):
	if the second noun is not a container,
		stop the action with library message inserting it into action number 2 for
			the second noun.

Check an actor inserting something into (this is the can't insert clothes being worn rule):
	if the actor is wearing the noun:
		issue library message inserting it into action number 6 for the noun;
		silently try the actor trying taking off the noun;
		if the actor is wearing the noun, stop the action;

Check an actor inserting something into (this is the can't insert if this exceeds
	carrying capacity rule):
	if the second noun provides the property carrying capacity:
		if the number of things in the second noun is at least the carrying capacity
		of the second noun,
			stop the action with library message inserting it into action number 7 for
				the second noun;

Carry out an actor inserting something into (this is the standard inserting rule):
	now the noun is in the second noun.

Report an actor inserting something into (this is the concise report inserting rule):
	if the actor is the player and the I6 parser is running multiple actions,
		stop the action with library message inserting it into action number 8
		for the noun;
	otherwise continue the action.

Report an actor inserting something into (this is the standard report inserting rule):
	if the actor is the player, issue library message inserting it into action
		number 9 for the noun;
	otherwise issue actor-based library message inserting it into action number 10 for the noun.

Eating is an action with past participle eaten, applying to one carried thing.
The eating action translates into I6 as "Eat".

The specification of the eating action is "Eating is the only one of the
built-in actions which can, in effect, destroy something: the carry out
rule removes what's being eaten from play, and nothing in the Standard
Rules can then get at it again.

Note that, uncontroversially, one can only eat things with the 'edible'
either/or property, and also that, more controversially, one can only
eat things currently being held. This means that a player standing next
to a bush with berries who types EAT BERRIES will force a '(first taking
the berries)' action."

Check an actor eating (this is the can't eat unless edible rule):
	if the noun is not a thing or the noun is not edible,
		stop the action with library message eating action number 1 for the noun.

Check an actor eating (this is the can't eat clothing without removing it first rule):
	if the actor is wearing the noun:
		issue library message dropping action number 3 for the noun;
		try the actor trying taking off the noun;
		if the actor is wearing the noun, stop the action;

Carry out an actor eating (this is the standard eating rule):
	remove the noun from play.

Report an actor eating (this is the standard report eating rule):
	if the actor is the player, issue library message eating action number 2
		for the noun;
	otherwise issue actor-based library message eating action number 3 for the noun.

Section SR4/3 - Actions which move the actor

Going is an action with past participle gone, applying to one visible thing.
The going action translates into I6 as "Go".

The specification of the going action is "This is the action which allows people
to move from one room to another, using whatever map connections and doors are
to hand. The Standard Rules are written so that the noun can be either a
direction or a door in the location of the actor: while the player's commands
only lead to going actions with directions as nouns, going actions can also
happen as a result of entering actions, and then the noun can indeed be
a door."

The going action has a room called the room gone from (matched as "from").
The going action has an object called the room gone to (matched as "to").
The going action has an object called the door gone through (matched as "through").
The going action has an object called the vehicle gone by (matched as "by").
The going action has an object called the thing gone with (matched as "with").

Rule for setting action variables for going (this is the standard set going variables rule):
	now the thing gone with is the item-pushed-between-rooms;
	now the room gone from is the location of the actor;
	if the actor is in an enterable vehicle (called the carriage),
		now the vehicle gone by is the carriage;
	let the target be nothing;
	if the noun is a direction:
		let direction D be the noun;
		let the target be the room-or-door direction D from the room gone from;
	otherwise:
		if the noun is a door, let the target be the noun;
	if the target is a door:
		now the door gone through is the target;
		now the target is the other side of the target from the room gone from;
	now the room gone to is the target.

Check an actor going (this is the can't travel in what's not a vehicle rule):
	let H be the holder of the actor;
	if H is the room gone from, continue the action;
	if H is the vehicle gone by, continue the action;
	stop the action with library message going action number 1 for H.

Check an actor going (this is the can't go through undescribed doors rule):
	if the door gone through is not nothing and the door gone through is undescribed,
		stop the action with library message going action number 2 for the room gone from.

Check an actor going (this is the can't go through closed doors rule):
	if the door gone through is not nothing and the door gone through is closed:
		if the noun is up, stop the action with library message going action number 3
			for the door gone through;
		if the noun is down, stop the action with library message going action number 4
			for the door gone through;
		stop the action with library message going action number 5 for the door gone
			through;

Check an actor going (this is the determine map connection rule):
	let the target be nothing;
	if the noun is a direction:
		let direction D be the noun;
		let the target be the room-or-door direction D from the room gone from;
	otherwise:
		if the noun is a door, let the target be the noun;
	if the target is a door:
		now the target is the other side of the target from the room gone from;
	now the room gone to is the target.

Check an actor going (this is the can't go that way rule):
	if the room gone to is nothing:
		if the door gone through is nothing, stop the action with library
			message going action number 2 for the room gone from;
		stop the action with library message going action number 6 for the door gone through;

Carry out an actor going (this is the move player and vehicle rule):
	if the vehicle gone by is nothing,
		surreptitiously move the actor to the room gone to during going;
	otherwise
		surreptitiously move the vehicle gone by to the room gone to during going.

Carry out an actor going (this is the move floating objects rule):
	if the actor is the player,
		update backdrop positions.

Carry out an actor going (this is the check light in new location rule):
	if the actor is the player,
		surreptitiously reckon darkness.

Report an actor going (this is the describe room gone into rule):
	if the player is the actor:
		produce a room description with going spacing conventions;
	otherwise:
		if the noun is a direction:
			if the location is the room gone from:
				if the location is the room gone to:
					continue the action;
				otherwise:
					if the noun is up :
						issue actor-based library message going action number 8;
					otherwise if the noun is down:
						issue actor-based library message going action number 9;
					otherwise:
						issue actor-based library message going action number 10 for the noun;
			otherwise:
				let the back way be the opposite of the noun;
				if the location is the room gone to:
					let the room back the other way be the room back way from the
						location;
					let the room normally this way be the room noun from the
						room gone from;
					if the room back the other way is the room gone from or
						the room back the other way is the room normally this way:
						if the back way is up:
							issue actor-based library message going action number 11;
						otherwise if the back way is down:
							issue actor-based library message going action number 12;
						otherwise:
							issue actor-based library message going action number 13
								for the back way;
					otherwise:
						issue actor-based library message going action number 14;
				otherwise:
					if the back way is up :
						issue actor-based library message going action number 15
							for the room gone to;
					otherwise if the back way is down:
						issue actor-based library message going action number 16
							for the room gone to;
					otherwise:
						issue actor-based library message going action number 17
							for the room gone to and the back way;
		otherwise if the location is the room gone from:
			issue actor-based library message going action number 18 for the noun;
		otherwise:
			issue actor-based library message going action number 19 for the noun;
		if the vehicle gone by is not nothing:
			say " ";
			if the vehicle gone by is a supporter, issue actor-based library message
				going action number 20 for the vehicle gone by;
			otherwise issue actor-based library message going action number 21
				for the vehicle gone by;
		if the thing gone with is not nothing:
			if the player is within the thing gone with:
				issue actor-based library message going action number 22 for the thing gone with;
			otherwise if the player is within the vehicle gone by:
				issue actor-based library message going action number 23 for the thing gone with;
			otherwise if the location is the room gone from:
				issue actor-based library message going action number 24 for the thing gone with;
			otherwise:
				issue actor-based library message going action number 25 for the thing gone with;
		if the player is within the vehicle gone by and the player is not
			within the thing gone with:
			issue actor-based library message going action number 26;
			say ".";
			try looking;
			continue the action;
		say ".";

Entering is an action applying to one thing.
The entering action translates into I6 as "Enter".

The specification of the entering action is "Whereas the going action allows
people to move from one location to another in the model world, the entering
action is for movement inside a location: for instance, climbing into a cage
or sitting on a couch. (Entering is not allowed to change location, so any
attempt to enter a door is converted into a going action.) What makes
entering trickier than it looks is that the player may try to enter an
object which is itself inside, or part of, something else, which might in
turn be... and so on. To preserve realism, the implicitly pass through other
barriers rule automatically generates entering and exiting actions needed
to pass between anything which might be in the way: for instance, in a
room with two open cages, an actor in cage A who tries entering cage B first
has to perform an exiting action."

Check an actor entering (this is the convert enter door into go rule):
	if the noun is a door, convert to the going action on the noun.

Check an actor entering (this is the convert enter compass direction into go rule):
	if the noun is a direction, convert to the going action on the noun.

Check an actor entering (this is the can't enter what's already entered rule):
	let the local ceiling be the common ancestor of the actor with the noun;
	if the local ceiling is the noun, stop the action with library message
		entering action number 1 for the noun.

Check an actor entering (this is the can't enter what's not enterable rule):
	if the noun is not enterable, stop the action with library message
		entering action number 2 for the noun.

Check an actor entering (this is the can't enter closed containers rule):
	if the noun is a closed container, stop the action with library message
		entering action number 3 for the noun.

Check an actor entering (this is the can't enter something carried rule):
	let the local ceiling be the common ancestor of the actor with the noun;
	if the local ceiling is the actor, stop the action with library message
		entering action number 4 for the noun.

Check an actor entering (this is the implicitly pass through other barriers rule):
	if the holder of the actor is the holder of the noun, continue the action;
	let the local ceiling be the common ancestor of the actor with the noun;
	while the holder of the actor is not the local ceiling:
		let the target be the holder of the actor;
		issue library message entering action number 6 for the target;
		silently try the actor trying exiting;
		if the holder of the actor is the target, stop the action;
	if the holder of the actor is the noun, stop the action;
	if the holder of the actor is the holder of the noun, continue the action;
	let the target be the holder of the noun;
	if the noun is part of the target, let the target be the holder of the target;
	while the target is a thing:
		if the holder of the target is the local ceiling:
			issue library message entering action number 7 for the target;
			silently try the actor trying entering the target;
			if the holder of the actor is not the target, stop the action;
			convert to the entering action on the noun;
			continue the action;
		let the target be the holder of the target;

Carry out an actor entering (this is the standard entering rule):
	surreptitiously move the actor to the noun.

Report an actor entering (this is the standard report entering rule):
	if the actor is the player:
		issue library message entering action number 5 for the noun;
	otherwise if the noun is a container:
		issue actor-based library message entering action number 8 for the noun;
	otherwise:
		issue actor-based library message entering action number 9 for the noun;
	continue the action.

Report an actor entering (this is the describe contents entered into rule):
	if the actor is the player, describe locale for the noun.

Exiting is an action applying to nothing.
The exiting action translates into I6 as "Exit".
The exiting action has an object called the container exited from.

The specification of the exiting action is "Whereas the going action allows
people to move from one location to another in the model world, and the
entering action is for movement deeper inside the objects in a location,
the exiting action is for movement back out towards the main floor area.
Climbing out of a cupboard, for instance, is an exiting action. Exiting
when already in the main floor area of a room with a map connection to
the outside is converted to a going action. Finally, note that whereas
entering works for either containers or supporters, exiting is purely for
getting oneself out of containers: if the actor is on top of a supporter
instead, an exiting action is converted to the getting off action."

Setting action variables for exiting:
	now the container exited from is the holder of the actor.

Check an actor exiting (this is the convert exit into go out rule):
	let the local room be the location of the actor;
	if the container exited from is the local room:
		if the room-or-door outside from the local room is not nothing,
			convert to the going action on the outside;

Check an actor exiting (this is the can't exit when not inside anything rule):
	let the local room be the location of the actor;
	if the container exited from is the local room, stop the action with
		library message exiting action number 1 for the actor.

Check an actor exiting (this is the can't exit closed containers rule):
	if the actor is in a closed container (called the cage), stop the action
		with library message exiting action number 2 for the cage.

Check an actor exiting (this is the convert exit into get off rule):
	if the actor is on a supporter (called the platform),
		convert to the getting off action on the platform.

Carry out an actor exiting (this is the standard exiting rule):
	let the former exterior be the not-counting-parts holder of the container exited from;
	surreptitiously move the actor to the former exterior.

Report an actor exiting (this is the standard report exiting rule):
	if the actor is the player:
		issue library message exiting action number 3 for the container exited from;
	otherwise:
		issue actor-based library message exiting action number 6 for the container exited from;
	continue the action.

Report an actor exiting (this is the describe room emerged into rule):
	if the actor is the player,
		produce a room description with going spacing conventions.

Getting off is an action with past participle got, applying to one thing.
The getting off action translates into I6 as "GetOff".

The specification of the getting off action is "The getting off action is for
actors who are currently on top of a supporter: perhaps standing on a platform,
but maybe only sitting on a chair or even lying down in bed. Unlike the similar
exiting action, getting off takes a noun: the platform, chair, bed or what
have you."

Check an actor getting off (this is the can't get off things rule):
	if the actor is on the noun, continue the action;
	if the actor is carried by the noun, continue the action;
	stop the action with library message getting off action number 1 for the noun.

Carry out an actor getting off (this is the standard getting off rule):
	let the former exterior be the not-counting-parts holder of the noun;
	surreptitiously move the actor to the former exterior.

Report an actor getting off (this is the standard report getting off rule):
	if the actor is the player:
		issue library message exiting action number 3 for the noun;
	otherwise:
		issue actor-based library message exiting action number 5 for the noun;
	continue the action.

Report an actor getting off (this is the describe room stood up into rule):
	if the actor is the player,
		produce a room description with going spacing conventions.

Section SR4/4 - Actions concerning the actor's vision

Looking is an action applying to nothing.
The looking action translates into I6 as "Look".

The specification of the looking action is "The looking action describes the
player's current room and any visible items, but is made more complicated
by the problem of visibility. Inform calculates this by dividing the room
into visibility levels. For an actor on the floor of a room, there is only
one such level: the room itself. But an actor sitting on a chair inside
a packing case which is itself on a gantry would have four visibility levels:
chair, case, gantry, room. The looking rules use a special phrase, 'the
visibility-holder of X', to go up from one level to the next: thus the
visibility-holder of the case is the gantry.

The 'visibility level count' is the number of levels which the player can
actually see, and the 'visibility ceiling' is the uppermost visible level.
For a player standing on the floor of a lighted room, this will be a count
of 1 with the ceiling set to the room. But a player sitting on a chair in
a closed opaque packing case would have visibility level count 2, and
visibility ceiling equal to the case. Moreover, light has to be available
in order to see anything at all: if the player is in darkness, the level
count is 0 and the ceiling is nothing.

Finally, note that several actions other than looking also produce room
descriptions in some cases. The most familiar is going, but exiting a
container or getting off a supporter will also generate a room description.
(The phrase used by the relevant rules is 'produce a room description with
going spacing conventions' and carry out or report rules for newly written
actions are welcome to use this too if they would like to. The spacing
conventions affect paragraph division, and note that the main description
paragraph may be omitted for a place not newly visited, depending on the
VERBOSE settings.) Room descriptions like this are produced by running the
check, carry out and report rules for looking, but are not subject to
before, instead or after rules: so they do not count as a new action. The
looking variable 'room-describing action' holds the action name of the
reason a room description is currently being made: if the player typed
LOOK, this will indeed be set to the looking action, but if we're
describing a room just reached by GO EAST, say, it will be set to the going
action. This can be used to customise carry out looking rules so that
different forms of description are used on going to a room as compared with
looking around while already there."

The looking action has an action-name called the room-describing action.
The looking action has a truth state called abbreviated form allowed.
The looking action has a number called the visibility level count.
The looking action has an object called the visibility ceiling.

Setting action variables for looking (this is the determine visibility ceiling
	rule):
	if the actor is the player, calculate visibility ceiling at low level;
	now the visibility level count is the visibility ceiling count calculated;
	now the visibility ceiling is the visibility ceiling calculated;
	now the room-describing action is the looking action.

Carry out looking (this is the room description heading rule):
	say bold type;
	if the visibility level count is 0:
		begin the printing the name of a dark room activity;
		if handling the printing the name of a dark room activity,
			issue miscellaneous library message number 71;
		end the printing the name of a dark room activity;
	otherwise if the visibility ceiling is the location:
		say "[visibility ceiling]";
	otherwise:
		say "[The visibility ceiling]";
	say roman type;
	let intermediate level be the visibility-holder of the actor;
	repeat with intermediate level count running from 2 to the visibility level count:
		issue library message looking action number 8 for the intermediate level;
		let the intermediate level be the visibility-holder of the intermediate level;
	say line break;
	say run paragraph on with special look spacing.

Carry out looking (this is the room description body text rule):
	if the visibility level count is 0:
		if set to abbreviated room descriptions, continue the action;
		if set to sometimes abbreviated	room descriptions and
			abbreviated form allowed is true and
			darkness witnessed is true,
			continue the action;
		begin the printing the description of a dark room activity;
		if handling the printing the description of a dark room activity,
			issue miscellaneous library message number 17;
		end the printing the description of a dark room activity;
	otherwise if the visibility ceiling is the location:
		if set to abbreviated room descriptions, continue the action;
		if set to sometimes abbreviated	room descriptions and abbreviated form
			allowed is true and the location is visited, continue the action;
		print the location's description;

Carry out looking (this is the room description paragraphs about objects rule):
	if the visibility level count is greater than 0:
		let the intermediate position be the actor;
		let the IP count be the visibility level count;
		while the IP count is greater than 0:
			now the intermediate position is marked for listing;
			let the intermediate position be the visibility-holder of the
				intermediate position;
			decrease the IP count by 1;
		let the top-down IP count be the visibility level count;
		while the top-down IP count is greater than 0:
			let the intermediate position be the actor;
			let the IP count be 0;
			while the IP count is less than the top-down IP count:
				let the intermediate position be the visibility-holder of the
					intermediate position;
				increase the IP count by 1;
			[if we ever support I6-style inside descriptions, here's where]
			describe locale for the intermediate position;
			decrease the top-down IP count by 1;
	continue the action;

Carry out looking (this is the check new arrival rule):
	if in darkness:
		now the darkness witnessed is true;
	otherwise:
		if the location is a room, now the location is visited;

Report an actor looking (this is the other people looking rule):
	if the actor is not the player,
		issue actor-based library message looking action number 9.

Examining is an action applying to one visible thing and requiring light.
The examining action translates into I6 as "Examine".

The specification of the examining action is "The act of looking closely at
something. Note that the noun could be either a direction or a thing, which
is why the Standard Rules include the 'examine directions rule' to deal with
directions: it simply says 'You see nothing unexpected in that direction.'
and stops the action. (If you would like to handle directions differently,
list another rule instead of this one in the carry out examining rules.)

For arcane reasons to do with the Inform 6 library underlying what Inform
does, these rules test to see 'if the noun goes undescribed by source
text' (rather than more simply testing whether the description property
of the noun is blank). If so, we search (i.e., look inside) a container,
and show the status (switched on or off) of a device, but otherwise
give up with a bland response. For traditional reasons, we also show
the status of a device as a second paragraph even after any description
property has been printed: this is done by the examine described devices
rule."

Carry out examining (this is the examine undescribed containers rule):
	if the noun goes undescribed by source text and
		the noun is a container,
		convert to the searching action on the noun.

Carry out examining (this is the examine undescribed devices rule):
	if the noun goes undescribed by source text and
		the noun is a device,
		stop the action with library message examining action number 3
			for the noun.

Carry out examining (this is the examine undescribed things rule):
	if the noun goes undescribed by source text,
		stop the action with library message examining action number 2
			for the noun.

Carry out examining (this is the examine directions rule):
	if the noun is a direction,
		stop the action with library message examining action number 5
			for the noun.

Carry out examining (this is the standard examining rule):
	say "[the description of the noun][line break]".

Carry out examining (this is the examine described devices rule):
	if the noun is a device,
		stop the action with library message examining action number 3
			for the noun.

Report an actor examining (this is the report other people examining rule):
	if the actor is not the player,
		issue actor-based library message examining action number 4 for the noun.

Looking under is an action applying to one visible thing and requiring light.
The looking under action translates into I6 as "LookUnder".

The specification of the looking under action is "The standard Inform world
model does not have a concept of things being under other things, so this
action is only minimally provided by the Standard Rules, but it exists here
for traditional reasons (and because, after all, LOOK UNDER TABLE is the
sort of command which ought to be recognised even if it does nothing useful).
The action ordinarily either tells the player he finds nothing of interest,
or reports that somebody else has looked under something.

The usual way to make this action do something useful is to write a rule
like 'Instead of looking under the cabinet for the first time: now the
player has the silver key; say ...' and so on."

Carry out an actor looking under (this is the standard looking under rule):
	stop the action with library message looking under action number 2 for
		the noun.

Report an actor looking under (this is the report other people looking under rule):
	if the actor is not the player,
		issue actor-based library message looking under action number 3 for the noun.

Searching is an action applying to one thing and requiring light.
The searching action translates into I6 as "Search".

The specification of the searching action is "Searching looks at the contents
of an open or transparent container, or at the items on top of a supporter.
These are often mentioned in room descriptions already, and then the action
is unnecessary, but that wouldn't be true for something like a kitchen
cupboard which is scenery - mentioned in passing in a room description, but
not made a fuss of. Searching such a cupboard would then, by listing its
contents, give the player more information than the ordinary room description
shows.

The usual check rules restrict searching to containers and supporters: so
the Standard Rules do not allow the searching of people, for instance. But
it is easy to add instead rules ('Instead of searching Dr Watson: ...') or
even a new carry out rule ('Check searching someone (called the suspect): ...')
to extend the way searching normally works."

Check an actor searching (this is the can't search unless container or supporter rule):
	if the noun is not a container and the noun is not a supporter,
		stop the action with library message searching action number 4 for
			the noun.

Check an actor searching (this is the can't search closed opaque containers rule):
	if the noun is a closed opaque container,
		stop the action with library message searching action number 5 for
			the noun.

Report searching a container (this is the standard search containers rule):
	if the noun contains a described thing which is not scenery,
		issue library message searching action number 7 for the noun;
	otherwise
		issue library message searching action number 6 for the noun.

Report searching a supporter (this is the standard search supporters rule):
	if the noun supports a described thing which is not scenery,
		issue library message searching action number 3 for the noun;
	otherwise
		issue library message searching action number 2 for the noun.

Report an actor searching (this is the report other people searching rule):
	if the actor is not the player,
		issue actor-based library message searching action number 8 for the noun.

Consulting it about is an action applying to one thing and one topic.
The consulting it about action translates into I6 as "Consult".

The specification of the consulting it about action is "Consulting is a very
flexible and potentially powerful action, but only because it leaves almost
all of the work to the author to deal with directly. The idea is for it to
respond to commands such as LOOK UP HENRY FITZROY IN HISTORY BOOK, where
the topic would be the snippet of command HENRY FITZROY and the thing would
be the book.

The Standard Rules simply parry such requests by saying that the player finds
nothing of interest. All interesting responses must be provided by the author,
using rules like 'Instead of consulting the history book about...'"

Report an actor consulting something about (this is the block consulting rule):
	if the actor is the player,
		issue library message consulting it about action number 1 for the noun;
	otherwise
		issue actor-based library message consulting it about action number 2 for the noun.

Section SR4/5 - Actions which change the state of things

Locking it with is an action applying to one thing and one carried thing.
The locking it with action translates into I6 as "Lock".

The specification of the locking it with action is "Locking is the act of
using an object such as a key to ensure that something such as a door or
container cannot be opened unless first unlocked. (Only closed things can be
locked.)

Locking can be performed on any kind of thing which provides the either/or
properties lockable, locked, openable and open. The 'can't lock without a lock
rule' tests to see if the noun both provides the lockable property, and if
it is in fact lockable: it is then assumed that the other properties can
safely be checked. In the Standard Rules, the container and door kinds both
satisfy these requirements.

We can create a new kind on which opening, closing, locking and unlocking
will work thus: 'A briefcase is a kind of thing. A briefcase can be openable.
A briefcase can be open. A briefcase can be lockable. A briefcase can be
locked. A briefcase is usually openable, lockable, open and unlocked.'

Inform checks whether the key fits using the 'can't lock without the correct
key rule'. To satisfy this, the actor must be directly holding the second
noun, and it must be the current value of the 'matching key' property for
the noun. (This property is seldom referred to directly because it is
automatically set by assertions like 'The silver key unlocks the wicket
gate.')

The Standard Rules provide locking and unlocking actions at a fairly basic
level: they can be much enhanced using the extension Locksmith by Emily
Short, which is included with all distributions of Inform."

Check an actor locking something with (this is the can't lock without a lock rule):
	if the noun provides the property lockable and the noun is lockable,
		continue the action;
	stop the action with library message locking it with action number 1 for the noun.

Check an actor locking something with (this is the can't lock what's already
	locked rule):
	if the noun is locked,
		stop the action with library message locking it with action number 2 for the noun.

Check an actor locking something with (this is the can't lock what's open rule):
	if the noun is open,
		stop the action with library message locking it with action number 3 for the noun.

Check an actor locking something with (this is the can't lock without the correct key rule):
	if the holder of the second noun is not the actor or
		the noun does not provide the property matching key or
		the matching key of the noun is not the second noun,
		stop the action with library message locking it with action number 4 for the second noun.

Carry out an actor locking something with (this is the standard locking rule):
	now the noun is locked.

Report an actor locking something with (this is the standard report locking rule):
	if the actor is the player:
		issue library message locking it with action number 5 for the noun;
	otherwise:
		if the actor is visible, issue actor-based library message locking it with
			action number 6 for the noun;

Unlocking it with is an action applying to one thing and one carried thing.
The unlocking it with action translates into I6 as "Unlock".

The specification of the unlocking it with action is "Unlocking undoes the
effect of locking, and renders the noun openable again provided that the
actor is carrying the right key (which must be the second noun).

Unlocking can be performed on any kind of thing which provides the either/or
properties lockable, locked, openable and open. The 'can't unlock without a lock
rule' tests to see if the noun both provides the lockable property, and if
it is in fact lockable: it is then assumed that the other properties can
safely be checked. In the Standard Rules, the container and door kinds both
satisfy these requirements.

We can create a new kind on which opening, closing, locking and unlocking
will work thus: 'A briefcase is a kind of thing. A briefcase can be openable.
A briefcase can be open. A briefcase can be lockable. A briefcase can be
locked. A briefcase is usually openable, lockable, open and unlocked.'

Inform checks whether the key fits using the 'can't unlock without the correct
key rule'. To satisfy this, the actor must be directly holding the second
noun, and it must be the current value of the 'matching key' property for
the noun. (This property is seldom referred to directly because it is
automatically set by assertions like 'The silver key unlocks the wicket
gate.')

The Standard Rules provide locking and unlocking actions at a fairly basic
level: they can be much enhanced using the extension Locksmith by Emily
Short, which is included with all distributions of Inform."

Check an actor unlocking something with (this is the can't unlock without a lock rule):
	if the noun provides the property lockable and the noun is lockable,
		continue the action;
	stop the action with library message unlocking it with action number 1 for the noun.

Check an actor unlocking something with (this is the can't unlock what's already unlocked rule):
	if the noun is not locked,
		stop the action with library message unlocking it with action number 2 for the noun.

Check an actor unlocking something with (this is the can't unlock without the correct key rule):
	if the holder of the second noun is not the actor or
		the noun does not provide the property matching key or
		the matching key of the noun is not the second noun,
		stop the action with library message unlocking it with action number 3 for the
			second noun.

Carry out an actor unlocking something with (this is the standard unlocking rule):
	now the noun is not locked.

Report an actor unlocking something with (this is the standard report unlocking rule):
	if the actor is the player:
		issue library message unlocking it with action number 4 for the noun;
	otherwise:
		if the actor is visible, issue actor-based library message unlocking it with
			action number 5 for the noun;

Switching on is an action applying to one thing.
The switching on action translates into I6 as "SwitchOn".

The specification of the switching on action is "The switching on and switching
off actions are for the simplest kind of machinery operation: they are for
objects representing machines (or more likely parts of machines), which are
considered to be either off or on at any given time.

The actions are intended to be used where the noun is a device, but in fact
they could work just as well with any kind which can be 'switched on' or
'switched off'."

Check an actor switching on (this is the can't switch on unless switchable rule):
	if the noun provides the property switched on, continue the action;
	stop the action with library message switching on action number 1 for the noun.

Check an actor switching on (this is the can't switch on what's already on rule):
	if the noun is switched on,
		stop the action with library message switching on action number 2 for the noun.

Carry out an actor switching on (this is the standard switching on rule):
	now the noun is switched on.

Report an actor switching on (this is the standard report switching on rule):
	if the actor is the player, issue library message switching on action number 3
		for the noun;
	otherwise issue actor-based library message switching on action number 4 for the noun;

Switching off is an action applying to one thing.
The switching off action translates into I6 as "SwitchOff".

The specification of the switching off action is "The switching off and switching
on actions are for the simplest kind of machinery operation: they are for
objects representing machines (or more likely parts of machines), which are
considered to be either off or on at any given time.

The actions are intended to be used where the noun is a device, but in fact
they could work just as well with any kind which can be 'switched on' or
'switched off'."

Check an actor switching off (this is the can't switch off unless switchable rule):
	if the noun provides the property switched on, continue the action;
	stop the action with library message switching off action number 1 for the noun.

Check an actor switching off (this is the can't switch off what's already off rule):
	if the noun is switched off,
		stop the action with library message switching off action number 2 for the noun.

Carry out an actor switching off (this is the standard switching off rule):
	now the noun is switched off.

Report an actor switching off (this is the standard report switching off rule):
	if the actor is the player, issue library message switching off action number 3
		for the noun;
	otherwise issue actor-based library message switching off action number 4 for the noun;

Opening is an action applying to one thing.
The opening action translates into I6 as "Open".

The specification of the opening action is "Opening makes something no longer
a physical barrier. The action can be performed on any kind of thing which
provides the either/or properties openable and open. The 'can't open unless
openable rule' tests to see if the noun both can be and actually is openable.
(It is assumed that anything which can be openable can also be open.)
In the Standard Rules, the container and door kinds both satisfy these
requirements.

In the event that the thing to be opened is also lockable, we are forbidden
to open it when it is locked. Both containers and doors can be lockable,
but the opening and closing actions would also work fine with kinds which
cannot be.

We can create a new kind on which opening and closing will work thus:
'A case file is a kind of thing. A case file can be openable.
A case file can be open. A case file is usually openable and closed.'

The meaning of open and closed is different for different kinds of thing.
When a container is closed, that means people outside cannot reach in,
and vice versa; when a door is closed, people cannot use the 'going' action
to pass through it. If we were to create a new kind such as 'case file',
we would also need to write rules to make the open and closed properties
interesting for this kind."

Check an actor opening (this is the can't open unless openable rule):
	if the noun provides the property openable and the noun is openable,
		continue the action;
	stop the action with library message opening action number 1 for the noun.

Check an actor opening (this is the can't open what's locked rule):
	if the noun provides the property lockable and the noun is locked,
		stop the action with library message opening action number 2 for the noun.

Check an actor opening (this is the can't open what's already open rule):
	if the noun is open,
		stop the action with library message opening action number 3 for the noun.

Carry out an actor opening (this is the standard opening rule):
	now the noun is open.

Report an actor opening (this is the reveal any newly visible interior rule):
	if the actor is the player and
		the noun is an opaque container and
		the first thing held by the noun is not nothing and
		the noun does not enclose the actor,
		stop the action with library message opening action number 4 for the noun.

Report an actor opening (this is the standard report opening rule):
	if the actor is the player:
		issue library message opening action number 5 for the noun;
	otherwise if the player can see the actor:
		issue actor-based library message opening action number 6 for the noun;
	otherwise:
		issue actor-based library message opening action number 7 for the noun;

Closing is an action applying to one thing.
The closing action translates into I6 as "Close".

The specification of the closing action is "Closing makes something into
a physical barrier. The action can be performed on any kind of thing which
provides the either/or properties openable and open. The 'can't close unless
openable rule' tests to see if the noun both can be and actually is openable.
(It is assumed that anything which can be openable can also be open, and
hence can also be closed.) In the Standard Rules, the container and door
kinds both satisfy these requirements.

We can create a new kind on which opening and closing will work thus:
'A case file is a kind of thing. A case file can be openable.
A case file can be open. A case file is usually openable and closed.'

The meaning of open and closed is different for different kinds of thing.
When a container is closed, that means people outside cannot reach in,
and vice versa; when a door is closed, people cannot use the 'going' action
to pass through it. If we were to create a new kind such as 'case file',
we would also need to write rules to make the open and closed properties
interesting for this kind."

Check an actor closing (this is the can't close unless openable rule):
	if the noun provides the property openable and the noun is openable,
		continue the action;
	stop the action with library message closing action number 1 for the noun.

Check an actor closing (this is the can't close what's already closed rule):
	if the noun is closed,
		stop the action with library message closing action number 2 for the noun.

Carry out an actor closing (this is the standard closing rule):
	now the noun is closed.

Report an actor closing (this is the standard report closing rule):
	if the actor is the player:
		issue library message closing action number 3 for the noun;
	otherwise if the player can see the actor:
		issue actor-based library message closing action number 4 for the noun;
	otherwise:
		issue actor-based library message closing action number 5 for the noun;

Wearing is an action with past participle worn, applying to one carried thing.
The wearing action translates into I6 as "Wear".

The specification of the wearing action is "The Standard Rules give Inform
only a simple model of clothing. A thing can be worn only if it has the
either/or property of being 'wearable'. (Typing a sentence like 'Mr Jones
wears the Homburg hat.' automatically implies that the hat is wearable,
which is why we only seldom need to use the word 'wearable' directly.)
There is no checking of how much or how little any actor is wearing, or
how incongruous this may appear: nor any distinction between under or
over-clothes.

To put on an article of clothing, the actor must be directly carrying it,
as enforced by the 'can't wear what's not held rule'."

Check an actor wearing (this is the can't wear what's not clothing rule):
	if the noun is not a thing or the noun is not wearable,
		stop the action with library message wearing action number 1 for the noun.

Check an actor wearing (this is the can't wear what's not held rule):
	if the holder of the noun is not the actor,
		stop the action with library message wearing action number 2 for the noun.

Check an actor wearing (this is the can't wear what's already worn rule):
	if the actor is wearing the noun,
		stop the action with library message wearing action number 3 for the noun.

Carry out an actor wearing (this is the standard wearing rule):
	now the actor wears the noun.

Report an actor wearing (this is the standard report wearing rule):
	if the actor is the player, issue library message wearing action number 4
		for the noun;
	otherwise issue actor-based library message wearing action number 5
		for the noun.

Taking off is an action with past participle taken, applying to one carried thing.
The taking off action translates into I6 as "Disrobe".

The specification of the taking off action is "The Standard Rules give Inform
only a simple model of clothing. A thing can be worn only if it has the
either/or property of being 'wearable'. (Typing a sentence like 'Mr Jones
wears the Homburg hat.' automatically implies that the hat is wearable,
which is why we only seldom need to use the word 'wearable' directly.)
There is no checking of how much or how little any actor is wearing, or
how incongruous this may appear: nor any distinction between under or
over-clothes.

When an article of clothing is taken off, it becomes a thing directly
carried by its former wearer, rather than being (say) dropped onto the floor."

Check an actor taking off (this is the can't take off what's not worn rule):
	if the actor is not wearing the noun,
		stop the action with library message taking off action number 1 for the noun.

Carry out an actor taking off (this is the standard taking off rule):
	now the actor carries the noun.

Report an actor taking off (this is the standard report taking off rule):
	if the actor is the player, issue library message taking off action number 2
		for the noun;
	otherwise issue actor-based library message taking off action number 3 for the noun.

Section SR4/6 - Actions concerning other people

Giving it to is an action with past participle given, applying to one carried thing and one thing.
The giving it to action translates into I6 as "Give".

The specification of the giving it to action is "This action is indexed by
Inform under 'Actions concerning other people', but it could just as easily
have gone under 'Actions concerning the actor's possessions' because -
like dropping, putting it on or inserting it into - this is an action
which gets rid of something being carried.

The Standard Rules implement this action fully - if it reaches the carry
out and report rulebooks, then the item is indeed transferred to the
recipient, and this is properly reported. But giving something to
somebody is not like putting something on a shelf: the recipient has
to agree. The final check rule, the 'block giving rule', assumes that
the recipient does not consent - so the gift fails to happen. The way
to make the giving action use its abilities fully is to replace the
block giving rule with a rule which makes a more sophisticated decision
about who will accept what from whom, and only blocks some attempts,
letting others run on into the carry out and report rules."

Check an actor giving something to (this is the can't give what you haven't got rule):
	if the actor is not the holder of the noun,
		stop the action with library message giving it to action number 1 for the noun.

Check an actor giving something to (this is the can't give to yourself rule):
	if the actor is the second noun,
		stop the action with library message giving it to action number 2 for the noun.

Check an actor giving something to (this is the can't give to a non-person rule):
	if the second noun is not a person,
		stop the action with library message giving it to action number 4 for the
			second noun.

Check an actor giving something to (this is the block giving rule):
	stop the action with library message giving it to action number 3 for the
		second noun.

Carry out an actor giving something to (this is the standard giving rule):
	move the noun to the second noun.

Report an actor giving something to (this is the standard report giving rule):
	if the actor is the player:
		issue library message giving it to action number 5 for the noun;
	otherwise if the second noun is the player:
		issue actor-based library message giving it to action number 6 for the noun;
	otherwise:
		issue actor-based library message giving it to action number 7 for the noun;

Showing it to is an action with past participle shown, applying to one carried thing
	and one visible thing.
The showing it to action translates into I6 as "Show".

The specification of the showing it to action is "Anyone can show anyone
else something which they are carrying, but not some nearby piece of
scenery, say - so this action is suitable for showing the emerald locket
to Katarina, but not showing the Orange River Rock Room to Mr Douglas.

The Standard Rules implement this action in only a minimal way, checking
that it makes sense but then blocking all such attempts with a message
such as 'Katarina is not interested.' - this is the task of the 'block
showing rule'. As a result, there are no carry out or report rules. To
make it into a systematic and interesting action, we would need to
unlist the block showing rule and then to write carry out and report
rules: but usually for IF purposes we only need to make a handful of
special cases of showing work properly, and for those we can simply
write Instead rules to handle them."

Check an actor showing something to (this is the can't show what you haven't
	got rule):
	if the actor is not the holder of the noun,
		stop the action with library message showing it to action number 1
			for the noun.

Check an actor showing something to (this is the convert show to yourself to
	examine rule):
	if the actor is the second noun,
		convert to the examining action on the noun.

Check an actor showing something to (this is the block showing rule):
	stop the action with library message showing it to action number 2
		for the second noun.

Waking is an action with past participle woken, applying to one thing.
The waking action translates into I6 as "WakeOther".

The specification of the waking action is "This is the act of jostling
a sleeping person to wake him or her up, and it finds its way into the
Standard Rules only for historical reasons. Inform does not by default
provide any model for people being asleep or awake, so this action does
not do anything in the standard implementation: instead, it is always
stopped by the block waking rule."

Check an actor waking (this is the block waking rule):
	stop the action with library message waking action number 1 for the noun.

Throwing it at is an action with past participle thrown, applying to one carried
	thing and one visible thing.
The throwing it at action translates into I6 as "ThrowAt".

The specification of the throwing it at action is "Throwing something at
someone or something is difficult for Inform to model. So many considerations
apply: just because the actor can see the target, does it follow that the
target can accurately hit it? What if the projectile is heavy, like an
anvil, or something not easily aimable, like a feather? What if there
is a barrier in the way, like a cage with bars spaced so that only items
of a certain size get through? And then: what should happen as a result?
Will the projectile break, or do damage, or fall to the floor, or into
a container or onto a supporter? And so on.

Because it seems hopeless to try to model this in any general way,
Inform instead provides the action for the user to attach specific rules to.
The check rules in the Standard Rules simply require that the projectile
is not an item of clothing still worn (this will be relevant for women
attending a Tom Jones concert) but then, in either the 'futile to throw
things at inanimate objects rule' or the 'block throwing at rule', will
refuse to carry out the action with a bland message.

To make throwing do something, then, we must either write Instead rules
for special circumstances, or else unlist these check rules and write
suitable carry out and report rules to pick up the thread."

Check an actor throwing something at (this is the implicitly remove thrown clothing rule):
	if the actor is wearing the noun:
		issue library message dropping action number 3 for the noun;
		silently try the actor trying taking off the noun;
		if the actor is wearing the noun, stop the action;

Check an actor throwing something at (this is the futile to throw things at inanimate
	objects rule):
	if the second noun is not a person,
		stop the action with library message throwing it at action number 1
			for the second noun.

Check an actor throwing something at (this is the block throwing at rule):
	stop the action with library message throwing it at action number 2
		for the noun.

Attacking is an action applying to one thing.
The attacking action translates into I6 as "Attack".

The specification of the attacking action is "Violence is seldom the answer,
and attempts to attack another person are normally blocked as being unrealistic
or not seriously meant. (I might find a shop assistant annoying, but IF is
not Grand Theft Auto, and responding by killing him is not really one of
my options.) So the Standard Rules simply block attempts to fight people,
but the action exists for rules to make exceptions."

Check an actor attacking (this is the block attacking rule):
	stop the action with library message attacking action number 1 for the noun.

Kissing is an action applying to one thing.
The kissing action translates into I6 as "Kiss".

The specification of the kissing action is "Possibly because Inform was
originally written by an Englishman, attempts at kissing another person are
normally blocked as being unrealistic or not seriously meant. So the
Standard Rules simply block attempts to kiss people, but the action exists
for rules to make exceptions."

Check an actor kissing (this is the kissing yourself rule):
	if the noun is the actor,
		stop the action with library message touching action number 3 for the noun.

Check an actor kissing (this is the block kissing rule):
	stop the action with library message kissing action number 1 for the noun.

Answering it that is an action applying to one thing and one topic.
The answering it that action translates into I6 as "Answer".

The specification of the answering it that action is "The Standard Rules do
not include any systematic way to handle conversation: instead, Inform is
set up so that it is as easy as we can make it to write specific rules
handling speech in particular games, and so that if no such rules are
written then all attempts to communicate are gracefully if not very
interestingly rejected.

The topic here can be any double-quoted text, which can itself contain
tokens in square brackets: see the documentation on Understanding.

Answering is an action existing so that the player can say something free-form
to somebody else. A convention of IF is that a command such as DAPHNE, TAKE
MASK is a request to Daphne to perform an action: if the persuasion rules in
force mean that she consents, the action 'Daphne taking the mask' does
indeed then result. But if the player types DAPHNE, 12375 or DAPHNE, GREAT
HEAVENS - or anything else not making sense as a command - the action
'answering Daphne that ...' will be generated.

The name of the action arises because it is also caused by typing, say,
ANSWER 12375 when Daphne (say) has asked a question."

Report an actor answering something that (this is the block answering rule):
	stop the action with library message answering it that action number 1
		for the noun.

Telling it about is an action with past participle told, applying to one thing and one topic.
The telling it about action translates into I6 as "Tell".

The specification of the telling it about action is "The Standard Rules do
not include any systematic way to handle conversation: instead, Inform is
set up so that it is as easy as we can make it to write specific rules
handling speech in particular games, and so that if no such rules are
written then all attempts to communicate are gracefully if not very
interestingly rejected.

The topic here can be any double-quoted text, which can itself contain
tokens in square brackets: see the documentation on Understanding.

Telling is an action existing only to catch commands like TELL ALEX ABOUT
GUITAR. Customarily in IF, such a command is shorthand which the player
accepts as a conventional form: it means 'tell Alex what I now know about
the guitar' and would make sense if the player had himself recently
discovered something significant about the guitar which might interest
Alex."

Check an actor telling something about (this is the telling yourself rule):
	if the actor is the noun,
		stop the action with library message telling it about action number 1
			for the noun.

Report an actor telling something about (this is the block telling rule):
	stop the action with library message telling it about action number 2
		for the noun.

Asking it about is an action applying to one thing and one topic.
The asking it about action translates into I6 as "Ask".

The specification of the asking it about action is "The Standard Rules do
not include any systematic way to handle conversation: instead, Inform is
set up so that it is as easy as we can make it to write specific rules
handling speech in particular games, and so that if no such rules are
written then all attempts to communicate are gracefully if not very
interestingly rejected.

The topic here can be any double-quoted text, which can itself contain
tokens in square brackets: see the documentation on Understanding.

Asking is an action existing only to catch commands like ASK STEPHEN ABOUT
PENELOPE. Customarily in IF, such a command is shorthand which the player
accepts as a conventional form: it means 'engage Mary in conversation and
try to find out what she might know about'. It's understood as a convention
of the genre that Mary should not be expected to respond in cases where
there is no reason to suppose that she has anything relevant to pass on -
ASK JANE ABOUT RICE PUDDING, for instance, need not conjure up a recipe
even if Jane is a 19th-century servant and therefore almost certainly
knows one."

Report an actor asking something about (this is the block asking rule):
	stop the action with library message asking it about action number 1
		for the noun.

Asking it for is an action applying to two things.
The asking it for action translates into I6 as "AskFor".

The specification of the asking it for action is "The Standard Rules do
not include any systematic way to handle conversation, but this is
action is not quite conversation: it doesn't involve any spoken text as
such. It exists to catch commands like ASK SALLY FOR THE EGG WHISK,
where the whisk is something which Sally has and the player can see.

Slightly oddly, but for historical reasons, an actor asking himself for
something is treated to an inventory listing instead. All other cases
are converted to the giving action: that is, ASK SALLY FOR THE EGG WHISK
is treated as if it were SALLY, GIVE ME THE EGG WHISK - an action for
Sally to perform and which then follows rules for giving.

To ask for information or something intangible, see the asking it about
action."

Check an actor asking something for (this is the asking yourself for something rule):
	if the actor is the noun and the actor is the player,
		try taking inventory instead.

Check an actor asking something for (this is the translate asking for to giving rule):
	convert to request of the noun to perform giving it to action with the
		second noun and the actor.

Section SR4/7 - Actions which are checked but then do nothing unless rules intervene

Waiting is an action applying to nothing.
The waiting action translates into I6 as "Wait".

The specification of the waiting action is "The inaction action: where would
we be without waiting? Waiting does not cause time to pass by - that happens
anyway - but represents a positive choice by the actor not to fill that time.
It is an action so that rules can be attached to it: for instance, we could
imagine that a player who consciously decides to sit and wait might notice
something which a busy player does not, and we could write a rule accordingly.

Note the absence of check or carry out rules - anyone can wait, at any time,
and it makes nothing happen."

Report an actor waiting (this is the standard report waiting rule):
	if the actor is the player, stop the action with library message waiting
		action number 1 for the actor;
	issue actor-based library message waiting action number 2.

Touching is an action applying to one thing.
The touching action translates into I6 as "Touch".

The specification of the touching action is "Touching is just that, touching
something without applying pressure: a touch-sensitive screen or a living
creature might react, but a standard push-button or lever will probably not.

In the Standard Rules there are no check touching rules, since touchability
is already a requirement of the noun for the action anyway, and no carry out
rules because nothing in the standard Inform world model reacts to
a mere touch - though report rules do mean that attempts to touch other
people provoke a special reply."

Report an actor touching (this is the report touching yourself rule):
	if the noun is the actor:
		if the actor is the player, issue library message touching action number 3
			for the noun;
		otherwise issue actor-based library message touching action number 4;
		stop the action;
	continue the action.

Report an actor touching (this is the report touching other people rule):
	if the noun is a person:
		if the actor is the player:
			issue library message touching action number 1 for the noun;
		otherwise if the noun is the player:
			issue actor-based library message touching action number 5;
		otherwise:
			issue actor-based library message touching action number 6 for the noun;
		stop the action;
	continue the action.

Report an actor touching (this is the report touching things rule):
	if the actor is the player, issue library message touching action number 2
		for the noun;
	otherwise issue actor-based library message touching action number 6 for the noun.

Waving is an action applying to one thing.
The waving action translates into I6 as "Wave".

The specification of the waving action is "Waving in this sense is like
waving a sceptre: the item to be waved must be directly held (or worn)
by the actor.

In the Standard Rules there are no carry out rules for this action because
nothing in the standard Inform world model which reacts to it. The action
is provided for authors to hang more interesting behaviour onto for special
cases: say, waving a particular rusty iron rod with a star on the end."

Check an actor waving (this is the can't wave what's not held rule):
	if the actor is not the holder of the noun,
		stop the action with library message waving action number 1 for the noun.

Report an actor waving (this is the report waving things rule):
	if the actor is the player, issue library message waving action number 2
		for the noun;
	otherwise issue actor-based library message waving action number 3 for the noun.

Pulling is an action applying to one thing.
The Pulling action translates into I6 as "Pull".

The specification of the pulling action is "Pulling is the act of pulling
something not grossly larger than the actor by an amount which would not
substantially move it.

In the Standard Rules there are no carry out rules for this action because
nothing in the standard Inform world model which reacts to it. The action
is provided for authors to hang more interesting behaviour onto for special
cases: say, pulling a lever. ('The big red lever is a fixed in place device.
Instead of pulling the big red lever, try switching on the lever. Instead
of pushing the big red lever, try switching off the lever.')"

Check an actor pulling (this is the can't pull what's fixed in place rule):
	if the noun is fixed in place,
		stop the action with library message pulling action number 1 for the noun.

Check an actor pulling (this is the can't pull scenery rule):
	if the noun is scenery,
		stop the action with library message pulling action number 2 for the noun.

Check an actor pulling (this is the can't pull people rule):
	if the noun is a person,
		stop the action with library message pulling action number 4 for the noun.

Report an actor pulling (this is the report pulling rule):
	if the actor is the player, issue library message pulling action number 3
		for the noun;
	otherwise issue actor-based library message pulling action number 5 for the noun.

Pushing is an action applying to one thing.
The Pushing action translates into I6 as "Push".

The specification of the pushing action is "Pushing is the act of pushing
something not grossly larger than the actor by an amount which would not
substantially move it. (See also the pushing it to action, which involves
a longer-distance push between rooms.)

In the Standard Rules there are no carry out rules for this action because
nothing in the standard Inform world model which reacts to it. The action
is provided for authors to hang more interesting behaviour onto for special
cases: say, pulling a lever. ('The big red lever is a fixed in place device.
Instead of pulling the big red lever, try switching on the lever. Instead
of pushing the big red lever, try switching off the lever.')"

Check an actor pushing something (this is the can't push what's fixed in place rule):
	if the noun is fixed in place,
		stop the action with library message pushing action number 1 for the noun.

Check an actor pushing something (this is the can't push scenery rule):
	if the noun is scenery,
		stop the action with library message pushing action number 2 for the noun.

Check an actor pushing something (this is the can't push people rule):
	if the noun is a person,
		stop the action with library message pushing action number 4 for the noun.

Report an actor pushing something (this is the report pushing rule):
	if the actor is the player, issue library message pushing action number 3
		for the noun;
	otherwise issue actor-based library message pushing action number 6 for the noun.

Turning is an action applying to one thing.
The Turning action translates into I6 as "Turn".

The specification of the turning action is "Turning is the act of rotating
something - say, a dial.

In the Standard Rules there are no carry out rules for this action because
nothing in the standard Inform world model which reacts to it. The action
is provided for authors to hang more interesting behaviour onto for special
cases: say, turning a capstan."

Check an actor turning (this is the can't turn what's fixed in place rule):
	if the noun is fixed in place,
		stop the action with library message turning action number 1 for the noun.

Check an actor turning (this is the can't turn scenery rule):
	if the noun is scenery,
		stop the action with library message turning action number 2 for the noun.

Check an actor turning (this is the can't turn people rule):
	if the noun is a person,
		stop the action with library message turning action number 4 for the noun.

Report an actor turning (this is the report turning rule):
	if the actor is the player, issue library message turning action number 3
		for the noun;
	otherwise issue actor-based library message turning action number 7 for the noun.

Pushing it to is an action applying to one thing and one visible thing.
The Pushing it to action translates into I6 as "PushDir".

The specification of the pushing it to action is "This action covers pushing
a large object, not being carried, so that the actor pushes it from one room
to another: for instance, pushing a bale of hay to the east.

This is rapidly converted into a special form of the going action. If the
noun object has the either/or property 'pushable between rooms', then the
action is converted to going by the 'standard pushing in directions rule'.
If that going action succeeds, then the original pushing it to action
stops; it's only if that fails that we run on into the 'block pushing in
directions rule', which then puts an end to the matter."

Check an actor pushing something to (this is the can't push unpushable things rule):
	if the noun is not pushable between rooms,
		stop the action with library message pushing it to action number 1 for
			the noun.

Check an actor pushing something to (this is the can't push to non-directions rule):
	if the second noun is not a direction,
		stop the action with library message pushing it to action number 2 for
			the noun.

Check an actor pushing something to (this is the can't push vertically rule):
	if the second noun is up or the second noun is down,
		stop the action with library message pushing it to action number 3 for
			the noun.

Check an actor pushing something to (this is the standard pushing in directions rule):
	convert to special going-with-push action.

Check an actor pushing something to (this is the block pushing in directions rule):
	stop the action with library message pushing it to action number 1 for
		the noun.

Squeezing is an action applying to one thing.
The Squeezing action translates into I6 as "Squeeze".

The specification of the squeezing action is "Squeezing is an action which
can conveniently vary from squeezing something hand-held, like a washing-up
liquid bottle, right up to squeezing a pillar in a bear hug.

In the Standard Rules there are no carry out rules for this action because
nothing in the standard Inform world model which reacts to it. The action
is provided for authors to hang more interesting behaviour onto for special
cases. A mildly fruity message is produced to players who attempt to
squeeze people, which is blocked by a check squeezing rule."

Check an actor squeezing (this is the innuendo about squeezing people rule):
	if the noun is a person,
		stop the action with library message squeezing action number 1 for
			the noun.

Report an actor squeezing (this is the report squeezing rule):
	if the actor is the player, issue library message squeezing action number 2
		for the noun;
	otherwise issue actor-based library message squeezing action number 3 for the noun.

Section SR4/8 - Actions which always do nothing unless rules intervene

Saying yes is an action with past participle said and applying to nothing.
The Saying yes action translates into I6 as "Yes".

The specification of the saying yes action is
"The Standard Rules define this action in only a minimal way, blocking it
with a check rule which stops it in all cases. It exists so that before
or instead rules can be written to make it do interesting things in special
cases. (Or to reconstruct the action as something more substantial, unlist
the block rule and supply carry out and report rules, together perhaps
with some further check rules.)"

Check an actor saying yes (this is the block saying yes rule):
	stop the action with library message saying yes action number 1.

Saying no is an action with past participle said and applying to nothing.
The Saying no action translates into I6 as "No".

The specification of the saying no action is
"The Standard Rules define this action in only a minimal way, blocking it
with a check rule which stops it in all cases. It exists so that before
or instead rules can be written to make it do interesting things in special
cases. (Or to reconstruct the action as something more substantial, unlist
the block rule and supply carry out and report rules, together perhaps
with some further check rules.)"

Check an actor saying no (this is the block saying no rule):
	stop the action with library message saying no action number 1.

Burning is an action applying to one thing.
The Burning action translates into I6 as "Burn".

The specification of the burning action is
"The Standard Rules define this action in only a minimal way, blocking it
with a check rule which stops it in all cases. It exists so that before
or instead rules can be written to make it do interesting things in special
cases. (Or to reconstruct the action as something more substantial, unlist
the block rule and supply carry out and report rules, together perhaps
with some further check rules.)"

Check an actor burning (this is the block burning rule):
	stop the action with library message burning action number 1.

Waking up is an action with past participle woken and applying to nothing.
The Waking up action translates into I6 as "Wake".

The specification of the waking up action is
"The Standard Rules define this action in only a minimal way, blocking it
with a check rule which stops it in all cases. It exists so that before
or instead rules can be written to make it do interesting things in special
cases. (Or to reconstruct the action as something more substantial, unlist
the block rule and supply carry out and report rules, together perhaps
with some further check rules.)"

Check an actor waking up (this is the block waking up rule):
	stop the action with library message waking up action number 1.

Thinking is an action past participle thought and applying to nothing.
The Thinking action translates into I6 as "Think".

The specification of the thinking action is
"The Standard Rules define this action in only a minimal way, blocking it
with a check rule which stops it in all cases. It exists so that before
or instead rules can be written to make it do interesting things in special
cases. (Or to reconstruct the action as something more substantial, unlist
the block rule and supply carry out and report rules, together perhaps
with some further check rules.)"

Check an actor thinking (this is the block thinking rule):
	stop the action with library message thinking action number 1.

Smelling is an action applying to one thing.
The Smelling action translates into I6 as "Smell".

The specification of the smelling action is
"The Standard Rules define this action in only a minimal way, blocking it
with a check rule which stops it in all cases. It exists so that before
or instead rules can be written to make it do interesting things in special
cases. (Or to reconstruct the action as something more substantial, unlist
the block rule and supply carry out and report rules, together perhaps
with some further check rules.)"

Check an actor smelling (this is the block smelling rule):
	stop the action with library message smelling action number 1 for the noun.

Listening to is an action applying to one thing.
The Listening to action translates into I6 as "Listen".

The specification of the listening to action is
"The Standard Rules define this action in only a minimal way, blocking it
with a check rule which stops it in all cases. It exists so that before
or instead rules can be written to make it do interesting things in special
cases. (Or to reconstruct the action as something more substantial, unlist
the block rule and supply carry out and report rules, together perhaps
with some further check rules.)"

Check an actor listening (this is the block listening rule):
	stop the action with library message listening to action number 1 for the noun.

Tasting is an action applying to one thing.
The Tasting action translates into I6 as "Taste".

The specification of the tasting action is
"The Standard Rules define this action in only a minimal way, blocking it
with a check rule which stops it in all cases. It exists so that before
or instead rules can be written to make it do interesting things in special
cases. (Or to reconstruct the action as something more substantial, unlist
the block rule and supply carry out and report rules, together perhaps
with some further check rules.)"

Check an actor tasting (this is the block tasting rule):
	stop the action with library message tasting action number 1 for the noun.

Cutting is an action with past participle cut and applying to one thing.
The Cutting action translates into I6 as "Cut".

The specification of the cutting action is
"The Standard Rules define this action in only a minimal way, blocking it
with a check rule which stops it in all cases. It exists so that before
or instead rules can be written to make it do interesting things in special
cases. (Or to reconstruct the action as something more substantial, unlist
the block rule and supply carry out and report rules, together perhaps
with some further check rules.)"

Check an actor cutting (this is the block cutting rule):
	stop the action with library message cutting action number 1 for the noun.

Jumping is an action applying to nothing.
The Jumping action translates into I6 as "Jump".

The specification of the jumping action is
"The Standard Rules define this action in only a minimal way, blocking it
with a check rule which stops it in all cases. It exists so that before
or instead rules can be written to make it do interesting things in special
cases. (Or to reconstruct the action as something more substantial, unlist
the block rule and supply carry out and report rules, together perhaps
with some further check rules.)"

Check an actor jumping (this is the block jumping rule):
	stop the action with library message jumping action number 1.

Tying it to is an action with past participle tied, and applying to two things.
The Tying it to action translates into I6 as "Tie".

The specification of the tying it to action is
"The Standard Rules define this action in only a minimal way, blocking it
with a check rule which stops it in all cases. It exists so that before
or instead rules can be written to make it do interesting things in special
cases. (Or to reconstruct the action as something more substantial, unlist
the block rule and supply carry out and report rules, together perhaps
with some further check rules.)"

Check an actor tying (this is the block tying rule):
	stop the action with library message tying it to action number 1 for the noun.

Drinking is an action with past participle drunk, and applying to one thing.
The Drinking action translates into I6 as "Drink".

The specification of the drinking action is
"The Standard Rules define this action in only a minimal way, blocking it
with a check rule which stops it in all cases. It exists so that before
or instead rules can be written to make it do interesting things in special
cases. (Or to reconstruct the action as something more substantial, unlist
the block rule and supply carry out and report rules, together perhaps
with some further check rules.)"

Check an actor drinking (this is the block drinking rule):
	stop the action with library message drinking action number 1 for the noun.

Saying sorry is an action with past participle said and applying to nothing.
The Saying sorry action translates into I6 as "Sorry".

The specification of the saying sorry action is
"The Standard Rules define this action in only a minimal way, blocking it
with a check rule which stops it in all cases. It exists so that before
or instead rules can be written to make it do interesting things in special
cases. (Or to reconstruct the action as something more substantial, unlist
the block rule and supply carry out and report rules, together perhaps
with some further check rules.)"

Check an actor saying sorry (this is the block saying sorry rule):
	stop the action with library message saying sorry action number 1.

Swearing obscenely is an action censored, with past participle sworn, and applying to nothing.
The Swearing obscenely action translates into I6 as "Strong".

The specification of the swearing obscenely action is
"The Standard Rules define this action in only a minimal way, blocking it
with a check rule which stops it in all cases. It exists so that before
or instead rules can be written to make it do interesting things in special
cases. (Or to reconstruct the action as something more substantial, unlist
the block rule and supply carry out and report rules, together perhaps
with some further check rules.)"

Check an actor swearing obscenely (this is the block swearing obscenely rule):
	stop the action with library message swearing obscenely action number 1.

Swearing mildly is an action censored, with past participle sworn, and applying to nothing.
The Swearing mildly action translates into I6 as "Mild".

The specification of the swearing mildly action is
"The Standard Rules define this action in only a minimal way, blocking it
with a check rule which stops it in all cases. It exists so that before
or instead rules can be written to make it do interesting things in special
cases. (Or to reconstruct the action as something more substantial, unlist
the block rule and supply carry out and report rules, together perhaps
with some further check rules.)"

Check an actor swearing mildly (this is the block swearing mildly rule):
	stop the action with library message swearing mildly action number 1.

Swinging is an action past participle swung and applying to one thing.
The Swinging action translates into I6 as "Swing".

The specification of the swinging action is
"The Standard Rules define this action in only a minimal way, blocking it
with a check rule which stops it in all cases. It exists so that before
or instead rules can be written to make it do interesting things in special
cases. (Or to reconstruct the action as something more substantial, unlist
the block rule and supply carry out and report rules, together perhaps
with some further check rules.)"

Check an actor swinging (this is the block swinging rule):
	stop the action with library message swinging action number 1 for the noun.

Rubbing is an action applying to one thing.
The Rubbing action translates into I6 as "Rub".

The specification of the rubbing action is
"The Standard Rules define this action in only a minimal way, blocking it
with a check rule which stops it in all cases. It exists so that before
or instead rules can be written to make it do interesting things in special
cases. (Or to reconstruct the action as something more substantial, unlist
the block rule and supply carry out and report rules, together perhaps
with some further check rules.)"

Check an actor rubbing (this is the block rubbing rule):
	stop the action with library message rubbing action number 1 for the noun.

Setting it to is an action with past participle set, applying to one thing and one topic.
The Setting it to action translates into I6 as "SetTo".

The specification of the setting it to action is
"The Standard Rules define this action in only a minimal way, blocking it
with a check rule which stops it in all cases. It exists so that before
or instead rules can be written to make it do interesting things in special
cases. (Or to reconstruct the action as something more substantial, unlist
the block rule and supply carry out and report rules, together perhaps
with some further check rules.)"

Check an actor setting something to (this is the block setting it to rule):
	stop the action with library message setting it to action number 1 for the noun.

Waving hands is an action applying to nothing.
The Waving hands action translates into I6 as "WaveHands".

The specification of the waving hands action is
"The Standard Rules define this action in only a minimal way, blocking it
with a check rule which stops it in all cases. It exists so that before
or instead rules can be written to make it do interesting things in special
cases. (Or to reconstruct the action as something more substantial, unlist
the block rule and supply carry out and report rules, together perhaps
with some further check rules.)"

Check an actor waving hands (this is the block waving hands rule):
	stop the action with library message waving hands action number 1.

Buying is an action with past participle bought, and applying to one thing.
The Buying action translates into I6 as "Buy".

The specification of the buying action is
"The Standard Rules define this action in only a minimal way, blocking it
with a check rule which stops it in all cases. It exists so that before
or instead rules can be written to make it do interesting things in special
cases. (Or to reconstruct the action as something more substantial, unlist
the block rule and supply carry out and report rules, together perhaps
with some further check rules.)"

Check an actor buying (this is the block buying rule):
	stop the action with library message buying action number 1 for the noun.

Singing is an action with past participle sung and applying to nothing.
The Singing action translates into I6 as "Sing".

The specification of the singing action is
"The Standard Rules define this action in only a minimal way, blocking it
with a check rule which stops it in all cases. It exists so that before
or instead rules can be written to make it do interesting things in special
cases. (Or to reconstruct the action as something more substantial, unlist
the block rule and supply carry out and report rules, together perhaps
with some further check rules.)"

Check an actor singing (this is the block singing rule):
	stop the action with library message singing action number 1.

Climbing is an action applying to one thing.
The Climbing action translates into I6 as "Climb".

The specification of the climbing action is
"The Standard Rules define this action in only a minimal way, blocking it
with a check rule which stops it in all cases. It exists so that before
or instead rules can be written to make it do interesting things in special
cases. (Or to reconstruct the action as something more substantial, unlist
the block rule and supply carry out and report rules, together perhaps
with some further check rules.)"

Check an actor climbing (this is the block climbing rule):
	stop the action with library message climbing action number 1 for the noun.

Sleeping is an action past participle slept and applying to nothing.
The Sleeping action translates into I6 as "Sleep".

The specification of the sleeping action is
"The Standard Rules define this action in only a minimal way, blocking it
with a check rule which stops it in all cases. It exists so that before
or instead rules can be written to make it do interesting things in special
cases. (Or to reconstruct the action as something more substantial, unlist
the block rule and supply carry out and report rules, together perhaps
with some further check rules.)"

Check an actor sleeping (this is the block sleeping rule):
	stop the action with library message sleeping action number 1.

Section SR4/9 - Actions which happen out of world

Quitting the game is an action out of world and applying to nothing.
The quitting the game action translates into I6 as "Quit".

The quit the game rule is listed in the carry out quitting the game rulebook.
The quit the game rule translates into I6 as "QUIT_THE_GAME_R".

Saving the game is an action out of world and applying to nothing.
The saving the game action translates into I6 as "Save".

The save the game rule is listed in the carry out saving the game rulebook.
The save the game rule translates into I6 as "SAVE_THE_GAME_R".

Restoring the game is an action out of world and applying to nothing.
The restoring the game action translates into I6 as "Restore".

The restore the game rule is listed in the carry out restoring the game rulebook.
The restore the game rule translates into I6 as "RESTORE_THE_GAME_R".

Restarting the game is an action out of world and applying to nothing.
The restarting the game action translates into I6 as "Restart".

The restart the game rule is listed in the carry out restarting the game rulebook.
The restart the game rule translates into I6 as "RESTART_THE_GAME_R".

Verifying the story file is an action out of world, with past participle verified
	and applying to nothing.
The verifying the story file action translates into I6 as "Verify".

The verify the story file rule is listed in the carry out verifying the story file rulebook.
The verify the story file rule translates into I6 as "VERIFY_THE_STORY_FILE_R".

Switching the story transcript on is an action out of world and applying to nothing.
The switching the story transcript on action translates into I6 as "ScriptOn".

The switch the story transcript on rule is listed in the carry out switching the story
	transcript on rulebook.
The switch the story transcript on rule translates into I6 as "SWITCH_TRANSCRIPT_ON_R".

Switching the story transcript off is an action out of world and applying to nothing.
The switching the story transcript off action translates into I6 as "ScriptOff".

The switch the story transcript off rule is listed in the carry out switching the story
	transcript off rulebook.
The switch the story transcript off rule translates into I6 as "SWITCH_TRANSCRIPT_OFF_R".

Requesting the story file version is an action out of world and applying to nothing.
The requesting the story file version action translates into I6 as "Version".

The announce the story file version rule is listed in the carry out requesting the story
	file version rulebook.
The announce the story file version rule translates into I6 as "ANNOUNCE_STORY_FILE_VERSION_R".

Requesting the score is an action out of world and applying to nothing.
The requesting the score action translates into I6 as "Score".

The announce the score rule is listed in the carry out requesting the score rulebook.
The announce the score rule translates into I6 as "ANNOUNCE_SCORE_R".

Preferring abbreviated room descriptions is an action out of world and applying to nothing.
The preferring abbreviated room descriptions action translates into I6 as "LMode3".

The prefer abbreviated room descriptions rule is listed in the carry out preferring
	abbreviated room descriptions rulebook.
The prefer abbreviated room descriptions rule translates into I6 as "PREFER_ABBREVIATED_R".

Preferring unabbreviated room descriptions is an action out of world and applying to nothing.
The preferring unabbreviated room descriptions action translates into I6 as "LMode2".

The prefer unabbreviated room descriptions rule is listed in the carry out preferring
	unabbreviated room descriptions rulebook.
The prefer unabbreviated room descriptions rule translates into I6 as "PREFER_UNABBREVIATED_R".

Preferring sometimes abbreviated room descriptions is an action out of world and
	applying to nothing.
The preferring sometimes abbreviated room descriptions action translates into I6 as "LMode1".

The prefer sometimes abbreviated room descriptions rule is listed in the carry out
	preferring sometimes abbreviated room descriptions rulebook.
The prefer sometimes abbreviated room descriptions rule translates into I6 as
	"PREFER_SOMETIMES_ABBREVIATED_R".

Switching score notification on is an action out of world and applying to nothing.
The switching score notification on action translates into I6 as "NotifyOn".

The switch score notification on rule is listed in the carry out switching score
	notification on rulebook.
The switch score notification on rule translates into I6 as "SWITCH_SCORE_NOTIFY_ON_R".

Switching score notification off is an action out of world and applying to nothing.
The switching score notification off action translates into I6 as "NotifyOff".

The switch score notification off rule is listed in the carry out switching score
	notification off rulebook.
The switch score notification off rule translates into I6 as "SWITCH_SCORE_NOTIFY_OFF_R".

Requesting the pronoun meanings is an action out of world and applying to nothing.
The requesting the pronoun meanings action translates into I6 as "Pronouns".

The announce the pronoun meanings rule is listed in the carry out requesting the
	pronoun meanings rulebook.
The announce the pronoun meanings rule translates into I6 as "ANNOUNCE_PRONOUN_MEANINGS_R".

The understand token a time period translates into I6 as "RELATIVE_TIME_TOKEN".

Section SR4/10 - Grammar

Understand "take [things]" as taking.
Understand "take off [something]" as taking off.
Understand "take [things inside] from [something]" as removing it from.
Understand "take [things inside] off [something]" as removing it from.
Understand "take inventory" as taking inventory.
Understand the commands "carry" and "hold" as "take".

Understand "get out/off/up" as exiting.
Understand "get [things]" as taking.
Understand "get in/into/on/onto [something]" as entering.
Understand "get off [something]" as getting off.
Understand "get [things inside] from [something]" as removing it from.

Understand "pick up [things]" or "pick [things] up" as taking.

Understand "stand" or "stand up" as exiting.
Understand "stand on [something]" as entering.

Understand "remove [something preferably held]" as taking off.
Understand "remove [things inside] from [something]" as removing it from.

Understand "shed [something preferably held]" as taking off.
Understand the commands "doff" and "disrobe" as "shed".

Understand "wear [something preferably held]" as wearing.
Understand the command "don" as "wear".

Understand "put [other things] in/inside/into [something]" as inserting it into.
Understand "put [other things] on/onto [something]" as putting it on.
Understand "put on [something preferably held]" as wearing.
Understand "put down [things preferably held]" or "put [things preferably held] down" as dropping.

Understand "insert [other things] in/into [something]" as inserting it into.

Understand "drop [things preferably held]" as dropping.
Understand "drop [other things] in/into/down [something]" as inserting it into.
Understand "drop [other things] on/onto [something]" as putting it on.
Understand "drop [something preferably held] at/against/on/onto [something]" as throwing it at.
Understand the commands "throw" and "discard" as "drop".

Understand "give [something preferably held] to [someone]" as giving it to.
Understand "give [someone] [something preferably held]" as giving it to (with nouns reversed).
Understand the commands "pay" and "offer" and "feed" as "give".

Understand "show [someone] [something preferably held]" as showing it to (with nouns reversed).
Understand "show [something preferably held] to [someone]" as showing it to.
Understand the commands "present" and "display" as "show".

Understand "go" as going.
Understand "go [direction]" as going.
Understand "go [something]" as entering.
Understand "go into/in/inside/through [something]" as entering.
Understand the commands "walk", "leave" and "run" as "go".

Understand "inventory" as taking inventory.
Understand the commands "i" and "inv" as "inventory".

Understand "look" as looking.
Understand "look at [something]" as examining.
Understand "look inside/in/into/through [something]" as searching.
Understand "look under [something]" as looking under.
Understand "look up [text] in [something]" as consulting it about (with nouns reversed).
Understand the command "l" as "look".

Understand "consult [something] on/about [text]" as consulting it about.

Understand "open [something]" as opening.
Understand "open [something] with [something preferably held]" as unlocking it with.
Understand the commands "unwrap", "uncover" as "open".

Understand "close [something]" as closing.
Understand "close up [something]" as closing.
Understand "close off [something]" as switching off.
Understand the commands "shut" and "cover" as "close".

Understand "enter [something]" as entering.
Understand the command "cross" as "enter".

Understand "sit on top of [something]" as entering.
Understand "sit on/in/inside [something]" as entering.

Understand "exit" as exiting.
Understand the command "out" as "exit".

Understand "examine [something]" as examining.
Understand the commands "x", "watch", "describe" and "check" as "examine".

Understand "read [something]" as examining.
Understand "read about [text] in [something]" as consulting it about (with nouns reversed).
Understand "read [text] in [something]" as consulting it about (with nouns reversed).

Understand "yes" as saying yes.
Understand the command "y" as "yes".

Understand "no" as saying no.

Understand "sorry" as saying sorry.

Understand "bother" as swearing mildly.
Understand the commands "curses", "drat" and "darn" as "bother".
Understand "shit" as swearing obscenely.
Understand the commands "fuck" and "damn" as "shit".

Understand "search [something]" as searching.

Understand "wave" as waving hands.

Understand "wave [something]" as waving.

Understand "set [something] to [text]" as setting it to.
Understand the command "adjust" as "set".

Understand "pull [something]" as pulling.
Understand the command "drag" as "pull".

Understand "push [something]" as pushing.
Understand "push [something] [direction]" or "push [something] to [direction]" as pushing it to.
Understand the commands "move", "shift", "clear" and "press" as "push".

Understand "turn [something]" as turning.
Understand "turn [something] on" or "turn on [something]" as switching on.
Understand "turn [something] off" or "turn off [something]" as switching off.
Understand the commands "rotate", "twist", "unscrew" and "screw" as "turn".

Understand "switch [something]" or "switch on [something]" or "switch [something] on" as
	switching on.
Understand "switch [something] off" or "switch off [something]" as switching off.

Understand "lock [something] with [something preferably held]" as locking it with.

Understand "unlock [something] with [something preferably held]" as unlocking it with.

Understand "attack [something]" as attacking.
Understand the commands "break", "smash", "hit", "fight", "torture", "wreck", "crack", "destroy",
	"murder", "kill", "punch" and "thump" as "attack".

Understand "wait" as waiting.
Understand the command "z" as "wait".

Understand "answer [text] to [someone]" as answering it that (with nouns reversed).
Understand the commands "say", "shout" and "speak" as "answer".

Understand "tell [someone] about [text]" as telling it about.

Understand "ask [someone] about [text]" as asking it about.
Understand "ask [someone] for [something]" as asking it for.

Understand "eat [something preferably held]" as eating.

Understand "sleep" as sleeping.
Understand the command "nap" as "sleep".

Understand "sing" as singing.

Understand "climb [something]" or "climb up/over [something]" as climbing.
Understand the command "scale" as "climb".

Understand "buy [something]" as buying.
Understand the command "purchase" as "buy".

Understand "squeeze [something]" as squeezing.
Understand the command "squash" as "squeeze".

Understand "swing [something]" or "swing on [something]" as swinging.

Understand "wake" or "wake up" as waking up.
Understand "wake [someone]" or "wake [someone] up" or "wake up [someone]" as waking.
Understand the commands "awake" and "awaken" as "wake".

Understand "kiss [someone]" as kissing.
Understand the commands "embrace" and "hug" as "kiss".

Understand "think" as thinking.

Understand "smell" as smelling.
Understand "smell [something]" as smelling.
Understand the command "sniff" as "smell".

Understand "listen" as listening.
Understand "hear [something]" as listening.
Understand "listen to [something]" as listening.

Understand "taste [something]" as tasting.

Understand "touch [something]" as touching.
Understand the command "feel" as "touch".

Understand "rub [something]" as rubbing.
Understand the commands "shine", "polish", "sweep", "clean", "dust", "wipe" and "scrub" as "rub".

Understand "tie [something] to [something]" as tying it to.
Understand the commands "attach", "fix" and "fasten" as "tie".

Understand "burn [something]" as burning.
Understand the command "light" as "burn".

Understand "drink [something]" as drinking.
Understand the commands "swallow" and "sip" as "drink".

Understand "cut [something]" as cutting.
Understand the commands "slice", "prune" and "chop" as "cut".

Understand "jump" as jumping.
Understand the commands "skip" and "hop" as "jump".

Understand "score" as requesting the score.
Understand "quit" or "q" as quitting the game.
Understand "save" as saving the game.
Understand "restart" as restarting the game.
Understand "restore" as restoring the game.
Understand "verify" as verifying the story file.
Understand "version" as requesting the story file version.
Understand "script" or "script on" or "transcript" or "transcript on" as switching the story
	transcript on.
Understand "script off" or "transcript off" as switching the story transcript off.
Understand "superbrief" or "short" as preferring abbreviated room descriptions.
Understand "verbose" or "long" as preferring unabbreviated room descriptions.
Understand "brief" or "normal" as preferring sometimes abbreviated room descriptions.
Understand "nouns" or "pronouns" as requesting the pronoun meanings.
Understand "notify" or "notify on" as switching score notification on.
Understand "notify off" as switching score notification off.


Part SR5 - Phrasebook

Section SR5/1/1 - Saying - Values

To say (something - text)
	(documented at ph_say):
	(- print (PrintText) {something}; -).
To say (something - number):
	(- print (say__n={something}); -).

To say (ch - unicode-character) -- running on:
	(- #ifdef TARGET_ZCODE; @print_unicode {ch};
	#ifnot; if (unicode_gestalt_ok) glk_put_char_uni({ch}); else print "?"; #endif; -).

To say (something - number) in words
	(documented at ph_sayn):
	(- print (number) say__n=({something}); -).
To say (something - time) in words:
	(- print (PrintTimeOfDayEnglish) {something}; -).
To say s
	(documented at ph_sayn):
	(- STextSubstitution(); -).

Section SR5/1/2 - Saying - Names with articles

To say (something - object):
	(- print (name) {something}; -).
To say a (something - object)
	(documented at ph_saya):
	(- print (a) {something}; -).
To say an (something - object)
	(documented at ph_saya):
	(- print (a) {something}; -).
To say A (something - object):
	(- CIndefArt({something}); -).
To say An (something - object):
	(- CIndefArt({something}); -).
To say the (something - object):
	(- print (the) {something}; -).
To say The (something - object):
	(- print (The) {something}; -).

Section SR5/1/3 - Saying - Say if and otherwise

To say if (c - condition)
	(documented at ph_sayif): (- {-erase}
	if (~~({c})) jump {-next-label:Say};
		-).
To say unless (c - condition): (- {-erase}
	if ({c}) jump {-next-label:Say};
		-).
To say end if: (- {-erase}
	.{-label:Say}; .{-label:SayX};
		-).
To say end unless: (- {-erase}
	.{-label:Say}; .{-label:SayX};
		-).
To say otherwise/else if (c - condition): (- {-erase}
	jump {-next-label:SayX}; .{-label:Say}; if (~~({c})) jump {-next-label:Say};
		-).
To say otherwise/else unless (c - condition): (- {-erase}
	jump {-next-label:SayX}; .{-label:Say}; if ({c}) jump {-next-label:Say};
		-).
To say otherwise: (- {-erase}
	jump {-next-label:SayX}; .{-label:Say};
		-).
To say else: (- {-erase}
	jump {-next-label:SayX}; .{-label:Say};
		-).

Section SR5/1/4 - Saying - Say one of

To say one of -- beginning say_one_of (documented at ph_sayoneof):
	(- {-allocate-storage:say_one_of}I7_ST_say_one_of-->{-counter:say_one_of} =
	{-final-segment-marker}(I7_ST_say_one_of-->{-counter:say_one_of}, {-segment-count});
	switch((I7_ST_say_one_of-->{-advance-counter:say_one_of})%({-segment-count}+1)-1) {-open-brace}
		0: -).
To say or -- continuing say_one_of:
	(- @nop; {-segment-count}: -).
To say purely at random -- ending say_one_of with marker I7_SOO_PAR:
	(- {-close-brace} -).
To say at random -- ending say_one_of with marker I7_SOO_RAN:
	(- {-close-brace} -).
To say sticky random -- ending say_one_of with marker I7_SOO_STI:
	(- {-close-brace} -).
To say as decreasingly likely outcomes -- ending say_one_of with marker I7_SOO_TAP:
	(- {-close-brace} -).
To say in random order -- ending say_one_of with marker I7_SOO_SHU:
	(- {-close-brace} -).
To say cycling -- ending say_one_of with marker I7_SOO_CYC:
	(- {-close-brace} -).
To say stopping -- ending say_one_of with marker I7_SOO_STOP:
	(- {-close-brace} -).

Section SR5/1/5 - Saying - Paragraph control

To say line break -- running on
	(documented at ph_lbreak):
	(- new_line; -).
To say no line break -- running on: do nothing.
To say conditional paragraph break -- running on:
	(- DivideParagraphPoint(); -).
To say command clarification break -- running on:
	(- CommandClarificationBreak(); -).
To say paragraph break -- running on:
	(- DivideParagraphPoint(); new_line; -).
To say run paragraph on -- running on:
	(- RunParagraphOn(); -).
To say run paragraph on with special look spacing -- running on:
	(- SpecialLookSpacingBreak(); -).
To decide if a paragraph break is pending:
	(- (say__p) -).

Section SR5/1/6 - Saying - Special characters

To say bracket -- running on:
	(- print "["; -).
To say close bracket -- running on:
	(- print "]"; -).
To say apostrophe/' -- running on:
	(- print "'"; -).
To say quotation mark -- running on:
	(- print "~"; -).

Section SR5/1/7 - Saying - Fonts and visual effects

To say bold type -- running on
	(documented at ph_types):
	(- style bold; -).
To say italic type -- running on:
	(- style underline; -).
To say roman type -- running on:
	(- style roman; -).
To say fixed letter spacing -- running on:
	(- font off; -).
To say variable letter spacing -- running on:
	(- font on; -).
To display the boxed quotation (Q - boxed-quotation)
	(documented at ph_boxed):
	(- DisplayBoxedQuotation({Q}); -).

Section SR5/1/8 - Saying - Some built-in texts

To say the/-- banner text
	(documented at act_banner):
	(- Banner(); -).
To say the/-- list of extension credits
	(documented at ph_extcr):
	(- ShowExtensionVersions(); -).
To say the/-- complete list of extension credits:
	(- ShowFullExtensionVersions(); -).
To say the/-- player's surroundings
	(documented at ph_surrounds):
	(- SL_Location(); -).

Section SR5/1/9 - Saying - Saying lists of things

To list the contents of (O - an object),
	with newlines,
	indented,
	giving inventory information,
	as a sentence,
	including contents,
	including all contents,
	tersely,
	giving brief inventory information,
	using the definite article,
	listing marked items only,
	prefacing with is/are,
	not listing concealed items,
	suppressing all articles
	and/or with extra indentation
	(documented at ph_list):
	(- WriteListFrom(child({O}), {phrase options}); -).

To say contents of (O - an object):
	list the contents of O, as a sentence.

To say the contents of (O - an object):
	list the contents of O, as a sentence, using the definite article.

To say a list of (OS - description):
	(- @push subst__v;
		objectloop (subst__v ofclass Object) if ({-bind-variable:OS})
		give subst__v workflag2; else give subst__v ~workflag2;
		WriteListOfMarkedObjects(ENGLISH_BIT);
		@pull subst__v; -).
To say A list of (OS - description):
	(- @push subst__v;
		objectloop (subst__v ofclass Object) if ({-bind-variable:OS})
		give subst__v workflag2; else give subst__v ~workflag2;
		WriteListOfMarkedObjects(ENGLISH_BIT+CFIRSTART_BIT);
		@pull subst__v; -).
To say list of (OS - description):
	(- @push subst__v;
		objectloop (subst__v ofclass Object) if ({-bind-variable:OS})
		give subst__v workflag2; else give subst__v ~workflag2;
		WriteListOfMarkedObjects(ENGLISH_BIT+NOARTICLE_BIT);
		@pull subst__v; -).
To say the list of (OS - description):
	(- @push subst__v;
		objectloop (subst__v ofclass Object) if ({-bind-variable:OS})
		give subst__v workflag2; else give subst__v ~workflag2;
		WriteListOfMarkedObjects(ENGLISH_BIT+DEFART_BIT);
		@pull subst__v; -).
To say The list of (OS - description):
	(- @push subst__v;
		objectloop (subst__v ofclass Object) if ({-bind-variable:OS})
		give subst__v workflag2; else give subst__v ~workflag2;
		WriteListOfMarkedObjects(ENGLISH_BIT+DEFART_BIT+CFIRSTART_BIT);
		@pull subst__v; -).
To say is-are a list of (OS - description):
	(- @push subst__v;
		objectloop (subst__v ofclass Object) if ({-bind-variable:OS})
		give subst__v workflag2; else give subst__v ~workflag2;
		WriteListOfMarkedObjects(ENGLISH_BIT+ISARE_BIT);
		@pull subst__v; -).
To say is-are list of (OS - description):
	(- @push subst__v;
		objectloop (subst__v ofclass Object) if ({-bind-variable:OS})
		give subst__v workflag2; else give subst__v ~workflag2;
		WriteListOfMarkedObjects(ENGLISH_BIT+ISARE_BIT+NOARTICLE_BIT);
		@pull subst__v; -).
To say is-are the list of (OS - description):
	(- @push subst__v;
		objectloop (subst__v ofclass Object) if ({-bind-variable:OS})
		give subst__v workflag2; else give subst__v ~workflag2;
		WriteListOfMarkedObjects(ENGLISH_BIT+DEFART_BIT+ISARE_BIT);
		@pull subst__v; -).
To say a list of (OS - description) including contents:
	(- @push subst__v;
		objectloop (subst__v ofclass Object) if ({-bind-variable:OS})
		give subst__v workflag2; else give subst__v ~workflag2;
		WriteListOfMarkedObjects(ENGLISH_BIT+RECURSE_BIT+PARTINV_BIT+
			TERSE_BIT+CONCEAL_BIT);
		@pull subst__v; -).

Section SR5/1/10 - Saying - Group in and omit from lists

To group (OS - description) together
	(documented at ph_group):
	(- @push subst__v;
		objectloop (subst__v provides list_together) if ({-bind-variable:OS})
		subst__v.list_together = {-list-together};
		@pull subst__v; -).
To group (OS - description) together giving articles:
	(- @push subst__v;
		objectloop (subst__v provides list_together) if ({-bind-variable:OS})
		subst__v.list_together = {-articled-list-together};
		@pull subst__v; -).
To group (OS - description) together as (T - text):
	(- @push subst__v;
		objectloop (subst__v provides list_together) if ({-bind-variable:OS})
		subst__v.list_together = {T};
		@pull subst__v; -).
To omit contents in listing
	(documented at ph_omit):
	(- c_style = c_style &~ (RECURSE_BIT+FULLINV_BIT+PARTINV_BIT); -).

Section SR5/1/11 - Saying - Lists of values

To say (L - a list of values) in brace notation
	(documented at ph_saylv):
	(- LIST_OF_TY_Say({-pointer-to:L}, 1); -).
To say (L - a list of objects) with definite articles:
	(- LIST_OF_TY_Say({-pointer-to:L}, 2); -).
To say (L - a list of objects) with indefinite articles:
	(- LIST_OF_TY_Say({-pointer-to:L}, 3); -).

Section SR5/1/12 - Saying - Filtering contents - Unindexed

To filter list recursion to (D - description):
	(- list_filter_routine = {D}; -).
To unfilter list recursion:
	(- list_filter_routine = 0; -).

Section SR5/2/1 - Values and data structures - Counting

To decide which number is number of (S - description)
	(documented at ph_numof):
	(- {-number-of:S} -).
To decide which number is number of (S - domain-description)
	(documented at ph_numof):
	(- {-number-of:S} -).

Section SR5/2/2 - Values and data structures - Arithmetic

To decide which number is (X - number) + (Y - number)
	(arithmetic operation 0)
	(documented at ph_plus): (- ({X}+{Y}) -).
To decide which number is (X - number) - (Y - number)
	(arithmetic operation 1):
	(- ({X}-{Y}) -).
To decide which number is (X - number) * (Y - number)
	(arithmetic operation 2):
	(- ({X}*{Y}) -).
To decide which number is (X - number) / (Y - number)
	(arithmetic operation 3):
	(- (IntegerDivide({X},{Y})) -).
To decide which number is (X - number) plus (Y - number)
	(arithmetic operation 0):
	(- ({X}+{Y}) -).
To decide which number is (X - number) minus (Y - number)
	(arithmetic operation 1):
	(- ({X}-{Y}) -).
To decide which number is (X - number) times (Y - number)
	(arithmetic operation 2):
	(- ({X}*{Y}) -).
To decide which number is (X - number) multiplied by (Y - number)
	(arithmetic operation 2):
	(- ({X}*{Y}) -).
To decide which number is (X - number) divided by (Y - number)
	(arithmetic operation 3):
	(- (IntegerDivide({X},{Y})) -).
To decide which number is remainder after dividing (X - number)
	by (Y - number)
	(arithmetic operation 4):
	(- (IntegerRemainder({X},{Y})) -).
To decide which number is total (p - property) of (S - description)
	(arithmetic operation 5)
	(documented at ph_total):
	(- {-total-of:S} -).

Section SR5/2/3 - Values and data structures - Truth states

To decide what truth state is whether or not (C - condition)
	(documented at ph_wornot):
	(- ({C}) -).

Section SR5/2/4 - Values and data structures - Randomness

To decide which object is a/-- random (S - description)
	(documented at ph_randobj):
	(- {-random-of:S} -).
To decide which number is a random number from (N - number) to (M - number)
	(documented at ph_random):
	(- (GenerateRandomNumber({N}, {M})) -).
To decide which number is a random number between (N - number) and (M - number):
	(- (GenerateRandomNumber({N}, {M})) -).
To decide whether a random chance of (N - number) in (M - number) succeeds:
	(- (GenerateRandomNumber(1, {M}) <= {N}) -).
To seed the random-number generator with (N - number):
	(- VM_Seed_RNG({N}); -).
To decide which value is a/-- random (S - domain-description)
	(amended kind of value 1):
	(- {-random-of:S} -).

Section SR5/2/5 - Values and data structures - Changing stored values

To now (cn - now-condition)
	(documented at ph_now):
	(- {cn} -).

To let (t - nonexisting variable) be (u - word value)
	(documented at ph_let):
	(- {t} = {u}; -).
To let (t - nonexisting variable) be (u - pointer value):
	(- {-pointer-to:t}={-allocate-storage-for:u};BlkValueCopy({-pointer-to:t},{-pointer-to:u}); -).
To let (t - nonexisting variable) be (u - kind of word value):
	(- {t} = {-default-value-for:u}; -).
To let (t - nonexisting variable) be (u - kind of pointer value):
	(- {-pointer-to:t} = {-allocate-storage-for:u}; -).
To let (t - existing variable) be (u - assignable-value):
	(- {-assignment}; -).

To change (p - property-value) to (w - assignable-value)
	(documented at ph_changev):
	(- {-property-assignment}; -).
To change (gv - global variable) to (w - assignable-value)
	(documented at ph_change):
	(- {-assignment}; -).
To change (tr - table-reference) to (w - assignable-value):
	(- {-table-assignment}; -).
To change (le - list-entry) to (w - assignable-value):
	(- {-list-assignment}; -).
To change (o - object) to (p - property)
	(documented at ph_changep):
	(- SetEitherOrProperty({o}, {p}, false, {-adjective-definition:p}); -).
To change (o - object) to (w - value):
	(- WriteValueProperty({o},{-convert-adjectival-constants:w},{w}); -).
To change (lv - existing variable) to (w - assignable-value):
	(- {-assignment}; -).

Section SR5/2/6 - Values and data structures - Increase and decrease

To increase/increment (p - property-value) by (w - value)
	(documented at ph_increase):
	(- Write{p}{-delete},{p}+{w}); -).
To increase/increment (gv - global variable) by (w - value):
	(- {gv} = {gv} + {w}; -).
To increase/increment (lv - existing variable) by (w - value):
	(- {lv} = {lv} + {w}; -).
To increase/increment (tr - table-reference) by (w - value):
	(- {tr}{-delete},2,{w}); -).
To decrease/decrement (p - property-value) by (w - value):
	(- Write{p}{-delete},{p}-{w}); -).
To decrease/decrement (gv - global variable) by (w - value):
	(- {gv} = {gv} - {w}; -).
To decrease/decrement (lv - existing variable) by (w - value):
	(- {lv} = {lv} - {w}; -).
To decrease/decrement (tr - table-reference) by (w - value):
	(- {tr}{-delete},3,{w}); -).

Section SR5/2/7 - Values and data structures - Property provision

To decide if (o - object) provides the property (p - property):
	(- (WhetherProvides({o}, {-aorp:p})) -).
To decide if (o - object) does not provide the property (p - property):
	(- (WhetherProvides({o}, {-aorp:p})==false) -).

Section SR5/2/8 - Values and data structures - Tables

To decide which number is number of rows in/from (T - table-name)
	(documented at ph_numrows):
	(- TableRows({T}) -).
To decide which number is number of blank rows in/from (T - table-name)
	(documented at ph_numblank):
	(- TableBlankRows({T}) -).
To decide which number is number of filled rows in/from (T - table-name):
	(- TableFilledRows({T}) -).
To decide if there is (TR - table-reference)
	(documented at ph_thereis):
	(- (Exists{-do-not-dereference:TR}) -).
To decide if there is no (TR - table-reference):
	(- (Exists{-do-not-dereference:TR} == false) -).
To delete (tr - table-reference)
	(documented at ph_blankout):
	(- {tr}{-delete},4); -).
To blank out the whole row
	(documented at ph_blankout):
	(- {-require-ctvs}TableBlankOutRow(ct_0, ct_1); -).
To choose a/the/-- row (N - number) in/from (T - table-name)
	(documented at ph_chooserow):
	(- {-require-ctvs}ct_0 = {T}; ct_1 = {N}; -).
To choose a/the/-- row with (TC - table-column) of (w - value) in/from (T - table-name):
	(- {-require-ctvs}ct_0 = {T}; ct_1 = TableRowCorr(ct_0, {TC}, {w}); -).
To choose a/the/-- blank row in/from (T - table-name):
	(- {-require-ctvs}ct_0 = {T}; ct_1 = TableBlankRow(ct_0); -).
To choose a/the/-- random row in/from (T - table-name):
	(- {-require-ctvs}ct_0 = {T}; ct_1 = TableRandomRow(ct_0); -).

Section SR5/2/9 - Values and data structures - Sorting tables

To sort (T - table-name) in random order
	(documented at ph_sort):
	(- TableShuffle({T}); -).
To sort (T - table-name) in (TC - table-column) order:
	(- TableSort({T}, {TC}, 1); -).
To sort (T - table-name) in reverse (TC - table-column) order:
	(- TableSort({T}, {TC}, -1); -).

Section SR5/2/10 - Values and data structures - Indexed text

To decide what number is the number of characters in (txb - indexed text)
	(documented at ph_numofc):
	(- IT_BlobAccess({-pointer-to:txb}, CHR_BLOB) -).
To decide what indexed text is character number (N - a number) in (txb - indexed text):
	(- IT_GetBlob({-pointer-to-new:INDEXED_TEXT_TY}, {-pointer-to:txb}, {N}, CHR_BLOB) -).
To replace character number (N - a number) in (txb - indexed text)
	with (rtxb - indexed text):
	(- IT_ReplaceBlob(CHR_BLOB, {-pointer-to:txb}, {N}, {-pointer-to:rtxb}); -).

To decide what number is the number of words in (txb - indexed text):
	(- IT_BlobAccess({-pointer-to:txb}, WORD_BLOB) -).
To decide what indexed text is word number (N - a number) in (txb - indexed text):
	(- IT_GetBlob({-pointer-to-new:INDEXED_TEXT_TY}, {-pointer-to:txb}, {N}, WORD_BLOB) -).
To replace word number (N - a number) in (txb - indexed text)
	with (rtxb - indexed text):
	(- IT_ReplaceBlob(WORD_BLOB, {-pointer-to:txb}, {N}, {-pointer-to:rtxb}); -).

To decide what number is the number of punctuated words in (txb - indexed text):
	(- IT_BlobAccess({-pointer-to:txb}, PWORD_BLOB) -).
To decide what indexed text is punctuated word number (N - a number) in (txb - indexed text):
	(- IT_GetBlob({-pointer-to-new:INDEXED_TEXT_TY}, {-pointer-to:txb}, {N}, PWORD_BLOB) -).
To replace punctuated word number (N - a number) in (txb - indexed text)
	with (rtxb - indexed text):
	(- IT_ReplaceBlob(PWORD_BLOB, {-pointer-to:txb}, {N}, {-pointer-to:rtxb}); -).

To decide what number is the number of unpunctuated words in (txb - indexed text):
	(- IT_BlobAccess({-pointer-to:txb}, UWORD_BLOB) -).
To decide what indexed text is unpunctuated word number (N - a number) in (txb - indexed text):
	(- IT_GetBlob({-pointer-to-new:INDEXED_TEXT_TY}, {-pointer-to:txb}, {N}, UWORD_BLOB) -).
To replace unpunctuated word number (N - a number) in (txb - indexed text)
	with (rtxb - indexed text):
	(- IT_ReplaceBlob(UWORD_BLOB, {-pointer-to:txb}, {N}, {-pointer-to:rtxb}); -).

To decide what number is the number of lines in (txb - indexed text):
	(- IT_BlobAccess({-pointer-to:txb}, LINE_BLOB) -).
To decide what indexed text is line number (N - a number) in (txb - indexed text):
	(- IT_GetBlob({-pointer-to-new:INDEXED_TEXT_TY}, {-pointer-to:txb}, {N}, LINE_BLOB) -).
To replace line number (N - a number) in (txb - indexed text) with (rtxb - indexed text):
	(- IT_ReplaceBlob(LINE_BLOB, {-pointer-to:txb}, {N}, {-pointer-to:rtxb}); -).

To decide what number is the number of paragraphs in (txb - indexed text):
	(- IT_BlobAccess({-pointer-to:txb}, PARA_BLOB) -).
To decide what indexed text is paragraph number (N - a number) in (txb - indexed text):
	(- IT_GetBlob({-pointer-to-new:INDEXED_TEXT_TY}, {-pointer-to:txb}, {N}, PARA_BLOB) -).
To replace paragraph number (N - a number) in (txb - indexed text) with (rtxb - indexed text):
	(- IT_ReplaceBlob(PARA_BLOB, {-pointer-to:txb}, {N}, {-pointer-to:rtxb}); -).

Section SR5/2/11 - Values and data structures - Matching text

To decide if (txb - indexed text) exactly matches the text (ftxb - indexed text),
	case insensitively
	(documented at ph_exactm):
	(- IT_Replace_RE(CHR_BLOB,{-pointer-to:txb},{-pointer-to:ftxb},0,{phrase options},1) -).
To decide if (txb - indexed text) matches the text (ftxb - indexed text),
	case insensitively:
	(- IT_Replace_RE(CHR_BLOB,{-pointer-to:txb},{-pointer-to:ftxb},0,{phrase options}) -).
To decide what number is number of times (txb - indexed text) matches the text
	(ftxb - indexed text), case insensitively:
	(- IT_Replace_RE(CHR_BLOB,{-pointer-to:txb},{-pointer-to:ftxb},1,{phrase options}) -).

To decide if (txb - indexed text) exactly matches the regular expression (ftxb - indexed text),
	case insensitively
	(documented at ph_regexp):
	(- IT_Replace_RE(REGEXP_BLOB,{-pointer-to:txb},{-pointer-to:ftxb},0,{phrase options},1) -).
To decide if (txb - indexed text) matches the regular expression (ftxb - indexed text),
	case insensitively:
	(- IT_Replace_RE(REGEXP_BLOB,{-pointer-to:txb},{-pointer-to:ftxb},0,{phrase options}) -).
To decide what number is number of times (txb - indexed text) matches the regular expression
	(ftxb - indexed text),case insensitively:
	(- IT_Replace_RE(REGEXP_BLOB,{-pointer-to:txb},{-pointer-to:ftxb},1,{phrase options}) -).
To decide what indexed text is text matching regular expression:
	(- IT_RE_GetMatchVar({-pointer-to-new:INDEXED_TEXT_TY}, 0) -).
To decide what indexed text is text matching subexpression (N - a number):
	(- IT_RE_GetMatchVar({-pointer-to-new:INDEXED_TEXT_TY}, {N}) -).

Section SR5/2/12 - Values and data structures - Replacing text

To replace the text (ftxb - indexed text) in (txb - indexed text) with (rtxb - indexed text),
	case insensitively
	(documented at ph_replace):
	(- IT_Replace_RE(REGEXP_BLOB, {-pointer-to:txb}, {-pointer-to:ftxb},
		{-pointer-to:rtxb}, {phrase options}); -).
To replace the regular expression (ftxb - indexed text) in (txb - indexed text) with
	(rtxb - indexed text), case insensitively:
	(- IT_Replace_RE(REGEXP_BLOB, {-pointer-to:txb}, {-pointer-to:ftxb},
		{-pointer-to:rtxb}, {phrase options}); -).
To replace the word (ftxb - indexed text) in (txb - indexed text) with
	(rtxb - indexed text):
	(- IT_ReplaceText(WORD_BLOB, {-pointer-to:txb}, {-pointer-to:ftxb}, {-pointer-to:rtxb}); -).
To replace the punctuated word (ftxb - indexed text) in (txb - indexed text)
	with (rtxb - indexed text):
	(- IT_ReplaceText(PWORD_BLOB, {-pointer-to:txb}, {-pointer-to:ftxb}, {-pointer-to:rtxb}); -).

Section SR5/2/13 - Values and data structures - Casing of text

To decide what indexed text is (txb - indexed text) in lower case
	(documented at ph_casing):
	(- IT_CharactersToCase({-pointer-to-new:INDEXED_TEXT_TY}, {-pointer-to:txb}, 0) -).
To decide what indexed text is (txb - indexed text) in upper case:
	(- IT_CharactersToCase({-pointer-to-new:INDEXED_TEXT_TY}, {-pointer-to:txb}, 1) -).
To decide what indexed text is (txb - indexed text) in title case:
	(- IT_CharactersToCase({-pointer-to-new:INDEXED_TEXT_TY}, {-pointer-to:txb}, 2) -).
To decide what indexed text is (txb - indexed text) in sentence case:
	(- IT_CharactersToCase({-pointer-to-new:INDEXED_TEXT_TY}, {-pointer-to:txb}, 3) -).
To decide if (txb - indexed text) is in lower case:
	(- IT_CharactersOfCase({-pointer-to:txb}, 0) -).
To decide if (txb - indexed text) is in upper case:
	(- IT_CharactersOfCase({-pointer-to:txb}, 1) -).

Section SR5/2/14 - Values and data structures - Lists

To add (N - value = kov reference 1) to (L - list of values = list of kov marker 1),
	if absent
	(documented at ph_addlist):
	(- LIST_OF_TY_InsertItem({-pointer-to:L}, {N}, 0, 0, {phrase options}); -).
To add (N - value = kov reference 1) at entry (E - number) in
	(L - list of values = list of kov marker 1):
	(- LIST_OF_TY_InsertItem({-pointer-to:L}, {N}, 1, {E}, 0); -).
To add (LX - list of values = list of kov reference 1) to
	(L - list of values = list of kov marker 1), if absent:
	(- LIST_OF_TY_AppendList({-pointer-to:L}, {-pointer-to:LX}, 0, 0, {phrase options}); -).
To add (LX - list of values = list of kov reference 1) at entry (E - number)
	in (L - list of values = list of kov marker 1):
	(- LIST_OF_TY_AppendList({-pointer-to:L}, {-pointer-to:LX}, 1, {E}, 0); -).
To remove (N - value = kov reference 1) from (L - list of values = list of kov marker 1),
	if present
	(documented at ph_remlist):
	(- LIST_OF_TY_RemoveValue({-pointer-to:L}, {N}, {phrase options}); -).
To remove (N - list of values = list of kov reference 1) from
	(L - list of values = list of kov marker 1), if present:
	(- LIST_OF_TY_Remove_List({-pointer-to:L}, {-pointer-to:N}, {phrase options}); -).
To remove entry (N - number) from (L - list of values), if present:
	(- LIST_OF_TY_RemoveItemRange({-pointer-to:L}, {N}, {N}, {phrase options}); -).
To remove entries (N - number) to (N2 - number) from (L - list of values), if present:
	(- LIST_OF_TY_RemoveItemRange({-pointer-to:L}, {N}, {N2}, {phrase options}); -).

To decide if (N - value = kov reference 1) is listed in
	(L - list of values = list of kov marker 1)
	(documented at ph_islistedin):
	(- (LIST_OF_TY_FindItem({-pointer-to:L}, {N})) -).
To decide if (N - value = kov reference 1) is not listed in
	(L - list of values = list of kov marker 1):
	(- (LIST_OF_TY_FindItem({-pointer-to:L}, {N}) == false) -).

To decide what list of objects is the list of (D - description)
	(documented at ph_listd):
	(- LIST_OF_TY_Desc({-pointer-to-new:LIST_OF_TY}, {D}) -).
To decide what list of values is the list of (D - domain-description)
	(amended kind of value 0):
	(- LIST_OF_TY_Desc({-pointer-to-new:LIST_OF_TY}, {D}, {-domain-kov:D}) -).
To decide what list of objects is the multiple object list
	(documented at ph_mobjl):
	(- LIST_OF_TY_Mol({-pointer-to-new:LIST_OF_TY}) -).
To alter the multiple object list to (L - list of objects):
	(- LIST_OF_TY_Set_Mol({-pointer-to:L}); -).

Section SR5/2/15 - Values and data structures - Length of lists

To decide what number is the number of entries in/of (L - a list of values)
	(documented at ph_numel):
	(- LIST_OF_TY_GetLength({-pointer-to:L}) -).
To truncate (L - a list of values) to (N - a number) entries
	(documented at ph_truncate):
	(- LIST_OF_TY_SetLength({-pointer-to:L}, {N}, -1, 1); -).
To truncate (L - a list of values) to the first (N - a number) entries:
	(- LIST_OF_TY_SetLength({-pointer-to:L}, {N}, -1, 1); -).
To truncate (L - a list of values) to the last (N - a number) entries:
	(- LIST_OF_TY_SetLength({-pointer-to:L}, {N}, -1, -1); -).
To extend (L - a list of values) to (N - a number) entries:
	(- LIST_OF_TY_SetLength({-pointer-to:L}, {N}, 1); -).
To change (L - a list of values) to have (N - a number) entries:
	(- LIST_OF_TY_SetLength({-pointer-to:L}, {N}, 0); -).

Section SR5/2/16 - Values and data structures - Reversing and rotating lists

To reverse (L - a list of values)
	(documented at ph_revl):
	(- LIST_OF_TY_Reverse({-pointer-to:L}); -).
To rotate (L - a list of values)
	(documented at ph_rotl):
	(- LIST_OF_TY_Rotate({-pointer-to:L}, 0); -).
To rotate (L - a list of values) backwards:
	(- LIST_OF_TY_Rotate({-pointer-to:L}, 1); -).

Section SR5/2/17 - Values and data structures - Sorting lists

To sort (L - a list of values)
	(documented at ph_sortl):
	(- LIST_OF_TY_Sort({-pointer-to:L}, 1); -).
To sort (L - a list of values) in reverse order:
	(- LIST_OF_TY_Sort({-pointer-to:L}, -1); -).
To sort (L - a list of values) in random order:
	(- LIST_OF_TY_Sort({-pointer-to:L}, 2); -).
To sort (L - a list of objects) in (P - property) order:
	(- LIST_OF_TY_Sort({-pointer-to:L}, 1, {P}); -).
To sort (L - a list of objects) in reverse (P - property) order:
	(- LIST_OF_TY_Sort({-pointer-to:L}, -1, {P}); -).

Section SR5/2/18 - Values and data structures - Use options

To decide whether using the/-- (UO - use-option)
	(documented at ph_testuo):
	(- (TestUseOption({UO})) -).

Section SR5/2/19 - Values and data structures - Relations

To decide which object is next step via (R - abstract-relation)
	from (O1 - object) to (O2 - object)
	(documented at ph_nextstep):
	(- RelationRouteTo({R},{O1},{O2},false) -).
To decide which number is number of steps via (R - abstract-relation)
	from (O1 - object) to (O2 - object):
	(- RelationRouteTo({R},{O1},{O2},true) -).

Section SR5/3/1 - Loops and conditionals - If and unless

To if (c - condition) then (ph - phrase)
	(documented at ph_if):
	(- if {c} {ph} -).
To if (c - condition) , (ph - phrase):
	(- if {c} {ph} -).
To unless (c - condition) , (ph - phrase):
	(- if (~~{c}) {ph} -).

To otherwise if (c - condition) -- in if:
	(- } else if {c} { -).
To else if (c - condition) -- in if:
	(- } else if {c} { -).
To otherwise unless (c - condition) -- in if:
	(- } else if (~~{c}) { -).
To else unless (c - condition) -- in if:
	(- } else if (~~{c}) { -).
To otherwise (ph - phrase)
	(documented at ph_otherwise):
	(- else {ph} -).
To else (ph - phrase)
	(documented at ph_otherwise):
	(- else {ph} -).
To if (c - condition) begin -- end:
	(- if {c}  -).
To unless (c - condition) begin -- end:
	(- if (~~{c})  -).

To if (V - word value) is begin -- end:
	(- switch({V})  -).

Section SR5/3/2 - Loops and conditionals - While

To while (c - condition) repeatedly (ph - phrase)
	(documented at ph_while):
	(- while {c} {ph} -).
To while (c - condition) , (ph - phrase):
	(- while {c} {ph} -).
To while (c - condition) begin -- end:
	(- while {c}  -).

Section SR5/3/3 - Loops and conditionals - Repeat

To repeat with (loopvar - nonexisting number variable)
	running from (v - number) to (w - number) begin -- end
	(documented at ph_repeat):
		(- for ({loopvar}={v}: {loopvar}<={w}: {loopvar}++)  -).
To repeat with (loopvar - nonexisting object variable)
	running through (OS - description) begin -- end
	(documented at ph_runthrough):
		(- {-loop-over:OS}  -).
To repeat with (loopvar - nonexisting object variable)
	running through (OS - domain-description) begin -- end
	(documented at ph_runthrough):
		(- {-loop-over-domain:OS}  -).

Section SR5/3/4 - Loops and conditionals - Repeat through tables

To repeat through (T - table-name) begin -- end
	(documented at ph_tabrepeat):
	(- {-push-ctvs}
		for ({-ct-v0}={T},{-ct-v1}=1,ct_0={-ct-v0},ct_1={-ct-v1}:{-ct-v1}<=TableRows({-ct-v0}):{-ct-v1}++,ct_0={-ct-v0},ct_1={-ct-v1}) if (TableRowIsBlank(ct_0,ct_1)==false)  -).
To repeat through (T - table-name) in reverse order begin -- end:
	(- {-push-ctvs}
		for ({-ct-v0}={T},{-ct-v1}=TableRows({-ct-v0}),ct_0={-ct-v0},ct_1={-ct-v1}:{-ct-v1}>=1:{-ct-v1}--,ct_0={-ct-v0},ct_1={-ct-v1}) if (TableRowIsBlank(ct_0,ct_1)==false)  -).
To repeat through (T - table-name) in (TC - table-column) order begin -- end:
	(- {-push-ctvs}
		for ({-ct-v0}={T},{-ct-v1}=TableNextRow({-ct-v0},{TC},0,1),ct_0={-ct-v0},ct_1={-ct-v1}:{-ct-v1}~=0:{-ct-v1}=TableNextRow({-ct-v0},{TC},{-ct-v1},1),ct_0={-ct-v0},ct_1={-ct-v1})  -).
To repeat through (T - table-name) in reverse (TC - table-column) order begin -- end:
	(- {-push-ctvs}
		for ({-ct-v0}={T},{-ct-v1}=TableNextRow({-ct-v0},{TC},0,-1),ct_0={-ct-v0},ct_1={-ct-v1}:{-ct-v1}~=0:{-ct-v1}=TableNextRow({-ct-v0},{TC},{-ct-v1},-1),ct_0={-ct-v0},ct_1={-ct-v1})  -).

Section SR5/3/5 - Loops and conditionals - Repeat through lists

To repeat with (loopvar - nonexisting object variable)
	running through (L - list of values) begin -- end:
	(- {-loop-over-list:L}  -).

Section SR5/3/6 - Loops and conditionals - Changing the flow of loops

To break -- in loop (documented at ph_break):
	(- break; -).
To next -- in loop (documented at ph_next):
	(- continue; -).

Section SR5/3/7 - Loops and conditionals - Deciding outcomes

To yes
	(documented at ph_yes):
	(- rtrue; -) - in to decide if only.
To decide yes:
	(- rtrue; -) - in to decide if only.
To no:
	(- rfalse; -) - in to decide if only.
To decide no:
	(- rfalse; -) - in to decide if only.

To decide on (something - value)
	(documented at ph_result):
	(- return {-check-return-type:something}; -).

Section SR5/3/8 - Loops and conditionals - Stop or go

To do nothing (documented at ph_nothing):
	(- ; -).
To stop (documented at ph_stop):
	(- return; -) - in to only.

Section SR5/4/1 - Actions, activities and rules - Trying actions

To try (doing something - action)
	(documented at ph_try):
	(- {doing something}; -).
To silently try (doing something - action):
	(- @push keep_silent; keep_silent=1; {doing something}; @pull keep_silent; -).
To try silently (doing something - action):
	(- @push keep_silent; keep_silent=1; {doing something}; @pull keep_silent; -).

Section SR5/4/2 - Actions, activities and rules - Action requirements

To decide whether the action requires a touchable noun
	(documented at ph_requires):
	(- (NeedToTouchNoun()) -).
To decide whether the action requires a touchable second noun:
	(- (NeedToTouchSecondNoun()) -).
To decide whether the action requires a carried noun:
	(- (NeedToCarryNoun()) -).
To decide whether the action requires a carried second noun:
	(- (NeedToCarrySecondNoun()) -).
To decide whether the action requires light:
	(- (NeedLightForAction()) -).

Section SR5/4/3 - Actions, activities and rules - Stop or continue

To stop the action
	(documented at ph_stopac):
	(- rtrue; -) - in to only.
To continue the action:
	(- rfalse; -) - in to only.

Section SR5/4/4 - Actions, activities and rules - Stored actions

To decide what stored action is the current action
	(documented at ph_curract):
	(- STORED_ACTION_TY_Current({-pointer-to-new:STORED_ACTION_TY}) -).
To decide what stored action is the action of (A - action):
	(- {A}{-delete}{-delete}, STORED_ACTION_TY_Current({-pointer-to-new:STORED_ACTION_TY})) -).
To try (S - stored action):
	(- STORED_ACTION_TY_Try({S}); -).
To silently try (S - stored action):
	(- STORED_ACTION_TY_Try({S}, true); -).
To try silently (S - stored action):
	(- STORED_ACTION_TY_Try({S}, true); -).
To decide if (act - a stored action) involves (X - an object)
	(documented at ph_involves):
	(- (STORED_ACTION_TY_Involves({-pointer-to:act}, {X})) -).
To decide what action-name is the action-name part of (act - a stored action):
	(- (STORED_ACTION_TY_Part({-pointer-to:act}, 0)) -).
To decide what object is the noun part of (act - a stored action):
	(- (STORED_ACTION_TY_Part({-pointer-to:act}, 1)) -).
To decide what object is the second noun part of (act - a stored action):
	(- (STORED_ACTION_TY_Part({-pointer-to:act}, 2)) -).
To decide what object is the actor part of (act - a stored action):
	(- (STORED_ACTION_TY_Part({-pointer-to:act}, 3)) -).

Section SR5/4/5 - Actions, activities and rules - Carrying out activities

To carry out the (A - activity) activity
	(documented at ph_carryout):
	(- CarryOutActivity({A}); -).
To carry out the (A - activity) activity with (O - object):
	(- CarryOutActivity({A}, {O}); -).

[To decide whether (A - activity) activity is going on
	(documented at ph_goingon):
	(- (TestActivity({A})) -).
To decide whether (A - activity) activity is not going on:
	(- (~~(TestActivity({A}))) -).
To decide whether (A - activity) activity is empty:
	(- (ActivityEmpty({A})) -).
To decide whether (A - activity) activity is not empty:
	(- (~~(ActivityEmpty({A}))) -).]

To continue the activity:
	(- rfalse; -) - in to only.

Section SR5/4/6 - Actions, activities and rules - Advanced activities

To begin the (A - activity) activity
	(documented at ph_beginact):
	(- BeginActivity({A}); -).
To begin the (A - activity) activity with (O - object):
	(- BeginActivity({A}, {O}); -).
To decide whether handling (A - activity) activity:
	(- (~~(ForActivity({A}))) -).
To decide whether handling (A - activity) activity with (O - object):
	(- (~~(ForActivity({A}, {O}))) -).
To end the (A - activity) activity:
	(- EndActivity({A}); -).
To end the (A - activity) activity with (O - object):
	(- EndActivity({A}, {O}); -).
To abandon the (A - activity) activity:
	(- AbandonActivity({A}); -).
To abandon the (A - activity) activity with (O - object):
	(- AbandonActivity({A}, {O}); -).

Section SR5/4/7 - Actions, activities and rules - Following rules

To follow (RL - a rule)
	(documented at ph_follow):
	(- FollowRulebook({RL}); -).
To follow (RL - a rule) for (O - object):
	(- FollowRulebook({RL}, {O}, true); -).
To consider (RL - a rule)
	(documented at ph_consider):
	(- ProcessRulebook({RL}); -).
To consider (RL - a rule) for (O - object):
	(- ProcessRulebook({RL}, {O}, true); -).
To abide by (RL - a rule)
	(documented at ph_abide):
	(- if (ProcessRulebook({RL})) rtrue; -) - in to only.
To abide by (RL - a rule) for (O - object):
	(- if (ProcessRulebook({RL}, {O}, true)) rtrue; -) - in to only.
To anonymously abide by (RL - a rule):
	(- if (temporary_value = ProcessRulebook({RL})) {
		if (RulebookSucceeded()) ActRulebookSucceeds(temporary_value);
		else ActRulebookFails(temporary_value);
		rtrue;
	} -) - in to only.
To anonymously abide by (RL - a rule) for (O - object):
	(- if (temporary_value = ProcessRulebook({RL}, {O}, true)) {
		if (RulebookSucceeded()) ActRulebookSucceeds(temporary_value);
		else ActRulebookFails(temporary_value);
		rtrue;
	} -) - in to only.

Section SR5/4/8 - Actions, activities and rules - Success and failure

To make no decision: (- rfalse; -) - in to only.
To rule succeeds
	(documented at ph_succeeds):
	(- RulebookSucceeds(); rtrue; -) - in to only.
To rule fails:
	(- RulebookFails(); rtrue; -) - in to only.
To rule succeeds with result (O - a miscellaneous-value):
	(- RulebookSucceeds(true,{O}); rtrue; -) - in to only.
To rule fails with result (O - a miscellaneous-value):
	(- RulebookFails(true,{O}); rtrue; -) - in to only.
To decide if rule succeeded:
	(- (RulebookSucceeded()) -).
To decide if rule succeeded with result (O - miscellaneous-value):
	(- ((RulebookSucceeded()) && (ResultOfRule() == {O})) -).
To decide if rule failed:
	(- (RulebookFailed()) -).
To decide if rule failed with result (O - miscellaneous-value):
	(- ((RulebookFailed()) && (ResultOfRule() == {O})) -).
To decide which miscellaneous-value is the result of the rule
	(documented at ph_resultr):
	(- (ResultOfRule()) -).
To decide which rulebook-outcome is the outcome of the rulebook
	(documented at ph_outcomer):
	(- (ResultOfRule()) -).

Section SR5/4/9 - Actions, activities and rules - Procedural manipulation

To ignore (RL - a rule)
	(documented at ph_ignore):
	(- SuppressRule({RL}); -).
To reinstate (RL - a rule):
	(- ReinstateRule({RL}); -).
To reject the result of (RL - a rule):
	(- DonotuseRule({RL}); -).
To accept the result of (RL - a rule):
	(- DonotuseRule({RL}); -).
To substitute (RL1 - a rule) for (RL2 - a rule):
	(- SubstituteRule({RL1},{RL2}); -).
To restore the original (RL1 - a rule):
	(- SubstituteRule({RL1},{RL1}); -).
To move (RL1 - a rule) to before (RL2 - a rule):
	(- MoveRuleBefore({RL1},{RL2}); -).
To move (RL1 - a rule) to after (RL2 - a rule):
	(- MoveRuleAfter({RL1},{RL2}); -).

Section SR5/5/1 - Model world - Score

To award (some - number) point/points
	(documented at ph_awardp):
	(- score=score+{some}; -).

Section SR5/5/2 - Model world - Outcome of play

To end the game in death (documented at ph_end): (- deadflag=1; -).
To end the game in victory: (- deadflag=2; -).
To end the game saying (finale - text): (- deadflag={finale}; -).
To resume the game: (- resurrect_please = true; -).
To decide whether the game is in progress: (- (deadflag==0) -).
To decide whether the game is over: (- (deadflag~=0) -).
To decide whether the game ended in death: (- (deadflag==1) -).
To decide whether the game ended in victory: (- (deadflag==2) -).

Section SR5/5/3 - Model world - Times of day

To decide which time is (t - time) to the nearest (t2 - time)
	(documented at ph_nearest):
	(- (RoundOffTime({t},{t2})) -).
To decide which number is the minutes part of (t - time)
	(documented at ph_minspart):
	(- ({t}%ONE_HOUR) -).
To decide which number is the hours part of (t - time):
	(- ({t}/ONE_HOUR) -).

To decide if (t - time) is before (t2 - time)
	(documented at ph_timeshift):
	(- ((({t}+20*ONE_HOUR)%(TWENTY_FOUR_HOURS))<(({t2}+20*ONE_HOUR)%(TWENTY_FOUR_HOURS))) -).
To decide if (t - time) is after (t2 - time):
	(- ((({t}+20*ONE_HOUR)%(TWENTY_FOUR_HOURS))>(({t2}+20*ONE_HOUR)%(TWENTY_FOUR_HOURS))) -).
To decide if it is before (t2 - time):
	(- (((the_time+20*ONE_HOUR)%(TWENTY_FOUR_HOURS))<(({t2}+20*ONE_HOUR)%(TWENTY_FOUR_HOURS))) -).
To decide if it is after (t2 - time):
	(- (((the_time+20*ONE_HOUR)%(TWENTY_FOUR_HOURS))>(({t2}+20*ONE_HOUR)%(TWENTY_FOUR_HOURS))) -).
To decide which time is (t - time) before (t2 - time)
	(documented at ph_timeshift):
	(- (({t2}-{t}+TWENTY_FOUR_HOURS)%(TWENTY_FOUR_HOURS)) -).
To decide which time is (t - time) after (t2 - time):
	(- (({t2}+{t}+TWENTY_FOUR_HOURS)%(TWENTY_FOUR_HOURS)) -).

Section SR5/5/4 - Model world - Durations

To decide which time is (n - number) minutes
	(documented at ph_durations):
	(- (({n})%(TWENTY_FOUR_HOURS)) -).
To decide which time is (n - number) hours:
	(- (({n}*ONE_HOUR)%(TWENTY_FOUR_HOURS)) -).

Section SR5/5/5 - Model world - Timed events

To (R - rule) in (t - number) turn from now
	(documented at ph_future):
	(- SetTimedEvent({R}, {t}+1, 0); -).
To (R - rule) in (t - number) turns from now:
	(- SetTimedEvent({R}, {t}+1, 0); -).
To (R - rule) at (t - time):
	(- SetTimedEvent({R}, {t}, 1); -).
To (R - rule) in (t - time) from now:
	(- SetTimedEvent({R}, (the_time+{t})%(TWENTY_FOUR_HOURS), 1); -).

Section SR5/5/6 - Model world - Scenes

[To decide if (sc - scene) is happening
	(documented at ph_happening):
	(- (scene_status-->({sc}-1) == 1) -).
To decide if (sc - scene) is not happening:
	(- (scene_status-->({sc}-1) ~= 1) -).]

To decide if (sc - scene) has happened:
	(- (scene_endings-->({sc}-1)) -).
To decide if (sc - scene) has not happened:
	(- (scene_endings-->({sc}-1) == 0) -).
To decide if (sc - scene) has ended:
	(- (scene_endings-->({sc}-1) > 1) -).
To decide if (sc - scene) has not ended:
	(- (scene_endings-->({sc}-1) <= 1) -).

Section SR5/5/7 - Model world - Timing of scenes

To decide which time is the time since (sc - scene) began
	(documented at ph_tsince):
	(- ((the_time - scene_started-->({sc}-1))%(TWENTY_FOUR_HOURS)) -).
To decide which time is the time when (sc - scene) began:
	(- (scene_started-->({sc}-1)) -).
To decide which time is the time since (sc - scene) ended:
	(- ((the_time - scene_ended-->({sc}-1))%(TWENTY_FOUR_HOURS)) -).
To decide which time is the time when (sc - scene) ended:
	(- (scene_ended-->({sc}-1)) -).

Section SR5/5/8 - Model world - Player's identity and location

To change the/-- player to (O - an object)
	(documented at ph_changep):
	(- ChangePlayer({O}); -).
To decide whether in (somewhere - an object):
	(- (WhetherIn({somewhere})) -).
To decide whether in darkness
	(documented at ph_indarkness):
	(- (location==thedark) -).

Section SR5/5/9 - Model world - Moving and removing things

To move (something - object) to (something else - object),
	without printing a room description
	or printing an abbreviated room description
	(documented at ph_move):
	(- MoveObject({something}, {something else}, {phrase options}, false); -).
To remove (something - object) from play
	(documented at ph_remove):
	(- RemoveFromPlay({something}); -).
To move (O - object) backdrop to all (D - description)
	(documented at ph_movebd):
	(- MoveBackdrop({O}, {D}); -).
To update backdrop positions
	(documented at ph_updatebp):
	(- MoveFloatingObjects(); -).

Section SR5/5/10 - Model world - The map

To decide which room is location of (O - object)
	(documented at ph_locof):
	(- LocationOf({O}) -).
To decide which room is room (D - direction) from (R1 - room)
	(documented at ph_roomdirof):
	(- MapConnection({R1},{D}) -).
To decide which object is the other side of (D - door) from (R1 - room):
	(- OtherSideOfDoor({D},{R1}) -).
To decide which object is the direction of (D - door) from (R1 - room):
	(- DirectionDoorLeadsIn({D},{R1}) -).
To decide which object is room-or-door (D - direction) from (R1 - room):
	(- RoomOrDoorFrom({R1},{D}) -).
To change (D - direction) exit of (R1 - room) to (R2 - room)
	(documented at ph_changex):
	(- AssertMapConnection({R1},{D},{R2}); -).
To change (D - direction) exit of (R1 - room) to nothing/nowhere:
	(- AssertMapConnection({R1},{D},nothing); -).
To decide which room is the front side of (D - object)
	(documented at ph_frontside):
	(- FrontSideOfDoor({D}) -).
To decide which room is the back side of (D - object):
	(- BackSideOfDoor({D}) -).

Section SR5/5/11 - Model world - Route-finding

To decide which object is best route from (R1 - object) to (R2 - object),
	using doors or using even locked doors
	(documented at ph_bestroute):
	(- MapRouteTo({R1},{R2},0,{phrase options}) -).
To decide which number is number of moves from (R1 - object) to (R2 - object),
	using doors or using even locked doors:
	(- MapRouteTo({R1},{R2},0,{phrase options},true) -).
To decide which object is best route from (R1 - object) to (R2 - object) through
	(RS - description),
	using doors or using even locked doors:
	(- MapRouteTo({R1},{R2},{RS},{phrase options}) -).
To decide which number is number of moves from (R1 - object) to (R2 - object) through
	(RS - description),
	using doors or using even locked doors:
	(- MapRouteTo({R1},{R2},{RS},{phrase options},true) -).

Section SR5/5/12 - Model world - The object tree

To decide which object is holder of (something - object)
	(documented at ph_holder):
	(- (HolderOf({something})) -).
To decide which object is next thing held after (something - object):
	(- (sibling({something})) -).
To decide which object is first thing held by (something - object):
	(- (child({something})) -).

Section SR5/6/1 - Understanding - Asking yes/no questions

To decide whether player consents
	(documented at ph_consents):
		(- YesOrNo() -).

Section SR5/6/2 - Understanding - The player's command

To decide if (S - a snippet) matches (T - a topic)
	(documented at act_reading):
	(- (SnippetMatches({S}, {T})) -).
To decide if (S - a snippet) does not match (T - a topic)
	(documented at act_reading):
	(- (SnippetMatches({S}, {T}) == false) -).
To decide if (S - a snippet) includes (T - a topic):
	(- (matched_text=SnippetIncludes({T},{S})) -).
To decide if (S - a snippet) does not include (T - a topic):
	(- (SnippetIncludes({T},{S})==0) -).

Section SR5/6/3 - Understanding - Changing the player's command

To change the text of the player's command to (txb - indexed text)
	(documented at act_reading):
	(- SetPlayersCommand({-pointer-to:txb}); -).
To replace (S - a snippet) with (T - text):
	(- SpliceSnippet({S}, {T}); -).
To cut (S - a snippet):
	(- SpliceSnippet({S}, 0); -).
To reject the player's command:
	(- RulebookFails(); rtrue; -) - in to only.

Section SR5/6/4 - Understanding - Scope and pronouns

To place (O - an object) in scope, but not its contents
	(documented at ph_scope):
	(- PlaceInScope({O}, {phrase options}); -).
To place the/-- contents of (O - an object) in scope
	(documented at ph_scope):
	(- ScopeWithin({O}); -).
To set pronouns from possessions of the player:
	(- PronounNoticeHeldObjects(); -).
To set pronouns from (O - an object):
	(- PronounNotice({O}); -).

Section SR5/6/5 - Understanding - I6 parser errors

To decide whether parser error is didn't understand
	(documented at act_parsererror):
	(- (etype == STUCK_PE) -).
To decide whether parser error is only understood as far as:
	(- (etype == UPTO_PE) -).
To decide whether parser error is didn't understand that number:
	(- (etype == NUMBER_PE) -).
To decide whether parser error is can't see any such thing:
	(- (etype == CANTSEE_PE) -).
To decide whether parser error is said too little:
	(- (etype == TOOLIT_PE) -).
To decide whether parser error is aren't holding that:
	(- (etype == NOTHELD_PE) -).
To decide whether parser error is can't use multiple objects:
	(- (etype == MULTI_PE) -).
To decide whether parser error is can only use multiple objects:
	(- (etype == MMULTI_PE) -).
To decide whether parser error is not sure what it refers to:
	(- (etype == VAGUE_PE) -).
To decide whether parser error is excepted something not included:
	(- (etype == EXCEPT_PE) -).
To decide whether parser error is can only do that to something animate:
	(- (etype == ANIMA_PE) -).
To decide whether parser error is not a verb I recognise:
	(- (etype == VERB_PE) -).
To decide whether parser error is not something you need to refer to:
	(- (etype == SCENERY_PE) -).
To decide whether parser error is can't see it at the moment:
	(- (etype == ITGONE_PE) -).
To decide whether parser error is didn't understand the way that finished:
	(- (etype == JUNKAFTER_PE) -).
To decide whether parser error is not enough of those available:
	(- (etype == TOOFEW_PE) -).
To decide whether parser error is nothing to do:
	(- (etype == NOTHING_PE) -).
To decide whether parser error is I beg your pardon:
	(- (etype == BLANKLINE_PE) -).
To decide whether parser error is noun did not make sense in that context:
	(- (etype == NOTINCONTEXT_PE) -).

Section SR5/7/1 - Using external resources - Files

To read (filename - external-file) into (T - table-name)
	(documented at ph_filetables):
	(- FileIO_GetTable({filename}, {T}); -).
To write (filename - external-file) from (T - table-name):
	(- FileIO_PutTable({filename}, {T}); -).
To decide if (filename - external-file) exists:
	(- (FileIO_Exists({filename}, false)) -).
To decide if ready to read (filename - external-file)
	(documented at ph_readytr):
	(- (FileIO_Ready({filename}, false)) -).
To mark (filename - external-file) as ready to read:
	(- FileIO_MarkReady({filename}, true); -).
To mark (filename - external-file) as not ready to read:
	(- FileIO_MarkReady({filename}, false); -).
To write (T - text) to (FN - external-file)
	(documented at ph_writef):
	(- FileIO_PutContents({FN}, {-allow-stack-frame-access:T}, false); -).
To append (T - text) to (FN - external-file):
	(- FileIO_PutContents({FN}, {-allow-stack-frame-access:T}, true); -).
To say text of (FN - external-file):
	(- FileIO_PrintContents({FN}); say__p = 1; -).

Section SR5/7/2 - Using external resources - Figures and sound effects

To display (F - figure-name), one time only
	(documented at ph_displayf):
	(- DisplayFigure({F}, {phrase options}); -).
To play (SFX - sound-name), one time only
	(documented at ph_playsf):
	(- PlaySound({SFX}, {phrase options}); -).

Section SR5/8/1 - Message support - Issuance - Unindexed

To stop the action with library message (AN - an action-name) number
	(N - a number) for (H - an object):
	(- return GL__M({AN},{N},{H}); -) - in to only.
To stop the action with library message (AN - an action-name) number
	(N - a number):
	(- return GL__M({AN},{N},noun); -) - in to only.
To issue miscellaneous library message number (N - a number):
	(- GL__M(##Miscellany,{N}); -).
To issue library message (AN - an action-name) number
	(N - a number) for (H - an object):
	(- GL__M({AN},{N},{H}); -).
To issue library message (AN - an action-name) number (N - a number):
	(- GL__M({AN},{N},noun); -).
To issue actor-based library message (AN - an action-name) number
	(N - a number) for (H - an object) and (H2 - an object):
	(- AGL__M({AN},{N},{H},{H2}); -).
To issue actor-based library message (AN - an action-name) number
	(N - a number) for (H - an object):
	(- AGL__M({AN},{N},{H}); -).
To issue actor-based library message (AN - an action-name) number (N - a number):
	(- AGL__M({AN},{N},noun); -).

To issue score notification message:
	(- NotifyTheScore(); -).
To say pronoun dictionary word:
	(- print (address) pronoun_word; -).
To say recap of command:
	(- PrintCommand(); -).
The pronoun reference object is an object that varies.
The pronoun reference object variable translates into I6 as "pronoun_obj".
The library message action is an action-name that varies.
The library message action variable translates into I6 as "lm_act".
The library message number is a number that varies.
The library message number variable translates into I6 as "lm_n".
The library message amount is a number that varies.
The library message amount variable translates into I6 as "lm_o".
The library message object is an object that varies.
The library message object variable translates into I6 as "lm_o".
The library message actor is an object that varies.
The library message actor variable translates into I6 as "actor".
The second library message object is an object that varies.
The second library message object variable translates into I6 as "lm_o2".

Section SR5/8/2 - Message support - Intervention - Unindexed

To decide if intervened in miscellaneous message:
	decide on false;

To decide if intervened in miscellaneous list message:
	decide on false;

To decide if intervened in action message:
	decide on false;

Section SR5/9/1 - Miscellaneous other phrases - Unindexed

To decide which object is the component parts core of (X - an object):
	(- CoreOf({X}) -).
To decide which object is the common ancestor of (O - an object) with
	(P - an object):
	 (- (CommonAncestor({O}, {P})) -).
To decide which object is the not-counting-parts holder of (O - an object):
	 (- (CoreOfParentOfCoreOf({O})) -).
To decide which object is the visibility-holder of (O - object):
	(- VisibilityParent({O}) -).
To calculate visibility ceiling at low level:
	(- FindVisibilityLevels(); -).

To decide which number is the visibility ceiling count calculated:
	(- visibility_levels -).
To decide which object is the visibility ceiling calculated:
	(- visibility_ceiling -).

To produce a room description with going spacing conventions:
	(- LookAfterGoing(); -).

To print the location's description:
	(- PrintOrRun(location, description); -).
To decide if (O - object) goes undescribed by source text:
	(- ({O}.description == 0) -).

To decide whether the I6 parser is running multiple actions:
	(- (multiflag==1) -).

To decide if set to sometimes abbreviated room descriptions:
	(- (lookmode == 1) -).
To decide if set to unabbreviated room descriptions:
	(- (lookmode == 2) -).
To decide if set to abbreviated room descriptions:
	(- (lookmode == 3) -).

To convert to (AN - an action-name) on (O - an object):
	(- return GVS_Convert({AN},{O},0); -) - in to only.
To convert to request of (X - object) to perform (AN - action-name) with
	(Y - object) and (Z - object):
	(- TryAction(true, {X}, {AN}, {Y}, {Z}); rtrue; -).
To convert to special going-with-push action:
	(- ConvertToGoingWithPush(); rtrue; -).

To surreptitiously move (something - object) to (something else - object):
	(- move {something} to {something else}; -).
To surreptitiously move (something - object) to (something else - object) during going:
	(- MoveDuringGoing({something}, {something else}); -).
To surreptitiously reckon darkness:
	(- SilentlyConsiderLight(); -).

The Standard Rules end here.

---- DOCUMENTATION ----

Unlike other extensions, the Standard Rules are compulsorily included
with every project. They define the phrases, kinds and relations which
are basic to Inform, and which are described throughout the documentation.
