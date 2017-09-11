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

#include "ATOMSOLVGUI.h"
#include "ATOMSOLVGUI_DataModel.h"
#include "ATOMSOLVGUI_Displayer.h"
#include "ATOMSOLVGUI_Selection.h"
#include "ATOMSOLVGUI_TransparencyDlg.h"

#include <ATOMSOLV_version.h>

#include <SUIT_MessageBox.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SUIT_Desktop.h>
#include <SUIT_ViewManager.h>

#include <QtxPopupMgr.h>

#include <SalomeApp_Application.h>
#include <SalomeApp_Study.h>

#include <LightApp_DataOwner.h>
#include <LightApp_SelectionMgr.h>
#include <LightApp_Preferences.h>

#include <SVTK_ViewModel.h>
#include <SVTK_ViewWindow.h>
#include <SALOME_Actor.h>

#include <SALOME_LifeCycleCORBA.hxx>

// QT includes
#include <qinputdialog.h>
#include <qaction.h>
#include <qcolordialog.h>
#include <qstringlist.h>

// VTK includes
#include <vtkActorCollection.h>
#include <vtkRenderer.h>

#include <utilities.h>

ATOMSOLV_ORB::ATOMSOLV_Gen_var ATOMSOLVGUI::myEngine = ATOMSOLV_ORB::ATOMSOLV_Gen::_nil();

// Constructor
ATOMSOLVGUI::ATOMSOLVGUI() :
  SalomeApp_Module( "ATOMSOLV" )
{
}

// Initialize a reference to the module's engine
void ATOMSOLVGUI::InitATOMSOLVGen( SalomeApp_Application* app )
{
  if ( !app )
    myEngine = ATOMSOLV_ORB::ATOMSOLV_Gen::_nil();
  else {
    Engines::EngineComponent_var comp = app->lcc()->FindOrLoad_Component( "FactoryServer", "ATOMSOLV" );
    ATOMSOLV_ORB::ATOMSOLV_Gen_ptr atomGen = ATOMSOLV_ORB::ATOMSOLV_Gen::_narrow(comp);
    ASSERT( !CORBA::is_nil( atomGen ) );
    myEngine = atomGen;
  }
}

// Gets an reference to the module's engine
ATOMSOLV_ORB::ATOMSOLV_Gen_var ATOMSOLVGUI::GetATOMSOLVGen()
{
  if ( CORBA::is_nil( myEngine ) ) {
    SUIT_Application* suitApp = SUIT_Session::session()->activeApplication();
    SalomeApp_Application* app = dynamic_cast<SalomeApp_Application*>( suitApp );
    InitATOMSOLVGen( app );
  }
  return myEngine;
}

