dnl ##################################################################
dnl The macro AC_CHECK_FUNC_PROTO
dnl comes from aclocal.m4 in the sample code associated
dnl with "UNIX Network Programming: Volume 1", Second Edition,
dnl by W. Richard Stevens.  That sample code is available via
dnl ftp://ftp.kohala.com/pub/rstevens/unpv12e.tar.gz.
dnl
dnl Version $Id: ac_check_func_proto.m4,v 1.2 2004/08/24 12:36:52 root Exp $
dnl
dnl AC_CHECK_FUNC_PROTO(function, header)
dnl Check for a function prototype in a given system header file.
dnl Runs the preprocessor on the system header file then checks to see if 'function'
dnl (treated as an egrep regexp) matches the output of the preprocessor.
dnl If found, defines C preprocessor macro HAVE_function_PROTO (with function name made uppercase).
dnl Sets the cache variable ac_cv_have_function_proto to 'yes' or 'no'.
dnl
dnl To improve the appearance of the resulting config.h.in file, for each function 'foo' you call us with,
dnl you'll probably want to add something like the following to the file that calls this macro:
dnl  AH_TEMPLATE([HAVE_FOO_PROTO], [Define the following if foo() is declared])
dnl
AC_DEFUN([AC_CHECK_FUNC_PROTO],
	[AC_CACHE_CHECK(for $1 function prototype in $2, ac_cv_have_$1_proto,
		AC_EGREP_HEADER($1, $2,
			ac_cv_have_$1_proto=yes,
			ac_cv_have_$1_proto=no))
	if test $ac_cv_have_$1_proto = yes ; then
		ac_tr_func=HAVE_`echo $1 | tr 'abcdefghijklmnopqrstuvwxyz' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'`_PROTO
		AC_DEFINE_UNQUOTED($ac_tr_func)
	fi
])
