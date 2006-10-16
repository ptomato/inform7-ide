!links.h library extension file
!By Jayson Smith
!This library provides some classes and the basic interfacing routines
!and verbs for Spider and Web style linkable objects.  If you have no idea
!what I'm talking about, please go play Andrew Plotkin's excellent game,
!"Spider and Web".  Also, a brief summary can be found in the piece of
!paper in links.inf, the sample game.
!Note that this code was not, repeat not, used in Spider and Web.  In fact,
!S&W was what inspired me to write this library!
!This library and the sample game may be distributed freely.  Having said
!that, if you use any of this code in a game, I'd appreciate at least a
!"Thank you" in your game.
!Notes:
!This code is very poorly formatted E.G. no indentation.  Also, it may
!be written sloppily, but appears not to have much in the way of bugs.
!If you do find any bugs, please send me a letter at jaybird@coldmail.com.
!Classes in this library are:
!actuator, used for actuators E.G. buttons, switches, timers, etc, which 
!are used to activate or trigger devices.
!device is for devices, E.G. beepers, lights, blenders, etc.
!switch is what it sounds like, an actuator that is a switch, with proper
!entries for switchon and switchoff to call the turnon and turnoff routines.
!remote is used for a device which is meant to remotely control an actuator.
!This is used for such things as the voice and key transmitters in Spider
!and Web, and the remote control in my links.inf sample game.
!Attributes you need to put on your linkable objects are:
!pluslink indicates that this object has a plus link.  This is usually, but
!not always, used for actuators, at least it always was in Spider and Web.
!minuslink means this object has a minus link, which is usually, but not
!always, used for devices, although it always was in Spider and Web.
!Note that you can't link two objects with the same type of link together.
!Also note that once you've chosen what types of links to use for actuators
!and devices, it's a good idea to stick to those types so as not to confuse
!anybody.
!Routines and properties you should provide and know about:
!turnon is a string or routine which gets executed or run when a device is
!turned on via an actuator.  It can do anything you want, even kill the
!player, win the game, or anything else that can normally be done.
!turnoff is the opposite of turnon in that it is what is run/executed when
!a device is turned off.
!brief is what happens when a device is activated briefly, such as the push
!of a button.  If it's a device which does the same thing either way, such
!as a bomb, you can just have the brief call self.turnon to get the same
!result.
!baseunit is what tells a remote control what to control.  Although it
!really doesn't get used in this code, your code may use it if it wishes.
!Also note that remote baseunits needn't have any means by which to activate
!them, because that's what the remotes are there for!
!Have fun with this!

