dnl Process this file with autoconf to produce a configure script.
AC_INIT(src/dhcp_probe.c)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_MACRO_DIR(m4)

AM_INIT_AUTOMAKE(dhcp_probe, 1.0.6)

dnl Determine hosttype.
AC_CANONICAL_HOST

dnl ##########################################################################

dnl Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB

dnl ##########################################################################

dnl OS-specific libraries.
case "$host" in
*solaris*)
	AC_CHECK_LIB(socket, socket, , AC_MSG_ERROR(Can't find libsocket.))
	AC_CHECK_LIB(nsl, inet_ntoa, , AC_MSG_ERROR(Can't find libnsl.))
	;;
esac

dnl ##########################################################################

dnl Checks for header files.
AC_HEADER_STDC

AC_CHECK_HEADERS(ctype.h errno.h fcntl.h netdb.h pthread.h signal.h stdio.h stdlib.h) 
AC_CHECK_HEADERS(string.h strings.h stropts.h syslog.h time.h unistd.h)
AC_CHECK_HEADERS(arpa/inet.h)
AC_CHECK_HEADERS(sys/socket.h)


AC_CHECK_HEADERS(net/if.h,,,
[[if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
]])

AC_CHECK_HEADERS(net/if_arp.h,,,
[[if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
]])

AC_CHECK_HEADERS(netinet/in.h)

AC_CHECK_HEADERS(netinet/if_ether.h,,,
[[if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
]])

AC_CHECK_HEADERS(netinet/in_systm.h)

AC_CHECK_HEADERS(netinet/ip.h,,,
[[if HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
]])

AC_CHECK_HEADERS(netinet/udp.h,,,
[[if HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
]])

AC_CHECK_HEADERS(sys/file.h sys/filio.h sys/ioctl.h sys/select.h) 
AC_CHECK_HEADERS(sys/sockio.h sys/stat.h sys/time.h sys/types.h sys/uio.h)
AC_CHECK_HEADERS(sys/un.h sys/wait.h)
dnl netinet/ether.h is supplied by GNU C Library
AC_CHECK_HEADERS(netinet/ether.h)

dnl ##########################################################################

dnl Checks for other required or optional packages.

dnl We require the libpcap header and library.
IST_REQUIRE_PCAP

dnl We require the libnet header and library.
IST_REQUIRE_LIBNET

dnl ##########################################################################

dnl Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_TIME
AC_STRUCT_TM

dnl We use some standard types that some systems don't define.
dnl We use the AC_UNP_CHECK_TYPE macro here, which checks more headers
dnl than does the standard AC_CHECK_TYPE macro.
AC_UNP_CHECK_TYPE(u_int32_t, unsigned int)

dnl We use the AC_CHECK_STRUCT_FOR macro here, to determine if
dnl struct sockaddr() has an sa_len member.  
AH_TEMPLATE([STRUCT_SOCKADDR_HAS_SA_LEN], [Define if struct sockaddr{} has a sa_len member])
AC_CHECK_STRUCT_FOR(
[#include <sys/types.h>
#include <sys/socket.h>],
sockaddr,sa_len,[])

dnl We use these macros to determine if sys_nerr exists and is declared, 
dnl and to determine if sys_errlist exists and is declared.
AC_SYS_NERR
AC_DECL_SYS_NERR
AC_SYS_ERRLIST
AC_DECL_SYS_ERRLIST

dnl ##################################################################
dnl The AC_CHECK_FUNC_PROTO macro checks for function prototypes in specific headers.

AH_TEMPLATE([HAVE_INET_ATON_PROTO], [Define the following if the inet_aton() function prototype is in <arpa/inet.h>])
AC_CHECK_FUNC_PROTO(inet_aton, arpa/inet.h)

AH_TEMPLATE([HAVE_STRERROR_PROTO], [Define the following if strerror() is declared])
AC_CHECK_FUNC_PROTO(strerror, string.h)

AH_TEMPLATE([HAVE_ETHER_ATON_PROTO], [Define the following if ether_aton() is declared])
AC_CHECK_FUNC_PROTO(ether_aton, netinet/ether.h)

AH_TEMPLATE([HAVE_ETHER_NTOA_PROTO], [Define the following if ether_ntoa() is declared])
AC_CHECK_FUNC_PROTO(ether_ntoa, netinet/ether.h)

dnl ##################################################################
dnl Checks for library functions.
AC_CHECK_FUNCS(bcmp bcopy bzero) 
AC_CHECK_FUNCS(vsnprintf) 

dnl Check for oft-missing library functions for which we provide a fallback implementation.
AC_REPLACE_FUNCS(strerror)
AC_REPLACE_FUNCS(inet_aton)

dnl ##################################################################
dnl miscellaneous platform-specific

dnl Check if socket ioctls must use STREAM ioctls instead of SIOCxxx ioctls.
IST_SYS_SOCKET_IOCTLS_USE_STREAMS

dnl ##################################################################

AC_OUTPUT(Makefile lib/Makefile src/Makefile doc/Makefile, [date > stamp-h])
