
include $(top_srcdir)/config/Make-rules

SUBDIRS = \
    linear \
    tdf \
    utilities \
	.

INCLUDE_DIR = $(prefix)/include/scams/impl/solver

install-data-local:
	@$(NORMAL_INSTALL)

uninstall-local:
	@$(NORMAL_UNINSTALL)
	-rmdir $(INCLUDE_DIR)

## Taf!
