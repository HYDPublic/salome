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

#ifndef ATOMICGUI_ADD_ATOM_OP_H
#define ATOMICGUI_ADD_ATOM_OP_H

#include "ATOMICGUI_Operation.h"

#include "ATOMICGUI_ATOMICGUI.hxx"

class ATOMICGUI_AddAtomDlg;

/*!
 * Class       : ATOMICGUI_AddAtomOp
 * Description : Operation class for Atom creation
 *               Uses ATOMICGUI_AddAtomDlg class
 */
class ATOMICGUI_EXPORT ATOMICGUI_AddAtomOp : public ATOMICGUI_Operation
{
  Q_OBJECT 

public:
  ATOMICGUI_AddAtomOp();
  virtual ~ATOMICGUI_AddAtomOp();

  virtual LightApp_Dialog* dlg() const;

protected slots:
  virtual void onApply();

private:
  ATOMICGUI_AddAtomDlg*  myDlg;
};


#endif
