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
# Author : Anthony Geay (CEA/DEN)

INCLUDE_DIRECTORIES(
  ${OMNIORB_INCLUDE_DIR} ${OMNIORB_INCLUDE_DIRS}
  ${MEDCOUPLING_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}/../../../idl
  )

SET(medcouplingclient_SOURCES
  DataArrayDoubleClient.cxx
  DataArrayIntClient.cxx
  MEDCouplingCMeshClient.cxx
  MEDCouplingIMeshClient.cxx
  MEDCoupling1SGTUMeshClient.cxx
  MEDCoupling1DGTUMeshClient.cxx
  MEDCouplingCurveLinearMeshClient.cxx
  MEDCouplingExtrudedMeshClient.cxx
  MEDCouplingFieldDoubleClient.cxx
  MEDCouplingFieldOverTimeClient.cxx
  MEDCouplingFieldTemplateClient.cxx
  MEDCouplingMeshClient.cxx
  MEDCouplingMultiFieldsClient.cxx
  MEDCouplingUMeshClient.cxx
  )

ADD_LIBRARY(medcouplingclient SHARED ${medcouplingclient_SOURCES})
TARGET_LINK_LIBRARIES(medcouplingclient ${MEDCoupling_medcoupling} SalomeIDLMED ${OMNIORB_LIBRARIES} ${PLATFORM_LIBS})
INSTALL(TARGETS medcouplingclient EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

FILE(GLOB medcouplingclient_HEADERS_HXX "${CMAKE_CURRENT_SOURCE_DIR}/*.hxx")
INSTALL(FILES ${medcouplingclient_HEADERS_HXX} DESTINATION ${SALOME_INSTALL_HEADERS})
