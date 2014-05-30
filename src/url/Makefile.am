urldir = $(top_srcdir)/src/url
urlhdir = $(includedir)/pa/url

noinst_PROGRAMS +=  host_t
bin_PROGRAMS += url_t

urlh_HEADERS = $(urldir)/url.h 
urlh_HEADERS += $(urldir)/ext.h 
#urlh_HEADERS += $(urldir)/hostfilter.h 
urlh_HEADERS += $(urldir)/host.h 
urlh_HEADERS += $(urldir)/uri.h 
#urlh_HEADERS += $(urldir)/xhostfilter.h 

libpa_a_SOURCES += $(urlh_HEADERS)
libpa_a_SOURCES += $(urldir)/url.cpp 
libpa_a_SOURCES += $(urldir)/ext.cpp 
#libpa_a_SOURCES += $(urldir)/hostfilter.cpp 
libpa_a_SOURCES += $(urldir)/host.cpp 
libpa_a_SOURCES += uri.cc
#libpa_a_SOURCES += $(urldir)/xhostfilter.cpp 

url_t_SOURCES = $(urldir)/url_t.cpp
host_t_SOURCES = $(urldir)/host_t.cpp

uri.cc	:	$(urldir)/uri.l 
	$(LEX) -+ -PURI -o$@ -d $^
