/* Copyright (c) 2000-2008, The Trustees of Princeton University, All Rights Reserved. */

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


int
init_libnet_context_queue(void)
{
/*  Init the libnet context queue (an unnamed global structure maintained by libnet).
    The queue contains one context for each probe packet flavor we will later write.
    We construct these probe packets here once, as they do not change during run, 
      except if signalled to re-read the configuration file.
	We read the global packet_flavors[] to get a list of all the flavors.
    Return 1 on success, 0 on failure.

    We're called as part of startup.
    We're also called if signalled to re-read the configuration file.
*/
	int i;

	/* Destroy all the libnet contexts (if any), and the context queue (if any). */
	libnet_cq_destroy();

	/* construct a libnet context for each flavor packet */
	for (i = 0; i < NUM_FLAVORS; i++) {

		libnet_t *l = NULL;
		char *libnet_errbuf;
		struct bootp *udp_payload = NULL;

		/* Need a fresh errbuf for each libnet context, 
		   one that will not disappear when we go out of scope.
		*/
		libnet_errbuf = (char *) smalloc(LIBNET_ERRBUF_SIZE, 0);

		/* Initialize libnet context. */
		if ((l = libnet_init(LIBNET_LINK, ifname, libnet_errbuf)) == NULL) {
			report(LOG_ERR, "init_libnet_context_queue: libnet_init: error initializing libnet for interface '%s': %s", ifname, libnet_errbuf);
			return(0);
		}

		/* build the DHCP/BOOTP packet; i.e. the DHCP/BOOTP header and payload */
		if ((udp_payload = build_dhcp_packet(i)) == NULL)
			return(0);

		/* build the UDP, IPv4, and Ethernet headers */
		if (! build_frame(l, udp_payload))
			return(0);

		/* add the current libnet context (a completed packet) to the libnet context queue */
		char label[NUM_FLAVORS_MAXSTRING];
		snprintf(label, sizeof(label)-1, "%d", i);
		if (libnet_cq_add(l, label) == -1) {
			report(LOG_ERR, "init_libnet_context_queue: libnet_cq_add: error adding libnet context '%s' to queue: %s", label, libnet_errbuf);
			return(0);
		}
	}

	return 1; /* success */
}




struct bootp *
build_dhcp_packet(enum dhcp_flavor_t flavor)
{
/* Allocate, build, and return the DHCP/BOOTP packet; i.e. the DHCP/BOOTP header and payload.
   Return pointer to the packet on success.
   On failure we return NULL, and no packet has been allocated.
   Caller must specify which flavor of packet payload we should create.

   We do NOT use libnet_build_dhcpv4() because we want finer-grain control over the construction of
   the packet.

   The packet we return is suitable for inclusion as 'optional payload' in a libnet_build_udp() constructor.
*/

	struct bootp *packet;
	unsigned char *next_vendor_option; /* place to write next vendor option */

	/* XXX I cannot determine whether the contents of the optional payload passed to libnet_build_udp() is COPIED by libnet,
	   or if libnet just keeps a copy of the pointer.  If the former, I should create the buffer using an auto var;
	   if the latter I should malloc the buffer.
	   To be safe, I malloc the buffer.
	   However, note that if libnet actually COPIES the contents of my buffer into its own private buffer,
	   then presumably libnet_destroy() will not free the buffer I malloc'd, so each time we re-read the configuration file,
	   we will leak the buffer I malloc'd.
	*/
	packet = (struct bootp *) smalloc(sizeof(struct bootp), 1);
	next_vendor_option = packet->bootp_options;

	/* XXX: we don't check to ensure 'next_vendor_option' stays inside 'bootp_options' field, since we know
	   that for the packets we build, it'll all fit.  If we needed to, we could use the larger
	   'bootp_options' field allowed by DHCP (but not BootP), but then all our payloads won't be the same
	   size, a simplifying assumption we're making throughout the code.
	*/


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


		/* We prefer to make the 'DHCP Message Type' the first option (after RFC1048 cookie) if possible,
		   because we have seen one (broken) DHCP server that only answered DHCP clients if they specifed this
		   option right after the RFC1048 cookie.
		*/

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

				report(LOG_ERR, "build_dhcp_packet: internal error: invalid packet flavor");
				free(packet);
				return NULL;
				break;

		} /* switch */

		/* add DHCP client id to options field */
		init_option_clientid();
		insert_option(&next_vendor_option, vendor_option_clientid, sizeof(vendor_option_clientid));


	} /* flavor != BOOTP */

	/* add END option to options field */
	insert_option(&next_vendor_option, vendor_option_end, sizeof(vendor_option_end));

	return packet; /* success */
}




