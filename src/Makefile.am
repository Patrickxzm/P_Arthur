## Makefile.am 
AUTOMAKE_OPTIONS = 
#INCLUDES = -I$(top_srcdir)/src -I$(top_builddir)
noinst_PROGRAMS = 
check_PROGRAMS =
lib_LIBRARIES = libpa.a
bin_PROGRAMS = 
bin_SCRIPTS = $(top_srcdir)/P_Arthur.install
libpa_a_SOURCES =

LDADD = $(top_builddir)/src/libpa.a -lssl -lcrypto -lunp

include util/Makefile.am
include url/Makefile.am
include commu/Makefile.am
include htmlstruct/Makefile.am
include crawler/Makefile.am
