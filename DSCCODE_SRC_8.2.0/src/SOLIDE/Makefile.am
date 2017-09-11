include $(top_srcdir)/adm_local/make_common_starter.am

AM_CFLAGS=$(KERNEL_INCLUDES) -fexceptions

salomelib_LTLIBRARIES = libSOLIDEEngine.la
libSOLIDEEngine_la_SOURCES      = SOLIDE.cxx solid.f
nodist_libSOLIDEEngine_la_SOURCES = 
libSOLIDEEngine_la_CXXFLAGS = -I$(top_builddir)/idl  $(KERNEL_INCLUDES) 
libSOLIDEEngine_la_FFLAGS = $(KERNEL_INCLUDES)  -fexceptions
libSOLIDEEngine_la_LIBADD   = -L$(top_builddir)/idl -lDSCCODE $(FLIBS) $(KERNEL_LIBS)
salomeinclude_HEADERS = SOLIDE.hxx

