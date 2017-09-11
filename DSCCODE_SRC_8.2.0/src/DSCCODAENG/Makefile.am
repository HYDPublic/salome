include $(top_srcdir)/adm_local/make_common_starter.am

salomelib_LTLIBRARIES = libDSCCODAEngine.la
libDSCCODAEngine_la_SOURCES      = DSCCODAENG.cxx
nodist_libDSCCODAEngine_la_SOURCES = 
libDSCCODAEngine_la_CXXFLAGS = -I$(top_builddir)/idl  $(KERNEL_INCLUDES) 
libDSCCODAEngine_la_LIBADD   = -L$(top_builddir)/idl -lDSCCODE $(KERNEL_LIBS)
salomeinclude_HEADERS = DSCCODAENG.hxx

