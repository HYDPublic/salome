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
to convert output files .mesh of Tepal V2
to output files files .faces and .points of Tepal V1
assume compatibility GHS3DPRL Plugin Tepal V1 => Tepal V2
example of use (when Tepal V2):
  facespoints2mesh.py GHS3DPRL
  tepal2med --casename=GHS3DPRL --number=12 --medname=DOMAIN --launchtepal=no
  mesh2facespoints.py DOMAIN
"""

import os
import sys
import glob
import fileinput
import string

file_m='GHS3DPRL'
if len(sys.argv)==2:
   file_m=sys.argv[1]
   
def find_cr_or_not(fs,tag):
  """find number after tag with cr or sp"""
  for line in fs:
    if tag+'\n' in line:
      #print tag+'<cr>'
      res=fs.readline()
      break
    if tag+' ' in line:
      #print 'vertices<sp>'
      res=line.split()[1]
      break
  res=res.strip("\t\n ")
  print tag+' '+res
  return res

def m2fp(f1):
  """convert .mesh file to .points and .faces and .noboite"""
  print '\nconversion '+f1+' to .points and .faces and .noboite'
  #fs=open(f1, 'r')
  #fs=fileinput.FileInput(f1,mode='r') #mode not in v2.4.4
  fs=fileinput.FileInput(f1)
  (shortname, extension)=os.path.splitext(f1)
  f2=shortname+'.points'
  print 'creating',f2
  fp=open(f2, 'w')
  nb=find_cr_or_not(fs,'Vertices')
  np=nb #for .noboite
  fp.write(nb+'\n')
  for i in xrange(0,int(nb)):
    fp.write(fs.readline())
  fp.close()
  
  f2=shortname+'.faces'
  print 'creating',f2
  ff=open(f2, 'w')
  nb=find_cr_or_not(fs,'Triangles')
  ff.write(nb+' 0\n')
  for i in xrange(0,int(nb)):
    ff.write('3 '+fs.readline().strip('\t\n ')+' 0 0 0\n')
  ff.close()
  
  ne=find_cr_or_not(fs,'Tetrahedra')
  f3=shortname+'.noboite'
  fb=open(f3, 'w')
  npfixe="0"
  fb.write(ne+' '+np+' '+npfixe+' 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n')
  for i in xrange(0,int(ne)):
    lig=fs.readline().strip('\t\n ')
    lig=lig.split()
    fb.write(lig[0]+" "+lig[1]+" "+lig[2]+" "+lig[3]+" ")
  fb.write('\n')
  fs.close()
  fs=fileinput.FileInput(shortname+'.points')
  nb=fs.readline() #eqal to np
  for i in xrange(0,int(nb)):
    lig=fs.readline().strip('\t\n ')
    lig=lig.split()
    fb.write(lig[0]+" "+lig[1]+" "+lig[2]+" ")
  fb.write('\n0\n') #subnumber
  fs.close()
  fb.close()

def rename_tepal_v1(f1,imax):
  """rename files as version v1 of tepal expect"""
  (shortname, extension)=os.path.splitext(f1)
  fs=os.path.splitext(shortname)
  i=int(fs[1].strip('.'))
  ff=fs[0]+'.'+str(imax)+'.'+string.zfill(str(i),len(str(imax)))
  #noboite en ".32.02.noboite!"
  mvcp='mv ' #ou 'cp '
  f2=shortname+'.points' ; f3=ff+os.path.splitext(f2)[1]
  print f2,'->',f3 ; os.system(mvcp+f2+' '+f3)
  f2=shortname+'.faces' ; f3=ff+os.path.splitext(f2)[1]
  print f2,'->',f3 ; os.system(mvcp+f2+' '+f3)
  f2=shortname+'.noboite' ; f3=ff+os.path.splitext(f2)[1]
  print f2,'->',f3 ; os.system(mvcp+f2+' '+f3)
  f2=shortname+'.glo' ; f3=ff+os.path.splitext(f2)[1]
  print f2,'->',f3 ; os.system(mvcp+f2+' '+f3)
  f2=shortname+'.msg' ; f3=ff+os.path.splitext(f2)[1]
  print f2,'->',f3 ; os.system(mvcp+f2+' '+f3)

def my_test(a): return int(os.path.basename(a).split('.')[1])

f0=file_m+'.?????.mesh'
#print f0
fics=glob.glob(f0)
fics.sort(lambda a, b: cmp(my_test(b), my_test(a))) #tri ordre decroissant
print 'conversion of files:\n',fics

imax=len(fics)
for f in fics:
  m2fp(f)
  rename_tepal_v1(f,imax)
