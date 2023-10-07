#ifndef CONFIGFILE_H
#define CONFIGFILE_H


/* (re)read the configuration file */
int read_configfile(const char *fname);

/* accessor functions for all the configuration file data */
struct ether_addr * GetChaddr (void);
struct ether_addr * GetEther_src (void);
struct in_addr * GetClient_ip_address(void);
struct in_addr * GetServer_id(void);
unsigned GetCycle_time(void);
int GetResponse_wait_time(void);
int isLegalServersMember(struct in_addr *ipaddr);
int isLegalServerEthersrcsMember(struct ether_addr *eaddr);
int isInLeaseNetworksOfConcern(struct in_addr *ipaddr);

char * GetAlert_program_name(void);
char * GetAlert_program_name2(void);




#endif /* not CONFIGFILE_H */

