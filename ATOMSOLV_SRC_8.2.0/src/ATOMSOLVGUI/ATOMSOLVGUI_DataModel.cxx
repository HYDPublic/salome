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

#include "ATOMSOLVGUI_DataModel.h"
#include "ATOMSOLVGUI_DataObject.h"
#include "ATOMSOLVGUI.h"

#include <LightApp_Study.h>

#include <CAM_Module.h>
#include <CAM_Application.h>

/*! Constructor */
ATOMSOLVGUI_DataModel::ATOMSOLVGUI_DataModel( CAM_Module* module )
  : LightApp_DataModel( module )
{
}

/*! Destructor */
ATOMSOLVGUI_DataModel::~ATOMSOLVGUI_DataModel()
{
}

/*! Called on update of the structure of Data Objects */
void ATOMSOLVGUI_DataModel::build()
{
  ATOMSOLVGUI_ModuleObject* modelRoot = dynamic_cast<ATOMSOLVGUI_ModuleObject*>( root() );
  if( !modelRoot )  {  // root is not set yet
    modelRoot = new ATOMSOLVGUI_ModuleObject( this, 0 );
    setRoot( modelRoot );
  }

  // create 'molecule' objects under model root object and 'atom' objects under 'molecule'-s
  ATOMSOLV_ORB::ATOMSOLV_Gen_var engine = ATOMSOLVGUI::GetATOMSOLVGen();
  if ( !CORBA::is_nil( engine ) ) {
    const int studyID = getStudy()->id();
    ATOMSOLV_ORB::TMoleculeList_var molecules;
    if ( !engine->getData( studyID, molecules ) ) {
      //      printf( " -- DataModel: getData() returned false! \n" );
      return;
    }
    for ( int i = 0, n = molecules->length(); i < n; i++ ) {
      ATOMSOLVGUI_DataObject* molDO = new ATOMSOLVGUI_DataObject ( modelRoot, i );
      const ATOMSOLV_ORB::TMolecule& mol = molecules[i];
      const int atoms = mol.molecule->getNbAtoms();
      for ( int j = 0; j < atoms; j++ )
	new ATOMSOLVGUI_DataObject ( molDO, i, j );
    }
  }

  //  root()->dump();
}
