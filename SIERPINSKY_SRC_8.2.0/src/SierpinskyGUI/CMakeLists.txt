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

INCLUDE(UseQtExt)

# --- options ---
# additional include directories
INCLUDE_DIRECTORIES(
  ${QT_INCLUDES}
  ${CAS_INCLUDE_DIRS}
  ${VTK_INCLUDES}
  ${KERNEL_INCLUDE_DIRS}
  ${SMESH_INCLUDE_DIRS}
  ${RANDOMIZER_INCLUDE_DIRS}
  ${GUI_INCLUDE_DIRS}
  ${OMNIORB_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}
  ${PROJECT_BINARY_DIR}/idl
  ${PROJECT_SOURCE_DIR}/src/Sierpinsky
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${BOOST_DEFINITIONS}
  ${OMNIORB_DEFINITIONS}
  ${CAS_DEFINITIONS}
  ${KERNEL_DEFINITIONS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${QT_LIBS}
  ${KERNEL_SalomeLifeCycleCORBA}
  ${GUI_SalomeApp}
  ${RANDOMIZER_SalomeIDLRANDOMIZER}
  SIERPINSKYEngine
)

# --- headers ---

# header files / to be processed by moc
SET(_moc_HEADERS
  SierpinskyGUI_RunDlg.h
  SierpinskyGUI.h
)

SET(_other_HEADERS
  SierpinskyGUI.hxx
)

SET (SIERPINSKY_HEADERS ${_moc_HEADERS} ${_other_HEADERS})

# --- sources ---

# sources / moc wrappings
QT_WRAP_MOC(_moc_SOURCES ${_moc_HEADERS})

# sources / static
SET(_other_SOURCES
  SierpinskyGUI.cxx
  SierpinskyGUI_RunDlg.cxx
)

# sources / to compile
SET(SIERPINSKY_SOURCES ${_other_SOURCES} ${_moc_SOURCES})

# --- resources ---

# resource files / to be processed by lrelease
SET(_ts_RESOURCES
  SIERPINSKY_icons.ts
  SIERPINSKY_msg_en.ts
  SIERPINSKY_msg_fr.ts
) 

# --- rules ---

ADD_LIBRARY(SIERPINSKY ${SIERPINSKY_SOURCES})
TARGET_LINK_LIBRARIES(SIERPINSKY ${_link_LIBRARIES} )
INSTALL(TARGETS SIERPINSKY EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${SIERPINSKY_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
QT_INSTALL_TS_RESOURCES("${_ts_RESOURCES}" "${SALOME_SIERPINSKY_INSTALL_RES_DATA}")
