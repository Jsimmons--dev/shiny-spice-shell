AC_INIT([shell],[1.0],[none])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
    Makefile
    src/Makefile
    tests/Makefile
])
AM_PATH_CHECK([0.11.0],[],[AC_MSG_ERROR([check is missing])])

AC_OUTPUT
