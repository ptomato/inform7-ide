grahamnelsondir = $(pkgdatadir)/Inform7/Extensions/Graham Nelson
grahamnelson_DATA = RideableVehicles.i7x StandardRules.i7x \
UnicodeCharacterNames.i7x UnicodeFullCharacterNames.i7x
EXTRA_DIST = $(grahamnelson_DATA)

install-data-hook:
	pushd "$(DESTDIR)$(grahamnelsondir)"; \
	mv RideableVehicles.i7x Rideable\ Vehicles.i7x; \
	mv StandardRules.i7x Standard\ Rules.i7x; \
	mv UnicodeCharacterNames.i7x Unicode\ Character\ Names.i7x; \
	mv UnicodeFullCharacterNames.i7x Unicode\ Full\ Character\ Names.i7x; \
	popd
	
uninstall-hook:
	pushd "$(DESTDIR)$(grahamnelsondir)"; \
	rm -f Rideable\ Vehicles.i7x; \
	rm -f Standard\ Rules.i7x; \
	rm -f Unicode\ Character\ Names.i7x; \
	rm -f Unicode\ Full\ Character\ Names.i7x; \
	popd
