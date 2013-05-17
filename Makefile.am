ACLOCAL_AMFLAGS=-I m4
SUBDIRS=src
if WITH_TESTS
SUBDIRS+=test
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = rnc-1.0.pc

# DOXYGEN SUPPORT
include $(top_srcdir)/doxygen.am

# ensure the distribution of the doxygen configuration file
EXTRA_DIST = doc/doxygen.cfg
