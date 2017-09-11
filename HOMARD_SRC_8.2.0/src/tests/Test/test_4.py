# -*- coding: utf-8 -*-
# Copyright (C) 2011-2016  CEA/DEN, EDF R&D
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
"""
Python script for HOMARD
Test test_4
"""
__revision__ = "V2.2"

#========================================================================
TEST_NAME = "test_4"
DEBUG = False
N_ITER_TEST_FILE = 3
DX = 600.
DY = 400.
DZ = 200.
#========================================================================
import os
import tempfile
import sys
import numpy as np
import salome
import GEOM
import SMESH
import HOMARD
import MEDCoupling as mc
import MEDLoader as ml
#
# ==================================
PATH_HOMARD = os.getenv('HOMARD_ROOT_DIR')
# Repertoire des scripts utilitaires
REP_PYTHON = os.path.join(PATH_HOMARD, "bin", "salome", "test", "HOMARD")
REP_PYTHON = os.path.normpath(REP_PYTHON)
sys.path.append(REP_PYTHON)
from test_util import remove_dir
from test_util import test_results
# Repertoire des donnees du test
REP_DATA = os.path.join(PATH_HOMARD, "share", "salome", "homardsamples")
REP_DATA = os.path.normpath(REP_DATA)
# Repertoire des resultats
if DEBUG :
  DIRCASE = os.path.join("/tmp", TEST_NAME)
  if ( os.path.isdir(DIRCASE) ) :
    remove_dir(DIRCASE)
  os.mkdir(DIRCASE)
else :
  DIRCASE = tempfile.mkdtemp()
# ==================================

salome.salome_init()

import SALOMEDS
from salome.geom import geomBuilder
from salome.smesh import smeshBuilder
from salome.StdMeshers import StdMeshersBuilder
#
from MEDCouplingRemapper import MEDCouplingRemapper

import iparameters
IPAR = iparameters.IParameters(salome.myStudy.GetCommonParameters("Interface Applicative", 1))
IPAR.append("AP_MODULES_LIST", "Homard")
#
#========================================================================
#========================================================================
def geom_smesh_exec(theStudy):
  """
Python script for GEOM and SMESH
  """
  error = 0
#
  while not error :
  #
    geompy = geomBuilder.New(theStudy)
  #
  # Creation of the box
  # ===================
    box_g = geompy.MakeBoxDXDYDZ(DX, DY, DZ, "BOX")

  # Creation of the mesh
  # ====================
    smesh = smeshBuilder.New(theStudy)
    box_m = smesh.Mesh(box_g)
    smesh.SetName(box_m.GetMesh(), 'MESH')
  #
  # Creation of the hypotheses
  # ==========================
    regular_1d = box_m.Segment()
    smesh.SetName(regular_1d.GetAlgorithm(), 'Regular_1D')
    length = min(DX, DY, DZ) / 5.
    local_length = regular_1d.LocalLength(length, None, 1e-07)
    smesh.SetName(local_length, 'Local Length')
  #
    quadrangle_2d = box_m.Quadrangle(algo=smeshBuilder.QUADRANGLE)
    smesh.SetName(quadrangle_2d.GetAlgorithm(), 'Quadrangle_2D')
    quadrangle_parameters = quadrangle_2d.QuadrangleParameters(StdMeshersBuilder.QUAD_STANDARD, -1, [], [])
    smesh.SetName(quadrangle_parameters, 'Quadrangle Parameters')
  #
    hexa_3d = box_m.Hexahedron(algo=smeshBuilder.Hexa)
    smesh.SetName(hexa_3d.GetAlgorithm(), 'Hexa_3D')
  #
  # Computation
  # ===========
  #
    isDone = box_m.Compute()
    if not isDone :
      error = 1
      break
  #
  # MED exportation
  # ===============
  #
    try:
      ficmed = os.path.join(DIRCASE, 'maill.00.med')
      box_m.ExportMED( ficmed, 0, SMESH.MED_V2_2, 1, None, 1)
    except Exception, eee:
      error = 2
      raise Exception('ExportToMEDX() failed. '+eee.message)
  #
    break
  #
  return error

#========================================================================
#
#========================================================================
def field_exec(theStudy, niter):
  """
Python script for MEDCoupling
  """
  error = 0
