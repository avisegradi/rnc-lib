bin_PROGRAMS=rnc-test-fq
rnc_test_fq_SOURCES=test-fq.cpp
rnc_test_fq_CPPFLAGS=$(GLIB_CFLAGS) -I$(top_srcdir)/include -I$(top_srcdir)/test/common
rnc_test_fq_LDFLAGS=$(GTHREAD_LIBS) -L$(top_builddir)/src/.libs -lrnc-1.0 -L$(top_builddir)/test/common/.libs -lrnc-1.0 -ltest
