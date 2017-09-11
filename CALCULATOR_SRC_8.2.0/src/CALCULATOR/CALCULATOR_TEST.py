# Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
#
# Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
# CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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

import os

import MED_ORB
import CALCULATOR_ORB

import salome

from MEDCoupling import *
from MEDLoader import *
from MEDCouplingCorba import *
from MEDCouplingClient import *

# This test program is based on the field named fieldcelldoublevector in 
# med file ${DATA_DIR}/MedFiles/pointe.med
medFile = os.path.join(os.environ["DATA_DIR"], "MedFiles", "pointe.med")
meshName = "maa1"
fieldName = "fieldcelldoublevector"

# init SALOME session
salome.salome_init()

# Get MED component
print "[CALC] Get reference to MED component ..."
med = salome.lcc.FindOrLoadComponent("FactoryServer", "MED")
print "[CALC] ---"

# Get CALCULATOR component
print "[CALC] Get reference to CALCULATOR component ..."
calculator = salome.lcc.FindOrLoadComponent("FactoryServer", "CALCULATOR")
print "[CALC] ---"

# Get a CORBA field proxy on the distant field (located in the med server)
print "[CALC] Read field %s ..." % fieldName
f = ReadFieldCell(medFile, meshName, 0, fieldName, -1, -1)
fieldcelldouble = MEDCouplingFieldDoubleServant._this(f)

print "[CALC] -> fieldcelldouble is:"
print f
print f.getName()
print f.getDescription()
print f.getNumberOfComponents()
print "[CALC] ---"

print "[CALC] Clone field: fieldcelldoublevector -> f1,f2,f3,f4 ..."
(f1,f2,f3,f4) = calculator.cloneField(fieldcelldouble)
print "[CALC] -> f1 is:"
calculator.printField(f1)
print "[CALC] ---"

print "[CALC] Add fields f2+f3 ..."
f_add = calculator.add(f2, f3)
print "[CALC] -> f_add is:"
calculator.printField(f_add)
print "[CALC] ---"

print "[CALC] Apply linear function to f4 ..."
f_lin = calculator.applyLin(f4, 2.0, 1.0)
print "[CALC] -> f_add is:"
calculator.printField(f_lin)
print "[CALC] ---"

print "[CALC] Apply Norm Max to f_lin ..."
norm = calculator.normMax(f_lin)
print "[CALC] -> norm is ", norm
print "[CALC] ---"

print "[CALC] Clone fields created by Calculator via client classes ..."
f_addLocal = MEDCouplingFieldDoubleClient.New(f_add)
f_addLocal.setName(f_addLocal.getName() + "add")
f_linLocal = MEDCouplingFieldDoubleClient.New(f_lin)
f_linLocal.setName(f_linLocal.getName() + "lin")
print "[CALC] -> f_addLocal is ", f_addLocal
print "[CALC] -> f_linLocal is ", f_linLocal
print "[CALC] ---"

print "[CALC] Get information from the local copy of the distant mesh"
meshLocal = f_addLocal.getMesh()
print "[CALC] -> meshLocal is", meshLocal
print "[CALC] ---"

print "[CALC] Write mesh and fields to MED file ..."
import tempfile
outfile = tempfile.NamedTemporaryFile(prefix="Calculator_pointe_", suffix=".med")
outfile.close()
WriteUMesh(outfile.name, meshLocal, True)
WriteFieldUsingAlreadyWrittenMesh(outfile.name, f_addLocal)
WriteFieldUsingAlreadyWrittenMesh(outfile.name, f_linLocal)
os.remove(outfile.name)
print "[CALC] ---"

print "[CALC] End of Calculator Test!"
