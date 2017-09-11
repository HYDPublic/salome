# - Find gd library
# Sets the following variables:
#   LIBGD_FOUND        - TRUE if gd library is detected
#   LIBGD_INCLUDE_DIRS - path to the gd include directories
#   LIBGD_LIBRARIES    - path to the gd libraries to be linked against
#

#########################################################################
# Copyright (C) 2013-2016  OPEN CASCADE
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

# ------

IF(NOT LibGd_FIND_QUIETLY)
  MESSAGE(STATUS "Check for gd ...")
ENDIF()

# ------

SET(LIBGD_ROOT_DIR $ENV{LIBGD_ROOT_DIR})

IF(LIBGD_ROOT_DIR)
 LIST(APPEND CMAKE_PREFIX_PATH "${LIBGD_ROOT_DIR}")
ENDIF()

FIND_PATH(LIBGD_INCLUDE_DIRS gd.h)
FIND_LIBRARY(LIBGD_LIBRARIES NAMES gd)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBGD REQUIRED_VARS LIBGD_INCLUDE_DIRS LIBGD_LIBRARIES)

IF(LIBGD_FOUND)
  IF(NOT LibGd_FIND_QUIETLY)
    MESSAGE(STATUS "gd library: ${LIBGD_LIBRARIES}")
  ENDIF()
ENDIF()
