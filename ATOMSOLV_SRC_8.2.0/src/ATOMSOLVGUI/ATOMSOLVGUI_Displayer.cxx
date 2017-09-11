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

#include "ATOMSOLVGUI_Displayer.h"
#include "ATOMSOLVGUI.h"

#include <SUIT_Session.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Application.h>
#include <SUIT_Study.h>

#include <SVTK_ViewModel.h>
#include <SVTK_Prs.h>
#include <SALOME_Actor.h>

#include <SALOMEconfig.h>
#include CORBA_CLIENT_HEADER(ATOMSOLV)

#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActorCollection.h>

const float radius = 5.; // radius of a sphere that represents an atom in VTK viewer
const float quality_coefficient = .1; // for VTK viewer presentation: parameter that controls
// the quality of presentation of sphere (atom).  0 - very bad, 1. - very good (and slow).

// default representation mode, is taken from preferences
int ATOMSOLVGUI_Displayer::myRepresentation = SUIT_Session::session()->resourceMgr(
                                              )->integerValue( "ATOMSOLV", "Representation", 2 );
// default radius, is taken from preferences
double ATOMSOLVGUI_Displayer::myRadius = SUIT_Session::session()->resourceMgr(
                                         )->doubleValue( "ATOMSOLV", "Radius", 5. );

ATOMSOLVGUI_Displayer::ATOMSOLVGUI_Displayer()
{
}

ATOMSOLVGUI_Displayer::~ATOMSOLVGUI_Displayer()
{
}

bool ATOMSOLVGUI_Displayer::canBeDisplayed( const QString& entry, const QString& viewer_type ) const
{
  QStringList es = entry.split( "_" );
  bool result = ( es.count() == 3 && es[ 0 ] == "ATOMSOLVGUI" && viewer_type == SVTK_Viewer::Type() ); 
  //  printf ( " canBeDisplayed : entry = %s, count = %d, res = %d \n", entry.latin1(), es.count(), result );
  return result; // entry of an atom for sure
}

ATOMGEN_ORB::Atom_ptr getAtom( const QString& entry, const int studyID, double& temprature )
{
  QStringList es = entry.split( "_" );

  bool isAtom = (es.count() == 3 && es[ 0 ] == "ATOMSOLVGUI");
  if (!isAtom)
    return ATOMGEN_ORB::Atom::_nil();

  const int molID = es[1].toInt();
  const int atomID = es[2].toInt();

  ATOMSOLV_ORB::ATOMSOLV_Gen_var engine = ATOMSOLVGUI::GetATOMSOLVGen();
  ATOMSOLV_ORB::TMoleculeList_var molecules;
  if ( engine->getData( studyID, molecules ) && molID >= 0 && molID < molecules->length() ) {
    ATOMSOLV_ORB::TMolecule tmol = molecules[ molID ];
    ATOMGEN_ORB::Molecule_var mol = tmol.molecule;
    if ( atomID < mol->getNbAtoms() ) {
      temprature = tmol.temperature;
      return mol->getAtom( atomID );
    }
  }
  return ATOMGEN_ORB::Atom::_nil();
}

void setTemperature( SALOME_Actor* actor, double temperature )
{
  if ( actor ) {
    temperature = (((int)temperature) % 10 + 1.) / 10.;
    actor->SetColor( temperature, 0., 1. - temperature );
  }
}

int getStudyID()
{
  int studyID = -1;
  if ( SUIT_Application* app = SUIT_Session::session()->activeApplication() )
    if ( SUIT_Study* study = app->activeStudy() )
      studyID = study->id();
  return studyID;
}

SALOME_Prs* ATOMSOLVGUI_Displayer::buildPresentation( const QString& entry, SALOME_View* view )
{
  const int studyID = getStudyID();
  if ( studyID == -1 )
    return 0;

  SVTK_Prs* prs = dynamic_cast<SVTK_Prs*>( LightApp_Displayer::buildPresentation( entry, view ) );

  if ( !prs ) return 0;

  double temperature;
  ATOMGEN_ORB::Atom_var atom = getAtom( entry, studyID, temperature );
  
  if ( !CORBA::is_nil( atom ) ) {
    //    PRESENTATION FOR OCC VIEWER
    //    double x = atom->getX(), y = atom->getY(), z = atom->getZ();
    //    BRepPrimAPI_MakeSphere makeSphere( gp_Pnt( x, y, z ), radius );
    //    Handle(AIS_Shape) aisShape = new AIS_Shape( makeSphere.Shell() );
    //    if( !aisShape.IsNull() ) {
    //      aisShape->SetOwner( new SALOME_InteractiveObject( entry.latin1(), "ATOMSOLV" ) );
    //      return new SOCC_Prs( aisShape );
    //    }
    double center[ 3 ];
    center[ 0 ] = atom->getX();
    center[ 1 ] = atom->getY();
    center[ 2 ] = atom->getZ();
    
    vtkSphereSource* vtkObj = vtkSphereSource::New();
    vtkObj->SetRadius( radius );
    vtkObj->SetCenter( center );
    vtkObj->SetThetaResolution( (int)( vtkObj->GetEndTheta() * quality_coefficient ) );
    vtkObj->SetPhiResolution( (int)( vtkObj->GetEndPhi() * quality_coefficient ) );

    vtkPolyDataMapper* vtkMapper = vtkPolyDataMapper::New();
    vtkMapper->SetInputConnection( vtkObj->GetOutputPort() );
    
    vtkObj->Delete();
    
    SALOME_Actor* actor = SALOME_Actor::New();
    actor->SetMapper( vtkMapper );
    actor->setIO( new SALOME_InteractiveObject( entry.toLatin1(), "ATOMSOLV" ) );
    setTemperature( actor, temperature );

    actor->SetRepresentation( 2 ); // 2 == surface mode

    vtkMapper->Delete();
    
    prs->AddObject( actor );
  }

  return prs;
}

