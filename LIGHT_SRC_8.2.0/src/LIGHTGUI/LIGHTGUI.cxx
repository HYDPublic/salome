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
// File   : LIGHTGUI.cxx
// Author : Julia DOROVSKIKH

#include "LIGHTGUI.h"
#include "LIGHTGUI_DataModel.h"
#include "LIGHTGUI_Selection.h"
#include "LIGHTGUI_TextPrs.h"
#include "LIGHT_version.h"

#include <LightApp_Application.h>
#include <LightApp_DataOwner.h>
#include <LightApp_SelectionMgr.h>
#include <OCCViewer_ViewManager.h>
#include <SUIT_Desktop.h>
#include <SUIT_MessageBox.h>
#include <SOCC_Prs.h>
#include <SOCC_ViewModel.h>

#include <QIcon>
#include <QInputDialog>
#include <QStringList>

/*!
  \class LIGHTGUI
  \brief Implementation of the sample light (no-CORBA-engine) SALOME module.

  This class represents a GUI module itself; the instance of the class is
  created and exported by the static function, which is automatically invoked
  from SALOME GUI; SALOME GUI application loads the module when user activates
  it via the "Components" toolbar.
  
  The LIGHTGUI class is responsible for the creation of the data model.
  It dispatches user actions to the corresponding slots.

  Method engineIOR() of this class returns empty string which means that this 
  module uses default persistence mechanism provided by the SALOME GUI via
  the SalomeApp embedded engine (SalomeAppEngine class).
*/

/*!
  \brief Constructor. Sets the default name for the module.
*/
LIGHTGUI::LIGHTGUI()
  : LightApp_Module( "LIGHTGUI" )
{
}

/*!
  \brief Destructor.
*/
LIGHTGUI::~LIGHTGUI()
{
}

/*!
  \brief Initialize module. Creates menus, prepares context menu, etc.
  \param app application instance
*/
void LIGHTGUI::initialize( CAM_Application* app )
{
  LightApp_Module::initialize( app );

  SUIT_Desktop* desk = application()->desktop();

  createAction( lgLoadFile, tr( "TOP_LOAD_FILE" ), QIcon(), tr( "MEN_LOAD_FILE" ),
                tr( "STB_LOAD_FILE" ), 0, desk, false, this, SLOT( onLoadFile() ) );
  createAction( lgDisplayLine, tr( "TOP_DISPLAY_LINE" ), QIcon(), tr( "MEN_DISPLAY_LINE" ),
                tr( "STB_DISPLAY_LINE" ), 0, desk, false, this, SLOT( onDisplayLine() ) );
  createAction( lgEraseLine, tr( "TOP_ERASE_LINE" ), QIcon(), tr( "MEN_ERASE_LINE" ),
                tr( "STB_ERASE_LINE" ), 0, desk, false, this, SLOT( onEraseLine() ) );
  createAction( lgSaveFile, tr( "TOP_SAVE_FILE" ), QIcon(), tr( "MEN_SAVE_FILE" ),
                tr( "STB_SAVE_FILE" ), 0, desk, false, this, SLOT( onSaveFile() ) );
  createAction( lgEditLine, tr( "TOP_EDIT_LINE" ), QIcon(), tr( "MEN_EDIT_LINE" ),
                tr( "STB_EDIT_LINE" ), 0, desk, false, this, SLOT( onEditLine() ) );
  createAction( lgAddLine,  tr( "TOP_ADD_LINE" ),  QIcon(), tr( "MEN_ADD_LINE" ),
                tr( "STB_ADD_LINE" ),  0, desk, false, this, SLOT( onAddLine() ) );
  createAction( lgDelLine,  tr( "TOP_DEL_LINE" ),  QIcon(), tr( "MEN_DEL_LINE" ),
                tr( "STB_DEL_LINE" ),  0, desk, false, this, SLOT( onDelLine() ) );
  createAction( lgClear,    tr( "TOP_CLEAR_ALL" ), QIcon(), tr( "MEN_CLEAR_ALL" ),
                tr( "STB_CLEAR_ALL" ), 0, desk, false, this, SLOT( onClear() ) );

  int aFileMnu = createMenu( tr( "MEN_FILE" ), -1, -1 );
  createMenu( separator(), aFileMnu, -1, 10 );
  createMenu( lgLoadFile,  aFileMnu, 10 );
  createMenu( lgSaveFile,  aFileMnu, 10 );

  int aLightMnu = createMenu( tr( "MEN_LIGHT" ), -1, -1, 50 );
  createMenu( lgAddLine,      aLightMnu, 10 );
  createMenu( lgEditLine,     aLightMnu, 10 );
  createMenu( lgDelLine,      aLightMnu, 10 );
  createMenu( separator(),    aLightMnu, -1, 10 );
  createMenu( lgClear,        aLightMnu, 10 );

  QString rule = "(client='ObjectBrowser' or client='OCCViewer') and selcount=1 and type='TextLine' and !empty";

  popupMgr()->insert ( action( lgDisplayLine ), -1, 0 );
  popupMgr()->setRule( action( lgDisplayLine ), rule + " and !visible"  );

  popupMgr()->insert ( action( lgEraseLine ), -1, 0 );
  popupMgr()->setRule( action( lgEraseLine ), rule + " and activeView='OCCViewer' and visible"  );

  rule = "client='ObjectBrowser' and selcount=1 and type='TextLine'";

  popupMgr()->insert ( action( lgEditLine ), -1, 0 );
  popupMgr()->setRule( action( lgEditLine ), rule  );

  popupMgr()->insert ( action( lgAddLine ),  -1, 0 );
  popupMgr()->setRule( action( lgAddLine ),  rule );

  popupMgr()->insert ( separator(),          -1, 0 );

  popupMgr()->insert ( action( lgDelLine ),  -1, 0 );
  popupMgr()->setRule( action( lgDelLine ),  rule );

  rule = "client='ObjectBrowser'";

  popupMgr()->insert ( action( lgClear ),    -1, 0 );
  popupMgr()->setRule( action( lgClear ),    rule );
}

