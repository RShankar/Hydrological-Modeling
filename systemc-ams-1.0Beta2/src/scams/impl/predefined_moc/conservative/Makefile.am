
include $(top_srcdir)/config/Make-rules

noinst_LIBRARIES = libconservative.a

H_FILES = \
    sca_conservative_module.h \
    sca_conservative_interface.h \
    sca_conservative_signal.h
		
NO_H_FILES = \
    sca_conservative_view.h

noinst_HEADERS = $(H_FILES)

CXX_FILES = \
	sca_conservative_module.cpp \
	sca_conservative_signal.cpp \
	sca_conservative_view.cpp

NO_CXX_FILES =

libconservative_a_SOURCES = $(H_FILES) $(NO_H_FILES) $(CXX_FILES) 

INCLUDE_DIR = $(prefix)/include/scams/impl/predefined_moc/conservative

install-data-local:
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(INCLUDE_DIR)
	for file in $(noinst_HEADERS); do \
	  $(INSTALL_DATA) $(srcdir)/$$file $(INCLUDE_DIR)/$$file; \
	done
	$(AR_TOOL) $(top_builddir)/src/$(MAIN_LIBRARY) \
	  $(libconservative_a_OBJECTS)

uninstall-local:
	@$(NORMAL_UNINSTALL)
	for file in $(noinst_HEADERS); do \
	  rm -f $(INCLUDE_DIR)/$$file; \
	done
	-rmdir $(INCLUDE_DIR)

## Taf!
