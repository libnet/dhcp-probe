/* Copyright (c) 2000-2002, The Trustees of Princeton University, All Rights Reserved. */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "defs.h"

#include "defaults.h"
#include "dhcp_probe.h"
#include "bootp.h"
#include "configfile.h"
#include "report.h"




/* declare, and in some cases initialize, vendor option arrays */
unsigned char vendor_option_vm_cookie_rfc1048[] = VENDOR_OPTION_VM_COOKIE_RFC1048;
unsigned char vendor_option_end[] = VENDOR_OPTION_END;
unsigned char vendor_option_dhcpmessagetype_dhcpdiscover[] = VENDOR_OPTION_DHCPDISCOVER;
unsigned char vendor_option_dhcpmessagetype_dhcprequest[]  = VENDOR_OPTION_DHCPREQUEST;
unsigned char vendor_option_clientid[1 + 1 + 1 + sizeof(struct ether_addr)]; /* option code, length byte,  htype byte, ether_addr */
unsigned char vendor_option_serverid[1 + 1 + 4]; /* option code, length byte, ip_addr */
unsigned char vendor_option_requestedipaddr[1 + 1 + 4]; /* option code, length byte, ip_addr */


void
init_all_frames(int num_flavors, int write_packet_len)
{
/*  Init the global write_packets[] with ptrs to a packet of each flavor.
	We read the global packet_flavors[] to get a list of all the flavors; we populate write_packets[]
	   with packet ptrs so they are parallel arrays.
	The actual memory for the packet frames is allocated here, via libnet_init_packet().
	You are responsible for freeing each when you are done with them, using libnet_destroy_packet().
	Arg 'write_packet_len' is the length of the entire frame, the same for all packet flavors.
*/
	int i;
	struct bootp packet_payload;	/* temp space to hold payload for current flavor */

	/* construct all the flavors of packets we may send */
	for (i = 0; i < num_flavors; i++) {

		/* build payload for this flavor packet */
		build_dhcp_payload(packet_flavors[i], &packet_payload);

		/* allocate buffer for entire frame, store ptr to allocated buffer in write_packets[i] */
		if (libnet_init_packet(write_packet_len, &write_packets[i]) == -1) {
			report(LOG_ERR, "libnet_init_packet failed");
			cleanup();
			exit(1);
		}

		/* insert the payload we built into UDP/IP, frame it with Ethernet, use *write_packets[i] to hold result */
		build_frame(&packet_payload, write_packets[i]);
	}

	return;
}




