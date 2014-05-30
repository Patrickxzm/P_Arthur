AC_DEFUN([AC_SEARCH_LIBDIR],
[
	AC_MSG_CHECKING([for the location of library dir $1 ])
	BASE_LIB_DIR="$HOME/local/lib $HOME/lib /usr/local/lib64 /usr/local/lib /usr/lib64 /usr/lib"
	LIBDIR_Loc=
	for m in $BASE_LIB_DIR
	do 
		if test -d "$m/$1"
		then
			LDFLAGS="-L$m/$1 $LDFLAGS"
			LIBDIR_Loc=$m
			break
		fi
	done
	if test -z "$LIBDIR_Loc"
	then
		AC_MSG_RESULT([no])
	else	
		AC_MSG_RESULT([$LIBDIR_Loc])
	fi
])

AC_DEFUN([AC_SEARCH_DLIBDIR],
[
	AC_MSG_CHECKING([for the location of dynamic library dir $1 ])
	BASE_LIB_DIR="$HOME/local/lib $HOME/lib /usr/local/lib64 /usr/local/lib /usr/lib64 /usr/lib"
	DLIBDIR_Loc=
	for m in $BASE_LIB_DIR
	do 
		if test -d "$m/$1"
		then
			LDFLAGS="-L$m/$1 -Wl,-rpath,$m/$1 $LDFLAGS"
			DLIBDIR_Loc=$m
			break
		fi
	done
	if test -z "$DLIBDIR_Loc"
	then
		AC_MSG_RESULT([no])
	else	
		AC_MSG_RESULT([$DLIBDIR_Loc])
	fi
])

AC_DEFUN([AC_SEARCH_INCDIR],
[
	AC_MSG_CHECKING([for the location of include dir $1 ])
	BASE_INC_DIR="$HOME/local/include $HOME/include /usr/local/include /usr/include"
	INCDIR_Loc=
	for m in $BASE_INC_DIR
	do
		if test -d "$m/$1"
		then 
			CPPFLAGS="-I$m/$1 $CPPFLAGS"
			INCDIR_Loc=$m
			break
		fi
	done
	if test -z "$INCDIR_Loc"
	then
		AC_MSG_RESULT([no])
		m4_default([$3], :)
	else	
		AC_MSG_RESULT([$INCDIR_Loc])
		m4_default([$2], :)
	fi
])
