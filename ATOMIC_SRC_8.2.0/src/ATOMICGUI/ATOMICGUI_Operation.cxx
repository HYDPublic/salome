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

#include "ATOMICGUI_Operation.h"
#include "ATOMICGUI_DataModel.h"
#include "ATOMICGUI.h"

#include <LightApp_Application.h>
#include <LightApp_Dialog.h>

/*! Constructor */
ATOMICGUI_Operation::ATOMICGUI_Operation()
: LightApp_Operation()
{
}

/*! Destructor */
ATOMICGUI_Operation::~ATOMICGUI_Operation()
{
}

/*! Called when Operation is started. */
void ATOMICGUI_Operation::startOperation()
{
  if( dlg() )
  {
    disconnect( dlg(), SIGNAL( dlgOk() ),     this, SLOT( onOk() ) );
    disconnect( dlg(), SIGNAL( dlgApply() ),  this, SLOT( onApply() ) );
    disconnect( dlg(), SIGNAL( dlgClose() ),  this, SLOT( onClose() ) );

    connect( dlg(), SIGNAL( dlgOk() ),     this, SLOT( onOk() ) );
    connect( dlg(), SIGNAL( dlgApply() ),  this, SLOT( onApply() ) );
    connect( dlg(), SIGNAL( dlgClose() ),  this, SLOT( onClose() ) );

    dlg()->show();
  }
}

/*! Called when Operation is aborted. */
void ATOMICGUI_Operation::abortOperation()
{
  finish();
}

/*! Called when Operation is committed (normally ended). */
void ATOMICGUI_Operation::commitOperation()
{
  finish();
}

/*! Called when Operation is done. Finalization steps. */
void ATOMICGUI_Operation::finish()
{
  if( dlg() )
    dlg()->hide();
    
  module()->getApp()->updateObjectBrowser();
}

/*! Returns a Data Model. */
ATOMICGUI_DataModel* ATOMICGUI_Operation::dataModel() const
{
  return dynamic_cast<ATOMICGUI_DataModel*>( module()->dataModel() );
}

/*! Returns ATOMICGUI class instance. */
ATOMICGUI* ATOMICGUI_Operation::atomModule() const
{
  return dynamic_cast<ATOMICGUI*>( module() );
}

/*! slot. Called when user presses OK button of the dialog (if it was set). */
void ATOMICGUI_Operation::onOk()
{
  onApply();
  commit();
}

/*! slot. Called when user presses Apply button of the dialog (if it was set). */
void ATOMICGUI_Operation::onApply()
{
}

/*! slot. Called when user presses Close button of the dialog (if it was set). */
void ATOMICGUI_Operation::onClose()
{
  commit();
}
