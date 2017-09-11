include $(top_srcdir)/adm_local/make_common_starter.am

salomelib_LTLIBRARIES = libDSCCODCEngine.la
salomeinclude_HEADERS = DSCCODCENG.hxx
libDSCCODCEngine_la_SOURCES      = DSCCODCENG.cxx
nodist_libDSCCODCEngine_la_SOURCES = 
libDSCCODCEngine_la_CXXFLAGS     = -I$(top_builddir)/idl  $(KERNEL_INCLUDES) 
libDSCCODCEngine_la_LIBADD     = -L$(top_builddir)/idl -lDSCCODE $(KERNEL_LIBS)

