
SUBDIRS = \
	src \
	documents \
	.

EXTRA_DIST = \
	AUTHORS \
	COPYING \
	ChangeLog \
	INSTALL \
	LICENSE \
	NEWS \
	NOTICE \
	README \
	RELEASENOTES \
	msvc80

debug:
	$(MAKE) all CXXFLAGS="$(EXTRA_CXXFLAGS) $(DEBUG_CXXFLAGS)"

opt:
	$(MAKE) all CXXFLAGS="$(EXTRA_CXXFLAGS) $(OPT_CXXFLAGS)"

	
## Taf!