# Copyright (C) 2012-2016  OPEN CASCADE
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

# --- options ---
# additional include directories
INCLUDE_DIRECTORIES(
  ${OMNIORB_INCLUDE_DIR}
  ${KERNEL_INCLUDE_DIRS}
  ${SMESH_INCLUDE_DIRS}
  ${HDF5_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${PROJECT_BINARY_DIR}
  ${PROJECT_BINARY_DIR}/idl
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${BOOST_DEFINITIONS}
  ${HDF5_DEFINITIONS}
  ${OMNIORB_DEFINITIONS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${KERNEL_SalomeContainer}
  ${KERNEL_SalomeNS}
  ${KERNEL_OpUtil}
  ${KERNEL_Registry}
  ${KERNEL_SalomeNotification}
  ${KERNEL_SALOMELocalTrace}
  ${KERNEL_gd}
  ${KERNEL_SalomeIDLKernel}
  ${SMESH_MEDWrapper}
  ${CORBA_LIBS}
  SalomeIDLSIERPINSKY
)

IF(SALOME_SIERPINSKY_USE_LIBGD)
  LIST(APPEND _link_LIBRARIES gd)
ENDIF(SALOME_SIERPINSKY_USE_LIBGD)

# --- headers ---

# header files / no moc processing
SET(SIERPINSKYEngine_HEADERS
  SIERPINSKY_Gen_i.hxx
)

# --- sources ---

# sources / static
SET(SIERPINSKYEngine_SOURCES
  SIERPINSKY_Gen_i.cxx 
)

# --- rules ---

ADD_LIBRARY(SIERPINSKYEngine ${SIERPINSKYEngine_SOURCES})
TARGET_LINK_LIBRARIES(SIERPINSKYEngine ${_link_LIBRARIES} )
INSTALL(TARGETS SIERPINSKYEngine EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${SIERPINSKYEngine_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
