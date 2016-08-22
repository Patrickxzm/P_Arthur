## Makefile.am 
lib_LIBRARIES = libpa.a
libpa_a_SOURCES =

utilh_HEADERS = util/bitmap.h util/shadow.h util/multi_hash.h util/arg.h util/cgi.h\
   util/cutem.h util/xmlGet.h util/excep.h util/util.h util/xmlfile.h \
   util/cutil.h util/redirect.h util/charset.h util/rmdirtree.h \
   util/pat_types.h util/memory.hpp util/sort.hpp util/index_queue.hpp
utilhdir = $(includedir)/pa/util
util_src = util/bitmap.cpp util/shadow.cpp util/multi_hash.cpp util/cutil.cpp \
   util/arg.cpp util/cgi.cpp util/cutem.cpp util/xmlGet.cpp util/util.cpp \
   util/xmlfile.cpp util/redirect.cpp util/charset.cpp util/rmdirtree.cpp
libpa_a_SOURCES += $(utilh_HEADERS) $(util_src)

urlh_HEADERS = url/url.h url/ext.h url/hostfilter.h url/host.h url/uri.h 
urlhdir = $(includedir)/pa/url
url_src = url/url.cpp url/ext.cpp url/host.cpp url/uri.cc
url/uri.cc	:	url/uri.l 
	$(LEX) -+ -PURI -o$@ -d $^
libpa_a_SOURCES += $(urlh_HEADERS) $(url_src)

commuh_HEADERS = commu/TcpClient.h commu/TcpServer.h commu/TcpBuf.hpp commu/http_bits.h\
   commu/http_request.h commu/http_reply.h commu/http.h commu/cookie.h \
   commu/UltraServer.h commu/TcpStream.hpp commu/tw_raw.h commu/filterMall.h \
   commu/mcast.h commu/mcast_session.h
commuhdir = $(includedir)/pa/commu
commu_src = commu/TcpClient.cpp commu/TcpServer.cpp commu/TcpBuf.cpp  \
   commu/http_bits.cpp commu/http_request.cpp commu/http_reply.cpp commu/http.cpp \
   commu/cookie.cpp commu/UltraServer.cpp commu/tw_raw.cpp commu/filterMall.cpp \
   commu/mcast.cpp commu/mcast_session.cpp 
libpa_a_SOURCES += $(commuh_HEADERS) $(commu_src)

htmlstructh_HEADERS = htmlstruct/attr.h htmlstruct/text.h htmlstruct/tag.h \
   htmlstruct/comment.h htmlstruct/encapsuled.h htmlstruct/htmlitem.h \
   htmlstruct/htmllexic.h htmlstruct/ref.h htmlstruct/htmlref.h htmlstruct/refresh.h
htmlstructhdir = $(includedir)/pa/htmlstruct
htmlstruct_src = htmlstruct/attr.cpp htmlstruct/text.cpp htmlstruct/tag.cpp \
   htmlstruct/comment.cpp htmlstruct/encapsuled.cpp htmlstruct/htmlitem.cpp \
   htmlstruct/htmllexic.cpp htmlstruct/ref.cpp htmlstruct/htmlref.cpp \
   htmlstruct/refresh.cpp 
libpa_a_SOURCES += $(htmlstructh_HEADERS) $(htmlstruct_src)

crawlerh_HEADERS = crawler/common.h crawler/task_stream.h \
   crawler/task.h crawler/shadow_chain.h crawler/page_status.h crawler/mosquito.h \
   crawler/robots.h crawler/host4import.h
crawlerhdir = $(includedir)/pa/crawler
crawler_src = crawler/shadow_chain.cpp crawler/common.cpp crawler/mosquito.cpp \
   crawler/robots.cpp crawler/task.cpp crawler/task_stream.cpp crawler/host4import.cpp
libpa_a_SOURCES += $(crawlerh_HEADERS) $(crawler_src)

SUBDIRS = . util url commu htmlstruct crawler script
