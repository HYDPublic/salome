include $(top_srcdir)/adm_local/make_common_starter.am

AM_CFLAGS=$(KERNEL_INCLUDES) -fexceptions
AM_FFLAGS=$(KERNEL_INCLUDES) -fexceptions
AM_CXXFLAGS = -I$(top_builddir)/idl  $(KERNEL_INCLUDES) 
AM_LDFLAGS  = -L$(top_builddir)/idl -lDSCCODE $(FLIBS) $(KERNEL_LIBS)

salomelib_LTLIBRARIES = libINTERPIEngine.la libINTERPJEngine.la libINTERPKEngine.la

libINTERPIEngine_la_SOURCES      = INTERPI.cxx interi.f inter.f
libINTERPJEngine_la_SOURCES      = INTERPI.cxx interi.f inter.f
libINTERPKEngine_la_SOURCES      = INTERPK.cxx

salomeinclude_HEADERS = INTERPI.hxx INTERPK.hxx

