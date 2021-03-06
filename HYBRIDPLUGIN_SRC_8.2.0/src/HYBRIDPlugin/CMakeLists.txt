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

# --- options ---
# additional include directories
INCLUDE_DIRECTORIES(
  ${KERNEL_INCLUDE_DIRS}
  ${CAS_INCLUDE_DIRS}
  ${GEOM_INCLUDE_DIRS}
  ${SMESH_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  ${MESHGEMS_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${OMNIORB_INCLUDE_DIR}
  ${PROJECT_BINARY_DIR}/idl
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${OMNIORB_DEFINITIONS}
  ${CAS_DEFINITIONS}
  ${BOOST_DEFINITIONS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${CAS_TKBRep}
  ${CAS_TKG2d}
  ${CAS_TKG3d}
  ${CAS_TKTopAlgo}
  ${CAS_TKGeomBase}
  ${CAS_TKGeomAlgo}
  ${CAS_TKCDF}
  ${MESHGEMS_HYBRID_LIBRARY}
  ${SMESH_SMESHimpl}
  ${SMESH_SMESHEngine}
  ${SMESH_SMESHDS}
  ${SMESH_SMDS}
  ${SMESH_StdMeshers}
  ${SMESH_MeshDriverGMF}
  ${KERNEL_SalomeGenericObj}
  ${KERNEL_SALOMELocalTrace}
  ${KERNEL_SALOMEBasics}
  ${KERNEL_SalomeNS}
  ${KERNEL_OpUtil}
  SalomeIDLHYBRIDPLUGIN
)

# --- headers ---

# header files
SET(HYBRIDEngine_HEADERS
  HYBRIDPlugin_Defs.hxx
  HYBRIDPlugin_HYBRID.hxx
  HYBRIDPlugin_HYBRID_i.hxx
  HYBRIDPlugin_Hypothesis.hxx
  HYBRIDPlugin_Hypothesis_i.hxx
  MG_HYBRID_API.hxx
)

# --- sources ---

# sources / static
SET(HYBRIDEngine_SOURCES
  HYBRIDPlugin_HYBRID.cxx
  HYBRIDPlugin_HYBRID_i.cxx
  HYBRIDPlugin_i.cxx
  HYBRIDPlugin_Hypothesis.cxx
  HYBRIDPlugin_Hypothesis_i.cxx
  MG_HYBRID_API.cxx
)

# --- scripts ---

# scripts / static
SET(_bin_SCRIPTS
  __init__.py
  HYBRIDPluginBuilder.py
)

# --- rules ---

ADD_LIBRARY(HYBRIDEngine ${HYBRIDEngine_SOURCES})
TARGET_LINK_LIBRARIES(HYBRIDEngine ${_link_LIBRARIES} )
INSTALL(TARGETS HYBRIDEngine EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${HYBRIDEngine_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})

SALOME_INSTALL_SCRIPTS("${_bin_SCRIPTS}" ${SALOME_INSTALL_PYTHON}/salome/HYBRIDPlugin)
