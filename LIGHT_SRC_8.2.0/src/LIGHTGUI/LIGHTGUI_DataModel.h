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
// File   : LIGHTGUI_DataModel.h
// Author : Julia DOROVSKIKH

#ifndef LIGHTGUI_DATAMODEL_H
#define LIGHTGUI_DATAMODEL_H

#include "LIGHTGUI_Exports.h"

#include <LightApp_DataModel.h>

class LIGHTGUI_DataObject;

class LIGHT_EXPORT LIGHTGUI_DataModel : public LightApp_DataModel
{
  Q_OBJECT

public:
  LIGHTGUI_DataModel( CAM_Module* theModule );
  virtual ~LIGHTGUI_DataModel();

  virtual bool         open( const QString&, CAM_Study*, QStringList );
  virtual bool         save( QStringList& );
  virtual bool         saveAs( const QString&, CAM_Study*, QStringList& );
  virtual bool         close();
  virtual bool         create( CAM_Study* );

  virtual bool         isModified() const;
  virtual bool         isSaved() const;

  virtual void         update( LightApp_DataObject* = 0, LightApp_Study* = 0 );

  bool                 loadFile( const QString&, CAM_Study* = 0 );
  bool                 dumpFile( const QString& = QString() );
  QString              fileName() const;

  QString              getLineText( const int ) const;
  bool                 setLineText( const int, const QString& );
  bool                 insertLineBefore( const int, const QString& );
  bool                 deleteTextLine( const int );

  void                 clearAll();

  int                  generateId();

  static int           id( const QString& );
  static QString       entry( const int );

  int                  lineNb( const QString& );
  int                  lineNb( const int );

  QList<int>           getIds() const;

private:
  void                 buildTree( SUIT_DataObject*, const QStringList& );
  LIGHTGUI_DataObject* findObject( const int ) const;

private:
  QString              myFileName;
  QString              myStudyURL;
  int                  myMaxId;
};

#endif // LIGHTGUI_DATAMODEL_H
