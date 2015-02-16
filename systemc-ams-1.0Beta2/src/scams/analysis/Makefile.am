
include $(top_srcdir)/config/Make-rules

SUBDIRS = \
    ac \
	.

INCLUDE_DIR = $(prefix)/include/scams/analysis

install-data-local:
	@$(NORMAL_INSTALL)

uninstall-local:
	@$(NORMAL_UNINSTALL)
	-rmdir $(INCLUDE_DIR)

## Taf!
