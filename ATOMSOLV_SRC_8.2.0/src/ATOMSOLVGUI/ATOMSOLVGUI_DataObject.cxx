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

#include "ATOMSOLVGUI_DataObject.h"
#include "ATOMSOLVGUI.h"

#include <LightApp_Study.h>
#include <LightApp_DataModel.h>

#include <SUIT_Session.h>
#include <SUIT_ResourceMgr.h>

#include <CAM_DataModel.h>
#include <CAM_Module.h>

/*! Constructor */
ATOMSOLVGUI_DataObject::ATOMSOLVGUI_DataObject( SUIT_DataObject* parent, int mi, int ai )
  : LightApp_DataObject( parent ),
    CAM_DataObject( parent),
    myMoleculeIndex( mi ),
    myAtomIndex( ai )
{
}

/*! Destructor */
ATOMSOLVGUI_DataObject::~ATOMSOLVGUI_DataObject()
{
}

/*! Returns unique object ID */
QString ATOMSOLVGUI_DataObject::entry() const
{
  QString id = "root";
  if ( myMoleculeIndex > -1 ) {
    id = QString::number( myMoleculeIndex );
    if ( myAtomIndex >= 0 )
	id += QString( "_%1" ).arg( myAtomIndex );
  }

  return QString( "ATOMSOLVGUI_%1" ).arg( id );
}

/*! Returns a name of the Data Object. */
QString ATOMSOLVGUI_DataObject::name() const
{
  ATOMSOLV_ORB::TMolecule tmolecule = getTMolecule();
  ATOMGEN_ORB::Molecule_var mol = tmolecule.molecule;
  if ( !CORBA::is_nil( mol ) ) {
    if ( !isAtom() )
      return QString( "%1 [%2]" ).arg( mol->getName() ).arg( tmolecule.temperature );
    else if ( myAtomIndex < mol->getNbAtoms() )
      return mol->getAtom( myAtomIndex )->getName();	
  }
  return QString("-Error-");
}

/*! Returns an icon of the Data Object. */
QPixmap ATOMSOLVGUI_DataObject::icon(const int) const
{
  static QPixmap pxmole = SUIT_Session::session()->resourceMgr()->loadPixmap( "ATOMSOLV", QObject::tr( "ICON_MOLECULE" ), false );
  static QPixmap pxatom = SUIT_Session::session()->resourceMgr()->loadPixmap( "ATOMSOLV", QObject::tr( "ICON_ATOM" ), false );
  return isAtom() ? pxatom : pxmole;
}

/*! Returns a tooltip for the object (to be displayed in the Object Browser). */
QString ATOMSOLVGUI_DataObject::toolTip(const int) const
{
  ATOMSOLV_ORB::TMolecule tmolecule = getTMolecule();
  ATOMGEN_ORB::Molecule_var mol = tmolecule.molecule;
  if ( !CORBA::is_nil( mol ) ) {
    if ( !isAtom() )
      return QString( QObject::tr( "MOLECULE" ) + ": %1, temperature: %2" ).
	arg( mol->getName() ).arg( tmolecule.temperature );
    else {
      ATOMGEN_ORB::Atom_var atom = mol->getAtom( myAtomIndex );
      if ( !CORBA::is_nil( atom ) )
	return QString( QObject::tr( "ATOM" ) + ": %1: %2,%3,%4" ).
	  arg( atom->getName() ).arg( atom->getX() ).arg( atom->getY() ).arg( atom->getZ() );
    }
  }
  return QString();
}

ATOMSOLV_ORB::TMolecule ATOMSOLVGUI_DataObject::getTMolecule() const
{
  ATOMSOLV_ORB::ATOMSOLV_Gen_var engine = ATOMSOLVGUI::GetATOMSOLVGen();
  LightApp_RootObject* rootObj = dynamic_cast<LightApp_RootObject*> ( root() );
  if ( rootObj && !CORBA::is_nil( engine ) ) {
    const int studyID = rootObj->study()->id();
    if ( studyID > 0 ) {
      ATOMSOLV_ORB::TMoleculeList_var molecules;
      if ( engine->getData( studyID, molecules ) && 
	   myMoleculeIndex > -1 && 
	   myMoleculeIndex < molecules->length() )
	return molecules[ myMoleculeIndex ];
    }
  }
  return ATOMSOLV_ORB::TMolecule();
}


/*! Constructor */
ATOMSOLVGUI_ModuleObject::ATOMSOLVGUI_ModuleObject( CAM_DataModel* dm, SUIT_DataObject* parent )
: ATOMSOLVGUI_DataObject( parent, -1 ),
  LightApp_ModuleObject( dm, parent ),
  CAM_DataObject( parent )
{
}

/*! Returns a name of the root object. */
QString ATOMSOLVGUI_ModuleObject::name() const
{
  return CAM_ModuleObject::name();
}

/*! Returns an icon of the root object. */
QPixmap ATOMSOLVGUI_ModuleObject::icon(const int) const
{
  QPixmap px;
  if ( dataModel() ) {
    QString anIconName = dataModel()->module()->iconName();
    if ( !anIconName.isEmpty() )
      px = SUIT_Session::session()->resourceMgr()->loadPixmap( "ATOMIC", anIconName, false );
  }
  return px;
}

/*! Returns a tooltip of the root object. */
QString ATOMSOLVGUI_ModuleObject::toolTip(const int) const
{
  return QObject::tr( "ATOMIC_ROOT_TOOLTIP" );
}

