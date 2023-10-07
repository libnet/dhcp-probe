/* dhcp_probe:

	Broadcast BOOTPREQUEST, DHCPDISCOVER, and DHCPREQUEST packets out specified interfaces,
	listen for answers, discard those from known "good" servers, log the others.
	The intent is to provide a way to find rogue BootP and DHCP servers.
	This will only find rogue servers that happen to answer us; rogue servers configured to
	only answer a selected set of clients will not be discovered.
*/

/* Copyright (c) 2000-2008, The Trustees of Princeton University, All Rights Reserved. */


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "defs.h"
#include "defaults.h"

#include "dhcp_probe.h"
#include "bootp.h"
#include "daemonize.h"
#include "get_myeaddr.h"
#include "get_myipaddr.h"
#include "configfile.h"
#include "report.h"
#include "utils.h"

#ifndef lint
static const char rcsid[] = "dhcp_probe version " VERSION;
static const char copyright[] = "Copyright 2000-2008, The Trustees of Princeton University.  All rights reserved.";
static const char contact[] = "networking at princeton dot edu";
#endif

/* initialize options to defaults */
int debug = 0;
int dont_fork = 0;
char *config_file = CONFIG_FILE;
char *pid_file = PID_FILE;
char *capture_file = NULL;
/* Init snaplen to the max number of bytes we might need to capture in response to a single packet we send.
   This needs to include the complete size of the ethernet frame.
   Of course, we can't really know this ahead of time; who knows how many servers out there might
   answer us, and how large their responses might be?
   The simplest approach is to just overestimate generously.  Although a normal reply is under 600 bytes,
   nothing prevents someone from sending maximum-size Ethernet frames (1514 bytes) as responses.  
   So if you want to be prepared to handle 20 responses to a single packet, you would set snaplen to 20*1514.
   Note than since pcap_open_live() declares this an 'int', don't specify a value larger than that.
*/
int snaplen = CAPTURE_BUFSIZE;
int socket_receive_timeout_feature = 0;

char *prog = NULL;
char *logfile_name = NULL;

int sockfd;

volatile sig_atomic_t reread_config_file; /* for signal handler */
volatile sig_atomic_t reopen_log_file; /* for signal handler */
volatile sig_atomic_t reopen_capture_file; /* for signal handler */
volatile sig_atomic_t quit_requested; /* for signal requested */

pcap_t *pd = NULL;					/* libpcap - packet capture descriptor used for actual packet capture */
pcap_t *pd_template = NULL;			/* libpcap - packet capture descriptor just used as template */

pcap_dumper_t *pcap_dump_d = NULL;	/* libpcap - dump descriptor */

/* An array listing all the valid packet flavors we may write */
enum dhcp_flavor_t packet_flavors[] = {BOOTP, DHCP_INIT, DHCP_SELECTING, DHCP_INIT_REBOOT, DHCP_REBINDING};

char *ifname;
struct ether_addr my_eaddr;

int use_8021q = 0;
int vlan_id = 0;

