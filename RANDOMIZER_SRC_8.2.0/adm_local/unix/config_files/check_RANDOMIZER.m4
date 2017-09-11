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

###########################################################
# File    : check_Randomizer.m4
# Author  : Vadim SANDLER (OCN)
# Created : 13/07/05
# Check availability of Randomizer binary distribution
###########################################################

AC_DEFUN([CHECK_RANDOMIZER],[

AC_CHECKING(for Randomizer)

Randomizer_ok=no

RANDOMIZER_CXXFLAGS=""
RANDOMIZER_LDFLAGS=""

AC_ARG_WITH(rnd,
	    [  --with-rnd=DIR root directory path of RANDOMIZER module installation ],
	    RANDOMIZER_DIR="$withval",RANDOMIZER_DIR="")

if test "x$RANDOMIZER_DIR" == "x" ; then
   # no --with-rnd-dir option used
   if test "x$RANDOMIZER_ROOT_DIR" != "x" ; then
      # RANDOMIZER_ROOT_DIR environment variable defined
      RANDOMIZER_DIR=$RANDOMIZER_ROOT_DIR
   fi
fi

if test -f ${RANDOMIZER_DIR}/idl/salome/Randomizer.idl ; then
   Randomizer_ok=yes
   AC_MSG_RESULT(Using Randomizer module distribution in ${RANDOMIZER_DIR})

   if test "x$RANDOMIZER_ROOT_DIR" == "x" ; then
      RANDOMIZER_ROOT_DIR=${RANDOMIZER_DIR}
   fi

   RANDOMIZER_LDFLAGS=-L${RANDOMIZER_ROOT_DIR}/lib${LIB_LOCATION_SUFFIX}/salome
   RANDOMIZER_CXXFLAGS=-I${RANDOMIZER_ROOT_DIR}/include/salome

   AC_SUBST(RANDOMIZER_ROOT_DIR)
   AC_SUBST(RANDOMIZER_LDFLAGS)
   AC_SUBST(RANDOMIZER_CXXFLAGS)
else
   AC_MSG_WARN("Cannot find Randomizer module sources")
fi
  
AC_MSG_RESULT(for Randomizer: $Randomizer_ok)
 
])dnl
 
