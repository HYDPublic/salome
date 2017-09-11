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
# File   : check_GHS3DPRL.m4
# Author : Vadim SANDLER, Open CASCADE S.A.S (vadim.sandler@opencascade.com)
#
# ---

AC_DEFUN([CHECK_GHS3DPRL],[

AC_REQUIRE([AC_PROG_CXX])dnl
AC_REQUIRE([AC_PROG_CXXCPP])dnl

AC_CHECKING(for GHS3DPRL(Tepal) commercial product)

AC_LANG_SAVE
AC_LANG_CPLUSPLUS

AC_ARG_WITH(,
	    [  --with-ghs3dprl=DIR     root directory path of GHS3DPRL (Tepal) installation])

GHS3DPRL_ok=no

if test "$with_ghs3dprl" == "no" ; then
    AC_MSG_WARN(You have choosen building plugin without GHS3DPRL)
else
    if test "$with_ghs3dprl" == "yes" || test "$with_ghs3dprl" == "auto"; then
        GHS3DPRL_HOME=""
    else
        GHS3DPRL_HOME="$with_ghs3dprl"
    fi
    
    if test "$GHS3DPRL_HOME" == "" ; then
        if test "x$GHS3DPRLHOME" != "x" ; then
            GHS3DPRL_HOME=$GHS3DPRLHOME
        else
            AC_MSG_WARN(Build plugin without GHS3DPRL)
        fi
    fi
    
    if test "x$GHS3DPRL_HOME" != "x"; then
    
      echo
      echo -------------------------------------------------
      echo You are about to choose to use somehow the
      echo "GHS3DPRL(Tepal) commercial product to generate 3D tetrahedral mesh."
      echo
    
      AC_CHECKING(for GHS3DPRL(Tepal) executable)
    
      AC_CHECK_PROG(GHS3DPRL,tepal,found)
    
      if test "x$GHS3DPRL" == x ; then
        AC_MSG_WARN(Tepal program not found in PATH variable)
        AC_MSG_WARN(Build plugin without GHS3DPRL)
      else
        GHS3DPRL_ok=yes
      fi
    
    fi
fi

AC_MSG_RESULT(for GHS3DPRL(Tepal): $GHS3DPRL_ok)
AC_LANG_RESTORE

])dnl
