dnl Tell autoconf we're compiling a C++ library, using automake & libtool
AC_PREREQ([2.69])
AC_INIT([adplug], [2.3.2])
AC_CONFIG_MACRO_DIR([m4])

dnl Check whether we want to set defaults for CFLAGS, CXXFLAGS, CPPFLAGS and LDFLAGS
AC_MSG_CHECKING([whether configure should try to set CFLAGS/CXXFLAGS/CPPFLAGS/LDFLAGS])
AS_IF([test "x${CFLAGS+set}" = "xset" || test "x${CXXFLAGS+set}" = "xset" || test "x${CPPFLAGS+set}" = "xset" || test "x${LDFLAGS+set}" = "xset"], [
    enable_flags_setting=no
], [
    enable_flags_setting=yes
])
AC_MSG_RESULT([${enable_flags_setting}])

dnl Enable debugging on user request.
AX_CHECK_ENABLE_DEBUG([], [DEBUG])

AC_CONFIG_SRCDIR([src/adplug.cpp])
AM_INIT_AUTOMAKE([1.14 foreign dist-bzip2 no-dist-gzip subdir-objects])
LT_INIT

dnl Check for a sane C/C++ build environment.
AC_PROG_SED
AC_PROG_MKDIR_P
AC_PROG_MAKE_SET
AC_PROG_INSTALL

AC_PROG_CC
AC_LANG([C])
AX_COMPILER_VENDOR
AX_COMPILER_VERSION

AC_PROG_CXX
AC_LANG([C++])
AX_COMPILER_VENDOR
AX_COMPILER_VERSION

AC_C_BIGENDIAN

AS_IF([test "x${enable_flags_setting}" = "xyes" && test "x${enable_debug}" = "xno"], [
    AC_LANG([C])
    AX_APPEND_COMPILE_FLAGS([-O2 -pipe], [CFLAGS])

    AC_LANG([C++])
    AX_APPEND_COMPILE_FLAGS([-O2 -pipe], [CXXFLAGS])

    AS_CASE([${host_os}],
        [darwin*], [
            ldflags_test="-Wl,-dead_strip_dylibs"],
        [linux*], [
            ldflags_test="-Wl,-O1 -Wl,--as-needed -Wl,--no-undefined -Wl,--gc-sections"]
        )
    AX_APPEND_LINK_FLAGS([${ldflags_test}], [LDFLAGS])
])

dnl define $pkgconfigdir
AX_REQUIRE_DEFINED([PKG_INSTALLDIR])
PKG_INSTALLDIR

dnl Check for needed libraries.
PKG_CHECK_MODULES([libbinio], [libbinio >= 1.4])

dnl Check if getopt header is installed on this system
AC_CHECK_HEADERS([getopt.h], [], [AC_SUBST([GETOPT_OBJECTS], ['adplugdb/getopt.$(OBJEXT)'])])

dnl On some platforms glibc does not provide sys/io.h
dnl https://bugs.gentoo.org/645296
AC_CHECK_HEADERS([sys/io.h])

dnl Sanitize some compiler features, which may be broken...
AC_C_CONST
AC_C_INLINE

AC_CONFIG_FILES([
    Makefile
    src/version.h
    adplug.pc
])
AC_OUTPUT

AX_RECURSIVE_EVAL([$libdir], [full_absolute_libdir])
AX_RECURSIVE_EVAL([$bindir], [full_absolute_bindir])
AX_RECURSIVE_EVAL([$pkgconfigdir], [full_absolute_pkgconfigdir])
AC_MSG_RESULT([
-=-=-=-=-=-=-=-=-=-= Configuration Complete =-=-=-=-=-=-=-=-=-=-=-

  Configuration summary :

    adplug version : ...................... ${VERSION}

    Host CPU : ............................ ${host_cpu}
    Host Vendor : ......................... ${host_vendor}
    Host OS : ............................. ${host_os}

    CFLAGS : .............................. ${CFLAGS}
    CXXFLAGS : ............................ ${CXXFLAGS}
    CPPFLAGS : ............................ ${CPPFLAGS}
    LDFLAGS : ............................. ${LDFLAGS}

  Tools :

    C Compiler Vendor is : ................ ${ax_cv_c_compiler_vendor} (${ax_cv_c_compiler_version})
    CXX Compiler Vendor is : .............. ${ax_cv_cxx_compiler_vendor} (${ax_cv_cxx_compiler_version})

  Installation directories :

    Library directory : ................... ${full_absolute_libdir}
    Program directory : ................... ${full_absolute_bindir}
    Pkgconfig directory : ................. ${full_absolute_pkgconfigdir}
])