int 
main(int argc, char **argv)
{
	int c, errflag=0;
	struct in_addr my_ipaddr;
	extern char *optarg;
	extern int optind, opterr, optopt;
	struct sigaction sa;
	FILE *pid_fp;
	char *cwd = CWD;
	int i;

	int write_packet_len;
	int bytes_written;

	unsigned int time_to_sleep;
	sigset_t new_sigset, old_sigset;

	/* for libpcap */
	bpf_u_int32 netnumber,  netmask;
	struct bpf_program bpf_code;
	int linktype;
	char pcap_errbuf[PCAP_ERRBUF_SIZE], pcap_errbuf2[PCAP_ERRBUF_SIZE];

	/* for libnet */
	char libnet_errbuf[LIBNET_ERRBUF_SIZE];

	/* get progname = last component of argv[0] */
	prog = strrchr(argv[0], '/');
	if (prog)
		prog++;
	else 
		prog = argv[0];

	while ((c = getopt(argc, argv, "c:d:fhl:o:p:Q:s:Tvw:")) != EOF) {
		switch (c) {
			case 'c':
				if (optarg[0] != '/') {
					fprintf(stderr, "%s: invalid config file '%s', must be an absolute pathname\n", prog, optarg);
					errflag++;
					break;
				}
				config_file = optarg;
				break;
			case 'd': {
				char *stmp = optarg;
				if ((sscanf(stmp, "%d", &debug) != 1) || (debug < 0)) {
					fprintf(stderr, "%s: invalid debug level '%s'\n", prog, optarg);
					debug = 0;
					errflag++;
				}
				break;
			}
			case 'f':
				dont_fork = 1;
				break;
			case 'h':
				usage();
				my_exit(0, 0, 0);
			case 'l':
				if (optarg[0] != '/') {
					fprintf(stderr, "%s: invalid log file '%s', must be an absolute pathname\n", prog, optarg);
					errflag++;
					break;
				}
				logfile_name = optarg;
				break;
			case 'o':
				if (optarg[0] != '/') {
					fprintf(stderr, "%s: invalid capture file '%s', must be an absolute pathname\n", prog, optarg);
					errflag++;
					break;
				}
				capture_file = optarg;
				break;
			case 'p':
				if (optarg[0] != '/') {
					fprintf(stderr, "%s: invalid pid file '%s', must be an absolute pathname\n", prog, optarg);
					errflag++;
					break;
				}
				pid_file = optarg;
				break;
			case 'Q': {
				char *stmp = optarg;
				if ((sscanf(stmp, "%d", &vlan_id) != 1) || (vlan_id < VLAN_ID_MIN) || (vlan_id > VLAN_ID_MAX)) {
					fprintf(stderr, "%s: invalid vlan ID '%s', must be integer %d ... %d\n", prog, optarg, VLAN_ID_MIN, VLAN_ID_MAX);
					errflag++;
				} else {
					use_8021q++;
				}
				break;
			}
			case 's': {
				char *stmp = optarg;
				/* XXX sscanf() silently forces to integer range.  If you specify a value outside
				   the range, and the conversion results in a positive value within the range, we
				   will silently use the converted value.
				*/
				if ((sscanf(stmp, "%d", &snaplen) != 1) || (snaplen < 1)) {
					fprintf(stderr, "%s: invalid capture buffer size '%s'\n", prog, optarg);
					snaplen = CAPTURE_BUFSIZE;
					errflag++;
				}
				break;
			}
			case 'T':
				socket_receive_timeout_feature = 1;
				break;
			case 'v':
				printf("DHCP Probe version %s\n", VERSION);
				my_exit(0, 0, 0);
			case 'w':
				if (optarg[0] != '/') {
					fprintf(stderr, "%s: invalid working directory '%s', must be an absolute pathname\n", prog, optarg);
					errflag++;
					break;
				}
				cwd = optarg;
				break;
			case '?':
				usage();
				my_exit(0, 0, 0);
			default:
				errflag++;
				break;
		}
	}
	if (optind == argc || errflag) {
		usage();
		my_exit(1, 0, 1);
	}

	if (! dont_fork)
		daemonize();

	/* initialize logging */
	report_init(dont_fork, logfile_name);

	if (chdir(cwd) < 0) {
		report(LOG_ERR, "chdir(%s): %s", cwd, get_errmsg());
		my_exit(1, 0, 1);
	}

	report(LOG_NOTICE, "starting, version %s", VERSION);

	/* Before writing our pid, prepare to respond reasonably if we get any of our supported signals.
			SIGHUP,SIGUSR1,SIGUSR2 - ignore until our internal data structs are ready for it
			SIGINT,SIGTERM,SIGQUIT - leave default for now, so it will still kill us, but not try to look at uninit'd pcap structs
	*/
	if (dont_fork) { /* we didn't daemonize earlier */
		/* ignore SIGHUP */
		sigemptyset(&sa.sa_mask);
		sa.sa_handler = SIG_IGN;
		if (sigaction(SIGHUP, &sa, NULL) < 0) {
			report(LOG_ERR, "sigaction: %s", get_errmsg());
			my_exit(1, 0, 1);
		}
	} /* else we already set SIGHUP to ignore while daemonizing, so we don't need to do it again */
	/* ignore SIGUSR1 */
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = SIG_IGN;
	if (sigaction(SIGUSR1, &sa, NULL) < 0) {
		report(LOG_ERR, "sigaction: %s", get_errmsg());
		my_exit(1, 0, 1);
	}
	/* ignore SIGUSR2 */
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = SIG_IGN;
	if (sigaction(SIGUSR2, &sa, NULL) < 0) {
		report(LOG_ERR, "sigaction: %s", get_errmsg());
		my_exit(1, 0, 1);
	}


	/* write pid file as soon as possible after (possibly) forking */
	if ((pid_fp = open_for_writing(pid_file)) == NULL) {
		report(LOG_ERR, "could not open pid file %s for writing", pid_file);
		my_exit(1, 0, 1);
	} else {
		fprintf(pid_fp, "%d\n", (int) getpid());
		fclose(pid_fp);
	}

	if (! read_configfile(config_file)) {
		my_exit(1, 1, 1);
	}

	reread_config_file = 0; /* set by signal handler */
	reopen_log_file = 0; /* set by signal handler */
	reopen_capture_file = 0; /* set by signal handler */
	
	ifname = strdup(argv[optind]); /* interface name is a required final argument */

	/* general purpose dgram socket for various uses */
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		report(LOG_ERR, "socket(): %s", get_errmsg());
		my_exit(1, 1, 1);
	}

	/* lookup netnumber and netmask for specified interface */
	if (pcap_lookupnet(ifname, &netnumber, &netmask, pcap_errbuf) < 0) {
		report(LOG_ERR, "%s: bad interface '%s': %s", prog, ifname, pcap_errbuf);
		my_exit(1, 1, 1);
	}

	/* We need to know the Ethernet address for the named interface, but don't have a direct
	   way to look that up.
	   So we go the roundabout route of looking up the (first) IP address associated with that interface,
	   then looking in our ARP cache for this IP address to see the associated ethernet address. */

	/* lookup IP address for specified interface */
	if (get_myipaddr(sockfd, ifname, &my_ipaddr) < 0) {
		report(LOG_ERR, "couldn't determine IP addr for interface %s", ifname);
		my_exit(1, 1, 1);
	}

	/* lookup ethernet address for specified IP address */
	/* note that my_eaddr must be init'd before calling GetChaddr() */
	if (get_myeaddr(sockfd, &my_ipaddr, &my_eaddr, ifname) < 0) {
		report(LOG_ERR, "couldn't determine my ethernet addr for my IP address %s", inet_ntoa(my_ipaddr));
		my_exit(1, 1, 1);
	}

	if (debug > 0) {
		if (use_8021q) {
			report(LOG_INFO, "using interface %s, 802.1Q VLAN ID %d (IP address %s, hardware address %s)", 
				ifname, vlan_id, inet_ntoa(my_ipaddr), ether_ntoa(&my_eaddr));
		} else {
			report(LOG_INFO, "using interface %s, no 802.1Q (IP address %s, hardware address %s)", 
				ifname, inet_ntoa(my_ipaddr), ether_ntoa(&my_eaddr));
		}
		if (socket_receive_timeout_feature)
			report(LOG_INFO, "socket receive timeout feature enabled");
	}

	/* We're ready to handle SIGINT, SIGTERM, SIGQUIT ourself */
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = catcher;
	sa.sa_flags = 0;
	if (sigaction(SIGINT, &sa, NULL) < 0) {
		report(LOG_ERR, "sigaction: %s", get_errmsg());
		my_exit(1, 1, 1);
	}
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = catcher;
	sa.sa_flags = 0;
	if (sigaction(SIGTERM, &sa, NULL) < 0) {
		report(LOG_ERR, "sigaction: %s", get_errmsg());
		my_exit(1, 1, 1);
	}
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = catcher;
	sa.sa_flags = 0;
	if (sigaction(SIGQUIT, &sa, NULL) < 0) {
		report(LOG_ERR, "sigaction: %s", get_errmsg());
		my_exit(1, 1, 1);
	}



	/* install SIGHUP handler to re-read config files on demand */
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = catcher;
	sa.sa_flags = 0;
	if (sigaction(SIGHUP, &sa, NULL) < 0) {
		report(LOG_ERR, "sigaction: %s", get_errmsg());
		my_exit(1, 1, 1);
	}

	/* install SIGUSR1 handler to close/re-open logfile (if logfile being used) */
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = catcher;
	sa.sa_flags = 0;
	if (sigaction(SIGUSR1, &sa, NULL) < 0) {
		report(LOG_ERR, "sigaction: %s", get_errmsg());
		my_exit(1, 1, 1);
	}

	/* install SIGUSR2 handler to close/re-open capture file (if capture file being used) */
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = catcher;
	sa.sa_flags = 0;
	if (sigaction(SIGUSR2, &sa, NULL) < 0) {
		report(LOG_ERR, "sigaction: %s", get_errmsg());
		my_exit(1, 1, 1);
	}

	/* install SIGCHLD handler to reap children (e.g. when alert_program_name or alert_program_name2 is specified */
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = catcher;
	sa.sa_flags = 0;
	if (sigaction(SIGCHLD, &sa, NULL) < 0) {
		report(LOG_ERR, "sigaction: %s", get_errmsg());
		my_exit(1, 1, 1);
	}

	/* each packet we may write is the same length */
	write_packet_len = LIBNET_IPV4_H + LIBNET_UDP_H + sizeof(struct bootp);
	if (use_8021q) {
		write_packet_len +=  LIBNET_802_1Q_H; 
	} else {
		write_packet_len +=  LIBNET_ETH_H; 
	}

	/* init all the frames we may write */
	if (! init_libnet_context_queue()) {
		my_exit(1, 1, 1);
	}

	if (capture_file) { /* we are saving unexpected responses to a capture file */

		/* open a packet capture descriptor
		   This is NOT the one we'll actually use to read the interface to capture packets! 
		   When we call pcap_dump_open() to open a savefile, we're supposed to pass it the packet capture descriptor;
		   that's just so pcap_dump_open() can figure out what sort of interface and snaplen are involved -- it needs
		   to squirrel that info away to write a good header into the dump file, and to know how many bytes to
		   write for each packet.  The problem is that we're not going to keep capturing from a single
		   packet capture descriptor; instead we open and close packet capture descriptors repeatedly, to allow
		   us to NOT be listening when we don't need to (and also to vary some capture parms based on the changing cf file).  
		   To avoid having to open and close our dump file repeatedly (each time writing a *unique* dump file), we will open
		   a SECOND packet capture descriptor 'pd_template' which we'll keep open for the program's life.  That
		   one will share the key characteristics with the ones we actually use to capture packets (i.e. interface and snaplen).
		   Note this implies we must not let the user change those values during the run.
		*/
		pcap_errbuf[0] = '\0'; /* so we can tell if a warning was produced on success */
		if ((pd_template = pcap_open_live(ifname, snaplen, 0, 1, pcap_errbuf)) == NULL) {
			report(LOG_ERR, "pcap_open_live %s: %s", ifname, pcap_errbuf2);
			my_exit(1, 1, 1);
		}
		if (pcap_errbuf[0] != '\0')
			/* even on success, a warning may be produced */
			report(LOG_WARNING, "pcap_open_live %s: %s", ifname, pcap_errbuf);
	
		/* XXX Note pcap_dump_open() does does an fopen() on capture_file with mode "w", and writes
		   a pcap header to it.  It's up to the user to ensure the capture_file specified is safe.
		   Since we are probably running as root, opportunities for abuse abound.  The user
		   must be careful to specify a capture_file located in a directory no one else may write to,
		   and to ensure the capture_file does not exist, or if it does, it safe to overwrite.
		*/
		if ((pcap_dump_d = pcap_dump_open(pd_template, capture_file)) == NULL) {
			report(LOG_ERR, "pcap_dump_open: %s", pcap_geterr(pd_template));
			my_exit(1, 1, 1);
		}
	}

	while (1) { /* MAIN EVENT LOOP */
		int promiscuous;
		libnet_t *l;						/* to iterate through libnet context queue */
		/* struct pcap_stat ps;	*/			/* to hold pcap stats */

		if (debug > 10)
			report(LOG_DEBUG, "starting new cycle");

		/* handle signals.  
		   If this is not the first time through the main event loop, this
		   is where signals that arrived while we were sleeping get handled.  Note that
		   we also handle signals at a second location in the main event loop (after capturing responses
		   before we go to sleep).
		*/
		if (quit_requested) { /* set by signal handler */
			if (debug > 1)
				report(LOG_INFO, "received request to quit");
			break;
		}
		if (reopen_log_file) { /* set by signal handler */
			close_and_reopen_log_file(logfile_name);
			reopen_log_file = 0;
		}
		if (reopen_capture_file) { /* set by signal handler */
			close_and_reopen_capture_file();
			reopen_capture_file = 0;
		}
		if (reread_config_file)	{ /* set by signal handler */
			reconfigure(write_packet_len);
			reread_config_file = 0;
		}


		/* We open (and later close) the packet capture descriptor on each packet sent (rather than just
		   once for the entire program) because a change in the configfile (specifically, 'chaddr')
		   can change whether we need to listen promiscuously or not, and GetResponse_wait_time().
		   And we need to do it for each sent packet (as opposed to each cycle) to be able to specify
		   a fresh timeout each time, apparently (???).
		   Too, if we are listening promiscuously and the cycle_time is long, we'd prefer to leave the
		   interface in promiscuous mode as little as possible, since that can affect the host's performance.
		*/

		/* If we're going to claim a chaddr different than my_eaddr, some of the responses
		   may come back to chaddr (as opposed to my_eaddr or broadcast), so we'll need to
		   listen promiscuously.
		   If we're going to claim an ether_src different than my_eaddr, in theory that should
		   make no difference; bootp/dhcp servers should rely on chaddr, not ether_src.  Still,
		   it's possible there's a server out there that does it wrong, and might therefore mistakenly
		   send responses to ether_src.  So lets also listen promiscuously if ether_src != my_eaddr.
		*/
		if (bcmp(GetChaddr(), &my_eaddr, sizeof(struct ether_addr)) ||
		    bcmp(GetEther_src(), &my_eaddr, sizeof(struct ether_addr)))
			promiscuous = 1;
		else
			promiscuous = 0;


		for (l = libnet_cq_head(); libnet_cq_last(); l = libnet_cq_next()) { /* write one flavor packet and listen for answers */

			int pcap_rc;
			int pcap_open_retries;

			/* We set up for packet capture BEFORE writing our packet, to minimize the delay
			   between our writing and when we are able to start capturing.  (I cannot tell from
			   the pcap(3) doc whether packets matching the filter that arrive after pcap_open_live()
			   and before pcap_loop() are actually captured and buffered.  I assume not, if only because
			   that would imply that until calling pcap_setfilter(), we'd be capturing and buffering more than
			   we wanted!
			*/

			/* open packet capture descriptor */
			/* XXX On Solaris 7, sometimes pcap_open_live() fails with a message like:
					pcap_open_live qfe0: recv_ack: info unexpected primitive ack 0x8
			   It's not clear what causes this, or what the 0x8 code indicates.
			   The error appears to be transient; retrying sometimes will work, so I've wrapped the call in a retry loop.
			   I've also added a delay after each failure; perhaps the failure has something to do with the fact that
			   we call pcap_open_live() so soon after pcap_close() (for the second and succeeding packets in each cycle);
			   adding a delay might help in that case.
			*/
			pcap_open_retries = PCAP_OPEN_LIVE_RETRY_MAX;
			while (pcap_open_retries--) {
				pcap_errbuf[0] = '\0'; /* so we can tell if a warning was produced on success */
				if ((pd = pcap_open_live(ifname, snaplen, promiscuous, GetResponse_wait_time(), pcap_errbuf)) != NULL) {
					break; /* success */
				} else { /* failure */
					if (pcap_open_retries == 0) {
						report(LOG_DEBUG, "pcap_open_live(%s): %s; retry count (%d) exceeded, giving up", ifname, pcap_errbuf, PCAP_OPEN_LIVE_RETRY_MAX);
						my_exit(1, 1, 1);
					} else {
						if (debug > 1)
							report(LOG_DEBUG, "pcap_open_live(%s): %s; will retry", ifname, pcap_errbuf);
						sleep(PCAP_OPEN_LIVE_RETRY_DELAY); /* before next retry */
					}
				} /* failure */
			}
			if (pcap_errbuf[0] != '\0')
				/* even on success, a warning may be produced */
				report(LOG_WARNING, "pcap_open_live(%s): succeeded but with warning: %s", ifname, pcap_errbuf);

			/* make sure this interface is ethernet */
			linktype = pcap_datalink(pd);
			if (linktype != DLT_EN10MB) {
				report(LOG_ERR, "interface %s link layer type %d not ethernet", ifname, linktype);
				my_exit(1, 1, 1);
			}
			/* compile bpf filter to select just udp/ip traffic to udp port bootpc  */
			if (pcap_compile(pd, &bpf_code, "udp dst port bootpc", 1, netmask) < 0) {
				report(LOG_ERR, "pcap_compile: %s", pcap_geterr(pd));
				my_exit(1, 1, 1);
			}
			/* install compiled filter */
			if (pcap_setfilter(pd, &bpf_code) < 0) {
				report(LOG_ERR, "pcap_setfilter: %s", pcap_geterr(pd));
				my_exit(1, 1, 1);
			}
			if (socket_receive_timeout_feature)
				set_pcap_timeout(pd);

			/* write one packet */

			if (debug > 10)
				report(LOG_DEBUG, "writing packet %s", (char *) libnet_cq_getlabel(l));

			if ((bytes_written = libnet_write(l)) == -1) {
				report(LOG_ERR, "libnet_write failed: %s", libnet_geterror(l));
			} else {
				if (bytes_written < write_packet_len)
					report(LOG_ERR, "libnet_write: bytes written: %d (expected %d)", bytes_written, write_packet_len);
			}


			/* XXX Are response packets lost if they arrive between our call (above) to libnet_write(), 
			   and our call (below) to pcap_dispatch() ?   It's a long enough window for packets to arrive,
			   especially if debugging is high enough that we log a message below.
			*/

			/* listen for all replies until the timeout specified in pcap_open_live() expires.
			   For each reply, 'process_response' is called with ptrs to the reply packet.

			   XXX If you didn't specify enough buffer space, it appears that the packets that didn't fit
			   are silently lost; pcap_dispatch() doesn't provide any indication via a negative rc, and
			   even pcap_stats() doesn't show these as drops, so we can't provide some indication to the
			   user that the buffer specified is too small.
			*/

			if (debug > 10)
				report(LOG_DEBUG, "listening for answers for %d milliseconds", GetResponse_wait_time());


			/* XXX I often find that pcap_dispatch() returns well before the timeout specified earlier.
			   I ensure that there's no alarm() still left over before we start, and also ensure we don't
			   get interrupted by SIGCHLD (possible since process_response() could fork an alert_program or alert_program2 child).
			   But we STILL often return from pcap_dispatch() too soon!
			   April 2001: An update to the pcap(3) man page around version 0.6 (?), along with postings 
			   on the tcpdump workers mailing list explains what's going on.  The timeout specified in 
			   pcap_open_live() isn't a timeout in the sense one might expect.  The pcap_dispatch() call 
			   can return sooner than expected (even immediately), or if no packets are received, might 
			   never return at all; the behavior is platform-dependant.  I don't have a way to work
			   around this issue; it means this program  just won't work reliably (or at all) on some
			   platforms.
			*/

			alarm(0); /* just in case a previous alarm was still left */

			sigemptyset(&new_sigset);
			sigaddset(&new_sigset, SIGCHLD);
			sigprocmask(SIG_BLOCK, &new_sigset, &old_sigset);  /* block SIGCHLD */

			pcap_rc = pcap_dispatch(pd, -1, process_response, NULL);

			sigprocmask(SIG_SETMASK, &old_sigset, NULL);  /* unblock SIGCHLD */

			if (pcap_rc < 0)
				report(LOG_ERR, "pcap_dispatch(): %s", pcap_geterr(pd));
			else if (debug > 10)
				report(LOG_DEBUG, "done listening, captured %d packets", pcap_rc);

			/* I was hoping that perhaps pcap_stats() would return a nonzero number of packets dropped when
			   the buffer size specified to pcap_open_live() turns out to be too small -- so we could
			   provide some indication that you need to specify a larger buffer.  Alas, even in that situation
			   the ps_drop field is still 0.
			 *
			 *	if (pcap_stats(pd, &ps) < 0) {
			 *		report(LOG_ERR, "pcap_stats(): %s", pcap_geterr(pd));
			 *	} else if (debug > 10) {
			 *		report(LOG_DEBUG, "pcap_stats: packets received %u, packets dropped %u",  ps.ps_recv, ps.ps_drop);
			 *	}
			 */

			/* close packet capture descriptor */
			pcap_close(pd); 

			/* check for 'quit' request after each packet, since waiting until end of probe cycle
			   would impose a substantial delay. */
			if (quit_requested) { /* set by signal handler */
				if (debug > 1)
					report(LOG_INFO, "received request to quit");
				break;
			}
			/* don't check for requests to re-read configuration file here, because that sort of change
			   requires we construct new packets to send, not something to do in the middle of a cycle...
			   and can alter the response_timeout value used within the cycle. */
			/* don't check for requests to close-and-reopen the logfile, or close-and-reopen the
			   capture file here.  (should we?) */

		} /* write each flavor packet and listen for answers */

		/* Cleanup from iterating through the context queue. */
		if (!libnet_cq_end_loop()) {
			report(LOG_ERR, "libnet_cq_end_loop() failed");
			my_exit(1, 1, 1);
		}

		if (debug > 10)
			report(LOG_DEBUG, "cycle complete, going to sleep for %d seconds", GetCycle_time());

		/* Although we already handled signals at the top of the main event loop,
		   we do so again here, because the time through the main loop can be substantial due
		   to the time we capture packets, and signals may have come in...we don't want to postpone
		   handling them until we finish sleeping as well.
		*/
		if (quit_requested) { /* set by signal handler */
			if (debug > 1)
				report(LOG_INFO, "received request to quit");
			break;
		}
		if (reopen_log_file) { /* set by signal handler */
			close_and_reopen_log_file(logfile_name);
			reopen_log_file = 0;
		}
		if (reopen_capture_file) { /* set by signal handler */
			close_and_reopen_capture_file();
			reopen_capture_file = 0;
		}
		if (reread_config_file)	{ /* set by signal handler */
			reconfigure(write_packet_len);
			reread_config_file = 0;
		}

		/* We allow must signals that come in during our sleep() to interrupt us.  E.g. we want to cut short
		   our sleep when we're signalled to exit.  But we must block SIGCHLD during our sleep.  That's because
		   if we forked an alert_program or alert_program2 child above, its termination will likely happen while we're sleeping;
		   we'll end up being interrupted by the SIGCHLD almost immediately, cutting short our sleep and forcing
		   us to proceed to the next probe cycle far too soon.
		*/

		alarm(0); /* cancel any alarm left over, just in-case something's left by libpcap */
		time_to_sleep = GetCycle_time();

		sigemptyset(&new_sigset);
		sigaddset(&new_sigset, SIGCHLD);
		sigprocmask(SIG_BLOCK, &new_sigset, &old_sigset);  /* block SIGCHLD */

		sleep(time_to_sleep);

		sigprocmask(SIG_SETMASK, &old_sigset, NULL);  /* unblock SIGCHLD */

		alarm(0); /* cancel any alarm left over, just in case we were interrupted */

	} /* MAIN EVENT LOOP */


	/* we only reach here after receiving a signal requesting we quit */

	if (pd_template) /* only used if a capture file requested */
		pcap_close(pd_template); 

	my_exit(0, 1, 1);
}


