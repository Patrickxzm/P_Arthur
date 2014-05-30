utildir = $(top_srcdir)/src/util
utilhdir = $(includedir)/pa/util

noinst_PROGRAMS += rmdirtree_t mysqlpp_test iconv_t util_t
bin_PROGRAMS += xpathGet shadow_t
bin_SCRIPTS += $(utildir)/select.awk

utilh_HEADERS = $(utildir)/bitmap.h 
utilh_HEADERS += $(utildir)/shadow.h 
utilh_HEADERS += $(utildir)/multi_hash.h 
utilh_HEADERS += $(utildir)/arg.h 
utilh_HEADERS += $(utildir)/cgi.h 
utilh_HEADERS += $(utildir)/cutem.h 
utilh_HEADERS += $(utildir)/xmlGet.h 
utilh_HEADERS += $(utildir)/excep.h
utilh_HEADERS += $(utildir)/util.h
utilh_HEADERS += $(utildir)/xmlfile.h
utilh_HEADERS += $(utildir)/cutil.h
utilh_HEADERS += $(utildir)/redirect.h
utilh_HEADERS += $(utildir)/charset.h
utilh_HEADERS += $(utildir)/rmdirtree.h
utilh_HEADERS += $(utildir)/pat_types.h
utilh_HEADERS += $(utildir)/memory.hpp
utilh_HEADERS += $(utildir)/sort.hpp
utilh_HEADERS += $(utildir)/index_queue.hpp

libpa_a_SOURCES += $(utilh_HEADERS)
libpa_a_SOURCES += $(utildir)/bitmap.cpp 
libpa_a_SOURCES += $(utildir)/shadow.cpp 
libpa_a_SOURCES += $(utildir)/multi_hash.cpp 
libpa_a_SOURCES += $(utildir)/cutil.cpp 
libpa_a_SOURCES += $(utildir)/arg.cpp 
libpa_a_SOURCES += $(utildir)/cgi.cpp 
libpa_a_SOURCES += $(utildir)/cutem.cpp 
libpa_a_SOURCES += $(utildir)/xmlGet.cpp 
libpa_a_SOURCES += $(utildir)/util.cpp 
libpa_a_SOURCES += $(utildir)/xmlfile.cpp
libpa_a_SOURCES += $(utildir)/redirect.cpp 
libpa_a_SOURCES += $(utildir)/charset.cpp 
libpa_a_SOURCES += $(utildir)/rmdirtree.cpp 

shadow_t_SOURCES = $(utildir)/shadow_t.cpp
mysqlpp_test_SOURCES = $(utildir)/mysqlpp_test.cpp
rmdirtree_t_SOURCES = $(utildir)/rmdirtree_t.cpp
xpathGet_SOURCES = $(utildir)/xpathGet.cpp
util_t_SOURCES = $(utildir)/util_t.cpp
iconv_t_SOURCES = $(utildir)/iconv_t.cpp
