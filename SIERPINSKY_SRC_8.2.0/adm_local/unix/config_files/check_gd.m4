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

AC_DEFUN([CHECK_GDLIB],[
AC_REQUIRE([AC_PROG_CC])dnl
AC_REQUIRE([AC_PROG_CPP])dnl

AC_CHECKING(for gd library)

gd_ok=no

dnl check gd.h header
AC_CHECK_HEADER(gd.h, gd_ok=yes, gd_ok=no)

if test "x$gd_ok" = "xyes"; then
  dnl check gd library
  AC_MSG_CHECKING(linking with libgd.so)
  LIBS_old=$LIBS
  LIBS="$LIBS -lgd"
  AC_TRY_LINK([#include <gd.h>],
              [gdImagePtr image = gdImageCreate(10,10);],
              gd_ok=yes,
              gd_ok=no)
  LIBS=$LIBS_old
  AC_MSG_RESULT(done)
fi

if test "x$gd_ok" = "xno"; then
  AC_MSG_WARN(gd library is not installed!)
fi

AC_MSG_RESULT(for gd: $gd_ok)

])dnl
