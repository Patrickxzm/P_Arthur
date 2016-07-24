AM_CPPFLAGS = -I$(top_srcdir)/src
LDADD = ../libpa.a
bin_PROGRAMS = host_crawler envhost_t scheduler_t check_host raw_filter task2url \
  outlinks mosquito.parser_t
bin_SCRIPTS = insert_fail_candidate.sed insert_OK_candidate.sed update_inlinks.sed \
  reset_host.awk guard.sh outlinks.11272010.trans.sed
check_PROGRAMS = raw2link index_link link2list 


host_crawler_SOURCES = host_crawler.cpp
envhost_t_SOURCES = envhost_t.cpp envhost.cpp envhost.h scheduler.cpp scheduler.h
scheduler_t_SOURCES = scheduler_t.cpp scheduler.cpp scheduler.h envhost.cpp envhost.h
check_host_SOURCES = check_host.cpp
raw_filter_SOURCES = raw_filter.cpp
raw2link_SOURCES = raw2link.cpp links.cpp links.h
index_link_SOURCES = index_link.cpp links.cpp links.h
link2list_SOURCES = link2list.cpp links.h links.cpp
task2url_SOURCES = task2url.cpp
outlinks_SOURCES = outlinks.cpp
mosquito_parser_t_SOURCES = mosquito.parser_t.cpp
