# Copyright (C) 2013-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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
  ${PTHREAD_INCLUDE_DIR}
  ${OMNIORB_INCLUDE_DIR}
  ${KERNEL_INCLUDE_DIRS}
  ${ATOMGEN_INCLUDE_DIRS}
  ${PROJECT_BINARY_DIR}
  ${PROJECT_BINARY_DIR}/idl
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${OMNIORB_DEFINITIONS}
)


# libraries to link to
SET(_link_LIBRARIES
  ${ATOMGEN_SalomeIDLATOMGEN}
  ${KERNEL_SalomeContainer}
  SalomeIDLATOMSOLV
)

# --- headers ---

# header files / no moc processing
SET(ATOMSOLV_HEADERS
  ATOMSOLV.hxx
)

# --- sources ---

# sources / static
SET(ATOMSOLV_SOURCES
  ATOMSOLV.cxx
)

# --- rules ---

ADD_LIBRARY(ATOMSOLVEngine ${ATOMSOLV_SOURCES})
TARGET_LINK_LIBRARIES(ATOMSOLVEngine ${_link_LIBRARIES} )
INSTALL(TARGETS ATOMSOLVEngine EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${ATOMSOLV_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})