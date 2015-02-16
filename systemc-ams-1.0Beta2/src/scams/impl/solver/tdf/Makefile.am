
include $(top_srcdir)/config/Make-rules

noinst_LIBRARIES = libsolvertdf.a

H_FILES = 
		
NO_H_FILES = \
    sca_tdf_solver.h

noinst_HEADERS = $(H_FILES)

CXX_FILES = \
	sca_tdf_solver.cpp

NO_CXX_FILES =

libsolvertdf_a_SOURCES = $(H_FILES) $(NO_H_FILES) $(CXX_FILES) 

INCLUDE_DIR = $(prefix)/include/scams/impl/solver/tdf

install-data-local:
#	@$(NORMAL_INSTALL)
#	$(mkinstalldirs) $(INCLUDE_DIR)
#	for file in $(noinst_HEADERS); do \
#	  $(INSTALL_DATA) $(srcdir)/$$file $(INCLUDE_DIR)/$$file; \
#	done
	$(AR_TOOL) $(top_builddir)/src/$(MAIN_LIBRARY) \
	  $(libsolvertdf_a_OBJECTS)

#uninstall-local:
#	@$(NORMAL_UNINSTALL)
#	for file in $(noinst_HEADERS); do \
#	  rm -f $(INCLUDE_DIR)/$$file; \
#	done
#	-rmdir $(INCLUDE_DIR)

## Taf!
