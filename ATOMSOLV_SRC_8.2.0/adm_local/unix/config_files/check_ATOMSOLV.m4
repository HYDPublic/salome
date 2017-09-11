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

AC_DEFUN([CHECK_ATOMSOLV],[

AC_CHECKING(for ATOMSOLV)

ATOMSOLV_ok=no

AC_ARG_WITH(atomic,
	    [  --with-atomic=DIR root directory path of ATOMSOLV module installation ],
	    ATOMSOLV_DIR="$withval",ATOMSOLV_DIR="")

if test "x$ATOMSOLV_DIR" == "x" ; then
   # no --with-atomic-dir option used
   if test "x$ATOMSOLV_ROOT_DIR" != "x" ; then
      # ATOMSOLV_ROOT_DIR environment variable defined
      ATOMSOLV_DIR=$ATOMSOLV_ROOT_DIR
   fi
fi

if test -f ${ATOMSOLV_DIR}/lib/salome/libATOMSOLV.so ; then
   ATOMSOLV_ok=yes
   AC_MSG_RESULT(Using ATOMSOLV module distribution in ${ATOMSOLV_DIR})

   if test "x$ATOMSOLV_ROOT_DIR" == "x" ; then
      ATOMSOLV_ROOT_DIR=${ATOMSOLV_DIR}
   fi
   AC_SUBST(ATOMSOLV_ROOT_DIR)
else
   AC_MSG_WARN("Cannot find ATOMSOLV module sources")
fi
  
AC_MSG_RESULT(for ATOMSOLV: $ATOMSOLV_ok)
 
])dnl
 
