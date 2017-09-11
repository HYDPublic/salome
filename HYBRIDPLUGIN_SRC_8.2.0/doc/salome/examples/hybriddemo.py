
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
BLSURF = hybridMesh.Triangle(algo=smeshBuilder.BLSURF)
HYBRID = hybridMesh.Tetrahedron(algo=smeshBuilder.HYBRID)

# compute the mesh
hybridMesh.Compute()

# End of script

