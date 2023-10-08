# For replacement functions in lib/
AUTOMAKE_OPTIONS = subdir-objects

sbin_PROGRAMS = dhcp_probe

dhcp_probe_CFLAGS  = $(libpcap_CFLAGS) $(libnet_CFLAGS)
dhcp_probe_LDADD   = $(libpcap_LIBS) $(libnet_LIBS) $(LIBS) $(LIBOBJS)
dhcp_probe_SOURCES = bootp.c configfile.c dhcp_probe.c \
	get_myeaddr.c get_myipaddr.c  \
	daemonize.c open_max.c report.c utils.c \
	bootp.h configfile.h dhcp_probe.h \
	defaults.h defs.h \
	get_myeaddr.h get_myipaddr.h \
	daemonize.h open_max.h report.h utils.h