void
process_response(u_char *user, const struct pcap_pkthdr *pkthdr, const u_char *packet)
{
/* Process one response packet. 
   We are called by pcap_dispatch() for each packet it captures.
   When we return, control returns to pcap_dispatch() so it can continue capturing packets.
*/

	struct ether_header *ether_header; /* access ethernet header */
	struct ip *ip_header;				/* access ip header */
	bpf_u_int32 ether_len;		/* bpf_u_int32 from pcap.h */
	struct udphdr *udp_header; /* access UDP header */
	struct bootp *bootp_pkt; /* access bootp/dhcp packet */
	int bootp_min_len; 
	int isYiaddrInLeaseNetworksOfConcern = 0; /* boolean */
	char yiaddr_network_of_concern_addenda[STR_MAXLEN];
	int isLegalServer;			/* boolean */

	/* fields parsed out from packet*/
	struct ether_addr ether_dhost, ether_shost;
	struct in_addr ip_src, ip_dst, yiaddr;
	/* string versions of same */
	char ether_dhost_str[MAX_ETHER_ADDR_STR], ether_shost_str[MAX_ETHER_ADDR_STR];
	char ip_src_str[MAX_IP_ADDR_STR], ip_dst_str[MAX_IP_ADDR_STR], yiaddr_str[MAX_IP_ADDR_STR];
	int ip_header_len_bytes;
	int udp_len; /* XXX why does udp.h declare this as signed? */
	int udp_payload_len;

	char *alert_program_name, *alert_program_name2;

	if (debug > 10)
		report(LOG_DEBUG, "   captured a packet");

	if ((pkthdr->caplen < (ether_len = pkthdr->len)) && (debug > 1)) {
		report(LOG_WARNING, "interface %s, packet truncated (ethernet frame length %u, captured %u), ignoring", ifname, ether_len, pkthdr->caplen);
		return;
	}

	if ((ether_len < sizeof(sizeof(struct ether_header))) && (debug > 1)) {
		report(LOG_WARNING, "interface %s, short packet (got %d bytes, smaller than an Ethernet header)", ifname, ether_len);
		return;
	}

	/* we use ether_header to access the Ethernet header */
	ether_header = (struct ether_header *) packet;

	/* parse fields out of ethernet header for easier access */
	bcopy(&(ether_header->ether_dhost), &ether_dhost, sizeof(ether_dhost));
	bcopy(&(ether_header->ether_shost), &ether_shost, sizeof(ether_shost));
	/* create printable versions of the fields we parsed above */
	bcopy(ether_ntoa(&ether_dhost), &ether_dhost_str, sizeof(ether_dhost_str));
	bcopy(ether_ntoa(&ether_shost), &ether_shost_str, sizeof(ether_shost_str));

	if (debug > 10)
		report(LOG_DEBUG, "     interface %s, from ether %s to %s", ifname, ether_shost_str, ether_dhost_str);

	if (ether_len < sizeof(sizeof(struct ether_header)) + sizeof(struct ip)) {
		report(LOG_WARNING, "interface %s, ether src %s: short packet (got %d bytes, smaller than IP header in Ethernet)", ifname, ether_shost_str, ether_len);
		return;
	}	

	/* we use ip_header to access the IP header */
	ip_header = (struct ip *) (packet + sizeof(struct ether_header));

	/* parse fields out of ip header for easier access */
	bcopy(&(ip_header->ip_src), &ip_src, sizeof(ip_header->ip_src));
	bcopy(&(ip_header->ip_dst), &ip_dst, sizeof(ip_header->ip_dst));
	/* create printable versions of the fields we parsed above */
	bcopy(inet_ntoa(ip_src), &ip_src_str, sizeof(ip_src_str));
	bcopy(inet_ntoa(ip_dst), &ip_dst_str, sizeof(ip_dst_str));

	if (debug > 10)
		report(LOG_DEBUG, "     from IP %s to %s", ip_src_str, ip_dst_str);

	ip_header_len_bytes = ip_header->ip_hl << 2;

	/* Repeat the packet size check (through IP header), but taking into account ip_header_len_bytes */
	if (ether_len < sizeof(sizeof(struct ether_header)) + ip_header_len_bytes) {
		report(LOG_WARNING, "interface %s, short packet (got %d bytes, smaller than IP header in Ethernet)", ifname, ether_len);
		return;
	}	

	/* we use udp_header to access the UDP header */
	udp_header = (struct udphdr *) (packet + sizeof(struct ether_header) + ip_header_len_bytes);

	if (ether_len <  sizeof(sizeof(struct ether_header)) + ip_header_len_bytes + sizeof(struct udphdr)) {
		report(LOG_WARNING, "interface %s ether src %s: short packet (got %d bytes, smaller than UDP/IP header in Ethernet)", ifname, ether_shost_str, ether_len);
		return;
	}	

	udp_len = udp_header->uh_ulen;
	if (udp_len < sizeof(struct udphdr)) {
		report(LOG_WARNING, "interface %s, ether src %s: invalid UDP packet (UDP length %d, smaller than minimum value %d)", ifname, ether_shost_str, udp_len, sizeof(struct udphdr));
		return;
	}

	udp_payload_len = udp_len - sizeof(struct udphdr);

	/* The smallest bootp/dhcp packet (the UDP payload) is actually smaller than sizeof(struct bootp),
	   as it's possible for DHCP replies to have shorter bootp_options fields.
	*/
	bootp_min_len = sizeof(struct bootp) - BOOTP_OPTIONS_LEN;

	if (udp_payload_len < bootp_min_len) {
		report(LOG_WARNING, "interface %s, ether src %s: invalid BootP/DHCP packet (UDP payload length %d, smaller than minimal BootP/DHCP payload %d)", ifname, ether_shost_str, udp_payload_len, bootp_min_len);
		return;
	}

	/* we use bootp_pkt to access the bootp/dhcp packet */
	bootp_pkt = (struct bootp *) (packet + sizeof(struct ether_header) + ip_header_len_bytes + sizeof(struct udphdr));

	/* Make sure the packet is in response to our query, otherwise ignore it.
	   Our query had bootp_htype=HTYPE_ETHER, bootp_hlen=HLEN_ETHER, and bootp_chaddr=GetChaddr().
	   Any reply with different values isn't in response to our probe, so we must ignore it.
	*/
	if (bootp_pkt->bootp_htype != HTYPE_ETHER) {
		if (debug > 10)
			report(LOG_DEBUG, "     bootp_htype (%d) != HTYPE_ETHER (%d), so this is not a response to my probe, ignoring", bootp_pkt->bootp_htype, HTYPE_ETHER);
		return;
	}

	if (bootp_pkt->bootp_hlen != HLEN_ETHER) {
		if (debug > 10)
			report(LOG_DEBUG, "     bootp_hlen (%d) != HLEN_ETHER (%d), so this is not a response to my probe, ignoring", bootp_pkt->bootp_hlen, HLEN_ETHER);
		return;
	}

	if (bcmp(bootp_pkt->bootp_chaddr, GetChaddr(), HLEN_ETHER)) {
		if (debug > 10) {
			struct ether_addr ether_tmp;
			char ether_tmp_str[MAX_ETHER_ADDR_STR];

			/* create printable version of bootp_pkt->bootp_chaddr */
			bcopy(&(bootp_pkt->bootp_chaddr), &ether_tmp, sizeof(ether_tmp));
			bcopy(ether_ntoa(&ether_tmp), &ether_tmp_str, sizeof(ether_tmp_str));

			report(LOG_DEBUG, "     bootp_chaddr (%s) != my chaddr (%s), so this is not a response to my probe, ignoring", ether_tmp_str, ether_ntoa(GetChaddr()));
		}
		return;
	}

	/* at this point we know the packet is a response to my probe */

	/* Determine if the response is from an expected server. */
	isLegalServer = 1; /* start by assuming it is expected. */

	if (!isLegalServersMember(&ip_src)) {
		if (debug > 10)
			report(LOG_DEBUG, "     ip_src %s is not a legal server", ip_src_str);
		isLegalServer = 0;
	}

	if (!isLegalServerEthersrcsMember(&ether_shost)) {
		if (debug > 10)
			report(LOG_DEBUG, "     ether_shost %s is not a legal server", ether_shost_str);
		isLegalServer = 0;
	}

	if (isLegalServer) {
		if (debug > 10)
			report(LOG_DEBUG, "     this is a legal server, ignoring");
		return;
	}

	/* at this point we know the responder is unexpected */

	/* parse yiaddr out of bootp packet easier access */
	bcopy(&(bootp_pkt->bootp_yiaddr), &yiaddr, sizeof(bootp_pkt->bootp_yiaddr));
	/* create printable version of the field we parsed above */
	bcopy(inet_ntoa(yiaddr), &yiaddr_str, sizeof(yiaddr_str));

	if (yiaddr.s_addr != INADDR_ANY) {
		if (isInLeaseNetworksOfConcern(&yiaddr)) {
			isYiaddrInLeaseNetworksOfConcern = 1;
			if (debug > 10)
				report(LOG_DEBUG, "     yiaddr %s is in inside a lease_network_of_concern", yiaddr_str);
		}
	}


	/* report unexpected server */
	/* Producing this log message is our entire reason for existance. */

	/* The log message may end with an addenda to further alert you that the yiaddr was inside a network of concern.
	   Prepare that possible addenda first.
	*/
	if (isYiaddrInLeaseNetworksOfConcern) {
		snprintf(yiaddr_network_of_concern_addenda, sizeof(yiaddr_network_of_concern_addenda), "  Response also contains yiaddr %s inside a network of concern.", yiaddr_str);
	} else {
		yiaddr_network_of_concern_addenda[0] = '\0';
	}

	report(LOG_WARNING, "received unexpected response on interface %s from BootP/DHCP server with IP source %s (ether src %s).%s", ifname, ip_src_str, ether_shost_str, yiaddr_network_of_concern_addenda);


	/* also save the response packet if we are writing to capture file */
	if (pcap_dump_d) {
		pcap_dump((u_char *) pcap_dump_d, pkthdr, packet);
	}

	/* Also call the alert_program_name if the user has specified one. */
	/* We must fetch it anew as it may have changed due to configfile change */
	alert_program_name = GetAlert_program_name();
	if (alert_program_name) {
		/* We run it in a child, so we don't block waiting for it to return. */
		pid_t pid;
		if ((pid = fork()) < 0) {
			report(LOG_ERR, "can't fork to run %s: %s", alert_program_name, get_errmsg());
			/* just skip running alert_program_name, but keep running since we're still fine */
		} else if (pid == 0) { /* child */
			/* We do allow child to inherit fd 0,1,2.  If we're logging to stderr, we want child to have it too. */
			if (sockfd) /* We don't want child to inherit the general purpose dgram socket */
				close(sockfd);
			libnet_cq_destroy(); /* We don't want child to inherit to inherit libnet context queue */
			if (pd) /* We don't want child to inherit packet capture descriptor, nor packet dumpfile descriptor. */
				pcap_close(pd);
			if (pcap_dump_d)
				pcap_dump_close(pcap_dump_d);
			if (execl(alert_program_name, alert_program_name, prog, ifname, ip_src_str, ether_shost_str, (char *) 0 ) < 0) {
				report(LOG_ERR, "can't execute alert_program_name '%s': %s", alert_program_name, get_errmsg());
				exit(0);  /* child exits */
			}
		}
	} /* if (alert_program_name) */

	/* Also call the alert_program_name2 if the user has specified one. */
	/* We must fetch it anew as it may have changed due to configfile change */
	alert_program_name2 = GetAlert_program_name2();
	if (alert_program_name2) {
		/* We run it in a child, so we don't block waiting for it to return. */
		pid_t pid;
		if ((pid = fork()) < 0) {
			report(LOG_ERR, "can't fork to run %s: %s", alert_program_name2, get_errmsg());
			/* just skip running alert_program_name2, but keep running since we're still fine */
		} else if (pid == 0) { /* child */
			int execl_rc;
			/* We do allow child to inherit fd 0,1,2.  If we're logging to stderr, we want child to have it too. */
			if (sockfd) /* We don't want child to inherit the general purpose dgram socket */
				close(sockfd);
			libnet_cq_destroy(); /* We don't want child to inherit to inherit libnet context queue */
			if (pd) /* We don't want child to inherit packet capture descriptor, nor packet dumpfile descriptor. */
				pcap_close(pd);
			if (pcap_dump_d)
				pcap_dump_close(pcap_dump_d);
			if (isYiaddrInLeaseNetworksOfConcern) {
				/* include "-y yiaddr' option */
				execl_rc = execl(alert_program_name2, alert_program_name2, "-p", prog, "-I", ifname, "-i", ip_src_str, "-m", ether_shost_str, "-y", yiaddr_str, (char *) 0 );
			} else {
				/* do not include "-y yiaddr' option */
				execl_rc = execl(alert_program_name2, alert_program_name2, "-p", prog, "-I", ifname, "-i", ip_src_str, "-m", ether_shost_str, (char *) 0 );
			}
			if (execl_rc < 0) {
				report(LOG_ERR, "can't execute alert_program_name2 '%s': %s", alert_program_name2, get_errmsg());
				exit(0);  /* child exits */
			}
		}
	} /* if (alert_program_name2) */


	return;
}