int
build_frame(libnet_t *l, struct bootp *udp_payload)
{
/* Build UDP, IPv4, and Ethernet headers. 
   Caller provides libnet context into which we build.
   Caller provides already constructed udp_payload.
   We return true on success, false on failure.
   Note that you must have already init'd the global 'my_eaddr' before calling this!
*/
	u_char ether_bcast_eaddr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	/* build the UDP header, and copy in the UDP payload */
	if (libnet_build_udp(
		PORT_BOOTPC,													/* src_port */
		PORT_BOOTPS,													/* dst_port */
		LIBNET_UDP_H + sizeof(struct bootp),							/* UDP packet len */
		0,																/* UDP checksum, 0 = autofill */
		(u_int8_t *) udp_payload, sizeof(struct bootp),					/* optional payload  (the DHCP/BOOTP packet) */
		l,																/* libnet context */
		0																/* libnet ptag, 0 == create */
		) == -1) {
			report(LOG_ERR, "build_frame: libnet_build_udp failed: %s", libnet_geterror(l));
			return(0);
	}

	/* build the IPv4 header */
	if (libnet_build_ipv4(
		LIBNET_IPV4_H + LIBNET_UDP_H + sizeof(struct bootp),	/* total IP packet length including all subsequent data */
		0,														/* tos */
		1,														/* id */
		0,														/* frag/offset bits */
		60,														/* ttl */
		IPPROTO_UDP,											/* protocol */
		0,														/* IP checksum, 0 = autofill */
		INADDR_ANY,												/* saddr == 0.0.0.0 */
		INADDR_BROADCAST,										/* daddr == 255.255.255.255 */
		NULL, 0, 												/* no optional payload */
		l,														/* libnet context */
		0														/* libnet ptag, 0 == create */
		) == -1) {
			report(LOG_ERR, "build_frame: libnet_build_ipv4 failed: %s", libnet_geterror(l));
			return(0);
	}

	if (use_8021q) {

		/* build the Ethernet 802.1Q header */
		if (libnet_build_802_1q(
			ether_bcast_eaddr,										/* ether_dst */
#ifdef STRUCT_ETHER_ADDR_HAS_ETHER_ADDR_OCTET
			GetEther_src()->ether_addr_octet,						/* ether_src */
#elif defined STRUCT_ETHER_ADDR_HAS_OCTET
			GetEther_src()->octet,									/* ether_src */
#else
#error "struct ether_addr{} has neither an ether_addr_octet nor an octet member, cannot proceed."
#endif
			ETHERTYPE_VLAN,											/* TPI */
			VLAN_PRIORITY,											/* priority (0-7) */
			VLAN_CFI_FLAG,											/* CFI flag */
			vlan_id,												/* VLAN ID  (0-4095) */
			ETHERTYPE_IP,											/* 802.3 len or Ethernet Type II ethertype */
			NULL, 0,												/* no optional payload */
			l,														/* libnet context */
			0														/* libnet ptag, 0 == create */
			) == -1) {
				report(LOG_ERR, "build_frame: libnet_build_802_1q failed: %s", libnet_geterror(l));
				return(0);
		}
	} else {

		/* build the Ethernet header */
		if (libnet_build_ethernet(
			ether_bcast_eaddr,										/* ether_dst */
#ifdef STRUCT_ETHER_ADDR_HAS_ETHER_ADDR_OCTET
			GetEther_src()->ether_addr_octet,						/* ether_src */
#elif defined STRUCT_ETHER_ADDR_HAS_OCTET
			GetEther_src()->octet,									/* ether_src */
#else
#error "struct ether_addr{} has neither an ether_addr_octet nor an octet member, cannot proceed."
#endif
			ETHERTYPE_IP,											/* ethertype */
			NULL, 0,												/* no optional payload */
			l,														/* libnet context */
			0														/* libnet ptag, 0 == create */
			) == -1) {
				report(LOG_ERR, "build_frame: libnet_build_ethernet failed: %s", libnet_geterror(l));
				return(0);
		}
	}

	return 1; /* success */
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
