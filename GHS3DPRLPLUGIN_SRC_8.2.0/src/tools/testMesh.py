#!/usr/bin/env python
# -*- coding: utf-8 -*-

# %% LICENSE_SALOME_CEA_BEGIN
# Copyright (C) 2008-2016  CEA/DEN
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
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
# 
# See http://www.salome-platform.org or email : webmaster.salome@opencascade.com
# %% LICENSE_END


"""
run coherency tests on one and some mesh.
initially used for test output(s) mg_tetra_hpc_mpi.exe

WARNING: is for small meshes, obviously no optimisation.
WARNING: printing and .mesh indices are 1 to n when stored list python 0 to n-1

example linux usage:
- simple run:
  ./testMesh.py --verbose --testall --files ./GHS3DPRL_out.000001.mesh ./GHS3DPRL_out.000002.mesh
  ./testMesh.py -a -f /tmp/GHS3DPRL_out.00000?.mesh
"""

import os
import sys
import platform
import argparse as AP
import pprint as PP #pretty print

verbose = False

OK = "ok"
KO = "KO"
OKSYS = 0 #for linux
KOSYS = 1 #for linux


#########################################
# utilities

def okToSys(aResult, verbose=False):
  """to get windows or linux result of script"""
  
  def extendList(alist):
    """utility extend list of lists of string results with ok or KO"""
    #bad: list(itertools.chain.from_list(alist)) iterate on str
    res = []
    if type(alist) != list:
      return [alist]
    else:
      for i in alist:
        if type(i) == str:
           res.append(i)
        else:
           res.extend(extendList(i))
    return res
      
  resList = extendList(aResult)
  if resList == []:
    if verbose: print("WARNING: result no clear: []")
    return KOSYS
    
  rr = OK
  for ri in resList:
    if ri[0:2] != OK:
      if verbose: print ri
      rr = KO

  if verbose: print("INFO: result: %s" % rr)
  if rr == OK:
    return OKSYS
  else:
    return KOSYS

def getDirAndName(datafile):
  path, namefile = os.path.split(os.path.realpath(datafile))
  rootpath = os.getcwd()
  return (path, rootpath, namefile)

def resumeList(aList):
  if len(aList) == 0: 
    return []
  if len(aList) < 3:
    return aList
  res = [aList[0], "...", aList[-1]]
  return res

def resumeLines(aList, ilines):
  if verbose: print("INFO: resumeLines", ilines)
  if len(aList) == 0: 
    return []
  if len(aList) < 3:
    return aList
  res = []
  for i in ilines:
    if i != None: # if not existing tetrahedra for example
      resi = [ii.strip("\n") for ii in aList[i:i+4]]
      resi.append("...")
      res.append(resi)
  return res


#########################################
class XXVert(object): 
  """Vertices, Nodes"""
  def __init__(self, x, y, z, color=0, indexglobal=0):
    self.x = x
    self.y = y
    self.z = z
    self.color = color
    self.indexglobal = indexglobal

  def compare(self, vb, args, withAll=True):
    if self.x != vb.x: return False
    if self.y != vb.y: return False
    if self.z != vb.z: return False
    if withAll:
      if args.withColor:
        if self.color != vb.color: return False
      if args.withIndex:
        if self.indexglobal != vb.indexglobal: return False
    return True

  def __eq__(self, vb):
    """equality test without color or indexglobal"""
    #print "vertice equality"
    if self.x != vb.x: return False
    if self.y != vb.y: return False
    if self.z != vb.z: return False
    return True

  def __ne__(self, vb):
    """inequality test without color or indexglobal"""
    #print "vertice inequality"
    if self.x == vb.x and self.y == vb.y and self.z != vb.z: 
      return True
    return False

  def __repr__(self):
    return "XXVert(%.4f %.4f %.4f (%i %i))" % \
           (self.x, self.y, self.z, self.color, self.indexglobal)

  def __str__(self):
    return "(%s %s %s (%i %i))" % \
           (self.x, self.y, self.z, self.color, self.indexglobal)

  def dist(self, vb):
    res = (self.x - vb.x)**2 + (self.y - vb.y)**2 + (self.z -vb.z)**2
    return res**.5



