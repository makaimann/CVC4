# CVC4_CHECK_FOR_EXTMINISAT
# ------------------
# Look for external minisat and link it in, but only if user requested.
AC_DEFUN([CVC4_CHECK_FOR_EXTMINISAT], [
AC_MSG_CHECKING([whether user requested external minisat support])

have_libextminisat=0
EXTMINISAT_HOME=/home/makai/repos/forks/CVC4/minisat/
EXTMINISAT_LIBS=
EXTMINISAT_LDFLAGS=

have_libextminisat=0
if test "$with_extminisat" = no; then
  AC_MSG_RESULT([no, extminisat disabled by user])
elif test -n "$with_extminisat"; then
  AC_MSG_RESULT([yes, extminisat requested by user])
  AC_ARG_VAR(EXTMINISAT_HOME, [path to top level of extminisat source tree])
  AC_ARG_WITH(
    [extminisat-dir],
    AS_HELP_STRING(
      [--with-extminisat-dir=PATH],
      [path to top level of extminisat source tree]
    ),
    EXTMINISAT_HOME="$withval",
    [ if test -z "$EXTMINISAT_HOME" && ! test -e "$ac_abs_confdir/minisat/bin/minisat"; then
        AC_MSG_FAILURE([must give --with-extminisat-dir=PATH, define environment variable EXTMINISAT_HOME, or use contrib/get-extminisat to setup extminisat5 for CVC4!])
      fi
    ]
  )

  # Check if extminisat was installed via contrib/get-extminisat
  AC_MSG_CHECKING([whether extminisat was already installed via contrib/get-extminisat])
  if test -z "$EXTMINISAT_HOME" && test -e "$ac_abs_confdir/minisat/bin/minisat"; then
    EXTMINISAT_HOME="$ac_abs_confdir/minisat"
    AC_MSG_RESULT([yes, $EXTMINISAT_HOME])
  else
    AC_MSG_RESULT([no])
  fi

  if ! test -d "$EXTMINISAT_HOME" || ! test -x "$EXTMINISAT_HOME/bin/minisat" ; then
    AC_MSG_FAILURE([either $EXTMINISAT_HOME is not a minisat install tree or it's not yet built])
  fi

  AC_MSG_CHECKING([how to link extminisat])

  dnl TODO FIXME:
  dnl For some reason the CVC4_TRY_EXTMINISAT is not working correctly
  CVC4_TRY_EXTMINISAT_WITH([-pthread])
  CVC4_TRY_EXTMINISAT_WITH([-pthread -lm4ri])

  if test -z "$EXTMINISAT_LIBS"; then
    AC_MSG_FAILURE([cannot link against libminisat!])
  else
    AC_MSG_RESULT([$EXTMINISAT_LIBS])
    have_libextminisat=1
  fi

  EXTMINISAT_LDFLAGS="-L$EXTMINISAT_HOME/lib"

else
  AC_MSG_RESULT([no, user didn't request extminisat])
  with_extminisat=no
fi

])# CVC4_CHECK_FOR_EXTMINISAT

# CVC4_TRY_STATIC_EXTMINISAT_WITH(LIBS)
# ------------------------------
# Try AC_CHECK_LIB(extminisat) with the given linking libraries
AC_DEFUN([CVC4_TRY_EXTMINISAT_WITH], [
if test -z "$EXTMINISAT_LIBS"; then
  AC_LANG_PUSH([C++])

  cvc4_save_LIBS="$LIBS"
  cvc4_save_LDFLAGS="$LDFLAGS"
  cvc4_save_CPPFLAGS="$CPPFLAGS"

  LDFLAGS="-L$EXTMINISAT_HOME/lib"
  CPPFLAGS="$CPPFLAGS -I$EXTMINISAT_HOME/include"
  LIBS="-lminisat $1"

  AC_LINK_IFELSE(
    [AC_LANG_PROGRAM([[#include <minisast/simp/SimpSolver.h>]],
      [[Minisat::SimpSolver test()]])], [EXTMINISAT_LIBS="-lminisat $1"],
    [EXTMINISAT_LIBS=])

  LDFLAGS="$cvc4_save_LDFLAGS"
  CPPFLAGS="$cvc4_save_CPPFLAGS"
  LIBS="$cvc4_save_LIBS"

  AC_LANG_POP([C++])
fi
])# CVC4_TRY_EXTMINISAT_WITH
