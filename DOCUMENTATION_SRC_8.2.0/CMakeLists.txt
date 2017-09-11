# Copyright (C) 2012-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.8 FATAL_ERROR)
INCLUDE(CMakeDependentOption)

PROJECT(SalomeDOCUMENTATION)

# Versioning
# ========
# Project name, upper case
STRING(TOUPPER ${PROJECT_NAME} PROJECT_NAME_UC)

SET(${PROJECT_NAME_UC}_MAJOR_VERSION 8)
SET(${PROJECT_NAME_UC}_MINOR_VERSION 2)
SET(${PROJECT_NAME_UC}_PATCH_VERSION 0)
SET(${PROJECT_NAME_UC}_VERSION
  ${${PROJECT_NAME_UC}_MAJOR_VERSION}.${${PROJECT_NAME_UC}_MINOR_VERSION}.${${PROJECT_NAME_UC}_PATCH_VERSION})
SET(${PROJECT_NAME_UC}_VERSION_DEV 0)

# Common CMake macros
# ===================
SET(CONFIGURATION_ROOT_DIR $ENV{CONFIGURATION_ROOT_DIR} CACHE PATH "Path to the Salome CMake configuration files")
IF(EXISTS ${CONFIGURATION_ROOT_DIR})
  LIST(APPEND CMAKE_MODULE_PATH "${CONFIGURATION_ROOT_DIR}/cmake")
  INCLUDE(SalomeMacros)
ELSE()
  MESSAGE(FATAL_ERROR "We absolutely need the Salome CMake configuration files, please define CONFIGURATION_ROOT_DIR !")
ENDIF()

INCLUDE(SalomeSetupPlatform)
INCLUDE(SalomeMacros)

# Find prerequisites needed to documentation generation
# ==================================================
FIND_PACKAGE(SalomePythonInterp)
FIND_PACKAGE(SalomePythonLibs)
FIND_PACKAGE(SalomeDoxygen)
FIND_PACKAGE(SalomeGraphviz)
FIND_PACKAGE(SalomeSphinx)

# Directories
# =========
SET(SALOME_INSTALL_DOC_GUI   gui   CACHE PATH "Install path: user documentation")
SET(SALOME_INSTALL_DOC_TUI   tui   CACHE PATH "Install path: developer documentation")
SET(SALOME_INSTALL_DOC_DEV   dev   CACHE PATH "Install path: additional developer documentation")
SET(SALOME_INSTALL_DOC_EXTRA extra CACHE PATH "Install path: extra documentation")

MARK_AS_ADVANCED(
 SALOME_INSTALL_DOC_GUI 
 SALOME_INSTALL_DOC_TUI
 SALOME_INSTALL_DOC_DEV
 SALOME_INSTALL_DOC_EXTRA
)

# Sources 
# =====
ADD_SUBDIRECTORY(tui)
ADD_SUBDIRECTORY(gui)
ADD_SUBDIRECTORY(extra)
ADD_SUBDIRECTORY(dev)

# Install rules
# ==========
FILE(GLOB COMMON_HTML_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.html")
FILE(GLOB COMMON_PNG_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.png")

INSTALL(FILES ${COMMON_HTML_FILES} ${COMMON_PNG_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX})
