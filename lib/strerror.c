/* Provide a strerror() routine for those systems that
   don't already have one.

   A portion of this code is subject to the GNU General Public License;
   see the comments below.  

*/


/*
 * strerror() - for those systems that don't have it yet.
 * We'll return a string from sys_errlist[] if your system has it.
 * Else we'll just return "Error %d", filling in the errno.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "strerror.h"

/*
 * Check for if sys_errlist[] and sys_nerr exist and are defined.
 * This section of the code came from configure.in distributed with LPRng-2.0.8.
 * It is subject to the GNU General Public License, see the COPYING for
 * the GNU General Public License.
*/
#if defined(HAVE_SYS_NERR)
#   if !defined(HAVE_SYS_NERR_DECL)
      extern int sys_nerr;
#   endif
#   define num_errors    (sys_nerr)
# else
#   define num_errors    (-1)            /* always use "Error %d" */
#endif
#if defined(HAVE_SYS_ERRLIST)
#  if !defined(HAVE_SYS_ERRLIST_DECL)
    extern const char *const sys_errlist[];
#  endif
# else
#  undef  num_errors
#  define num_errors   (-1)            /* always use "Error %d" */
#endif





static char errmsg[80];	/* holds "Error %d" */

char *
strerror(en)
	int en;
{
#ifdef HAVE_SYS_ERRLIST
	if ((0 <= en) && (en < num_errors))
		return (char *) sys_errlist[en]; /* cast away const since strerror(3) isn't declared that way */
#endif

	/* SAFE use of sprintf */
	sprintf(errmsg, "Error %d", en);
	return errmsg;
}
