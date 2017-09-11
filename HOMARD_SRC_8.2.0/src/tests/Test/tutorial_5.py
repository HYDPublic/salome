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
Test tutorial_5 associe au tutorial 5
"""
__revision__ = "V3.1"

#========================================================================
TEST_NAME = "tutorial_5"
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
gzip_gunzip(DATA_TUTORIAL, 5, -1)
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
  # Frontiere
  # =========
  # Creation of the discrete boundary Boun_5_1
  boun_5_1 = HOMARD.CreateBoundaryDi('Boun_5_1', 'MAIL_EXT', DATA_TUTORIAL+'/tutorial_5.fr.med')
  #
  # Creation des zones
  # ==================
  # Creation of the disk with hole enveloppe
  enveloppe = HOMARD.CreateZoneDiskWithHole( 'enveloppe', 0., 0., 250., 193., 1 )
  # Creation of the rectangle quart_sup
  quart_sup = HOMARD.CreateZoneBox2D( 'quart_sup', 0., 250., 0., 250., 1 )
  #
  # Hypotheses
  # ==========
  # Creation of the hypothesis hypo_5
  hypo_5 = HOMARD.CreateHypothesis('hypo_5')
  hypo_5.AddZone('enveloppe', 1)
  # Creation of the hypothesis hypo_5_bis
  hypo_5_bis = HOMARD.CreateHypothesis('hypo_5_bis')
  hypo_5_bis.AddZone('quart_sup', 1)
  #
  # Cas
  # ===
  case_5 = HOMARD.CreateCase('case_5', 'COEUR_2D', DATA_TUTORIAL+'/tutorial_5.00.med')
  case_5.SetDirName(DIRCASE)
  case_5.SetConfType(1)
  case_5.AddBoundaryGroup('Boun_5_1', '')
  #
  # Iteration "iter_5_1"
  # ====================
  iter_5_1 = case_5.NextIteration('iter_5_1')
  iter_5_1.SetMeshName('COEUR_2D_01')
  iter_5_1.SetMeshFile(DIRCASE+'/maill.01.med')
  iter_5_1.AssociateHypo('hypo_5')
  error = iter_5_1.Compute(1, 2)
  #
  # Iteration "iter_5_2"
  # ====================
  iter_5_2 = iter_5_1.NextIteration('iter_5_2')
  iter_5_2.SetMeshName('COEUR_2D_02')
  iter_5_2.SetMeshFile(DIRCASE+'/maill.02.med')
  iter_5_2.AssociateHypo('hypo_5_bis')
  error = iter_5_2.Compute(1, 2)
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
gzip_gunzip(DATA_TUTORIAL, 5, 1)
# ==================================
#
if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser(True)
  iparameters.getSession().restoreVisualState(1)

