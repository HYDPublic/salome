# Copyright (C) 2016  CEA/DEN, EDF R&D
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
# Author : Anthony Geay (EDF R&D)

INCLUDE_DIRECTORIES(
  ${MEDCOUPLING_INCLUDE_DIRS}
  ${MEDFILE_INCLUDE_DIRS}
  ${HDF5_INCLUDE_DIRS})

VTK_MODULE_LIBRARY(vtkMEDWriter vtkMEDWriter.cxx)
TARGET_LINK_LIBRARIES(vtkMEDWriter vtkPVVTKExtensionsRendering vtkFiltersGeneral vtkFiltersCore ${PARAVIEW_LIBRARIES} ${MEDCoupling_medloader} ${MEDFILE_C_LIBRARIES})
INSTALL(TARGETS vtkMEDWriter RUNTIME DESTINATION ${VTK_INSTALL_RUNTIME_DIR} LIBRARY DESTINATION ${VTK_INSTALL_LIBRARY_DIR} ARCHIVE DESTINATION ${VTK_INSTALL_ARCHIVE_DIR})
