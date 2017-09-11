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

#if !defined(ATOMSOLVGUI_DATAMODEL_H)
#define ATOMSOLVGUI_DATAMODEL_H

#include "ATOMSOLVGUI_ATOMSOLVGUI.hxx"

#include <LightApp_DataModel.h>

class CAM_Module;

/*!
 * Class       : ATOMSOLVGUI_DataModel
 * Description : Data Model of ATOMIC component
 */
class ATOMSOLVGUI_EXPORT ATOMSOLVGUI_DataModel : public LightApp_DataModel
{
public:
  ATOMSOLVGUI_DataModel ( CAM_Module* );
  virtual ~ATOMSOLVGUI_DataModel();

protected:
  virtual void          build();
};

#endif // ATOMSOLVGUI_DATAMODEL_H
