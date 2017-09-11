AC_INIT([Salome2 Project DSCCODE module], [8.2.0], [paul.rascle@edf.fr], [SalomeDSCCODE])
AC_CONFIG_AUX_DIR(adm_local)
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS(dsccode_config.h)
AC_PROG_LIBTOOL

AC_PROG_CC
dnl C++ 
AC_PROG_CXX

CHECK_F77

echo
echo ---------------------------------------------
echo BOOST Library
echo ---------------------------------------------
echo

CHECK_BOOST

dnl Check Salome Install
CHECK_KERNEL

if test "x$Kernel_ok" = "xno"; then
  AC_MSG_ERROR([You must define a correct KERNEL_ROOT_DIR !])
fi

MACHINE=PCLINUX
AC_SUBST(MACHINE)

CHECK_OMNIORB

AC_CONFIG_FILES([
        Makefile
        idl/Makefile
        resources/Makefile
        src/Makefile
        src/DSCCODAENG/Makefile
        src/DSCCODBENG/Makefile
        src/DSCCODCENG/Makefile
        src/DSCCODDENG/Makefile
        src/NEUTRO/Makefile
        src/FLUIDE/Makefile
        src/SOLIDE/Makefile
        src/INTERPI/Makefile
        src/PYCOMPO/Makefile
        ])
AC_OUTPUT
