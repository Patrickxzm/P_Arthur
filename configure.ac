#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([P_Arthur], [1.0])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_AWK
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_PROG_MKDIR_P
AC_PROG_LEX
# AC_PROG_LIBTOOL
AC_LANG([C++])

## Checks for utility.
AC_CHECK_VERSION([$LEX], [2.5.35],
[],
[
	AC_MSG_ERROR("$LEX version 2.5.35 or higher is required")
])
AC_CHECK_VERSION([xml2-config], [2.7.8],
[],
[
	AC_MSG_ERROR("libxml2(xml2-config) version 2.7.8 or higher is required")
])

## Init LDFLAGS and CPPFLAGS
for mylib in /usr/lib /usr/lib64 /usr/local/lib /usr/local/lib64 $HOME/lib $HOME/local/lib; do
	if test -d $mylib; then
		LDFLAGS="-L$mylib -Wl,-rpath,$mylib $LDFLAGS"
	fi
done
CPPFLAGS="-I$HOME/local/include -I$HOME/include $CPPFLAGS"


## Checks for user's command line arguments
AC_ARG_ENABLE(debug,
  [  --enable-debug   Produce code more debugable with "-Wall -g" option],
  [  CFLAGS="-Wall -g"
     CXXFLAGS="-Wall -g"
  ],
  [
     CFLAGS="-Wall $CFLAGS"
     CXXFLAGS="-Wall $CXXFLAGS"
  ])


## Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h sys/ioctl.h sys/socket.h sys/time.h sys/timeb.h iconv.h])
AC_CHECK_HEADERS([openssl/md5.h],
[],
[
	AC_MSG_ERROR([openssl/md5.h not found])
])

# MYSQL_API_LOCATION
# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_STRUCT_TIMEZONE
AC_C_VOLATILE

# Checks for functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_FORK
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MKTIME
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([alarm atexit bzero ftime ftruncate getcwd gethostbyname isascii memmove memset mkdir mkfifo munmap putenv select socket strcasecmp strchr strdup strerror strncasecmp strndup strstr tzset])
AC_CHECK_FUNCS([strnstr getline strcasestr intoa])

## Checks for libraries.
AC_CHECK_LIB([iconv], [iconv_open],
[],
[
	AC_MSG_WARN([iconv library not found])
])
AC_CHECK_FUNCS([iconv iconv_open iconv_close], 
[],
[
	AC_MSG_ERROR([iconv,iconv_open,iconv_close not found])
])
AC_CHECK_LIB([z], [compress],
[],
[
	AC_MSG_ERROR([z library not found])
])

## Looking for libraries
AC_SEARCH_INCDIR([mysql])
AC_SEARCH_INCDIR([unp/lib], [],
[
	AC_MSG_ERROR("unp/lib for unix network programming package is required")
])
AC_SEARCH_DLIBDIR([mysql])
AC_SEARCH_INCDIR([libxml2])
AC_SEARCH_INCDIR([mysql++])

AC_CHECK_HEADERS([libxml/xmlsave.h],
[],
[
	AC_MSG_ERROR([libxml/xmlsave.h not found])
])

AC_CHECK_LIB([unp], [Udp_client],
[],
[
	AC_MSG_ERROR([unix network programming library not found])
])
AC_CHECK_LIB([mysqlclient], [mysql_real_connect],
[],
[
	AC_MSG_ERROR([mysqlclient library not found])
])
# FIXME: Replace `main' with a function in `-lmysqlpp':
AX_CXX_CHECK_LIB([mysqlpp], [mysqlpp::Connection::connected() const],
[],
[
	AC_MSG_ERROR([mysqlpp library not found])
])
# FIXME: Replace `main' with a function in `-lxml2':
AC_CHECK_LIB([xml2], [xmlXPathNewContext],
[], 
[
	AC_MSG_ERROR([xml2 library not found])
])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