int
build_dhcp_payload(enum dhcp_flavor_t flavor, struct bootp *packet)
{
/* Build payload for a DHCP (or BootP) packet.
   Return true on success, false on failure.
   Caller must provide the bootp packet buffer into which we'll write.
   Caller must specify which flavor of packet we should create.
*/
	unsigned char *next_vendor_option; /* place to write next vendor option */

	/* XXX compiler produces warning because:
	   next_vendor_option is a pointer to unsigned char, but
	   &packet->bootp_options is a pointer to an array[64] of char
	   If in the assignment statement below, I cast the latter to a pointer to unsigned char (to eliminate the compiler warning),
	   the compiler appears to **alias** the two pointers to each other, breaking everything.  Huh?
	*/
	next_vendor_option = &packet->bootp_options;

	/* XXX: we don't check to ensure 'next_vendor_option' stays inside 'options' field, since we know
	   that for the packets we build below, it'll all fit.  If we needed to, we could use the larger
	   'options' field allowed by DHCP (but not BootP), but then all our payloads won't be the same
	   size, a simplifying assumption we're making throughout the code.
	*/

	bzero(packet, sizeof(struct bootp));

	/* set some fields to fixed values (for most fields, the '0' we just copied in is appropriate) */
	packet->bootp_htype = HTYPE_ETHER;
	packet->bootp_hlen = HLEN_ETHER;
	packet->bootp_xid = BOOTP_XID;
	packet->bootp_op = BOOTREQUEST;
	bcopy(GetChaddr(), &packet->bootp_chaddr, sizeof(struct ether_addr));

	/* add RFC1048 cookie to options field */
	insert_option(&next_vendor_option, vendor_option_vm_cookie_rfc1048, sizeof(vendor_option_vm_cookie_rfc1048));

	if (flavor != BOOTP) {
		struct in_addr * ciaddr; /* temp value of Client IP Address, if needed */

		/* add DHCP client id to options field */
		init_option_clientid();
		insert_option(&next_vendor_option, vendor_option_clientid, sizeof(vendor_option_clientid));

		switch (flavor) {

			case DHCP_INIT:

				/* add DHCP Message Type to options field, set to DHCPDISCOVER */
				insert_option(&next_vendor_option, vendor_option_dhcpmessagetype_dhcpdiscover, sizeof(vendor_option_dhcpmessagetype_dhcpdiscover));

				break;

			case DHCP_SELECTING:

				/* add DHCP Message Type to options field, set to DHCPREQUEST */
				insert_option(&next_vendor_option, vendor_option_dhcpmessagetype_dhcprequest, sizeof(vendor_option_dhcpmessagetype_dhcprequest));

				/* add DHCP Server Identifier to options field, set to specified bogus value */
				init_option_serverid();
				insert_option(&next_vendor_option, vendor_option_serverid, sizeof(vendor_option_serverid));

				/* add DHCP Requested IP Address to options field, set to specified bogus value */
				init_option_requestedipaddr();
				insert_option(&next_vendor_option, vendor_option_requestedipaddr, sizeof(vendor_option_requestedipaddr));

				break;

			case DHCP_INIT_REBOOT:

				/* add DHCP Message Type to options field, set to DHCPREQUEST */
				insert_option(&next_vendor_option, vendor_option_dhcpmessagetype_dhcprequest, sizeof(vendor_option_dhcpmessagetype_dhcprequest));

				/* add DHCP Requested IP Address to options field, set to specified bogus value */
				init_option_requestedipaddr();
				insert_option(&next_vendor_option, vendor_option_requestedipaddr, sizeof(vendor_option_requestedipaddr));

				break;

			case DHCP_REBINDING:

				/* add DHCP Message Type to options field, set to DHCPREQUEST */
				insert_option(&next_vendor_option, vendor_option_dhcpmessagetype_dhcprequest, sizeof(vendor_option_dhcpmessagetype_dhcprequest));

				/* set ciaddr to specified bogus value */
				ciaddr = GetClient_ip_address();
				packet->bootp_ciaddr.s_addr = ciaddr->s_addr;

				break;

			default:

				report(LOG_ERR, "build_dhcp_payload: internal error: invalid packet flavor");
				return 0;
				break;

		} /* switch */

	} /* flavor != BOOTP */


	/* add END option to options field */
	insert_option(&next_vendor_option, vendor_option_end, sizeof(vendor_option_end));

	return 1;
}




