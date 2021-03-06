// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

//  SALOME Container : implementation of container and engine for Kernel
//  File   : SALOME_Container.cxx
//  Author : Paul RASCLE, EDF - MARC TAJCHMAN, CEA
//  Module : SALOME
//  $Header$
//
#include <iostream>
#include <string>
#include <stdio.h>

#ifndef WIN32
#include <unistd.h>
#else
#include <process.h>
#endif
#include "SALOME_Container_i.hxx"
#include "utilities.h"
#include "Utils_ORB_INIT.hxx"
#include "Utils_SINGLETON.hxx"
#include "SALOMETraceCollector.hxx"
#include "OpUtil.hxx"

#ifdef CHECKTIME
#include <Utils_Timer.hxx>
#endif

#ifdef HAVE_MPI2
#include <mpi.h>
#endif

//CCRT
#include <sstream>
#include <time.h>
#ifndef WIN32
#include <sys/time.h>
#include <sys/stat.h>
#endif
//end-CCRT

#include "Container_init_python.hxx"

using namespace std;

extern "C" void HandleServerSideSignals(CORBA::ORB_ptr theORB);

int main(int argc, char* argv[])
{
#ifdef HAVE_MPI2
  MPI_Init(&argc,&argv);
#endif

  // Initialise the ORB.
  //SRN: BugID: IPAL9541, it's necessary to set a size of one message to be at least 100Mb
  //CORBA::ORB_var orb = CORBA::ORB_init( argc , argv ) ;
  ORB_INIT &init = *SINGLETON_<ORB_INIT>::Instance() ;
  ASSERT(SINGLETON_<ORB_INIT>::IsAlreadyExisting());
  CORBA::ORB_var orb = init(0 , 0 ) ;
	  
  //SALOMETraceCollector *myThreadTrace = SALOMETraceCollector::instance(orb);
  INFOS_COMPILATION;
  BEGIN_OF(argv[0]);

  ASSERT(argc > 1);
  SCRUTE(argv[1]);
  bool isSupervContainer = false;
  if (strcmp(argv[1],"SuperVisionContainer") == 0) isSupervContainer = true;

  if (!isSupervContainer)
    {
      int _argc = 1;
      char* _argv[] = {""};
      KERNEL_PYTHON::init_python(argc,argv);
    }
  else
    {
      Py_Initialize() ;
      PySys_SetArgv( argc , argv ) ;
    }
    
  char *containerName = "";
  if(argc > 1)
    {
      containerName = argv[1] ;
    }

  try
    {  
      CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
      ASSERT(!CORBA::is_nil(obj));
      PortableServer::POA_var root_poa = PortableServer::POA::_narrow(obj);

      PortableServer::POAManager_var pman = root_poa->the_POAManager();

      // add new container to the kill list
#ifndef WNT
      char aCommand[40];
      sprintf(aCommand, "addToKillList.py %d SALOME_Container", getpid());
      system(aCommand);
#endif
      
      Engines_Container_i * myContainer 
	= new Engines_Container_i(orb, root_poa, containerName , argc , argv );
      
      pman->activate();
      
#ifdef CHECKTIME
      Utils_Timer timer;
      timer.Start();
      timer.Stop();
      MESSAGE("SALOME_Registry_Server.cxx - orb->run()");
      timer.ShowAbsolute();
#endif
      
      HandleServerSideSignals(orb);
      
    }
  catch(CORBA::SystemException&)
    {
      INFOS("Caught CORBA::SystemException.");
    }
  catch(PortableServer::POA::ServantAlreadyActive&)
    {
      INFOS("Caught CORBA::ServantAlreadyActiveException");
    }
  catch(CORBA::Exception&)
    {
      INFOS("Caught CORBA::Exception.");
    }
  catch(std::exception& exc)
    {
      INFOS("Caught std::exception - "<<exc.what()); 
    }
  catch(...)
    {
      INFOS("Caught unknown exception.");
    }

#ifdef HAVE_MPI2
  MPI_Finalize();
#endif

  END_OF(argv[0]);
  // delete myThreadTrace;
  return 0 ;
}

