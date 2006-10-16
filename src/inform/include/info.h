! Info.h - A Library Extention For Consultation And Conversation
! By Jesse Burneko
!
! Purpose:
! The idea for this library extention has been brewing around my mind
! for quite some time.  The idea is to allow Consult, Ask, Tell, and
! Answer to access a database of topic objects rathen than having to
! parse the information themselves.  As a mock up version of this
! library file I had taken Gareth Rees' Frobozzica.inf example, edited
! it to handle Tell and Answer as well as Ask, and then transformed it
! into an include file.  As inform has developed I began to suspect
! that there was a much simpler way of doing things.  This library file 
! is the fruit of my labor.
!
! Usage:
! 1) Create a Topics object to be the parent object of all topics.
! 2) Create a series of topic objects.  The only requirements being that
!    they are a child of the Topics object, inherit from the
!    Topic class, provide the name and/or parse_name property for
!    identification in the usual way.
! 3) In the usual manner trap the Consult, Ask, Tell, and/or Answer
!    action in the before or life routine of the target object.  The topic
!    being refered to will be held in the second variable.
!
! Example:
!
! Object Topics;
!
! Topic -> Virginia_Woolf
!   with   name "virgina" "woolf",
!
! Topic -> Orlando
!   with   name "orlando" "novel" "book",
!
! Object Encyclopedia "Encyclopedia Of English Literature",
!   with   name "encyclopedia" "of" "english" "literature" "book",
!   before
!   [;
!      Consult:
!        switch(second)
!        {
!         Virginia_Woolf:
!            "~Woolf, Virginia (1182-1941), English novelist and
!            essayist.  The daughter of Sir Leslie Stephen, she
!            married the critic Leonard Sidney Woolf (1880-1969) and
!            they established the Hogarth Press (1917).  Novels using
!            stream of consciousness, such as Mrs. Dalloway (1925), To
!            the Lighthouse (1927) and The Waves (1931), concern her
!            characters' thoughs and feelings about common
!            experiences.  Some of her brilliant criticism was
!            published in The Common Reader (1925).  Subject to fits
!            of mental instability, she finally drowned herself.~";
!         Orlando:
!            "~A novel written in 1928 by Virgina Woolf outlining the
!            unual life of Orlando, an individual who lives for 400
!            years and is at one time male and another time female.
!            She also: Woolf, Virgina.~";
!         }
!   ];
!
! Note: Topics created need not initially be a child of the Topics
! object.  This allows for topics to begin out of scope and then as
! the player gains more information topics can be moved in and out of
! the Topics object at will.

[ TopicScope;
    switch(scope_stage)
    {
     1: rfalse;
     2: ScopeWithin(Topics);
	rtrue;
     3: "Error: Parsing should have matched previous grammar line.";
    }
];

Class 	Topic
 with 	short_name "that",
 has 	proper;

Extend 'look' first
    * 					-> Look
    * 'up' scope = TopicScope 'in' noun	-> Consult reverse;

Extend 'consult' first
    * noun 'about' scope = TopicScope 	-> Consult;

Extend 'ask' first
    * creature 'about' scope = TopicScope -> Ask;

Extend 'tell' first
    * creature 'about' scope = TopicScope -> Tell;

Extend 'answer' first
    * scope = TopicScope 'to' creature 	-> Answer;


