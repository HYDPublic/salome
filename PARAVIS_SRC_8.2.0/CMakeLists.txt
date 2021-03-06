# Copyright (C) 2010-2016  CEA/DEN, EDF R&D
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
PROJECT(SalomePARAVIS C CXX)

IF(WIN32)
  STRING( REPLACE "INCREMENTAL:YES" "INCREMENTAL:NO" replacementFlags ${CMAKE_SHARED_LINKER_FLAGS_DEBUG} )
  SET( CMAKE_SHARED_LINKER_FLAGS_DEBUG "${replacementFlags}" )
ENDIF(WIN32)

# Ensure a proper linker behavior:
CMAKE_POLICY(SET CMP0003 NEW)
IF(WIN32)
  CMAKE_POLICY(SET CMP0020 OLD) # disable automatic linking to qtmain.lib 
ENDIF(WIN32)

# Versioning
# ===========
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

# Find KERNEL
# ===========
SET(KERNEL_ROOT_DIR $ENV{KERNEL_ROOT_DIR} CACHE PATH "Path to the Salome KERNEL")
IF(EXISTS ${KERNEL_ROOT_DIR})
  LIST(APPEND CMAKE_MODULE_PATH "${KERNEL_ROOT_DIR}/salome_adm/cmake_files")
  FIND_PACKAGE(SalomeKERNEL REQUIRED)
  ADD_DEFINITIONS(${KERNEL_DEFINITIONS})
  INCLUDE_DIRECTORIES(${KERNEL_INCLUDE_DIRS})
ELSE(EXISTS ${KERNEL_ROOT_DIR})
  MESSAGE(FATAL_ERROR "We absolutely need a Salome KERNEL, please define KERNEL_ROOT_DIR")
ENDIF(EXISTS ${KERNEL_ROOT_DIR})

# Platform setup
# ==============
INCLUDE(SalomeSetupPlatform)   # From KERNEL
# Always build libraries as shared objects:
SET(BUILD_SHARED_LIBS TRUE)

# User options
# ============
OPTION(SALOME_BUILD_DOC "Generate SALOME GUI documentation" ON)
OPTION(SALOME_BUILD_TESTS "Build SALOME tests" ON)
OPTION(SALOME_PARAVIS_ALL_TEST "Add all tests for Salome PARAVIS module" OFF)
OPTION(SALOME_PARAVIS_BUILD_PLUGINS "Build PARAVIS plugins (MEDReader, etc ...)" ON)
OPTION(SALOME_PARAVIS_BUILD_CORBA_PLUGINS "Build PARAVIS CORBA plugins (ParaMEDCorba -- this requires the SALOME MED module)" ON)
OPTION(SALOME_PARAVIS_USE_GEOM_SOURCE "Import a GEOMETRY object to PARAVIS via its Corba IOR" OFF)
OPTION(SALOME_PARAVIS_USE_SMESH_SOURCE "Import a MESH object to PARAVIS via its Corba IOR" OFF)
CMAKE_DEPENDENT_OPTION(SALOME_LIGHT_ONLY "Build SALOME Light only (no CORBA)" OFF
                       "NOT SALOME_KERNEL_LIGHT_ONLY" ON)

# Prerequisites
# =============
# Find "big" prerequisites first - they reference themselves many others
#   -> this can help finding the smaller prerequisites and detect conficts.
# In our case KERNEL has already loaded many prereq which are also used in GUI:

##
## From KERNEL:
##
FIND_PACKAGE(SalomePythonInterp REQUIRED)
FIND_PACKAGE(SalomePythonLibs REQUIRED)

IF(NOT SALOME_LIGHT_ONLY)
  FIND_PACKAGE(SalomeOmniORB REQUIRED)
ENDIF()

IF(SALOME_BUILD_DOC)
  FIND_PACKAGE(SalomeDoxygen)
  FIND_PACKAGE(SalomeGraphviz)
  FIND_PACKAGE(SalomeSphinx)
  SALOME_LOG_OPTIONAL_PACKAGE(Doxygen SALOME_BUILD_DOC)
  SALOME_LOG_OPTIONAL_PACKAGE(Graphviz SALOME_BUILD_DOC)
  SALOME_LOG_OPTIONAL_PACKAGE(Sphinx SALOME_BUILD_DOC)
