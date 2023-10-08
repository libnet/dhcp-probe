#ifndef INET_ATON_H
#define INET_ATON_H

#ifndef HAVE_INET_ATON
# include	<sys/types.h>
# include	<netinet/in.h>
extern int inet_aton(const char *, struct in_addr *);
#endif

#endif /* INET_ATON_H */
