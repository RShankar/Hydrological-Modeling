
include $(top_srcdir)/config/Make-rules

SUBDIRS = \
    sparse_library \
	.

INCLUDE_DIR = $(prefix)/include/scams/impl/solver/utilities

install-data-local:
	@$(NORMAL_INSTALL)

uninstall-local:
	@$(NORMAL_UNINSTALL)
	-rmdir $(INCLUDE_DIR)

## Taf!