/*!
  \brief Get list of compliant dockable GUI elements
  \param m map to be filled in ("type":"default_position")
*/
void LIGHTGUI::windows( QMap<int, int>& m ) const
{
  m.insert( LightApp_Application::WT_ObjectBrowser, Qt::LeftDockWidgetArea );
#ifndef DISABLE_PYCONSOLE
  m.insert( LightApp_Application::WT_PyConsole, Qt::BottomDockWidgetArea );
#endif
}

/*!
  \brief Create custom popup menu selection object.
  \return new selected object
*/
LightApp_Selection* LIGHTGUI::createSelection() const
{
  return new LIGHTGUI_Selection();
}

/*!
  \brief Create data model.
  \return module specific data model
*/
CAM_DataModel* LIGHTGUI::createDataModel()
{
  return new LIGHTGUI_DataModel( this );
}

/*!
  \brief Get the identifier of the currently selected object.
  \return ID of the currently selected line or -1 if not appropriate
  object (or multiple objects) is selected.
*/
int LIGHTGUI::selectedLine()
{
  int id = -1; // bad value
  
  // Look for selected lines
  LightApp_Application* app = getApp();
  LightApp_SelectionMgr* mgr = app ? app->selectionMgr() : 0;
  if ( mgr ) {
    SUIT_DataOwnerPtrList anOwnersList;
    mgr->selected( anOwnersList );
    
    // Get index of the single selected line
    if ( anOwnersList.size() == 1 ) {
      const LightApp_DataOwner* owner =
	dynamic_cast<const LightApp_DataOwner*>( anOwnersList[0].get() );
      QString anEntry = owner->entry();
      id = LIGHTGUI_DataModel::id( anEntry );
    }
  }
  return id;
}

