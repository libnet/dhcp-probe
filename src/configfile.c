/* Routines for dealing with the application's config data:
	read_configfile() reads the config file and stores the data in storage private to this module.
    All the rest are accessor functions to this private data.
	Do not call any accessor function before read_configfile().
*/

/* Copyright (c) 2000-2008, The Trustees of Princeton University, All Rights Reserved. */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "defs.h"

#include "dhcp_probe.h"
#include "defaults.h"
#include "configfile.h"
#include "report.h"


/* chaddr to use for bootp header 'chaddr' and to construct ClientID option */
/* optionally specified by user; if unspecified, GetChaddr() returns my_eaddr */
struct ether_addr chaddr; 
int is_chaddr_specified; /* flag */

/* ether_addr to use for ethernet frame src */
/* optionally specified by user; if unspecified, GetEther_src() returns my_eaddr */
struct ether_addr ether_src; 
int is_ether_src_specified; /* flag */

/* An ipaddr to use for "Server Identifer" option  (when this is needed)
   if unspecified, defaults to SERVER_ID
*/
struct in_addr server_id;

/* ipaddr to use for "Client IP Address" option and 'ciaddr' value (when these
   are needed)
   if unspecified, defaults to CLIENT_IP_ADDRESS
*/
struct in_addr client_ip_address;

/* milliseconds to wait for a response after sending one packet */
int response_wait_time;

/* seconds to wait after completing one set of checks before starting next set */
unsigned cycle_time;

/* array of legal DHCP servers' IPsrc addresses, and number elems in array */
struct in_addr legal_servers[MAX_LEGAL_SERVERS];
int num_legal_servers;

/* array of legal DHCP servers' ethersrc addresses, and number elems in array */
struct ether_addr legal_server_ethersrcs[MAX_LEGAL_SERVER_ETHERSRCS];
int num_legal_server_ethersrcs;

/* parallel arrays of "lease networks of concern" (address & mask), and number of elems in arrays.
   We also precompute a parallel array of 'addr & mask' to save computing these repeatedly later.
/* If a response already identified as being from a rogue DHCP server contains a yiaddr field field
   that falls into one of these networks, it is reported as a matter of special concern.
*/
struct in_addr lease_networks_of_concern_addr[MAX_LEASE_NETWORKS_OF_CONCERN];
struct in_addr lease_networks_of_concern_netmask[MAX_LEASE_NETWORKS_OF_CONCERN];
struct in_addr lease_networks_of_concern_addr_and_netmask[MAX_LEASE_NETWORKS_OF_CONCERN];
int num_lease_networks_of_concern;

/* optional name of external alert program to call */
char *alert_program_name = NULL; /* old style alert program */
char *alert_program_name2 = NULL; /* newer style alert program */


