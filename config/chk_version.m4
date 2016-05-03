AC_DEFUN([AC_CHECK_VERSION],
[
	qualify="yes"
	VERSION=`$1 --version|sed "s/[[^0-9]]*\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\1\.\2.\3/"`
	AC_MSG_CHECKING([for the version of program $1 ])
	for i in 1 2 3; do
 		select_string="s/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\)/\\$i/"
	        V=`echo $VERSION|sed $select_string`
                v=`echo $2|sed $select_string`
                if test $V -gt $v; then
                    qualify="yes"
                    break
                fi
                if test $V -lt $v; then
		    qualify="no"
		    break
                fi
	done
	AC_MSG_RESULT([$VERSION])
	if test "x$qualify" = xyes
	then
		m4_default([$3], :)
	else
		m4_default([$4], :)
	fi
])
