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

#if !defined(ATOMICGUI_DATAMODEL_H)
#define ATOMICGUI_DATAMODEL_H

#include "ATOMICGUI_ATOMICGUI.hxx"

#include "ATOMICGUI_Data.h"
#include <LightApp_DataModel.h>
#include <qlist.h>

class CAM_Module;
class LightApp_Study;
class LightApp_DataObject;
class ATOMICGUI_DataObject;

/*!
 * Class       : ATOMICGUI_DataModel
 * Description : Data Model of ATOMIC component
 */
class ATOMICGUI_EXPORT ATOMICGUI_DataModel : public LightApp_DataModel
{
  Q_OBJECT

public:
  ATOMICGUI_DataModel ( CAM_Module* );
  virtual ~ATOMICGUI_DataModel();

  virtual bool          open   ( const QString&, CAM_Study*, QStringList );
  virtual bool          save   ( QStringList& );
  virtual bool          saveAs ( const QString&, CAM_Study*, QStringList& );
  virtual bool          close  ();
  virtual bool          create ( CAM_Study* );

  virtual bool          isModified () const;
  virtual bool          isSaved    () const;
  virtual bool          dumpPython( const QString&, CAM_Study*, bool, QStringList& );

  bool                  importFile ( const QString&, CAM_Study* = 0 );
  bool                  exportFile ( const QString& = QString::null );
  QString               createMolecule ();
  bool                  addAtom    ( const QString&, const QString&, 
				     const double, const double, const double );
  bool                  renameObj  ( const QString&, const QString& );
  bool                  deleteObjs ( const QStringList& );

  ATOMICGUI_DataObject* findObject   ( const QString& ); 
  ATOMICGUI_DataObject* findMolecule ( const QString& ); 

protected:
  virtual void          build();

private:
  QString                              myStudyURL;
  QList<ATOMICGUI_AtomicMolecule>      myMolecules;
};

#endif // ATOMICGUI_DATAMODEL_H