#########################################
class XXEdge(object):
  """Edges, 2 Nodes"""
  def __init__(self, a, b, color=0, indexglobal=0):
    self.a = a
    self.b = b
    self.color = color
    self.indexglobal = indexglobal

  def compare(self, eb, args):
    res = self.a.compare(eb.a, args) and \
          self.b.compare(eb.b, args)
    if res:
      if args.withColor:
        if self.color != eb.color: return False
      if args.withIndex:
        if self.indexglobal != eb.indexglobal: return False
    return res

  def __repr__(self):
    return "XXEdge(%i %i (%i %i))" % \
           (self.a, self.b, self.color, self.indexglobal)

  def __str__(self):
    return "(%i %i (%i %i))" % \
           (self.a, self.b, self.color, self.indexglobal)

  def inTria(self, tria, args):
    t = [tria.a, tria.b, tria.c]
    if not self.a in t: return False
    if not self.b in t: return False
    return True

  def getVertices(self, mesh):
    v1 = mesh.verts[self.a - 1]
    v2 = mesh.verts[self.b - 1]
    return [v1, v2]


#########################################
class XXTria(object):
  """Triangles, Faces, 3 nodes"""
  def __init__(self, a, b, c, color=0, indexglobal=0):
    self.a = a
    self.b = b
    self.c = c
    self.color = color
    self.indexglobal = indexglobal

  def compare(self, trb, args):
    res = self.a.compare(trb.a, args) and \
          self.b.compare(trb.b, args) and \
          self.c.compare(trb.c, args)
    if res:
      if args.withColor:
        if self.color != trb.color: return False
      if args.withIndex:
        if self.indexglobal != trb.indexglobal: return False
    return res

  def __repr__(self):
    return "XXTria(%i %i %i (%i %i))" % \
           (self.a, self.b, self.c, self.color, self.indexglobal)

  def __str__(self):
    return "(%i %i %i (%i %i))" % \
           (self.a, self.b, self.c, self.color, self.indexglobal)

  def inTetra(self, tetra, args):
    t = [tetra.a, tetra.b, tetra.c, tetra.d]
    if not self.a in t: return False
    if not self.b in t: return False
    if not self.c in t: return False
    return True

  def getVertices(self, mesh):
    v1 = mesh.verts[self.a - 1]
    v2 = mesh.verts[self.b - 1]
    v3 = mesh.verts[self.c - 1]
    return [v1, v2, v3]



#########################################
class XXTetra(object):
  """Tetra, 4 nodes"""
  def __init__(self, a, b, c, d, color=0, indexglobal=0):
    self.a = a
    self.b = b
    self.c = c
    self.d = d
    self.color = color
    self.indexglobal = indexglobal

  def compare(self, teb, args):
    res = self.a.compare(teb.a, args) and \
          self.b.compare(teb.b, args) and \
          self.c.compare(teb.c, args) and \
          self.d.compare(teb.d, args)
    if res:
      if args.withColor:
        if self.color != teb.color: return False
      if args.withIndex:
        if self.indexglobal != teb.indexglobal: return False
    return res

  def __repr__(self):
    return "XXTetra(%i %i %i %i (%i %i))" % \
           (self.a, self.b, self.c, self.d, self.color, self.indexglobal)

  def __str__(self):
    return "(%i %i %i %i (%i %i))" % \
           (self.a, self.b, self.c, self.d, self.color, self.indexglobal)

  def getVertices(self, mesh):
    v1 = mesh.verts[self.a - 1]
    v2 = mesh.verts[self.b - 1]
    v3 = mesh.verts[self.c - 1]
    v4 = mesh.verts[self.d - 1]
    return [v1, v2, v3, v4]


