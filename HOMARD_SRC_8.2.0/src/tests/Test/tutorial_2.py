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
Test tutorial_2 associe au tutorial 2
"""
__revision__ = "V3.1"

#========================================================================
TEST_NAME = "tutorial_2"
DEBUG = False
N_ITER_TEST_FILE = 2
#========================================================================
import os
import tempfile
import sys
import HOMARD
import salome
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
# Repertoire des donnees du tutorial
DATA_TUTORIAL = os.path.join(PATH_HOMARD, "share", "doc", "salome", "gui", "HOMARD", "fr", "_downloads")
DATA_TUTORIAL = os.path.normpath(DATA_TUTORIAL)
sys.path.append(DATA_TUTORIAL)
from tutorial_util import gzip_gunzip
# ==================================
gzip_gunzip(DATA_TUTORIAL, 2, -1)
# ==================================

salome.salome_init()
import iparameters
IPAR = iparameters.IParameters(salome.myStudy.GetCommonParameters("Interface Applicative", 1))
IPAR.append("AP_MODULES_LIST", "Homard")
#
#========================================================================
#========================================================================
def homard_exec(theStudy):
  """
Python script for HOMARD
  """
  #
  HOMARD.SetCurrentStudy(theStudy)
  #
  # Creation des zones
  # ==================
  # Box "Zone_12_0"
  zone_12_0 = HOMARD.CreateZoneBox ('Zone_12_0', -0.1, 1.1, -0.1, 1.1, 0.9, 1.1)
  #
  # Sphere "Zone_12_1"
  zone_12_1 = HOMARD.CreateZoneSphere ('Zone_12_1', 0., 0., 0., 1.05)
  #
  # Box "Zone_12_2"
  zone_12_2 = HOMARD.CreateZoneBox ('Zone_12_2', -0.1, 0.51, -0.1, 0.51, -0.1, 0.51)
  #
  # Hypothese "hypo_2"
  # ==================
  hypo_2 = HOMARD.CreateHypothesis('hypo_2')
  hypo_2.AddZone('Zone_12_1', 1)
  hypo_2.AddZone('Zone_12_0', 1)
  #
  # Hypothese "hypo_2_bis"
  # ======================
  hypo_2_bis = HOMARD.CreateHypothesis('hypo_2_bis')
  hypo_2_bis.AddZone('Zone_12_0', -1)
  hypo_2_bis.AddZone('Zone_12_2', 1)
  #
  # Cas
  # ===
  case_2 = HOMARD.CreateCase('case_2', 'MZERO', DATA_TUTORIAL+'/tutorial_2.00.med')
  case_2.SetDirName(DIRCASE)
  #
  # Iteration "iter_2_1"
  # ====================
  iter_2_1 = case_2.NextIteration('iter_2_1')
  iter_2_1.SetMeshName('M_1')
  iter_2_1.SetMeshFile(DIRCASE+'/maill.01.med')
  iter_2_1.AssociateHypo('hypo_2')
  error = iter_2_1.Compute(1, 2)
  #
  # Iteration "iter_2_2"
  # ====================
  iter_2_2 = iter_2_1.NextIteration('iter_2_2')
  iter_2_2.SetMeshName('M_2')
  iter_2_2.SetMeshFile(DIRCASE+'/maill.02.med')
  iter_2_2.AssociateHypo('hypo_2_bis')
  error = iter_2_2.Compute(1, 2)
  #
  return error

#========================================================================

HOMARD = salome.lcc.FindOrLoadComponent('FactoryServer', 'HOMARD')
assert HOMARD is not None, "Impossible to load HOMARD engine"
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
# ==================================
gzip_gunzip(DATA_TUTORIAL, 2, 1)
# ==================================
#
if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser(True)
  iparameters.getSession().restoreVisualState(1)

