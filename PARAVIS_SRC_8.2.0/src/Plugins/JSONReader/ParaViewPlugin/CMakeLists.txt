# Copyright (C) 2015-2016  CEA/DEN, EDF R&D
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
# Author : Roman NIKOLAEV

INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/JSONParser )

ADD_PARAVIEW_PLUGIN(JSONReader "1.0"
  SERVER_MANAGER_XML JSONReader.xml
  SERVER_MANAGER_SOURCES vtkJSONReader.cxx
  REQUIRED_ON_SERVER)

TARGET_LINK_LIBRARIES(JSONReader vtkjsoncpp)

IF(WIN32)
  ADD_DEFINITIONS(-DJSON_DLL)
ENDIF()

INSTALL(TARGETS JSONReader
  DESTINATION lib/paraview
)
