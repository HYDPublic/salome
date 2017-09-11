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

#ifndef TRANSPARENCYDLG_H
#define TRANSPARENCYDLG_H

#include "ATOMSOLVGUI_ATOMSOLVGUI.hxx"

#include <qdialog.h>
#include <qstringlist.h>

class QSlider;

//=================================================================================
// class    : ATOMSOLVGUI_TransparencyDlg
// purpose  : dialog box that allows to modify transparency of displayed objects
//=================================================================================
class ATOMSOLVGUI_EXPORT ATOMSOLVGUI_TransparencyDlg : public QDialog
{ 
    Q_OBJECT

public:
    ATOMSOLVGUI_TransparencyDlg( QWidget* parent, const QStringList& entries );

private:
    QSlider*  mySlider; 
    QStringList myEntries;
    
private slots: 
  void ClickOnHelp();
  void ValueHasChanged( int ) ;
    
};

#endif // TRANSPARENCYDLG_H

