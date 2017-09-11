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
INCLUDE(UsePyQt)

# --- options ---
# additional include directories
INCLUDE_DIRECTORIES(
  $(QT_INCLUDES)
  ${GUI_INCLUDE_DIRS}
  ${SIP_INCLUDE_DIR}
  ${PYTHON_INCLUDE_DIRS}
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}/src/ATOMICGUI
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${QT_DEFINITIONS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${QT_LIBRARIES}
  ${GUI_LightApp}
  ATOMIC
)

# --- headers ---

# header files / no moc processing
SET(AtomicPy_HEADERS
  AtomicPy.h
)

# --- sources ---

# sources / static
SET(_other_SOURCES
  AtomicPy.cxx
)

# sip files / to be processed by sip
SET(_sip_files AtomicPy.sip)

# sources / sip wrappings
PYQT_WRAP_SIP(_sip_SOURCES ${_sip_files})

# sources / to compile
SET(AtomicPy_SOURCES ${_other_SOURCES} ${_sip_SOURCES})

# --- rules ---

ADD_LIBRARY(AtomicPy ${AtomicPy_SOURCES})
TARGET_LINK_LIBRARIES(AtomicPy ${_link_LIBRARIES})
INSTALL(TARGETS AtomicPy EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${AtomicPy_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
