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

// File   : SalomePyQt.cxx
// Author : Roman NIKOLAEV, Open CASCADE S.A.S. (roman.nikolaev@opencascade.com)

#include <SUIT_Session.h>
#include <LightApp_Application.h>
#include <LightApp_Module.h>
#include <SALOME_Event.h>

#include "AtomicPy.h"
#include "ATOMICGUI_Data.h"
#include "ATOMICGUI_DataModel.h"

class TCreateMoleculeEvent: public SALOME_Event
{
public:
  QString myName;
  typedef QString TResult;
  TResult myResult;
  
  TCreateMoleculeEvent(const QString& name) : myResult(""), myName(name) {}
  virtual void Execute()
  {
    if ( !SUIT_Session::session() )
      return;
    LightApp_Application* app =  dynamic_cast<LightApp_Application*>( SUIT_Session::session()->activeApplication() );
    if(!app)
      return;
    
    LightApp_Module* module = dynamic_cast<LightApp_Module*>( app->module("Atomic") );
    if(!module)
      return;
      
    ATOMICGUI_DataModel* model = dynamic_cast<ATOMICGUI_DataModel*>(module->dataModel());
    if(!model)
      return;
    
    myResult = model->createMolecule();
    model->renameObj(myResult,myName);
  }
};

class TAddAtomEvent: public SALOME_Event
{
public:
  QString myId;
  QString myName;
  double myX, myY, myZ;

  TAddAtomEvent(const QString& id, 
		const QString& name,
		const double x,
		const double y,
		const double z) : myId(id), myName(name), myX(x), myY(y), myZ(z) {}
  virtual void Execute()
  {
    if ( !SUIT_Session::session() )
      return;
    LightApp_Application* app =  dynamic_cast<LightApp_Application*>( SUIT_Session::session()->activeApplication() );
    if(!app)
      return;
    
    LightApp_Module* module = dynamic_cast<LightApp_Module*>( app->module("Atomic") );
    if(!module)
      return;
    
    ATOMICGUI_DataModel* model = dynamic_cast<ATOMICGUI_DataModel*>(module->dataModel());
    if(!model)
      return;
    
    model->addAtom(myId,myName,myX,myY,myZ);
  }
};


AtomicMolecule::AtomicMolecule( const QString& name )
{
  myId = ProcessEvent( new TCreateMoleculeEvent( name ) );  
}

void AtomicMolecule::addAtom( const QString& atom, const double x, const double y, const double z ){
  ProcessVoidEvent( new TAddAtomEvent( myId, atom, x, y, x ) );
}
