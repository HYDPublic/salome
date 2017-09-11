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

#  Check availability of SIERPINSKY binary distribution
#
#  Author : Marc Tajchman (CEA, 2002)
#------------------------------------------------------------

AC_DEFUN([CHECK_SIERPINSKY],[

AC_CHECKING(for Sierpinsky)

Sierpinsky_ok=no

AC_ARG_WITH(sier,
	    --with-sierpinsky=DIR  root directory path of SIERPINSKY build or installation,
	    SIERPINSKY_DIR="$withval",SIERPINSKY_DIR="")

if test "x$SIERPINSKY_DIR" = "x" ; then

# no --with-gui-dir option used

  if test "x$SIERPINSKY_ROOT_DIR" != "x" ; then

    # SALOME_ROOT_DIR environment variable defined
    SIERPINSKY_DIR=$SIERPINSKY_ROOT_DIR

  else

    # search Salome binaries in PATH variable
    AC_PATH_PROG(TEMP, libSIERPINSKY.so)
    if test "x$TEMP" != "x" ; then
      SIERPINSKY_DIR=`dirname $TEMP`
    fi

  fi

fi

if test -f ${SIERPINSKY_DIR}/lib${LIB_LOCATION_SUFFIX}/salome/libSIERPINSKY.so  ; then
  Sierpinsky_ok=yes
  AC_MSG_RESULT(Using SIERPINSKY module distribution in ${SIERPINSKY_DIR})

  if test "x$SIERPINSKY_ROOT_DIR" == "x" ; then
    SIERPINSKY_ROOT_DIR=${SIERPINSKY_DIR}
  fi
  
  SIERPINSKY_LDFLAGS=-L${SIERPINSKY_ROOT_DIR}/lib${LIB_LOCATION_SUFFIX}/salome
  SIERPINSKY_CXXFLAGS=-I${SIERPINSKY_ROOT_DIR}/include/salome

  AC_SUBST(SIERPINSKY_ROOT_DIR)
  AC_SUBST(SIERPINSKY_LDFLAGS)
  AC_SUBST(SIERPINSKY_CXXFLAGS)

else
  AC_MSG_WARN("Cannot find compiled SIERPINSKY module distribution")
fi
  
AC_MSG_RESULT(for SIERPINSKY: $Sierpinsky_ok)
 
])dnl
 
