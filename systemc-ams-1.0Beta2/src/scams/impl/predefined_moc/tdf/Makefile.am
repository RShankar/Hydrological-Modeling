
include $(top_srcdir)/config/Make-rules

noinst_LIBRARIES = libimpl_tdf.a

H_FILES = \
    sca_tdf_port_impl.h \
    sca_tdf_port_attributes.h \
    sca_tdf_signal_impl.h \
    sca_tdf_signal_impl_base.h \
    sca_tdf_trace_variable_base.h

NO_H_FILES = \
    sca_tdf_ct_ltf_nd_proxy.h \
    sca_tdf_view.h \
    sca_tdf_ct_ltf_zp_proxy.h \
    sca_tdf_ct_vector_ss_proxy.h \
    sca_ct_delay_buffer.h

noinst_HEADERS = $(H_FILES)

CXX_FILES = \
	sca_tdf_ct_ltf_nd_proxy.cpp \
	sca_tdf_ltf_nd.cpp \
	sca_tdf_module.cpp \
	sca_tdf_port_attributes.cpp \
	sca_tdf_signal_impl_base.cpp \
	sca_tdf_view.cpp \
	sca_tdf_trace_variable_base.cpp \
	sca_tdf_ct_ltf_zp_proxy.cpp \
	sca_tdf_ltf_zp.cpp \
	sca_tdf_ct_vector_ss_proxy.cpp \
	sca_tdf_ss.cpp \
	sca_ct_delay_buffer.cpp
	

NO_CXX_FILES =

libimpl_tdf_a_SOURCES = $(H_FILES) $(NO_H_FILES) $(CXX_FILES) 

INCLUDE_DIR = $(prefix)/include/scams/impl/predefined_moc/tdf

install-data-local:
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(INCLUDE_DIR)
	for file in $(noinst_HEADERS); do \
	  $(INSTALL_DATA) $(srcdir)/$$file $(INCLUDE_DIR)/$$file; \
	done
	$(AR_TOOL) $(top_builddir)/src/$(MAIN_LIBRARY) \
	  $(libimpl_tdf_a_OBJECTS)

uninstall-local:
	@$(NORMAL_UNINSTALL)
	for file in $(noinst_HEADERS); do \
	  rm -f $(INCLUDE_DIR)/$$file; \
	done
	-rmdir $(INCLUDE_DIR)

## Taf!
