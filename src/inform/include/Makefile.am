## Automake template for standard include files.

INCLUDES = action.h adhints.h altmenu.h boxclever.h branch.h		    \
calyx_adjectives.h ccpl.h center.h clothes.h clothing.h compass.h daemons.h \
diary.h dirs.h domenu.h doors.h dynobj.h easydoors.h expertgrammar.h	    \
fkeys.h follower.h footnote.h goneighbour.h helproutines.h hints.h	    \
hintsms.h howtoplay.h info.h istring.h links.h locksmith.h longint.h	    \
manual.h menus.h mistype.h money.h moveclass.h movie.h nameable.h	    \
newbiegrammar.h newmenu.h newmenus.h npc_engine.h oldaemon.h outofrch.h	    \
phtalkoo.h physics.h pluralob.h pname.h printtime.h recept.h		    \
sampletranscript.h scenic.h scoper.h showobj.h shuffle.h smartcantgo.h	    \
sound.h style.h text_functions.h timepiece.h timesys.h timewait.h torch.h   \
trinitystat.h utility.h waittime.h whatis.h whowhat.h yesno.h zclock.h	    \
znsi.h

EXTRA_NAMES = CCPL.h AdHints.h ExpertGrammar.h GoNeighbour.h HelpRoutines.h \
Locksmith.h Menus.h NewMenu.h NewbieGrammar.h NPC_ENGINE.h		    \
SampleTranscript.h Scenic.h Scoper.h SmartCantGo.h

DOCS = adhints.txt links.inf npc_engine.inf npc_engine.txt pname.txt	\
text_functions.inf

include_HEADERS = $(INCLUDES) $(DOCS)

EXTRA_DIST = $(DOCS)

if CYGWIN
install-data-hook: add-localdir
uninstall-local: remove-localdir
else
install-data-hook: add-localdir add-names
uninstall-local: remove-localdir remove-names
endif

add-localdir:
	$(mkinstalldirs) $(DESTDIR)$(localincdir)

remove-localdir:
	rmdir $(DESTDIR)$(localincdir)

add-names:
	@ cd $(DESTDIR)$(includedir);					\
	for file in $(EXTRA_NAMES); do					\
	   realfile=`echo $$file | tr '[A-Z]' '[a-z]'`;			\
	   echo " linking $$file to $$realfile";			\
	   rm -f $$file;						\
	   $(LN_S) $$realfile $$file;					\
	done

remove-names:
	@ for file in $(EXTRA_NAMES); do				\
	   echo " unlinking $$file";					\
	   rm -f $(DESTDIR)$(includedir)/$$file;			\
	done

@make_include@