void 
set_pcap_timeout(pcap_t *pd)
{
/*
	Set a receive timeout on the socket underlying the pcap descriptor.

	Ideally, this would not be necessary, as we already passed a timeout to pcap_open_live().
	But as the pcap(3) man page explains, that timeout is not supported on some platforms.
	In those cases, applying a timeout directly to the underlying socket might help.
*/

	struct timeval timeout;
	int time_wait;

	time_wait = GetResponse_wait_time();
	timeout.tv_sec  = time_wait / 1000;
	timeout.tv_usec = (time_wait % 1000) * 1000;
	if(setsockopt(pcap_fileno(pd), SOL_SOCKET, SO_RCVTIMEO,
			&timeout, sizeof(timeout)) < 0) {
		report(LOG_ERR, "set_pcap_timeout(): unable to set receive timeout: %s", get_errmsg());
		my_exit(1, 1, 1);
	}

}


void
reconfigure(const int write_packet_len)
{
/* Perform all necessary functions to handle a request to reconfigure.
   Must not be called until after initial configuration is complete.
*/
   
	int i;

	if (! read_configfile(config_file)) {
		my_exit(1, 1, 1);
	}

	/* Contents of the packets we send may need to change as a result of change
	   to the configuration.  Free the packets we've already constructed, and build new ones. */
	if (! init_libnet_context_queue()) {
		my_exit(1, 1, 1);
	}

	return;
}


