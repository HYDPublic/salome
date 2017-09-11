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

#if !defined(ATOMSOLVGUI_DATAOBJECT_H)
#define ATOMSOLVGUI_DATAOBJECT_H

#include "ATOMSOLVGUI_ATOMSOLVGUI.hxx"

#include <LightApp_DataObject.h>

#include <qstring.h>

#include <SALOMEconfig.h>
#include CORBA_CLIENT_HEADER(ATOMSOLV)

/*!
 * Class       : ATOMSOLVGUI_DataObject
 * Description : Data Object for data of ATOMSOLV component
 *               Using the same Data Object for Atoms and Molecules
 *               If it is an Atom then myIndex field is >= 0
 *               For Molecule myIndex equals to -1.
 */
class ATOMSOLVGUI_EXPORT ATOMSOLVGUI_DataObject : public virtual LightApp_DataObject
{
public:
  ATOMSOLVGUI_DataObject ( SUIT_DataObject*, int MoleculeIndex, int AtomIndex = -1 );
  virtual ~ATOMSOLVGUI_DataObject();
    
  virtual QString entry() const;

  QString    name()    const;
  QPixmap    icon(const int = NameId)    const;
  QString    toolTip(const int = NameId) const;

  int        getMoleculeIndex() const { return myMoleculeIndex; }
  int        getAtomIndex() const { return myAtomIndex; }

  bool       isMolecule() const { return !isAtom(); }
  bool       isAtom() const { return myAtomIndex > -1; }

private:
  ATOMSOLV_ORB::TMolecule    getTMolecule() const;

  int                        myMoleculeIndex;
  int                        myAtomIndex;
};

class ATOMSOLVGUI_EXPORT ATOMSOLVGUI_ModuleObject : public ATOMSOLVGUI_DataObject,
                                 public LightApp_ModuleObject
{
public:
  ATOMSOLVGUI_ModuleObject ( CAM_DataModel*, SUIT_DataObject* = 0 );

  virtual QString name()    const;
  QPixmap         icon(const int = NameId)    const;
  QString         toolTip(const int = NameId) const;
};

#endif // ATOMSOLVGUI_DATAOBJECT_H
