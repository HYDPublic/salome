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
//  File   : SIGNALSComponent.hxx
//  Author : MARC TAJCHMAN, CEA
//  Module : SuperVisionTest
//  $Header$
//
#ifndef _SIGNALSCOMPONENT_IMPL_HXX_
#define _SIGNALSCOMPONENT_IMPL_HXX_

#include "SIGNALSComponent_defines.hxx"
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SIGNALSComponent)
#include CORBA_SERVER_HEADER(SALOME_Component)
#include "SALOME_Component_i.hxx"

class SIGNALSCOMPONENTENGINE_EXPORT SIGNALSComponent_Impl :  public POA_SuperVisionTest::SIGNALSComponent ,
                                                             public Engines_Component_i {
public:
  SIGNALSComponent_Impl() ;
  SIGNALSComponent_Impl( CORBA::ORB_ptr orb,
		      PortableServer::POA_ptr poa,
		      PortableServer::ObjectId * contId, 
		      const char *instanceName,
                      const char *interfaceName);

  virtual ~SIGNALSComponent_Impl();

  virtual char* getVersion();

  CORBA::Long SIGSEGVfunc() ;
  CORBA::Long SIGFPEfunc( CORBA::Long a , CORBA::Long b ) ;
  void MethodToKill() ;
  void WaitKill() ;

private:

  long * NULLPTR ;

};

extern "C"
  SIGNALSCOMPONENTENGINE_EXPORT
  PortableServer::ObjectId * SIGNALSComponentEngine_factory
                                   ( CORBA::ORB_ptr orb ,
                                     PortableServer::POA_ptr poa , 
                                     PortableServer::ObjectId * contId ,
                                     const char *instanceName ,
                                     const char *interfaceName ) ;

#endif
