dnl ##################################################################
dnl The macro AC_UNP_CHECK_TYPE
dnl comes from aclocal.m4 in the sample code associated
dnl with "UNIX Network Programming: Volume 1", Second Edition,
dnl by W. Richard Stevens.  That sample code is available via
dnl ftp://ftp.kohala.com/pub/rstevens/unpv12e.tar.gz.
dnl
dnl Version $Id: ac_unp_check_type.m4,v 1.6 2007/06/19 19:19:08 root Exp $
dnl
dnl Usage:
dnl   AC_UNP_CHECK_TYPE(type, default)
dnl Like the standard AC_CHECK_TYPE macro, this checks if the type 'type'
dnl is defined in certain system headers, and if not, defines 'type' to
dnl be the C builtin type 'default'.
dnl
dnl You would use this macro instead of the standard AC_CHECK_TYPE
dnl because AC_CHECK_TYPE #includes only <sys/types.h>, <stdlib.h>,
dnl and <stddef.h>.
dnl Unfortunately, many implementations today hide typedefs in weird
dnl locations: Solaris 2.5.1 has uint8_t and uint32_t in <pthread.h>.
dnl SunOS 4.1.x has int8_t in <sys/bitypes.h>.
dnl So AC_UNP_CHECK_TYPE #includes more headers, then tries to compile
dnl a test program that uses the typedef.
dnl
dnl This macro should be invoked in configure.in after all the header checks
dnl (e.g. AC_CHECK_HEADERS) have been performed, since our test program will
dnl #include "confdefs.h" (the C macro definitions accumulated by configure so far),
dnl then will #include various header files based on some HAVE_foo_H values we expect
dnl may appear in "confdefs.h".  Of course, to make this work, you should have invoked
dnl AC_CHECK_HEADERS in configure.in on all those possible header files.
dnl
dnl If a type you need to check for might be defined in some *other* header file, you'll
dnl need to edit your AC_CHECK_HEADERS invocations in configure.in to add that header
dnl file, then edit the macro definition below to #include that header file
dnl (based on the appropriate HAVE_foo_H value).
dnl
AC_DEFUN([AC_UNP_CHECK_TYPE],
	[AC_MSG_CHECKING(if $1 defined)
	AC_CACHE_VAL(ac_cv_type_$1,
		AC_COMPILE_IFELSE( [AC_LANG_SOURCE(
							[[
								#include	"confdefs.h"	/* the header built by configure so far */
								#ifdef	HAVE_SYS_TYPES_H
								#  include	<sys/types.h>
								#endif
								#ifdef	HAVE_SYS_SOCKET_H
								#  include	<sys/socket.h>
								#endif
								#ifdef	HAVE_SYS_TIME_H
								#  include    <sys/time.h>
								#endif
								#ifdef	HAVE_NETINET_IN_H
								#  include    <netinet/in.h>
								#endif
								#ifdef	HAVE_ARPA_INET_H
								#  include    <arpa/inet.h>
								#endif
								#ifdef	HAVE_ERRNO_H
								#  include    <errno.h>
								#endif
								#ifdef	HAVE_FCNTL_H
								#  include    <fcntl.h>
								#endif
								#ifdef	HAVE_NETDB_H
								#  include	<netdb.h>
								#endif
								#ifdef	HAVE_SIGNAL_H
								#  include	<signal.h>
								#endif
								#ifdef	HAVE_STDIO_H
								#  include	<stdio.h>
								#endif
								#ifdef	HAVE_STDLIB_H
								#  include	<stdlib.h>
								#endif
								#ifdef	HAVE_STRING_H
								#  include	<string.h>
								#endif
								#ifdef	HAVE_SYS_STAT_H
								#  include	<sys/stat.h>
								#endif
								#ifdef	HAVE_SYS_UIO_H
								#  include	<sys/uio.h>
								#endif
								#ifdef	HAVE_UNISTD_H
								#  include	<unistd.h>
								#endif
								#ifdef	HAVE_SYS_WAIT_H
								#  include	<sys/wait.h>
								#endif
								#ifdef	HAVE_SYS_UN_H
								#  include	<sys/un.h>
								#endif
								#ifdef	HAVE_SYS_SELECT_H
								# include   <sys/select.h>
								#endif
								#ifdef	HAVE_STRINGS_H
								# include   <strings.h>
								#endif
								#ifdef	HAVE_SYS_IOCTL_H
								# include   <sys/ioctl.h>
								#endif
								#ifdef	HAVE_SYS_FILIO_H
								# include   <sys/filio.h>
								#endif
								#ifdef	HAVE_SYS_SOCKIO_H
								# include   <sys/sockio.h>
								#endif
								#ifdef	HAVE_PTHREAD_H
								#  include	<pthread.h>
								#endif
							]]
							[[
			 					$1 foo ;
							]]
							)
						],
			ac_cv_type_$1=yes
		,
			ac_cv_type_$1=no
		)
	)
		AC_MSG_RESULT($ac_cv_type_$1)
		if test $ac_cv_type_$1 = no ; then
			AC_DEFINE($1, $2, [On systems that lack a typedef for $1, define it as $2.])
		fi
])
