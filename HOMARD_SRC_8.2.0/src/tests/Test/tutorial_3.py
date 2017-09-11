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
Test tutorial_3 associe au tutorial 3
"""
__revision__ = "V3.1"

#========================================================================
TEST_NAME = "tutorial_3"
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
gzip_gunzip(DATA_TUTORIAL, 3, -1)
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
  # Hypothese "hypo_0vers1"
  # =======================
  hypo_0vers1 = HOMARD.CreateHypothesis('hypo_0vers1')
  # Characterization of the field
  hypo_0vers1.SetField('SOLU_0__QIRE_ELEM_SIGM__________')
  hypo_0vers1.SetUseComp(0)
  hypo_0vers1.AddComp('ERREST          ')
  hypo_0vers1.SetRefinThr(3, 1.0)
  hypo_0vers1.SetTypeFieldInterp(2)
  hypo_0vers1.AddFieldInterp('SOLU_0__DEPL____________________')
  hypo_0vers1.AddFieldInterp('SOLU_0__ERRE_ELEM_SIGM__________')
  #
  # Hypothese "hypo_1vers2"
  # =======================
  hypo_1vers2 = HOMARD.CreateHypothesis('hypo_1vers2')
  # Characterization of the field
  hypo_1vers2.SetField('SOLU_1__QIRE_ELEM_SIGM__________')
  hypo_1vers2.SetUseComp(0)
  hypo_1vers2.AddComp('ERREST          ')
  hypo_1vers2.SetRefinThr(3, 1.5)
  hypo_1vers2.SetUnRefThr(3, 6.)
  hypo_1vers2.SetTypeFieldInterp(2)
  hypo_1vers2.AddFieldInterp('SOLU_1__DEPL____________________')
  hypo_1vers2.AddFieldInterp('SOLU_1__QIRE_ELEM_SIGM__________')
  #
  # Hypothese "hypo_1vers2_bis"
  # ===========================
  hypo_1vers2_bis = HOMARD.CreateHypothesis('hypo_1vers2_bis')
  # Characterization of the field
  hypo_1vers2_bis.SetField('SOLU_1__DEPL____________________')
  hypo_1vers2_bis.SetUseComp(1)
  hypo_1vers2_bis.AddComp('DX')
  hypo_1vers2_bis.AddComp('DY')
  hypo_1vers2_bis.AddComp('DZ')
  hypo_1vers2_bis.SetRefinThr(1, 0.0001)
  hypo_1vers2_bis.SetUnRefThr(1, 0.000001)
  hypo_1vers2_bis.SetTypeFieldInterp(0)
  #
  # Cas
  # ===
  case_3 = HOMARD.CreateCase('case_3', 'G_0', DATA_TUTORIAL+'/tutorial_3.00.med')
  case_3.SetDirName(DIRCASE)
  #
  # Iteration "iter_3_1"
  # ====================
  iter_3_1 = case_3.NextIteration('iter_3_1')
  iter_3_1.SetMeshName('H_1')
  iter_3_1.SetMeshFile(DIRCASE+'/maill.01.med')
  iter_3_1.SetFieldFile(DATA_TUTORIAL+'/tutorial_3.00.med')
  iter_3_1.SetTimeStepRank( 1, 1)
  iter_3_1.AssociateHypo('hypo_0vers1')
  error = iter_3_1.Compute(1, 2)
  #
  # Iteration "iter_3_2"
  # ====================
  iter_3_2 = iter_3_1.NextIteration('iter_3_2')
  iter_3_2.SetMeshName('H_2')
  iter_3_2.SetMeshFile(DIRCASE+'/maill.02.med')
  iter_3_2.SetFieldFile(DATA_TUTORIAL+'/tutorial_3.01.med')
  iter_3_2.SetTimeStepRank(1, 1)
  iter_3_2.AssociateHypo('hypo_1vers2')
  error = iter_3_2.Compute(1, 2)
  #
  # Iteration "iter_3_2_bis"
  # ========================
  iter_3_2_bis = iter_3_1.NextIteration('iter_3_2_bis')
  iter_3_2_bis.SetMeshName('H_2_bis')
  iter_3_2_bis.SetMeshFile(DIRCASE+'/maill.02.bis.med')
  iter_3_2_bis.SetFieldFile(DATA_TUTORIAL+'/tutorial_3.01.med')
  iter_3_2_bis.SetTimeStepRank(1, 1)
  iter_3_2_bis.AssociateHypo('hypo_1vers2_bis')
  error = iter_3_2_bis.Compute(1, 2)
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
N_REP_TEST_FILE = 3
DESTROY_DIR = not DEBUG
test_results(REP_DATA, TEST_NAME, DIRCASE, N_ITER_TEST_FILE, N_REP_TEST_FILE, DESTROY_DIR)
#
# ==================================
gzip_gunzip(DATA_TUTORIAL, 3, 1)
# ==================================
#
if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser(True)
  iparameters.getSession().restoreVisualState(1)

