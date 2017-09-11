// Copyright (C) 2005-2016  OPEN CASCADE
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

///////////////////////////////////////////////////////////
// File    : SIERPINSKY_Gen_i.hxx
// Author  : Vadim SANDLER (OCN)
// Created : 13/07/05
///////////////////////////////////////////////////////////
//
#ifndef __SIERPINSKY_GEN_I_HXX
#define __SIERPINSKY_GEN_I_HXX

#include "SALOME_Component_i.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(Sierpinsky)
#include CORBA_CLIENT_HEADER(SALOMEDS)

#include <list>

#ifdef WIN32
 #if defined SIERPINSKYENGINE_EXPORTS || defined SIERPINSKYEngine_EXPORTS
  #define SIERPINSKYENGINE_EXPORT __declspec( dllexport )
 #else
  #define SIERPINSKYENGINE_EXPORT __declspec( dllimport )
 #endif
#else
 #define SIERPINSKYENGINE_EXPORT
#endif

struct MyPoint
{
  double myX;
  double myY;
  MyPoint( const double x = 0., const double y = 0. ) : myX( x ), myY( y ) {}
};

class SIERPINSKYENGINE_EXPORT SIERPINSKY_Gen_i: public virtual POA_SIERPINSKY_ORB::SIERPINSKY,
                        public virtual Engines_Component_i
{
public:
  // Default constructor
  SIERPINSKY_Gen_i();
  // Constructor
  SIERPINSKY_Gen_i( CORBA::ORB_ptr            orb,
                    PortableServer::POA_ptr   poa,
                    PortableServer::ObjectId* contId, 
                    const char*               instanceName, 
                    const char*               interfaceName );
  // Destructor
  ~SIERPINSKY_Gen_i();
  
  // Version information
  virtual char* getVersion();

  // Initializes engine with three reference points
  void Init( CORBA::Double theX1, CORBA::Double theY1, 
             CORBA::Double theX2, CORBA::Double theY2, 
             CORBA::Double theX3, CORBA::Double theY3 );
  
  // Initializes engine with three default reference points: (0.5, 1), (0, 0), (1, 0)
  void Reset();
  
  // Generates next iteration point
  void NextPoint( CORBA::Double  theX,  CORBA::Double theY, 
                  CORBA::Long    theIter, 
                  CORBA::Double& theNextX, CORBA::Double& theNextY );
  
  // Exports data to the JPEG image
  CORBA::Boolean ExportToJPEG( const char* theFileName, CORBA::Long theSize );
  
  // Exports data to the MED file
  CORBA::Boolean ExportToMED( const char* theFileName, CORBA::Double theSize );

private:
  std::list<MyPoint> myPoints;
  MyPoint            myRefPoints[3];
};

#endif // __SIERPINSKY_GEN_I_HXX