void
close_and_reopen_capture_file(void) 
{
/*	Close and re-open capture file.
	If we are not capturing to a file, return silently.
	Returns on success, exits on error.

	Note that since pcap_dump_open() opens the file with mode "w" and writes a pcap header to it,
	if you want to keep the existing capture file's contents, you must move the existing 
	capture file aside before this routine is called.  In practice, that means you move the
	file aside first, then send a signal triggering the close and re-open.
*/

	if (pcap_dump_d) { /* a capture file was already open */
		if (debug > 1)
			report(LOG_NOTICE, "closing capture file");

		/* close */
		 pcap_dump_close(pcap_dump_d);

		/* re-open */
		/* XXX Note pcap_dump_open() does does an fopen() on capture_file with mode "w", and writes
		   a pcap header to it.  It's up to the user to ensure the capture_file specified is safe.
		   Since we are probably running as root, opportunities for abuse abound.  The user
		   must be careful to specify a capture_file located in a directory no one else may write to,
		   and to ensure the capture_file does not exist, or if it does, it safe to overwrite.
		*/
		if ((pcap_dump_d = pcap_dump_open(pd_template, capture_file)) == NULL) {
			report(LOG_ERR, "close_and_reopen_capture_file: pcap_dump_open: %s", pcap_geterr(pd_template));
			my_exit(1, 1, 1);
		}

		if (debug > 1)
			report(LOG_NOTICE, "re-opened capture file");

	}
	return;
}


