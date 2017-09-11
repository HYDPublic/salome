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

//  SuperVisionTest SIGNALSComponent : example of component that devides two numbers
//  File   : SIGNALSComponentEngine.cxx
//  Author : MARC TAJCHMAN, CEA
//  Module : SuperVisionTest
//
#include <stdio.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <fstream>
#include <sstream>
#include <string>

//#include "utilities.h"
#include "SIGNALSComponent.hxx"

using namespace std;

SIGNALSComponentEngine::SIGNALSComponentEngine( CORBA::ORB_ptr orb,
				    PortableServer::POA_ptr poa,
				    PortableServer::ObjectId * contId, 
				    const char *instanceName,
                                    const char *interfaceName) :
  Engines_Component_i(orb, poa, contId, instanceName, interfaceName,1,true)
{
  MESSAGE("SIGNALSComponentEngine::SIGNALSComponentEngine activate object instanceName("
          << instanceName << ") interfaceName(" << interfaceName << ")" )
  _thisObj = this ;
  _id = _poa->activate_object(_thisObj);
  NULLPTR = NULL ;
}

SIGNALSComponentEngine::SIGNALSComponentEngine() :
  Engines_Component_i()
{
  NULLPTR = NULL ;
}

SIGNALSComponentEngine::~SIGNALSComponentEngine()
{
}

CORBA::Long SIGNALSComponentEngine::SIGSEGVfunc() {
  beginService( " SIGNALSComponentEngine::SIGSEGVfunc" );
  int S = 10 ;
  while ( S ) {
    S = sleep( S ) ;
  }
  long nullptr = *NULLPTR ;
  endService( " SIGNALSComponentEngine::SIGSEGVfunc"  );
  return nullptr ;
}

CORBA::Long SIGNALSComponentEngine::SIGFPEfunc( CORBA::Long a , CORBA::Long b ) {
  beginService( " SIGNALSComponentEngine::SIGFPEfunc" );
  int S = 10 ;
  while ( S ) {
    S = sleep( S ) ;
  }
  long c = a/b ;
  endService( " SIGNALSComponentEngine::SIGFPEfunc"  );
  return c ;
}

extern "C"
{
  PortableServer::ObjectId * SIGNALSComponentEngine_factory
     (CORBA::ORB_ptr orb,
      PortableServer::POA_ptr poa, 
      PortableServer::ObjectId * contId,
      const char *instanceName,
      const char *interfaceName)
  {
    MESSAGE("SIGNALSComponentEngine_factory SIGNALSComponentEngine ("
            << instanceName << "," << interfaceName << ")");
    SIGNALSComponentEngine * mySIGNALSComponent 
      = new SIGNALSComponentEngine(orb, poa, contId, instanceName, interfaceName);
    return mySIGNALSComponent->getId() ;
  }
}


