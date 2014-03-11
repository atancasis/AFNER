dnl Process this file using autoconf to create a configure script

dnl Initialization
AC_INIT(AFNER, 0.1)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR(src/main.cpp)
AC_CONFIG_HEADERS(src/config.h)
AC_CONFIG_FILES([Makefile src/Makefile doc/Makefile yasmet/Makefile])

dnl Check for programs
AC_PROG_CXX
AC_HEADER_STDC
AC_LANG(C++)

dnl Add warnings
CPPFLAGS="-Wall $CPPFLAGS"

dnl Test if we are under the Cygwin environment
AC_ARG_WITH(boost-lib-suffix,
            [  --with-boost-lib-suffix=STRING  suffix used for boost
libraries],
            [boost_lib_suffix=$withval],
            )
BOOST_SUFFIX=$boost_lib_suffix
AC_CANONICAL_HOST
if test "$host_os" = cygwin; then
  BOOST_SUFFIX="-gcc-mt"
fi

dnl Boost
boost_include_path="/usr/include" dnl Path to the Boost include files.
AC_ARG_WITH(boost-includes,
            [  --with-boost-includes=DIR  Path to the Boost include files],
            [boost_include_path=$withval],
            )
if test -z $boost_include_path; then
  AC_MSG_ERROR([Boost support is required.  You must
specify a path to a valid installation of the Boost include files. This
can be done using --with-boost-includes=DIR, or try 'configure --help' for
the list of possible options.])
fi
CPPFLAGS="-I$boost_include_path $CPPFLAGS"

dnl Do the check
AC_CHECK_FILE($boost_include_path/boost/regex.hpp,
              ,
              AC_MSG_ERROR([Boost include files not found])
             )
AC_CHECK_FILE($boost_include_path/boost/program_options.hpp,
              ,
              AC_MSG_ERROR([Boost include files not found])
             )
AC_CHECK_FILE($boost_include_path/boost/filesystem/path.hpp,
              ,
              AC_MSG_ERROR([Boost include files not found])
             )
AC_CHECK_FILE($boost_include_path/boost/serialization/serialization.hpp,
              ,
              AC_MSG_ERROR([Boost include files not found])
             )

boost_libs_path="/usr/lib" dnl Path to the Boost lib files.
AC_ARG_WITH(boost-libs,
            [  --with-boost-libs=DIR  Path to the Boost lib files],
            [boost_libs_path=$withval],
            )
if test -z $boost_libs_path; then
  AC_MSG_ERROR([Boost support is required.  You must
specify a path to a valid installation of the Boost lib files. This
can be done using --with-boost-libs=DIR, or try 'configure --help' for
the list of possible options.])
fi
AC_SUBST(LDFLAGS)
LDFLAGS="-L$boost_libs_path $LDFLAGS"
AC_CHECK_FILE($boost_libs_path/libboost_regex$BOOST_SUFFIX.so,
              ,
   AC_CHECK_FILE($boost_libs_path/libboost_regex$BOOST_SUFFIX.a,
              ,
              AC_MSG_ERROR([Boost lib files not found])
   )
             )
LIBS="-lboost_regex$BOOST_SUFFIX $LIBS"
AC_CHECK_FILE($boost_libs_path/libboost_program_options$BOOST_SUFFIX.so,
              ,
   AC_CHECK_FILE($boost_libs_path/libboost_program_options$BOOST_SUFFIX.a,
              ,
              AC_MSG_ERROR([Boost lib files not found])
   )
             )
LIBS="-lboost_program_options$BOOST_SUFFIX $LIBS"
AC_CHECK_FILE($boost_libs_path/libboost_filesystem$BOOST_SUFFIX.so,
              ,
   AC_CHECK_FILE($boost_libs_path/libboost_filesystem$BOOST_SUFFIX.a,
              ,
              AC_MSG_ERROR([Boost lib files not found])
   )
             )
LIBS="-lboost_filesystem$BOOST_SUFFIX $LIBS"
AC_CHECK_FILE($boost_libs_path/libboost_serialization$BOOST_SUFFIX.so,
              ,
   AC_CHECK_FILE($boost_libs_path/libboost_serialization$BOOST_SUFFIX.a,
              ,
              AC_MSG_ERROR([Boost lib files not found])
   )
             )
LIBS="-lboost_serialization$BOOST_SUFFIX $LIBS"
AC_CHECK_FILE(/usr/include/gdbm.h,
              ,
              AC_MSG_ERROR([Gdbm include files not found])
             )
LIBS="-lgdbm $LIBS"

AC_SUBST([TEST_LIBS])
dnl End Boost

AC_OUTPUT