/*!
  \brief Display the object with the specified identifier.
  \param id object ID
  \param allViewers if \c true the object is displayed in all existing viewers
  (OCC viewers are supported only)
  \param upd if \c true, update the viewer(s)
*/
void LIGHTGUI::displayLine( const int id, const bool allViewers, const bool upd )
{
  LIGHTGUI_DataModel* dm = dynamic_cast<LIGHTGUI_DataModel*>( dataModel() );
  if ( dm && id > 0 ) {
    ViewManagerList viewers;
    if ( allViewers )
      getApp()->viewManagers( "OCCViewer", viewers );
    else
      viewers.append( getApp()->getViewManager( "OCCViewer", true ) );
    
    for ( ViewManagerList::iterator it = viewers.begin(); it != viewers.end(); ++it ) {
      OCCViewer_ViewManager* aMgr = dynamic_cast<OCCViewer_ViewManager*>( *it );
      if ( !aMgr ) continue;
      SOCC_Viewer* aViewer = (SOCC_Viewer*)aMgr->getViewModel();  
      if ( !aViewer ) continue;
      QString aLine = dm->getLineText( id );
      QString entry = LIGHTGUI_DataModel::entry( id );
      SOCC_Prs* prs = dynamic_cast<SOCC_Prs*>( aViewer->CreatePrs( entry.toLatin1() ) );
      if ( prs ) {
	double aX = 0, aY = dm->lineNb( id ) * LIGHTGUI_TextPrs::TextSize(), aZ = 0;
	aViewer->Erase( prs, true );
	Handle(LIGHTGUI_TextPrs) aPrs = new LIGHTGUI_TextPrs( aLine.toLatin1(), gp_Pnt( aX, aY, aZ ) );
	aPrs->SetOwner( new SALOME_InteractiveObject( entry.toLatin1(), "" ) );
	prs->Clear();
	prs->AddObject( aPrs );
	aViewer->Display( prs );
	if ( upd )
	  aViewer->Repaint();
      }
    }
  }
}

/*!
  \brief Erase the object with the specified identifier.
  \param id object ID
  \param allViewers if \c true the object is erased in all existing viewers
  (OCC viewers are supported only)
  \param upd if \c true, update the viewer(s)
*/
void LIGHTGUI::eraseLine( const int id, const bool allViewers, const bool upd )
{
  LIGHTGUI_DataModel* dm = dynamic_cast<LIGHTGUI_DataModel*>( dataModel() );
  if ( dm && id > 0 ) {
    ViewManagerList viewers;
    if ( allViewers )
      getApp()->viewManagers( "OCCViewer", viewers );
    else
      viewers.append( getApp()->getViewManager( "OCCViewer", true ) );
    
    for ( ViewManagerList::iterator it = viewers.begin(); it != viewers.end(); ++it ) {
      OCCViewer_ViewManager* aMgr = dynamic_cast<OCCViewer_ViewManager*>( *it );
      if ( !aMgr ) continue;
      SOCC_Viewer* aViewer = (SOCC_Viewer*)aMgr->getViewModel();  
      if ( !aViewer ) continue;
      QString entry = LIGHTGUI_DataModel::entry( id );
      SOCC_Prs* prs = dynamic_cast<SOCC_Prs*>( aViewer->CreatePrs( entry.toLatin1() ) );
      if ( prs ) {
	aViewer->Erase( prs, false );
	if ( upd )
	  aViewer->Repaint();
      }
    }
  }
}

