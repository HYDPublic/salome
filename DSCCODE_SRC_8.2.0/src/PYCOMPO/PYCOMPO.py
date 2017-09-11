import sys,os
import traceback
import DSCCODE__POA
import calcium
import dsccalcium
import SALOME
import time
import LifeCycleCORBA

try:
  import numpy
except:
  numpy=None

class PYCOMPO(DSCCODE__POA.PYCOMPO,dsccalcium.PyDSCComponent):
  """
     To be identified as a SALOME component this Python class
     must have the same name as the component, inherit omniorb
     class DSCCODE__POA.PYCOMPO and DSC class dsccalcium.PyDSCComponent
     that implements DSC API.
  """
  def __init__ ( self, orb, poa, contID, containerName, instanceName, interfaceName ):
    print "PYCOMPO.__init__: ", containerName, ';', instanceName,interfaceName
    dsccalcium.PyDSCComponent.__init__(self, orb, poa,contID,containerName,instanceName,interfaceName)
    self.lcc= LifeCycleCORBA.LifeCycleCORBA(self._orb)

  def init_service(self,service):
    print "init_service:",service
    calcium.create_calcium_port(self.proxy,"tabin","CALCIUM_double","IN","I");
    calcium.create_calcium_port(self.proxy,"tabout","CALCIUM_double","OUT","I");
    return True

  def run( self):
    try:
      self.beginService("PYCOMPO.run")
      ndim=10

      force=calcium.doubleArray(ndim)
      for i in xrange(ndim):
        force[i]=i
      err=calcium.cp_edb(self.proxy,calcium.CP_ITERATION ,0.,1,"tabout",ndim,force)

      if numpy:
        val=numpy.zeros(ndim,'d')
      else:
        val=calcium.doubleArray(ndim)
      err,t,i,nval=calcium.cp_ldb(self.proxy,calcium.CP_ITERATION,0.,0.,1,"tabin",ndim,val)
      print err,t,i,nval
      if numpy:
        print val
      else:
        for i in xrange(ndim):
          print val[i],
        print

      self.endService("PYCOMPO.run")
      sys.stdout.flush()
    except:
      sys.stdout.flush()
      self.endService("PYCOMPO.run")
      exc_typ,exc_val,exc_fr=sys.exc_info()
      l=traceback.format_exception(exc_typ,exc_val,exc_fr)
      raise SALOME.SALOME_Exception(SALOME.ExceptionStruct(SALOME.BAD_PARAM,"".join(l),"PYCOMPO.py",0))

  def gnuplot( self):
    print "PYCOMPO.gnuplot"
    try:
      import Gnuplot
      g = Gnuplot.Gnuplot(debug=1,persist=1)
      x = numpy.arange(4)
      y = numpy.arange(6)
      g.xlabel('x')
      g.ylabel('y')
      g.zlabel('temp')
      g('set parametric')
      g('set data style lines')
      g('set hidden')
      g('set view 60, 240')
      g('set pm3d at bs')
      g('set palette')
      ndim=24

      force=calcium.floatArray(ndim)
      for i in xrange(ndim):
        force[i]=i
      err=calcium.cp_ere(self.proxy,calcium.CP_ITERATION ,0.,1,"tabout",ndim,force)

      if numpy:
        #val=numpy.zeros(ndim,'f')
        val=numpy.zeros((6,4),'f')
      else:
        val=calcium.floatArray(ndim)
      for iter in xrange(20):
        err,t,i,nval=calcium.cp_lre(self.proxy,calcium.CP_ITERATION,0.,0.,iter,"tabin",ndim,val)
        print err,t,i,nval
        if numpy:
          print val
        else:
          for i in xrange(ndim):
            print val[i],
          print
        g.title('Surface plot (temperature) iter=%d' % iter)
        g.splot(Gnuplot.GridData(val,y,x, binary=0))
        #g('pause mouse')
        time.sleep(0.1)

      print "End of PYCOMPO.gnuplot"
      sys.stdout.flush()
    except:
      sys.stdout.flush()
      exc_typ,exc_val,exc_fr=sys.exc_info()
      l=traceback.format_exception(exc_typ,exc_val,exc_fr)
      raise SALOME.SALOME_Exception(SALOME.ExceptionStruct(SALOME.BAD_PARAM,"".join(l),"PYCOMPO.py",0))

  def visu( self):
    print "PYCOMPO.visu"
    try:
      import libMedCorba_Swig
      import VISU
      import libMEDMEM_Swig
      #from libMEDMEM_Swig import *
      visu_comp = self.lcc.FindOrLoadComponent("FactoryServer", "VISU")
      myViewManager = visu_comp.GetViewManager()
      myView = myViewManager.Create3DView()


      nx=4
      ny=6
      ndim=24
      spaceDimension=2
      numberOfNodes=(nx+1)*(ny+1)
      ncells=nx*ny
      coordinates=[]
      dx=1.
      dy=1.
      self.fields=[]
      self.corbafields=[]

      for j in xrange(ny+1):
        y=j*dy
        for i in xrange(nx+1):
          x=i*dx
          coordinates.append(x)
          coordinates.append(y)
      myMeshing = libMEDMEM_Swig.MESHING()
      self.myMeshing=myMeshing
      myMeshing.setName("mymesh")
      myMeshing.setMeshDimension(2)
      myMeshing.setCoordinates(spaceDimension,numberOfNodes,coordinates,"CARTESIAN",libMEDMEM_Swig.MED_FULL_INTERLACE)
      myMeshing.setCoordinateName("X",0)
      myMeshing.setCoordinateUnit("cm",0)
      myMeshing.setCoordinateName("Y",1)
      myMeshing.setCoordinateUnit("cm",1)
      myMeshing.setNumberOfTypes(1,libMEDMEM_Swig.MED_CELL)
      myMeshing.setTypes([libMEDMEM_Swig.MED_QUAD4],libMEDMEM_Swig.MED_CELL)
      myMeshing.setNumberOfElements([ncells],libMEDMEM_Swig.MED_CELL)
      connectivityQuad = []
      for j in xrange(ny):
        for i in xrange(nx):
          n1=i+1+(j)*(nx+1)
          n2=i+2+(j)*(nx+1)
          n3=i+2+(j+1)*(nx+1)
          n4=i+1+(j+1)*(nx+1)
          connectivityQuad.append(n1)
          connectivityQuad.append(n2)
          connectivityQuad.append(n3)
          connectivityQuad.append(n4)

      myMeshing.setConnectivity(connectivityQuad,libMEDMEM_Swig.MED_CELL,libMEDMEM_Swig.MED_QUAD4)
      supportOnCells = libMEDMEM_Swig.SUPPORT(myMeshing,"On_All_Cells",libMEDMEM_Swig.MED_CELL)
      self.supportOnCells=supportOnCells
      numberOfCells = supportOnCells.getNumberOfElements(libMEDMEM_Swig.MED_ALL_ELEMENTS)

      filename="mytest.med"
      idMed = myMeshing.addDriver(libMEDMEM_Swig.MED_DRIVER,filename,myMeshing.getName())
      myMeshing.write(idMed)

      corbasupport=libMedCorba_Swig.createCorbaSupport(supportOnCells)
      #avoid the object be destroyed on method exit (needed by VISU component)
      self.corbasupport=corbasupport

      force=calcium.floatArray(ndim)
      for i in xrange(ndim):
        force[i]=i
      err=calcium.cp_ere(self.proxy,calcium.CP_ITERATION ,0.,1,"tabout",ndim,force)

      if numpy:
        val=numpy.zeros(ndim,'f')
      else:
        val=calcium.floatArray(ndim)

      #start loop
      for iter in xrange(20):
        err,t,i,nval=calcium.cp_lre(self.proxy,calcium.CP_ITERATION,0.,0.,iter,"tabin",ndim,val)
        print err,t,i,nval
        if numpy:
          print val
        else:
          for i in xrange(ndim):
            print val[i],
          print

        fieldDoubleScalarOnCells = libMEDMEM_Swig.FIELDDOUBLE(supportOnCells,1)
        self.fields.append(fieldDoubleScalarOnCells)
        fieldDoubleScalarOnCells.setName("temperature")
        fieldDoubleScalarOnCells.setIterationNumber(1)
        fieldDoubleScalarOnCells.setOrderNumber(-1)
        fieldDoubleScalarOnCells.setTime(0.0)

        fieldDoubleScalarOnCells.setComponentName(1,"Temp")
        fieldDoubleScalarOnCells.setComponentDescription(1,"temperature")
        fieldDoubleScalarOnCells.setMEDComponentUnit(1,"K")

        t=val.tolist()
        print "temperature",t

        fieldDoubleScalarOnCells.setValue(t)
        print fieldDoubleScalarOnCells.getValue()
        idMed = fieldDoubleScalarOnCells.addDriver(libMEDMEM_Swig.MED_DRIVER,filename,fieldDoubleScalarOnCells.getName())
        fieldDoubleScalarOnCells.write(idMed)

        corbafield = libMedCorba_Swig.createCorbaFieldDouble(corbasupport,fieldDoubleScalarOnCells)
        self.corbafields.append(corbafield)
        result=visu_comp.ImportMedField(corbafield)

        scmap = visu_comp.Plot3DOnField(result,"mymesh",VISU.CELL,"temperature",1)
        scmap.SetRange(100.,1600.)
        scmap.SetScaleFactor(0.002)
        myView.DisplayOnly(scmap)
        myView.FitAll()

      print "End of PYCOMPO.visu"
      sys.stdout.flush()
    except:
      sys.stdout.flush()
      exc_typ,exc_val,exc_fr=sys.exc_info()
      l=traceback.format_exception(exc_typ,exc_val,exc_fr)
      raise SALOME.SALOME_Exception(SALOME.ExceptionStruct(SALOME.BAD_PARAM,"".join(l),"PYCOMPO.py",0))
