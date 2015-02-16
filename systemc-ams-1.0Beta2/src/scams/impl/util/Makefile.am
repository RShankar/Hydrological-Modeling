
include $(top_srcdir)/config/Make-rules

SUBDIRS = \
    data_types \
    tracing \
    reporting \
	.

INCLUDE_DIR = $(prefix)/include/scams/impl/util

install-data-local:
	@$(NORMAL_INSTALL)

uninstall-local:
	@$(NORMAL_UNINSTALL)
	-rmdir $(INCLUDE_DIR)

## Taf!
