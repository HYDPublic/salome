#!/usr/bin/env python
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
Exemple de couplage HOMARD-Salome
Copyright EDF-R&D 1996, 2011, 2014
"""
__revision__ = "V3.1"
#
import os
import sys
#
# ==================================
PATH_HOMARD = os.getenv('HOMARD_ROOT_DIR')
# Repertoire des donnees du tutorial
DATA_TUTORIAL = os.path.join(PATH_HOMARD, "share", "doc", "salome", "gui", "HOMARD", "fr", "_downloads")
DATA_TUTORIAL = os.path.normpath(DATA_TUTORIAL)
sys.path.append(DATA_TUTORIAL)
from tutorial_util import gzip_gunzip
from tutorial_util import creation_dircase
# ==================================
DIRCASE = creation_dircase(4)
gzip_gunzip(DATA_TUTORIAL, 4, -1)
# ==================================
#
import salome
salome.salome_init()
import HOMARD
#
homard = salome.lcc.FindOrLoadComponent("FactoryServer", "HOMARD")
study_main = salome.myStudyManager.NewStudy("HOMARD")
homard.SetCurrentStudy(salome.myStudy)
#
# Frontieres
# ==========
boun_4_1 = homard.CreateBoundaryDi('intersection', 'PIQUAGE', DATA_TUTORIAL+'/tutorial_4.fr.med')
#
boun_4_2 = homard.CreateBoundaryCylinder('cyl_1_ext', 0.0, 25., -25., 25., 50., 75., 100.)
#
boun_4_3 = homard.CreateBoundaryCylinder('cyl_2_ext', 17.5, -2.5, -12.5, -100., -75., -25., 50.)
#
boun_4_4 = homard.CreateBoundaryCylinder('cyl_1_int', 0.0, 25., -25., 25., 50., 75., 75.)
#
boun_4_5 = homard.CreateBoundaryCylinder('cyl_2_int', 17.5, -2.5, -12.5, -100., -75., -25., 25.)
#
# Hypotheses
# ==========
# Creation of the hypothesis hypo_4
hypo_4 = homard.CreateHypothesis('hypo_4')
hypo_4.SetUnifRefinUnRef(1)
hypo_4.AddGroup('T1_INT_I')
hypo_4.AddGroup('T1_INT_O')
hypo_4.AddGroup('T2_INT')
# Creation of the hypothesis hypo_4_bis
hypo_4_bis = homard.CreateHypothesis('hypo_4_bis')
hypo_4_bis.SetUnifRefinUnRef(1)
hypo_4_bis.AddGroup('T1_EXT_I')
hypo_4_bis.AddGroup('T1_EXT_O')
hypo_4_bis.AddGroup('T2_EXT')
#
# Cas
# ===
case_4 = homard.CreateCase('Case_4', 'PIQUAGE', DATA_TUTORIAL+'/tutorial_4.00.med')
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

# ==================================
gzip_gunzip(DATA_TUTORIAL, 4, 1)
# ==================================

if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser(True)
