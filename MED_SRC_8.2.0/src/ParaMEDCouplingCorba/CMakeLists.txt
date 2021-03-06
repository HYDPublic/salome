# Copyright (C) 2012-2016  CEA/DEN, EDF R&D
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

INCLUDE_DIRECTORIES(
  ${OMNIORB_INCLUDE_DIR} ${OMNIORB_INCLUDE_DIRS}
  ${MPI_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}/../../idl
  ${CMAKE_CURRENT_SOURCE_DIR}/../MEDCouplingCorba
  ${MEDCOUPLING_INCLUDE_DIRS}
  )

SET(paramedcouplingcorba_SOURCES
  ParaMEDCouplingFieldDoubleServant.cxx
  ParaMEDCouplingUMeshServant.cxx
  )

ADD_LIBRARY(paramedcouplingcorba SHARED ${paramedcouplingcorba_SOURCES})
SET_TARGET_PROPERTIES(paramedcouplingcorba PROPERTIES COMPILE_FLAGS "${MPI_DEFINITIONS} ${OMNIORB_DEFINITIONS}")
TARGET_LINK_LIBRARIES(paramedcouplingcorba medcouplingcorba ${MPI_LIBS})
INSTALL(TARGETS paramedcouplingcorba EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

FILE(GLOB paramedcouplingcorba_HEADERS_HXX "${CMAKE_CURRENT_SOURCE_DIR}/*.hxx")
INSTALL(FILES ${paramedcouplingcorba_HEADERS_HXX} DESTINATION ${SALOME_INSTALL_HEADERS})
