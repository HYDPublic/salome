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

AC_DEFUN([CHECK_ATOMGEN],[

AC_CHECKING(for ATOMGEN)

ATOMGEN_ok=no

AC_ARG_WITH(atomgen,
	    [  --with-atomgen=DIR root directory path of ATOMGEN module installation ],
	    ATOMGEN_DIR="$withval",ATOMGEN_DIR="")

if test "x$ATOMGEN_DIR" == "x" ; then
   # no --with-atomgen-dir option used
   if test "x$ATOMGEN_ROOT_DIR" != "x" ; then
      # ATOMGEN_ROOT_DIR environment variable defined
      ATOMGEN_DIR=$ATOMGEN_ROOT_DIR
   fi
fi

if test -f ${ATOMGEN_DIR}/bin/salome/ATOMGEN.py ; then
   ATOMGEN_ok=yes
   AC_MSG_RESULT(Using ATOMGEN module distribution in ${ATOMGEN_DIR})

   if test "x$ATOMGEN_ROOT_DIR" == "x" ; then
      ATOMGEN_ROOT_DIR=${ATOMGEN_DIR}
   fi
   AC_SUBST(ATOMGEN_ROOT_DIR)
else
   AC_MSG_WARN("Cannot find ATOMGEN module sources")
fi
  
AC_MSG_RESULT(for ATOMGEN: $ATOMGEN_ok)
 
])dnl
 