// Module's initialization
void ATOMSOLVGUI::initialize( CAM_Application* app )
{
  SalomeApp_Module::initialize( app );

  InitATOMSOLVGen( dynamic_cast<SalomeApp_Application*>( app ) );

  SUIT_Desktop* aParent = app->desktop();
  SUIT_ResourceMgr* aResourceMgr = app->resourceMgr();

  // create actions
  QPixmap aPixmap = aResourceMgr->loadPixmap( "ATOMSOLV",tr( "ICON_IMPORT_DATA" ) );
  createAction( RetrieveData, tr( "TLT_RETRIEVE_DATA" ), QIcon( aPixmap ), tr( "MEN_RETRIEVE_DATA" ),
                tr( "STS_RETRIEVE_DATA" ), 0, aParent, false, this, SLOT( OnRetrieveData() ) );
  aPixmap = aResourceMgr->loadPixmap( "ATOMSOLV",tr( "ICON_PROCESS_DATA" ) );
  createAction( ProcessData, tr( "TLT_PROCESS_DATA" ), QIcon( aPixmap ), tr( "MEN_PROCESS_DATA" ),
                tr( "STS_PROCESS_DATA" ), 0, aParent, false, this, SLOT( OnProcessData() ) );

  // create menus
  int aMenuId = createMenu( tr( "MEN_ATOMSOLV" ), -1, -1, 30 );
  createMenu( RetrieveData, aMenuId, 10 );
  createMenu( ProcessData, aMenuId, 10 );

  // create toolbars
  int aToolId = createTool ( tr( "TOOL_ATOMSOLV" ), QString( "AtomSolv" ) );
  createTool( RetrieveData, aToolId );
  createTool( ProcessData, aToolId );

  // custom actions handled by displayer (display, erase, display mode, etc.)
  aPixmap = aResourceMgr->loadPixmap( "ATOMSOLV",tr( "ICON_DISPLAY" ) );
  createAction( Display, tr( "TLT_DISPLAY" ), QIcon( aPixmap ), tr( "MEN_DISPLAY" ),
                tr( "STS_DISPLAY" ), 0, aParent, false, this, SLOT( OnDisplayerCommand() ) );
  aPixmap = aResourceMgr->loadPixmap( "ATOMSOLV",tr( "ICON_ERASE" ) );
  createAction( Erase, tr( "TLT_ERASE" ), QIcon( aPixmap ), tr( "MEN_ERASE" ),
                tr( "STS_ERASE" ), 0, aParent, false, this, SLOT( OnDisplayerCommand() ) );
  createAction( Shading, tr( "TLT_SHADING" ), QIcon(), tr( "MEN_SHADING" ),
                tr( "STS_SHADING" ), 0, aParent, true, this, SLOT( OnDisplayerCommand() ) );
  createAction( Wireframe, tr( "TLT_WIREFRAME" ), QIcon(), tr( "MEN_WIREFRAME" ),
                tr( "STS_WIREFRAME" ), 0, aParent, true, this, SLOT( OnDisplayerCommand() ) );
  createAction( PointsMode, tr( "TLT_POINTSMODE" ), QIcon(), tr( "MEN_POINTSMODE" ),
                tr( "STS_POINTSMODE" ), 0, aParent, true, this, SLOT( OnDisplayerCommand() ) );
  createAction( Color, tr( "TLT_COLOR" ), QIcon(), tr( "MEN_COLOR" ),
                tr( "STS_COLOR" ), 0, aParent, false, this, SLOT( OnDisplayerCommand() ) );
  createAction( Transparency, tr( "TLT_TRANSPARENCY" ), QIcon(), tr( "MEN_TRANSPARENCY" ),
                tr( "STS_TRANSPARENCY" ), 0, aParent, false, this, SLOT( OnDisplayerCommand() ) );

  QtxPopupMgr* mgr = popupMgr();
  mgr->insert( action( Display ), -1, 0 );
  mgr->insert( action( Erase ), -1, 0 );
  int dispmodeId = mgr->insert(  tr( "MEN_DISPLAY_MODE" ), -1, -1 ); // display mode menu
  mgr->insert( action(  PointsMode ), dispmodeId, -1 );
  mgr->insert( action(  Wireframe ), dispmodeId, -1 );
  mgr->insert( action(  Shading ), dispmodeId, -1 );
  mgr->insert( action( Color ), -1, 0 );
  mgr->insert( action( Transparency ), -1, 0 );

  mgr->setRule( action( Display ), "true in $canBeDisplayed and activeModule='ATOMSOLV' and !isVisible" );
  mgr->setRule( action( Erase ), "true in $canBeDisplayed and activeModule='ATOMSOLV' and isVisible" );
  mgr->setRule( action( PointsMode ), "client='VTKViewer' and selcount>0 and isVisible" );
  mgr->setRule( action( Wireframe ), "client='VTKViewer' and selcount>0 and isVisible" );
  mgr->setRule( action( Shading ), "client='VTKViewer' and selcount>0 and isVisible" );
  mgr->setRule( action( Color ), "client='VTKViewer' and selcount>0 and isVisible" );
  mgr->setRule( action( Transparency ), "client='VTKViewer' and selcount>0 and isVisible" );

  mgr->setRule( action( PointsMode ), "$displaymode={'Points'}", QtxPopupMgr::ToggleRule );
  mgr->setRule( action( Wireframe ), "$displaymode={'Wireframe'}", QtxPopupMgr::ToggleRule );
  mgr->setRule( action( Shading ), "$displaymode={'Surface'}", QtxPopupMgr::ToggleRule );
}

// Module's engine IOR
QString ATOMSOLVGUI::engineIOR() const
{
  CORBA::String_var anIOR = getApp()->orb()->object_to_string( GetATOMSOLVGen() );
  return QString( anIOR.in() );
}

// Module's activation
bool ATOMSOLVGUI::activateModule( SUIT_Study* theStudy )
{
  bool bOk = SalomeApp_Module::activateModule( theStudy );

  setMenuShown( true );
  setToolShown( true );

  return bOk;
}

// Module's deactivation
bool ATOMSOLVGUI::deactivateModule( SUIT_Study* theStudy )
{
  setMenuShown( false );
  setToolShown( false );

  return SalomeApp_Module::deactivateModule( theStudy );
}

// Study closed callback
void ATOMSOLVGUI::studyClosed( SUIT_Study* theStudy )
{
  if ( theStudy ) {
    ATOMSOLV_ORB::ATOMSOLV_Gen_var engine = GetATOMSOLVGen();
    ATOMSOLV_ORB::TMoleculeList lst;
    lst.length( 0 );
    engine->setData( theStudy->id(), lst );
  }

  SalomeApp_Module::studyClosed( theStudy );
}

