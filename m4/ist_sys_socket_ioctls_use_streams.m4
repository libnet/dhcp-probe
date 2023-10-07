dnl ##################################################################
dnl The macro IST_SYS_SOCKET_IOCTLS_USE_STREAMS
dnl is by Irwin Tillman, and may be distributed under the GNU Public License.
dnl See the COPYING file for details.
dnl
dnl Version $Id: ist_sys_socket_ioctls_use_streams.m4,v 1.4 2008/09/17 17:47:03 root Exp $
dnl
dnl Usage:
dnl    IST_SYS_SOCKET_IOCTLS_USE_STREAMS
dnl Defines SYS_SOCKET_IOCTLS_USE_STREAMS if this system requires you to use STREAMS ioctls on sockets 
dnl instead of SIOCxxx ioctls on sockets.
dnl Also sets the cache variable ist_cv_sys_socket_ioctls_use_streams to 'yes' or 'no'.
dnl (Side effects: also defines cache variable ac_cv_header_stropts_h. and C preprocessor macro 
dnl HAVE_STROPTS_H based on whether stropts.h header is present.)
dnl
dnl Our approach is to guess that if your systems has stropts.h header (STREAMS user options definitions) 
dnl and is on a short list of known host platforms, that we should use STREAMS ioctls.  
dnl (Having a list of host platforms here isn't good; we really should try compiling and running a 
dnl test program instead.)
dnl
AC_DEFUN([IST_SYS_SOCKET_IOCTLS_USE_STREAMS], [
	dnl Check for stropts.h just for the side-effect of setting ac_cv_header_stropts_h.
	AC_CHECK_HEADERS(stropts.h)

	dnl We need the $host variable to have been set
	AC_REQUIRE([AC_CANONICAL_HOST])

	AC_CACHE_CHECK(if socket ioctls should use STREAMS instead of SIOCxxx, ist_cv_sys_socket_ioctls_use_streams, [
		if test "$ac_cv_header_stropts_h" = "yes"; then
			dnl stropts.h present, check for particular platform flavors
			case "$host" in
				*-sni-sysv*)	ist_cv_sys_socket_ioctls_use_streams=yes ;;
				*-*-ptx*)		ist_cv_sys_socket_ioctls_use_streams=yes ;;
	    		*) 				ist_cv_sys_socket_ioctls_use_streams=no  ;;
			esac
		else
			dnl stropts.h missing, assume we don't need to use stream ioctls
			ist_cv_sys_socket_ioctls_use_streams=no 
		fi
	])

	if test "$ist_cv_sys_socket_ioctls_use_streams" = "yes"; then
 		AC_DEFINE(SYS_SOCKET_IOCTLS_USE_STREAMS, [], [Define if STREAMS ioctls should be used on socket descriptors, instead of SIOCxxx ioctls])
	fi
])
