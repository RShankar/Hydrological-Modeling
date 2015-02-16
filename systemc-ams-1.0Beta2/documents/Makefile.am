##
##
##

include $(top_srcdir)/config/Make-rules



H_FILES = 

noinst_HEADERS = $(H_FILES)

CXX_FILES =

NO_CXX_FILES =


INCLUDE_DIR = $(prefix)/include


uninstall-local:
	@$(NORMAL_UNINSTALL)
	for file in $(noinst_HEADERS); do \
	  rm -f $(INCLUDE_DIR)/$$file; \
	done
	-rmdir $(INCLUDE_DIR)
	rm -f $(prefix)/lib-$(TARGET_ARCH)/$(MAIN_LIBRARY)
	-rmdir $(prefix)/lib-$(TARGET_ARCH)

## Taf!
