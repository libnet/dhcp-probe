/* Daemonize correctly.

   Based closely on daemon_init() from
   "UNIX Network Programming: Volume 1", Second Edition,
   by W. Richard Stevens.  That sample code is available via
   ftp://ftp.kohala.com/pub/rstevens/unpv12e.tar.gz.
*/


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "defs.h"

#include "daemonize.h"
#include "report.h"


void
daemonize(void)
{
	int i;
	pid_t pid;
	struct sigaction sa;

	if ((pid = fork()) < 0) {
		report(LOG_ERR, "fork: %s", get_errmsg());
		exit(1);
	} else if (pid != 0)
		exit(0); /* parent terminates */

	/* first child continues */

	setsid();	/* become a session leader */
		
	/* ignore HUP we will receive when session leader (first child) terminates */
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = SIG_IGN;
	if (sigaction(SIGHUP, &sa, NULL) < 0) {
		report(LOG_ERR, "sigaction: %s", get_errmsg());
		exit(1);
	}

	if ((pid = fork()) < 0) {
		report(LOG_ERR, "fork: %s", get_errmsg());
		exit(1);
	} else if (pid != 0)
		exit(0); /* first child terminates */

	/* second child continues */

	/* We take care of the chdir in the main program
	chdir("/");
	*/

	/* We'll allow ourself to inherit the file mode creation mask
	umask(0);
	*/

	/* Close inherited descriptors.
	   There is no portable way to determine what descriptors were inherited.  
	   There is also no portable way to determine the maximum number of file descriptors; presently
	   we'll rely on the non-portable OPEN_MAX constant.
	*/
	for (i = 0; i < OPEN_MAX; i++)
		close(i);

	return;
}
