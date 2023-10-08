/* Daemonize correctly.
 *
 * Based on daemon_init() from "UNIX Network Programming: Volume 1",
 * Second Edition, by W. Richard Stevens.
 *
 * Corresponding examples for the Third Edition, published at GitHub
 * <https://github.com/unpbook/unpv13e>, provide the following insight
 * into the license and rights to distribute of this particular code:
 *
 *     The code is available to anyone on the Internet and should
 *     compile easily on most current Unix systems.  The majority of
 *     the 10,000 lines of C code are functions that one can use as
 *     building blocks (a network programming toolchest) inside their
 *     own network applications.  Many of these functions help hide
 *     the differences between IPv4 and IPv6, and can aid the reader
 *     in developing protocol-independent code.
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "defs.h"

#include "daemonize.h"
#include "report.h"
#include "open_max.h"


void
daemonize(void)
{
	int i;
	pid_t pid;
	struct sigaction sa;

	if ((pid = fork()) < 0) {
		report(LOG_ERR, "fork: %s", get_errmsg());
		report(LOG_NOTICE, "exiting");
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
		report(LOG_NOTICE, "exiting");
		exit(1);
	}

	if ((pid = fork()) < 0) {
		report(LOG_ERR, "fork: %s", get_errmsg());
		report(LOG_NOTICE, "exiting");
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
	   so we'll instead try to close all descriptors up to some maximum value.
	*/
	for (i = 0; i < open_max(); i++)
		close(i);

	return;
}
