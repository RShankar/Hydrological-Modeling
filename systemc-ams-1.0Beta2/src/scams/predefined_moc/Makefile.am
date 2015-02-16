
include $(top_srcdir)/config/Make-rules

SUBDIRS = \
    eln \
    lsf \
    tdf \
	.

INCLUDE_DIR = $(prefix)/include/scams/predefined_moc

install-data-local:
	@$(NORMAL_INSTALL)

uninstall-local:
	@$(NORMAL_UNINSTALL)
	-rmdir $(INCLUDE_DIR)

## Taf!