int
read_configfile(const char *fname)
{
/* Read the config file, initializing appropriate data structures from it.
   Return 1 on success, 0 on fatal error.
   (A syntax/semantic error inside the file is not a fatal error.
   Fatal errors include: being unable to open the file.

   We assume the caller has already init'd global my_eaddr.
*/
	FILE *fp;
	char buf[BUFSIZ]; /* one entire line read from input file */
	int line; /* input file line counter */
	char str1[BUFSIZ], str2[BUFSIZ], str3[BUFSIZ], str4[BUFSIZ]; /* tokens parsed by sscanf */
	int tokens; /* number of tokens successfully read by sscanf */
	int tmpint;
	unsigned int tmpuint;
	struct ether_addr *enet;
	struct in_addr inaddr, inaddr2;
	
	/* init all values to defaults */
	is_chaddr_specified = 0;
	/* chaddr can be left uninit'd, since we only use it when is_chaddr_specified is set */
	is_ether_src_specified = 0;
	/* ether_src can be left uninit'd, since we only use it when is_ether_src_specified is set */
	inet_aton(SERVER_ID, &server_id);
	inet_aton(CLIENT_IP_ADDRESS, &client_ip_address);
	cycle_time = CYCLE_TIME;
	response_wait_time = RESPONSE_WAIT_TIME;
	num_legal_servers = 0;
	num_legal_server_ethersrcs = 0;
	num_lease_networks_of_concern = 0;
	if (alert_program_name) { /* we must have malloc'd it last time we read the config file */
		free(alert_program_name);
		alert_program_name = NULL;
	}
	if (alert_program_name2) { /* we must have malloc'd it last time we read the config file */
		free(alert_program_name2);
		alert_program_name2 = NULL;
	}

	if (debug > 1)
		report(LOG_INFO, "read_configfile: starting");

	if ((fp = fopen(fname, "r")) == NULL) {
		report(LOG_ERR, "read_configfile: fopen(%s): %s", fname, strerror(errno));
		return(0); /* fatal error */
	}

	line = 0;
	while (fgets(buf, BUFSIZ, fp) != NULL) { /* process one line */

		line++;
		str1[0] = str2[0] = str3[0] = str4[0] = '\0'; /* rc is sometimes ambiguous, ensure we don't try to read old values/garbage */

		tokens = sscanf(buf, "%s %s %s %s", str1, str2, str3, str4);

        if ((tokens == EOF) || (tokens == 0)) /* blank line */
            continue;
		else if (str1[0] == '#') /* comment line */
			continue;

		if (! strcasecmp(str1, "chaddr")) {

			/* token2: required ethernet address  */
			if (tokens < 2) {
				report(LOG_ERR, "read_configfile: line %d, not enough values: %s", line, buf);
				continue;
			}

			if ((enet = ether_aton(str2)) == NULL) {
				report(LOG_ERR, "read_configfile: line %d, skipping invalid ethernet address: %s", line, str2);
				continue;
			}

			bcopy(enet, &chaddr, sizeof(chaddr));	
			if (debug > 2)
				report(LOG_DEBUG, "read_configfile: chaddr %s", ether_ntoa(&chaddr));

			is_chaddr_specified = 1;

		} else if (! strcasecmp(str1, "ether_src")) {

			/* token2: required ethernet address  */
			if (tokens < 2) {
				report(LOG_ERR, "read_configfile: line %d, not enough values: %s", line, buf);
				continue;
			}

			if ((enet = ether_aton(str2)) == NULL) {
				report(LOG_ERR, "read_configfile: line %d, skipping invalid ethernet address: %s", line, str2);
				continue;
			}

			bcopy(enet, &ether_src, sizeof(ether_src));	
			if (debug > 2)
				report(LOG_DEBUG, "read_configfile: ether_src %s", ether_ntoa(&ether_src));

			is_ether_src_specified = 1;

		} else if (! strcasecmp(str1, "server_id")) {

			/* token2: required IP address  */
			if (tokens < 2) {
				report(LOG_ERR, "read_configfile: line %d, not enough values: %s", line, buf);
				continue;
			}

			/* convert address */
			if (inet_aton(str2, &inaddr) == 0) {
				report(LOG_ERR, "read_configfile: line %d, invalid IP address: %s", line, str2);
				continue;
			}
			/* address converted ok, save it */
			server_id.s_addr = inaddr.s_addr;
			if (debug > 2)
				report(LOG_DEBUG, "read_configfile: server_id %s", inet_ntoa(server_id));

		} else if (! strcasecmp(str1, "client_ip_address")) {

			/* token2: required IP address  */
			if (tokens < 2) {
				report(LOG_ERR, "read_configfile: line %d, not enough values: %s", line, buf);
				continue;
			}

			/* convert address */
			if (inet_aton(str2, &inaddr) == 0) {
				report(LOG_ERR, "read_configfile: line %d, invalid IP address: %s", line, str2);
				continue;
			}
			/* address converted ok, save it */
			client_ip_address.s_addr = inaddr.s_addr;
			if (debug > 2)
				report(LOG_DEBUG, "read_configfile: client_ip_address %s", inet_ntoa(client_ip_address));

		} else if (! strcasecmp(str1, "response_wait_time")) {

			/* token2: required number of seconds */
			if (tokens < 2) {
				report(LOG_ERR, "read_configfile: line %d, not enough values: %s", line, buf);
				continue;
			}

			/* convert string to positive integer */
			if (!sscanf(str2, "%d", &tmpint) || (tmpint < 1)) {
				report(LOG_ERR, "read_configfile: line %d, response_wait_time must be an positive integer > 0", line);
				continue;
			}

			response_wait_time = tmpint;

			if (debug > 2)
				report(LOG_DEBUG, "read_configfile: response_wait_time %u", response_wait_time);

		} else if (! strcasecmp(str1, "cycle_time")) {

			/* token2: required number of seconds */
			if (tokens < 2) {
				report(LOG_ERR, "read_configfile: line %d, not enough values: %s", line, buf);
				continue;
			}

			/* convert string to unsigned integer */
			if (!sscanf(str2, "%u", &tmpuint) || (tmpuint < 1)) {
				report(LOG_ERR, "read_configfile: line %d, cycle_time must be an unsigned integer", line);
				continue;
			}

			cycle_time = tmpuint;

			if (debug > 2)
				report(LOG_DEBUG, "read_configfile: cycle_time %u", cycle_time);

		} else if (! strcasecmp(str1, "legal_server")) {

			if (num_legal_servers == MAX_LEGAL_SERVERS) {
				report(LOG_ERR, "read_configfile: line %d, number of legal_server statements exceeds maximum (%d), ignoring", line, MAX_LEGAL_SERVERS);
				report(LOG_ERR, "You may increase the maximum by adjusting MAX_LEGAL_SERVERS and recompiling.");
				continue;
			}

			/* token2: required IP address  */
			if (tokens < 2) {
				report(LOG_ERR, "read_configfile: line %d, not enough values: %s", line, buf);
				continue;
			}

			/* convert address */
			if (inet_aton(str2, &inaddr) == 0) {
				report(LOG_ERR, "read_configfile: line %d, invalid IP address: %s", line, str2);
				continue;
			}

			/* address converted ok, save it */
			legal_servers[num_legal_servers].s_addr = inaddr.s_addr;
			if (debug > 2)
				report(LOG_DEBUG, "read_configfile: legal_server %s", inet_ntoa(legal_servers[num_legal_servers]));

			num_legal_servers++;

		} else if (! strcasecmp(str1, "legal_server_ethersrc")) {

			if (num_legal_server_ethersrcs == MAX_LEGAL_SERVER_ETHERSRCS) {
				report(LOG_ERR, "read_configfile: line %d, number of legal_server_ethersrc statements exceeds maximum (%d), ignoring", line, MAX_LEGAL_SERVER_ETHERSRCS);
				report(LOG_ERR, "You may increase the maximum by adjusting MAX_LEGAL_SERVER_ETHERSRCS and recompiling.");
				continue;
			}

			/* token2: required Ethernet address */
			if (tokens < 2) {
				report(LOG_ERR, "read_configfile: line %d, not enough values: %s", line, buf);
				continue;
			}

			if ((enet = ether_aton(str2)) == NULL) {
				report(LOG_ERR, "read_configfile: line %d, skipping invalid ethernet address: %s", line, str2);
				continue;
			}

			bcopy(enet, &legal_server_ethersrcs[num_legal_server_ethersrcs], sizeof(legal_server_ethersrcs[num_legal_server_ethersrcs]));
			if (debug > 2)
				report(LOG_DEBUG, "read_configfile: legal_server_ethersrc %s", ether_ntoa(&legal_server_ethersrcs[num_legal_server_ethersrcs]));

			num_legal_server_ethersrcs++;


		} else if (! strcasecmp(str1, "lease_network_of_concern")) {

			if (num_lease_networks_of_concern == MAX_LEASE_NETWORKS_OF_CONCERN) {
				report(LOG_ERR, "read_configfile: line %d, number of lease_network_of_concern statements exceeds maximum (%d), ignoring", line, MAX_LEASE_NETWORKS_OF_CONCERN);
				report(LOG_ERR, "You may increase the maximum by adjusting MAX_LEASE_NETWORKS_OF_CONCERN and recompiling.");
				continue;
			}

			/* token 2: required network IP address.  token 3: required network mask */
			if (tokens < 3) {
				report(LOG_ERR, "read_configfile: line %d, not enough values: %s", line, buf);
				continue;
			}

			/* convert network address */
			if (inet_aton(str2, &inaddr) == 0) {
				report(LOG_ERR, "read_configfile: line %d, invalid IP network address: %s", line, str2);
				continue;
			}

			/* convert network mask */
			if (inet_aton(str3, &inaddr2) == 0) {
				report(LOG_ERR, "read_configfile: line %d, invalid IP network mask: %s", line, str3);
				continue;
			}

			/* address and mask converted ok, save them */
			lease_networks_of_concern_addr[num_lease_networks_of_concern].s_addr = inaddr.s_addr;
			lease_networks_of_concern_netmask[num_lease_networks_of_concern].s_addr = inaddr2.s_addr;

			/* precompute 'addr & mask' to save us the effort of repeatedly computing it later. */
			lease_networks_of_concern_addr_and_netmask[num_lease_networks_of_concern].s_addr =  inaddr.s_addr & inaddr2.s_addr;

			if (debug > 2) {
				/* Calling inet_ntoa() twice in the same statement isn't safe.
				   Avoid the need to do so by computing one of the results first and temporarily copying it.
				*/
				char addr_str_tmp[MAX_IP_ADDR_STR];
				bcopy(inet_ntoa(lease_networks_of_concern_addr[num_lease_networks_of_concern]), &addr_str_tmp, sizeof(addr_str_tmp));
				report(LOG_DEBUG, "read_configfile: lease_network_of_concern %s %s", addr_str_tmp, inet_ntoa(lease_networks_of_concern_netmask[num_lease_networks_of_concern]));
			}

			num_lease_networks_of_concern++;

		} else if (! strcasecmp(str1, "alert_program_name")) {

			if (alert_program_name) {
				report(LOG_ERR, "read_configfile: line %d, alert_program_name may be specified only once, ignoring", line);
				continue;
			}

			if (alert_program_name2) {
				report(LOG_ERR, "read_configfile: line %d, alert_program_name may not be specified when alert_program_name2 was already specified, as only one may be used, ignoring", line);
				continue;
			}

			/* token2: required program name */
			if (tokens < 2) {
				report(LOG_ERR, "read_configfile: line %d, not enough values: %s", line, buf);
				continue;
			}

			if (str2[0] != '/') {
				report(LOG_ERR, "read_configfile: line %d, invalid alert_program_name '%s', must be an absolute pathname, ignoring", line, str2);
				continue;
			}

			alert_program_name = strdup(str2);
			if (! alert_program_name) {
				report(LOG_ERR, "read_configfile: line %d, can't save alert_program_name because strdup() could not malloc() space, ignoring", line);
				continue;
			}

			if (debug > 2)
				report(LOG_DEBUG, "read_configfile: alert_program_name %s", alert_program_name);

		} else if (! strcasecmp(str1, "alert_program_name2")) {

			if (alert_program_name2) {
				report(LOG_ERR, "read_configfile: line %d, alert_program_name2 may be specified only once, ignoring", line);
				continue;
			}

			if (alert_program_name) {
				report(LOG_ERR, "read_configfile: line %d, alert_program_name2 may not be specified when alert_program_name was already specified, as only one may be used, ignoring", line);
				continue;
			}

			/* token2: required program name */
			if (tokens < 2) {
				report(LOG_ERR, "read_configfile: line %d, not enough values: %s", line, buf);
				continue;
			}

			if (str2[0] != '/') {
				report(LOG_ERR, "read_configfile: line %d, invalid alert_program_name2 '%s', must be an absolute pathname, ignoring", line, str2);
				continue;
			}

			alert_program_name2 = strdup(str2);
			if (! alert_program_name2) {
				report(LOG_ERR, "read_configfile: line %d, can't save alert_program_name2 because strdup() could not malloc() space, ignoring", line);
				continue;
			}

			if (debug > 2)
				report(LOG_DEBUG, "read_configfile: alert_program_name2 %s", alert_program_name2);


		} else {
			report(LOG_ERR, "read_configfile: line %d, unrecognized token: %s", line, str1);
		}

	} /* process one line */

	fclose(fp);

	if (debug > 1)
		report(LOG_INFO, "read_configfile: done");

	return(1); /* success */

}