#########################################
class XXMesh(object):
  """Mesh: vertices, edges, triangles, tetrahedra"""
  def __init__(self):
    self.nameFile = ""
    self.verts = []
    self.edges = []
    self.trias = []
    self.tetras = []

  def initFromFileMesh(self, fileName, args, withGlobal=True):
    if not os.path.isfile(fileName):
      raise Exception("ERROR: inexisting file '%s'" % fileName)
    with open(fileName, "r") as f:
      lines = f.readlines()
    iverts, iedges, itrias, itetras = self.getIndexInMeshFile(lines)
    self.verts = self.getMeshVerts(lines, iverts)
    self.edges = self.getMeshEdges(lines, iedges)
    self.trias = self.getMeshTrias(lines, itrias)
    self.tetras = self.getMeshTetras(lines, itetras)
    self.nameFile = fileName
    if args.globalNumerotation == True and withGlobal==True:
      self.initFromFileGlobal(fileName, args)
    if verbose: 
      print("\nINFO: initFromFileMesh: read file: %s" % str(self))
      print(self.strResume())
      print(PP.pformat(resumeLines(lines, [iverts, iedges, itrias, itetras])))

  def initFromFileGlobal(self, fileNameMeshOrGlobal, args):
    shortname, extension = os.path.splitext(fileNameMeshOrGlobal)
    if extension == ".mesh":
      fileName = shortname +  ".global"
    elif extension == "global":
      fileName = fileNameMeshOrGlobal
    else:
      raise Exception("ERROR: initFromFileGlobal: unexpected file '%s'" % fileName)
    if not os.path.isfile(fileName):
      raise Exception("ERROR: initFromFileGlobal: inexisting file '%s'" % fileName)
    
    with open(fileName, "r") as f:
      lines = f.readlines()
    nbverts, nbedges, nbtrias, nbtetras = [int(i) for i in lines[0].split()]
    if verbose:
      print("\nINFO: initFromFileGlobal: read file: %s" % str(self))
      print("  nbverts %i\n  nbedges %i\n  nbtrias %i\n  nbtetras %i" % (nbverts, nbedges, nbtrias, nbtetras))
    if nbverts != len(self.verts): 
      raise Exception("ERROR: in file '%s' unexpected number of Vertices %i<>%i" % (fileName, nbverts, len(self.verts)))
    if nbedges != len(self.edges): 
      raise Exception("ERROR: in file '%s' unexpected number of Edges %i<>%i" % (fileName, nbedges, len(self.edges)))
    if nbtrias != len(self.trias): 
      raise Exception("ERROR: in file '%s' unexpected number of Triangles %i<>%i" % (fileName, nbtrias, len(self.trias)))
    if nbtetras != len(self.tetras): 
      raise Exception("ERROR: in file '%s' unexpected number of Tetrahedra %i<>%i" % (fileName, nbtetras, len(self.tetras)))
    i = 1 #begin index line 1
    for ii in range(nbverts):
      self.verts[ii].indexglobal = long(lines[i])
      i +=1
    for ii in range(nbedges):
      self.edges[ii].indexglobal = long(lines[i])
      i +=1
    for ii in range(nbtrias):
      self.trias[ii].indexglobal = long(lines[i])
      i +=1
    for ii in range(nbtetras):
      self.tetras[ii].indexglobal = long(lines[i])
      i +=1


  def __repr__(self):
    return "XXMesh(nameFile='%s', nbverts=%i, nbedges=%i, nbtrias=%i, nbtetras=%i)" % \
           (self.nameFile, len(self.verts), len(self.edges), len(self.trias), len(self.tetras))

  def strResume(self):
    res = str(self)
    contents = {
      "Vertices": resumeList(self.verts),
      "Edges": resumeList(self.edges),
      "Triangles": resumeList(self.trias),
      "Tetrahedra": resumeList(self.tetras),
    }
    res = res + "\n" + PP.pformat(contents)
    return res

  def getIndexInMeshFile(self, lines):
    res = []
    for s in ["Vertices", "Edges", "Triangles", "Tetrahedra"]:
      try:
        i = lines.index(s+"\n")
      except:
        i = None
      res.append(i)
    return res

  def getMeshVerts(self, lines, i):
    res=[]
    try:
      idep = i+2
      ilen = int(lines[i+1])
      ifin =  idep+ilen
      for line in lines[idep:ifin]:
        li = line.split(" ")
        x, y, z, color = float(li[0]), float(li[1]), float(li[2]), int(li[3])
        res.append(XXVert(x, y, z, color))
      return res
    except:
      return res

  def getMeshEdges(self, lines, i):
    res=[]
    try:
      idep = i+2
      ilen = int(lines[i+1])
      ifin =  idep+ilen
      for line in lines[idep:ifin]:
        li = line.split(" ")
        a, b, color = int(li[0]), int(li[1]), int(li[2])
        res.append(XXEdge(a, b, color))
      return res
    except:
      return res

  def getMeshTrias(self, lines, i):
    res=[]
    try:
      idep = i+2
      ilen = int(lines[i+1])
      ifin =  idep+ilen
      for line in lines[idep:ifin]:
        li = line.split(" ")
        a, b, c, color = int(li[0]), int(li[1]), int(li[2]), int(li[3])
        res.append(XXTria(a, b, c, color))
      return res
    except:
      return res

  def getMeshTetras(self, lines, i):
    res=[]
    try:
      idep = i+2
      ilen = int(lines[i+1])
      ifin =  idep+ilen
      for line in lines[idep:ifin]:
        li = line.split(" ")
        a, b, c, d, color = int(li[0]), int(li[1]), int(li[2]), int(li[3]), int(li[4])
        res.append(XXTetra(a, b, c, d, color))
      return res
    except:
      return res

  def haveVertsDistinct(self, args):
    """stop a first KO"""
    i = 0
    verts = self.verts
    for v1 in verts[:-1]:
      i += 1
      j = i
      for v2 in verts[i:]:
        j += 1
        if v1.compare(v2, args):
          #printing indices 1 to n
          print("ERROR: %s vert[%i] equal vert[%i]: v1=%s v2=%s" % (self.nameFile, i, j, v1, v2))
          return KO + " ERROR: %s some equal vertices" % self.nameFile #stop a first KO
    return OK + " INFO: no equal vertices"

  def getVertices(self, elem):
    """functionnal raccourci to XXElem.getVertices(XXMesh)"""
    return elem.getVertices(self)

  def compareListOfVertices(self, v1s, v2s, ordered=False):
    """not ordered for now"""
    if ordered:
      res = [i for i, j in zip(v1s, v2s) if i == j]
      return len(res)==len(v1s)
    else:       
      res = 0
      for i in v1s:
        for j in v2s:
          if i == j: 
            res += 1
            break
      return res==len(v1s)
    
    
  def getCommonVerts(self, mesh, args):
    res = []
    for v1 in self.verts:
      for v2 in mesh.verts:
        if v1.compare(v2, args, withAll=False):
          res.append((v1, v2))
    return res

  def getVertices(self, elem):
    return elem.getVertices(self)

  def getCommonEdges(self, mesh, args):
    res = []
    for e1 in self.edges:
      v1s = self.getVertices(e1)
      for e2 in mesh.edges:
        v2s = mesh.getVertices(e2)
        if self.compareListOfVertices(v1s, v2s):
          res.append((e1, e2))
    return res

  def getCommonTriangles(self, mesh, args):
    res = []
    for e1 in self.trias:
      v1s = self.getVertices(e1)
      for e2 in mesh.trias:
        v2s = mesh.getVertices(e2)
        if self.compareListOfVertices(v1s, v2s):
          res.append((e1, e2))
    return res

  def getCommonTetras(self, mesh, args):
    res = []
    for e1 in self.tetras:
      v1s = self.getVertices(e1)
      for e2 in mesh.tetras:
        v2s = mesh.getVertices(e2)
        if self.compareListOfVertices(v1s, v2s):
          res.append((e1, e2))
    return res

  def areEdgesInTrias(self, args):
    """stop a first KO"""
    done = False
    i = 0
    edges = self.edges
    trias = self.trias
    res = OK + " INFO: %s all edges in trias" % self.nameFile
    for e in edges:
      i += 1
      j = 0
      found = False
      for t in trias:
        j += 1
        if e.inTria(t, args):
          #if verbose: print("INFO: %s edges[%i] in trias[%i]: edge=%s tria=%s" % (self.nameFile, i, j, e, t))
          found = True
          break
      if not found:
        print("ERROR: %s edges[%i] not in trias: edge=%s" % (self.nameFile, i, e))
        if verbose and not done: 
          print("Triangles:\n%s" % PP.pformat(self.trias))
          done = True
        res = KO+" ERROR: %s some edges not in trias" % (self.nameFile)
    return res


  def areTriasInTetras(self, args):
    """no stop a first KO"""
    done = False
    i = 0
    trias = self.trias
    tetras = self.tetras
    if tetras == []: #supposed skin without tetrahedra
      res = OK +" WARNING: %s no tetrahedra in mesh" % (self.nameFile)
      return res
    res = OK + " INFO: %s all trias in tetras" % self.nameFile
    for t in trias:
      i += 1
      j = 0
      found = False
      for h in tetras:
        j += 1
        if t.inTetra(h, args):
          #if verbose: print("INFO: %s trias[%i] in tetras[%i]: tria=%s tetra=%s" % (self.nameFile, i, j, t, h))
          found = True
          break
      if not found:
        if verbose: print("ERROR: %s trias[%i] not in tetras: tria=%s" % (self.nameFile, i, t))
        if verbose and not done: 
          print("INFO: Tetrahedra:\n%s" % PP.pformat(self.tetras))
          done = True
        res = KO+" ERROR: %s some trias not in tetras" % (self.nameFile)
    return res

  def testIntersection(self, mesh, args):
    """intersection coherency between self and mesh"""

    def storeAndInfoIntersection():
      """used as macro: avoid duplicate code"""
      #store info in args to use later...
      args.intersections[title+name] = commons
      if commons == []:
        res.append(OK + " INFO: no %s" % title+name)
      else:
        res.append(OK + " INFO: existing %s" % title+name)
      return
      
    res=[]
    name = "%s<->%s" % (self.nameFile, mesh.nameFile)
    
    title = "Vertices intersection: "
    commons = self.getCommonVerts(mesh, args)
    storeAndInfoIntersection()

    title = "Edges intersection: "
    commons = self.getCommonEdges(mesh, args)
    storeAndInfoIntersection()

    title = "Triangles intersection: "
    commons = self.getCommonTriangles(mesh, args)
    storeAndInfoIntersection()

    title = "Tetrahedra intersection: "
    commons = self.getCommonTetras(mesh, args)
    storeAndInfoIntersection()

    return res
      
  def testIndexGlobal(self, mesh, args):
    """global index coherency between self and mesh"""
    
    def storeAndInfoIndexGlobal():
      """used as macro: avoid duplicate code"""
      #store info in args to use later...
      args.indexglobal[title+name] = problems
      if verbose: print("\nINFO: %s\n%s" % (title+name, PP.pformat(problems)))
      if problems == []:
        res.append(OK + " INFO: coherent %s" % title+name)
      else:
        res.append(KO + " ERROR: some problems %s" % title+name)
      return

    def testIndexGlobal():
      """used as macro: avoid duplicate code"""
      nameElem = title.split(' ')[0]
      #something like 'Vertices intersection: /tmp/GHS3DPRL_out.000002.mesh<->/tmp/GHS3DPRL_out.000003.mesh'
      commonsTitle = nameElem + " intersection: "+ name
      #if verbose: print "testIndexGlobal",title,commonsTitle
      try:
        intersection = args.intersections[commonsTitle]
      except:
        intersection = []
      problems = []
      for ii, jj in intersection:
        if ii.indexglobal != jj.indexglobal:
          problems.append((ii, jj))
      return problems

    res=[]
    name = "%s<->%s" % (self.nameFile, mesh.nameFile)

    title = "Vertices indexglobal: "
    problems = testIndexGlobal()
    storeAndInfoIndexGlobal()

    title = "Edges indexglobal: "
    problems = testIndexGlobal()
    storeAndInfoIndexGlobal()

    title = "Triangles indexglobal: "
    problems = testIndexGlobal()
    storeAndInfoIndexGlobal()

    title = "Tetrahedra indexglobal: "
    problems = testIndexGlobal()
    storeAndInfoIndexGlobal()

    return res