void
catcher(int sig)
{
/*	Signal catcher. */

	if ((sig == SIGINT) || (sig == SIGTERM) || (sig == SIGQUIT))  { /* quit gracefully */
		quit_requested = 1;
		return;

	} else if (sig == SIGHUP) { /* re-read config file */
		/* Doing the reread while in the signal handler is way too dangerous.
		   We'll do it at the start or end of the next main event loop.
		*/
		reread_config_file = 1;
		return;

	} else if (sig == SIGUSR1) { /* close and re-open logfile (if logfile being used) */
		/* Doing the close and reopen in the signal handler is way too dangerous.
		   We'll do it at the start or end of the next main event loop.
		*/
		reopen_log_file = 1;
		return;
	} else if (sig == SIGUSR2) { /* close and re-open capture file (if capture file being used) */
		/* Doing the close and reopen in the signal handler is way too dangerous.
		   We'll do it at the start or end of the next main event loop.
		*/
		reopen_capture_file = 1;
		return;
	} else if (sig == SIGCHLD) { /* reap, e.g. calls to user-specified alert_program_name */
		int stat, errno_save;

		errno_save = errno;
		while ((waitpid(-1, &stat, WNOHANG)) > 0)
			;
		errno = errno_save;
		return;
	}
		
	return;
}