struct ether_addr *
GetChaddr (void)
{
/* Return copy of chaddr (if specified) else my_eaddr.
   Not re-entrant; we use static storage to hold the value address we return.
   Should not be called until global 'my_eaddr' has been init'd.
*/
	static struct ether_addr chaddr_copy;

	/* we re-init the static copy on each call, since we don't know if the
	   	caller has written into it. */

	if (is_chaddr_specified) {
		bcopy(&chaddr, &chaddr_copy, sizeof(chaddr_copy));
	} else {
		bcopy(&my_eaddr, &chaddr_copy, sizeof(chaddr_copy));
	}

	return &chaddr_copy;
}


struct ether_addr *
GetEther_src (void)
{
/* Return copy of ether_src (if specified) else my_eaddr.
   Not re-entrant; we use static storage to hold the value address we return.
   Should not be called until global 'my_eaddr' has been init'd.
*/
	static struct ether_addr ether_src_copy;

	/* we re-init the static copy on each call, since we don't know if the
	   	caller has written into it. */

	if (is_ether_src_specified) {
		bcopy(&ether_src, &ether_src_copy, sizeof(ether_src_copy));
	} else {
		bcopy(&my_eaddr, &ether_src_copy, sizeof(ether_src_copy));
	}

	return &ether_src_copy;
}


