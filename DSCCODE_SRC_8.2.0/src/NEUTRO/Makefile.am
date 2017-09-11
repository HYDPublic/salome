include $(top_srcdir)/adm_local/make_common_starter.am

AM_CFLAGS=$(KERNEL_INCLUDES) -fexceptions

salomelib_LTLIBRARIES = libNEUTROEngine.la
libNEUTROEngine_la_SOURCES      = NEUTRO.cxx neutro.f
nodist_libNEUTROEngine_la_SOURCES = 
libNEUTROEngine_la_CXXFLAGS = -I$(top_builddir)/idl  $(KERNEL_INCLUDES) 
libNEUTROEngine_la_FFLAGS = $(KERNEL_INCLUDES)  -fexceptions
libNEUTROEngine_la_LIBADD   = -L$(top_builddir)/idl -lDSCCODE $(FLIBS) $(KERNEL_LIBS)
salomeinclude_HEADERS = NEUTRO.hxx

