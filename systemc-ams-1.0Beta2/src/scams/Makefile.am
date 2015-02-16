
include $(top_srcdir)/config/Make-rules

SUBDIRS = \
	analysis \
	core \
	impl \
	predefined_moc \
	utility \
	.

INCLUDE_DIR = $(prefix)/include/scams

install-data-local:
	@$(NORMAL_INSTALL)

uninstall-local:
	@$(NORMAL_UNINSTALL)
	-rmdir $(INCLUDE_DIR)

## Taf!
