
include $(top_srcdir)/config/Make-rules

noinst_LIBRARIES = libsystemc-ams.a

H_FILES = \
	systemc-ams \
	systemc-ams.h \
	config.h

noinst_HEADERS = $(H_FILES)

CXX_FILES =

NO_CXX_FILES =

libsystemc_ams_a_SOURCES = $(H_FILES) $(CXX_FILES)

SUBDIRS = \
	scams \
	.

INCLUDE_DIR = $(prefix)/include

install-data-local:
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(INCLUDE_DIR)
	for file in $(noinst_HEADERS); do \
	  $(INSTALL_DATA) $(srcdir)/$$file $(INCLUDE_DIR)/$$file; \
	done
	$(mkinstalldirs) $(prefix)/lib-$(TARGET_ARCH)
	$(INSTALL_DATA) $(MAIN_LIBRARY) \
	  $(prefix)/lib-$(TARGET_ARCH)/$(MAIN_LIBRARY)

uninstall-local:
	@$(NORMAL_UNINSTALL)
	for file in $(noinst_HEADERS); do \
	  rm -f $(INCLUDE_DIR)/$$file; \
	done
	-rmdir $(INCLUDE_DIR)
	rm -f $(prefix)/lib-$(TARGET_ARCH)/$(MAIN_LIBRARY)
	-rmdir $(prefix)/lib-$(TARGET_ARCH)

## Taf!