#########################################
# tests
 
def testAll(args):
  """test all on meshes from tetra_hpc_mpi"""
  res = [] 
  if verbose: print("\n*****testAll*****\n")
  args.skinMesh = None
  if args.skinInputFile != None:
    args.skinMesh = XXMesh()
    #a priori no global numerotation file.global for input tetra_hpc_mpi mesh
    args.skinMesh.initFromFileMesh(args.skinInputFile, args, withGlobal=False)
    res.append(testStandaloneMesh(args.skinMesh, args))
    print("\nINFO: testAll skin input file:\n%s" % (PP.pformat(args.skinMesh)))
    
  meshes = []
  for fileName in args.files:
    xxmesh = XXMesh()
    xxmesh.initFromFileMesh(fileName, args)
    meshes.append(xxmesh)
  print("\nINFO: testAll ouput files:\n%s\n" % (PP.pformat(meshes)))
  #test coherence of one by one meshes
  for mesh in meshes:
    res.append(testStandaloneMesh(mesh, args))
  #test coherence of intersections an global numerotation of tetra_hpc_mpi output meshes
  res.append(testParallelMesh(meshes, args))
  res.append(testParallelMeshAndSkin(meshes, args))
  res.append(testParallelMeshAndSkinColor(meshes, args))
  return res

  
