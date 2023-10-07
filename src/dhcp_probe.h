#ifndef DHCPPROBE_H
#define DHCPPROBE_H

#include "defaults.h"


/* options, etc */
extern char *prog;
extern char *logfile_name;
extern char *pid_file;
extern int debug;
extern int dont_fork;
extern char *capture_file;
extern int snaplen;
extern char *ifname;
extern int use_8021q;
extern int vlan_id;

extern int sockfd; /* general purpose datagram socket fd for temp use throughout */

extern struct ether_addr my_eaddr;



/* What flavor of DHCP or BootP packet to send? */
/* We construct the packets and insert them into a libnet context queue in the order they appear below.
   When we ask libnet to walk the queue in order, it seems to return them in reverse order, as if it were a LIFO stack.
   So we list the flavors below in the reverse of the order we actually wish to send them.
*/
enum dhcp_flavor_t {
	DHCP_REBINDING = 0, /* send DHCPREQUEST, client in REBINDING state */
	DHCP_INIT_REBOOT,   /* send DHCPREQUEST, client in INIT-REBOOT state */
	DHCP_SELECTING,     /* send DHCPREQUEST, client in SELECTING state */
	BOOTP,              /* send BOOTPREQUEST */
	DHCP_INIT           /* send DHCPDISCOVER */
};
#define NUM_FLAVORS 5

/* A width at least large enough for a string that will contain the string version of the number NUM_FLAVORS */
#define NUM_FLAVORS_MAXSTRING (NUM_FLAVORS + 2)

/* An array listing all the valid packet flavors */
extern enum dhcp_flavor_t packet_flavors[];


/* forward decls for functions */
void process_response(u_char *user, const struct pcap_pkthdr *pkthdr, const u_char *packet);
void set_pcap_timeout(pcap_t *pc);
void reconfigure(const int write_packet_len);
void catcher(int signal);
void cleanup(void);
void my_exit(int exit_status, int do_cleanup, int do_log);
void usage(void);
void close_and_reopen_capture_file(void);



enum { MAX_ETHER_ADDR_STR = 18 };
enum { MAX_IP_ADDR_STR = 18 };

#define VLAN_ID_MIN 0
#define VLAN_ID_MAX 4095
#define VLAN_PRIORITY 0x0
#define VLAN_CFI_FLAG 0x0


#endif /* not DCHPPROBE_H */
