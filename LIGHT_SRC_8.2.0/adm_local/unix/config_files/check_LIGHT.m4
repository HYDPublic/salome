dnl Copyright (C) 2005-2016  OPEN CASCADE
dnl
dnl This library is free software; you can redistribute it and/or
dnl modify it under the terms of the GNU Lesser General Public
dnl License as published by the Free Software Foundation; either
dnl version 2.1 of the License, or (at your option) any later version.
dnl
dnl This library is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl Lesser General Public License for more details.
dnl
dnl You should have received a copy of the GNU Lesser General Public
dnl License along with this library; if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
dnl
dnl See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
dnl

#  Check availability of LIGHT module binary distribution

AC_DEFUN([CHECK_LIGHT],[

AC_CHECKING(for Light)

Light_ok=no

LIGHT_LDFLAGS=""
LIGHT_CXXFLAGS=""

AC_ARG_WITH(gui,
	    --with-light=DIR root directory path of LIGHT installation,
	    LIGHT_DIR="$withval",LIGHT_DIR="")

if test "x$LIGHT_DIR" = "x" ; then

# no --with-light option used

  if test "x$LIGHT_ROOT_DIR" != "x" ; then

    # LIGHT_ROOT_DIR environment variable defined
    LIGHT_DIR=$LIGHT_ROOT_DIR

  else

    # search LIGHT binaries in PATH variable
    AC_PATH_PROG(TEMP, libLIGHT.so)
    if test "x$TEMP" != "x" ; then
      LIGHT_BIN_DIR=`dirname $TEMP`
      LIGHT_DIR=`dirname $LIGHT_BIN_DIR`
    fi

  fi
#
fi

if test -f ${LIGHT_DIR}/lib/salome/libLIGHT.so  ; then
  Light_ok=yes
  AC_MSG_RESULT(Using LIGHT distribution in ${LIGHT_DIR})

  if test "x$LIGHT_ROOT_DIR" == "x" ; then
    LIGHT_ROOT_DIR=${LIGHT_DIR}
  fi
  AC_SUBST(LIGHT_ROOT_DIR)

  LIGHT_LDFLAGS=-L${LIGHT_DIR}/lib${LIB_LOCATION_SUFFIX}/salome
  LIGHT_CXXFLAGS=-I${LIGHT_DIR}/include/salome

  AC_SUBST(LIGHT_LDFLAGS)
  AC_SUBST(LIGHT_CXXFLAGS)
else
  AC_MSG_WARN("Cannot find compiled LIGHT distribution")
fi
  
AC_MSG_RESULT(for LIGHT: $Light_ok)
 
])dnl
 
