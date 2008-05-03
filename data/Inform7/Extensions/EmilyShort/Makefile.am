emilyshortdir = $(pkgdatadir)/Inform7/Extensions/Emily Short
emilyshort_DATA = \
BasicHelpMenu BasicScreenEffects CaseManagement ComplexListing \
GlulxEntryPoints GlulxImageCentering GlulxTextEffects Locksmith Menus \
Plurality PunctuationRemoval
EXTRA_DIST = $(emilyshort_DATA)

install-data-hook:
	pushd "$(DESTDIR)$(emilyshortdir)"; \
	mv BasicHelpMenu Basic\ Help\ Menu; \
	mv BasicScreenEffects Basic\ Screen\ Effects; \
	mv CaseManagement Case\ Management; \
	mv ComplexListing Complex\ Listing; \
	mv GlulxEntryPoints Glulx\ Entry\ Points; \
	mv GlulxImageCentering Glulx\ Image\ Centering; \
	mv GlulxTextEffects Glulx\ Text\ Effects; \
	mv PunctuationRemoval Punctuation\ Removal; \
	popd

uninstall-hook:
	pushd "$(DESTDIR)$(emilyshortdir)"; \
	rm -f Basic\ Help\ Menu; \
	rm -f Basic\ Screen\ Effects; \
	rm -f Case\ Management; \
	rm -f Complex\ Listing; \
	rm -f Glulx\ Entry\ Points; \
	rm -f Glulx\ Image\ Centering; \
	rm -f Glulx\ Text\ Effects; \
	rm -f Punctuation\ Removal; \
	popd
