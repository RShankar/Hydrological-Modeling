
include $(top_srcdir)/config/Make-rules

SUBDIRS = \
    conservative \
    eln \
    tdf \
    lsf \
	.

INCLUDE_DIR = $(prefix)/include/scams/impl/predefined_moc

install-data-local:
	@$(NORMAL_INSTALL)

uninstall-local:
	@$(NORMAL_UNINSTALL)
	-rmdir $(INCLUDE_DIR)

## Taf!
