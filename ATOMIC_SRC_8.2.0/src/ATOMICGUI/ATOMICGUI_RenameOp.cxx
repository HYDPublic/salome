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

#include "ATOMICGUI_RenameOp.h"
#include "ATOMICGUI_DataModel.h"
#include "ATOMICGUI_DataObject.h"
#include "ATOMICGUI.h"

#include <LightApp_Application.h>
#include <SUIT_Desktop.h>

#include <qinputdialog.h>

/*! Constructor */
ATOMICGUI_RenameOp::ATOMICGUI_RenameOp()
: ATOMICGUI_Operation()
{
}

/*! Destructor */
ATOMICGUI_RenameOp::~ATOMICGUI_RenameOp()
{
}

/*! Operation itself. Renames the selected object. */
void ATOMICGUI_RenameOp::startOperation()
{
  ATOMICGUI_DataModel* dm = dataModel();
  QStringList entries;
  atomModule()->selected( entries, false );

  if( entries.isEmpty() || !dm )
  {
    abort();
    return;
  }

  ATOMICGUI_DataObject* obj = dm->findObject( entries.first() );
  if ( obj && ( obj->isMolecule() || obj->isAtom() ) )
  {
    bool bOk;
    QString name = QInputDialog::getText( module()->getApp()->desktop(),
                                          tr( "ATOMICGUI_RENAME_OBJECT" ),
					  tr( "ATOMICGUI_NEW_NAME"),
					  QLineEdit::Normal,
					  obj->name(),
					  &bOk);
    if( bOk && !name.toLatin1().trimmed().isEmpty() && dm->renameObj( entries.first(), name ) )
      commit();
    else
      abort();
  }
}
