
include $(top_srcdir)/config/Make-rules

noinst_LIBRARIES = libcore.a

H_FILES = \
    sca_port_base.h \
    sca_port_impl.h
     
		
NO_H_FILES = \
    sca_globals.h \
    sca_globals.h \
    sca_object_manager.h \
    sca_simcontext.h \
    sca_solver_base.h \
    sca_solver_manager.h \
    sca_view_base.h \
    sca_view_manager.h \
    sca_view_object.h

noinst_HEADERS = $(H_FILES)

CXX_FILES = \
	sca_globals.cpp \
	sca_interface.cpp \
	sca_module.cpp \
	sca_object_manager.cpp \
	sca_parameter_base.cpp \
	sca_port_base.cpp \
	sca_prim_channel.cpp \
	sca_simcontext.cpp \
	sca_solver_base.cpp \
	sca_solver_manager.cpp \
	sca_view_base.cpp \
	sca_view_manager.cpp \
	sca_view_object.cpp

NO_CXX_FILES =

libcore_a_SOURCES = $(H_FILES) $(NO_H_FILES) $(CXX_FILES) 

INCLUDE_DIR = $(prefix)/include/scams/impl/core

install-data-local:
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(INCLUDE_DIR)
	for file in $(noinst_HEADERS); do \
	  $(INSTALL_DATA) $(srcdir)/$$file $(INCLUDE_DIR)/$$file; \
	done
	$(AR_TOOL) $(top_builddir)/src/$(MAIN_LIBRARY) \
	  $(libcore_a_OBJECTS)

uninstall-local:
	@$(NORMAL_UNINSTALL)
	for file in $(noinst_HEADERS); do \
	  rm -f $(INCLUDE_DIR)/$$file; \
	done
	-rmdir $(INCLUDE_DIR)

## Taf!
