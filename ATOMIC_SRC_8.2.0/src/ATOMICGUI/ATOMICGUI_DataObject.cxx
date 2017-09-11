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

#include "ATOMICGUI_DataObject.h"
#include "ATOMICGUI_Data.h"

#include <SUIT_Session.h>
#include <SUIT_ResourceMgr.h>
#include <CAM_DataModel.h>
#include <CAM_Module.h>

/*! Constructor */
ATOMICGUI_DataObject::ATOMICGUI_DataObject( SUIT_DataObject* parent,
					    ATOMICGUI_AtomicMolecule* mol, 
					    int idx )
  : LightApp_DataObject( parent ),
    CAM_DataObject( parent),
    myMolecule( mol ),
    myIndex( idx )
{
}

/*! Destructor */
ATOMICGUI_DataObject::~ATOMICGUI_DataObject()
{
}

/*! Returns unique object ID */
QString ATOMICGUI_DataObject::entry() const
{
  QString id = "root";
  if ( myMolecule )
    id = QString::number( myMolecule->id() ); 
  if ( myIndex >= 0 )
    id += QString( "_%1" ).arg( QString::number( myMolecule->atomId( myIndex ) ) ); 
  return QString( "ATOMICGUI_%1" ).arg( id );
}

/*! Returns a name of the Data Object. */
QString ATOMICGUI_DataObject::name() const
{
  QString n;
  if ( myMolecule )
    n = myIndex < 0 ? myMolecule->name() 
                    : myMolecule->atomName( myIndex );
  return n;
}

/*! Returns an icon of the Data Object. */
QPixmap ATOMICGUI_DataObject::icon(const int column) const
{
  static QPixmap pxmole = SUIT_Session::session()->resourceMgr()->loadPixmap( "ATOMIC", QObject::tr( "ICON_MOLECULE" ), false );
  static QPixmap pxatom = SUIT_Session::session()->resourceMgr()->loadPixmap( "ATOMIC", QObject::tr( "ICON_ATOM" ), false );
  static QPixmap nullpx;
  return column == NameId ? ( myIndex < 0 ? pxmole : pxatom ) : nullpx;
}

/*! Returns a tooltip for the object (to be displayed in the Object Browser). */
QString ATOMICGUI_DataObject::toolTip(const int) const
{
  QString tt;
  if ( myMolecule )
    tt = myIndex < 0 ? QString( QObject::tr( "ATOMIC_MOLECULE" ) + ": %1" ).arg( myMolecule->name() )
      : QString( QObject::tr( "ATOMIC_ATOM" ) + ": %1: %2,%3,%4" ).
      arg( myMolecule->atomName( myIndex ) ).
      arg( myMolecule->atomX( myIndex ) ).
      arg( myMolecule->atomY( myIndex ) ).
      arg( myMolecule->atomZ( myIndex ) );
  return tt;
}

/*! Returns true if the Data Object corresponds to a Molecule. */
bool ATOMICGUI_DataObject::isMolecule() const
{
  return myMolecule && myIndex < 0;
}

/*! Returns true if the Data Object corresponds to an Atom. */
bool ATOMICGUI_DataObject::isAtom() const
{
  return myMolecule && myIndex >= 0;
}

/*! Constructor */
ATOMICGUI_ModuleObject::ATOMICGUI_ModuleObject( CAM_DataModel* dm, SUIT_DataObject* parent )
: ATOMICGUI_DataObject( parent ),
  LightApp_ModuleObject( dm, parent ),
  CAM_DataObject( parent )
{
}

/*! Returns a name of the root object. */
QString ATOMICGUI_ModuleObject::name() const
{
  return CAM_ModuleObject::name();
}

/*! Returns an icon of the root object. */
QPixmap ATOMICGUI_ModuleObject::icon(const int column) const
{
  QPixmap px;
  if ( column == NameId && dataModel() ) {
    QString anIconName = dataModel()->module()->iconName();
    if ( !anIconName.isEmpty() )
      px = SUIT_Session::session()->resourceMgr()->loadPixmap( "ATOMIC", anIconName, false );
  }
  return px;
}

/*! Returns a tooltip of the root object. */
QString ATOMICGUI_ModuleObject::toolTip(const int) const
{
  return QObject::tr( "ATOMIC_ROOT_TOOLTIP" );
}
