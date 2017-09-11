include $(top_srcdir)/adm_local/make_common_starter.am

salomelib_LTLIBRARIES = libDSCCODBEngine.la
salomeinclude_HEADERS = DSCCODBENG.hxx
libDSCCODBEngine_la_SOURCES      = DSCCODBENG.cxx
nodist_libDSCCODBEngine_la_SOURCES = 
libDSCCODBEngine_la_CXXFLAGS     = -I$(top_builddir)/idl  $(KERNEL_INCLUDES)
libDSCCODBEngine_la_LIBADD     = -L$(top_builddir)/idl -lDSCCODE $(KERNEL_LIBS)

