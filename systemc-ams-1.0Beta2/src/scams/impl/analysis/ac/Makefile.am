
include $(top_srcdir)/config/Make-rules

noinst_LIBRARIES = libimpl_ac.a

H_FILES = 
    

NO_H_FILES = \
    sca_ac_domain_db.h \
    sca_ac_domain_entity.h \
    sca_ac_domain_globals.h \
    sca_ac_domain_eq.h \
    sca_solve_ac_linear.h \
    sca_ac_domain_solver.h
    

noinst_HEADERS = $(H_FILES)

CXX_FILES = \
	sca_ac_domain_db.cpp \
	sca_ac_domain_entity.cpp \
	sca_ac_domain_eq.cpp \
	sca_solve_ac_linear.cpp \
	sca_ac_domain_solver.cpp \
	sca_ac_domain_globals.cpp \
	sca_ac_start.cpp \
	sca_ac_noise_start.cpp \
	sca_ac_functions.cpp

NO_CXX_FILES =

libimpl_ac_a_SOURCES = $(H_FILES) $(NO_H_FILES) $(CXX_FILES) 

INCLUDE_DIR = $(prefix)/include/scams/impl/analysis/ac

install-data-local:
#	@$(NORMAL_INSTALL)
#	$(mkinstalldirs) $(INCLUDE_DIR)
#	for file in $(noinst_HEADERS); do \
#	  $(INSTALL_DATA) $(srcdir)/$$file $(INCLUDE_DIR)/$$file; \
#	done
	$(AR_TOOL) $(top_builddir)/src/$(MAIN_LIBRARY) \
	  $(libimpl_ac_a_OBJECTS)

#uninstall-local:
#	@$(NORMAL_UNINSTALL)
#	for file in $(noinst_HEADERS); do \
#	  rm -f $(INCLUDE_DIR)/$$file; \
#	done
#	-rmdir $(INCLUDE_DIR)

## Taf!