// Default windows
void ATOMSOLVGUI::windows( QMap<int, int>& theMap ) const
{
  theMap.insert( SalomeApp_Application::WT_ObjectBrowser, Qt::LeftDockWidgetArea );
#ifndef DISABLE_PYCONSOLE
  theMap.insert( SalomeApp_Application::WT_PyConsole,     Qt::BottomDockWidgetArea );
#endif
}

// Default view managers
void ATOMSOLVGUI::viewManagers( QStringList& theViewMgrs ) const
{
  theViewMgrs.append( SVTK_Viewer::Type() );
}

// Create custom data model
CAM_DataModel* ATOMSOLVGUI::createDataModel()
{
  return new ATOMSOLVGUI_DataModel( this );
}

// Create custom selection object
LightApp_Selection* ATOMSOLVGUI::createSelection() const
{
  return new ATOMSOLVGUI_Selection();
}

/*! Returns list of entities of selected objects. */
void ATOMSOLVGUI::selected( QStringList& entries, const bool multiple )
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
    if ( es.count() > 1 && es[ 0 ] == "ATOMSOLVGUI" && es[ 1 ] != "root" )
    {
      if ( !entries.contains( owner->entry() ) )
        entries.append( owner->entry() );
      if( !multiple )
        break;
    }
  }
}

// Action slot
void ATOMSOLVGUI::OnRetrieveData()
{
  ATOMSOLV_ORB::ATOMSOLV_Gen_var engine = GetATOMSOLVGen();
  SalomeApp_Application* app = getApp();
  if ( !CORBA::is_nil( engine ) && app ) {
    Engines::EngineComponent_var comp = app->lcc()->FindOrLoad_Component( "FactoryServerPy","ATOMGEN" );
    ATOMGEN_ORB::ATOMGEN_Gen_var atomGen = ATOMGEN_ORB::ATOMGEN_Gen::_narrow( comp );
    SalomeApp_Study* appStudy = dynamic_cast<SalomeApp_Study*>( app->activeStudy() );
    if ( !CORBA::is_nil( atomGen ) && appStudy ) {
      const int studyID = appStudy->id();
      // in case current study is not loaded by ATOMGEN component - call Load on it
      if ( _PTR( Study ) studyDS = appStudy->studyDS() ) {
        if ( _PTR( SComponent ) atomGenSComp = studyDS->FindComponent( "ATOMGEN" ) ) {
          _PTR( StudyBuilder ) builder = studyDS->NewBuilder();
          std::string atomGenIOR = app->orb()->object_to_string( atomGen );
          builder->LoadWith( atomGenSComp, atomGenIOR );
        }
      }

      // retrieve data from ATOMGEN
      ATOMGEN_ORB::MoleculeList_var inData = atomGen->getData( studyID );

      // "convert" Molecules to TMolecules, set default temperature of '0'
      const int n = inData->length();
      ATOMSOLV_ORB::TMoleculeList_var outData = new ATOMSOLV_ORB::TMoleculeList();
      outData->length( n );
      for ( int i = 0; i < n; i++ ) {
        ATOMSOLV_ORB::TMolecule_var tmol = new ATOMSOLV_ORB::TMolecule();
        tmol->molecule = ATOMGEN_ORB::Molecule::_duplicate( inData[i] );
        tmol->temperature = 0;
        outData[ i ] = tmol;
      }

      engine->setData( studyID, outData );

      app->updateObjectBrowser();
    }
  }
}

// Action slot
void ATOMSOLVGUI::OnProcessData()
{
  ATOMSOLV_ORB::ATOMSOLV_Gen_var engine = GetATOMSOLVGen();
  SalomeApp_Application* app = getApp();
  if ( !CORBA::is_nil( engine ) && app ) {
    if ( const int studyID = app->activeStudy()->id() > 0 ) {
      // call processData on engine
      engine->processData( studyID );
      // update data objects in object browser
      updateObjBrowser();
      // redisplay atoms that are already displayed (as their temperature changed..)
      ViewManagerList vmans;
      getApp()->viewManagers( SVTK_Viewer::Type(), vmans );
      for ( QList<SUIT_ViewManager*>::iterator it = vmans.begin(); it != vmans.end(); ++it ) {
        QVector<SUIT_ViewWindow*> views = (*it)->getViews();
        for ( int i = 0; i < views.size(); i++ ) {
          if ( SVTK_ViewWindow* svtkView = dynamic_cast<SVTK_ViewWindow*>( views[ i ] ) ) {
            vtkActorCollection* actors = svtkView->getRenderer()->GetActors();
            actors->InitTraversal();
            while ( vtkActor* actor = actors->GetNextActor() )
              if ( SALOME_Actor* salomeActor = SALOME_Actor::SafeDownCast( actor ) )
                if ( salomeActor->hasIO() ) {
                  //                  printf( " -- must redisplay actor salomeActor: %s\n", salomeActor->getIO()->getEntry() );
                  ATOMSOLVGUI_Displayer().updateActor( salomeActor );
                }
          }
        }
      }
    }
  }
}

