SUBDIRS=common original test-fq test-matr

TEST_CPP_FLAGS=-I$(abs_top_srcdir)/test/common -W -Wall --pedantic @TEST_ADD_CPP_FLAGS@
TEST_LD_FLAGS=-L$(abs_top_builddir)/test/common/.libs -ltest @TEST_ADD_LD_FLAGS@

export TEST_CPP_FLAGS TEST_LD_FLAGS
