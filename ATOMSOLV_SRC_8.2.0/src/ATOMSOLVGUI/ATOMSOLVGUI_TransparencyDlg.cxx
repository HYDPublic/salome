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

#include "ATOMSOLVGUI_TransparencyDlg.h"
#include "ATOMSOLVGUI_Displayer.h"

#include <qframe.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qapplication.h>
#include <qgroupbox.h>


ATOMSOLVGUI_TransparencyDlg::ATOMSOLVGUI_TransparencyDlg( QWidget* parent, const QStringList& entries )
  :QDialog( parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint )
{
  myEntries = entries;

  resize(152, 107); 
  setWindowTitle(tr("TRANSPARENCY_DLG_TITLE"));
  setSizeGripEnabled(true);
  QGridLayout* lay = new QGridLayout(this); 
  lay->setSpacing(6);
  lay->setMargin(11);

  /*************************************************************************/
  QGroupBox* GroupButtons = new QGroupBox( this );
  QGridLayout* GroupButtonsLayout = new QGridLayout( GroupButtons );
  GroupButtonsLayout->setAlignment( Qt::AlignTop );
  GroupButtonsLayout->setSpacing( 6 );
  GroupButtonsLayout->setMargin( 11 );
  
  QPushButton* buttonOk = new QPushButton( GroupButtons );
  buttonOk->setText( tr( "BUT_OK" ) );
  buttonOk->setAutoDefault( true );
  buttonOk->setDefault( true );

  QPushButton* buttonHelp = new QPushButton( GroupButtons );
  buttonHelp->setText( tr( "BUT_HELP" ) );
  buttonHelp->setAutoDefault( true );
  buttonHelp->setDefault( true );
  
  GroupButtonsLayout->addWidget( buttonOk, 0, 0 );
  GroupButtonsLayout->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 1 );
  GroupButtonsLayout->addWidget( buttonHelp, 0, 2 );

  /*************************************************************************/
  QGroupBox* GroupC1 = new QGroupBox( this );
  QGridLayout* GroupC1Layout = new QGridLayout( GroupC1 );
  GroupC1Layout->setAlignment( Qt::AlignTop );
  GroupC1Layout->setSpacing( 6 );
  GroupC1Layout->setMargin( 11 );
  GroupC1Layout->setColumnMinimumWidth(1, 100);
  
  QLabel* TextLabelOpaque = new QLabel( GroupC1 );
  TextLabelOpaque->setText( tr( "TRANSPARENCY_TRANSPARENT"  ) );
  TextLabelOpaque->setAlignment( Qt::AlignLeft );
  GroupC1Layout->addWidget( TextLabelOpaque, 0, 0 );
  GroupC1Layout->addItem( new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum ), 0, 1 );
  
  QLabel* TextLabelTransparent = new QLabel( GroupC1 );
  TextLabelTransparent->setText( tr( "TRANSPARENCY_OPAQUE"  ) );
  TextLabelTransparent->setAlignment( Qt::AlignRight );
  GroupC1Layout->addWidget( TextLabelTransparent, 0, 2 );
  
  mySlider = new QSlider( Qt::Horizontal, GroupC1 );
  mySlider->setMinimum( 0 );
  mySlider->setMaximum( 10 );
  GroupC1Layout->addWidget( mySlider, 0, 1 );
  /*************************************************************************/
  
  lay->addWidget(GroupC1, 0,  0);
  lay->addWidget(GroupButtons, 1, 0);
    
  if ( myEntries.count() == 1 ) {
    int value = (int)(ATOMSOLVGUI_Displayer().getTransparency( myEntries[ 0 ] ) * 10.);
    if ( value > 0 )
      mySlider->setValue( value );
  }
  ValueHasChanged(mySlider->value());

  // signals and slots connections : after ValueHasChanged()
  connect(buttonOk, SIGNAL(clicked()), this, SLOT(accept()));
  connect(buttonHelp, SIGNAL(clicked()), this, SLOT(ClickOnHelp()));
  connect(mySlider, SIGNAL(valueChanged(int)), this, SLOT(ValueHasChanged(int)));
}

//=================================================================================
// function : ClickOnHelp()
// purpose  :
//=================================================================================
void ATOMSOLVGUI_TransparencyDlg::ClickOnHelp()
{
  // display a help in browser for example..
  // not implemented yet..
}

//=================================================================================
// function : ValueHasChanged()
// purpose  : Called when value of slider change
//          : or the first time as initilisation
//=================================================================================
void ATOMSOLVGUI_TransparencyDlg::ValueHasChanged( int newValue )
{
  ATOMSOLVGUI_Displayer().setTransparency( myEntries, (float)mySlider->value() * .1 );
}
