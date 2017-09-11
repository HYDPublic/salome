# -*- coding: utf-8 -*-

# It is possible to constrain HYBRID with another mesh or group.
# The constraint can refer to the nodes, edges or faces.
# This feature is available only on 2D meshes without geometry.
# The constraining elements are called enforced elements for the mesh.
# They can be recovered using groups if necessary.

# In the following examples, a box and a cylinder are meshed in 2D.
# The mesh of the cylinder will be used as a constraint for the 
# 3D mesh of the box.

import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

box = geompy.MakeBoxDXDYDZ(200, 200, 200)
geompy.addToStudy( box, "box" )
cylindre = geompy.MakeCylinderRH(50, 50)
geompy.TranslateDXDYDZ(cylindre, 100, 100, 30)
face_cyl = geompy.ExtractShapes(cylindre, geompy.ShapeType["FACE"], True)[1]
geompy.addToStudy( cylindre, 'cylindre' )
geompy.addToStudyInFather( cylindre, face_cyl, 'face_cyl' )
p1 = geompy.MakeVertex(20, 20, 20)
p2 = geompy.MakeVertex(180, 180, 20)
c = geompy.MakeCompound([p1,p2])
geompy.addToStudy( p1, "p1" )
geompy.addToStudy( p2, "p2" )
geompy.addToStudy( c, "c" )

# Create the 2D algorithm and hypothesis
BLSURF = smesh.CreateHypothesis('BLSURF', 'BLSURFEngine')
# For the box
BLSURF_Parameters = smesh.CreateHypothesis('BLSURF_Parameters', 'BLSURFEngine')
BLSURF_Parameters.SetPhysicalMesh( 1 )
BLSURF_Parameters.SetPhySize( 200 )
# For the cylinder
BLSURF_Parameters2 = smesh.CreateHypothesis('BLSURF_Parameters', 'BLSURFEngine')
BLSURF_Parameters2.SetGeometricMesh( 1 )

# Create the 3D algorithm and hypothesis
HYBRID = smesh.CreateHypothesis('HYBRID_3D', 'HYBRIDEngine')
HYBRID_Parameters_in = smesh.CreateHypothesis('HYBRID_Parameters', 'HYBRIDEngine')
HYBRID_Parameters_in.SetBoundaryLayersInward( True )
HYBRID_Parameters_out = smesh.CreateHypothesis('HYBRID_Parameters', 'HYBRIDEngine')
HYBRID_Parameters_out.SetBoundaryLayersInward( False ) # to mesh outside the cylinder

# Create the mesh on the cylinder
Mesh_cylindre = smesh.Mesh(cylindre)
smesh.SetName(Mesh_cylindre,"Mesh_cylindre")
Mesh_cylindre.AddHypothesis( BLSURF )
Mesh_cylindre.AddHypothesis( BLSURF_Parameters2 )
# Create some groups
face_cyl_faces = Mesh_cylindre.GroupOnGeom(face_cyl,'group_face_cyl', SMESH.FACE)
face_cyl_edges = Mesh_cylindre.GroupOnGeom(face_cyl,'group_edge_cyl', SMESH.EDGE)
face_cyl_nodes = Mesh_cylindre.GroupOnGeom(face_cyl,'group_node_cyl', SMESH.NODE)
Mesh_cylindre.Compute()

# Create the mesh on the cylinder
Mesh_box_tri = smesh.Mesh(box)
smesh.SetName(Mesh_box_tri,"Mesh_box_tri")
Mesh_box_tri.AddHypothesis( BLSURF )
Mesh_box_tri.AddHypothesis( BLSURF_Parameters )
Mesh_box_tri.Compute()

# Create 2 copies of the 2D mesh to test the 2 types of BoundaryLayers (inward outward)
# from the whole mesh and from groups of elements.
# Then the 3D algo and hypothesis are assigned to them.

mesh_in = smesh.CopyMesh( Mesh_box_tri, 'inward', 0, 0)
mesh_in.AddHypothesis( HYBRID )
mesh_in.AddHypothesis( HYBRID_Parameters_in)

mesh_out = smesh.CopyMesh( Mesh_box_tri, 'outward', 0, 0)
mesh_out.AddHypothesis( HYBRID )
mesh_out.AddHypothesis( HYBRID_Parameters_out)


# Add the enforced elements
#HYBRID_Parameters_in.SetEnforcedMeshWithGroup(Mesh_cylindre.GetMesh(),SMESH.FACE,"faces from cylinder")
#HYBRID_Parameters_out.SetEnforcedMeshWithGroup(face_cyl_nodes,SMESH.NODE,"nodes from face_cyl_nodes")

#Compute the meshes
mesh_in.Compute()
mesh_out.Compute()

# End of script
