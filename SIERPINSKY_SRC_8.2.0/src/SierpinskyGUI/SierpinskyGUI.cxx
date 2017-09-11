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

///////////////////////////////////////////////////////////
// File    : SierpinskyGUI.cxx
// Author  : Vadim SANDLER (OCN)
// Created : 13/07/05
///////////////////////////////////////////////////////////

#include "SierpinskyGUI.h"

#include <SIERPINSKY_version.h>

#include <SUIT_MessageBox.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Desktop.h>
#include <SalomeApp_Application.h>
#include <SVTK_ViewModel.h>
#include <SALOME_LifeCycleCORBA.hxx>
#include <SierpinskyGUI_RunDlg.h>
#include <utilities.h>

/*!
 * Constructor
 */
SierpinskyGUI::SierpinskyGUI() : 
  SalomeApp_Module("Sierpinsky")
{
}

/*! 
 * Gets an reference to the module's engine
 */
SIERPINSKY_ORB::SIERPINSKY_ptr SierpinskyGUI::InitSIERPINSKYGen( SalomeApp_Application* app )
{
  SIERPINSKY_ORB::SIERPINSKY_ptr clr;
  try{
    Engines::EngineComponent_var comp = app->lcc()->FindOrLoad_Component( "FactoryServer","SIERPINSKY" );
    clr = SIERPINSKY_ORB::SIERPINSKY::_narrow(comp);
  }
  catch (CORBA::Exception&){
    MESSAGE("Caught CORBA::Exception.");
  }
  catch (...){
    MESSAGE("Caught unknown exception.");
  }
  return clr;
}

/*!
 * Module's initialization
 */
void SierpinskyGUI::initialize( CAM_Application* app )
{
  SalomeApp_Module::initialize( app );

  InitSIERPINSKYGen( dynamic_cast<SalomeApp_Application*>( app ) );

  QWidget* aParent = app->desktop();
  SUIT_ResourceMgr* aResourceMgr = app->resourceMgr();

  // create actions
  QPixmap aPixmap = aResourceMgr->loadPixmap( "SIERPINSKY",tr( "ICON_RUN_DLG" ) );
  createAction( 901, tr( "Run Dialog" ), QIcon(aPixmap), tr( "RUN" ), tr( "CAPTION" ), 0, aParent, false,
		this, SLOT( OnRun() ) );

  // create menus
  int aMenuId;
  aMenuId = createMenu( tr( "MENU_Sierpinsky" ), -1, -1, 30 );
  createMenu( 901, aMenuId, 10 );

  // create toolbars
  int aToolId = createTool ( tr( "TOOL_Sierpinsky" ) );
  createTool( 901, aToolId );
}

/*!
 * Module's engine IOR
 */
QString SierpinskyGUI::engineIOR() const
{
  CORBA::String_var anIOR = getApp()->orb()->object_to_string( InitSIERPINSKYGen( getApp() ) );
  return QString( anIOR.in() );
}

/*!
 * Required dockable windows
 */
void SierpinskyGUI::windows( QMap<int, int>& wmap ) const
{
  wmap.insert( SalomeApp_Application::WT_ObjectBrowser, Qt::LeftDockWidgetArea );
}

/*!
 * Required viewers list
 */
void SierpinskyGUI::viewManagers( QStringList& vmlist ) const
{
  vmlist.append( SVTK_Viewer::Type() );
}

/*!
 * Module's activation
 */
bool SierpinskyGUI::activateModule( SUIT_Study* theStudy )
{
  bool bOk = SalomeApp_Module::activateModule( theStudy );

  setMenuShown(true);
  setToolShown(true);

  return bOk;
}

/*!
 * Module's deactivation
 */
bool SierpinskyGUI::deactivateModule( SUIT_Study* theStudy )
{
  setMenuShown(false);
  setToolShown(false);

  return SalomeApp_Module::deactivateModule( theStudy );
}

/*!
 * Destructor
 */
SierpinskyGUI::~SierpinskyGUI()
{
}


// Action slot
void SierpinskyGUI::OnRun()
{
  QWidget* aParent = getApp()->desktop();
  SierpinskyGUI_RunDlg* aDlg = new SierpinskyGUI_RunDlg( aParent, this );
  aDlg->exec();
}

extern "C" {
  SIERPINSKY_EXPORT
  CAM_Module* createModule()
  {
    return new SierpinskyGUI();
  }
  SIERPINSKY_EXPORT
  char* getModuleVersion()
  {
    return (char*)SIERPINSKY_VERSION_STR;
  }
}
