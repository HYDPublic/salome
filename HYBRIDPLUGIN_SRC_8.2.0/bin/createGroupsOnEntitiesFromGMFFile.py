# -*- coding: iso-8859-1 -*-

"""
to create Groups On Entities From GMFFile /tmp/tmp.mesh
"""

import sys
import salome

salome.salome_init()
theStudy = salome.myStudy

import salome_notebook
notebook = salome_notebook.NoteBook(theStudy)
#sys.path.insert( 0, r'/somewhere')

###
### GEOM component
###

import GEOM
from salome.geom import geomBuilder
import math
import SALOMEDS
import  SMESH, SALOMEDS

from salome.smesh import smeshBuilder

smesh = smeshBuilder.New(theStudy)

  
nameFile = None
the_mesh = None

nameFile = r'/tmp/tmp.mesh'

if nameFile != None:
  #do not work (the_mesh, error) = smesh.CreateMeshesFromMED(nameFile)
  (the_mesh, error) = smesh.CreateMeshesFromGMF(nameFile)


#print "the_mesh", the_mesh

if the_mesh != None:
  blue = SALOMEDS.Color(0,0,1)
  red = SALOMEDS.Color(1,0,0)
  green = SALOMEDS.Color(0,1,0)
  yellow = SALOMEDS.Color(1,1,0)

  for i in dir(SMESH):
    if "Entity_" in i: print i

  entities = [("Tetra", SMESH.Entity_Tetra, blue), 
              ("Pyramid", SMESH.Entity_Pyramid, red), 
              ("Prism", SMESH.Entity_Penta, yellow),
              ("Hexa", SMESH.Entity_Hexa, green)]

  for name, entity, color in entities:
    aCriteria = []
    aCriterion = smesh.GetCriterion(SMESH.VOLUME,SMESH.FT_EntityType,SMESH.FT_Undefined,entity)
    aCriteria.append(aCriterion)
    aFilter = smesh.GetFilterFromCriteria(aCriteria)
    aFilter.SetMesh(the_mesh.GetMesh())
    Group = the_mesh.GroupOnFilter( SMESH.VOLUME, name, aFilter )
    Group.SetColor( color )
  
  if salome.sg.hasDesktop():
    salome.sg.updateObjBrowser(True)