/*!
  \brief Update the object presentation.
  \param id object ID
  \param allViewers if \c true the object is updated in all existing viewers
  (OCC viewers are supported only)
  \param upd if \c true, update the viewer(s)
*/
void LIGHTGUI::updateLine( const int id, const bool allViewers, const bool upd )
{
  LIGHTGUI_DataModel* dm = dynamic_cast<LIGHTGUI_DataModel*>( dataModel() );
  if ( dm && id > 0 ) {
    ViewManagerList viewers;
    if ( allViewers )
      getApp()->viewManagers( "OCCViewer", viewers );
    else
      viewers.append( getApp()->getViewManager( "OCCViewer", true ) );
    
    for ( ViewManagerList::iterator it = viewers.begin(); it != viewers.end(); ++it ) {
      OCCViewer_ViewManager* aMgr = dynamic_cast<OCCViewer_ViewManager*>( *it );
      if ( !aMgr ) continue;
      SOCC_Viewer* aViewer = (SOCC_Viewer*)aMgr->getViewModel();  
      if ( !aViewer ) continue;
      QString aLine = dm->getLineText( id );
      QString entry = LIGHTGUI_DataModel::entry( id );
      if ( aViewer->isVisible( new SALOME_InteractiveObject( entry.toLatin1(), "" ) ) ) {
	SOCC_Prs* prs = dynamic_cast<SOCC_Prs*>( aViewer->CreatePrs( entry.toLatin1() ) );
	if ( prs ) {
	  if ( aLine.isEmpty() ) {
	    aViewer->Erase( prs, false );
	  }
	  else {
	    double aX = 0, aY = dm->lineNb( id ) * LIGHTGUI_TextPrs::TextSize(), aZ = 0;
	    aViewer->Erase( prs, true );
	    Handle(LIGHTGUI_TextPrs) aPrs = new LIGHTGUI_TextPrs( aLine.toLatin1(), gp_Pnt( aX, aY, aZ ) );
	    aPrs->SetOwner( new SALOME_InteractiveObject( entry.toLatin1(), "" ) );
	    prs->Clear();
	    prs->AddObject( aPrs );
	    aViewer->Display( prs );
	  }
	  if ( upd )
	    aViewer->Repaint();
	}
      }
    }
  }
}

/*!
  \brief Erase objects.
  \param l objects IDs list
  \param allViewers if \c true the objects are erased in all existing viewers
  (OCC viewers are supported only)
*/
void LIGHTGUI::eraseLines( const QList<int>& l, const bool allViewers )
{
  for ( QList<int>::const_iterator it = l.begin(); it != l.end(); ++it )
    eraseLine( *it, allViewers );
}

/*!
  \brief Update objects presentations.
  \param l objects IDs list
  \param allViewers if \c true the objects are updated in all existing viewers
  (OCC viewers are supported only)
*/
void LIGHTGUI::updateLines( const QList<int>& l, const bool allViewers )
{
  for ( QList<int>::const_iterator it = l.begin(); it != l.end(); ++it )
    updateLine( *it, allViewers );
}

/*!
  \brief Activate module.
  \param study current study
  \return \c true if activaion is done successfully or 0 to prevent
  activation on error
*/
bool LIGHTGUI::activateModule( SUIT_Study* study )
{
  bool isDone = LightApp_Module::activateModule( study );
  if ( !isDone ) return false;

  setMenuShown( true );

  return isDone;
}

/*!
  \brief Deactivate module.
  \param study current study
  \return \c true if deactivaion is done successfully or 0 to prevent
  deactivation on error
*/
bool LIGHTGUI::deactivateModule( SUIT_Study* study )
{
  // hide menus
  setMenuShown( false );

  return LightApp_Module::deactivateModule( study );
}

/*!
  \brief Called when "load text file" action is activated.
*/
void LIGHTGUI::onLoadFile()
{
  LIGHTGUI_DataModel* dm = dynamic_cast<LIGHTGUI_DataModel*>( dataModel() );
  if ( !dm ) return;

  QStringList filtersList;
  filtersList.append( tr( "LIGHTGUI_MEN_TXT_FILES" ) );
  filtersList.append( tr( "LIGHTGUI_MEN_ALL_FILES" ) );

  // Select a file to be loaded
  QString aFileName = getApp()->getFileName( true, QString(), filtersList.join( ";;" ), tr( "LIGHTGUI_MEN_LOAD" ), 0 );
  if ( !aFileName.isEmpty() ) {
    // Load the file
    QList<int> l = dm->getIds();
    if ( dm->loadFile( aFileName ) ) {
      eraseLines( l, true );
      updateObjBrowser( true );
    } 
    else {
      SUIT_MessageBox::warning ( application()->desktop(),
			         tr( "WRN_WARNING" ),
			         tr( "WRN_LOAD_FAILED" ) );
      }
  }
}

