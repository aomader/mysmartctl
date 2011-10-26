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
                  termios.h sys/select.h unistd.h sys/time.h time.h signal.h \
                  ncurses.h])

AM_CFLAGS="-Wall -pedantic"
AM_LDFLAGS="-Wl,--as-needed"
AC_SUBST(AM_CFLAGS)
AC_SUBST(AM_LDFLAGS)

AC_OUTPUT
