dnl Process this file with autoconf to produce a configure script.
AC_INIT(src/dhcp_probe.c)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_MACRO_DIR(m4)

AM_INIT_AUTOMAKE(dhcp_probe, 1.3.0)

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
AC_CHECK_HEADERS(stdint.h inttypes.h)

AC_CHECK_HEADERS([net/if.h], [], [],
[/* In Solaris, before using net/if.h you need sys/socket.h. */
#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
])

AC_CHECK_HEADERS([net/if_arp.h], [], [],
[#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
])

AC_CHECK_HEADERS(netinet/in.h)

AC_CHECK_HEADERS([netinet/if_ether.h], [], [],
[/* In Solaris, before using netinet/if_ether.h you need sys/socket.h and net/if.h. */
#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#if HAVE_NET_IF_H
# include <net/if.h>
#endif
])

AC_CHECK_HEADERS(netinet/in_systm.h)

AC_CHECK_HEADERS([netinet/ip.h], [], [],
[#if HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
])

AC_CHECK_HEADERS([netinet/udp.h], [], [],
[#if HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
])

AC_CHECK_HEADERS(sys/file.h sys/filio.h sys/ioctl.h sys/resource.h sys/select.h) 
AC_CHECK_HEADERS(sys/sockio.h sys/stat.h sys/time.h sys/types.h sys/uio.h)
AC_CHECK_HEADERS(sys/un.h sys/wait.h)
dnl netinet/ether.h is supplied by GNU C Library
AC_CHECK_HEADERS(netinet/ether.h)

AC_CHECK_HEADERS(net/if_dl.h)
dnl ifaddrs.h is supplied by many *BSDs
AC_CHECK_HEADERS(ifaddrs.h)

dnl ##########################################################################

dnl Checks for types.

AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

dnl ##########################################################################

dnl Checks for other required or optional packages.

dnl We require the libpcap header and library.
IST_REQUIRE_PCAP

dnl We require the libnet header and library.
IST_REQUIRE_LIBNET

