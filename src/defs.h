#ifndef DEFS_H
#define DEFS_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <string.h>
#include <strings.h>

#include <time.h>


#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#endif

#ifdef HAVE_STRING_H
#include <signal.h>
#endif

#ifdef  STDC_HEADERS
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
#elif defined(HAVE_STDINT_H)
# include <stdint.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
/* On Solaris, you must include sys/types.h before sys/socket.h. */
#include <sys/socket.h>
#endif

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#ifdef HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include <limits.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <sys/wait.h>

#ifdef HAVE_NET_IF_H
#include <net/if.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

/* To get declarations of ether_aton(), ether_ntoa(), ether_ntohost(), ether_hostton() on Solaris 10,
   we need sys/ethernet.h.  But it's not a good idea to include sys/ethernet.h directly; while that
   would work in Solaris 10, it would break in Solaris 9 if we ever find we need to include netinet/if_ether;
   in Solaris 9 you must not include BOTH netinet/if_ether.h and sys/ethernet.h, since there are some definitions 
   duplicated in the two files.
   Fortunately, on Solaris 10, netinet/if_ether.h includes sys/ethernet.h for you, ultimately
   getting the ether_aton(), ether_ntoa(), ether_ntohost(), ether_hostton() declarations we want.
   So the result is that on Solaris 9, by including netinet/if_ether.h we avoid a potential future
   conflict  and we don't get ether_aton(), ether_ntoa(), ether_ntohost(), ether_hostton(), but those four declarations
   aren't available in Solaris 9 anyway.  And in Solaris 10, we get ether_aton(), ether_ntoa(), ether_ntohost(), ether_hostton().
*/
#ifdef HAVE_NETINET_IF_ETHER_H
/* In Solaris, before using netinet/if_ether.h you need sys/socket.h and net/if.h.   We already have both above. */
#include <netinet/if_ether.h>
#endif

#ifdef HAVE_NETINET_ETHER_H
/* ether_aton(), ether_ntoa(), ether_ntohost(), ether_hostton() are declared in netinet/ether.h on Redhat 9. */
#include <netinet/ether.h>
#endif

#include <netinet/in_systm.h> /* for n_long def used by netinet/ip.h */

/* In Linux, use the BSD flavor IP structure headers instead of the Linux flavor headers. */
#ifndef __FAVOR_BSD
#define __FAVOR_BSD 1
#endif

#ifdef HAVE_NETINET_IP_H
#include <netinet/ip.h>
#endif

#include <netinet/udp.h>

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifdef HAVE_NET_IF_ARP_H
#include <net/if_arp.h>
#endif

#ifndef HAVE_INET_ATON_PROTO
# include   <sys/types.h>
# include   <netinet/in.h>
extern int inet_aton(const char *, struct in_addr *);
#endif /* not HAVE_INET_ATON_PROTO */

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#include <pcap.h> /* libpcap */

#include <libnet.h> /* libnet */

#ifndef HAVE_BCOPY
#define bcopy(a, b, c)  memcpy(b, a, c)
#endif
#ifndef HAVE_BCMP
#define bcmp(a, b, c)   memcmp(a, b, c)
#endif
#ifndef HAVE_BZERO
#define bzero(s, n)     memset((s), 0, (n))
#endif

#ifdef HAVE_SIGNAL_SYSV
#define signal(s,f) sigset(s,f)
#endif


#define PASTE(a,b) a ## b

/* Prototypes for these routines are missing from some systems. */
#if !HAVE_DECL_ETHER_NTOA
extern char *ether_ntoa (const struct ether_addr *e);
#endif
#if !HAVE_DECL_ETHER_ATON
extern struct ether_addr *ether_aton(const char *hostname);
#endif
#if !HAVE_DECL_ETHER_NTOHOST
extern int ether_ntohost (char *hostname, const struct ether_addr *e);
#endif
#if !HAVE_DECL_ETHER_HOSTTON
extern int ether_hostton (const char *hostname, struct ether_addr *e);
#endif


#endif /* not DEFS_H */
