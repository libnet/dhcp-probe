#ifndef DEFAULTS_H_
#define DEFAULTS_H_

/* Default values. 
   You may edit this file before running 'make' to change these defaults 
   and enable/disable some features.
*/


/**********************************************************************/
/* You may want to change these, since you can't override them from   */
/* the commandline nor the configuration file.                        */

/* If we log via syslog, what facility name should we use?
   See syslog(3) for other possible facility names.
   This value is not used if we log to a logfile.
*/
#define LOG_FACILITY LOG_DAEMON

/* Size of arrays for multiply-occurring values in the configuration file.
   If you need to specify more values, increase these sizes.
*/
#define MAX_LEGAL_SERVERS 64
#define MAX_LEASE_NETWORKS_OF_CONCERN 128
#define MAX_LEGAL_SERVER_ETHERSRCS 64

/* transaction ID field in bootp header */
#define BOOTP_XID 0x19970112

/* When pcap_open_live() fails, number of times to retry before giving up.
   Should fit into an int.
*/
#define PCAP_OPEN_LIVE_RETRY_MAX 10

/* When pcap_open_live() fails, number of seconds to sleep before retrying.
   Should fit into an unsigned int.
*/
#define PCAP_OPEN_LIVE_RETRY_DELAY 1

/**********************************************************************/
/* Unless you are doing server development, leave these alone.        */


/**********************************************************************/
/* You probably shouldn't need to change any of these, since you may  */
/* override each of them from the config file.                        */

/* ip address, specified as a string constant */
#define SERVER_ID "10.254.254.254"

/* ip address, specified as a string constant */
#define CLIENT_IP_ADDRESS "172.31.254.254"

/* milliseconds, must fit into an int */
#define RESPONSE_WAIT_TIME 5000

/* seconds, must fit into an unsigned int */
#define CYCLE_TIME 300


/**********************************************************************/
/* You probably shouldn't need to change any of these, since you may  */
/* override each of them from the commandline.                        */

/* Absolute path to pidfile */ 
#define PID_FILE "/var/run/dhcp_probe.pid"

/* Absolute path to config file */
#define CONFIG_FILE "/etc/dhcp_probe.cf"

/* Absolute path to current working directory */
#define CWD "/"

/* Size of buffer (in bytes) to capture *all* responses to a single packet.
   Must fit into an 'int'
*/
#define CAPTURE_BUFSIZE (20 * 1514)

/* Sometimes use when allocating miscellaneous strings */
#define STR_MAXLEN 1024

/**********************************************************************/
/* You shouldn't need to change any of these, since you may           */
/* override each of them from the config file.                        */



#endif /* not DEFAULTS_H_ */