def testStandaloneMesh(mesh, args):
  """test coherence of one mesh alone"""
  if verbose: print("\nINFO: testStandaloneMesh:\n%s" % PP.pformat(mesh))
  res = []
  res.append(mesh.haveVertsDistinct(args))
  res.append(mesh.areEdgesInTrias(args))
  res.append(mesh.areTriasInTetras(args))
  return res


def testParallelMesh(meshes, args):
  """test intersection and overriding in tetra_hpc_mpi outputs GHS3DPRL_out.00000?.mesh"""
  i = 0
  res = []
  args.intersections = {}
  args.indexglobal = {}
  for m1 in meshes[:-1]:
    i += 1
    for m2 in meshes[i:]:
      res.append(m1.testIntersection(m2, args))
      res.append(m1.testIndexGlobal(m2, args))
  if verbose: 
    print("\nINFO: intersections\n%s" % PP.pformat(args.intersections))
    print("\nINFO: indexglobal\n%s" % PP.pformat(args.indexglobal))
  return res

def testParallelMeshAndSkin(meshes, args):
  """test coherency between input skin and tetra_hpc_mpi outputs GHS3DPRL_out.00000?.mesh"""
  res = []
  if args.skinMesh == None:
    print("INFO: no skin Mesh for testing intersectionsSkin\n")
    res = OK + "INFO: no skin Mesh for testing intersectionsSkin"
    return res
  nbtriasskin = len(args.skinMesh.trias)
  for m1 in meshes:
    res.append(args.skinMesh.testIntersection(m1, args))
    res.append(args.skinMesh.testIndexGlobal(m1, args))
  
  #test total Triangles in output parallel meshes vs input skin mesh
  if True:
    kk = {} 
    nbtriaspara = 0
    for k in args.intersections.keys():
      if args.skinMesh.nameFile in k:
        ll = len(args.intersections[k])
        if "Triangles intersection" in k: 
          nbtriaspara += ll
        kk[k] = len(args.intersections[k])
    print("INFO: skin intersections\n%s\n" % PP.pformat(kk))
    if nbtriaspara < nbtriasskin:
      res.append(KO + " ERROR: problem all skin triangles not in parallel meshes: %i<->%i" % (nbtriasskin, nbtriaspara))
  return res

