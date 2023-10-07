dnl ##################################################################
dnl The macro IST_REQUIRE_PCAP
dnl is by Irwin Tillman, and may be distributed under the GNU Public License.
dnl See the COPYING file for details.
dnl
dnl Version $Id: ist_require_pcap.m4,v 1.2 2004/11/04 20:47:40 root Exp $
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
	LDFLAGS="$LDFLAGS -L${IST_REQUIRE_PCAP_LIB}"
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
