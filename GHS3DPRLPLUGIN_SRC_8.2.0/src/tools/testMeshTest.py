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


import unittest
import testMesh as TM
from argparse import Namespace
    
verbose = False

class TestCase(unittest.TestCase):

  def test_010(self):
    self.failUnlessEqual(TM.okToSys([]), TM.KOSYS)
    self.failUnlessEqual(TM.okToSys([[]]), TM.KOSYS)
    
    res = TM.OK + " blah blah 0"
    self.failUnlessEqual(TM.okToSys([[res]]), TM.OKSYS)
    self.failUnlessEqual(TM.okToSys(res), TM.OKSYS)
    res = [res]
    self.failUnlessEqual(TM.okToSys(res), TM.OKSYS)
    res.append(TM.OK + " blah blah 1")
    self.failUnlessEqual(TM.okToSys(res), TM.OKSYS)  
    res.append([TM.OK + " blah blah 2"])
    self.failUnlessEqual(TM.okToSys(res), TM.OKSYS) 
    res.append([TM.KO + " blah blah 3"])
    self.failUnlessEqual(TM.okToSys(res), TM.KOSYS)
    self.failUnlessEqual(TM.okToSys([]), TM.KOSYS)
    res = [[[res]],[TM.OK + " blah blah 4"],[]]
    self.failUnlessEqual(TM.okToSys(res), TM.KOSYS)

  def test_030(self):
    a = TM.XXVert(1, 1, 1)
    b = TM.XXVert(2, 2, 2)
    c = TM.XXVert(1., 1., 1., 3, 4)
    self.failUnlessEqual(a==c, True)
    
    from argparse import Namespace
    args = Namespace(withColor=True, withIndex=True)
    self.failUnlessEqual(a.compare(c, args, withAll=True), False)
    self.failUnlessEqual(a.compare(c, args, withAll=False), True)
    
    args = Namespace(withColor=False, withIndex=False)
    self.failUnlessEqual(a.compare(c, args, withAll=True), True)
    
    self.failUnlessEqual(a==b, False)
    self.failUnlessEqual(a.dist(b), 3**.5)
    
    self.failUnlessEqual(str(c), '(1.0 1.0 1.0 (3 4))')
    self.failUnlessEqual(c.__repr__(), 'XXVert(1.0000 1.0000 1.0000 (3 4))')


if __name__ == '__main__':
  verbose = False
  unittest.main()
  pass
