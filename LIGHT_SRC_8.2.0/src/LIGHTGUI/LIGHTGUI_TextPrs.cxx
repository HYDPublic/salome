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
// File   : LIGHTGUI_TextPrs.cxx
// Author : Natalia DONIS

#include "LIGHTGUI_TextPrs.h"

#include <Graphic3d_ArrayOfPoints.hxx>
#include <Graphic3d_Group.hxx>
#include <Graphic3d_Vertex.hxx>
#include <Prs3d_Root.hxx>
#include <Select3D_SensitivePoint.hxx>
#include <SelectMgr_EntityOwner.hxx>
#include <SelectMgr_Selection.hxx>

/*!
  \class LIGHTGUI_TextPrs
  \brief Presentation object for the string line.

  This class is used to display a text line in the OCC 3D viewer.
*/

OCCT_IMPLEMENT_STANDARD_RTTIEXT( LIGHTGUI_TextPrs, AIS_InteractiveObject )

const Standard_Integer aCharSize = 16;

/*!
  \brief Contructor.
  \param theString text string
  \param thePos text position in the viewer
*/
LIGHTGUI_TextPrs::LIGHTGUI_TextPrs( const char* theString, const gp_Pnt& thePos )
: myPos( thePos )
{
  myString = new char[strlen( theString ) + 1];
  strcpy( myString, theString );
}

/*!
  \brief Destructor.
*/
LIGHTGUI_TextPrs::~LIGHTGUI_TextPrs()
{
  delete myString;
}

/*!
  \brief Get presentation text size.
*/
int LIGHTGUI_TextPrs::TextSize()
{
  return aCharSize;
}

/*!
  \brief Compute the presentation.
  \param prsMgr presentation manager (not used)
  \param thePrs presentation
  \param mode display mode
*/
void LIGHTGUI_TextPrs::Compute( const Handle(PrsMgr_PresentationManager3d)& /*prsMgr*/,
                                const Handle(Prs3d_Presentation)& thePrs,
                                const Standard_Integer /*mode*/ )
{
  Handle(Graphic3d_Group) aGroup = Prs3d_Root::CurrentGroup( thePrs );
  Graphic3d_Vertex aTextPos;
  aTextPos.SetCoord( myPos.X(), myPos.Y(), myPos.Z() );
  Handle(Graphic3d_ArrayOfPoints) anArrayOfPoints = new Graphic3d_ArrayOfPoints (1);
  anArrayOfPoints->AddVertex(myPos.X(), myPos.Y(), myPos.Z());
  aGroup->AddPrimitiveArray(anArrayOfPoints);
  aGroup->Text( myString, aTextPos, aCharSize );
}

/*!
  \brief Compute selection of the presentation.
  \param theSelection current selection object
  \param mode display mode
*/
void LIGHTGUI_TextPrs::ComputeSelection( const Handle(SelectMgr_Selection)& theSelection,
                                         const Standard_Integer /*mode*/ )
{
  Handle(SelectMgr_EntityOwner) eown = new SelectMgr_EntityOwner(this);
  eown -> SelectBasics_EntityOwner::Set( aCharSize );
  Handle(Select3D_SensitivePoint) seg = new Select3D_SensitivePoint(eown, myPos);
  theSelection->Add(seg);
}
