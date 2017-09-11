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
  ${QT_INCLUDES}
  ${CAS_INCLUDE_DIRS}
  ${GUI_INCLUDE_DIRS}
  ${PROJECT_BINARY_DIR}
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${QT_DEFINITIONS}
  ${CAS_DEFINITIONS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${QT_LIBRARIES}
  ${GUI_LightApp}
)

# --- headers ---

# header files / to be processed by moc
SET(_moc_HEADERS
  ATOMICGUI.h
  ATOMICGUI_DataModel.h
  ATOMICGUI_AddAtomDlg.h
  ATOMICGUI_Operation.h
  ATOMICGUI_CreateMolOp.h
  ATOMICGUI_DeleteOp.h
  ATOMICGUI_RenameOp.h
  ATOMICGUI_ImportExportOp.h
  ATOMICGUI_AddAtomOp.h
)

# header files / no moc processing
SET(_other_HEADERS
  ATOMICGUI_Data.h
  ATOMICGUI_DataObject.h
  ATOMICGUI_Selection.h
  ATOMICGUI_ATOMICGUI.hxx
)

# header files / to install
SET(ATOMICGUI_HEADERS ${_moc_HEADERS} ${_other_HEADERS})

# --- sources ---

# sources / moc wrappings
QT_WRAP_MOC(_moc_SOURCES ${_moc_HEADERS})

# sources / static
SET(_other_SOURCES
  ATOMICGUI.cxx
  ATOMICGUI_Data.cxx
  ATOMICGUI_DataObject.cxx
  ATOMICGUI_DataModel.cxx
  ATOMICGUI_AddAtomDlg.cxx
  ATOMICGUI_Selection.cxx
  ATOMICGUI_Operation.cxx
  ATOMICGUI_AddAtomOp.cxx
  ATOMICGUI_CreateMolOp.cxx
  ATOMICGUI_RenameOp.cxx
  ATOMICGUI_DeleteOp.cxx
  ATOMICGUI_ImportExportOp.cxx  
)

# sources / to compile
SET(ATOMICGUI_SOURCES ${_other_SOURCES} ${_moc_SOURCES})

# --- resources ---

# resource files / to be processed by lrelease
SET(_ts_RESOURCES
  resources/ATOMIC_images.ts
  resources/ATOMIC_msg_en.ts
  resources/ATOMIC_msg_ja.ts
) 

# --- rules ---

ADD_LIBRARY(ATOMIC ${ATOMICGUI_SOURCES})
TARGET_LINK_LIBRARIES(ATOMIC ${_link_LIBRARIES} )
INSTALL(TARGETS ATOMIC EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${ATOMICGUI_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
QT_INSTALL_TS_RESOURCES("${_ts_RESOURCES}" "${SALOME_ATOMIC_INSTALL_RES_DATA}")
