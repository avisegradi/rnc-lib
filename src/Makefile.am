library_includedir=$(includedir)/rnc-1.0
library_include_HEADERS = ../include/rnc
library_subdir_includedir=$(includedir)/rnc-1.0/rnc-lib
library_subdir_include_HEADERS = ../include/rnc-lib/matrix.h ../include/rnc-lib/fq.h


lib_LTLIBRARIES = librnc-1.0.la
librnc_1_0_la_SOURCES = matrix.cpp $(top_srcdir)/include/rnc-lib/matrix.h \
			fq.cpp $(top_srcdir)/include/rnc-lib/fq.h \
			$(top_srcdir)/include/rnc \
			$(top_srcdir)/include/mkstr $(top_srcdir)/include/auto_arr_ptr \
			pow_table_8 pow_table_16
librnc_1_0_la_CPPFLAGS=$(GLIB_CFLAGS) -I$(top_srcdir)/include
librnc_1_0_la_LDFLAGS =$(GTHREAD_LIBS) -version-info 1:0:0 -shared
