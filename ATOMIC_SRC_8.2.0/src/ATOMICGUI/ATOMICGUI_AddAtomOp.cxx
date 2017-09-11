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

#include "ATOMICGUI_AddAtomOp.h"
#include "ATOMICGUI_AddAtomDlg.h"
#include "ATOMICGUI_DataModel.h"
#include "ATOMICGUI.h"

#include <LightApp_Module.h>
#include <LightApp_Application.h>
#include <SUIT_Desktop.h>

/*! Constructor */
ATOMICGUI_AddAtomOp::ATOMICGUI_AddAtomOp()
  : ATOMICGUI_Operation(),
    myDlg( 0 )
{
}

/*! Destructor */
ATOMICGUI_AddAtomOp::~ATOMICGUI_AddAtomOp()
{
  if ( myDlg )
    delete myDlg;
}

/*! Returns Operation dialog */
LightApp_Dialog* ATOMICGUI_AddAtomOp::dlg() const
{
  if ( !myDlg )
    const_cast<ATOMICGUI_AddAtomOp*>( this )->myDlg = new ATOMICGUI_AddAtomDlg( module()->getApp()->desktop() );

  return myDlg;
}

/*! slot, called when user presses Apply button of the dialog */
void ATOMICGUI_AddAtomOp::onApply()
{
  QStringList entries;
  atomModule()->selected( entries, false );
  ATOMICGUI_AddAtomDlg* d = dynamic_cast<ATOMICGUI_AddAtomDlg*>( dlg() );
  if( dataModel() && d && d->acceptData( entries ) )
  {
    QString name;
    double x, y, z;
    d->data( name, x, y, z );
    dataModel()->addAtom( entries.first(), name, x, y, z );
    module()->getApp()->updateObjectBrowser();
  }
}

