
include $(top_srcdir)/config/Make-rules

SUBDIRS = \
    data_types \
    reporting \
    tracing \
	.

INCLUDE_DIR = $(prefix)/include/scams/utility

install-data-local:
	@$(NORMAL_INSTALL)

uninstall-local:
	@$(NORMAL_UNINSTALL)
	-rmdir $(INCLUDE_DIR)

## Taf!
