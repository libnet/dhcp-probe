/*
	Attempt to return the maximum number of descriptors in semi portable way.
	If we encounter difficulties, we'll return a guess.
*/

/*
   Based on open_max() from
   "Advanced Programming in the UNIX Environment", Second Edition,
   by W. Richard Stevens, Stephen Rago.
*/


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "defs.h"

#include "open_max.h"

/* If we fail to obtain a reasonable value, use this guess.
   There's no guarantee this is large enough.
*/
#define OPEN_MAX_GUESS 256

/* compute this only once */
static long openmax = 0;

long
open_max(void)
{

	if (openmax == 0) {

		/* first time through */

#ifdef HAVE_SYSCONF
#ifdef _SC_OPEN_MAX
		/* Attempt to obtain the value via sysconf. */
		if ((openmax = sysconf(_SC_OPEN_MAX)) < 0) {
			/* Regardless of the cause of the failure, discard the value. */
			openmax = 0;
		}
#endif /* _SC_OPEN_MAX */
#endif /* HAVE_SYSCONF */

#ifdef HAVE_GETRLIMIT
#ifdef RLIMIT_NOFILE
		if (openmax == 0) {
			/* We (still) don't have a value. */
			/* Attempt to obtain the value via getrlimit. */

			struct rlimit rlim;

			if (getrlimit(RLIMIT_NOFILE, &rlim) == 0) {
				/* Don't accept the value if it means "no limit." */
				if (rlim.rlim_max != RLIM_INFINITY)
					openmax = rlim.rlim_max;
			}
		}
#endif /* RLIMIT_NOFILE */
#endif /* HAVE_GETRLIMIT */

		if (openmax == 0) {
			/* We (still) don't have a value. */

#ifdef OPEN_MAX
			/* OPEN_MAX is not portable. */
			openmax = OPEN_MAX;
#else
			openmax = OPEN_MAX_GUESS;
#endif
		}

	}

	return(openmax);
}
