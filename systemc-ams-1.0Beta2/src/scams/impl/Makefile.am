
include $(top_srcdir)/config/Make-rules

SUBDIRS = \
    core \
    predefined_moc \
    solver \
    synchronization \
    util \
    analysis \
	.

INCLUDE_DIR = $(prefix)/include/scams/impl

install-data-local:
	@$(NORMAL_INSTALL)

uninstall-local:
	@$(NORMAL_UNINSTALL)
	-rmdir $(INCLUDE_DIR)

## Taf!
