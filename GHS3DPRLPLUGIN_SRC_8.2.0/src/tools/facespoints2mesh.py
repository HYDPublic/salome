#!/usr/bin/env python
#  -*- coding: utf-8 -*-
# Copyright (C) 2007-2016  CEA/DEN, EDF R&D
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
these file is using in GHS3DPRL Plugin
to convert input files .faces and .points of Tepal V1
to input file .mesh of Tepal V2
assume compatibility GHS3DPRL Plugin Tepal V1 => Tepal V2
example of use (when Tepal V2):
  facespoints2mesh.py GHS3DPRL
  tepal2med --casename=GHS3DPRL --number=12 --medname=DOMAIN --launchtepal=no
  mesh2facespoints.py DOMAIN
"""

import os
import sys

file_fp='GHS3DPRL'
if len(sys.argv)==2:
   file_fp=sys.argv[1]

f1=file_fp+'.points'
f2=file_fp+'.faces'
f3=file_fp+'.mesh'

fs=open(f1, 'r')
ft=open(f3, 'w')
ft.write('MeshVersionFormatted 1\n')
ft.write('\nDimension\n3\n')
tmp=fs.readline()
nb=int(tmp)
ft.write('\nVertices\n')
ft.write(tmp)
for i in xrange(0,nb):
  lig=fs.readline()
  ft.write(lig)
fs.close()

fs=open(f2, 'r')
lig=fs.readline()
nb=int(lig.split()[0])
ft.write('\nTriangles\n'+lig.split()[0]+'\n')
for i in xrange(0,nb):
  lig=fs.readline()
  lig=lig.split()
  ft.write(lig[1]+' '+lig[2]+' '+lig[3]+' '+lig[4]+'\n')
ft.write('\nEnd\n')
ft.close()
print 'facespoints2mesh creation of file '+f3