#
  while not error :
  #
  # The mesh
  # ========
    ficmed = os.path.join(DIRCASE, 'maill.%02d.med' % niter)
    meshMEDFileRead = ml.MEDFileMesh.New(ficmed)
    meshRead0 = meshMEDFileRead.getMeshAtLevel(0)
  # Valeurs of the field
  # ====================
    nbNodes = meshRead0.getNumberOfNodes()
    valeur = mc.DataArrayDouble(nbNodes)
    for iaux, taux in enumerate(meshRead0.getCoords()) :
      #ligne   = "x = %f" % taux[0]
      #ligne  += ", y = %f" % taux[1]
      #ligne  += ", z = %f" % taux[2]
      #print ligne
      #distance = (taux[0]-DX*0.2)**2 + (taux[1]-DY*0.2)**2 + (taux[2]-DZ*0.4)**2
      distance = min(abs(taux[0]-DX*0.4), abs(taux[1]-DY*0.2), abs(taux[2]-DZ*0.4))
      valeur[iaux] = 1.e0 / max ( 1.e-5, np.sqrt(distance) )
    #print ". valeur", valeur
    nparr = valeur.toNumPyArray()
    print ". mini/maxi", nparr.min(), nparr.max()
  #
  # Creation of the field
  # =====================
    field = ml.MEDCouplingFieldDouble(ml.ON_NODES, ml.ONE_TIME)
    field.setArray(valeur)
    field.setMesh(meshRead0)
    field.setName("DISTANCE")
  #
    fMEDFile_ch = ml.MEDFileField1TS()
    fMEDFile_ch.setFieldNoProfileSBT(field)     # No profile desired on the field, Sort By Type
    fMEDFile_ch.write(ficmed, 0) # 0 to indicate that we *append* (and no overwrite) to the MED file
  #
    break
  #
  return error

#========================================================================
#========================================================================
def homard_exec(theStudy):
  """
Python script for HOMARD
  """
  error = 0
