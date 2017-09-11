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

INCLUDE(UseQtExt)

# --- options ---
# additional include directories
INCLUDE_DIRECTORIES(
  ${PTHREAD_INCLUDE_DIR}
  ${CAS_INCLUDE_DIRS}
  ${KERNEL_INCLUDE_DIRS}
  ${GUI_INCLUDE_DIRS} 
  ${ATOMGEN_INCLUDE_DIRS}
  ${QT_INCLUDES}
  ${OMNIORB_INCLUDE_DIR}
  ${VTK_INCLUDE_DIRS}
  ${PROJECT_BINARY_DIR}
  ${PROJECT_BINARY_DIR}/idl
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${QT_DEFINITIONS}
  ${CAS_DEFINITIONS}
  ${OMNIORB_DEFINITIONS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${GUI_CAM}
  ${GUI_LightApp}
  ${GUI_SalomeApp}
  ${ATOMGEN_SalomeIDLATOMGEN}
  SalomeIDLATOMSOLV
)

# --- headers ---

# header files / to be processed by moc
SET(_moc_HEADERS
  ATOMSOLVGUI.h
  ATOMSOLVGUI_TransparencyDlg.h
)

# header files / no moc processing
SET(_other_HEADERS
  ATOMSOLVGUI_DataModel.h
  ATOMSOLVGUI_DataObject.h
  ATOMSOLVGUI_Displayer.h
  ATOMSOLVGUI_Selection.h
)

# header files / to install
SET(ATOMSOLVGUI_HEADERS ${_moc_HEADERS} ${_other_HEADERS})

# --- sources ---

# sources / moc wrappings
QT_WRAP_MOC(_moc_SOURCES ${_moc_HEADERS})

# sources / static
SET(_other_SOURCES
  ATOMSOLVGUI.cxx
  ATOMSOLVGUI_TransparencyDlg.cxx
  ATOMSOLVGUI_DataModel.cxx
  ATOMSOLVGUI_DataObject.cxx
  ATOMSOLVGUI_Displayer.cxx
  ATOMSOLVGUI_Selection.cxx
)

# sources / to compile
SET(ATOMSOLVGUI_SOURCES ${_other_SOURCES} ${_moc_SOURCES})

# --- resources ---

# resource files / to be processed by lrelease
SET(_ts_RESOURCES
  resources/ATOMSOLV_icons.ts
  resources/ATOMSOLV_msg_en.ts
  resources/ATOMSOLV_msg_ja.ts
) 

# --- rules ---

ADD_LIBRARY(ATOMSOLV ${ATOMSOLVGUI_SOURCES})
TARGET_LINK_LIBRARIES(ATOMSOLV ${_link_LIBRARIES} )
INSTALL(TARGETS ATOMSOLV EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${ATOMSOLVGUI_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
QT_INSTALL_TS_RESOURCES("${_ts_RESOURCES}" "${SALOME_ATOMSOLV_INSTALL_RES_DATA}")
