/* Routines associated with reporting/logging messages:
	report_init()
	report()
	close_and_reopen_log_file()
	get_errmsg()
*/


/*	This is based on report.c from Carnegie Mellon University (CMU) dhcpd 3.3.7.
	As a result, it is subject to copyright, see the COPYING file for details.
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "defs.h"
#include "defaults.h"

#include "dhcp_probe.h"
#include "report.h"
#include "utils.h"

#ifndef LOG_NDELAY
#define LOG_NDELAY	0
#endif
#ifndef LOG_DAEMON
#define LOG_DAEMON	0
#endif

extern int debug;
extern char *prog;

static const char *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

/* These are  initialized so you get only stderr until you call report_init() */
static int use_stderr = 1;
static int use_syslog = 0;
static int use_logfile = 0;

static FILE *logfile;  /* only used if we use a logfile */


void
report_init(int dont_fork, char *logfile_name)
{
/*	Call this once before using the the report() function.
	Set dont_fork to true if the daemon is not forking.
	Set logfile_name to the log file (fully-qualified pathname) or NULL.

	If dont_fork is set, we log to stderr, and ignore any logfile_name.
	If dont_fork is unset, we log to logfile_name if that's set, else to syslog().
*/

	if (dont_fork) {
		use_stderr = 1;
		use_syslog = 0;
		use_logfile = 0;
	} else {
		use_stderr = 0;
		if (logfile_name) {
			use_syslog = 0;
			use_logfile = 1;
		} else {
			use_syslog = 1;
			use_logfile = 0;
		}
	}

	if (use_syslog) {
		openlog(prog, LOG_PID | LOG_NDELAY, LOG_FACILITY);
	}

	if (use_logfile) {
		/* Because open_append() might call report(), we must reset logging to use just stderr
		   for the duration of this call, to avoid using logfile before it is ready.  
		   (The messages it passes to report() may still get lost if stderr has been closed.)
		   Then we restore logging afterwards.
		*/

		int save_use_logfile, save_use_stderr;

		/* save */
		save_use_logfile = use_logfile;
		save_use_stderr = use_stderr;

		use_logfile = 0;
		use_stderr = 1;
		if ((logfile = open_append(logfile_name)) == NULL) {
			/* We can't use report() to report this error, and stderr may be closed.
			   At least we can call syslog directly, so it hopefully this logged somewhere. */
			syslog(LOG_ERR, "%s: report_init(): error opening logfile %s\n", prog, logfile_name);
			my_exit(1, 1, 1);
		}

		/* restore */
		use_logfile = save_use_logfile;
		use_stderr = save_use_stderr;

		/* log file should be line buffered to avoid lengthy delays in logging */
		setvbuf(logfile, NULL, _IOLBF, 0);
	}
}


void
close_and_reopen_log_file(char *logfile_name)
{
/*	Close and reopen logfile.
	If we are not logging to a logfile, return silently.
	Returns on success, exits on error.
*/

	if (use_logfile) {

		if (debug > 1)
			report(LOG_NOTICE, "closing logfile");

		if (fclose(logfile) < 0) {
			/* We can't use report() to report this error, and stderr may be closed.
			   At least we can call syslog directly, so it hopefully this logged somewhere. */
			syslog(LOG_ERR, "%s: close_and_reopen_log_file(): error closing logfile\n", prog);
			my_exit(1, 1, 1);
		}
		if ((logfile = open_append(logfile_name)) == NULL) {
			/* We can't use report() to report this error, and stderr may be closed.
			   At least we can call syslog directly, so it hopefully this logged somewhere. */
			syslog(LOG_ERR, "%s: close_and_reopen_log_file(): can't open logfile %s\n", prog, logfile_name);
			my_exit(1, 1, 1);
		}
		/* log file should be line buffered to avoid lengthy delays in logging */
		setvbuf(logfile, NULL, _IOLBF, 0);

		if (debug > 1)
			report(LOG_NOTICE, "re-opened logfile");
	}

	return;
}


static char *levelnames[] = {
#ifdef LOG_SALERT
	"level: ",
	"alert: ",
	"alert: ",
	"emerg: ",
	"error: ",
	"crit:  ",
	"warn:  ",
	"note:  ",
	"info:  ",
	"debug: ",
	"unknownlevel: "
#else /* not LOG_SALERT */
	"emerg: ",
	"alert: ",
	"crit:  ",
	"error: ",
	"warn:  ",
	"note:  ",
	"info:  ",
	"debug: ",
	"unknownlevel: "
#endif /* not LOG_SALERT */
};
static int numlevels = sizeof(levelnames) / sizeof(levelnames[0]);


#ifdef STDC_HEADERS
void
report(int priority, char *fmt,...)
#else /* not STDC_HEADERS */
/*VARARGS2*/
void
report(priority, fmt, va_alist)
	int priority;
	char *fmt;
	va_dcl
#endif /* not STDC_HEADERS */
{
/*	Report errors and such via stderr and syslog() and logfile if
	appropriate.  It just helps avoid a lot of "#ifdef SYSLOG" constructs
	from being scattered throughout the code.

	The syntax is identical to syslog(3), but %m is not considered special
	for output to stderr (i.e. you'll see "%m" in the output. . .).  Also,
	control strings should normally end with \n since newlines aren't
	automatically generated for stderr and logfile output (whereas syslog strips out all
	newlines and adds its own at the end).
*/

	va_list ap;
	static char buf[256];

	if ((priority < 0) || (priority >= numlevels)) {
		priority = numlevels - 1;
	}
#ifdef STDC_HEADERS
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
#ifdef HAVE_VSNPRINTF
	vsnprintf(buf, sizeof(buf), fmt, ap);
#else
	vsprintf(buf, fmt, ap);
#endif /* not HAVE_VSNPRINTF */
	va_end(ap);

	/*
	 * Print the message
	 */

	if (use_stderr)
		/* omit program name and timestamp, add priority level */
		fprintf(stderr, "%s %s\n", levelnames[priority], buf);

	if (use_syslog)
		/* sylog prepends its own timestamp and the program name we set in 'openlog' */
		syslog((priority | LOG_FACILITY), "%s", buf);

	if (use_logfile) {
		/* omit program name, add priority level and timestamp */

		char time_buf[256];
		struct timeval tv;
		struct tm *local_tm = NULL;

		/* retrieve current local time and format it */
		time_buf[0] = '\0'; /* so we can safely print it even if we fail to set it */
		if (gettimeofday(&tv, NULL) < 0) {
			/* we can't report error since we're already in report() */
		} else {
			local_tm = localtime((time_t *) &tv.tv_sec);
		}
		if (local_tm != NULL) {
			sprintf(time_buf, "%02d-%s-%4d %02d:%02d:%02d ",
				local_tm->tm_mday, 
				months[local_tm->tm_mon],
				local_tm->tm_year+1900, 
				local_tm->tm_hour,
				local_tm->tm_min, 
				local_tm->tm_sec);
		}
		fprintf(logfile, "%s%s %s\n", time_buf, levelnames[priority], buf);
	}
}


char *
get_errmsg()
{
/*	Return pointer to static string which gives full filesystem error message.
*/
	extern int errno;
	extern char *strerror();

	return strerror(errno);
}
