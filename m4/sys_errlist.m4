dnl ##################################################################
dnl The macro AC_SYS_NERR
dnl is based on code from configure.in distributed with LPRng-2.0.8.
dnl See the COPYING file for details.
dnl
dnl Version $Id: sys_errlist.m4,v 1.3 2004/08/24 15:13:55 root Exp $
dnl 
dnl Usage:
dnl  AC_SYS_NERR
dnl Check if sys_nerr exists.
dnl Tries to link a program that mentions sys_nerr.
dnl If it exists, defines C preprocessor macro HAVE_SYS_NERR.
dnl Sets the cache variable ac_cv_var_sys_nerr to 'yes' or 'no'.
dnl
AC_DEFUN([AC_SYS_NERR],
[AC_MSG_CHECKING(for sys_nerr)
AC_CACHE_VAL(ac_cv_var_sys_nerr,
[AC_TRY_LINK(,[extern int sys_nerr; return (sys_nerr);],
	ac_cv_var_sys_nerr=yes, ac_cv_var_sys_nerr=no)
])
AC_MSG_RESULT($ac_cv_var_sys_nerr)
if test $ac_cv_var_sys_nerr = yes; then
	AC_DEFINE(HAVE_SYS_NERR, [], [Define the following if sys_nerr exists])
fi;
])

dnl ##################################################################
dnl The macro AC_DECL_SYS_NERR
dnl is based on code from configure.in distributed with LPRng-2.0.8.
dnl See the COPYING file for details.
dnl
dnl Usage:
dnl  AC_DECL_SYS_NERR
dnl Check if sys_nerr is declared in one of the usual system headers.
dnl Tries to compile a program that uses sys_nerr.
dnl If the declaration exists, defines C preprocessor macro HAVE_SYS_NERR_DECL.
dnl Sets the cache variable ac_cv_decl_sys_nerr to 'yes' or 'no'.
dnl
dnl This macro should be invoked in configure.in after all the header checks
dnl (e.g. AC_CHECK_HEADERS) have been performed, since our test program will
dnl #include "confdefs.h" (the C macro definitions accumulated by configure so far),
dnl then will #include various header files based on HAVE_STDLIB_H and HAVE_UNISTD_H we expect
dnl may appear in "confdefs.h".  Of course, to make this work, you should have invoked
dnl AC_CHECK_HEADERS(stdlib.h unistd.h) in configure.in.
dnl
AC_DEFUN([AC_DECL_SYS_NERR],
[AC_MSG_CHECKING(for sys_nerr declaration)
AC_CACHE_VAL(ac_cv_decl_sys_nerr,
[AC_TRY_COMPILE([
#include "confdefs.h"    /* the header built by configure so far */
#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif],
[printf("%d",sys_nerr);],
ac_cv_decl_sys_nerr=yes, ac_cv_decl_sys_nerr=no)
])
AC_MSG_RESULT($ac_cv_decl_sys_nerr)
if test $ac_cv_decl_sys_nerr = yes; then
    AC_DEFINE(HAVE_SYS_NERR_DECL, [], [Define the following if sys_nerr is declared])
fi
])

dnl ##################################################################
dnl The macro AC_SYS_ERRLIST
dnl is based on code from configure.in distributed with LPRng-2.0.8.
dnl See the COPYING file for details.
dnl
dnl Usage:
dnl  AC_SYS_ERRLIST
dnl Check if sys_errlist exists.
dnl Tries to link a program that mentions sys_errlist.
dnl If it exists, defines C preprocessor macro HAVE_SYS_ERRLIST.
dnl Sets the cache variable ac_cv_var_sys_errlist to 'yes' or 'no'.
dnl
AC_DEFUN([AC_SYS_ERRLIST],
[AC_MSG_CHECKING(for sys_errlist)
AC_CACHE_VAL(ac_cv_var_sys_errlist,
[AC_TRY_LINK(,[extern char *sys_errlist[];
	sys_errlist[0];],
	ac_cv_var_sys_errlist=yes, ac_cv_var_sys_errlist=no)
])
AC_MSG_RESULT($ac_cv_var_sys_errlist)
if test $ac_cv_var_sys_errlist = yes; then
	AC_DEFINE(HAVE_SYS_ERRLIST, [], [Define the following if sys_errlist exists])
fi
])

dnl ##################################################################
dnl The macro AC_DECL_SYS_ERRLIST
dnl is based on code from configure.in distributed with LPRng-2.0.8.
dnl See the COPYING file for details.
dnl
dnl Usage:
dnl  AC_DECL_SYS_ERRLIST
dnl Check if sys_errlist is declared in one of the usual system headers.
dnl Tries to compile a program that uses sys_errlist.
dnl If the declaration exists, defines C preprocessor macro HAVE_SYS_ERRLIST_DECL.
dnl Sets the cache variable ac_cv_decl_sys_errlist to 'yes' or 'no'.
dnl
dnl This macro should be invoked in configure.in after all the header checks
dnl (e.g. AC_CHECK_HEADERS) have been performed, since our test program will
dnl #include "confdefs.h" (the C macro definitions accumulated by configure so far),
dnl then will #include various header files based on HAVE_STDLIB_H and HAVE_UNISTD_H we expect
dnl may appear in "confdefs.h".  Of course, to make this work, you should have invoked
dnl AC_CHECK_HEADERS(stdlib.h unistd.h) in configure.in.
dnl
AC_DEFUN([AC_DECL_SYS_ERRLIST],
[AC_MSG_CHECKING(for sys_errlist declaration)
AC_CACHE_VAL(ac_cv_decl_sys_errlist,
[AC_TRY_COMPILE([
#include "confdefs.h"    /* the header built by configure so far */
#include <stdio.h>
#include <errno.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif],
[printf("%s",sys_errlist[0]);],
ac_cv_decl_sys_errlist=yes, ac_cv_decl_sys_errlist=no)
])
AC_MSG_RESULT($ac_cv_decl_sys_errlist)
if test $ac_cv_decl_sys_errlist = yes; then
    AC_DEFINE(HAVE_SYS_ERRLIST_DECL, [], [Define the following if sys_nerr is declared])
fi
])

