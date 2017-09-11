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
Test tutorial_4 associe au tutorial 4
"""
__revision__ = "V3.1"

#========================================================================
TEST_NAME = "tutorial_4"
DEBUG = False
N_ITER_TEST_FILE = 3
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
gzip_gunzip(DATA_TUTORIAL, 4, -1)
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
  # Frontieres
  # ==========
  boun_4_1 = HOMARD.CreateBoundaryDi('intersection', 'PIQUAGE', DATA_TUTORIAL+'/tutorial_4.fr.med')
  #
  boun_4_2 = HOMARD.CreateBoundaryCylinder('cyl_1_ext', 0.0, 25., -25., 25., 50., 75., 100.)
  #
  boun_4_3 = HOMARD.CreateBoundaryCylinder('cyl_2_ext', 17.5, -2.5, -12.5, -100., -75., -25., 50.)
  #
  boun_4_4 = HOMARD.CreateBoundaryCylinder('cyl_1_int', 0.0, 25., -25., 25., 50., 75., 75.)
  #
  boun_4_5 = HOMARD.CreateBoundaryCylinder('cyl_2_int', 17.5, -2.5, -12.5, -100., -75., -25., 25.)
  #
  # Hypotheses
  # ==========
  # Creation of the hypothesis hypo_4
  hypo_4 = HOMARD.CreateHypothesis('hypo_4')
  hypo_4.SetUnifRefinUnRef(1)
  hypo_4.AddGroup('T1_INT_I')
  hypo_4.AddGroup('T1_INT_O')
  hypo_4.AddGroup('T2_INT')
  # Creation of the hypothesis hypo_4_bis
  hypo_4_bis = HOMARD.CreateHypothesis('hypo_4_bis')
  hypo_4_bis.SetUnifRefinUnRef(1)
  hypo_4_bis.AddGroup('T1_EXT_I')
  hypo_4_bis.AddGroup('T1_EXT_O')
  hypo_4_bis.AddGroup('T2_EXT')
  #
  # Cas
  # ===
  case_4 = HOMARD.CreateCase('case_4', 'PIQUAGE', DATA_TUTORIAL+'/tutorial_4.00.med')
  case_4.SetDirName(DIRCASE)
  case_4.AddBoundaryGroup( 'intersection', '' )
  case_4.AddBoundaryGroup( 'cyl_1_int', 'T1_INT_I' )
  case_4.AddBoundaryGroup( 'cyl_1_ext', 'T1_EXT_I' )
  case_4.AddBoundaryGroup( 'cyl_1_int', 'T1_INT_O' )
  case_4.AddBoundaryGroup( 'cyl_1_ext', 'T1_EXT_O' )
  case_4.AddBoundaryGroup( 'cyl_2_int', 'T2_INT' )
  case_4.AddBoundaryGroup( 'cyl_2_ext', 'T2_EXT' )
  #
  # Iterations
  # ==========
  # Iteration iter_4_1 : raffinement selon les faces internes
  iter_4_1 = case_4.NextIteration('iter_4_1')
  iter_4_1.SetMeshName('PIQUAGE_1')
  iter_4_1.SetMeshFile(DIRCASE+'/maill.01.med')
  iter_4_1.AssociateHypo('hypo_4')
  error = iter_4_1.Compute(1, 2)
  # Iteration iter_4_2 : raffinement selon les faces externes
  iter_4_2 = iter_4_1.NextIteration('iter_4_2')
  iter_4_2.SetMeshName('PIQUAGE_2')
  iter_4_2.SetMeshFile(DIRCASE+'/maill.02.med')
  iter_4_2.AssociateHypo('hypo_4_bis')
  error = iter_4_2.Compute(1, 2)
  # Iteration iter_4_3 : second raffinement selon les faces externes
  iter_4_3 = iter_4_2.NextIteration('iter_4_3')
  iter_4_3.SetMeshName('PIQUAGE_3')
  iter_4_3.SetMeshFile(DIRCASE+'/maill.03.med')
  iter_4_3.AssociateHypo('hypo_4_bis')
  error = iter_4_3.Compute(1, 2)
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
gzip_gunzip(DATA_TUTORIAL, 4, 1)
# ==================================
#
if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser(True)
  iparameters.getSession().restoreVisualState(1)

