/* 
 * get_myeaddr.c: get my own ethernet address (for a specified IP address)
 * Based on prmac.c on page 442 of
 * "UNIX Network Programming: Volume 1", Second Edition,
 * by W. Richard Stevens.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <strings.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h> /* struct in_addr */

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

#include "get_myeaddr.h"
#include "report.h"


int
get_myeaddr(int sockfd, struct in_addr *my_ipaddr, struct ether_addr *my_eaddr)
{
/* Given my IP address 'my_ipaddr', determine the corresponding Ethernet addr, store it in 'my_eaddr'.
   Needs a dgram sockfd for temp use.
   Return 0 on success, <0 on failure.
*/

	struct arpreq arpreq;
	struct sockaddr_in *sin;
	int rc;
#ifdef SYS_SOCKET_IOCTLS_USE_STREAMS
	/* On systems that don't support SIOCxxx ioctls on socket descriptors, we will issue
	   I_STR ioctls instead.  We use ioc to hold args/result, and copy any values we need
	   out of it to where we would expect to find them. */
	int fd;
	struct strioctl iocb;
#endif


	/* get access to the protocol address member of arpreq, treated as a sockaddr_in */
	sin = (struct sockaddr_in *) &arpreq.arp_pa;

	/* fill in sin with the IP address we are searching for */
	bzero(sin, sizeof(struct sockaddr_in));
	sin->sin_family = AF_INET;
	bcopy(my_ipaddr, &sin->sin_addr, sizeof(struct in_addr));

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
		report(LOG_ERR, "ioctl(SIOCGARP): %s", get_errmsg());
		return(-1);
	}

	/* it *should* be complete since it's our own, but let's be sure */
	if (! (arpreq.arp_flags & ATF_COM)) {
		report(LOG_ERR, "get_myeaddr: error retrieving ARP entry: entry not complete");
		return(-1);
	}

	bcopy(arpreq.arp_ha.sa_data, my_eaddr, sizeof (struct ether_addr));
	return(0); /* success */
}
