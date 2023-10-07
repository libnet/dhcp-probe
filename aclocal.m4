dnl aclocal.m4 generated automatically by aclocal 1.4

dnl Copyright (C) 1994, 1995-8, 1999 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY, to the extent permitted by law; without
dnl even the implied warranty of MERCHANTABILITY or FITNESS FOR A
dnl PARTICULAR PURPOSE.

# Do all the work for Automake.  This macro actually does too much --
# some checks are only needed if your package does certain things.
# But this isn't really a big deal.

# serial 1

dnl Usage:
dnl AM_INIT_AUTOMAKE(package,version, [no-define])

AC_DEFUN(AM_INIT_AUTOMAKE,
[AC_REQUIRE([AC_PROG_INSTALL])
PACKAGE=[$1]
AC_SUBST(PACKAGE)
VERSION=[$2]
AC_SUBST(VERSION)
dnl test to see if srcdir already configured
if test "`cd $srcdir && pwd`" != "`pwd`" && test -f $srcdir/config.status; then
  AC_MSG_ERROR([source directory already configured; run "make distclean" there first])
fi
ifelse([$3],,
AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE", [Name of package])
AC_DEFINE_UNQUOTED(VERSION, "$VERSION", [Version number of package]))
AC_REQUIRE([AM_SANITY_CHECK])
AC_REQUIRE([AC_ARG_PROGRAM])
dnl FIXME This is truly gross.
missing_dir=`cd $ac_aux_dir && pwd`
AM_MISSING_PROG(ACLOCAL, aclocal, $missing_dir)
AM_MISSING_PROG(AUTOCONF, autoconf, $missing_dir)
AM_MISSING_PROG(AUTOMAKE, automake, $missing_dir)
AM_MISSING_PROG(AUTOHEADER, autoheader, $missing_dir)
AM_MISSING_PROG(MAKEINFO, makeinfo, $missing_dir)
AC_REQUIRE([AC_PROG_MAKE_SET])])

#
# Check to make sure that the build environment is sane.
#

AC_DEFUN(AM_SANITY_CHECK,
[AC_MSG_CHECKING([whether build environment is sane])
# Just in case
sleep 1
echo timestamp > conftestfile
# Do `set' in a subshell so we don't clobber the current shell's
# arguments.  Must try -L first in case configure is actually a
# symlink; some systems play weird games with the mod time of symlinks
# (eg FreeBSD returns the mod time of the symlink's containing
# directory).
if (
   set X `ls -Lt $srcdir/configure conftestfile 2> /dev/null`
   if test "[$]*" = "X"; then
      # -L didn't work.
      set X `ls -t $srcdir/configure conftestfile`
   fi
   if test "[$]*" != "X $srcdir/configure conftestfile" \
      && test "[$]*" != "X conftestfile $srcdir/configure"; then

      # If neither matched, then we have a broken ls.  This can happen
      # if, for instance, CONFIG_SHELL is bash and it inherits a
      # broken ls alias from the environment.  This has actually
      # happened.  Such a system could not be considered "sane".
      AC_MSG_ERROR([ls -t appears to fail.  Make sure there is not a broken
alias in your environment])
   fi

   test "[$]2" = conftestfile
   )
then
   # Ok.
   :
else
   AC_MSG_ERROR([newly created file is older than distributed files!
Check your system clock])
fi
rm -f conftest*
AC_MSG_RESULT(yes)])

dnl AM_MISSING_PROG(NAME, PROGRAM, DIRECTORY)
dnl The program must properly implement --version.
AC_DEFUN(AM_MISSING_PROG,
[AC_MSG_CHECKING(for working $2)
# Run test in a subshell; some versions of sh will print an error if
# an executable is not found, even if stderr is redirected.
# Redirect stdin to placate older versions of autoconf.  Sigh.
if ($2 --version) < /dev/null > /dev/null 2>&1; then
   $1=$2
   AC_MSG_RESULT(found)
else
   $1="$3/missing $2"
   AC_MSG_RESULT(missing)
fi
AC_SUBST($1)])

# Like AC_CONFIG_HEADER, but automatically create stamp file.

