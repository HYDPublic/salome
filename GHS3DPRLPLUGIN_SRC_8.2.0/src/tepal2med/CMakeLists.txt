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
  ${MEDFILE_INCLUDE_DIRS}
  ${HDF5_INCLUDE_DIRS}
  ${LIBXML2_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${QT_DEFINITIONS}
  ${HDF5_DEFINITIONS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${QT_LIBRARIES}
  ${MEDFILE_C_LIBRARIES}
  ${LIBXML2_LIBRARIES}
)

# --- headers ---

# header files / to be processed by moc
SET(_moc_HEADERS
  dlg_ghs3dmain.h
)

SET(_other_HEADERS
  ghs3dprl_mesh_wrap.h
  ghs3dprl_msg_parser.h
)

SET(tepal2med_HEADERS ${_moc_HEADERS} ${_other_HEADERS})

# --- sources ---

# sources / moc wrappings
QT_WRAP_MOC(_moc_SOURCES ${_moc_HEADERS})

# sources / static
SET(_other_SOURCES
  ghs3dprl_mesh_wrap.cxx
  ghs3dprl_msg_parser.cxx
  dlg_ghs3dmain.cxx
)

# resource files / to be processed by uic
SET(_uic_files
  dlg_ghs3dmain.ui
)

# header files / uic wrappings
QT_WRAP_UIC(_uic_HEADERS ${_uic_files})

# sources / to compile
SET(tepal2med_SOURCES tepal2med.cxx ${_other_SOURCES} ${_moc_SOURCES} ${_uic_HEADERS})
SET(tetrahpc2med_SOURCES tetrahpc2med.cxx ${_other_SOURCES} ${_moc_SOURCES} ${_uic_HEADERS})

# --- rules ---

ADD_EXECUTABLE(tepal2med ${tepal2med_SOURCES})
ADD_EXECUTABLE(tetrahpc2med ${tetrahpc2med_SOURCES})
TARGET_LINK_LIBRARIES(tepal2med ${_link_LIBRARIES})
TARGET_LINK_LIBRARIES(tetrahpc2med ${_link_LIBRARIES})
INSTALL(TARGETS tepal2med tetrahpc2med EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_BINS})
