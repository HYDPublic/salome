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

#if !defined(ATOMICGUI_SELECTION_H)
#define ATOMICGUI_SELECTION_H

#include "ATOMICGUI_ATOMICGUI.hxx"

#include <QtxPopupMgr.h>
#include <LightApp_Selection.h>

class LightApp_SelectionMgr;

/*!
 * Class       : ATOMICGUI_Selection
 * Description : Selection class for ATOMIC component.  
 *               Computation of object parameters for popup manager.
 */
class ATOMICGUI_EXPORT ATOMICGUI_Selection : public LightApp_Selection
{
public:
  ATOMICGUI_Selection ();
  virtual ~ATOMICGUI_Selection ();

  virtual void     init( const QString&, LightApp_SelectionMgr* );
  virtual int      count () const;
  virtual QVariant parameter ( const int, const QString& ) const;

private:
  QStringList      myTypes;
};

#endif  // ATOMICGUI_SELECTION_H
