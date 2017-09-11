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

#include "ATOMICGUI_Selection.h"
#include "LightApp_DataOwner.h"

#include <LightApp_SelectionMgr.h>
#include <qstringlist.h>

/*! Constructor */
ATOMICGUI_Selection::ATOMICGUI_Selection ()
: LightApp_Selection()
{
}

/*! Destructor */
ATOMICGUI_Selection::~ATOMICGUI_Selection()
{
}

/*! Initialization of Selection */
void ATOMICGUI_Selection::init( const QString& client, LightApp_SelectionMgr* mgr )
{
  if ( mgr ) {
    SUIT_DataOwnerPtrList sel;
    mgr->selected( sel);
    SUIT_DataOwnerPtrList::const_iterator anIt  = sel.begin(),
                                          aLast = sel.end();
    for ( ; anIt != aLast; anIt++ ) {
      QString type = "Unknown";
      SUIT_DataOwner* owner = (SUIT_DataOwner*)( (*anIt).get() );
      LightApp_DataOwner* sowner = dynamic_cast<LightApp_DataOwner*>( owner );
      QStringList es = sowner->entry().split( "_" );
      if ( es.count() > 0 && es[ 0 ] == "ATOMICGUI" ) {
	if ( es.count() > 1 ) {
	  if( es[ 1 ] == "root" )
	    type = "Root";
	  else
	    type = "Molecule";
	  if ( es.count() > 2 )
	    type = "Atom";
	}
      }
      myTypes.append( type );
    }
  }
  LightApp_Selection::init( client, mgr );
}

/*! Returns number of objects of supported types. */
int ATOMICGUI_Selection::count() const
{
  return myTypes.count();
}

/*! Returns selection parameter value. */
QVariant ATOMICGUI_Selection::parameter( const int ind, const QString& p ) const
{
  if ( p == "type" )
    return myTypes[ ind ];
  return LightApp_Selection::parameter( ind, p );
}