struct in_addr *
GetClient_ip_address(void)
{
/* Return copy of client_ip_address.
   Not re-entrant; we use static storage to hold the value we return.
*/
	static struct in_addr client_ip_address_copy;

	/* we re-init the static copy on each call, since we don't know if the
	   	caller has written into it. */

	client_ip_address_copy.s_addr = client_ip_address.s_addr;	

	return &client_ip_address_copy;
}


struct in_addr *
GetServer_id(void)
{
/* Return copy of server_id.
   Not re-entrant; we use static storage to hold the value we return.
*/
	static struct in_addr server_id_copy;

	/* we re-init the static copy on each call, since we don't know if the
	   	caller has written into it. */

	server_id_copy.s_addr = server_id.s_addr;	

	return &server_id_copy;
}


unsigned
GetCycle_time(void)
{
/* Return value of cycle_time. */
	return cycle_time;
}


int
GetResponse_wait_time(void)
{
/* Return value of response_wait_time */
	return response_wait_time;
}


int
isLegalServersMember(struct in_addr *ipaddr)
{
/* If ipaddr is a member of legal_servers[], return true.
   Else return false.
*/
	int i;

	if (!ipaddr) {
		report(LOG_ERR, "isLegalServersMember: internal error, called with *ipaddr==NULL");
		return 0; /* not found */
	}

	for (i = 0; i < num_legal_servers; i++) {
		if (ipaddr->s_addr == legal_servers[i].s_addr)
			return 1; /* found */
	}
	return 0; /* not found */
}

