dnl ##################################################################
dnl The macro IST_REQUIRE_LIBNET
dnl is by Irwin Tillman, and may be distributed under the GNU Public License.
dnl See the COPYING file for details.
dnl
dnl Version $Id: ist_require_libnet.m4,v 1.3 2004/11/04 20:46:51 root Exp $
dnl
dnl Usage:
dnl    IST_REQUIRE_LIBNET
dnl Require that libnet header and lib (version 1.1 or later) are present.
dnl User may specify --with-libnet=DIR to specify parent of the include/ and lib/ directories that contain
dnl the libnet header and lib, respectively.
dnl User may specify -with-libnet-include=DIR to specify directory containing the libnet header file.
dnl User may specify -with-libnet-lib=DIR to specify directory containing the libnet lib file.
dnl
dnl If header or library is missing, we bomb out.
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
This package cannot be compiled without the libnet header and library.])
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

	dnl Modify CPPFLAGS and LDFLAGS to add dirs needed to find libnet header and library.
	CPPFLAGS="$CPPFLAGS -I${IST_REQUIRE_LIBNET_INCLUDE}"
	LDFLAGS="$LDFLAGS -L${IST_REQUIRE_LIBNET_LIB}"
	dnl we don't add -lnet to LIBS here, since ac_check_lib does that later
	dnl LIBS="$LIBS -lnet"

	dnl Test for the libnet header.
	AC_CHECK_HEADERS(libnet.h, ,
		AC_MSG_WARN([can't build without libnet.h (the libnet header)])
		IST_REQUIRE_LIBNET_RESULT=failed
	)

	dnl Test for the libnet library.
	AC_CHECK_LIB(net, libnet_build_ipv4, ,
		AC_MSG_WARN([can't build without libnet 1.1 or later])
		IST_REQUIRE_LIBNET_RESULT=failed
	)

	if test x"$IST_REQUIRE_LIBNET_RESULT" = "xfailed"; then
		dnl revert values and bomb out
		AC_MSG_ERROR([Specify where to find libnet 1.1 or later using --with-libnet=DIR (or --with-libnet-include, --with-libnet-lib).])
	fi
])
