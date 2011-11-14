AC_PREREQ([2.61])
AC_INIT([mysmartctl], [0.0], [http://reaktor42.de])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_LANG([C])
AC_PROG_CC_C99

AC_CONFIG_SRCDIR([src/mysmartctl.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile data/Makefile])

AC_HEADER_STDC
AC_CHECK_HEADERS([string.h strings.h getopt.h errno.h sys/stat.h fcntl.h \
                  termios.h sys/select.h unistd.h sys/time.h time.h signal.h])

AC_CHECK_HEADER([ncurses.h],
                [AC_DEFINE([HAVE_NCURSES_H], 1,
                    [Set var to 1 if it is available.])],
                [AC_MSG_ERROR([Cannot find the ncurses.h header file. Please install the ncurses development kit. E.g. in debian aptitude install libncurses<version>-dev.])])

AC_SEARCH_LIBS([curs_set],[ncurses],[],
              [AC_MSG_ERROR([Cannot find the ncurses library. Please install the ncurses library.])])
               

AM_CFLAGS="-Wall -pedantic"
AM_LDFLAGS="-Wl,--as-needed"
AC_SUBST(AM_CFLAGS)
AC_SUBST(AM_LDFLAGS)

AC_OUTPUT