dnl We require a declaration for the libnet_cq_end_loop() function.
dnl It should be declared as a result of including libnet.h if the user has added it as per INSTALL.dhcp_probe.
AC_CHECK_DECL(libnet_cq_end_loop, [], AC_MSG_ERROR([Didn't find a declaration for the libnet_cq_end_loop() function.  See INSTALL.dhcp_probe.]), [[#include <libnet.h>]])

dnl We require a definition for the libnet_cq_end_loop() function.
dnl It should be defined in the libnet library if the user has added it as per INSTALL.dhcp_probe.
AC_CHECK_FUNC(libnet_cq_end_loop, [], AC_MSG_ERROR([Didn't find the libnet_cq_end_loop() function. See INSTALL.dhcp_probe.]))

dnl ##########################################################################

dnl Checks for typedefs, structures, and compiler characteristics.

AC_HEADER_TIME
AC_STRUCT_TM

dnl We use the AC_CHECK_STRUCT_FOR macro here, to determine if
dnl struct sockaddr() has an sa_len member.  
AH_TEMPLATE([STRUCT_SOCKADDR_HAS_SA_LEN], [Define if struct sockaddr{} has a sa_len member])
AC_CHECK_STRUCT_FOR(
[#include <sys/types.h>
#include <sys/socket.h>],
sockaddr,sa_len,[])

dnl We use the AC_CHECK_STRUCT_FOR macro here, to determine if
dnl struct arpreq() has an arp_dev member.
AH_TEMPLATE([STRUCT_ARPREQ_HAS_ARP_DEV], [Define if struct arpreq{} has an arp_dev member])
AC_CHECK_STRUCT_FOR(
[#include <net/if_arp.h>],
arpreq,arp_dev,[])

dnl We use the AC_CHECK_MEMBER macro here, to determine if
dnl struct ether_addr{} has an ether_addr_octet member.
dnl In Solaris 9 and 10, we include netinet/if_ether.h get get the declaration of struct ether_addr.
AC_CHECK_MEMBER(
[struct ether_addr.ether_addr_octet], 
[ AC_DEFINE([STRUCT_ETHER_ADDR_HAS_ETHER_ADDR_OCTET], [1], [Define if struct ether_addr{} has an ether_addr_octet member]) ], 
[], 
[
#if HAVE_NETINET_IF_ETHER_H
  /* In Solaris, before using netinet/if_ether.h you need sys/socket.h and net/if.h. */
#if HAVE_SYS_TYPES_H
   /* On Solaris, you must include sys/types.h before sys/socket.h. */
#   include <sys/types.h>
#endif
#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#if HAVE_NET_IF_H
# include <net/if.h>
#endif
#include <netinet/if_ether.h>
#endif
]
)

dnl We use the AC_CHECK_MEMBER macro here, to determine if
dnl struct ether_addr{} has an octet member.
dnl In Linux and FreeBSD, we include netinet/if_ether.h get get the declaration of struct ether_addr.
AC_CHECK_MEMBER(
[struct ether_addr.octet], 
[ AC_DEFINE([STRUCT_ETHER_ADDR_HAS_OCTET], [1], [Define if struct ether_addr{} has an octet member]) ],
[],
[
#if HAVE_NETINET_IF_ETHER_H
  /* In Solaris, before using netinet/if_ether.h you need sys/socket.h and net/if.h. */
#if HAVE_SYS_TYPES_H
   /* On Solaris, you must include sys/types.h before sys/socket.h. */
#   include <sys/types.h>
#endif
#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#if HAVE_NET_IF_H
# include <net/if.h>
#endif
#include <netinet/if_ether.h>
#endif
]
)



dnl We use these macros to determine if sys_nerr exists and is declared, 
dnl and to determine if sys_errlist exists and is declared.
AC_SYS_NERR
AC_DECL_SYS_NERR
AC_SYS_ERRLIST
AC_DECL_SYS_ERRLIST

dnl ##################################################################

dnl Checks for function prototypes.

dnl The AC_CHECK_FUNC_PROTO macro checks for function prototypes in specific headers.

AH_TEMPLATE([HAVE_INET_ATON_PROTO], [Define the following if the inet_aton() function prototype is in <arpa/inet.h>])
AC_CHECK_FUNC_PROTO(inet_aton, arpa/inet.h)

AH_TEMPLATE([HAVE_STRERROR_PROTO], [Define the following if strerror() is declared])
AC_CHECK_FUNC_PROTO(strerror, string.h)

dnl While ether_aton(), ether_ntoa(), ether_ntohost(), ether_hostton() should always be available,
dnl some systems lack prototypes for them.
AC_CHECK_DECLS([ether_aton, ether_ntoa, ether_ntohost, ether_hostton], , , [[
/* On Redhat 9, they are declared in netinet/ether.h. */
#if HAVE_NETINET_ETHER_H
#include <netinet/ether.h>
#endif
/* In Solaris 10 (but not earlier), they are declared in sys/ethernet.h.
   But at build time, it's not a good idea to include sys/ethernet.h directly; while that
   would work in Solaris 10, it would break in Solaris 9 if we ever find we need to include netinet/if_ether;
   in Solaris 9 you must not include BOTH netinet/if_ether.h and sys/ethernet.h, since there are some definitions 
   duplicated in the two files.
   Since at build time we may indeed include netinet/if_ether.h, we cannot directly include sys/ethernet.h at build time.
   Since we know we will not directly include sys/ethernet.h at build time, it would be unwise at configure time
   to make decisions based on a test that involved directly including sys/ethernet.h.
   Fortunately, on Solaris 10, netinet/if_ether.h includes sys/ethernet.h for you, ultimately
   getting the ether_aton(), ether_ntoa(), ether_ntohost(), ether_hostton() declarations we want.
   So our approach is to include netinet/if_ether.h.
   On Solaris 9, this will not cause a problem; we won't find declarations for
   ether_aton(), ether_ntoa(), ether_ntohost(), ether_hostton(), but those four declarations
   aren't available in Solaris 9 anyway.  And in Solaris 10, we will find the declarations for
   ether_aton(), ether_ntoa(), ether_ntohost(), ether_hostton().
*/
#if HAVE_NETINET_IF_ETHER_H
  /* In Solaris, before using netinet/if_ether.h you need sys/socket.h and net/if.h. */
#if HAVE_SYS_TYPES_H
   /* On Solaris, you must include sys/types.h before sys/socket.h. */
#   include <sys/types.h>
#endif
#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#if HAVE_NET_IF_H
# include <net/if.h>
#endif
#include <netinet/if_ether.h>
#endif]])

dnl ##################################################################

dnl Checks for library functions.

AC_CHECK_FUNCS(bcmp bcopy bzero) 
AC_CHECK_FUNCS(vsnprintf) 
AC_CHECK_FUNCS(sysconf getrlimit)
AC_CHECK_FUNCS(getifaddrs)

dnl ##################################################################

dnl Check for oft-missing library functions for which we provide a fallback implementation.

AC_REPLACE_FUNCS(strerror)
AC_REPLACE_FUNCS(inet_aton)

dnl Include a library functions that we expect to be missing, to ensure that when our
dnl library of replacement functions is built, it contains at least one member.
dnl (Empty libraries are not portable.)
AC_REPLACE_FUNCS(my_null_proc)

dnl ##################################################################

dnl miscellaneous platform-specific

dnl Check if socket ioctls must use STREAM ioctls instead of SIOCxxx ioctls.
IST_SYS_SOCKET_IOCTLS_USE_STREAMS

dnl ##################################################################

AC_OUTPUT(Makefile lib/Makefile src/Makefile doc/Makefile, [date > stamp-h])
