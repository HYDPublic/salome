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

#ifndef _ATOMSOLVGUI_H_
#define _ATOMSOLVGUI_H_

#include "ATOMSOLVGUI_ATOMSOLVGUI.hxx"

#include <SalomeApp_Module.h>

#include <SALOMEconfig.h>
#include CORBA_CLIENT_HEADER(ATOMSOLV)

class SalomeApp_Application;

class ATOMSOLVGUI_EXPORT ATOMSOLVGUI: public SalomeApp_Module
{
  Q_OBJECT

public:
  ATOMSOLVGUI();

  void    initialize( CAM_Application* );
  void    windows( QMap<int, int>& ) const;
  void    viewManagers( QStringList& ) const;

  virtual LightApp_Displayer* displayer();
  
  virtual void createPreferences();
  virtual void preferencesChanged( const QString&, const QString& );

  void    selected( QStringList&, const bool );

  virtual QString engineIOR() const;

  static void InitATOMSOLVGen( SalomeApp_Application* );
  static ATOMSOLV_ORB::ATOMSOLV_Gen_var GetATOMSOLVGen();

protected:
  enum { ProcessData = 901,
	 RetrieveData,
	 Display,
	 Erase,
	 Shading,
	 PointsMode,
	 Wireframe,
	 Color,
	 Transparency };

  virtual CAM_DataModel*      createDataModel();
  virtual LightApp_Selection* createSelection() const;

public slots:
  virtual bool    deactivateModule( SUIT_Study* );
  virtual bool    activateModule( SUIT_Study* );

  virtual void    studyClosed( SUIT_Study* );

protected slots:
  void    OnProcessData();
  void    OnRetrieveData();
  void    OnDisplayerCommand();

private:
  static ATOMSOLV_ORB::ATOMSOLV_Gen_var myEngine;
};

#endif
