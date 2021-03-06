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

INCLUDE(UseQtExt)

# --- options ---
# additional include directories
INCLUDE_DIRECTORIES(
  ${QT_INCLUDES}
  ${CAS_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIRS}
  ${KERNEL_INCLUDE_DIRS}
  ${GUI_INCLUDE_DIRS}
  ${GEOM_INCLUDE_DIRS}
  ${SMESH_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${OMNIORB_INCLUDE_DIR}
  ${PROJECT_BINARY_DIR}/idl
  ${PROJECT_SOURCE_DIR}/src/GHS3DPRLPlugin
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${QT_DEFINITIONS}
  ${OMNIORB_DEFINITIONS}
  ${CAS_DEFINITIONS}
  ${BOOST_DEFINITIONS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${QT_LIBRARIES}
  ${KERNEL_SALOMELocalTrace}
  ${GEOM_GEOM}
  ${SMESH_SMESH}
  ${SMESH_PluginUtils}
  ${SMESH_StdMeshersGUI}
  ${SMESH_SMESHFiltersSelection}
  ${GUI_suit}
  ${GUI_qtx}
  ${GUI_SalomeApp}
  ${CAS_KERNEL}
  SalomeIDLGHS3DPRLPLUGIN
)

# --- headers ---

# header files / to be processed by moc
SET(GHS3DPRLPluginGUI_HEADERS
  GHS3DPRLPluginGUI_HypothesisCreator.h
)

# --- sources ---

# sources / moc wrappings
QT_WRAP_MOC(_moc_SOURCES ${GHS3DPRLPluginGUI_HEADERS})

# sources / static
SET(_other_SOURCES
  GHS3DPRLPluginGUI.cxx
  GHS3DPRLPluginGUI_HypothesisCreator.cxx
)

# --- resources ---

# resource files / to be processed by lrelease
SET(_ts_RESOURCES
  GHS3DPRLPlugin_images.ts
  GHS3DPRLPlugin_msg_en.ts
  GHS3DPRLPlugin_msg_fr.ts
  GHS3DPRLPlugin_msg_ja.ts
) 

# sources / to compile
SET(GHS3DPRLPluginGUI_SOURCES ${_other_SOURCES} ${_moc_SOURCES})

# --- rules ---

ADD_LIBRARY(GHS3DPRLPluginGUI ${GHS3DPRLPluginGUI_SOURCES})
TARGET_LINK_LIBRARIES(GHS3DPRLPluginGUI ${_link_LIBRARIES} )
INSTALL(TARGETS GHS3DPRLPluginGUI EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${GHS3DPRLPluginGUI_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})

QT_INSTALL_TS_RESOURCES("${_ts_RESOURCES}" "${SALOME_GHS3DPRLPLUGIN_INSTALL_RES_DATA}")