attribute remotely;
attribute pluslink;
attribute minuslink;
fake_action linktake;
fake_action linkdrop;
global temporary;
class actuator
with before [;
take:
linkedtake(self,self.linkto);
drop:
linkeddrop(self,self.linkto);
],
with after [;
examine: print "It has a ";
if (self has pluslink) print "plus link ";
else print "minus link ";
print " which ";
if (self.linkto==0) "isn't connected to anything.";
else print "is connected to ",(a) self.linkto,".^";
],
with turnon [;
if (self.linkto==0) {
if (self hasnt remotely) "Nothing happens.";
else "You perceive no other effect.";
};
temporary=self.linkto;
temporary.turnon();
rtrue;
],
with turnoff [;
if (self.linkto==0) {
if (self hasnt remotely) "Nothing happens.";
else "You perceive no other effect.";
};
temporary=self.linkto;
temporary.turnoff();
],
with brief [;
if (self.linkto==0) {
if (self hasnt remotely) "Nothing happens.";
else "You perceive no other effect.";
};
temporary=self.linkto;
temporary.brief();
],
with linkto;
class remote
with baseunit, turnon, turnoff, brief;
class device
with before [;
take:
linkedtake(self,self.linkto);
drop:
linkeddrop(self,self.linkto);
],
with after [;
examine: print "It has a ";
if (self has pluslink) print "plus link ";
else print "minus link ";
print "which ";
if (self.linkto==0) "isn't connected to anything.";
else print "is connected to ",(a) self.linkto,".^";
],
with brief, turnon, turnoff, linkto;
class switch
class actuator
has switchable,
with after [;
switchon: print "You flip ",(the) self," on.^";
self.turnon();
rtrue;
switchoff: print "You flip ",(the) self," off.^";
self.turnoff();
rtrue;
];
verb "link" "connect" * noun "to" noun -> link;
verb "unlink" "disconnect"
* noun -> unlink;
[linksub;
if (noun has remotely) {
print (the) noun," doesn't have a link.  It works via ",(the) self.baseunit,".^";
rtrue;
};
if (second has remotely) {
print (the) second," doesn't have a link.  It works via",
(the) second.baseunit,".^";
rtrue;
};
if (noun has pluslink && (second has minuslink)) {
link(noun,second);
rtrue;
}
if (noun has minuslink && (second has pluslink)) {
link(noun,second);
rtrue;
}
if (noun has pluslink && (second has pluslink)) {
"You can't link those together because they both have plus links.^";
}
if (noun has minuslink && (second has minuslink)) {
"You can't link those together because they both have minus links.";
}
if (noun hasnt pluslink && (noun hasnt minuslink)) {
print (the) noun," doesn't have a link so you can't link it to anything.^";
rtrue;
}
if (second hasnt pluslink && (second hasnt minuslink)) {
print (the) second," doesn't have a link so you can't link anything to it.^";
rtrue;
}
if (noun.linkto~=0) {
print (the) noun," is already linked to ",(the) noun.linkto,".  You'll 
have to unlink it first.^";
rtrue;
}
if (second.linkto~=0) {
print (the) second," is already linked to ",(the) second.linkto,".  You'll 
have to unlink it first.^";
rtrue;
}
];
[link i j;
i.linkto=j;
j.linkto=i;
print "You link ",(the) i," to ",(the) j,".^";
if (indirectlycontains(player,i) && (indirectlycontains(player,j) == false)) {
print "You also let go of ",(the) i,".^";
move i to parent(player);
};
if (indirectlycontains(player,i) == false && (indirectlycontains(player,j))) {
print "You also let go of ",(the) j,".^";
move j to parent(player);
};
if (i has on) {
j.turnon();
}
];
[unlinksub;
if (noun has remotely) {
print (the) noun," doesn't have a link.  It works via ", (the) noun.baseunit,".^";
rtrue;
};
if (second==0) {
if (noun provides linkto) {
temporary=noun.linkto;
if (temporary==0) "That isn't linked to anything.^";
print "You unlink ",(the) noun," from ",(the) temporary,".^";
noun.linkto=0;
temporary.linkto=0;
if (noun has on) {
temporary.turnoff();
}
rtrue;
}
}
else if (noun provides linkto && (second provides linkto)) {
if (noun.linkto ~= second) "Those aren't linked together.^";
print "You unlink ",(the) noun," from ",(the) second,".^";
noun.linkto=0;
second.linkto=0;
if (noun has on) {
second.turnoff();
}
rtrue;
}
else if (noun provides ~linkto) {
print (the) noun," doesn't have a link.^";
rtrue;
}
else if (second~=0) {
if (second provides ~linkto) {
print (the) second," doesn't have a link.^";
rtrue;
}
}
];
[linkedtake i j;
if (j==0) rtrue;
if (i in player) rtrue;
if (i has static || (i has scenery)) rtrue;
action=##linktake;
if (runroutines(j,before) ~= 0 || (j has static || (j has scenery))) {
print "You'll have to disconnect ",(the) i," from ",(the) j," first.^";
rtrue;
}
else {
if (runroutines(i,before)~=0 || (i has static || (i has scenery))) {
print "You'll have to disconnect ",(the) i," from ",(the) j," first.^";
rtrue;
}
else
if (j hasnt concealed && j hasnt static) move j to player;
if (i hasnt static && i hasnt concealed) move i to player;
action=##linktake;
if (runroutines(j,after) ~= 0) rtrue;
print "You take ",(the) i," and ",(the) j," connected to it.^";
rtrue;
}
];
[linkeddrop i j;
if (j==0) rtrue;
if (i notin player) rtrue;
action=##linkdrop;
if (runroutines(j,before)~=0) {
print "You'll have to disconnect ",(the) i," from ",(the) j," first.^";
rtrue;
}
else {
if (runroutines(i,before)~=0) {
print "You'll have to disconnect ",(the) i," from ",(the) j," first.^";
rtrue;
}
else
move i to location;
move j to location;
action=##linkdrop;
if (runroutines(j,after)~=0) rtrue;
print "You drop ",(the) i," and ",(the) j," connected to it.^";
rtrue;
}
];
