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
// File   : LIGHTGUI_Selection.cxx
// Author : Julia DOROVSKIKH

#include "LIGHTGUI_Selection.h"
#include "LIGHTGUI_DataModel.h"
#include "LIGHTGUI.h"

#include <LightApp_Application.h>
#include <LightApp_Study.h>
#include <SALOME_InteractiveObject.hxx>
#include <SOCC_ViewModel.h>
#include <SUIT_ViewManager.h>

/*!
  \class LIGHTGUI_Selection
  \brief Handles the data selection, builds the rules to be used in the
         popup menu.

  This class provides the methods of defining the context popup menu
  contents for the selected objects by using of popup menu manager
  from the Qtx library.
*/

/*!
  \brief Constructor.
*/
LIGHTGUI_Selection::LIGHTGUI_Selection()
: LightApp_Selection()
{
}

/*!
  \brief Destructor.
*/
LIGHTGUI_Selection::~LIGHTGUI_Selection()
{
}

/*!
  \brief Get selection parameter value.
  \param index selected object index
  \param p parameter name
  \return parameter value
*/
QVariant LIGHTGUI_Selection::parameter( const int index, const QString& p ) const
{
  if ( p == "type" )
    return type( index );
  else if ( p == "empty" )
    return isEmpty( index );
  else if ( p == "visible" )
    return isVisible( index );
  return LightApp_Selection::parameter( p );
}

/*!
  \brief Check if selected object is empty line (beginning of the paragraph)
  \param index selected object index
  \return \c true if selected object is empty line
*/
bool LIGHTGUI_Selection::isEmpty( const int index ) const
{
  bool empty = true;
  LightApp_Application* app = dynamic_cast<LightApp_Application*>( study()->application() );
  if ( app ) {
    LIGHTGUI_DataModel* dm = dynamic_cast<LIGHTGUI_DataModel*>( app->activeModule()->dataModel() );
    if ( dm ) {
      QString line = dm->getLineText( LIGHTGUI_DataModel::id( entry( index ) ) );
      empty = line.isEmpty();
    }
  }
  return empty;
}

/*!
  \brief Check if selected object is displayed in the active viewer.
  \param index selected object index
  \return \c true if selected object is displayed
*/
bool LIGHTGUI_Selection::isVisible( const int index ) const
{
  bool visible = false;
  LightApp_Application* app = dynamic_cast<LightApp_Application*>( study()->application() );
  if ( app ) {
    SUIT_ViewManager* vm = app->activeViewManager();
    if ( vm && vm->getType() == "OCCViewer" ) {
      SOCC_Viewer* v = (SOCC_Viewer*)vm->getViewModel();  
      if ( v ) {
	QString e = entry( index );
	int id = LIGHTGUI_DataModel::id( e );
	visible = id != -1 && v->isVisible( new SALOME_InteractiveObject( e.toLatin1(), "" ) );
      }
    }
  }
  return visible;
}

/*!
  \brief Get selected object type.
  \param index selected object index
  \return object type
*/
QString LIGHTGUI_Selection::type( const int index ) const
{
  return lineNb( entry( index ) ) > 0 ? "TextLine" : "Unknown";
}

/*!
  \brief Get line number corresponding to the selected object.
  \param index selected object index
  \return line number or 0 if the pbject is not appropriate
*/
int LIGHTGUI_Selection::lineNb( const QString& e ) const
{
  int pos = 0; // invalid position
  LightApp_Application* app = dynamic_cast<LightApp_Application*>( study()->application() );
  if ( app ) {
    LIGHTGUI_DataModel* dm = dynamic_cast<LIGHTGUI_DataModel*>( app->activeModule()->dataModel() );
    if ( dm ) {
      pos = dm->lineNb( e );
    }
  }
  return pos;
}