/*!
  \brief Called when "save text file" action is activated.
*/
void LIGHTGUI::onSaveFile()
{
  LIGHTGUI_DataModel* dm = dynamic_cast<LIGHTGUI_DataModel*>( dataModel() );
  if ( !dm ) return;

  QStringList filtersList;
  filtersList.append( tr( "LIGHTGUI_MEN_TXT_FILES" ) );
  filtersList.append( tr( "LIGHTGUI_MEN_ALL_FILES" ) );

  // Select a file name to dump the lines into
  QString aFileName = getApp()->getFileName( false, dm->fileName(), filtersList.join( ";;" ), tr( "LIGHTGUI_MEN_DUMP" ), 0 );
  if ( !aFileName.isEmpty() ) {
    // Dump the file
    if ( !dm->dumpFile( aFileName ) ) {
      SUIT_MessageBox::warning ( application()->desktop(),
			         tr( "WRN_WARNING" ),
			         tr( "WRN_DUMP_FAILED" ) );
      }
  }
}

/*!
  \brief Called when "display selected line" action is activated.
*/
void LIGHTGUI::onDisplayLine()
{
  LIGHTGUI_DataModel* dm = dynamic_cast<LIGHTGUI_DataModel*>( dataModel() );
  if ( !dm ) return;

  // Define position
  int id = selectedLine();
  QString aLine = dm->getLineText( id );
  
  // Check, that position is defined
  if ( id == -1 || aLine.isEmpty() ) {
    SUIT_MessageBox::warning ( application()->desktop(),
			       tr( "WRN_WARNING" ),
			       tr( "WRN_SELECT_LINE" ) );
    return;
  }
  displayLine( id );
}

/*!
  \brief Called when "erase selected line" action is activated.
*/
void LIGHTGUI::onEraseLine()
{
  LIGHTGUI_DataModel* dm = dynamic_cast<LIGHTGUI_DataModel*>( dataModel() );
  if ( !dm ) return;

  // Define position
  int id = selectedLine();
  QString aLine = dm->getLineText( id );
  
  // Check, that position is defined
  if ( id == -1 || aLine.isEmpty() ) {
    SUIT_MessageBox::warning ( application()->desktop(),
			       tr( "WRN_WARNING" ),
			       tr( "WRN_SELECT_LINE" ) );
    return;
  }
  eraseLine( id );
}

/*!
  \brief Called when "edit selected line" action is activated.
*/
void LIGHTGUI::onEditLine()
{
  LIGHTGUI_DataModel* dm = dynamic_cast<LIGHTGUI_DataModel*>( dataModel() );
  if ( !dm ) return;

  // Define position
  int id = selectedLine();
  
  // Check, that position is defined
  if ( id == -1 ) {
    SUIT_MessageBox::warning ( application()->desktop(),
			       tr( "WRN_WARNING" ),
			       tr( "WRN_SELECT_LINE" ) );
    return;
  }

  // Get new text
  bool isOk;
  QString aText = QInputDialog::getText( application()->desktop(),
                                         tr( "LIGHTGUI_EDIT_LINE" ),
					 QString(),
					 QLineEdit::Normal,
					 dm->getLineText( id ),
					 &isOk );
  if ( !isOk ) return;
  
  // try to change a text of the selected line
  
  isOk = dm->setLineText( id, aText );

  if ( isOk ) {
    updateLine( id, true );
    updateObjBrowser( true );
  } 
  else {
    SUIT_MessageBox::warning ( application()->desktop(),
			       tr( "WRN_WARNING" ),
			       tr( "WRN_EDIT_FAILED" ) );
  }
}

