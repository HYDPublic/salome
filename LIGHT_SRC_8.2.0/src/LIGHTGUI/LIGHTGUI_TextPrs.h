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
// File   : LIGHTGUI_TextPrs.h
// Author : Natalia DONIS

#ifndef LIGHTGUI_TEXTPRS_H
#define LIGHTGUI_TEXTPRS_H

#include <AIS_InteractiveObject.hxx>
#include <gp_Pnt.hxx>
#include <PrsMgr_PresentationManager3d.hxx>
#include <Prs3d_Presentation.hxx>
#include <SelectMgr_Selection.hxx>
#include <Standard_DefineHandle.hxx>

#include <Basics_OCCTVersion.hxx>

class LIGHTGUI_TextPrs: public AIS_InteractiveObject
{
public:
  Standard_EXPORT LIGHTGUI_TextPrs( const char*, const gp_Pnt& );
  Standard_EXPORT ~LIGHTGUI_TextPrs();

  static int                   TextSize();

private:
  Standard_EXPORT void         Compute( const Handle(PrsMgr_PresentationManager3d)&,
                                        const Handle(Prs3d_Presentation)&,
                                        const Standard_Integer = 0 );

  Standard_EXPORT virtual void ComputeSelection( const Handle(SelectMgr_Selection)&,
                                                 const Standard_Integer );

private:
  char*              myString;
  gp_Pnt             myPos;

public:
  OCCT_DEFINE_STANDARD_RTTIEXT(LIGHTGUI_TextPrs,AIS_InteractiveObject)
};

DEFINE_STANDARD_HANDLE( LIGHTGUI_TextPrs, AIS_InteractiveObject )

#endif // LIGHTGUI_TEXTPRS_H