def testParallelMeshAndSkinColor(meshes, args):
  """test coherency between color input skin and tetra_hpc_mpi outputs GHS3DPRL_out.00000?.mesh"""
  res = []
  if args.color == True:
    res.append(KO + " ERROR: test color TODO!!!")
  else:
    res.append(OK + " WARNING: test color not done")
  return res

if __name__ == '__main__':
  parser = AP.ArgumentParser(description='launch test(s) on tetra_hpc_mpi mesh(es)', argument_default=None)
  parser.add_argument(
    '-a', '--testAll', 
    help='test all on all meshes',
    action='store_true',
  )
  parser.add_argument(
    '-v', '--verbose', 
    help='set verbose, for deep debug',
    action='store_true',
  )
  parser.add_argument(
    '-g', '--globalNumerotation', 
    help='read and set files .global, if associated',
    action='store_true',
  )
  parser.add_argument(
    '-c', '--color', 
    help='read and test with color',
    action='store_true',
  )
  parser.add_argument(
    '-f', '--files', 
    help='launch test(s) on file(s)',
    nargs='*',
    metavar='.../file.mesh'
  )
  parser.add_argument(
    '-s', '--skinInputFile', 
    help='launch test(s) on tetra_hpc_mpi input file',
    nargs='?',
    metavar='.../skinInputFile.mesh'
  )
  """
  parser.add_argument(
    '-x', '--xoneargument', 
    nargs='?',
    metavar='0|1',
    choices=['0', '1'],
    help='one argument, for example',
    default='0'
  )
  """
  
    
  """
  args is Namespace, use it as global to store 
  parameters, data, used arrays and results and other...
  """
  args = parser.parse_args()
  
  verbose = args.verbose
  if verbose: print("INFO: args:\n%s" % PP.pformat(args))

  if len(sys.argv) == 1: #no args as --help
    parser.print_help()
    sys.exit(KOSYS)

  if args.files == None:
    print("\nERROR: Nothing to do: no files\n%s" % PP.pformat(args))
    parser.print_help()
    sys.exit(KOSYS)
  
  if args.testAll: 
    result = testAll(args)
  else:
    result = KO
    print("\nERROR: Nothing to do:\n%s" % PP.pformat(args))
  sys.exit(okToSys(result, verbose=True))
  