/*!
  \brief Called when "insert new line" action is activated.
*/
void LIGHTGUI::onAddLine()
{
  LIGHTGUI_DataModel* dm = dynamic_cast<LIGHTGUI_DataModel*>( dataModel() );
  if ( !dm ) return;

  // Define position
  int id = selectedLine();
  
  // Get new text
  bool isOk;
  QString aText = QInputDialog::getText( getApp()->desktop(),
                                         tr( "LIGHTGUI_ADD_LINE" ),
					 QString(),
					 QLineEdit::Normal,
					 QString(),
					 &isOk );
  if ( !isOk ) return;

  QList<int> l = dm->getIds();

  // try to insert/add text line
  isOk = dm->insertLineBefore( id, aText );

  if ( isOk ) {
    updateLines( l, true );
    updateObjBrowser( true );
  } 
  else {
    SUIT_MessageBox::warning ( application()->desktop(),
			       tr( "WRN_WARNING" ),
			       tr( "WRN_ADD_FAILED" ) );
  }
}

/*!
  \brief Called when "delete selected line" action is activated.
*/
void LIGHTGUI::onDelLine()
{
  LIGHTGUI_DataModel* dm = dynamic_cast<LIGHTGUI_DataModel*>( dataModel() );
  if ( !dm ) return;

  // Define position
  int id = selectedLine();
  
  // Check, that position is defined
  if ( id == -1 ) {
    SUIT_MessageBox::warning ( application()->desktop(),
			       tr( "WRN_WARNING" ),
			       tr( "WRN_SELECT_LINE" ) );
    return;
  }

  QList<int> l = dm->getIds();

  // try to delete line
  bool isOk = dm->deleteTextLine( id );
  if ( isOk ) {
    updateLines( l, true );
    updateObjBrowser( true );
  } 
  else {
    SUIT_MessageBox::warning ( application()->desktop(),
			       tr( "WRN_WARNING" ),
			       tr( "WRN_DELETE_FAILED" ) );
  }
}

/*!
  \brief Called when "clear all contents" action is activated.
*/
void LIGHTGUI::onClear()
{
  LIGHTGUI_DataModel* dm = dynamic_cast<LIGHTGUI_DataModel*>( dataModel() );
  if ( !dm ) return;

  QList<int> l = dm->getIds();
  eraseLines( l, true );

  dm->clearAll();

  updateObjBrowser( true );
}

/*!
  \brief Check if this object is can't be renamed in place

  This method can be re-implemented in the subclasses.
  Return true in case if object isn't reference or component (module root).

  \param entry column id
  \return \c true if the item can be renamed by the user in place (e.g. in the Object browser)
*/
bool LIGHTGUI::renameAllowed( const QString& entry ) const
{
  LIGHTGUI_DataModel* dm = dynamic_cast<LIGHTGUI_DataModel*>( dataModel() );
  return dm && dm->lineNb( entry ) >= 0;
}

/*!
  Rename object by entry.
  \param entry entry of the object
  \param name new name of the object
  \brief Return \c true if rename operation finished successfully, \c false otherwise.
*/
bool LIGHTGUI::renameObject( const QString& entry, const QString& name )
{
  bool result = false;

  LIGHTGUI_DataModel* dm = dynamic_cast<LIGHTGUI_DataModel*>( dataModel() );
  if ( dm && dm->lineNb( entry ) ) {
    int id = LIGHTGUI_DataModel::id( entry );
    result = dm->setLineText( id, name );
    if ( result ) {
      updateLine( id, true );
      updateObjBrowser( true );
    }
  }
  return result;
}

/*!
  \fn CAM_Module* createModule();
  \brief Export module instance (factory function).
  \return new created instance of the module
*/

/*!
  \fn char* getModuleVersion();
  \brief Get version of this module.
  \return version information
*/

extern "C" {
  LIGHT_EXPORT CAM_Module* createModule() {
    return new LIGHTGUI();
  }
  
  LIGHT_EXPORT char* getModuleVersion() {
    return (char*)LIGHT_VERSION_STR;
  }
}
