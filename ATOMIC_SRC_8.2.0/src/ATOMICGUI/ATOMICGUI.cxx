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

#include "ATOMICGUI.h"

#include "ATOMICGUI_DataModel.h"
#include "ATOMICGUI_DataObject.h"
#include "ATOMICGUI_Selection.h"
#include "ATOMICGUI_CreateMolOp.h"
#include "ATOMICGUI_AddAtomOp.h"
#include "ATOMICGUI_ImportExportOp.h"
#include "ATOMICGUI_RenameOp.h"
#include "ATOMICGUI_DeleteOp.h"

#include <ATOMIC_version.h>

#include <LightApp_Application.h>
#include <LightApp_DataOwner.h>
#include <LightApp_SelectionMgr.h>

#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SUIT_Desktop.h>

#include <qaction.h>

using namespace std;

/*! Constructor */
ATOMICGUI::ATOMICGUI()
: LightApp_Module( "ATOMICGUI" )
{
}

/*! Initialization funciton.  Called only once on first activation of GUI module.  */
void ATOMICGUI::initialize ( CAM_Application* app )
{
  LightApp_Module::initialize( app );// call parent's implementation

  SUIT_Desktop* parent = application()->desktop();
  SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr();

  // create actions
  createAction( agCreateMol, tr( "TOP_CREATE_MOL" ), resMgr->loadPixmap( "ATOMIC", tr( "ICON_MOLECULE" ) ), 
		tr( "MEN_CREATE_MOL" ), tr( "STB_CREATE_MOL" ), 0, parent, false, this, SLOT( onOperation() ) );
  createAction( agAddAtom,    tr( "TOP_ADD_ATOM" ),    resMgr->loadPixmap( "ATOMIC", tr( "ICON_ATOM" ) ), 
		tr( "MEN_ADD_ATOM" ),    tr( "STB_ADD_ATOM" ),    0, parent, false, this, SLOT( onOperation() ) );
  createAction( agRename,     tr( "TOP_RENAME" ),      resMgr->loadPixmap( "ATOMIC", tr( "ICON_RENAME" ) ),
		tr( "MEN_RENAME" ),      tr( "STB_RENAME" ), 0, parent, false, this, SLOT( onOperation() ) );
  createAction( agDelete,     tr( "TOP_DELETE_OBJ" ),  resMgr->loadPixmap( "ATOMIC", tr( "ICON_REMOVE" ) ),
		tr( "MEN_DELETE_OBJ" ),  tr( "STB_DELETE_OBJ" ),  0, parent, false, this, SLOT( onOperation() ) );
  createAction( agImportXML,  tr( "TOP_IMPORT_XML" ),  QIcon(), tr( "MEN_IMPORT_XML" ),
                tr( "STB_IMPORT_XML" ),  0, parent, false, this, SLOT( onOperation() ) );
  createAction( agExportXML,  tr( "TOP_EXPORT_XML" ),  QIcon(), tr( "MEN_EXPORT_XML" ),
                tr( "STB_EXPORT_XML" ),  0, parent, false, this, SLOT( onOperation() ) );

  // init popup menus
  int aFileMnu = createMenu( tr( "MEN_FILE" ), -1, -1 );
  createMenu( separator(), aFileMnu, -1, 10 );
  createMenu( agImportXML, aFileMnu, 10 );
  createMenu( agExportXML, aFileMnu, 10 );
  createMenu( separator(), aFileMnu, -1, 10 );

  int aEditMnu = createMenu( tr( "MEN_EDIT" ), -1, -1 );
  createMenu( separator(), aEditMnu, -1, 10 );
  createMenu( agRename,    aEditMnu, 10 );
  createMenu( agDelete,    aEditMnu, 10 );
  createMenu( separator(), aEditMnu, -1, 10 );

  int aAtomicMnu = createMenu( tr( "MEN_ATOMIC" ), -1, -1, 50 );
  createMenu( agCreateMol, aAtomicMnu, 10 );
  createMenu( separator(), aAtomicMnu, -1, 10 );
  createMenu( agAddAtom,   aAtomicMnu, 10 );

  // create toolbar
  int tbId = createTool( tr( "MEN_ATOMIC" ), QString( "Atomic" ) );
  createTool( agCreateMol, tbId );
  createTool( agAddAtom,   tbId );
  createTool( separator(),  tbId );
  createTool( agRename,     tbId );
  createTool( separator(),  tbId );
  createTool( agDelete,     tbId );

  // popup for object browser
  int parentId = -1;
  popupMgr()->insert ( separator(),            parentId, 0 );
  popupMgr()->insert ( action( agCreateMol ),  parentId, 0 );
  popupMgr()->insert ( action( agAddAtom ),    parentId, 0 );
  popupMgr()->insert ( separator(),            parentId, 0 );
  popupMgr()->insert ( action( agRename ),     parentId, 0 );
  popupMgr()->insert ( action( agDelete ),     parentId, 0 );
  popupMgr()->insert ( separator(),            parentId, 0 );

  QString rule = "client='ObjectBrowser' and selcount=1 and type='Molecule'";
  popupMgr()->setRule( action( agAddAtom ),  rule );

  rule = "client='ObjectBrowser' and selcount=1 and type='Root'";
  popupMgr()->setRule( action( agCreateMol ),rule );

  rule = "($type in {'Molecule' 'Atom'}) and client='ObjectBrowser' and selcount=1";
  popupMgr()->setRule( action( agRename ),   rule );

  rule = "($type in {'Molecule' 'Atom'}) and client='ObjectBrowser' and selcount>0";
  popupMgr()->setRule( action( agDelete ),   rule );
}

