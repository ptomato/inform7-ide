grahamnelsondir = $(pkgdatadir)/extensions/Graham Nelson
grahamnelson_DATA = RideableVehicles StandardRules UnicodeCharacterNames \
UnicodeFullCharacterNames
EXTRA_DIST = $(grahamnelson_DATA)

install-data-hook:
	pushd "$(grahamnelsondir)"; \
	mv RideableVehicles Rideable\ Vehicles; \
	mv StandardRules Standard\ Rules; \
	mv UnicodeCharacterNames Unicode\ Character\ Names; \
	mv UnicodeFullCharacterNames Unicode\ Full\ Character\ Names; \
	popd
	
uninstall-hook:
	pushd "$(grahamnelsondir)"; \
	rm -f Rideable\ Vehicles; \
	rm -f Standard\ Rules; \
	rm -f Unicode\ Character\ Names; \
	rm -f Unicode\ Full\ Character\ Names; \
	popd
