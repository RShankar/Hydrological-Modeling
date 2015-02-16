
include $(top_srcdir)/config/Make-rules

noinst_LIBRARIES = libsparse_library.a

H_FILES = 
		
NO_H_FILES = \
    ana_solv_data.h \
    linear_analog_solver.h \
    ma_sparse.h \
    ma_typedef.h \
    ma_util.h

noinst_HEADERS = $(H_FILES)

CXX_FILES = \
	ana_init.c \
	ana_reinit.c \
	ana_solv.c \
	MA_generate_sparse.c \
	MA_lequspar.c \
	MA_matfull.c \
	MA_matspars.c

NO_CXX_FILES =

libsparse_library_a_SOURCES = $(H_FILES) $(NO_H_FILES) $(CXX_FILES) 

INCLUDE_DIR = $(prefix)/include/scams/impl/solver/utilities/sparse_library

install-data-local:
#	@$(NORMAL_INSTALL)
#	$(mkinstalldirs) $(INCLUDE_DIR)
#	for file in $(noinst_HEADERS); do \
#	  $(INSTALL_DATA) $(srcdir)/$$file $(INCLUDE_DIR)/$$file; \
#	done
	$(AR_TOOL) $(top_builddir)/src/$(MAIN_LIBRARY) \
	  $(libsparse_library_a_OBJECTS)

#uninstall-local:
#	@$(NORMAL_UNINSTALL)
#	for file in $(noinst_HEADERS); do \
#	  rm -f $(INCLUDE_DIR)/$$file; \
#	done
#	-rmdir $(INCLUDE_DIR)

## Taf!
