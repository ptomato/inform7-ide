grahamnelsondir = $(pkgdatadir)/Inform7/Extensions/Graham Nelson
grahamnelson_DATA = RideableVehicles StandardRules UnicodeCharacterNames \
UnicodeFullCharacterNames
EXTRA_DIST = $(grahamnelson_DATA)

install-data-hook:
	pushd "$(DESTDIR)$(grahamnelsondir)"; \
	mv RideableVehicles Rideable\ Vehicles; \
	mv StandardRules Standard\ Rules; \
	mv UnicodeCharacterNames Unicode\ Character\ Names; \
	mv UnicodeFullCharacterNames Unicode\ Full\ Character\ Names; \
	ln -s Rideable\ Vehicles rideable\ vehicles; \
	ln -s Standard\ Rules standard\ rules; \
	ln -s Unicode\ Character\ Names unicode\ character\ names; \
	ln -s Unicode\ Full\ Character\ Names unicode\ full\ character\ names; \
	popd
	
uninstall-hook:
	pushd "$(DESTDIR)$(grahamnelsondir)"; \
	rm -f Rideable\ Vehicles rideable\ vehicles; \
	rm -f Standard\ Rules standard\ rules; \
	rm -f Unicode\ Character\ Names unicode\ character\ names; \
	rm -f Unicode\ Full\ Character\ Names unicode\ full\ character\ names; \
	popd
