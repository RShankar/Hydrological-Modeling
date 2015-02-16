
include $(top_srcdir)/config/Make-rules

# noinst_LIBRARIES = libdata_types.a

H_FILES = \
	sca_matrix_base.h \
	sca_method_list.h \
	sca_method_object.h \
	sca_function_vector.h
		
	
NO_H_FILES =

noinst_HEADERS = $(H_FILES)

CXX_FILES = 

NO_CXX_FILES =

# libdata_types_a_SOURCES = $(H_FILES) $(NO_H_FILES) $(CXX_FILES) 

INCLUDE_DIR = $(prefix)/include/scams/impl/util/data_types

install-data-local:
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(INCLUDE_DIR)
	for file in $(noinst_HEADERS); do \
	  $(INSTALL_DATA) $(srcdir)/$$file $(INCLUDE_DIR)/$$file; \
	done
#	$(AR_TOOL) $(top_builddir)/src/$(MAIN_LIBRARY) \
#	  $(libdata_types_a_OBJECTS)

uninstall-local:
	@$(NORMAL_UNINSTALL)
	for file in $(noinst_HEADERS); do \
	  rm -f $(INCLUDE_DIR)/$$file; \
	done
	-rmdir $(INCLUDE_DIR)

## Taf!
