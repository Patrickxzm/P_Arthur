#/******************************************************************
# *
# *	public lib , Tcp utilities; 
# *  Author :	Patrick Arthur, CNDS  08/02/2002 
# *
# *****************************************************************/
commudir = $(top_srcdir)/src/commu
commuhdir = $(includedir)/pa/commu

bin_PROGRAMS += httpf httpf1 tw_raw_t collectTWRaw mcast_t
check_PROGRAMS	+= TcpServer_t TcpClient_t UltraServer_t cookie_t 
noinst_PROGRAMS += mytest

commuh_HEADERS = $(commudir)/TcpClient.h 
commuh_HEADERS += $(commudir)/TcpServer.h 
commuh_HEADERS += $(commudir)/TcpBuf.hpp 
commuh_HEADERS += $(commudir)/http_bits.h 
commuh_HEADERS += $(commudir)/http_request.h 
commuh_HEADERS += $(commudir)/http_reply.h 
commuh_HEADERS += $(commudir)/http.h 
commuh_HEADERS += $(commudir)/cookie.h 
commuh_HEADERS += $(commudir)/UltraServer.h 
commuh_HEADERS += $(commudir)/TcpStream.hpp 
commuh_HEADERS += $(commudir)/tw_raw.h
commuh_HEADERS += $(commudir)/filterMall.h
commuh_HEADERS += $(commudir)/mcast.h
commuh_HEADERS += $(commudir)/mcast_session.h

libpa_a_SOURCES += $(commuh_HEADERS)
libpa_a_SOURCES	+= $(commudir)/TcpClient.cpp 
libpa_a_SOURCES += $(commudir)/TcpServer.cpp 
libpa_a_SOURCES += $(commudir)/TcpBuf.cpp 
libpa_a_SOURCES += $(commudir)/http_bits.cpp 
libpa_a_SOURCES += $(commudir)/http_request.cpp 
libpa_a_SOURCES += $(commudir)/http_reply.cpp 
libpa_a_SOURCES += $(commudir)/http.cpp 
libpa_a_SOURCES += $(commudir)/cookie.cpp 
libpa_a_SOURCES += $(commudir)/UltraServer.cpp
libpa_a_SOURCES += $(commudir)/tw_raw.cpp 
libpa_a_SOURCES += $(commudir)/filterMall.cpp 
libpa_a_SOURCES += $(commudir)/mcast.cpp 
libpa_a_SOURCES += $(commudir)/mcast_session.cpp 

httpf_SOURCES = $(commudir)/httpf.cpp
cookie_t_SOURCES = $(commudir)/cookie_t.cpp
tw_raw_t_SOURCES = $(commudir)/tw_raw_t.cpp
httpf1_SOURCES = $(commudir)/httpf1.cpp
collectTWRaw_SOURCES = $(commudir)/collectTWRaw.cpp
TcpServer_t_SOURCES = $(commudir)/TcpServer_t.cpp
TcpClient_t_SOURCES = $(commudir)/TcpClient_t.cpp
UltraServer_t_SOURCES = $(commudir)/UltraServer_t.cpp
mcast_t_SOURCES = $(commudir)/mcast_t.cpp
mytest_SOURCES = $(commudir)/mytest.cpp

