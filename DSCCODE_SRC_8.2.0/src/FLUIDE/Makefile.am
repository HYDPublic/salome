include $(top_srcdir)/adm_local/make_common_starter.am

AM_CFLAGS=$(KERNEL_INCLUDES) -fexceptions

salomelib_LTLIBRARIES = libFLUIDEEngine.la
libFLUIDEEngine_la_SOURCES      = FLUIDE.cxx fluide.f 
nodist_libFLUIDEEngine_la_SOURCES = 
libFLUIDEEngine_la_CXXFLAGS = -I$(top_builddir)/idl  $(KERNEL_INCLUDES) 
libFLUIDEEngine_la_FFLAGS = $(KERNEL_INCLUDES) -fexceptions
libFLUIDEEngine_la_LIBADD   = -L$(top_builddir)/idl -lDSCCODE $(FLIBS) $(KERNEL_LIBS)
salomeinclude_HEADERS = FLUIDE.hxx

