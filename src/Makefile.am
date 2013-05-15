library_includedir=$(includedir)/rnc-1.0
library_include_HEADERS = ../include/rnc ../include/rnc-lib/matrix.h ../include/rnc-lib/fq.h

lib_LTLIBRARIES = librnc-1.0.la
librnc_1_0_la_SOURCES = matrix.cpp rnc-lib/matrix.h \
			fq.cpp rnc-lib/fq.h \
			qsize mkstr auto_arr_ptr
librnc_1_0_la_CPPFLAGS=$(GLIB_CFLAGS) -I$(top_srcdir)/include
librnc_1_0_la_LDFLAGS =$(GTHREAD_LIBS) -version-info 1:0:0 -shared
