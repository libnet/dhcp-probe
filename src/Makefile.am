## Process this file with automake to produce Makefile.in

sbin_PROGRAMS = dhcp_probe

dhcp_probe_SOURCES = bootp.c configfile.c dhcp_probe.c \
	get_myeaddr.c get_myipaddr.c  \
	daemonize.c open_max.c report.c utils.c \
	bootp.h configfile.h dhcp_probe.h \
	defaults.h defs.h \
	get_myeaddr.h get_myipaddr.h \
	daemonize.h open_max.h report.h utils.h

LDADD = $(top_builddir)/lib/libreplace.a