int
isInLeaseNetworksOfConcern(struct in_addr *ipaddr)
{
/* If ipaddr is within any of the lease_networks_of_concern, return true.
   Else return false.
*/

	int i;

	if (!num_lease_networks_of_concern)
		/* No lease_networks_of_concern have been specified. */
		return 0; /* not found */

	for (i = 0; i < num_lease_networks_of_concern; i++) {
		if ((ipaddr->s_addr & lease_networks_of_concern_netmask[i].s_addr) == lease_networks_of_concern_addr_and_netmask[i].s_addr)
			return 1; /* found */
	}
	return 0; /* not found */
}

int
isLegalServerEthersrcsMember(struct ether_addr *eaddr)
{
/* If eaddr is a member of legal_server_ethersrcs[], return true.
   If legal_server_ethersrcs[] is empty, also return true.
   Else return false.
*/

	int i;

	if (!eaddr) {
		report(LOG_ERR, "isLegalServerEthersrcsMember: internal error, called with *eaddr==NULL");
		return 0; /* not found */
	}

	/* If no legal_server_ethersrc values have been specified, then the user doesn't want us
	   to check the response's ethersrc in the first place.  So treat eaddr as "legal"; return true.
	*/
	if (!num_legal_server_ethersrcs)
		return 1;

	for (i = 0; i < num_legal_server_ethersrcs; i++) {
		if (!bcmp(eaddr, &legal_server_ethersrcs[i], sizeof(struct ether_addr))) 
			return 1; /* found */
	}
	return 0; /* not found */
}


/* Return copy of alert_program_name string.
   Not re-entrant; we use static storage to hold the ptr to the string we return.
*/
char *
GetAlert_program_name(void)
{
    static char *alert_program_name_copy = NULL;

	if (alert_program_name_copy) {
		/* Space was allocated from a previous call.
		   We must not re-use that space, since it's possible that the alert_program_name has
		   gotten longer due to a re-read of the configfile.
		*/
		free(alert_program_name_copy);
		alert_program_name_copy = NULL;
	}

	if (!alert_program_name) {
		return (char *) NULL;
	}

	/* we re-init the static copy on each call, since we don't know if the caller has
	   written into it. */
	alert_program_name_copy = strdup(alert_program_name);

	if (!alert_program_name_copy) {
		report(LOG_ERR, "GetAlert_program_name: strdup() failed (presumably a malloc error)");
		my_exit(1, 1, 1);
	}

    return alert_program_name_copy;
}

/* Return copy of alert_program_name2 string.
   Not re-entrant; we use static storage to hold the ptr to the string we return.
*/
char *
GetAlert_program_name2(void)
{
    static char *alert_program_name2_copy = NULL;

	if (alert_program_name2_copy) {
		/* Space was allocated from a previous call.
		   We must not re-use that space, since it's possible that the alert_program_name2 has
		   gotten longer due to a re-read of the configfile.
		*/
		free(alert_program_name2_copy);
		alert_program_name2_copy = NULL;
	}

	if (!alert_program_name2) {
		return (char *) NULL;
	}

	/* we re-init the static copy on each call, since we don't know if the caller has
	   written into it. */
	alert_program_name2_copy = strdup(alert_program_name2);

	if (!alert_program_name2_copy) {
		report(LOG_ERR, "GetAlert_program_name2: strdup() failed (presumably a malloc error)");
		my_exit(1, 1, 1);
	}

    return alert_program_name2_copy;
}

