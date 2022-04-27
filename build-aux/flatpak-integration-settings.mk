# Special make settings for copying Inform core in the Flatpak environment.
# This file is installed as make-integration-settings.mk in the root build dir.

INTEGRATION = TRUE

# We copy the resources into /app/tmp which is cleaned up later, but the
# subsequent step can find the files it needs there.

BUILTINCOMPS = /app/tmp/intools
INTERNAL = /app/tmp/data
BUILTINHTML = /app/tmp/inform
BUILTINHTMLINNER = /app/tmp/inform/en

# Names of the tools and options are the same as in the regular Linux build,
# except we have an Indoc profile that installs to /app/tmp

INBLORBNAME = cBlorb
INFORM6NAME = inform6
INFORM7NAME = ni
INTESTNAME = intest

INDOCOPTS = gnome_flatpak_app
INRTPSOPTS = -font
