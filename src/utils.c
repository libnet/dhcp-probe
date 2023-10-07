/* Miscellaneous utilities:
    open_for_writing()
    smalloc()
    open_append()
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "defs.h"

#include <stdio.h> /* for fdopen */
#include <sys/stat.h>  /* for stat, open*/
#include <errno.h>

#include <strings.h> 
#include <stdlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h> /* for setuid/seteuid/setgid/setegid, unlink */
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h> /* for open */
#else /* not HAVE_FCNTL_H */
#ifdef HAVE_SYS_FILE_H
#include <sys/file.h> /* for open, O_ constants defined here on BSD systems */
#endif /* HAVE_SYS_FILE_H */
#endif /* not HAVE_FCNTL_H */

#include <sys/stat.h> /* for stat, open */

#include "dhcp_probe.h"
#include "utils.h"
#include "report.h"


FILE *
open_for_writing(char * filename) 
{
/*	Open a regular file for writing.
	If the file already exists, it will be removed first.
	On error we return NULL and log an error message.
	Based closely on write_open() from BIND 8.1.2.
*/
	int fd;
	FILE *stream;
	struct stat stat_buffer;

	if (stat(filename, &stat_buffer) < 0) {
		if (errno != ENOENT) {
			report(LOG_ERR, "open_for_writing(): stat of %s failed: %s", filename, get_errmsg());
			return NULL;
		} 
	} else { /* filename already exists */
		if (!(stat_buffer.st_mode & S_IFREG)) {
			/* since we just want to open a regular file for writing, its existance as a special
			   file probably means something is wrong, so let's TRY to avoid removing it. */
			report(LOG_ERR, "open_for_writing(): %s exists but isn't a regular file", filename);
			return(NULL);
		}
	}

	/* At this point, filename either does not exist, or already exists as a regular file.
	   Either way, it's toast.  Note that it is possible for it's status to have changed since
	   we checked earlier; e.g. it may have changed from non-existance to existance as a special file.
	   So it's still possible we'll wipe out a special file, but we've made an effort to avoid it. */
	if (unlink(filename) < 0) {
		if (errno != ENOENT) {
			report(LOG_ERR, "open_for_writing(): unlink of %s failed: %s", filename, get_errmsg());
			return NULL;
		}
	}

	/* Use open() to get the atomic behavior provided by O_CREAT|O_EXCL.  If it succeeds,
	   convert the file descriptor into a file stream, which is what we're supposed to return. */
	fd = open(filename, O_WRONLY|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if (fd < 0) {
		report(LOG_ERR, "open_for_writing(): open of %s failed: %s", filename, get_errmsg());
		return NULL;
	}
	stream = fdopen(fd, "w");
	if (stream == NULL) {
		report(LOG_ERR, "open_for_writing(): fdopen failed: %s", get_errmsg());
		(void) close(fd);
		return NULL;
	}
	return(stream);
}


void *
smalloc(size_t size, int init)
{
/*	safe malloc()
	Always returns a valid pointer (if it returns at all).  The allocated
	memory is initialized to all zeros if 'init' is non-zero.  
	If malloc() returns an error, a
	message is printed using the report() function and the program aborts
	with a status of 1.
*/


	void *rc;

	rc = malloc(size);
	if (!rc) {
		report(LOG_ERR, "malloc() failed");
		my_exit(1, 1, 1);
	}

	if (init)
		bzero((char *) rc, size);

	return rc;
}


FILE *
open_append(char *filename)
{
/*	Open a regular file for appending.
	If the file does not exist, it will be created.
	On error we return NULL and log an error message.
	(Note that if logging is going to the same file we are trying to open, the error message may be lost.)
	Based closely on write_open() from BIND 8.1.2.
*/
	int fd;
	FILE *stream;
	struct stat stat_buffer;


	fd = open(filename, O_WRONLY|O_APPEND|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if (fd < 0) {
		report(LOG_ERR, "open_append(): open of %s failed: %s", filename, get_errmsg());
		return NULL;
	}

	/* Verify that the file (if it already exists) is a regular file.
	   We do this after the open to avoid a race condition.
	*/
	if (stat(filename, &stat_buffer) < 0) {
		if (errno != ENOENT) {
			close(fd);
			report(LOG_ERR, "open_append(): stat of %s failed; %s", filename, get_errmsg());
			return NULL;
		}
	} else { /* filename already exists */
		if (!(stat_buffer.st_mode & S_IFREG)) {
			/* since we just want to open a regular file for appending, its existance as a special
			   file probably means something is wrong, so let's TRY to avoid stepping on it. */
			close(fd);
			report(LOG_ERR, "open_append(): %s exists but isn't a regular file", filename);
			return NULL;
		}
	}

	stream = fdopen(fd, "a");
	if (stream == NULL) {
		report(LOG_ERR, "open_append(): fdopen failed: %s", get_errmsg());
		close(fd);
		return NULL;
	}
	return stream;
}
