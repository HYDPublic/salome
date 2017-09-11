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
// File   : LIGHTGUI.h
// Author : Julia DOROVSKIKH

#ifndef LIGHTGUI_H
#define LIGHTGUI_H

#include "LIGHTGUI_Exports.h"

#include <LightApp_Module.h>

class LIGHT_EXPORT LIGHTGUI: public LightApp_Module
{
  Q_OBJECT
   
  //! Menu actions
  enum { 
    lgLoadFile = 931,   //!< load text file
    lgSaveFile,         //!< save text file
    lgDisplayLine,      //!< display selected line
    lgEraseLine,        //!< erase selected line
    lgEditLine,         //!< edit selected line
    lgAddLine,          //!< insert new line
    lgDelLine,          //!< delete selected line
    lgClear             //!< clear all contents
  };

public:
  LIGHTGUI();
  ~LIGHTGUI();

  void                   initialize( CAM_Application* );

  void                   windows( QMap<int, int>& ) const;

  LightApp_Selection*    createSelection() const;

  bool                   renameObject( const QString&, const QString& );
  bool                   renameAllowed( const QString& ) const;

protected:
  CAM_DataModel*         createDataModel();
  int                    selectedLine();

  void                   displayLine( const int, const bool = false, const bool = true );
  void                   eraseLine( const int, const bool = false, const bool = true );
  void                   updateLine( const int, const bool = false, const bool = true );
  void                   eraseLines( const QList<int>&, const bool = false );
  void                   updateLines( const QList<int>&, const bool = false );

public slots:
  bool                   activateModule( SUIT_Study* );
  bool                   deactivateModule( SUIT_Study* );

private slots:
  void                   onLoadFile();
  void                   onSaveFile();
  void                   onDisplayLine();
  void                   onEraseLine();
  void                   onEditLine();
  void                   onAddLine();
  void                   onDelLine();
  void                   onClear();
};

#endif // LIGHTGUI_H
