#ifndef BOOTP_H

#define BOOTP_CHADDR_LEN 16
#define BOOTP_SNAME_LEN 64
#define BOOTP_FILE_LEN 128
#define BOOTP_OPTIONS_LEN 64

/* A bootp packet */
struct bootp {
	uint8_t			bootp_op;							/* opcode */
	uint8_t			bootp_htype;						/* hardware address type */
	uint8_t			bootp_hlen;							/* hardware address length */
	uint8_t			bootp_hops;							/* hop count */
	uint32_t		bootp_xid;							/* transaction ID */
	uint16_t		bootp_secs;							/* seconds elapsed since boot started */
	uint16_t		bootp_flags;						/* bit flags */
	struct in_addr	bootp_ciaddr;						/* client IP addr */
	struct in_addr	bootp_yiaddr;						/* your IP addr */
	struct in_addr	bootp_siaddr;						/* next server IP addr */
	struct in_addr	bootp_giaddr;						/* relay agent IP addr */
	unsigned char	bootp_chaddr[BOOTP_CHADDR_LEN];		/* client hardware address */
	char			bootp_sname[BOOTP_SNAME_LEN];		/* server host name */
	char			bootp_file[BOOTP_FILE_LEN];			/* boot file name */
	unsigned char	bootp_options[BOOTP_OPTIONS_LEN];	/* options */
};

/* op codes */
#define BOOTREQUEST 1

/* DHCP message types */
#define DHCPDISCOVER 1
#define DHCPREQUEST  3

/* htype values */
#define HTYPE_ETHER 1

/* hlen values */
#define HLEN_ETHER 6

#define PORT_BOOTPS       67
#define PORT_BOOTPC       68


/* various codes for the 'options' field */
#define VENDOR_OPTION_VM_COOKIE_RFC1048   { 99, 130, 83, 99 }

#define VENDOR_OPTION_END { 255 }

/* the DHCP Message Type option consists of the option code, a length byte == 1, and message type code */
#define VENDOR_OPTION_DHCPMESSAGETYPE 53
#define VENDOR_OPTION_DHCPMESSAGETYPE_DHCPDISCOVER 1
#define VENDOR_OPTION_DHCPMESSAGETYPE_DHCPREQUEST  3
#define VENDOR_OPTION_DHCPDISCOVER { VENDOR_OPTION_DHCPMESSAGETYPE, 1, VENDOR_OPTION_DHCPMESSAGETYPE_DHCPDISCOVER }
#define VENDOR_OPTION_DHCPREQUEST  { VENDOR_OPTION_DHCPMESSAGETYPE, 1, VENDOR_OPTION_DHCPMESSAGETYPE_DHCPREQUEST  }

#define VENDOR_OPTION_REQUESTED_IP_ADDRESS 50

#define VENDOR_OPTION_SERVERID 54

#define VENDOR_OPTION_CLIENTID 61


int init_libnet_context_queue(void);
struct bootp *build_dhcp_packet(enum dhcp_flavor_t flavor);
int build_frame(libnet_t *l, struct bootp *udp_payload);
void insert_option(unsigned char **destination, unsigned char *option, int len);
void init_option_clientid(void);
void init_option_serverid(void);
void init_option_requestedipaddr(void);



#endif /* not BOOTP_H */
