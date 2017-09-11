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

#ifndef ATOMSOLVGUI_DISPLAYER_HEADER
#define ATOMSOLVGUI_DISPLAYER_HEADER

#include "ATOMSOLVGUI_ATOMSOLVGUI.hxx"

#include <LightApp_Displayer.h>

#include <qstringlist.h>
#include <qcolor.h>

class SALOME_Actor;

class ATOMSOLVGUI_EXPORT ATOMSOLVGUI_Displayer : public LightApp_Displayer
{
public:
  ATOMSOLVGUI_Displayer();
  virtual ~ATOMSOLVGUI_Displayer();

  virtual bool canBeDisplayed( const QString& /*entry*/, const QString& /*viewer_type*/ ) const;

  void updateActor( SALOME_Actor* );

  void setDisplayMode( const QStringList& entries, const QString& mode );
  void setColor( const QStringList& entries, const QColor& color );
  QColor getColor( const QString& entry );
  void setTransparency( const QStringList& entries, const float transparency );
  float getTransparency( const QString& entry );

  static void setDefaultRepresentation( const int r ) { myRepresentation = r ; }
  static int  getDefaultRepresentation() { return myRepresentation; }

  static void    setDefaultRadius( const double r ) { myRadius = r ; }
  static double  getDefaultRadius() { return myRadius; }

protected:
  virtual SALOME_Prs* buildPresentation( const QString&, SALOME_View* = 0 );

  static int myRepresentation; // default 3D Representation mode
  static double myRadius; // default radius of 3D sphere
};

#endif
