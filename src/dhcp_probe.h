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


extern int sockfd; /* general purpose datagram socket fd for temp use throughout */

extern struct ether_addr my_eaddr;




/* What flavor of DHCP or BootP packet to send? */
enum dhcp_flavor_t {
	BOOTP = 0,          /* send BOOTPREQUEST */
	DHCP_INIT,          /* send DHCPDISCOVER */
	DHCP_SELECTING,     /* send DHCPREQUEST, client in SELECTING state */
	DHCP_INIT_REBOOT,   /* send DHCPREQUEST, client in INIT-REBOOT state */
	DHCP_REBINDING      /* send DHCPREQUEST, client in REBINDING state */
};


/* An array listing all the valid packet flavors */
extern enum dhcp_flavor_t packet_flavors[];

/* An array containing ptrs to each of the packet frame flavors we may write */
extern unsigned char * write_packets[];

#define NUM_FLAVORS (sizeof(packet_flavors) / sizeof(packet_flavors[0]))



/* forward decls for functions */
void process_response(u_char *user, const struct pcap_pkthdr *pkthdr, const u_char *packet);
void reconfigure(const int write_packet_len);
void catcher(int signal);
void cleanup(void);
void usage(void);
void close_and_reopen_capture_file(void);



enum { MAX_ETHER_ADDR_STR = 18 };
enum { MAX_IP_ADDR_STR = 18 };


#endif /* not DCHPPROBE_H */
