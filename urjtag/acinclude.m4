dnl @synopsis VL_LIB_READLINE
dnl
dnl Searches for a readline compatible library.  If found, defines
dnl `HAVE_LIBREADLINE'.  If the found library has the `add_history'
dnl function, sets also `HAVE_READLINE_HISTORY'.  Also checks for the
dnl locations of the necessary include files and sets `HAVE_READLINE_H'
dnl or `HAVE_READLINE_READLINE_H' and `HAVE_READLINE_HISTORY_H' or
dnl 'HAVE_HISTORY_H' if the corresponding include files exists.
dnl
dnl The libraries that may be readline compatible are `libedit',
dnl `libeditline' and `libreadline'.  Sometimes we need to link a termcap
dnl library for readline to work, this macro tests these cases too by
dnl trying to link with `libtermcap', `libcurses', `libncurses' or
dnl `libtinfo' before giving up.
dnl
dnl Here is an example of how to use the information provided by this
dnl macro to perform the necessary includes or declarations in a C file:
dnl
dnl   #ifdef HAVE_LIBREADLINE
dnl   #  if defined(HAVE_READLINE_READLINE_H)
dnl   #    include <readline/readline.h>
dnl   #  elif defined(HAVE_READLINE_H)
dnl   #    include <readline.h>
dnl   #  else /* !defined(HAVE_READLINE_H) */
dnl   extern char *readline ();
dnl   #  endif /* !defined(HAVE_READLINE_H) */
dnl   char *cmdline = NULL;
dnl   #else /* !defined(HAVE_READLINE_READLINE_H) */
dnl     /* no readline */
dnl   #endif /* HAVE_LIBREADLINE */
dnl
dnl   #ifdef HAVE_READLINE_HISTORY
dnl   #  if defined(HAVE_READLINE_HISTORY_H)
dnl   #    include <readline/history.h>
dnl   #  elif defined(HAVE_HISTORY_H)
dnl   #    include <history.h>
dnl   #  else /* !defined(HAVE_HISTORY_H) */
dnl   extern void add_history ();
dnl   extern int write_history ();
dnl   extern int read_history ();
dnl   #  endif /* defined(HAVE_READLINE_HISTORY_H) */
dnl     /* no history */
dnl   #endif /* HAVE_READLINE_HISTORY */
dnl
dnl
dnl @version 1.1
dnl @author Ville Laurikari <vl@iki.fi>
dnl @author Ville Voipio <vv@iki.fi>; check for readline completion (not available in, e.g. Leopard)

AC_DEFUN([VL_LIB_READLINE], [
  AC_ARG_WITH([readline],
    [AS_HELP_STRING([--without-readline],
      [support command completion/history])])
  AS_IF([test "x$with_readline" = "xno"], [
    vl_cv_lib_readline=no
  ])

  AC_CACHE_CHECK([for a readline compatible library],
                 vl_cv_lib_readline, [
    ORIG_LIBS="$LIBS"
    for readline_lib in readline edit editline; do
      for termcap_lib in "" termcap curses ncurses tinfo; do
        if test -z "$termcap_lib"; then
          TRY_LIB="-l$readline_lib"
        else
          TRY_LIB="-l$readline_lib -l$termcap_lib"
        fi
        LIBS="$ORIG_LIBS $TRY_LIB"
        AC_TRY_LINK_FUNC(readline, vl_cv_lib_readline="$TRY_LIB")
        if test -n "$vl_cv_lib_readline"; then
          break
        fi
      done
      if test -n "$vl_cv_lib_readline"; then
        break
      fi
    done
    if test -z "$vl_cv_lib_readline"; then
      vl_cv_lib_readline="no"
      LIBS="$ORIG_LIBS"
    fi
  ])

  if test "$vl_cv_lib_readline" != "no"; then
    AC_DEFINE(HAVE_LIBREADLINE, 1,
              [Define if you have a readline compatible library])
    AC_CHECK_HEADERS(readline.h readline/readline.h)
    AC_CACHE_CHECK([whether readline supports history],
                   vl_cv_lib_readline_history, 
                   [vl_cv_lib_readline_history="no"
                    AC_TRY_LINK_FUNC(add_history, vl_cv_lib_readline_history="yes")])
    if test "$vl_cv_lib_readline_history" = "yes"; then
      AC_DEFINE(HAVE_READLINE_HISTORY, 1, [Define if your readline library has \`add_history'])
      AC_CHECK_HEADERS(history.h readline/history.h)
    fi

    AC_CACHE_CHECK([whether readline supports completion],
                   vl_cv_lib_readline_completion, 
                   [vl_cv_lib_readline_completion="no"
                    AC_TRY_LINK_FUNC(rl_completion_matches, vl_cv_lib_readline_completion="yes")])
    if test "$vl_cv_lib_readline_completion" = "yes"; then
      AC_DEFINE(HAVE_READLINE_COMPLETION, 1, [Define if your readline library has \`rl_completion_matches'])
    fi
  elif test "$with_readline$vl_cv_lib_readline" = "yesno" ; then
    AC_ERROR([readline support requested, but none found])
  fi

])dnl

# ACI_PROG_SED
# ------------
# Check for a fully functional sed program that truncates
# as few characters as possible.  Prefer GNU sed if found.
#
# Copyright (C) Free Software Foundation
# 
# Copied here from autoconf-2.60 programs.m4 (AC_PROG_SED) to maintain
# compatibility with autoconf-2.59. Can be removed from acinclude.m4 if
# autoconf-2.60 or newer is required for other reasons.. 
#
m4_ifndef([AC_PROG_SED],[dnl
AC_DEFUN([AC_PROG_SED],
[AC_CACHE_CHECK([for a sed that does not truncate output], ac_cv_path_SED,
    [dnl ac_script should not contain more than 99 commands (for HP-UX sed),
     dnl but more than about 7000 bytes, to catch a limit in Solaris 8 /usr/ucb/sed.
     ac_script=s/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb/
     for ac_i in 1 2 3 4 5 6 7; do
       ac_script="$ac_script$as_nl$ac_script"
     done
     echo "$ac_script" | sed 99q >conftest.sed
     $as_unset ac_script || ac_script=
     _AC_PATH_PROG_FEATURE_CHECK(SED, [sed gsed],
	[_AC_FEATURE_CHECK_LENGTH([ac_path_SED], [ac_cv_path_SED],
		["$ac_path_SED" -f conftest.sed])])])
 SED="$ac_cv_path_SED"
 AC_SUBST([SED])dnl
 rm -f conftest.sed
])# ACI_PROG_SED
])dnl


dnl AS_VAR_PREPEND
dnl -------------
dnl
dnl this isn't in autoconf (yet?)
dnl
m4_ifndef([AS_VAR_PREPEND],[dnl
AC_DEFUN([AS_VAR_PREPEND], [$1=$2${$1}])
])dnl


dnl AS_VAR_APPEND
dnl -------------
dnl
dnl this was added in autoconf-2.64+
dnl
m4_ifndef([AS_VAR_APPEND],[dnl
AC_DEFUN([AS_VAR_APPEND], [$1=${$1}$2])
])dnl


dnl m4_ifnblank
dnl -----------
dnl
dnl this was added in autoconf-2.64+
dnl
m4_ifndef([m4_ifnblank],[dnl
m4_define([m4_ifnblank], [m4_ifval(m4_normalize([$1]), [$2], [$3])])
])dnl


dnl LT_INIT
dnl -------
dnl
dnl this was added in libtool-2
dnl
m4_ifndef([LT_INIT],[dnl
m4_define([LT_INIT],[AC_PROG_LIBTOOL])
])dnl
