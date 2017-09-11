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
// File    : SIERPINSKY_Gen_i.cxx
// Author  : Vadim SANDLER (OCN)
// Created : 13/07/05
///////////////////////////////////////////////////////////

#include "SIERPINSKY_Gen_i.hxx"
#include "SIERPINSKY_version.h"
#include <MED_Factory.hxx>
#ifdef WITH_LIBGD
#include <gd.h>
#endif // WITH_LIBGD

/*!
 * Engine factory
 */
extern "C"
{ 
  SIERPINSKYENGINE_EXPORT
  PortableServer::ObjectId * SIERPINSKYEngine_factory( CORBA::ORB_ptr            orb,
                                                       PortableServer::POA_ptr   poa, 
                                                       PortableServer::ObjectId* contId,
                                                       const char*               instanceName,
                                                       const char*               interfaceName )
{
  SIERPINSKY_Gen_i* anEngine = new SIERPINSKY_Gen_i( orb, poa, contId, instanceName, interfaceName );
  return anEngine->getId() ;
}
}

/*!
 * Default constructor
 */
SIERPINSKY_Gen_i::SIERPINSKY_Gen_i()
{
}

/*!
 * Constructor
 */
SIERPINSKY_Gen_i::SIERPINSKY_Gen_i( CORBA::ORB_ptr            orb,
                                    PortableServer::POA_ptr   poa,
                                    PortableServer::ObjectId* contId, 
                                    const char*               instanceName, 
                                    const char*               interfaceName ) 
: Engines_Component_i( orb, poa, contId, instanceName, interfaceName ) 
{
  // activate servant
  _thisObj = this;
  _id = poa->activate_object(_thisObj);
  // set default values
  Reset();
}

/*!
 * Destructor
 */
SIERPINSKY_Gen_i::~SIERPINSKY_Gen_i()
{
  myPoints.clear();
}
  
/*!
 * Initializes engine with three reference points
 */
void SIERPINSKY_Gen_i::Init( CORBA::Double theX1, CORBA::Double theY1, 
                             CORBA::Double theX2, CORBA::Double theY2, 
                             CORBA::Double theX3, CORBA::Double theY3 )
{
  myRefPoints[0] = MyPoint( theX1, theY1 );
  myRefPoints[1] = MyPoint( theX2, theY2 );
  myRefPoints[2] = MyPoint( theX3, theY3 );
  myPoints.clear();
}
  
/*!
 * Initializes engine with three default reference points: (0.5, 1), (0, 0), (1, 0)
 */
void SIERPINSKY_Gen_i::Reset()
{
  myRefPoints[0] = MyPoint( 0.5, 1.0 );
  myRefPoints[1] = MyPoint( 0.0, 0.0 );
  myRefPoints[2] = MyPoint( 1.0, 0.0 );
  myPoints.clear();
}
  
/*!
 * Generates next iteration point
 */
void SIERPINSKY_Gen_i::NextPoint( CORBA::Double  theX,     CORBA::Double  theY, 
                                  CORBA::Long    theIter, 
                                  CORBA::Double& theNextX, CORBA::Double& theNextY )
{
  double x = theIter < 1 || theIter > 3 ? theX : ( theX + myRefPoints[ theIter-1 ].myX ) / 2;
  double y = theIter < 1 || theIter > 3 ? theY : ( theY + myRefPoints[ theIter-1 ].myY ) / 2;
  myPoints.push_back( MyPoint( x, y ) );
  theNextX = x;
  theNextY = y;
}
  
/*!
 * Exports data to the JPEG image
 */
CORBA::Boolean SIERPINSKY_Gen_i::ExportToJPEG( const char* theFileName, CORBA::Long theSize )
{
#ifdef WITH_LIBGD
  if ( theSize <= 0 ) return false;

  // open file
  FILE* fileDescriptor = fopen( theFileName, "wb" );
  if ( !fileDescriptor ) return false;

  // create an image
  gdImagePtr image = gdImageCreate( theSize, theSize );
  int white = gdImageColorAllocate( image, 255, 255, 255 );
  int black = gdImageColorAllocate( image,   0,   0,   0 );

  gdImageRectangle( image, 0, 0, theSize-1, theSize-1, white );

  // draw points
  std::list<MyPoint>::const_iterator iter;
  for ( iter = myPoints.begin(); iter != myPoints.end(); ++iter ) {
    gdImageSetPixel( image, (int)( (*iter).myX * theSize ), theSize - (int)( (*iter).myY * theSize ), black );
  }

  // export jpeg image
  gdImageJpeg( image, fileDescriptor, 95 );
  fclose( fileDescriptor );
  gdImageDestroy( image );
  
  return true;
#else // WITH_LIBGD
  printf("Warning: ExportToJPEG() is not supported (libgd is required)!");
  return false;
#endif // WITH_LIBGD
}
  
/*!
 * Exports data to the MED file
 */
CORBA::Boolean SIERPINSKY_Gen_i::ExportToMED( const char* theFileName, CORBA::Double theSize )
{
  MED::TErr anError;
  
  // if file already exists - remove it (MED cannot overwrite files correctly)
  FILE* aFile = fopen( theFileName, "rb" );
  if ( aFile ) {
    fclose( aFile );
    if ( remove( theFileName ) ) return false; // can't remove file
  }
  
  // create MED 2.2 file
  MED::PWrapper aMed = MED::CrWrapper( theFileName, MED::eV2_2 );

  // create 2D mesh
  MED::PMeshInfo aMesh = aMed->CrMeshInfo( 2, 2, "Sierpinsky" );
  aMed->SetMeshInfo( aMesh, &anError );
  if ( anError < 0 ) return false;

  // create nodes
  MED::TFloatVector nodes;
  MED::TIntVector   connect;
  std::list<MyPoint>::const_iterator iter;
  int ind = 1;
  for ( iter = myPoints.begin(); iter != myPoints.end(); ++iter ) {
    nodes.push_back( (*iter).myX * theSize );
    nodes.push_back( (*iter).myY * theSize );
    connect.push_back( ind++ );
  }
  MED::PNodeInfo aNodes = aMed->CrNodeInfo( aMesh, nodes, 
					    MED::eFULL_INTERLACE, MED::eCART, 
					    MED::TStringVector(2),
					    MED::TStringVector(2), 
					    MED::TIntVector( myPoints.size() ),
					    MED::TIntVector() );
  aMed->SetNodeInfo( aNodes, &anError );
  if ( anError < 0 ) return false;

  MED::PCellInfo aCells = aMed->CrCellInfo( aMesh, MED::eMAILLE, MED::ePOINT1, 
					    connect, MED::eNOD, 
					    MED::TIntVector( myPoints.size() ), 
					    MED::TIntVector( myPoints.size() ) );
  aMed->SetCellInfo( aCells, &anError );
  if ( anError < 0 ) return false;

  return true;
}

// Version information
char* SIERPINSKY_Gen_i::getVersion()
{
#if SIERPINSKY_DEVELOPMENT
  return CORBA::string_dup(SIERPINSKY_VERSION_STR"dev");
#else
  return CORBA::string_dup(SIERPINSKY_VERSION_STR);
#endif
}
