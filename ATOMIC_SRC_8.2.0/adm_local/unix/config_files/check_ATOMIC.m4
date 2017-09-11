# Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

AC_DEFUN([CHECK_ATOMIC],[

AC_CHECKING(for ATOMIC)

ATOMIC_ok=no

AC_ARG_WITH(atomic,
	    [  --with-atomic=DIR root directory path of ATOMIC module installation ],
	    ATOMIC_DIR="$withval",ATOMIC_DIR="")

if test "x$ATOMIC_DIR" == "x" ; then
   # no --with-atomic-dir option used
   if test "x$ATOMIC_ROOT_DIR" != "x" ; then
      # ATOMIC_ROOT_DIR environment variable defined
      ATOMIC_DIR=$ATOMIC_ROOT_DIR
   fi
fi

if test -f ${ATOMIC_DIR}/lib/salome/libATOMIC.so ; then
   ATOMIC_ok=yes
   AC_MSG_RESULT(Using ATOMIC module distribution in ${ATOMIC_DIR})

   if test "x$ATOMIC_ROOT_DIR" == "x" ; then
      ATOMIC_ROOT_DIR=${ATOMIC_DIR}
   fi
  
   ATOMIC_LDFLAGS=-L${ATOMIC_DIR}/lib${LIB_LOCATION_SUFFIX}/salome
   ATOMIC_CXXFLAGS=-I${ATOMIC_DIR}/include/salome
else
   AC_MSG_WARN("Cannot find valid ATOMIC module installation)
fi
  
AC_MSG_RESULT(for ATOMIC: $ATOMIC_ok)

AC_SUBST(ATOMIC_ROOT_DIR)
AC_SUBST(ATOMIC_LDFLAGS)
AC_SUBST(ATOMIC_CXXFLAGS)
 
])dnl
 
