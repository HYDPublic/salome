# Copyright (C) 2007-2016  CEA/DEN, EDF R&D
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

# ---
#
# File   : check_GHS3DPRLPLUGIN.m4
# Author : Vadim SANDLER, Open CASCADE S.A.S (vadim.sandler@opencascade.com)
#
# ---

AC_DEFUN([CHECK_GHS3DPRLPLUGIN],[

AC_CHECKING(for GHS3DPRL mesh plugin)

GHS3DPRLplugin_ok=no

GHS3DPRLPLUGIN_LDFLAGS=""
GHS3DPRLPLUGIN_CXXFLAGS=""

AC_ARG_WITH(GHS3DPRLplugin,
	    [  --with-GHS3DPRLplugin=DIR root directory path of GHS3DPRL mesh plugin installation ])

if test "$with_GHS3DPRLplugin" != "no" ; then
    if test "$with_GHS3DPRLplugin" == "yes" || test "$with_GHS3DPRLplugin" == "auto"; then
	if test "x$GHS3DPRLPLUGIN_ROOT_DIR" != "x" ; then
            GHS3DPRLPLUGIN_DIR=$GHS3DPRLPLUGIN_ROOT_DIR
        fi
    else
        GHS3DPRLPLUGIN_DIR="$with_GHS3DPRLplugin"
    fi

    if test "x$GHS3DPRLPLUGIN_DIR" != "x" ; then
	if test -f ${GHS3DPRLPLUGIN_DIR}/lib${LIB_LOCATION_SUFFIX}/salome/libGHS3DPRLEngine.so ; then
	    GHS3DPRLplugin_ok=yes
	    AC_MSG_RESULT(Using GHS3DPRL mesh plugin distribution in ${GHS3DPRLPLUGIN_DIR})
	    GHS3DPRLPLUGIN_ROOT_DIR=${GHS3DPRLPLUGIN_DIR}
	    GHS3DPRLPLUGIN_LDFLAGS=-L${GHS3DPRLPLUGIN_DIR}/lib${LIB_LOCATION_SUFFIX}/salome
	    GHS3DPRLPLUGIN_CXXFLAGS=-I${GHS3DPRLPLUGIN_DIR}/include/salome
	else
	    AC_MSG_WARN("Cannot find compiled GHS3DPRL mesh plugin distribution")
	fi
    else
	AC_MSG_WARN("Cannot find compiled GHS3DPRL mesh plugin distribution")
    fi
fi

AC_MSG_RESULT(for GHS3DPRL mesh plugin: $GHS3DPRLplugin_ok)

AC_SUBST(GHS3DPRLPLUGIN_ROOT_DIR)
AC_SUBST(GHS3DPRLPLUGIN_LDFLAGS)
AC_SUBST(GHS3DPRLPLUGIN_CXXFLAGS)
 
])dnl
