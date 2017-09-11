# Copyright (C) 2007-2016  CEA/DEN, EDF R&D
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

##
# @package GHS3DPRLPluginBuilder
# Python API for the MG-Tetra_HPC meshing plug-in module.

from salome.smesh.smesh_algorithm import Mesh_Algorithm
from salome.smesh.smeshBuilder import AssureGeomPublished

# import GHS3DPRLPlugin module if possible
noGHS3DPRLPlugin = 0
try:
    import GHS3DPRLPlugin
except ImportError:
    noGHS3DPRLPlugin = 1
    pass

# Optimization level of MG-Tetra_HPC
# V3.1
None_Optimization, Light_Optimization, Medium_Optimization, Strong_Optimization = 0,1,2,3
# V4.1 (partialy redefines V3.1). Issue 0020574
None_Optimization, Light_Optimization, Standard_Optimization, StandardPlus_Optimization, Strong_Optimization = 0,1,2,3,4

#----------------------------
# Mesh algo type identifiers
#----------------------------

## Algorithm type: MG-Tetra_HPC tetrahedron 3D algorithm, see GHS3DPRL_Algorithm
MG_Tetra_HPC = "MG-Tetra Parallel"
MG_Tetra_Parallel = MG_Tetra_HPC
GHS3DPRL          = MG_Tetra_HPC

#----------------------------
# Algorithms
#----------------------------

## Tetrahedron MG-Tetra_HPC 3D algorithm
#
#  It can be created by calling smeshBuilder.Mesh.Tetrahedron( smeshBuilder.MG_Tetra_HPC )
class GHS3DPRL_Algorithm(Mesh_Algorithm):

    ## name of the dynamic method in smeshBuilder.Mesh class
    #  @internal
    meshMethod = "Tetrahedron"
    ## type of algorithm used with helper function in smeshBuilder.Mesh class
    #  @internal
    algoType   = MG_Tetra_HPC
    ## doc string of the method in smeshBuilder.Mesh class
    #  @internal
    docHelper  = "Creates tetrahedron 3D algorithm for volumes"

    ## Private constructor.
    #  @param mesh parent mesh object algorithm is assigned to
    #  @param geom geometry (shape/sub-shape) algorithm is assigned to;
    #              if it is @c 0 (default), the algorithm is assigned to the main shape
    def __init__(self, mesh, geom=0):
        Mesh_Algorithm.__init__(self)
        if noGHS3DPRLPlugin: print "Warning: GHS3DPRLPlugin module unavailable"
        self.Create(mesh, geom, self.algoType, "libGHS3DPRLEngine.so")
        self.params = None
        pass

    ## Defines hypothesis having several parameters
    #  @return hypothesis object
    def Parameters(self):
        if not self.params:
            self.params = self.Hypothesis("MG-Tetra Parallel Parameters", [],
                                          "libGHS3DPRLEngine.so", UseExisting=0)
            pass
        return self.params

    ## To keep working files or remove them. Log file remains in case of errors anyway.
    #  @param toKeep "keep working files" flag value
    def SetKeepFiles(self, toKeep):
        self.Parameters().SetKeepFiles(toKeep)
        pass
    
    ## Sets MED files name and path.
    #  @param value MED file name
    def SetMEDName(self, value):
        self.Parameters().SetMEDName(value)
        pass

    ## Sets the number of partition of the initial mesh
    #  @param value number of partition value
    def SetNbPart(self, value):
        self.Parameters().SetNbPart(value)
        pass

    ## When big mesh, start tetra_hpc in background
    #  @param value "background mode" flag value
    def SetBackground(self, value):
        self.Parameters().SetBackground(value)
        pass

    ## use multithread version
    #  @param value "multithread mode" flag value
    def SetMultithread(self, value):
        self.Parameters().SetMultithread(value)
        pass

    ## To mesh "holes" in a solid or not. Default is to mesh.
    #  @param toMesh "mesh holes" flag value
    #def SetToMeshHoles(self, toMesh):
    #    self.Parameters().SetToMeshHoles(toMesh)
    #    pass
    
    ## Sets the neigbour cell gradation of the initial mesh
    #  @param value number of partition value
    def SetGradation(self, value):
        self.Parameters().SetGradation(value)
        pass

    ## Sets the cell min size of the initial mesh
    #  @param value number of partition value
    def SetMinSize(self, value):
        self.Parameters().SetMinSize(value)
        pass

    ## Sets the cell max size of the initial mesh
    #  @param value number of partition value
    def SetMaxSize(self, value):
        self.Parameters().SetMaxSize(value)
        pass
    
    ## Sets command line option as text.
    #  @param option command line option
    def SetAdvancedOption(self, option):
        self.Parameters().SetAdvancedOption(option)
        pass

    pass # end of GHS3DPRL_Algorithm class
