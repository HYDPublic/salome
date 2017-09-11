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
// File    : SierpinskyGUI.h
// Author  : Vadim SANDLER (OCN)
// Created : 13/07/05
///////////////////////////////////////////////////////////
//
#ifndef __SIERPINSKYGUI_H
#define __SIERPINSKYGUI_H

#include "SierpinskyGUI.hxx"

#include <SalomeApp_Module.h>

#include <SALOMEconfig.h>
#include CORBA_CLIENT_HEADER(Sierpinsky)

class SUIT_Desktop;
class SalomeApp_Application;

class SIERPINSKY_EXPORT SierpinskyGUI : public SalomeApp_Module
{
  Q_OBJECT

public:
  // Constructor
  SierpinskyGUI();
  // Destructor
  ~SierpinskyGUI();

  void    initialize( CAM_Application* );
  virtual QString     engineIOR() const;

  virtual void        windows( QMap<int, int>& ) const;
  virtual void        viewManagers( QStringList& ) const;

  static SIERPINSKY_ORB::SIERPINSKY_ptr InitSIERPINSKYGen( SalomeApp_Application* );
  
public slots:
  virtual bool        deactivateModule( SUIT_Study* );
  virtual bool        activateModule( SUIT_Study* );

protected slots:
  void    OnRun();

};

#endif // __SIERPINSKYGUI_H
