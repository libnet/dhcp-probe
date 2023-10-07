/* 
 * get_myeaddr.c: get my own ethernet address (for a specified IP address)
 *
 * The code for the case where we use ioctl(SIOCGARP) is not defined is based on prmac.c on page 442 of
 * "UNIX Network Programming: Volume 1", Second Edition,
 * by W. Richard Stevens.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <strings.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
/* On Solaris, you must include sys/types.h before sys/socket.h. */
#include <sys/socket.h>
#endif

#include <sys/ioctl.h>

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h> /* struct in_addr */
#endif

#ifdef HAVE_NET_IF_ARP_H
#include <net/if_arp.h>
#endif

#include "defs.h"

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_STROPTS_H
#include <stropts.h>
#endif

#ifdef HAVE_NET_IF_DL_H /* for sockaddr_dl{} and LLADDR */
#include <net/if_dl.h>
#endif

#ifdef HAVE_IFADDRS_H
#include <ifaddrs.h> /* for getifaddrs() */
#endif

#include "get_myeaddr.h"

#include "report.h"


int
get_myeaddr(int sockfd, struct in_addr *my_ipaddr, struct ether_addr *my_eaddr, const char *ifname)
{
/* If SIOCGIFHWADDR is defined, 
     We use the SIOCGIFHWADDR ioctl to do our work as follows:
     Given interface name 'ifname', determine the corresponding 
     Ethernet addr, store it in 'my_eaddr'.
     Ignores my_ipaddr.
     Needs a dgram sockfd for temp use.
   This is what you'll typically see on Solaris 9.

   Else if SIOCGARP is defined:
     We use the SIOCGARP ioctl to do our work as follows:
     Given my IP address 'my_ipaddr', determine the corresponding Ethernet addr, store it in 'my_eaddr'.
     Needs a dgram sockfd for temp use.
     May optionally be passed the interface name, which is needed on some platforms, else NULL.
   This is what you'll typically see on Linux.

   Else if HAVE_GETIFADDRS is defined:
     We use getifaddrs() to do our work as follows:
     Given interface name 'ifname', determine the corresponding
     Ethernet addr, store it in 'my_eaddr'.
     Ignores my_ipaddr.
   This is what you'll typically see on *BSD.

   Return 0 on success, <0 on failure.
*/

#ifdef SIOCGIFHWADDR
	struct ifreq ifr;

	bzero(&ifr, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0) {
		report(LOG_ERR, "get_myeaddr: ioctl(SIOCGIFHWADDR): %s", get_errmsg());
		return(-1);
	}

	bcopy(ifr.ifr_hwaddr.sa_data, my_eaddr, sizeof (struct ether_addr));

#elif defined SIOCGARP /* not SIOCGIFHWADDR */

	struct arpreq arpreq;
	struct sockaddr_in *sin;
	int rc;
#ifdef SYS_SOCKET_IOCTLS_USE_STREAMS
	/* On systems that don't support SIOCxxx ioctls on socket descriptors, we will issue
	   I_STR ioctls instead.  We use ioc to hold args/result, and copy any values we need
	   out of it to where we would expect to find them. */
	int fd;
	struct strioctl iocb;
#endif /* SYS_SOCKET_IOCTLS_USE_STREAMS */


	bzero(&arpreq, sizeof(arpreq));

	/* get access to the protocol address member of arpreq, treated as a sockaddr_in */
	sin = (struct sockaddr_in *) &arpreq.arp_pa;

	/* fill in sin with the IP address we are searching for */
	sin->sin_family = AF_INET;
	bcopy(my_ipaddr, &sin->sin_addr, sizeof(struct in_addr));

#ifdef STRUCT_ARPREQ_HAS_ARP_DEV
	/* Some systems have an arpreq.arp_dev member, which must be set to
	   the interface name.
	*/
	if (ifname) {
		strcpy(arpreq.arp_dev, ifname);
	}
#endif /* STRUCT_ARPREQ_HAS_ARP_DEV */

	/* retrieve arp cache entry */
#ifdef SYS_SOCKET_IOCTLS_USE_STREAMS
	/* this streams code hasn't been tested */
	if ((fd=open("/dev/arp", O_RDONLY)) < 0) {
		report(LOG_ERR, "get_myeaddr: open(/dev/arp): %s", get_errmsg);
		return(-1);
	}
	iocb.ic_cmd = SIOCGARP;
	iocb.ic_timout = 0;
	iocb.ic_dp = (char *) &arpreq;
	iocb.ic_len = sizeof(arpreq);
	rc = ioctl(fd, I_STR, (caddr_t)&iocb);
	close(fd);
#else /* SYS_SOCKET_IOCTLS_USE_STREAMS */
	rc = ioctl(sockfd, SIOCGARP, &arpreq);
#endif /* SYS_SOCKET_IOCTLS_USE_STREAMS */

	if (rc < 0) {
		report(LOG_ERR, "get_myeaddr: ioctl(SIOCGARP): %s", get_errmsg());
		return(-1);
	}

	/* it *should* be complete since it's our own, but let's be sure */
	if (! (arpreq.arp_flags & ATF_COM)) {
		report(LOG_ERR, "get_myeaddr: error retrieving ARP entry: entry not complete");
		return(-1);
	}

	bcopy(arpreq.arp_ha.sa_data, my_eaddr, sizeof (struct ether_addr));

#elif defined HAVE_GETIFADDRS /* not SIOCGARP */

	struct ifaddrs *ifaddrs_head;
	struct ifaddrs *ifp;
	int found;
	struct sockaddr_dl *sdl;

	if (getifaddrs(&ifaddrs_head) < 0) {
		report(LOG_ERR, "get_myeaddr: getifaddrs(): %s", get_errmsg());
		return(-1);
	}

	/* walk the interfaces addresses until we find an interface with the right name, of type 'link' */
	found = 0;
	for (ifp = ifaddrs_head; ifp && !found ; ifp = ifp->ifa_next) {

		/* We are only interested in interfaces with the name 'ifname' */
		if ((strlen(ifname) == strlen(ifp->ifa_name)) && !strcmp(ifp->ifa_name, ifname)) {

			/* we are only interested in interfaces of type 'link' */
			if (ifp->ifa_addr && (ifp->ifa_addr->sa_family == AF_LINK)) {
				found = 1;
				/* copy the result to my_eaddr */
				sdl = (struct sockaddr_dl *) ifp->ifa_addr;
				bcopy((const void *)LLADDR(sdl), my_eaddr, sizeof (struct ether_addr));

			}
		}
	}

	if (ifaddrs_head)
		freeifaddrs(ifaddrs_head);

	if (!found) {
		report(LOG_ERR, "get_myeaddr: getifaddrs() didn't find a link-layer interface with name %s", ifname);
		return(-1);
	}

#else /* not HAVE_GETIFADDRS */

#error "get_myeaddr: Unable to find a way to determine my ethernet address: SIOCGIFHWADDR, SIOCGARP, an HAVE_GETIFADDRS are all undefined"
	
#endif


	/* the data is now in my_eaddr */
	return(0); /* success */
}
