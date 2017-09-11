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

#include "ATOMSOLV.hxx"
#include "ATOMSOLV_version.h"
#include "utilities.h"

#include <string>

using namespace std;


ATOMSOLV::ATOMSOLV(CORBA::ORB_ptr orb,
	PortableServer::POA_ptr poa,
	PortableServer::ObjectId * contId, 
	const char *instanceName, 
	const char *interfaceName) :
  Engines_Component_i(orb, poa, contId, instanceName, interfaceName)
{
  MESSAGE("activate object");
  _thisObj = this ;
  _id = _poa->activate_object(_thisObj);
}

ATOMSOLV::~ATOMSOLV()
{
}

CORBA::Boolean ATOMSOLV::setData( CORBA::Long studyID, const ATOMSOLV_ORB::TMoleculeList& theData )
{
  const int n = theData.length();
  ATOMSOLV_ORB::TMoleculeList_var data = new ATOMSOLV_ORB::TMoleculeList();
  data->length( n );
  for ( int i = 0; i < n; i++ )
    data[ i ] = theData[ i ];

  //  printf( " -- ATOMSOLV::setData( %d ) length = %d\n", studyID, data->length() );
  myData[ studyID ] = data._retn();

  return (bool)n;
}

CORBA::Boolean ATOMSOLV::getData( CORBA::Long studyID, ATOMSOLV_ORB::TMoleculeList_out outData )
{
  if ( myData.find( studyID ) != myData.end() ) {
    ATOMSOLV_ORB::TMoleculeList* data = myData[ studyID ];

    //    printf( " -- ATOMSOLV::getData( %d ) length = %d\n", studyID, data->length() );
    const int n = data->length();
    outData = new ATOMSOLV_ORB::TMoleculeList();
    outData->length( n );

    for ( int i = 0; i < n; i++ )
      (*outData)[ i ] = (*data)[ i ];
    
    return true;
  }
  return false;
}

CORBA::Boolean ATOMSOLV::processData( CORBA::Long studyID )
{
  if ( myData.find( studyID ) != myData.end() ) {
    ATOMSOLV_ORB::TMoleculeList* data = myData[ studyID ];
    for ( int i = 0, n = data->length(); i < n; i++ )
      (*data)[i].temperature = rand();
  }
  return true;
}

char* ATOMSOLV::getVersion()
{
#if defined(ATOMSOLV_DEVELOPMENT)
  return CORBA::string_dup(ATOMSOLV_VERSION_STR"dev");
#else
  return CORBA::string_dup(ATOMSOLV_VERSION_STR);
#endif
}

extern "C"
{
  PortableServer::ObjectId * ATOMSOLVEngine_factory(
			       CORBA::ORB_ptr orb,
			       PortableServer::POA_ptr poa, 
			       PortableServer::ObjectId * contId,
			       const char *instanceName, 
		       	       const char *interfaceName)
  {
    MESSAGE("PortableServer::ObjectId * ATOMSOLVEngine_factory()");
    SCRUTE(interfaceName);
    ATOMSOLV * myATOMSOLV 
      = new ATOMSOLV(orb, poa, contId, instanceName, interfaceName);
    return myATOMSOLV->getId() ;
  }
}