#
  while not error :
  #
    HOMARD.SetCurrentStudy(theStudy)
  #
  # Creation of the zones
  # =====================
  #
    epsilon = min(DX, DY, DZ) / 100.
  # Creation of the box zone_4_1
    zone_4_1 = HOMARD.CreateZoneBox('Zone_4_1', -epsilon, DX/3.+epsilon, DY/4.-epsilon, 3.*DY/4.+epsilon, 4.*DZ/5.-epsilon, DZ+epsilon)

  # Creation of the sphere zone_4_2
    rayon = min(DX, DY, DZ) / 4.
    zone_4_2 = HOMARD.CreateZoneSphere('Zone_4_2', DX/3., DY*0.3, DZ*0.6, rayon)
  #
  # Creation of the hypotheses
  # ==========================
    dico = {}
    dico["1"] = "raffinement"
    dico["-1"] = "deraffinement"
  # Creation of the hypothesis hypo_4_1
    hyponame_1 = "Zone_1"
    print "-------- Creation of the hypothesis", hyponame_1
    hypo_4_1 = HOMARD.CreateHypothesis(hyponame_1)
    hypo_4_1.AddZone('Zone_4_1', 1)
    hypo_4_1.SetExtraOutput(2)
    laux = hypo_4_1.GetZones()
    nbzone = len(laux)/2
    jaux = 0
    for iaux in range(nbzone) :
      print hyponame_1, " : ", dico[laux[jaux+1]], "sur la zone", laux[jaux]
      jaux += 2
  # Creation of the hypothesis hypo_4_2
    hyponame_2 = "Zone_2"
    print "-------- Creation of the hypothesis", hyponame_2
    hypo_4_2 = HOMARD.CreateHypothesis(hyponame_2)
    hypo_4_2.AddZone('Zone_4_2', 1)
    hypo_4_2.SetExtraOutput(2)
    laux = hypo_4_2.GetZones()
    nbzone = len(laux)/2
    jaux = 0
    for iaux in range(nbzone) :
      print hyponame_2, " : ", dico[laux[jaux+1]], "sur la zone", laux[jaux]
      jaux += 2
  # Creation of the hypothesis DISTANCE INVERSE
    hyponame_3 = "DISTANCE INVERSE"
    print "-------- Creation of the hypothesis", hyponame_3
    hypo_4_3 = HOMARD.CreateHypothesis(hyponame_3)
    hypo_4_3.SetField('DISTANCE')
    hypo_4_3.SetUseComp(0)
    hypo_4_3.SetRefinThr(1, 0.3)
    hypo_4_3.SetUnRefThr(1, 0.2)
    hypo_4_3.AddFieldInterp('DISTANCE')
    hypo_4_3.SetExtraOutput(2)
    print hyponame_3, " : zones utilisées :", hypo_4_3.GetZones()
    print hyponame_3, " : champ utilisé :", hypo_4_3.GetFieldName()
    print hyponame_3, " : composantes utilisées :", hypo_4_3.GetComps()
    if ( len (hypo_4_3.GetFieldName()) > 0 ) :
      print ".. caractéristiques de l'adaptation :", hypo_4_3.GetField()
    print hyponame_3, " : champs interpolés :", hypo_4_3.GetFieldInterps()
  #
  # Creation of the cases
  # =====================
    # Creation of the case
    print "-------- Creation of the case", TEST_NAME
    mesh_file = os.path.join(DIRCASE, 'maill.00.med')
    case_test_4 = HOMARD.CreateCase(TEST_NAME, 'MESH', mesh_file)
    case_test_4.SetDirName(DIRCASE)
  #
  # Creation of the iterations
  # ==========================
  # Creation of the iteration 1
    iter_name = "I_" + TEST_NAME + "_1"
    print "-------- Creation of the iteration", iter_name
    iter_test_4_1 = case_test_4.NextIteration(iter_name)
    iter_test_4_1.AssociateHypo(hyponame_1)
    print ". Hypothese :", hyponame_1
    iter_test_4_1.SetMeshName('M1')
    iter_test_4_1.SetMeshFile(os.path.join(DIRCASE, 'maill.01.med'))
    error = iter_test_4_1.Compute(1, 2)
    if error :
      error = 1
      break

  # Creation of the iteration 2
    iter_name = "I_" + TEST_NAME + "_2"
    print "-------- Creation of the iteration", iter_name
    iter_test_4_2 = iter_test_4_1.NextIteration(iter_name)
    iter_test_4_2.AssociateHypo(hyponame_2)
    print ". Hypothese :", hyponame_2
    iter_test_4_2.SetMeshName('M2')
    iter_test_4_2.SetMeshFile(os.path.join(DIRCASE, 'maill.02.med'))
    error = iter_test_4_2.Compute(1, 2)
    if error :
      error = 2
      break

  # Creation of the iteration 3
  #
    error = field_exec(theStudy, 2)
    if error :
      error = 30
      break
  #
    iter_name = "I_" + TEST_NAME + "_3"
    print "-------- Creation of the iteration", iter_name
    iter_test_4_3 = iter_test_4_2.NextIteration(iter_name)
    iter_test_4_3.AssociateHypo(hyponame_3)
    print ". Hypothese :", hyponame_3
    iter_test_4_3.SetMeshName('M3')
    iter_test_4_3.SetFieldFile(os.path.join(DIRCASE, 'maill.02.med'))
    iter_test_4_3.SetMeshFile(os.path.join(DIRCASE, 'maill.03.med'))
    error = iter_test_4_3.Compute(1, 2)
    if error :
      error = 3
      break
  #
    break
  #
  return error

#========================================================================
#
# Geometry and Mesh
#
try :
  ERROR = geom_smesh_exec(salome.myStudy)
  if ERROR :
    raise Exception('Pb in geom_smesh_exec')
except Exception, eee:
  raise Exception('Pb in geom_smesh_exec: '+eee.message)

HOMARD = salome.lcc.FindOrLoadComponent('FactoryServer', 'HOMARD')
assert HOMARD is not None, "Impossible to load homard engine"
HOMARD.SetLanguageShort("fr")
#
# Exec of HOMARD-SALOME
#
try :
  ERROR = homard_exec(salome.myStudy)
  if ERROR :
    raise Exception('Pb in homard_exec at iteration %d' %ERROR )
except Exception, eee:
  raise Exception('Pb in homard_exec: '+eee.message)
#
# Test of the results
#
N_REP_TEST_FILE = N_ITER_TEST_FILE
DESTROY_DIR = not DEBUG
test_results(REP_DATA, TEST_NAME, DIRCASE, N_ITER_TEST_FILE, N_REP_TEST_FILE, DESTROY_DIR)
#
if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser(True)
  iparameters.getSession().restoreVisualState(1)

