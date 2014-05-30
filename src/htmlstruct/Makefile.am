htmlstructdir = $(top_srcdir)/src/htmlstruct
htmlstructhdir = $(includedir)/pa/htmlstruct

bin_PROGRAMS += ref_t htmllexic_t 

htmlstructh_HEADERS = $(htmlstructdir)/attr.h 
htmlstructh_HEADERS += $(htmlstructdir)/text.h 
htmlstructh_HEADERS += $(htmlstructdir)/tag.h 
htmlstructh_HEADERS += $(htmlstructdir)/comment.h 
htmlstructh_HEADERS += $(htmlstructdir)/encapsuled.h 
htmlstructh_HEADERS += $(htmlstructdir)/htmlitem.h 
htmlstructh_HEADERS += $(htmlstructdir)/htmllexic.h 
htmlstructh_HEADERS += $(htmlstructdir)/ref.h 
htmlstructh_HEADERS += $(htmlstructdir)/htmlref.h 
htmlstructh_HEADERS += $(htmlstructdir)/refresh.h

libpa_a_SOURCES += $(htmlstructh_HEADERS)
libpa_a_SOURCES += $(htmlstructdir)/attr.cpp 
libpa_a_SOURCES += $(htmlstructdir)/text.cpp 
libpa_a_SOURCES += $(htmlstructdir)/tag.cpp 
libpa_a_SOURCES += $(htmlstructdir)/comment.cpp 
libpa_a_SOURCES += $(htmlstructdir)/encapsuled.cpp 
libpa_a_SOURCES += $(htmlstructdir)/htmlitem.cpp 
libpa_a_SOURCES += $(htmlstructdir)/htmllexic.cpp 
libpa_a_SOURCES += $(htmlstructdir)/ref.cpp 
libpa_a_SOURCES += $(htmlstructdir)/htmlref.cpp 
libpa_a_SOURCES += $(htmlstructdir)/refresh.cpp 

htmllexic_t_SOURCES = $(htmlstructdir)/htmllexic_t.cpp 
ref_t_SOURCES = $(htmlstructdir)/ref_t.cpp

