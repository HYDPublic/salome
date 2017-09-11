
# An enforced vertex can be added via:
# - the coordinates x,y,z
# - a GEOM vertex or compound (No geometry, TUI only)
#
# The created enforced nodes can also be stored in
# a group.
#
# This feature is available only on meshes without geometry.

# Ex1: Add one enforced vertex with coordinates (50,50,100) 
#      and physical size 2.

import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

# create a box
box = geompy.MakeBoxDXDYDZ(200., 200., 200.)
geompy.addToStudy(box, "box")
# create a mesh on the box
hybridMesh = smesh.Mesh(box,"box: MG-Hybrid and BLSurf mesh")
# create a BLSurf algorithm for faces
hybridMesh.Triangle(algo=smeshBuilder.BLSURF)
# compute the mesh
hybridMesh.Compute()

# Make a copy of the 2D mesh
hybridMesh_wo_geometry = smesh.CopyMesh( hybridMesh, 'MG-Hybrid wo geometry', 0, 0)

# create a MG-Hybrid algorithm and hypothesis and assign them to the mesh
HYBRID = smesh.CreateHypothesis('HYBRID_3D', 'HYBRIDEngine')
HYBRID_Parameters = smesh.CreateHypothesis('HYBRID_Parameters', 'HYBRIDEngine')
hybridMesh.AddHypothesis( HYBRID )
hybridMesh.AddHypothesis( HYBRID_Parameters )
# Create the enforced vertex
HYBRID_Parameters.SetEnforcedVertex( 50, 50, 100, 2) # no group
# Compute the mesh
hybridMesh.Compute()


# Ex2: Add one vertex enforced by a GEOM vertex at (50,50,100) 
#      with physical size 5 and add it to a group called "My special nodes"

# Create another HYBRID hypothesis and assign it to the mesh without geometry
HYBRID_Parameters_wo_geometry = smesh.CreateHypothesis('HYBRID_Parameters', 'HYBRIDEngine')
hybridMesh_wo_geometry.AddHypothesis( HYBRID )
hybridMesh_wo_geometry.AddHypothesis( HYBRID_Parameters_wo_geometry )

# Create the enforced vertex
p1 = geompy.MakeVertex(150, 150, 100)
geompy.addToStudy(p1, "p1")
HYBRID_Parameters_wo_geometry.SetEnforcedVertexGeomWithGroup( p1, 5 , "My special nodes")
#HYBRID_Parameters.SetEnforcedVertexGeom( p1, 5 ) # no group

# compute the mesh
hybridMesh_wo_geometry.Compute()

# Erase all enforced vertices
HYBRID_Parameters.ClearEnforcedVertices()

# End of script
