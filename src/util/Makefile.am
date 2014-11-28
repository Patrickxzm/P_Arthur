utildir = $(top_srcdir)/src/util
utilhdir = $(includedir)/pa/util

noinst_PROGRAMS += rmdirtree_t mysqlpp_test iconv_t util_t
bin_PROGRAMS += xpathGet shadow_t
bin_SCRIPTS += util/select.awk

utilh_HEADERS = util/bitmap.h
utilh_HEADERS += util/shadow.h
utilh_HEADERS += util/multi_hash.h
utilh_HEADERS += util/arg.h
utilh_HEADERS += util/cgi.h
utilh_HEADERS += util/cutem.h
utilh_HEADERS += util/xmlGet.h
utilh_HEADERS += util/excep.h
utilh_HEADERS += util/util.h
utilh_HEADERS += util/xmlfile.h
utilh_HEADERS += util/cutil.h
utilh_HEADERS += util/redirect.h
utilh_HEADERS += util/charset.h
utilh_HEADERS += util/rmdirtree.h
utilh_HEADERS += util/pat_types.h
utilh_HEADERS += util/memory.hpp
utilh_HEADERS += util/sort.hpp
utilh_HEADERS += util/index_queue.hpp

libpa_a_SOURCES += $(utilh_HEADERS)
libpa_a_SOURCES += util/bitmap.cpp
libpa_a_SOURCES += util/shadow.cpp
libpa_a_SOURCES += util/multi_hash.cpp
libpa_a_SOURCES += util/cutil.cpp
libpa_a_SOURCES += util/arg.cpp
libpa_a_SOURCES += util/cgi.cpp
libpa_a_SOURCES += util/cutem.cpp
libpa_a_SOURCES += util/xmlGet.cpp
libpa_a_SOURCES += util/util.cpp
libpa_a_SOURCES += util/xmlfile.cpp
libpa_a_SOURCES += util/redirect.cpp
libpa_a_SOURCES += util/charset.cpp
libpa_a_SOURCES += util/rmdirtree.cpp

shadow_t_SOURCES = util/shadow_t.cpp
mysqlpp_test_SOURCES = util/mysqlpp_test.cpp
rmdirtree_t_SOURCES = util/rmdirtree_t.cpp
xpathGet_SOURCES = util/xpathGet.cpp
util_t_SOURCES = util/util_t.cpp
iconv_t_SOURCES = util/iconv_t.cpp
