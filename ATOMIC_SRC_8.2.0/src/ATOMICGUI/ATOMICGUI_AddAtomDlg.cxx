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

#include "ATOMICGUI_AddAtomDlg.h"
#include <qlayout.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <QtxDoubleSpinBox.h>

#define DBL_MAX 1000.0

/*! Constructor */
ATOMICGUI_AddAtomDlg::ATOMICGUI_AddAtomDlg( QWidget* parent )
: LightApp_Dialog( parent, 0, false, true, OK | Apply | Close )
{
  setWindowTitle( tr( "CAPTION" ) );

  QGridLayout* main = new QGridLayout( mainFrame() );

  QLabel* xn = new QLabel( tr( "ATOMIC_ATOM" ) + ":", mainFrame() );
  myName = new QLineEdit( mainFrame() );

  QLabel* xl = new QLabel( "X:", mainFrame() );
  myX = new QtxDoubleSpinBox( -DBL_MAX, DBL_MAX, 1, mainFrame() );
  myX->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

  QLabel* yl = new QLabel( "Y:", mainFrame() );
  myY = new QtxDoubleSpinBox( -DBL_MAX, DBL_MAX, 1, mainFrame() );
  myY->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

  QLabel* zl = new QLabel( "Z:", mainFrame() );
  myZ = new QtxDoubleSpinBox( -DBL_MAX, DBL_MAX, 1, mainFrame() );
  myZ->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

  main->addWidget( xn,      0, 0 );
  main->addWidget( myName,  0, 1 );
  main->addWidget( xl,  1, 0 );
  main->addWidget( myX, 1, 1 );
  main->addWidget( yl,  2, 0 );
  main->addWidget( myY, 2, 1 );
  main->addWidget( zl,  3, 0 );
  main->addWidget( myZ, 3, 1 );

  setButtonPosition( Right, Close );

  myX->setValue( 0 );
  myY->setValue( 0 );
  myZ->setValue( 0 );
}

/*! Destructor */
ATOMICGUI_AddAtomDlg::~ATOMICGUI_AddAtomDlg()
{
}

/*! Validates data. */
bool ATOMICGUI_AddAtomDlg::acceptData( const QStringList& selected ) const
{
  // check selection
  if( selected.count()!=1 )
    return false;

  // check entered data
  return !myName->text().toLatin1().trimmed().isEmpty();
}

/*! Returns values of dialog widgets. */
void ATOMICGUI_AddAtomDlg::data( QString& name, double& x, double& y, double& z ) const
{
  name = myName->text().toLatin1().trimmed();
  x = myX->value();
  y = myY->value();
  z = myZ->value();
}
