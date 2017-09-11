include $(top_srcdir)/adm_local/make_common_starter.am

salomelib_LTLIBRARIES = libDSCCODDEngine.la
libDSCCODDEngine_la_SOURCES      = DSCCODDENG.cxx
nodist_libDSCCODDEngine_la_SOURCES = 
libDSCCODDEngine_la_CXXFLAGS = -I$(top_builddir)/idl  $(KERNEL_INCLUDES) 
libDSCCODDEngine_la_LIBADD   = -L$(top_builddir)/idl -lDSCCODE $(KERNEL_LIBS)
salomeinclude_HEADERS = DSCCODDENG.hxx

