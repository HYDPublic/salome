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

#include "ATOMICGUI_ImportExportOp.h"
#include "ATOMICGUI_DataModel.h"

#include <LightApp_Module.h>
#include <LightApp_Application.h>

#include <SUIT_MessageBox.h>
#include <SUIT_Desktop.h>

/*! Constructor */
ATOMICGUI_ImportExportOp::ATOMICGUI_ImportExportOp( const bool import )
: ATOMICGUI_Operation(),
  myIsImport( import )
{
}

/*! Destructor */
ATOMICGUI_ImportExportOp::~ATOMICGUI_ImportExportOp()
{
}

/*! Operation itself. Imports or exports the data structure 
 *  calling the corresponing methods of Data Model. */
void ATOMICGUI_ImportExportOp::startOperation()
{
  ATOMICGUI_DataModel* dm = dataModel();
  if ( !dm )
  {
    abort();
    return;
  }

  QStringList filtersList;
  filtersList.append( tr( "XML_FILES" ) );

  // Select a file to be imported
  QString aFileName = module()->getApp()->getFileName( myIsImport, QString::null,
						       filtersList.join( ";;" ),
						       tr( myIsImport ? "ATOMICGUI_IMPORT_XML" : "ATOMICGUI_EXPORT_XML" ), 0 );

  if( !aFileName.isEmpty() )
  {
    if( ( myIsImport && dm->importFile( aFileName ) ) ||
        ( !myIsImport && dm->exportFile( aFileName ) ) )
    {
      commit();
      return;
    }
    else
      SUIT_MessageBox::warning( application()->desktop(),
			        tr( "WRN_WARNING" ),
			        tr( myIsImport ? "WRN_IMPORT_FAILED" : "WRN_EXPORT_FAILED" ),
			        tr( "BUT_OK" ) );
  }
  abort();
}