ENDIF()

# Find GUI
# ===================
SET(GUI_ROOT_DIR $ENV{GUI_ROOT_DIR} CACHE PATH "Path to the Salome GUI")
IF(EXISTS ${GUI_ROOT_DIR})
  LIST(APPEND CMAKE_MODULE_PATH "${GUI_ROOT_DIR}/adm_local/cmake_files")
  # We should skip detection of VTK (done automatically from GUI) as we need to detect ParaView instead;
  # otherwise ParaView targets are not found
  SET(OMIT_DETECT_PACKAGE_VTK ON)
  FIND_PACKAGE(SalomeGUI REQUIRED)
  SALOME_GUI_WITH_CORBA() # check whether GUI builded with CORBA
  SALOME_GUI_MODE(SALOME_USE_SALOMEOBJECT SALOME_USE_PVVIEWER)
  ADD_DEFINITIONS(${GUI_DEFINITIONS})
  INCLUDE_DIRECTORIES(${GUI_INCLUDE_DIRS})
ELSE()
  MESSAGE(FATAL_ERROR "We absolutely need a Salome GUI, please define GUI_ROOT_DIR")
ENDIF()

##
## From GUI
##

IF (NOT SALOME_GUI_BUILD_WITH_QT5)
  FIND_PACKAGE(SalomeQt4 REQUIRED COMPONENTS QtCore QtGui QtXml QtHelp)
ELSE()
  FIND_PACKAGE(SalomeQt5 REQUIRED)
ENDIF()

##
## Specific to ParaViS:
##

FIND_PACKAGE(SalomeParaView REQUIRED)

##
## From MEDCoupling / MEDFile / or MEDModule (if CORBA plugin required):
##

IF(SALOME_PARAVIS_BUILD_PLUGINS)
   # If CORBA plugins required, take in MED module 
   IF(SALOME_PARAVIS_BUILD_CORBA_PLUGINS)
      SET(MED_ROOT_DIR $ENV{MED_ROOT_DIR} CACHE PATH "Path to the SALOME MED module")
      IF(EXISTS ${MED_ROOT_DIR})
         LIST(APPEND CMAKE_MODULE_PATH "${MED_ROOT_DIR}/adm_local/cmake_files")
         FIND_PACKAGE(SalomeMED)
         SALOME_LOG_OPTIONAL_PACKAGE(SalomeMED SALOME_PARAVIS_BUILD_CORBA_PLUGINS)
      ENDIF()
    ELSE(SALOME_PARAVIS_BUILD_CORBA_PLUGINS)
       SET(MEDCOUPLING_ROOT_DIR $ENV{MEDCOUPLING_ROOT_DIR} CACHE PATH "Path to the MEDCoupling tool")
       IF(EXISTS ${MEDCOUPLING_ROOT_DIR})
          LIST(APPEND CMAKE_MODULE_PATH "${MEDCOUPLING_ROOT_DIR}/cmake_files")   
       ENDIF()
       FIND_PACKAGE(SalomeMEDCoupling)            # will load MEDFile
       SALOME_LOG_OPTIONAL_PACKAGE(MEDCoupling SALOME_PARAVIS_BUILD_PLUGINS)
    ENDIF()        
ENDIF(SALOME_PARAVIS_BUILD_PLUGINS)

##
## From GEOM
##

