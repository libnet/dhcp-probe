#ifndef DEFS_H
#define DEFS_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
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

#include <sys/types.h>

#include <sys/socket.h>

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

#include <unistd.h>

#include <sys/wait.h>

#ifdef HAVE_NET_IF_H
#include <net/if.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef HAVE_NETINET_IF_ETHER_H
#include <netinet/if_ether.h>
#endif

#include <netinet/in_systm.h> /* for n_long def used by netinet/ip.h */

#include <netinet/ip.h>

#include <netinet/udp.h>

#ifdef HAVE_NETINET_ETHER_H
#include <netinet/ether.h>
#endif

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
#ifndef HAVE_ETHER_NTOA_PROTO
char *ether_ntoa (struct ether_addr *e);
#endif
#ifndef HAVE_ETHER_ATON_PROTO
struct ether_addr *ether_aton(char *hostname);
#endif
#ifndef HAVE_ETHER_NTOHOST_PROTO
int ether_ntohost (char *hostname, struct ether_addr *e);
#endif
#ifndef HAVE_ETHER_HOSTTON_PROTO
int ether_hostton (char *hostname, struct ether_addr *e);
#endif


#endif /* not DEFS_H */
