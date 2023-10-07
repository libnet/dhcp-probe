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