int
build_frame(struct bootp *bootp_payload, unsigned char * write_packet)
{
/* Caller provides already constructed 'bootp_payload', and a buffer 'write_packet'
   in which to hold the completed frame.
   We fill in 'write_frame'.
   Note that you must have already init'd the global 'my_eaddr' before calling this!
*/
	u_char ether_bcast_eaddr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	/* build the ethernet header */
	if (libnet_build_ethernet(
		ether_bcast_eaddr,					/* ether_dst */
		GetEther_src()->ether_addr_octet,	/* ether_src */
		ETHERTYPE_IP,						/* ethertype */
		NULL, 0,							/* no optional payload */
		write_packet						/* start of ethernet header in packet */
		) < 0) {
			report(LOG_ERR, "libnet_build_ethernet: error");
			cleanup();
			exit(1);
	}

	/* build the IP header */
	if (libnet_build_ip(
		LIBNET_UDP_H + sizeof(struct bootp),	/* IP packet len (not including IP header) */
		0,										/* tos */
		1,										/* id */
		0,										/* frag/offset bits */
		60,										/* ttl */
		IPPROTO_UDP,							/* protocol */
		INADDR_ANY,								/* saddr == 0.0.0.0 */
		INADDR_BROADCAST,						/* daddr == 255.255.255.255 */
		NULL, 0, 								/* no optional payload */
		write_packet + LIBNET_ETH_H				/* start of ip header in packet */
		) < 0) {
			report(LOG_ERR, "libnet_build_ip: error");
			cleanup();
			exit(1);
	}

	/* build the UDP header, and copy in the udp payload */
	if (libnet_build_udp(
		PORT_BOOTPC,													/* src_port */
		PORT_BOOTPS,													/* dst_port */
		(u_char *) bootp_payload, sizeof(*bootp_payload),				/* optional payload */
		write_packet + LIBNET_ETH_H + LIBNET_IP_H						/* start of udp header in packet */
		) < 0) {
			report(LOG_ERR, "libnet_build_udp: error");
			cleanup();
			exit(1);
	}

	/* compute and insert the IP checksum */
	if (libnet_do_checksum(
		write_packet + LIBNET_ETH_H,			/* start of IP header */
		IPPROTO_IP,								/* compute and insert checksum for this transport protocol */
		LIBNET_IP_H  							/* IP header length */
	) < 0) {
		report(LOG_ERR, "libnet_do_checksum: error");
		cleanup();
		exit(1);
	}

	/* compute and insert the UDP checksum */
	if (libnet_do_checksum(
		write_packet + LIBNET_ETH_H,			/* start of IP header */
		IPPROTO_UDP,							/* compute and insert checksum for this transport protocol */
		LIBNET_UDP_H + sizeof(struct bootp)		/* IP packet length (not included IP header) */
	) < 0) {
		report(LOG_ERR, "libnet_do_checksum: error");
		cleanup();
		exit(1);
	}

	return 1;
}


void
init_option_clientid(void)
{
/* Init 'vendor_option_clientid' */
	/* option code */
	vendor_option_clientid[0] = VENDOR_OPTION_CLIENTID;

	/* length byte */
	vendor_option_clientid[1] = 1 + sizeof(struct ether_addr);

	/* fill in clientid value: htype code followed by enet addr */
	vendor_option_clientid[2] = 0x01;
	bcopy(GetChaddr(), vendor_option_clientid+3, sizeof(struct ether_addr));

	return;
}


void
init_option_serverid(void)
{
/* Init 'vendor_option_serverid' */
	struct in_addr *server_id;

	/* option code */
	vendor_option_serverid[0] = VENDOR_OPTION_SERVERID;

	/* length byte */
	vendor_option_serverid[1] = sizeof(server_id->s_addr);

	server_id = GetServer_id();

	bcopy(&server_id->s_addr, vendor_option_serverid+2, sizeof(server_id->s_addr));

	return;
}


void
init_option_requestedipaddr(void)
{
/* Init 'vendor_option_requestedipaddr' */
	struct in_addr *requestedipaddr;

	/* option code */
	vendor_option_requestedipaddr[0] = VENDOR_OPTION_REQUESTED_IP_ADDRESS;

	/* length byte */
	vendor_option_requestedipaddr[1] = sizeof(requestedipaddr->s_addr);

	requestedipaddr = GetClient_ip_address();

	bcopy(&requestedipaddr->s_addr, vendor_option_requestedipaddr+2, sizeof(requestedipaddr->s_addr));

	return;
}



void
insert_option(unsigned char **destination, unsigned char *option, int len)
{
/* Copy an option value (sequence of bytes) of length 'len' to *destination, then
   advance the destination ptr by that length
*/
	bcopy(option, *destination, len);
	*destination += len;

	return;
}
