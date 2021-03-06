# Copyright (C) 2016  CEA/DEN, EDF R&D
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
# Author : Anthony Geay (EDF R&D)

PROJECT(MEDWriter)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

IF(COMMAND cmake_policy)
  CMAKE_POLICY(SET CMP0003 NEW)
  #CMAKE_POLICY(SET CMP0022 OLD)
  #CMAKE_POLICY(SET CMP0023 OLD)
ENDIF(COMMAND cmake_policy)

SET(MED_WRITER_VERSION "0.0.0")

FIND_PACKAGE(ParaView REQUIRED)
IF(NOT ParaView_FOUND)
  MESSAGE(FATAL_ERROR "Please locate ParaView." )
ENDIF(NOT ParaView_FOUND)
INCLUDE(${PARAVIEW_USE_FILE})
PV_SETUP_MODULE_ENVIRONMENT("vtkMEDWriter")

OPTION(BUILD_SHARED_LIBS "Build with shared libraries." ${VTK_BUILD_SHARED_LIBS})

SET(VTK_INSTALL_RUNTIME_DIR lib/salome)
SET(VTK_INSTALL_LIBRARY_DIR lib/salome)
SET(VTK_INSTALL_ARCHIVE_DIR lib/salome)

SET(MEDCOUPLING_ROOT_DIR $ENV{MEDCOUPLING_ROOT_DIR} CACHE PATH "MEDCOUPLING_ROOT_DIR")
LIST(APPEND CMAKE_MODULE_PATH "${MEDCOUPLING_ROOT_DIR}/cmake_files")
INCLUDE(SalomeMacros)
FIND_PACKAGE(SalomeHDF5 REQUIRED)
FIND_PACKAGE(SalomeMEDCoupling REQUIRED)
FIND_PACKAGE(SalomeMEDFile REQUIRED)

PV_PROCESS_MODULES()

ADD_SUBDIRECTORY(ParaViewPlugin)
