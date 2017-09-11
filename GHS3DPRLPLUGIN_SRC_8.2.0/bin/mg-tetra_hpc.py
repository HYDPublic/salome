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
run mg_tetra_hpc.exe or mg_tetra_hpc_mpi.exe

example linux usage:
- simple run:
  ./mg-tetra_hpc.py --help
  ./mg-tetra_hpc.py -n 3 --in /tmp/GHS3DPRL.mesh --out /tmp/GHS3DPRL_out.mesh --gradation 1.05 --min_size 0.001 --max_size 1.1 --multithread no > /tmp/tetrahpc.log
"""

import os
import sys
import time
import platform
import argparse as AP
import pprint as PP #pretty print
import subprocess as SP #Popen
import multiprocessing as MP #cpu_count

verbose = False

OK = "ok"
KO = "KO"
OKSYS = 0 #for linux
KOSYS = 1 #for linux

NB_PROCS = MP.cpu_count()   #current cpu number of proc
NAME_OS = platform.system() #'Linux' or 'Windows'
 

##########################################################################
# utilities
##########################################################################

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

##########################################################################
def getDirAndName(datafile):
  path, namefile = os.path.split(os.path.realpath(datafile))
  rootpath = os.getcwd()
  return (path, rootpath, namefile)


##########################################################################
class ArgRange(object):
  """
  ArgumentParser utility for range float or in in arguments
  """
  def __init__(self, start, end):
    self.start = start
    self.end = end

  def __eq__(self, other):
    return self.start <= other <= self.end

  def __repr__(self):
    return "[%s,%s]" % (self.start, self.end)


##########################################################################
def exec_command(cmd, verbose=False):
  """Exec ONE command with popen"""

  time.sleep(3) #wait for (MPI) flush files
  if verbose: print("launch process:\n  %s" % cmd)
  try:
    pipe = SP.Popen(cmd, shell=True, stdout=SP.PIPE, stderr=SP.PIPE )
  except Exception as e:
    result = KO + " ERROR: we have a problem popen on: %s" % PP.pformat(cmd)
    return result
  
  (out, error) = pipe.communicate()
  pipe.wait()
  
  print(out)
  print(error)
 
  result = OK + " INFO: seems final ok for: %s" % PP.pformat(cmd)
  time.sleep(3) #wait for (MPI) flush files
  return result



##########################################################################
def force_DISTENE_LICENSE_FILE():
  """
  conditionaly overriding/set environ variable DISTENE_LICENSE_FILE,
  from/if existing FORCE_DISTENE_LICENSE_FILE environ variable
  (for test new version MeshGems etc...)
  """
  """
  #example:
  DISTENE_LICENSE_FILE=Use global envvar: DLIM8VAR
  DLIM8VAR=dlim8 1:1:29030@132.166.151.49/84c419b8::87af196ab2a936ab31363624539bff8096fbe1f3c83028c8f6b399b0a904ef85
  overriden by
  FORCE_DISTENE_LICENSE_FILE=/export/home/wambeke/essai_meshgems2.3/dlim8.key
  """
  force = os.getenv("FORCE_DISTENE_LICENSE_FILE")
  if force != None:
    os.environ["DISTENE_LICENSE_FILE"] = force
    os.environ["DLIM8VAR"] = "NOTHING"
    """
    #export PATH=/export/home/prerequisites_SALOME_780_LAURENT/openmpi-1.8.4/FROM_nothing/bin:$PATH
    #do not work prefer set before salome launch
    OPENMPI_INSTALL_DIR = "/export/home/prerequisites_SALOME_780_LAURENT/openmpi-1.8.4/FROM_nothing/bin"
    sys.path.insert(0, OPENMPI_INSTALL_DIR) 
    #INSTALL_DIR = /export/home/prerequisites_SALOME_780_LAURENT/openmpi-1.8.4/FROM_nothing
    for i in sys.path[0:10]: print "PATH",i
    """




##########################################################################
def launchMultithread(args):
  if verbose: print("INFO: launchMultithread for %s" % NAME_OS)
  
  if NAME_OS == 'Linux':
    #--out is ONE file: basename_tetra_hpc.mesh
    outputMulti = os.path.splitext(args.inputFile)[0] + "_tetra_hpc.mesh" #only one file if Multithread
    outputs = os.path.splitext(args.outputFiles)[0]
    outputMultiAsMpi = os.path.splitext(args.outputFiles)[0] + ".000001.mesh" #create one output file named as only one from mpi 
    cmd = "mg-tetra_hpc.exe --max_number_of_threads %i --in %s --gradation %s --max_size %s --min_size %s; cp %s %s; ls -alt %s*; " % \
          (args.number, args.inputFile, args.gradation, args.max_size, args.min_size, outputMulti, outputMultiAsMpi, outputs)
  else:
    return KO +  " ERROR: unknown operating system: %s" % NAME_OS
  
  result = exec_command(cmd, verbose=True)
  return result



##########################################################################
def launchMpi(args):
  if verbose: print("INFO: launchMpi for %s" % NAME_OS)
  
  if NAME_OS == 'Linux':  
    cmd  = ""
    
    """ compile libmeshgems_mpi.so: no needs
    COMPILDIR=os.getenv("MESHGEMSHOME") + "/stubs"
    TARGETDIR=os.getenv("MESHGEMSHOME") + "/lib/Linux_64"
    cmd  = "which mg-tetra_hpc_mpi.exe; which mpicc; rm /tmp/GHS3DPRL_out*; "
    cmd += "cd %s; mpicc meshgems_mpi.c -DMESHGEMS_LINUX_BUILD -I../include -shared -fPIC -o %s/libmeshgems_mpi.so; " % (COMPILDIR, TARGETDIR)
    """

    outputs = os.path.splitext(args.outputFiles)[0]
    cmd += "mpirun -n %i mg-tetra_hpc_mpi.exe --in %s --out %s --gradation %s --max_size %s --min_size %s; ls -alt %s*; " % \
          (args.number, args.inputFile, args.outputFiles, args.gradation, args.max_size, args.min_size, outputs)
  else:
    return KO +  " ERROR: unknown operating system: %s" % NAME_OS
  
  result = exec_command(cmd, verbose=True)
  return result


##########################################################################
# main
##########################################################################

if __name__ == '__main__':
  parser = AP.ArgumentParser(description='launch tetra_hpc.exe or tetra_hpc_mpi.exe mesh computation', argument_default=None)
  #./mg-tetra_hpc.py -n 3 --in=/tmp/GHS3DPRL.mesh --out=/tmp/GHS3DPRL_out.mesh --gradation=1.05 --min_size=0.001 --max_size=1.1 --multithread no > /tmp/tetrahpc.log

  parser.add_argument(
    '-v', '--verbose', 
    help='set verbose, for debug',
    action='store_true',
  )
  parser.add_argument(
    '-n', '--number', 
    help='if multithread: number of threads, else distributed: number of processes MPI',
    choices=[ArgRange(1, 999999)],
    type=int,
    metavar='integer >= 0',
    default=1,
  )
  parser.add_argument(
    '-m', '--multithread', 
    help='launch tetra_hpc multithread instead tetra_hpc distributed (MPI)',
    choices=["no", "yes"],
    default='no',
  )
  parser.add_argument(
    '-g', '--gradation', 
    help='size ratio adjacent cell, default 0 is 1.05',
    type=float,
    choices=[ArgRange(0.0, 3.0)],
    metavar='float in [0.,3]',
    default='0'
  )
  parser.add_argument(
    '-si', '--min_size', 
    help='min size cell, default 0 is no constraint',
    type=float,
    choices=[ArgRange(0.0, 9e99)],
    metavar='float >= 0',
    default='0'
  )
  parser.add_argument(
    '-sa', '--max_size', 
    help='max size cell, default 0 is no constraint',
    type=float,
    choices=[ArgRange(0.0, 9e99)],
    metavar='float >= 0',
    default='0'
  )
  parser.add_argument(
    '-i', '--inputFile', 
    help='input file name',
    #nargs='?',
    metavar='.../inputFile.mesh'
  )
  parser.add_argument(
    '-o', '--outputFiles', 
    help='output basename file(s) name',
    #nargs='?',
    metavar='.../outputFile.mesh'
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
  args is Namespace, may use it as global to store 
  parameters, data, used arrays and results and other...
  """
  args = parser.parse_args()
  
  verbose = args.verbose
  if verbose: print("INFO: args:\n%s" % PP.pformat(args.__dict__))

  if len(sys.argv) == 1: #no args as --help
    parser.print_help()
    sys.exit(KOSYS)

  if args.inputFile == None:
    print("\nERROR: Nothing to do: no input files\n\n%s\n" % PP.pformat(args))
    parser.print_help()
    sys.exit(KOSYS)

  if args.outputFiles == None:
    tmp, _ = os.path.splitext(args.inputFile)
    args.outputFiles = tmp + "_out.mesh"
    print("\nWARNING: Default ouput files: %s" % args.outputFiles)
   
  force_DISTENE_LICENSE_FILE()

  print("INFO: mg-tetra_hpc.py assume licence file set:\n  DLIM8VAR=%s\n  DISTENE_LICENSE_FILE=%s" % \
       (os.getenv("DLIM8VAR"), os.getenv("DISTENE_LICENSE_FILE")))
 
  if args.multithread == "yes": 
    result = launchMultithread(args)
  else:
    result = launchMpi(args)
  sys.exit(okToSys(result, verbose=True))
  
