#ifndef INET_ATON_H
#define INET_ATON_H

#ifndef HAVE_INET_ATON_PROTO
# include	<sys/types.h>
# include	<netinet/in.h>
extern int inet_aton(const char *, struct in_addr *);
#endif /* not HAVE_INET_ATON_PROTO */

#endif /* not INET_ATON_H */