void ATOMSOLVGUI_Displayer::updateActor( SALOME_Actor* actor )
{
  const int studyID = getStudyID();
  if ( actor && actor->hasIO() && studyID >= 0 ) {
    double temperature;
    getAtom( actor->getIO()->getEntry(), studyID, temperature );
    setTemperature( actor, temperature );
    actor->Update();
  }
}

void ATOMSOLVGUI_Displayer::setDisplayMode( const QStringList& entries, const QString& mode )
{
  SALOME_View* view = GetActiveView();
  if ( view && dynamic_cast<SVTK_Viewer*>( view ) ) {
    for ( QStringList::ConstIterator it = entries.begin(); it != entries.end(); ++it ) {    
      if ( SALOME_Prs* prs = view->CreatePrs( it->toLatin1() ) ) {
	SVTK_Prs* vtkPrs = dynamic_cast<SVTK_Prs*>( prs );
	vtkActorCollection* lst = vtkPrs ? vtkPrs->GetObjects() : 0;
	if ( lst ) {
	  lst->InitTraversal();
	  while ( vtkActor* actor = lst->GetNextActor() )
	    if ( SALOME_Actor* salomeActor = SALOME_Actor::SafeDownCast( actor ) ) {
	      salomeActor->SetRepresentation( mode == "Points" ? 0 : mode == "Wireframe" ? 1 : 2 );
	      salomeActor->Update();
	    }
	}
      }
    }
  }
}

void ATOMSOLVGUI_Displayer::setColor( const QStringList& entries, const QColor& color  )
{
  SALOME_View* view = GetActiveView();
  if ( view && dynamic_cast<SVTK_Viewer*>( view ) ) {
    for ( QStringList::ConstIterator it = entries.begin(); it != entries.end(); ++it ) {    
      if ( SALOME_Prs* prs = view->CreatePrs( it->toLatin1() ) ) {
	SVTK_Prs* vtkPrs = dynamic_cast<SVTK_Prs*>( prs );
	vtkActorCollection* lst = vtkPrs ? vtkPrs->GetObjects() : 0;
	if ( lst ) {
	  lst->InitTraversal();
	  while ( vtkActor* actor = lst->GetNextActor() )
	    if ( SALOME_Actor* salomeActor = SALOME_Actor::SafeDownCast( actor ) ) {
	      salomeActor->SetColor( (float)(color.red()) / 255., (float)(color.green()) / 255., (float)(color.blue()) / 255.);
	      salomeActor->Update();
	    }
	}
      }
    }
  }
}

QColor ATOMSOLVGUI_Displayer::getColor( const QString& entry )
{
  SALOME_View* view = GetActiveView();
  if ( view && dynamic_cast<SVTK_Viewer*>( view ) ) {
    if ( SALOME_Prs* prs = view->CreatePrs( entry.toLatin1() ) ) {
      SVTK_Prs* vtkPrs = dynamic_cast<SVTK_Prs*>( prs );
      vtkActorCollection* lst = vtkPrs ? vtkPrs->GetObjects() : 0;
      if ( lst ) {
	lst->InitTraversal();
	while ( vtkActor* actor = lst->GetNextActor() )
	  if ( SALOME_Actor* salomeActor = SALOME_Actor::SafeDownCast( actor ) ) {
	    double r, g, b;
	    salomeActor->GetColor( r, g, b );
	    return QColor( (int)(r * 255.), (int)(g * 255.), (int)(b * 255.) );
	  }
      } 
    }
  }
  return QColor();
}

void ATOMSOLVGUI_Displayer::setTransparency( const QStringList& entries, const float transparency )
{
  SALOME_View* view = GetActiveView();
  if ( view && dynamic_cast<SVTK_Viewer*>( view ) ) {
    for ( QStringList::ConstIterator it = entries.begin(); it != entries.end(); ++it ) {    
      if ( SALOME_Prs* prs = view->CreatePrs( it->toLatin1() ) ) {
	SVTK_Prs* vtkPrs = dynamic_cast<SVTK_Prs*>( prs );
	vtkActorCollection* lst = vtkPrs ? vtkPrs->GetObjects() : 0;
	if ( lst ) {
	  lst->InitTraversal();
	  while ( vtkActor* actor = lst->GetNextActor() )
	    if ( SALOME_Actor* salomeActor = SALOME_Actor::SafeDownCast( actor ) ) {
	      salomeActor->SetOpacity( transparency );
	      salomeActor->Update();
	    }
	}
      }
    }
  }
}

float ATOMSOLVGUI_Displayer::getTransparency( const QString& entry )
{
  SALOME_View* view = GetActiveView();
  if ( view && dynamic_cast<SVTK_Viewer*>( view ) ) {
    if ( SALOME_Prs* prs = view->CreatePrs( entry.toLatin1() ) ) {
      SVTK_Prs* vtkPrs = dynamic_cast<SVTK_Prs*>( prs );
      vtkActorCollection* lst = vtkPrs ? vtkPrs->GetObjects() : 0;
      if ( lst ) {
	lst->InitTraversal();
	while ( vtkActor* actor = lst->GetNextActor() )
	  if ( SALOME_Actor* salomeActor = SALOME_Actor::SafeDownCast( actor ) )
	    return salomeActor->GetOpacity();
      }
    }
  }
  return -1.;
}
