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

#include "ATOMSOLVGUI_Selection.h"

#include <LightApp_DataOwner.h>
#include <LightApp_Displayer.h>
#include <SalomeApp_Study.h>

#include <OCCViewer_ViewModel.h>

#include <SUIT_Session.h>
#include <SUIT_ViewWindow.h>
#include <SUIT_ViewManager.h>
#include <SUIT_ViewModel.h>

#include <SALOMEDSClient_SObject.hxx>
#include <SALOMEDSClient_Study.hxx>

#include <SALOME_Prs.h>
#include <SALOME_InteractiveObject.hxx>

#include <SVTK_Prs.h>
#include <SALOME_Actor.h>

#include <SVTK_ViewModel.h>


// VTK Includes
#include <vtkActorCollection.h>

ATOMSOLVGUI_Selection::ATOMSOLVGUI_Selection()
{
}

ATOMSOLVGUI_Selection::~ATOMSOLVGUI_Selection()
{
}

QVariant ATOMSOLVGUI_Selection::parameter( const int ind, const QString& p ) const
{
  if ( p == "displaymode" )
    return QVariant( displayMode( ind ) );
  
  return LightApp_Selection::parameter( ind, p );
}

QString ATOMSOLVGUI_Selection::displayMode( const int index ) const
{
  SALOME_View* view = LightApp_Displayer::GetActiveView();
  QString viewType = activeViewType();
  if ( view && viewType == SVTK_Viewer::Type() ) {
    if ( SALOME_Prs* prs = view->CreatePrs( entry( index ).toLatin1() ) ) {
      SVTK_Prs* vtkPrs = dynamic_cast<SVTK_Prs*>( prs );
      vtkActorCollection* lst = vtkPrs ? vtkPrs->GetObjects() : 0;
      if ( lst ) {
	lst->InitTraversal();
	vtkActor* actor = lst->GetNextActor();
	if ( actor ) {
	  SALOME_Actor* salActor = dynamic_cast<SALOME_Actor*>( actor );
	  if ( salActor ) {
	    int dm = salActor->GetRepresentation();
	    if ( dm == 0 )
	      return "Points";
	    else if ( dm == 1 )
	      return "Wireframe";
	    else if ( dm == 2 )
	      return "Surface";
	  } // if ( salome actor )
	} // if ( actor )
      } // if ( lst == vtkPrs->GetObjects() )
    }
  }
  return "Undefined";
}

