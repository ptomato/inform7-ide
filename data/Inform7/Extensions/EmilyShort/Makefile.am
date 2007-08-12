emilyshortdir = $(pkgdatadir)/Inform7/Extensions/Emily Short
emilyshort_DATA = \
BasicHelpMenu BasicScreenEffects CaseManagement ComplexListing \
GlulxEntryPoints GlulxImageCentering GlulxTextEffects Locksmith Menus \
Plurality PunctuationRemoval
EXTRA_DIST = $(emilyshort_DATA)

install-data-hook:
	pushd "$(emilyshortdir)"; \
	mv BasicHelpMenu Basic\ Help\ Menu; \
	mv BasicScreenEffects Basic\ Screen\ Effects; \
	mv CaseManagement Case\ Management; \
	mv ComplexListing Complex\ Listing; \
	mv GlulxEntryPoints Glulx\ Entry\ Points; \
	mv GlulxImageCentering Glulx\ Image\ Centering; \
	mv GlulxTextEffects Glulx\ Text\ Effects; \
	mv PunctuationRemoval Punctuation\ Removal; \
	ln -s Basic\ Help\ Menu basic\ help\ menu; \
	ln -s Basic\ Screen\ Effects basic\ screen\ effects; \
	ln -s Case\ Management case\ management; \
	ln -s Complex\ Listing complex\ listing; \
	ln -s Glulx\ Entry\ Points glulx\ entry\ points; \
	ln -s Glulx\ Image\ Centering glulx\ image\ centering; \
	ln -s Glulx\ Text\ Effects glulx\ text\ effects; \
	ln -s Locksmith locksmith; \
	ln -s Menus menus; \
	ln -s Plurality plurality; \
	ln -s Punctuation\ Removal punctuation\ removal; \
	popd

uninstall-hook:
	pushd "$(emilyshortdir)"; \
	rm -f Basic\ Help\ Menu basic\ help\ menu; \
	rm -f Basic\ Screen\ Effects basic\ screen\ effects; \
	rm -f Case\ Management case\ management; \
	rm -f Complex\ Listing complex\ listing; \
	rm -f Glulx\ Entry\ Points glulx\ entry\ points; \
	rm -f Glulx\ Image\ Centering glulx\ image\ centering; \
	rm -f Glulx\ Text\ Effects glulx\ text\ effects; \
	rm -f locksmith; \
	rm -f menus; \
	rm -f plurality; \
	rm -f Punctuation\ Removal punctuation\ removal; \
	popd