// Action slot
void ATOMSOLVGUI::OnDisplayerCommand()
{
  const QObject* obj = sender();
  if ( obj && obj->inherits( "QAction" ) ) {
    const int id = actionId ( (QAction*)obj );
    switch ( id ) {
    case Display : {
      QStringList entries;
      selected ( entries, true );
      ATOMSOLVGUI_Displayer d;
      for ( QStringList::const_iterator it = entries.begin(), last = entries.end(); it != last; it++ )
        d.Display( it->toLatin1(), /*updateviewer=*/false, 0 );
      d.UpdateViewer();
    } break;
    case Erase   : {
      QStringList entries;
      selected ( entries, true );
      ATOMSOLVGUI_Displayer d;
      for ( QStringList::const_iterator it = entries.begin(), last = entries.end(); it != last; it++ )
        d.Erase( *it, /*forced=*/true, /*updateViewer=*/false, 0 );
      d.UpdateViewer();
    } break;
    case Shading   : {
      QStringList entries;
      selected ( entries, true );
      ATOMSOLVGUI_Displayer().setDisplayMode( entries, "Surface" );
    } break;
    case Wireframe   : {
      QStringList entries;
      selected ( entries, true );
      ATOMSOLVGUI_Displayer().setDisplayMode( entries, "Wireframe" );
    } break;
    case PointsMode   : {
      QStringList entries;
      selected ( entries, true );
      ATOMSOLVGUI_Displayer().setDisplayMode( entries, "Points" );
    } break;
    case Color   : {
      QStringList entries;
      selected ( entries, true );
      QColor initialColor( "white" );
      if ( entries.count() == 1 )
        initialColor = ATOMSOLVGUI_Displayer().getColor( entries[ 0 ] );
      QColor color = QColorDialog::getColor( initialColor, getApp()->desktop() );
      if ( color.isValid() )
        ATOMSOLVGUI_Displayer().setColor( entries, color );
    } break;
    case Transparency   : {
      QStringList entries;
      selected ( entries, true );
      ATOMSOLVGUI_TransparencyDlg( getApp()->desktop(), entries ).exec();
    } break;
    default: printf( "ERROR: Action with ID = %d was not found in ATOMSOLVGUI\n", id ); break;
    }
  }
}

// returns a custom displayer object
LightApp_Displayer* ATOMSOLVGUI::displayer()
{
  return new ATOMSOLVGUI_Displayer();
}

// create preferences for ATOMSOLV component
void ATOMSOLVGUI::createPreferences()
{
  int tabId = addPreference( tr( "ATOMSOLV_PREFERENCES" ) );
  int groupId = addPreference( tr( "PRESENTATION_PREF_GROUP" ), tabId );

  setPreferenceProperty( groupId, "columns", 1 );

  // Representation mode preference
  int dispModeId = addPreference( tr( "DISPLAY_MODE_PREF" ), groupId,
                                  LightApp_Preferences::Selector, "ATOMSOLV", "Representation" );
  QList<QVariant> intDispModes;
  QStringList strDispModes;
  intDispModes.append( 0 ); strDispModes.append( tr( "MEN_POINTSMODE" ) );
  intDispModes.append( 1 ); strDispModes.append( tr( "MEN_WIREFRAME" ) );
  intDispModes.append( 2 ); strDispModes.append( tr( "MEN_SHADING" ) );

  setPreferenceProperty( dispModeId, "strings", strDispModes );
  setPreferenceProperty( dispModeId, "indexes", intDispModes );

  // Radius preference
  int radisusId = addPreference( tr( "RADIUS_PREF" ), groupId,
                                 LightApp_Preferences::DblSpin, "ATOMSOLV", "Radius" );
  setPreferenceProperty( radisusId, "min", .001 );
  setPreferenceProperty( radisusId, "max", 1000 );
  setPreferenceProperty( radisusId, "precision", 3 );
}

// preferences of changed: update the corresponding values
void ATOMSOLVGUI::preferencesChanged( const QString& group, const QString& param )
{
  if ( group == "ATOMSOLV" && param == "Representation" )
    ATOMSOLVGUI_Displayer::setDefaultRepresentation( getApp()->resourceMgr()->integerValue( group, param ) );
  else if ( group == "ATOMSOLV" && param == "Radius" )
    ATOMSOLVGUI_Displayer::setDefaultRadius( getApp()->resourceMgr()->doubleValue( group, param ) );
}


// Export the module
extern "C" {
  ATOMSOLVGUI_EXPORT
  CAM_Module* createModule()
  {
    return new ATOMSOLVGUI();
  }
  ATOMSOLVGUI_EXPORT
  char* getModuleVersion() 
  {
    return (char*)ATOMSOLV_VERSION_STR;
  }
}
