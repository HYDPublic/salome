// Copyright (C) 2005-2016  OPEN CASCADE
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
// File   : LIGHTGUI_DataObject.cxx
// Author : Julia DOROVSKIKH

#include "LIGHTGUI_DataObject.h"
#include "LIGHTGUI_DataModel.h"

#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>

/*!
  \class LIGHTGUI_DataObject
  \brief LIGHT module data object.

  This class presents an elementary unit of the data model.
  It includes presentation methods like: name(), icon(), toolTip(), etc.
*/

/*!
  \brief Constructor.
  \param id line identifier
  \param txt text line corresponding to the data object
  \param parent parent data object

  \note Due to virtual inheritance, the constructor explicitly
  invokes constructor of all base classes
*/
LIGHTGUI_DataObject::LIGHTGUI_DataObject( const int id, 
					  const QString& txt, 
					  SUIT_DataObject* parent )
: CAM_DataObject( parent ),
  LightApp_DataObject( parent ),
  myId( id ),
  myLineTxt( txt )
{
}

/*!
  \brief Destructor.
*/
LIGHTGUI_DataObject::~LIGHTGUI_DataObject()
{
}

/*!
  \brief Get data object name.
  \return data object name
*/
QString LIGHTGUI_DataObject::name() const
{
  return myLineTxt.trimmed().isEmpty() ? QObject::tr( "LIGHT_PARAGRAPH" ) : myLineTxt;
}

/*!
  \brief Get data object entry.
  \return data object entry
*/
QString LIGHTGUI_DataObject::entry() const
{
  return LIGHTGUI_DataModel::entry( id() );
}

/*!
  \brief Get data object icon.
  \param index data column index
  \return data object icon for the specified column
*/
QPixmap LIGHTGUI_DataObject::icon( const int index ) const
{
  if ( index == NameId ) {
    // show icon only for the "Name" column
    static QPixmap pxp = SUIT_Session::session()->resourceMgr()->loadPixmap( "LIGHT", QObject::tr( "ICON_PARAGRAPH" ), false );
    static QPixmap pxl = SUIT_Session::session()->resourceMgr()->loadPixmap( "LIGHT", QObject::tr( "ICON_LINE" ), false );
    return myLineTxt.trimmed().isEmpty() ? pxp : pxl;
  }
  return LightApp_DataObject::icon( index );
}

/*!
  \brief Get data object tooltip.
  \param index data column index
  \return data object tooltip for the specified column
*/
QString LIGHTGUI_DataObject::toolTip( const int /*index*/ ) const
{
  // show the same tooltip for all columns
  return lineText().trimmed().isEmpty() ?
    QString( QObject::tr( "LIGHT_PARAGRAPH") + " %1" ).arg( parent()->childPos( this ) + 1 ) :
    QString( QObject::tr( "LIGHT_LINE" ) + " %1: %2" ).arg( lineNb() ).arg( lineText() );
}

/*!
  \brief Get text line corresponding to the data object.
  \return text line (empty lines means paragraph)
  \sa setLineText()
*/
QString LIGHTGUI_DataObject::lineText() const
{
  return myLineTxt;
}

/*!
  \brief Set text line corresponding to the data object.
  \param txt text line (empty lines means paragraph)
  \sa lineText()
*/
void LIGHTGUI_DataObject::setLineText( const QString& txt )
{
  myLineTxt = txt;
}

/*!
  \brief Get line number.
  \return line number
*/
int LIGHTGUI_DataObject::lineNb() const
{
  if ( level() == 1 ) // root object
    return -1;
  int pos = 0;
  if ( parent() ) {
    SUIT_DataObject* o = (SUIT_DataObject*)this;
    if ( level() == 3 ) {
      pos += parent()->childPos( this ) + 1;
      o = parent();
    }
    o = o->prevBrother();
    while ( o ) {
      pos += o->childCount() + 1;
      o = o->prevBrother();
    }
  }
  return pos;
}

/*!
  \brief Get line identifier.
  \return line identifier
*/
int LIGHTGUI_DataObject::id() const
{
  return myId;
}
