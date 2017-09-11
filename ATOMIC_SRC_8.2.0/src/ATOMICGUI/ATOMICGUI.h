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

#if !defined(ATOMICGUI_H)
#define ATOMICGUI_H

#include "ATOMICGUI_ATOMICGUI.hxx"

#include <LightApp_Module.h>

/*!
 * Class       : ATOMICGUI
 * Description : GUI module class for ATOMIC component
 */
class ATOMICGUI_EXPORT ATOMICGUI: public LightApp_Module
{
  Q_OBJECT
  enum { agCreateMol,  
	 agAddAtom, 
	 agRename, 
	 agDelete,
         agImportXML,
         agExportXML };

public:
  ATOMICGUI();

  virtual void           initialize ( CAM_Application* );
  virtual QString        iconName () const;

  virtual void           windows ( QMap<int, int>& ) const;

  void                   selected( QStringList&, const bool );
  
protected:
  virtual CAM_DataModel*      createDataModel();
  virtual LightApp_Selection* createSelection() const;
  virtual LightApp_Operation* createOperation( const int ) const;

public slots:
  virtual bool           activateModule   ( SUIT_Study* );
  virtual bool           deactivateModule ( SUIT_Study* );

private slots:
  void                   onOperation();
};

#endif // ATOMICGUI_H
