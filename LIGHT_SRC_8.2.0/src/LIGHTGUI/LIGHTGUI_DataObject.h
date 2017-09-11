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
// File   : LIGHTGUI_DataObject.h
// Author : Julia DOROVSKIKH

#ifndef LIGHTGUI_DATAOBJECT_H
#define LIGHTGUI_DATAOBJECT_H

#include "LIGHTGUI_Exports.h"

#include <LightApp_DataObject.h>

class LIGHT_EXPORT LIGHTGUI_DataObject : public LightApp_DataObject
{
public:
  LIGHTGUI_DataObject( const int, const QString&, SUIT_DataObject* = 0 );
  virtual ~LIGHTGUI_DataObject();
    
  QString    name() const;
  QString    entry() const;

  QPixmap    icon( const int = NameId ) const;
  QString    toolTip( const int = NameId ) const;

  QString    lineText() const;
  void       setLineText( const QString& );
  int        lineNb() const;
  int        id() const;

private:
  int        myId;
  QString    myLineTxt;
};

#endif // LIGHTGUI_DATAOBJECT_H