void
cleanup(void)
{
/*	Cleanup tasks at exit. */

	/* Destroy all the libnet contexts (if any), and the context queue (if any). */
	libnet_cq_destroy();

	if (pcap_dump_d) /* capture file is open */
		pcap_dump_close(pcap_dump_d);

	if (pid_file)
		unlink(pid_file); /* may fail if file was never written */

	return;
}

void
my_exit(int exit_status, int do_cleanup, int do_log)
{
	/* A wrapper for exit().  */

	if (do_log)
		report(LOG_NOTICE, "exiting");

	if (do_cleanup)
		cleanup();

	exit(exit_status);
}


void
usage(void)
{
/*	Print usage message and return. */

	fprintf(stderr, "Usage: %s [-c config_file] [-d debuglevel] [-f] [-h] [-l log_file] [-o capture_file] [-p pid_file] [-Q vlan_id] [-s capture_bufsize] [-T] [-v] [-w cwd] interface_name\n", prog);
	fprintf(stderr, "   -c config_file                 override default config file [%s]\n", CONFIG_FILE);
	fprintf(stderr, "   -d debuglevel                  enable debugging at specified level\n");
	fprintf(stderr, "   -f                             don't fork (only use for debugging)\n");
	fprintf(stderr, "   -h                             display this help message then exit\n");
	fprintf(stderr, "   -l log_file                    log to file instead of syslog\n");
	fprintf(stderr, "   -o capture_file                enable capturing of unexpected answers\n");
	fprintf(stderr, "   -p pid_file                    override default pid file [%s]\n", PID_FILE);
	fprintf(stderr, "   -Q vlan_id                     tag outgoing frames with an 802.1Q VLAN ID\n");
	fprintf(stderr, "   -s capture_bufsize             override default capture bufsize [%d]\n", CAPTURE_BUFSIZE);
	fprintf(stderr, "   -T                             enable the socket receive timeout feature\n");
	fprintf(stderr, "   -v                             display version number then exit\n");
	fprintf(stderr, "   -w cwd                         override default working directory [%s]\n", CWD);
	fprintf(stderr, "   interface_name                 name of ethernet interface\n");

	return;
}

