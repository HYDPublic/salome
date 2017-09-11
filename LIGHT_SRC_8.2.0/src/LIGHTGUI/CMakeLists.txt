# Copyright (C) 2013-2016  OPEN CASCADE
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
  ${VTK_INCLUDE_DIRS}
  ${CAS_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIRS}
  ${KERNEL_INCLUDE_DIRS}
  ${GUI_INCLUDE_DIRS}
  ${PROJECT_BINARY_DIR}
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${QT_DEFINITIONS}
  ${CAS_DEFINITIONS}
  ${PYTHON_DEFINITIONS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${GUI_LightApp}
)

# --- headers ---

# header files / to be processed by moc
SET(_moc_HEADERS
  LIGHTGUI.h
  LIGHTGUI_DataModel.h
)

# header files / no moc processing
SET(_other_HEADERS
  LIGHTGUI_DataObject.h
  LIGHTGUI_Exports.h
  LIGHTGUI_Selection.h
  LIGHTGUI_TextPrs.h
)

# header files / to install
SET(LIGHT_HEADERS ${_moc_HEADERS} ${_other_HEADERS})

# --- sources ---

# sources / moc wrappings
QT_WRAP_MOC(_moc_SOURCES ${_moc_HEADERS})

# sources / static
SET(_other_SOURCES
  LIGHTGUI.cxx
  LIGHTGUI_DataModel.cxx
  LIGHTGUI_DataObject.cxx
  LIGHTGUI_Selection.cxx
  LIGHTGUI_TextPrs.cxx
)

# sources / to compile
SET(LIGHT_SOURCES ${_other_SOURCES} ${_moc_SOURCES})

# --- resources ---

# resource files / to be processed by lrelease
SET(_ts_RESOURCES
  resources/LIGHT_images.ts
  resources/LIGHT_msg_en.ts
  resources/LIGHT_msg_fr.ts
  resources/LIGHT_msg_ja.ts
) 

# --- rules ---

ADD_LIBRARY(LIGHT ${LIGHT_SOURCES})
TARGET_LINK_LIBRARIES(LIGHT ${_link_LIBRARIES} )
INSTALL(TARGETS LIGHT EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${LIGHT_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
QT_INSTALL_TS_RESOURCES("${_ts_RESOURCES}" "${SALOME_LIGHT_INSTALL_RES_DATA}")