IF(SALOME_PARAVIS_BUILD_PLUGINS AND SALOME_PARAVIS_USE_GEOM_SOURCE)
  SET(GEOM_ROOT_DIR $ENV{GEOM_ROOT_DIR} CACHE PATH "Path to the Salome GEOM")
  IF(EXISTS ${GEOM_ROOT_DIR})
    LIST(APPEND CMAKE_MODULE_PATH "${GEOM_ROOT_DIR}/adm_local/cmake_files")
    FIND_PACKAGE(SalomeGEOM REQUIRED COMPONENTS GEOMClient SalomeIDLGEOM OCC2VTK)
  ELSE()
    MESSAGE(WARNING "Salome GEOM is absolutely needed to import GEOM objects to PARAVIS.
                    As we do not find it, we set the variable SALOME_PARAVIS_USE_GEOM_SOURCE=OFF.
                    If you really need Salome GEOM, please define GEOM_ROOT_DIR and re-run configuration script.")
    SET(SALOME_PARAVIS_USE_GEOM_SOURCE OFF)
  ENDIF()
ELSEIF(NOT SALOME_PARAVIS_BUILD_PLUGINS AND SALOME_PARAVIS_USE_GEOM_SOURCE)
  MESSAGE(WARNING "PARAVIS plugins must be built to use the import of GEOM objects to PARAVIS.
                  As plugins are not built, we set the variable SALOME_PARAVIS_USE_GEOM_SOURCE=OFF.
                  If you really want to use Salome GEOM, please set SALOME_PARAVIS_BUILD_PLUGINS=ON and re-run configuration script.")
  SET(SALOME_PARAVIS_USE_GEOM_SOURCE OFF)
ENDIF()

##
## From SMESH
##

IF(SALOME_PARAVIS_BUILD_PLUGINS AND SALOME_PARAVIS_USE_SMESH_SOURCE)
  IF(NOT SALOME_PARAVIS_USE_GEOM_SOURCE)
    SET(GEOM_ROOT_DIR $ENV{GEOM_ROOT_DIR} CACHE PATH "Path to the Salome GEOM")
    IF(EXISTS ${GEOM_ROOT_DIR})
      LIST(APPEND CMAKE_MODULE_PATH "${GEOM_ROOT_DIR}/adm_local/cmake_files")
      FIND_PACKAGE(SalomeGEOM REQUIRED COMPONENTS SalomeIDLGEOM )
    ELSE()
      MESSAGE(WARNING "Salome GEOM is absolutely needed to import MESH objects to PARAVIS.
                      As we do not find it, we set the variable SALOME_PARAVIS_USE_SMESH_SOURCE=OFF.
                      If you really want to use Salome MESH, please define GEOM_ROOT_DIR and re-run configuration script.")
      SET(SALOME_PARAVIS_USE_GEOM_SOURCE OFF)
      SET(SALOME_PARAVIS_USE_SMESH_SOURCE OFF)
    ENDIF()
  ENDIF()
  SET(SMESH_ROOT_DIR $ENV{SMESH_ROOT_DIR} CACHE PATH "Path to the Salome SMESH")
  IF(SALOME_PARAVIS_USE_GEOM_SOURCE AND EXISTS ${SMESH_ROOT_DIR})
    LIST(APPEND CMAKE_MODULE_PATH "${SMESH_ROOT_DIR}/adm_local/cmake_files")
    FIND_PACKAGE(SalomeSMESH REQUIRED)
  ELSE()
    MESSAGE(WARNING "Salome MESH is absolutely needed to import MESH objects to PARAVIS.
                    As we do not find it, we set the variable SALOME_PARAVIS_USE_SMESH_SOURCE=OFF.
                    If you really need Salome MESH, please define SMESH_ROOT_DIR and re-run configuration script.")
    SET(SALOME_PARAVIS_USE_SMESH_SOURCE OFF)
  ENDIF()
ELSEIF(NOT SALOME_PARAVIS_BUILD_PLUGINS AND SALOME_PARAVIS_USE_SMESH_SOURCE)
  MESSAGE(WARNING "PARAVIS plugins must be built to use the import of MESH objects to PARAVIS.
                  As plugins are not built, we set the variable SALOME_PARAVIS_USE_SMESH_SOURCE=OFF.
                  If you really want to use Salome MESH, please set SALOME_PARAVIS_BUILD_PLUGINS=ON and re-run configuration script.")
  SET(SALOME_PARAVIS_USE_SMESH_SOURCE OFF)
ENDIF()

IF(SALOME_BUILD_TESTS)
  ENABLE_TESTING()
  FIND_PACKAGE(SalomeCppUnit)
  SALOME_LOG_OPTIONAL_PACKAGE(CppUnit SALOME_BUILD_TESTS)
ENDIF()

## Detection summary:
SALOME_PACKAGE_REPORT_AND_CHECK()

# Directories
# (default values taken from KERNEL)
# ===========
SET(SALOME_INSTALL_BINS "${SALOME_INSTALL_BINS}" CACHE PATH "Install path: SALOME binaries")
SET(SALOME_INSTALL_LIBS "${SALOME_INSTALL_LIBS}" CACHE PATH "Install path: SALOME libs")
SET(SALOME_INSTALL_IDLS "${SALOME_INSTALL_IDLS}" CACHE PATH "Install path: SALOME IDL files")
SET(SALOME_INSTALL_HEADERS "${SALOME_INSTALL_HEADERS}" CACHE PATH "Install path: SALOME headers")
SET(SALOME_INSTALL_SCRIPT_SCRIPTS "${SALOME_INSTALL_SCRIPT_SCRIPTS}" CACHE PATH
   "Install path: SALOME scripts")
SET(SALOME_INSTALL_SCRIPT_DATA "${SALOME_INSTALL_SCRIPT_DATA}" CACHE PATH
   "Install path: SALOME script data")
SET(SALOME_INSTALL_SCRIPT_PYTHON "${SALOME_INSTALL_SCRIPT_PYTHON}" CACHE PATH
   "Install path: SALOME Python scripts")
SET(SALOME_INSTALL_PYTHON "${SALOME_INSTALL_PYTHON}" CACHE PATH "Install path: SALOME Python stuff")
SET(SALOME_INSTALL_PYTHON_SHARED "${SALOME_INSTALL_PYTHON_SHARED}" CACHE PATH
   "Install path: SALOME Python shared modules")
SET(SALOME_INSTALL_CMAKE "${SALOME_INSTALL_CMAKE}" CACHE PATH "Install path: SALOME CMake files")
SET(SALOME_INSTALL_CMAKE_LOCAL "${SALOME_INSTALL_CMAKE_LOCAL}" CACHE PATH
    "Install path: local SALOME CMake files")
SET(SALOME_INSTALL_AMCONFIG_LOCAL "${SALOME_INSTALL_AMCONFIG_LOCAL}" CACHE PATH
  "Install path: local SALOME config files (obsolete, to be removed)")

SET(SALOME_INSTALL_RES "${SALOME_INSTALL_RES}" CACHE PATH "Install path: SALOME resources")
SET(SALOME_INSTALL_DOC "${SALOME_INSTALL_DOC}" CACHE PATH "Install path: SALOME documentation")

# Specific to PARAVIS:
SET(SALOME_PARAVIS_INSTALL_RES_DATA "${SALOME_INSTALL_RES}/paravis" CACHE PATH "Install path: SALOME PARAVIS specific data")

MARK_AS_ADVANCED(SALOME_INSTALL_BINS SALOME_INSTALL_LIBS SALOME_INSTALL_IDLS SALOME_INSTALL_HEADERS)
MARK_AS_ADVANCED(SALOME_INSTALL_SCRIPT_SCRIPTS SALOME_INSTALL_SCRIPT_DATA SALOME_INSTALL_SCRIPT_PYTHON)
MARK_AS_ADVANCED(SALOME_INSTALL_APPLISKEL_SCRIPTS  SALOME_INSTALL_APPLISKEL_PYTHON SALOME_INSTALL_CMAKE SALOME_INSTALL_CMAKE_LOCAL SALOME_INSTALL_RES)
MARK_AS_ADVANCED(SALOME_INSTALL_PYTHON SALOME_INSTALL_PYTHON_SHARED)
MARK_AS_ADVANCED(SALOME_INSTALL_AMCONFIG_LOCAL SALOME_INSTALL_DOC SALOME_PARAVIS_INSTALL_RES_DATA)

# Accumulate environment variables for PARAVIS module
SALOME_ACCUMULATE_ENVIRONMENT(PYTHONPATH NOCHECK ${CMAKE_INSTALL_PREFIX}/${SALOME_INSTALL_BINS}
                                                 ${CMAKE_INSTALL_PREFIX}/${SALOME_INSTALL_PYTHON})
SALOME_ACCUMULATE_ENVIRONMENT(LD_LIBRARY_PATH NOCHECK ${CMAKE_INSTALL_PREFIX}/${SALOME_INSTALL_LIBS})
SALOME_ACCUMULATE_ENVIRONMENT(PV_PLUGIN_PATH NOCHECK ${CMAKE_INSTALL_PREFIX}/lib/paraview)

# Sources
# ========
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(resources)

IF(SALOME_BUILD_DOC)
  ADD_SUBDIRECTORY(doc)
ENDIF()
IF(SALOME_BUILD_TESTS AND NOT SALOME_LIGHT_ONLY)
  ADD_SUBDIRECTORY(test)
ENDIF()
ADD_SUBDIRECTORY(bin)
ADD_SUBDIRECTORY(examples)

# Header configuration
# ====================
SALOME_XVERSION(${PROJECT_NAME})
SALOME_CONFIGURE_FILE(PARAVIS_version.h.in PARAVIS_version.h INSTALL ${SALOME_INSTALL_HEADERS})

# Configuration export
# (here only the level 1 prerequisites are exposed)
# ====================
INCLUDE(CMakePackageConfigHelpers)

# List of targets in this project we want to make visible to the rest of the world.
# They all have to be INSTALL'd with the option "EXPORT ${PROJECT_NAME}TargetGroup"
IF(NOT SALOME_LIGHT_ONLY)
  SET(_${PROJECT_NAME}_exposed_targets PARAVIS)

  # Add all targets to the build-tree export set
  EXPORT(TARGETS ${_${PROJECT_NAME}_exposed_targets}
    FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake)
ENDIF()

# Create the configuration files:
#   - in the build tree:

# Ensure the variables are always defined for the configure:
SET(KERNEL_ROOT_DIR "${KERNEL_ROOT_DIR}")
SET(GUI_ROOT_DIR "${GUI_ROOT_DIR}")
SET(MED_ROOT_DIR "${MED_ROOT_DIR}")
SET(MEDFILE_ROOT_DIR "${MEDFILE_ROOT_DIR}")
SET(MEDCOUPLING_ROOT_DIR "${MEDCOUPLING_ROOT_DIR}")
SET(GEOM_ROOT_DIR "${GEOM_ROOT_DIR}")
SET(SMESH_ROOT_DIR "${SMESH_ROOT_DIR}")

SET(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include" "${PROJECT_BINARY_DIR}/include")

# Build variables that will be expanded when configuring Salome<MODULE>Config.cmake:
SALOME_CONFIGURE_PREPARE(MEDFile)

CONFIGURE_PACKAGE_CONFIG_FILE(${PROJECT_NAME}Config.cmake.in
    ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION "${SALOME_INSTALL_CMAKE_LOCAL}"
    PATH_VARS CONF_INCLUDE_DIRS SALOME_INSTALL_CMAKE_LOCAL CMAKE_INSTALL_PREFIX
    KERNEL_ROOT_DIR GUI_ROOT_DIR MED_ROOT_DIR MEDFILE_ROOT_DIR MEDCOUPLING_ROOT_DIR GEOM_ROOT_DIR SMESH_ROOT_DIR)

#   - in the install tree (VSR 16/08/2013: TEMPORARILY COMMENT THIS - TO REMOVE?):
#       Get the relative path of the include directory so
#       we can register it in the generated configuration files:
#SET(CONF_INCLUDE_DIRS "${CMAKE_INSTALL_PREFIX}/${INSTALL_INCLUDE_DIR}")
#CONFIGURE_PACKAGE_CONFIG_FILE(adm_local/cmake_files/${PROJECT_NAME}Config.cmake.in
#    ${PROJECT_BINARY_DIR}/to_install/${PROJECT_NAME}Config.cmake
#    INSTALL_DESTINATION "${SALOME_INSTALL_CMAKE_LOCAL}"
#    PATH_VARS CONF_INCLUDE_DIRS SALOME_INSTALL_CMAKE_LOCAL CMAKE_INSTALL_PREFIX
#    KERNEL_ROOT_DIR GUI_ROOT_DIR MED_ROOT_DIR MEDFILE_ROOT_DIR)

WRITE_BASIC_PACKAGE_VERSION_FILE(${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    VERSION ${${PROJECT_NAME_UC}_VERSION}
    COMPATIBILITY AnyNewerVersion)

# Install the CMake configuration files:
INSTALL(FILES
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  DESTINATION "${SALOME_INSTALL_CMAKE_LOCAL}")

IF(NOT SALOME_LIGHT_ONLY)
  # Install the export set for use with the install-tree
  INSTALL(EXPORT ${PROJECT_NAME}TargetGroup DESTINATION "${SALOME_INSTALL_CMAKE_LOCAL}"
          FILE ${PROJECT_NAME}Targets.cmake)
ENDIF()
