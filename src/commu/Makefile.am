#/******************************************************************
# *
# *	public lib , Tcp utilities; 
# *  Author :	Patrick Arthur, CNDS  08/02/2002 
# *
# *****************************************************************/
AM_CPPFLAGS = -I$(top_srcdir)/src
LDADD = ../libpa.a
bin_PROGRAMS = httpf httpf1 tw_raw_t collectTWRaw mcast_t xpathRawGet
check_PROGRAMS	= TcpServer_t TcpClient_t UltraServer_t cookie_t 
noinst_PROGRAMS = mytest

httpf_SOURCES = httpf.cpp
cookie_t_SOURCES = cookie_t.cpp
tw_raw_t_SOURCES = tw_raw_t.cpp
httpf1_SOURCES = httpf1.cpp
collectTWRaw_SOURCES = collectTWRaw.cpp
TcpServer_t_SOURCES = TcpServer_t.cpp
TcpClient_t_SOURCES = TcpClient_t.cpp
UltraServer_t_SOURCES = UltraServer_t.cpp
mcast_t_SOURCES = mcast_t.cpp
mytest_SOURCES = mytest.cpp
xpathRawGet_SOURCES = xpathRawGet.cpp

