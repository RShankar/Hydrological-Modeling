
include $(top_srcdir)/config/Make-rules

noinst_LIBRARIES = libsynchronization.a

H_FILES = \
    sca_sync_value_handle.h
    
    
NO_H_FILES = \
    sca_synchronization_alg.h \
    sca_synchronization_layer_process.h \
    sca_synchronization_layer.h \
    sca_synchronization_obj_if.h

noinst_HEADERS = $(H_FILES)

CXX_FILES = \
	sca_sync_value_handle.cpp \
	sca_synchronization_alg.cpp \
	sca_synchronization_layer_process.cpp \
	sca_synchronization_layer.cpp

NO_CXX_FILES =

libsynchronization_a_SOURCES = $(H_FILES) $(NO_H_FILES) $(CXX_FILES) 

INCLUDE_DIR = $(prefix)/include/scams/impl/synchronization

install-data-local:
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(INCLUDE_DIR)
	for file in $(noinst_HEADERS); do \
	  $(INSTALL_DATA) $(srcdir)/$$file $(INCLUDE_DIR)/$$file; \
	done
	$(AR_TOOL) $(top_builddir)/src/$(MAIN_LIBRARY) \
	  $(libsynchronization_a_OBJECTS)

uninstall-local:
	@$(NORMAL_UNINSTALL)
	for file in $(noinst_HEADERS); do \
	  rm -f $(INCLUDE_DIR)/$$file; \
	done
	-rmdir $(INCLUDE_DIR)

## Taf!
