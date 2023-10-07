dnl ##################################################################
dnl The macro AC_CHECK_STRUCT_FOR is copyrighted by Wes Hardaker
dnl <wjhardaker@ucdavis.edu>, and was obtained from the GNU autoconf macro archive
dnl at http://research.cys.de/autoconf-archive/
dnl See the COPYING file for details.
dnl
dnl @synopsis AC_CHECK_STRUCT_FOR(INCLUDES,STRUCT,MEMBER,DEFINE,[no])
dnl
dnl Checks STRUCT for MEMBER and defines DEFINE if found.
dnl
dnl @version $Id: aclocal.m4,v 1.3 2001/01/17 01:20:29 irwin Exp $
dnl @author Wes Hardaker <wjhardaker@ucdavis.edu>
dnl
AC_DEFUN([AC_CHECK_STRUCT_FOR],[
ac_safe_struct=`echo "$2" | sed 'y%./+-%__p_%'`
ac_safe_member=`echo "$3" | sed 'y%./+-%__p_%'`
ac_safe_all="ac_cv_struct_${ac_safe_struct}_has_${ac_safe_member}"
changequote(, )dnl
  ac_uc_define=STRUCT_`echo "${ac_safe_struct}_HAS_${ac_safe_member}" | sed 'y%abcdefghijklmnopqrstuvwxyz./-%ABCDEFGHIJKLMNOPQRSTUVWXYZ___%'`
changequote([, ])dnl

AC_MSG_CHECKING([for $2.$3])
AC_CACHE_VAL($ac_safe_all,
[
if test "x$4" = "x"; then
  defineit="= 0"
elif test "x$4" = "xno"; then
  defineit=""
else
  defineit="$4"
fi
AC_TRY_COMPILE([
$1
],[
struct $2 testit;
testit.$3 $defineit;
], eval "${ac_safe_all}=yes", eval "${ac_safe_all}=no" )
])

if eval "test \"x$`echo ${ac_safe_all}`\" = \"xyes\""; then
  AC_MSG_RESULT(yes)
  AC_DEFINE_UNQUOTED($ac_uc_define)
else
  AC_MSG_RESULT(no)
fi
])


