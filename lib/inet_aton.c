/* Provide an inet_aton() routine for those systems that do not
   already have one in their libraries.

   It converts an IPv4 address in dot-quad ASCII format to struct in_addr format.
   User passes us a char * to the string in arg 1; and a struct in_addr * in
   arg 2 for us to use to return the address.  Return 1 on success, 0 on failure.

   This routine came from glibc 2.1.2 (the GNU C Library), which is covered
   by the GNU Library General Public License, Version 2.  See COPYING.LIB for details.
   
   The routine as distributed with that library includes several
   copyright statements, which are reproduced below.

*/


/*
 * ++Copyright++ 1983, 1990, 1993
 * -
 * Copyright (c) 1983, 1990, 1993
 *    The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 * 	This product includes software developed by the University of
 * 	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * -
 * Portions Copyright (c) 1993 by Digital Equipment Corporation.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies, and that
 * the name of Digital Equipment Corporation not be used in advertising or
 * publicity pertaining to distribution of the document or software without
 * specific, written prior permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND DIGITAL EQUIPMENT CORP. DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS.   IN NO EVENT SHALL DIGITAL EQUIPMENT
 * CORPORATION BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 * -
 * --Copyright--
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)inet_addr.c	8.1 (Berkeley) 6/17/93";
static char rcsid[] = "$Id: inet_addr.c,v 1.11 1999/04/29 18:19:53 drepper Exp $";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#ifdef _LIBC
# include <stdlib.h>
# include <limits.h>
# include <errno.h>
#endif

#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "inet_aton.h"

/* these are compatibility routines, not needed on recent BSD releases */

#if 0
/*
 * Ascii internet address interpretation routine.
 * The value returned is in network order.
 */
u_int32_t
inet_addr(cp)
	register const char *cp;
{
	struct in_addr val;

	if (inet_aton(cp, &val))
		return (val.s_addr);
	return (INADDR_NONE);
}
#endif

/*
 * Check whether "cp" is a valid ascii representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 * This replaces inet_addr, the return value from which
 * cannot distinguish between failure and a local broadcast address.
 */
int
inet_aton(const char *cp, struct in_addr *addr)
{
	static const u_int32_t max[4] = { 0xffffffff, 0xffffff, 0xffff, 0xff };
	register u_int32_t val;	/* changed from u_long --david */
#ifndef _LIBC
	register int base;
#endif
	register int n;
	register char c;
	u_int32_t parts[4];
	register u_int32_t *pp = parts;

#ifdef _LIBC
	int saved_errno = errno;
	__set_errno (0);
#endif

	memset (parts, '\0', sizeof (parts));

	c = *cp;
	for (;;) {
		/*
		 * Collect number up to ``.''.
		 * Values are specified as for C:
		 * 0x=hex, 0=octal, isdigit=decimal.
		 */
		if (!isdigit(c))
			goto ret_0;
#ifdef _LIBC
		{
			unsigned long ul = strtoul (cp, (char **) &cp, 0);
			if (ul == ULONG_MAX && errno == ERANGE)
				goto ret_0;
			if (ul > 0xfffffffful)
				goto ret_0;
			val = ul;
		}
		c = *cp;
#else
		base = 10;
		if (c == '0') {
			c = *++cp;
			if (c == 'x' || c == 'X')
				base = 16, c = *++cp;
			else
				base = 8;
		}
		val = 0;
		for (;;) {
			if (isascii(c) && isdigit(c)) {
				val = (val * base) + (c - '0');
				c = *++cp;
			} else if (base == 16 && isascii(c) && isxdigit(c)) {
				val = (val << 4) |
					(c + 10 - (islower(c) ? 'a' : 'A'));
				c = *++cp;
			} else
				break;
		}
#endif
		if (c == '.') {
			/*
			 * Internet format:
			 *	a.b.c.d
			 *	a.b.c	(with c treated as 16 bits)
			 *	a.b	(with b treated as 24 bits)
			 */
			if (pp >= parts + 3)
				goto ret_0;
			*pp++ = val;
			c = *++cp;
		} else
			break;
	}
	/*
	 * Check for trailing characters.
	 */
	if (c != '\0' && (!isascii(c) || !isspace(c)))
		goto ret_0;
	/*
	 * Concoct the address according to
	 * the number of parts specified.
	 */
	n = pp - parts + 1;

	if (n == 0	/* initial nondigit */
	    || parts[0] > 0xff || parts[1] > 0xff || parts[2] > 0xff
	    || val > max[n - 1])
	  goto ret_0;

	val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);

	if (addr)
		addr->s_addr = htonl(val);

#ifdef _LIBC
	__set_errno (saved_errno);
#endif
	return (1);

ret_0:
#ifdef _LIBC
	__set_errno (saved_errno);
#endif
	return (0);
}