/*! Returns default icon of a component. */
QString ATOMICGUI::iconName() const
{
  return QObject::tr( "ICON_ATOMIC" );
}

/*! Returns list of windows required for this GUI module. */
void ATOMICGUI::windows ( QMap<int, int>& aMap ) const
{
  aMap.insert( LightApp_Application::WT_ObjectBrowser, Qt::LeftDockWidgetArea );
  aMap.insert( LightApp_Application::WT_PyConsole, Qt::BottomDockWidgetArea);
}

/*! Returns list of entities of selected objects. */
void ATOMICGUI::selected( QStringList& entries, const bool multiple )
{
  LightApp_SelectionMgr* mgr = getApp()->selectionMgr();
  if( !mgr )
    return;

  SUIT_DataOwnerPtrList anOwnersList;
  mgr->selected( anOwnersList );
    
  for ( int i = 0; i < anOwnersList.size(); i++ )
  {
    const LightApp_DataOwner* owner = dynamic_cast<const LightApp_DataOwner*>( anOwnersList[ i ].get() );
    QStringList es = owner->entry().split( "_" );
    if ( es.count() > 1 && es[ 0 ] == "ATOMICGUI" && es[ 1 ] != "root" )
    {
      entries.append( owner->entry() );
      if( !multiple )
	break;
    }
  }
}

/*! Instantiation of a custom Data Model. */
CAM_DataModel* ATOMICGUI::createDataModel()
{
  return new ATOMICGUI_DataModel( this );
}

/*! Instantiation of a custom Selection object - manager of parameters for popup construction. */
LightApp_Selection* ATOMICGUI::createSelection() const
{
  return new ATOMICGUI_Selection();
}

/*! Activation function. Called on every activation of a GUI module. */
bool ATOMICGUI::activateModule ( SUIT_Study* study )
{
  bool isDone = LightApp_Module::activateModule( study );
  if ( !isDone ) return false;

  setMenuShown( true );
  setToolShown( true );

  return isDone;
}

/*! Deactivation function. Called on every deactivation of a GUI module. */
bool ATOMICGUI::deactivateModule ( SUIT_Study* study )
{
  setMenuShown( false );
  setToolShown( false );

  return LightApp_Module::deactivateModule( study );
}

/*! slot connected to all functions of the component (create molecule, add atom, etc.) */
void ATOMICGUI::onOperation()
{
  if( sender() && sender()->inherits( "QAction" ) )
  {
    int id = actionId( ( QAction* )sender() );
    startOperation( id );
  }
}

/*! Instantiation of a custom Operation object - component's action manager. */
LightApp_Operation* ATOMICGUI::createOperation( const int id ) const
{
  switch( id )
  {
  case agImportXML:
    return new ATOMICGUI_ImportExportOp( true );

  case agExportXML:
    return new ATOMICGUI_ImportExportOp( false );

  case agCreateMol:
    return new ATOMICGUI_CreateMolOp();

  case agAddAtom:
    return new ATOMICGUI_AddAtomOp();

  case agRename:
    return new ATOMICGUI_RenameOp();

  case agDelete:
    return new ATOMICGUI_DeleteOp();

  default:
    return 0;
  }
}

#if defined(WIN32)
#define ATOMICGUI_EXPORT __declspec(dllexport)
#else   // WIN32
#define ATOMICGUI_EXPORT
#endif  // WIN32

/*! GUI module instantiation function */
extern "C" {
  ATOMICGUI_EXPORT CAM_Module* createModule() {
    return new ATOMICGUI();
  }
  
  ATOMICGUI_EXPORT char* getModuleVersion() {
    return (char*)ATOMIC_VERSION_STR;
  }
}
