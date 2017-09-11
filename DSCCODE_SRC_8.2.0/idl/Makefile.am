include $(top_srcdir)/adm_local/make_common_starter.am

#################################################################

BUILT_SOURCES = DSCCODESK.cc DSCCODE_idl.py
IDL_FILES=DSCCODE.idl

#################################################################

salomelib_LTLIBRARIES = libDSCCODE.la
salomeidl_DATA = $(IDL_FILES)
salomepython_DATA = DSCCODE_idl.py 
libDSCCODE_la_SOURCES      = 
nodist_libDSCCODE_la_SOURCES = DSCCODESK.cc
libDSCCODE_la_CXXFLAGS     = -I.  $(KERNEL_INCLUDES)
libDSCCODE_la_LIBADD     = $(KERNEL_LIBS)


#################################################################
CLEANFILES = *.hh *SK.cc *.py 

clean-local:
	rm -rf DSCCODE DSCCODE__POA

install-data-local:
	${mkinstalldirs} $(DESTDIR)$(salomepythondir)
	cp -R DSCCODE DSCCODE__POA $(DESTDIR)$(salomepythondir)

uninstall-local:
	rm -rf $(DESTDIR)$(salomepythondir)/DSCCODE
	rm -rf $(DESTDIR)$(salomepythondir)/DSCCODE__POA
#################################################################

%SK.cc %.hh : %.idl
	$(OMNIORB_IDL) -bcxx $(OMNIORB_IDLCXXFLAGS) $(IDL_INCLUDES) $<

%_idl.py : %.idl
	$(OMNIORB_IDL) $(OMNIORB_IDLPYFLAGS) $(IDL_INCLUDES) $<

# we use cpp to generate dependencies between idl files.
# option x c tells the preprocessor to consider idl as a c file.
# if an idl is modified, all idl dependencies are rebuilt

.depidl: $(IDL_FILES)
	echo "" > $@
	for dep in $^ dummy; do \
	  if [ $$dep != "dummy" ]; then \
	    echo Building dependencies for $$dep; \
	    $(CPP) $(C_DEPEND_FLAG) -x c -I$(srcdir) $(IDL_INCLUDES) $$dep 2>/dev/null | \
	    sed 's/\.o/\SK.cc/' >>$@; \
	  fi; \
	done ;

-include .depidl

