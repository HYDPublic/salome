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

#if !defined(ATOMICGUI_DATAOBJECT_H)
#define ATOMICGUI_DATAOBJECT_H

#include "ATOMICGUI_ATOMICGUI.hxx"

#include <LightApp_DataObject.h>

#include <qstring.h>

class ATOMICGUI_AtomicMolecule;

/*!
 * Class       : ATOMICGUI_DataObject
 * Description : Data Object for data of ATOMIC component
 *               Using the same Data Object for Atoms and Molecules
 *               If it is an Atom then myIndex field is >= 0
 *               For Molecule myIndex equals to -1.
 */
class ATOMICGUI_EXPORT ATOMICGUI_DataObject : public virtual LightApp_DataObject
{
public:
  ATOMICGUI_DataObject ( SUIT_DataObject*, ATOMICGUI_AtomicMolecule* = 0, int = -1 );
  virtual ~ATOMICGUI_DataObject();
    
  virtual QString entry() const;

  QString    name()    const;
  QPixmap    icon(const int = NameId)    const;
  QString    toolTip(const int = NameId) const;

  ATOMICGUI_AtomicMolecule* molecule() const { return myMolecule;  }
  int        atomIndex()  const { return myIndex; }

  bool       isMolecule() const;
  bool       isAtom() const;

private:
  ATOMICGUI_AtomicMolecule*  myMolecule;
  int                        myIndex;
};

class ATOMICGUI_EXPORT ATOMICGUI_ModuleObject : public ATOMICGUI_DataObject,
                               public LightApp_ModuleObject
{
public:
  ATOMICGUI_ModuleObject ( CAM_DataModel*, SUIT_DataObject* = 0 );

  virtual QString name()    const;
  QPixmap         icon(const int = NameId)    const;
  QString         toolTip(const int = NameId) const;
};

#endif // ATOMICGUI_DATAOBJECT_H
