
include $(top_srcdir)/config/Make-rules

noinst_LIBRARIES = libeln.a

H_FILES =
	
		
NO_H_FILES = \
    sca_eln_view.h

noinst_HEADERS = $(H_FILES)

CXX_FILES = \
	sca_eln_c.cpp \
	sca_eln_cccs.cpp \
	sca_eln_ccvs.cpp \
	sca_eln_gyrator.cpp \
	sca_eln_ideal_transformer.cpp \
	sca_eln_isource.cpp \
	sca_eln_l.cpp \
	sca_eln_module.cpp \
	sca_eln_node_ref.cpp \
	sca_eln_node.cpp \
	sca_eln_nullor.cpp \
	sca_eln_r.cpp \
	sca_eln_sc_c.cpp \
	sca_eln_sc_isink.cpp \
	sca_eln_sc_isource.cpp \
	sca_eln_sc_l.cpp \
	sca_eln_sc_r.cpp \
	sca_eln_sc_rswitch.cpp \
	sca_eln_sc_vsink.cpp \
	sca_eln_sc_vsource.cpp \
	sca_eln_tdf_c.cpp \
	sca_eln_tdf_isink.cpp \
	sca_eln_tdf_isource.cpp \
	sca_eln_tdf_l.cpp \
	sca_eln_tdf_r.cpp \
	sca_eln_tdf_rswitch.cpp \
	sca_eln_tdf_vsink.cpp \
	sca_eln_tdf_vsource.cpp \
	sca_eln_vsource.cpp \
	sca_eln_terminal.cpp \
	sca_eln_vccs.cpp \
	sca_eln_vcvs.cpp \
	sca_eln_view.cpp \
	sca_eln_vsource.cpp \
	sca_eln_transmission_line.cpp

NO_CXX_FILES =

libeln_a_SOURCES = $(H_FILES) $(NO_H_FILES) $(CXX_FILES) 

INCLUDE_DIR = $(prefix)/include/scams/impl/predefined_moc/eln

install-data-local:
#	@$(NORMAL_INSTALL)
#	$(mkinstalldirs) $(INCLUDE_DIR)
#	for file in $(noinst_HEADERS); do \
#	  $(INSTALL_DATA) $(srcdir)/$$file $(INCLUDE_DIR)/$$file; \
#	done
	$(AR_TOOL) $(top_builddir)/src/$(MAIN_LIBRARY) \
	  $(libeln_a_OBJECTS)

#uninstall-local:
#	@$(NORMAL_UNINSTALL)
#	for file in $(noinst_HEADERS); do \
#	  rm -f $(INCLUDE_DIR)/$$file; \
#	done
#	-rmdir $(INCLUDE_DIR)

## Taf!