AC_DEFUN(AM_CONFIG_HEADER,
[AC_PREREQ([2.12])
AC_CONFIG_HEADER([$1])
dnl When config.status generates a header, we must update the stamp-h file.
dnl This file resides in the same directory as the config header
dnl that is generated.  We must strip everything past the first ":",
dnl and everything past the last "/".
AC_OUTPUT_COMMANDS(changequote(<<,>>)dnl
ifelse(patsubst(<<$1>>, <<[^ ]>>, <<>>), <<>>,
<<test -z "<<$>>CONFIG_HEADERS" || echo timestamp > patsubst(<<$1>>, <<^\([^:]*/\)?.*>>, <<\1>>)stamp-h<<>>dnl>>,
<<am_indx=1
for am_file in <<$1>>; do
  case " <<$>>CONFIG_HEADERS " in
  *" <<$>>am_file "*<<)>>
    echo timestamp > `echo <<$>>am_file | sed -e 's%:.*%%' -e 's%[^/]*$%%'`stamp-h$am_indx
    ;;
  esac
  am_indx=`expr "<<$>>am_indx" + 1`
done<<>>dnl>>)
changequote([,]))])

dnl ##################################################################
dnl The macro IST_REQUIRE_PCAP
dnl is by Irwin Tillman, and may be distributed under the GNU Public License.
dnl See the COPYING file for details.
dnl
dnl Version $Id: ist_require_pcap.m4,v 1.1 2001/01/01 17:00:16 irwin Exp $
dnl
dnl Usage:
dnl    IST_REQUIRE_PCAP
dnl Require that pcap include and lib are present.
dnl User may specify --with-pcap=DIR to specify parent of the include/ and lib/ directories that contain
dnl the pcap header and lib, respectively.
dnl User may specify -with-pcap-include=DIR to specify directory containing the pcap header file.
dnl User may specify -with-pcap-lib=DIR to specify directory containing the pcap lib file.
dnl
dnl If library or include is missing, we bomb out.
dnl If both are found, we adjust CPPFLAGS, LDFLAGS, and LIBS to make the PCAP includes and library available.
dnl
AC_DEFUN([IST_REQUIRE_PCAP],[
	dnl set IST_REQUIRE_PCAP_HOME to the default prefix we'll use to find include and lib
	IST_REQUIRE_PCAP_HOME=/usr/local

	dnl change IST_REQUIRE_PCAP_HOME to user-specified value
	AC_ARG_WITH([pcap],
		[  --with-pcap=DIR         parent of the include and lib dirs containing pcap header and lib (defaults to /usr/local)],
		[if test x"$withval" = "x" -o x"$withval" = "xyes"; then
			AC_MSG_ERROR([Missing directory for --with-pcap.])
		elif test x"$withval" = "xno"; then
			AC_MSG_ERROR([ Specifying --with-pcap=no or --without-pcap is not permitted.
This package cannot be compiled without the pcap include and library.])
		else
			IST_REQUIRE_PCAP_HOME="$withval"
		fi
		]
		dnl option not mentioned at all by user, leave it set to default
	)

	AC_ARG_WITH([pcap-include],
		[  --with-pcap-include=DIR directory containing pcap header (defaults to /usr/local/include)],
		[if test x"$withval" = "x" -o x"$withval" = "xyes"; then
			AC_MSG_ERROR([Missing directory for --with-pcap-include.])
		elif test x"$withval" = "xno"; then
			AC_MSG_ERROR([ Specifying --with-pcap-include=no or --without-pcap-include is not permitted.
This package cannot be compiled without the pcap header.])
		else
			IST_REQUIRE_PCAP_INCLUDE="$withval"
		fi
		],
		dnl option not mentioned at all by user, use our default
		[ IST_REQUIRE_PCAP_INCLUDE="$IST_REQUIRE_PCAP_HOME/include" ]
	)

	AC_ARG_WITH([pcap-lib],
		[  --with-pcap-lib=DIR     directory containing pcap library (defaults to /usr/local/lib)],
		[if test x"$withval" = "x" -o x"$withval" = "xyes"; then
			AC_MSG_ERROR([Missing directory for --with-pcap-lib.])
		elif test x"$withval" = "xno"; then
			AC_MSG_ERROR([ Specifying --with-pcap-lib=no or --without-pcap-lib is not permitted.
This package cannot be compiled without the pcap library.])
		else
			IST_REQUIRE_PCAP_LIB="$withval"
		fi
		],
		dnl option not mentioned at all by user, use our default
		[ IST_REQUIRE_PCAP_LIB="$IST_REQUIRE_PCAP_HOME/lib" ]
	)

	dnl Save existing values in case we must back out.
	IST_REQUIRE_PCAP_OLD_CPPFLAGS=$CPPFLAGS
	IST_REQUIRE_PCAP_OLD_LDFLAGS=$LDFLAGS
	dnl IST_REQUIRE_PCAP_OLD_LIBS=$LIBS
	dnl Modify values to reflect pcap
	CPPFLAGS="$CPPFLAGS -I${IST_REQUIRE_PCAP_INCLUDE}"
	LDFLAGS="$LDFLAGS -L ${IST_REQUIRE_PCAP_LIB}"
	dnl we don't add -lpcap to LIBS since ac_check_lib does that later
	dnl LIBS="$LIBS -lpcap"

	AC_CHECK_HEADERS(pcap.h, ,
		AC_MSG_WARN([can't build without pcap.h (the libpcap header)])
		IST_REQUIRE_PCAP_RESULT=failed
	)

	AC_CHECK_LIB(pcap, pcap_lookupdev, ,
		AC_MSG_WARN([can't build without libpcap])
		IST_REQUIRE_PCAP_RESULT=failed
	)

	if test x"$IST_REQUIRE_PCAP_RESULT" = "xfailed"; then
		dnl revert values and bomb out
		CPPFLAGS=$IST_REQUIRE_PCAP_OLD_CPPFLAGS
		LDFLAGS=$IST_REQUIRE_PCAP_OLD_LDFLAGS
		dnl LIBS=$IST_REQUIRE_PCAP_OLD_LIBS
		AC_MSG_ERROR([Specify where to find pcap using --with-pcap=DIR (or --with-pcap-include, --with-pcap-lib).])
	fi
])

dnl ##################################################################
dnl The macro IST_REQUIRE_LIBNET
dnl is by Irwin Tillman, and may be distributed under the GNU Public License.
dnl See the COPYING file for details.
dnl
dnl Version $Id: ist_require_libnet.m4,v 1.1 2001/01/03 18:32:08 irwin Exp $
dnl
dnl Usage:
dnl    IST_REQUIRE_LIBNET
dnl Require that libnet header and lib are present.
dnl User may specify --with-libnet=DIR to specify parent of the include/ and lib/ directories that contain
dnl the libnet header and lib, respectively.
dnl User may specify -with-libnet-include=DIR to specify directory containing the libnet header file.
dnl User may specify -with-libnet-lib=DIR to specify directory containing the libnet lib file.
dnl
dnl We look for the 'libnet-config' program (an executable installed by libnet) in the path, bombing out if missing.
dnl We modify CPPFLAGS, CFLAGS, and LIBS to append whatever the 'libnet-config' program believes are necessary.
dnl (Those values is particular to the current build and target (assumed by libnet to be the same), and was 
dnl hard-coded into the 'libnet-config' program at the time libnet is built and installed.)  
dnl
dnl If header or library is missing, we bomb out.
dnl If both are found, we adjust CPPFLAGS, LDFLAGS, and LIBS to make the libnet header and library available.
dnl
AC_DEFUN([IST_REQUIRE_LIBNET],[
	dnl set IST_REQUIRE_LIBNET_HOME to the default prefix we'll use to find include and lib
	IST_REQUIRE_LIBNET_HOME=/usr/local

	dnl change IST_REQUIRE_LIBNET_HOME to user-specified value
	AC_ARG_WITH([libnet],
		[  --with-libnet=DIR       parent of the include and lib dirs containing libnet header and lib (defaults to /usr/local)],
		[if test x"$withval" = "x" -o x"$withval" = "xyes"; then
			AC_MSG_ERROR([Missing directory for --with-libnet.])
		elif test x"$withval" = "xno"; then
			AC_MSG_ERROR([ Specifying --with-libnet=no or --without-libnet is not permitted.
This package cannot be compiled without the libnet include and library.])
		else
			IST_REQUIRE_LIBNET_HOME="$withval"
		fi
		]
		dnl option not mentioned at all by user, leave it set to default
	)

	AC_ARG_WITH([libnet-include],
		[  --with-libnet-include=DIR directory containing libnet header (defaults to /usr/local/include)],
		[if test x"$withval" = "x" -o x"$withval" = "xyes"; then
			AC_MSG_ERROR([Missing directory for --with-libnet-include.])
		elif test x"$withval" = "xno"; then
			AC_MSG_ERROR([ Specifying --with-libnet-include=no or --without-libnet-include is not permitted.
This package cannot be compiled without the libnet header.])
		else
			IST_REQUIRE_LIBNET_INCLUDE="$withval"
		fi
		],
		dnl option not mentioned at all by user, use our default
		[ IST_REQUIRE_LIBNET_INCLUDE="$IST_REQUIRE_LIBNET_HOME/include" ]
	)

	AC_ARG_WITH([libnet-lib],
		[  --with-libnet-lib=DIR   directory containing libnet library (defaults to /usr/local/lib)],
		[if test x"$withval" = "x" -o x"$withval" = "xyes"; then
			AC_MSG_ERROR([Missing directory for --with-libnet-lib.])
		elif test x"$withval" = "xno"; then
			AC_MSG_ERROR([ Specifying --with-libnet-lib=no or --without-libnet-lib is not permitted.
This package cannot be compiled without the libnet library.])
		else
			IST_REQUIRE_LIBNET_LIB="$withval"
		fi
		],
		dnl option not mentioned at all by user, use our default
		[ IST_REQUIRE_LIBNET_LIB="$IST_REQUIRE_LIBNET_HOME/lib" ]
	)

	dnl Save existing values in case we must back out.
	IST_REQUIRE_LIBNET_OLD_CPPFLAGS=$CPPFLAGS
	IST_REQUIRE_LIBNET_OLD_CFLAGS=$CFLAGS
	IST_REQUIRE_LIBNET_OLD_LDFLAGS=$LDFLAGS
	IST_REQUIRE_LIBNET_OLD_LIBS=$LIBS

	dnl libnet-config is a program installed as part of libnet.
	AC_CHECK_PROG(LIBNET_CONFIG, libnet-config, found)
	if test x"$LIBNET_CONFIG" = "x"; then
		AC_MSG_ERROR([libnet-config executable not found in path.  Make sure libnet is installed.])
	fi

	dnl Use the 'libnet-config' program to learn which CPPFLAGS (definitions), CFLAGS, and LIBS are needed by libnet
	dnl on this platform, adding them to the macros accordingly.
	dnl (It is possible, even likely, that some of the values added here will duplicate those already
	dnl present, or to be added later to these macros.  That's OK.)
	dnl These values don't reflect those that must be specified to LOCATE the libnet header and library, just
	dnl some "other" values libnet thinks you'll need to specify in order to USE libnet on your platform.
	dnl We must add these now, so that our header and lib checks later work, as they implicitly rely on those macros.
	CPPFLAGS="$CPPFLAGS `libnet-config --defines`"
	CFLAGS="$CFLAGS `libnet-config --cflags`"
	LIBS="$LIBS `libnet-config --libs`"

	dnl Modify CPPFLAGS and LDFLAGS to add dirs needed to find libnet header and library.
	CPPFLAGS="$CPPFLAGS -I${IST_REQUIRE_LIBNET_INCLUDE}"
	LDFLAGS="$LDFLAGS -L ${IST_REQUIRE_LIBNET_LIB}"
	dnl we don't add -lnet to LIBS here, since ac_check_lib does that later
	dnl LIBS="$LIBS -lnet"

	dnl Now that all the macros are set correctly, we can test for the header.
	dnl (The header is unusual in that it generates an error if you don't compile with the 
	dnl macro definitions specified by `libnet-config --defines`.)
	AC_CHECK_HEADERS(libnet.h, ,
		AC_MSG_WARN([can't build without libnet.h (the libnet header)])
		IST_REQUIRE_LIBNET_RESULT=failed
	)

	dnl Now that all the macros are set correctly, we can test for the library.
	AC_CHECK_LIB(net, libnet_build_ip, ,
		AC_MSG_WARN([can't build without libnet])
		IST_REQUIRE_LIBNET_RESULT=failed
	)

	if test x"$IST_REQUIRE_LIBNET_RESULT" = "xfailed"; then
		dnl revert values and bomb out
		CPPFLAGS=$IST_REQUIRE_LIBNET_OLD_CPPFLAGS
		CFLAGS=$IST_REQUIRE_LIBNET_OLD_CFLAGS
		LDFLAGS=$IST_REQUIRE_LIBNET_OLD_LDFLAGS
		LIBS=$IST_REQUIRE_LIBNET_OLD_LIBS
		AC_MSG_ERROR([Specify where to find libnet using --with-libnet=DIR (or --with-libnet-include, --with-libnet-lib).])
	fi
])

dnl ##################################################################
dnl The macro AC_UNP_CHECK_TYPE
dnl comes from aclocal.m4 in the sample code associated
dnl with "UNIX Network Programming: Volume 1", Second Edition,
dnl by W. Richard Stevens.  That sample code is available via
dnl ftp://ftp.kohala.com/pub/rstevens/unpv12e.tar.gz.
dnl
dnl Version $Id: ac_unp_check_type.m4,v 1.4 2001/01/26 18:54:34 irwin Exp $
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
AC_DEFUN(AC_UNP_CHECK_TYPE,
	[AC_MSG_CHECKING(if $1 defined)
	AC_CACHE_VAL(ac_cv_type_$1,
		AC_TRY_COMPILE(
[
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
#endif],
		[ $1 foo ],
		ac_cv_type_$1=yes,
		ac_cv_type_$1=no))
	AC_MSG_RESULT($ac_cv_type_$1)
	if test $ac_cv_type_$1 = no ; then
		AC_DEFINE($1, $2, [On systems that lack a typedef for $1, define it as $2.])
	fi
])

dnl ##################################################################
dnl The macro AC_CHECK_STRUCT_FOR is copyrighted by Wes Hardaker
dnl <wjhardaker@ucdavis.edu>, and was obtained from the GNU autoconf macro archive
dnl at http://research.cys.de/autoconf-archive/
dnl See the COPYING file for details.
dnl
dnl @synopsis AC_CHECK_STRUCT_FOR(INCLUDES,STRUCT,MEMBER,DEFINE,[no])
dnl
dnl Checks STRUCT for MEMBER and defines DEFINE if found.
dnl
dnl @version $Id: aclocal.m4,v 1.3 2001/01/17 01:20:29 irwin Exp $
dnl @author Wes Hardaker <wjhardaker@ucdavis.edu>
dnl
AC_DEFUN([AC_CHECK_STRUCT_FOR],[
ac_safe_struct=`echo "$2" | sed 'y%./+-%__p_%'`
ac_safe_member=`echo "$3" | sed 'y%./+-%__p_%'`
ac_safe_all="ac_cv_struct_${ac_safe_struct}_has_${ac_safe_member}"
changequote(, )dnl
  ac_uc_define=STRUCT_`echo "${ac_safe_struct}_HAS_${ac_safe_member}" | sed 'y%abcdefghijklmnopqrstuvwxyz./-%ABCDEFGHIJKLMNOPQRSTUVWXYZ___%'`
changequote([, ])dnl

AC_MSG_CHECKING([for $2.$3])
AC_CACHE_VAL($ac_safe_all,
[
if test "x$4" = "x"; then
  defineit="= 0"
elif test "x$4" = "xno"; then
  defineit=""
else
  defineit="$4"
fi
AC_TRY_COMPILE([
$1
],[
struct $2 testit;
testit.$3 $defineit;
], eval "${ac_safe_all}=yes", eval "${ac_safe_all}=no" )
])

if eval "test \"x$`echo ${ac_safe_all}`\" = \"xyes\""; then
  AC_MSG_RESULT(yes)
  AC_DEFINE_UNQUOTED($ac_uc_define)
else
  AC_MSG_RESULT(no)
fi
])



dnl ##################################################################
dnl The macro AC_SYS_NERR
dnl is based on code from configure.in distributed with LPRng-2.0.8.
dnl See the COPYING file for details.
dnl
dnl Version $Id: sys_errlist.m4,v 1.2 2001/01/26 19:02:40 irwin Exp $
dnl 
dnl Usage:
dnl  AC_SYS_NERR
dnl Check if sys_nerr exists.
dnl Tries to link a program that mentions sys_nerr.
dnl If it exists, defines C preprocessor macro HAVE_SYS_NERR.
dnl Sets the cache variable ac_cv_var_sys_nerr to 'yes' or 'no'.
dnl
AC_DEFUN(AC_SYS_NERR,
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
AC_DEFUN(AC_DECL_SYS_NERR,
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
AC_DEFUN(AC_SYS_ERRLIST,
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
AC_DEFUN(AC_DECL_SYS_ERRLIST,
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


dnl ##################################################################
dnl The macro AC_CHECK_FUNC_PROTO
dnl comes from aclocal.m4 in the sample code associated
dnl with "UNIX Network Programming: Volume 1", Second Edition,
dnl by W. Richard Stevens.  That sample code is available via
dnl ftp://ftp.kohala.com/pub/rstevens/unpv12e.tar.gz.
dnl
dnl Version $Id: ac_check_func_proto.m4,v 1.1 2001/01/26 18:55:06 irwin Exp $
dnl
dnl AC_CHECK_FUNC_PROTO(function, header)
dnl Check for a function prototype in a given system header file.
dnl Runs the preprocessor on the system header file then checks to see if 'function'
dnl (treated as an egrep regexp) matches the output of the preprocessor.
dnl If found, defines C preprocessor macro HAVE_function_PROTO (with function name made uppercase).
dnl Sets the cache variable ac_cv_have_function_proto to 'yes' or 'no'.
dnl
dnl To improve the appearance of the resulting config.h.in file, for each function 'foo' you call us with,
dnl you'll probably want to add something like the following to your acconfig.h file (after a '@BOTTOM@' statement):
dnl   /* Define the following if the foo() function prototype is in <header> */
dnl   #undef  HAVE_foo_PROTO    /* <header> */
dnl
AC_DEFUN(AC_CHECK_FUNC_PROTO,
	[AC_CACHE_CHECK(for $1 function prototype in $2, ac_cv_have_$1_proto,
		AC_EGREP_HEADER($1, $2,
			ac_cv_have_$1_proto=yes,
			ac_cv_have_$1_proto=no))
	if test $ac_cv_have_$1_proto = yes ; then
		ac_tr_func=HAVE_`echo $1 | tr 'abcdefghijklmnopqrstuvwxyz' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'`_PROTO
		AC_DEFINE_UNQUOTED($ac_tr_func)
	fi
])

dnl ##################################################################
dnl The macro IST_SYS_SOCKET_IOCTLS_USE_STREAMS
dnl is by Irwin Tillman, and may be distributed under the GNU Public License.
dnl See the COPYING file for details.
dnl
dnl Version $Id: ist_sys_socket_ioctls_use_streams.m4,v 1.3 2001/01/26 18:11:10 irwin Exp $
dnl
dnl Usage:
dnl    IST_SYS_SOCKET_IOCTLS_USE_STREAMS
dnl Defines SYS_SOCKET_IOCTLS_USE_STREAMS if this system requires you to use STREAMS ioctls on sockets 
dnl instead of SIOCxxx ioctls on sockets.
dnl Also sets the cache variable ist_sys_socket_ioctls_use_streams to 'yes' or 'no'.
dnl (Side effects: also defines cache variable ac_cv_header_stropts_h. and C preprocessor macro 
dnl HAVE_STROPTS_H based on whether stropts.h header is present.)
dnl
dnl Our approach is to guess that if your systems has stropts.h header (STREAMS user options definitions) 
dnl and is on a short list of known host platforms, that we should use STREAMS ioctls.  
dnl (Having a list of host platforms here isn't good; we really should try compiling and running a 
dnl test program instead.)
dnl
AC_DEFUN([IST_SYS_SOCKET_IOCTLS_USE_STREAMS], [
	dnl Check for stropts.h just for the side-effect of setting ac_cv_header_stropts_h.
	AC_CHECK_HEADERS(stropts.h)

	dnl We need the $host variable to have been set
	AC_REQUIRE([AC_CANONICAL_HOST])

	AC_CACHE_CHECK(if socket ioctls should use STREAMS instead of SIOCxxx, ist_sys_socket_ioctls_use_streams, [
		if test "$ac_cv_header_stropts_h" = "yes"; then
			dnl stropts.h present, check for particular platform flavors
			case "$host" in
				*-sni-sysv*)	ist_sys_socket_ioctls_use_streams=yes ;;
				*-*-ptx*)		ist_sys_socket_ioctls_use_streams=yes ;;
	    		*) 				ist_sys_socket_ioctls_use_streams=no  ;;
			esac
		else
			dnl stropts.h missing, assume we don't need to use stream ioctls
			ist_sys_socket_ioctls_use_streams=no 
		fi
	])

	if test "$ist_sys_socket_ioctls_use_streams" = "yes"; then
 		AC_DEFINE(SYS_SOCKET_IOCTLS_USE_STREAMS, [], [Define if STREAMS ioctls should be used on socket descriptors, instead of SIOCxxx ioctls])
	fi
])

