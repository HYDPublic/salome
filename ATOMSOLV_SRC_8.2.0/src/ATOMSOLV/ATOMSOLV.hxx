// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

#ifndef _ATOMSOLV_HXX_
#define _ATOMSOLV_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(ATOMSOLV)
#include CORBA_SERVER_HEADER(ATOMGEN)
#include "SALOME_Component_i.hxx"

#include <map>

#if defined WIN32
#  if defined ATOMSOLV_EXPORTS || defined ATOMSOLVEngine_EXPORTS
#    define ATOMSOLV_EXPORT __declspec( dllexport )
#  else
#    define ATOMSOLV_EXPORT __declspec( dllimport )
#  endif
#else
#  define ATOMSOLV_EXPORT
#endif


class ATOMSOLV_EXPORT ATOMSOLV:
  public POA_ATOMSOLV_ORB::ATOMSOLV_Gen,
  public Engines_Component_i 
{

public:
  ATOMSOLV(CORBA::ORB_ptr orb,
	   PortableServer::POA_ptr poa,
	   PortableServer::ObjectId * contId, 
	   const char *instanceName, 
	   const char *interfaceName);
  virtual ~ATOMSOLV();

  CORBA::Boolean setData( CORBA::Long studyID, const ATOMSOLV_ORB::TMoleculeList& theData );
  CORBA::Boolean getData( CORBA::Long studyID, ATOMSOLV_ORB::TMoleculeList_out outData );
  CORBA::Boolean processData( CORBA::Long studyID );
  virtual char*  getVersion();

private:
  std::map<long, ATOMSOLV_ORB::TMoleculeList*> myData;
};

extern "C"
 ATOMSOLV_EXPORT
PortableServer::ObjectId * ATOMSOLVEngine_factory(CORBA::ORB_ptr orb,
						  PortableServer::POA_ptr poa,
						  PortableServer::ObjectId * contId,
						  const char *instanceName,
						  const char *interfaceName);

#endif
