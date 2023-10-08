/* 
 * get_myipaddr.c: get my own IP address (for a specified interface)
 *
 * Based on get_ifi_info(), beginning on page 429 of "UNIX Network
 * Programming: Volume 1", Second Edition, by W. Richard Stevens.
 *
 * Modified to support platforms needing SYS_SOCKET_IOCTLS_USE_STREAMS.
 *
 * Corresponding examples for the Third Edition, published at GitHub
 * <https://github.com/unpbook/unpv13e>, provide the following insight
 * into the license and rights to distribute of this particular code:
 *
 *     The code is available to anyone on the Internet and should
 *     compile easily on most current Unix systems.  The majority of
 *     the 10,000 lines of C code are functions that one can use as
 *     building blocks (a network programming toolchest) inside their
 *     own network applications.  Many of these functions help hide
 *     the differences between IPv4 and IPv6, and can aid the reader
 *     in developing protocol-independent code.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h> /* struct in_addr */
#include <arpa/inet.h> /* inet_ntoa() */

#include <errno.h>

#include "defs.h"

#ifdef HAVE_STROPTS_H
#include <stropts.h>
#endif

#include "get_myipaddr.h"
#include "report.h"
#include "utils.h"

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

int
get_myipaddr(int sockfd, char *ifname, struct in_addr *my_ipaddr)
{
/* Given my interface named 'ifname', determine the corresponding IP addr, store it in 'my_ipaddr'.
   If the interface has multiple IP addrs, we only return the first one.
   Needs a dgram sockfd for temp use.
   Return 0 on success, <0 on failure.
*/

	int len, lastlen, rc;
	char *ptr, *buf;
	struct sockaddr_in *sip;
	struct ifreq *ifr;
	struct ifconf ifconf;				/* holds list of interfaces as returned by ioctl SIOCGIFCONF  */
#ifdef SYS_SOCKET_IOCTLS_USE_STREAMS
	/* On systems that don't support SIOCxxx ioctls on socket descriptors, we will issue
	   I_STR ioctls instead.  We use ioc to hold args/result, and copy any values we need 
       out of it to where we would expect to find them. */
	struct strioctl ioc;
#endif

	/* issue SIOCGIFCONF to get buf populated with all the struct ifreq's  */
	/* Since we don't know how many interfaces there are, we allocate some space for the buffer,
	   then issue the request in a loop, stopping when we have a succesful return and the returned
	   data length is the same as the previous attempt.  */
	lastlen = 0;
	len = 100 * sizeof(struct ifreq);	/* initial buffer size guess */
	for ( ; ; ) {
		buf = smalloc(len, 1);
#ifdef SYS_SOCKET_IOCTLS_USE_STREAMS
		/* this streams code hasn't been tested */
		ioc.ic_cmd = SIOCGIFCONF;
		ioc.ic_timout = 0;
		ioc.ic_len = len;
		ioc.ic_dp = (char *) buf;
		rc = ioctl(sockfd, I_STR, (char *) &ioc);
		ifconf.ifc_len = ioc.ic_len;
		ifconf.ifc_req = (void *) ioc.ic_dp;
#else /* not SYS_SOCKET_IOCTLS_USE_STREAMS */
		ifconf.ifc_len = len;
		ifconf.ifc_buf = buf;
		rc = ioctl(sockfd, SIOCGIFCONF, (void *)&ifconf);
#endif /* not SYS_SOCKET_IOCTLS_USE_STREAMS */
		if (rc < 0) {
			/* EINVAL is an expected error return on some systems, indicating a buffer too small; 
			   any other error return is presumably serious.  Something's also wrong if we get an
			   error return *after* a previous succesful call (which last lastlen>0). */
			if (errno != EINVAL || lastlen != 0) {
				report(LOG_ERR, "get_myipaddr: ioctl(SIOCGIFCONF): %s", get_errmsg());
				report(LOG_NOTICE, "exiting");
				exit(1);
			} /* else we know buffer was too small, so we'll just try again with larger buffer */
		} else { 
			/* no error return, but some systems just truncate the data and return 0.  
			   So we're only done if we got *something* back and 
			   we've previousy gotten the same datalength with a smaller buffer. */
			if (ifconf.ifc_len &&(ifconf.ifc_len == lastlen))
				break;		/* success, len has not changed */
			lastlen = ifconf.ifc_len;
		}
		len += 10 * sizeof(struct ifreq);		/* increment for next try */
		free(buf);
	}

	for (ptr = buf; ptr < buf + ifconf.ifc_len; ) {	/* walk through the struct ifreq's in buf */
													/* increment of ptr is done separately below */
		ifr = (struct ifreq *) ptr;
	
		/* XXX As each struct ifreq in the buffer lacks a 'next' pointer or a 'length' member,
		   and the size of each struct ifreq may vary, determining the length of the current struct ifreq
		   (so we know how much to increment ptr to reach the next struct ifreq) is problematic.

		   We should avoid using 'sizeof (struct ifreq)'; in some cases on some implementations, that's too small.

		   Our approach to compute 'len' is far from perfect; it can go awry.
		*/

		/* Assume that the struct ifreq contains a struct ifr->ifr_name member, and that this
		   member has a fixed length known at compile-time.

		   Assume that all the other element(s) of struct ifreq will add up to the size of
		   some sort of socket address structure.
		   (The reason for guessing this is that typically a struct ifreq contains just one member other than
		   the ifr->ifr_name member, and that member is declared as a union of various things, the largest
		   of which is probably a struct sockaddr_in6 or struct sockaddr.)

		   Assume that the struct ifreq contains no padding.
		*/
#ifdef STRUCT_SOCKADDR_HAS_SA_LEN
		len = max(ifr->ifr_addr.sa_len, sizeof(struct sockaddr)) + sizeof(ifr->ifr_name);

#else
		switch (ifr->ifr_addr.sa_family) {
#ifdef AF_INET6
			case AF_INET6:
				len = sizeof(struct sockaddr_in6) + sizeof(ifr->ifr_name);
				break;
#endif /* AF_INET6 */
			case AF_INET:
			default:
				len = sizeof(struct sockaddr) + sizeof(ifr->ifr_name);
		}
#endif /* not STRUCT_SOCKADDR_HAS_SA_LEN */

		/* If 'len' came out smaller than the size of a struct ifreq, it's likely incorrect;
		   increase it to the struct ifreq size, and hope for the best. */
		len = max(len, sizeof(*ifr));
			
		/* Increment ptr to next struct ifreq for next time through the loop.
		   This can go badly wrong, as noted above. */
		ptr += len;

		if ((strlen(ifname) != strlen(ifr->ifr_name)) || strcmp(ifname, ifr->ifr_name)) {
			/* This is NOT the interface we're looking for. */
			continue; /* for ptr ... */
		}

		/* this IS the interface we're looking for */

		/* get my IP address */
#ifdef SYS_SOCKET_IOCTLS_USE_STREAMS
		/* this streams code hasn't been tested */
		ioc.ic_cmd = SIOCGIFADDR;
		ioc.ic_timout = 0;
		ioc.ic_len = sizeof(ifr);
		ioc.ic_dp = (char *) &ifr;
		rc = ioctl(sockfd, I_STR, (char *) &ioc);
#else /* not SYS_SOCKET_IOCTLS_USE_STREAMS */
		rc = ioctl(sockfd, SIOCGIFADDR,  (void *) ifr);
#endif /* not SYS_SOCKET_IOCTLS_USE_STREAMS */
		if (rc < 0) {
			report(LOG_ERR, "get_myipaddr: ioctl(SIOCGIFADDR): %s", get_errmsg());
			return(-1);
		}

		sip = (struct sockaddr_in *) &ifr->ifr_addr;
		bcopy((char *) &sip->sin_addr, (char *) &(my_ipaddr->s_addr), sizeof(struct in_addr));

		free(buf); /* we're done with ifconf */
		return(0); /* success */

		/* go on to next interface */
	}

	/* only reached when we failed to locate 'ifname' in list of interfaces */
	report(LOG_ERR, "get_myipaddr: couldn't locate interface %s", ifname);
	free(buf);	 /* we're done with ifconf */
	return(-1); /* failure */
}

