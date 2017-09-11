// Copyright (C) 2007-2016  CEA/DEN, EDF R&D
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

// ---
// File   : GHS3DPRLPluginGUI_HypothesisCreator.cxx
// Author : Christian VAN WAMBEKE (CEA) (from Hexotic plugin Lioka RAZAFINDRAZAKA)
// ---
//
#include "GHS3DPRLPluginGUI_HypothesisCreator.h"

#include <SMESHGUI_Utils.h>
#include <SMESHGUI_HypothesesUtils.h>
#include <SMESH_AdvOptionsWdg.h>

#include CORBA_SERVER_HEADER(GHS3DPRLPlugin_Algorithm)

#include <SUIT_Session.h>
#include <SUIT_ResourceMgr.h>
#include <SalomeApp_Tools.h>
#include <QtxIntSpinBox.h>
#include <QtxDoubleSpinBox.h>

#include <QCheckBox>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QTabWidget>
#include <QVBoxLayout>

GHS3DPRLPluginGUI_HypothesisCreator::GHS3DPRLPluginGUI_HypothesisCreator( const QString& theHypType )
: SMESHGUI_GenericHypothesisCreator( theHypType ),
  myIs3D( true )
{
  //printf("Hypo creator GHS3DPRLPlugin !!!!!!!!!!!!!!!!!!!!!!!!!! RNV");
}

GHS3DPRLPluginGUI_HypothesisCreator::~GHS3DPRLPluginGUI_HypothesisCreator()
{
}

bool GHS3DPRLPluginGUI_HypothesisCreator::checkParams() const
{
  GHS3DPRLHypothesisData data_old, data_new;
  readParamsFromHypo( data_old );
  readParamsFromWidgets( data_new );
  bool res = storeParamsToHypo( data_new );
  storeParamsToHypo( data_old );
  return res;
}

QFrame* GHS3DPRLPluginGUI_HypothesisCreator::buildFrame()
{
  QFrame* fr = new QFrame( 0 );
  QVBoxLayout* lay = new QVBoxLayout( fr );
  lay->setMargin( 5 );
  lay->setSpacing( 0 );

  // tab
  QTabWidget* tab = new QTabWidget( fr );
  tab->setTabShape( QTabWidget::Rounded );
  tab->setTabPosition( QTabWidget::North );
  lay->addWidget( tab );

  // basic parameters
  QWidget* GroupC1 = new QWidget();
  lay->addWidget( GroupC1 );

  QGridLayout* l = new QGridLayout( GroupC1 );
  l->setSpacing( 6 );
  l->setMargin( 11 );

  int row = 0;
  myName = 0;
  if( isCreation() ) {
    QLabel* aNameLab = new QLabel( tr( "SMESH_NAME" ), GroupC1 );
    aNameLab->setWhatsThis( tr( "GHS3DPRL_WhatsThis_Name" ) );
    l->addWidget( aNameLab, row, 0, 1, 1 );
    myName = new QLineEdit( GroupC1 );
    l->addWidget( myName, row++, 1, 1, 1 );
  }

  //GHS3DPRLPlugin::GHS3DPRLPlugin_Hypothesis_var h =
  //GHS3DPRLPlugin::GHS3DPRLPlugin_Hypothesis::_narrow( initParamsHypothesis() );

  QLabel* aMEDNameText = new QLabel( tr( "GHS3DPRL_MEDName" ), GroupC1 );
  aMEDNameText->setWhatsThis( tr( "GHS3DPRL_WhatsThis_MEDName" ) );
  l->addWidget( aMEDNameText, row, 0, 1, 1 );
  myMEDName = new QLineEdit( GroupC1 );
  l->addWidget( myMEDName, row++, 1, 1, 1 );

  QLabel* aNbPartText = new QLabel( tr( "GHS3DPRL_NbPart" ), GroupC1 );
  aNbPartText->setWhatsThis( tr( "GHS3DPRL_WhatsThis_NbPart" ) );
  l->addWidget( aNbPartText, row, 0, 1, 1 );
  myNbPart = new QtxIntSpinBox( GroupC1 );
  l->addWidget( myNbPart, row++, 1, 1, 1 );
  myNbPart->setMinimum( 1 );
  myNbPart->setMaximum( 256 );
  myNbPart->setSingleStep( 1 );

  myKeepFiles = new QCheckBox( tr( "GHS3DPRL_KeepFiles" ), GroupC1 );
  myKeepFiles->setWhatsThis( tr( "GHS3DPRL_WhatsThis_KeepFiles" ) );
  l->addWidget( myKeepFiles, row++, 0, 1, 2 );
  
  myBackground = new QCheckBox( tr( "GHS3DPRL_Background" ), GroupC1 );
  myBackground->setWhatsThis( tr( "GHS3DPRL_WhatsThis_Background" ) );
  l->addWidget( myBackground, row++, 0, 1, 2 );

  myMultithread = new QCheckBox( tr( "GHS3DPRL_Multithread" ), GroupC1 );
  myMultithread->setWhatsThis( tr( "GHS3DPRL_WhatsThis_Multithread" ) );
  l->addWidget( myMultithread, row++, 0, 1, 2 );

  //myToMeshHoles = new QCheckBox( tr( "GHS3DPRL_ToMeshHoles" ), GroupC1 );
  //myToMeshHoles->setWhatsThis( tr( "GHS3DPRL_WhatsThis_ToMeshHoles" ) );
  //l->addWidget( myToMeshHoles, row++, 0, 1, 2 );
  
  //myToMergeSubdomains = new QCheckBox( tr( "GHS3DPRL_ToMergeSubdomains" ), GroupC1 );
  //myToMergeSubdomains->setWhatsThis( tr( "GHS3DPRL_WhatsThis_ToMergeSubdomains" ) );
  //l->addWidget( myToMergeSubdomains, row++, 0, 1, 2 );

  QLabel* aGradationText = new QLabel( tr( "GHS3DPRL_Gradation" ), GroupC1 );
  aGradationText->setWhatsThis( tr( "GHS3DPRL_WhatsThis_Gradation" ) );
  l->addWidget( aGradationText, row, 0, 1, 1 );
  myGradation = new QtxDoubleSpinBox( GroupC1 );
  l->addWidget( myGradation, row++, 1, 1, 1 );
  myGradation->setMinimum( 0 );
  myGradation->setMaximum( 3 );
  myGradation->setSingleStep( 0.01 );

  QLabel* aMinSizeText = new QLabel( tr( "GHS3DPRL_MinSize" ), GroupC1 );
  aMinSizeText->setWhatsThis( tr( "GHS3DPRL_WhatsThis_MinSize" ) );
  l->addWidget( aMinSizeText, row, 0, 1, 1 );
  myMinSize = new QtxDoubleSpinBox( GroupC1 );
  l->addWidget( myMinSize, row++, 1, 1, 1 );
  myMinSize->setMinimum( 0 );
  //myMinSize->setMaximum( 999999 );
  //myMinSize->setSingleStep( 1 );

  QLabel* aMaxSizeText = new QLabel( tr( "GHS3DPRL_MaxSize" ), GroupC1 );
  aMaxSizeText->setWhatsThis( tr( "GHS3DPRL_WhatsThis_MaxSize" ) );
  l->addWidget( aMaxSizeText, row, 0, 1, 1 );
  myMaxSize = new QtxDoubleSpinBox( GroupC1 );
  l->addWidget( myMaxSize, row++, 1, 1, 1 );
  myMaxSize->setMinimum( 0 );
  //myMaxSize->setMaximum( 999999 );
  //myMaxSize->setSingleStep( 1 );

  myIs3D = true;

  myAdvTable = new SMESH_AdvOptionsWdg( fr );

  // add tabs
  tab->insertTab( 0, GroupC1, tr( "SMESH_ARGUMENTS" ));
  tab->insertTab( 1, myAdvTable, tr( "SMESH_ADVANCED" ));

  return fr;
}

void GHS3DPRLPluginGUI_HypothesisCreator::retrieveParams() const
{
  GHS3DPRLHypothesisData data;
  readParamsFromHypo( data );

  if ( myName ) myName->setText( data.myName );
  myMEDName->setText( data.myMEDName );
  myNbPart->setValue( data.myNbPart );
  myKeepFiles->setChecked( data.myKeepFiles );
  myBackground->setChecked( data.myBackground );
  myMultithread->setChecked( data.myMultithread );
  //myToMeshHoles->setChecked( data.myToMeshHoles );
  //myToMergeSubdomains->setChecked( data.myToMergeSubdomains );
  myGradation->setValue( data.myGradation );
  myMinSize->setValue( data.myMinSize );
  myMaxSize->setValue( data.myMaxSize );
  myAdvTable->SetCustomOptions( data.myAdvOptions );
}

QString GHS3DPRLPluginGUI_HypothesisCreator::storeParams() const
{
  GHS3DPRLHypothesisData data;
  readParamsFromWidgets( data );
  storeParamsToHypo( data );

  QString valStr;
  valStr += tr( "GHS3DPRL_MEDName" ) + " = " + data.myMEDName + "; ";
  valStr += tr( "GHS3DPRL_NbPart" ) + " = " + QString::number( data.myNbPart )   + "; ";
  //valStr += tr( "GHS3DPRL_ToMeshHoles" )  + " = " + QString::number( data.myToMeshHoles ) + "; ";
  //valStr += tr( "GHS3DPRL_ToMergeSubdomains" )  + " = " + QString::number( data.myToMergeSubdomains ) + "; ";
  valStr += tr( "GHS3DPRL_Gradation" )  + " = " + QString::number( data.myGradation ) + "; ";
  valStr += tr( "GHS3DPRL_MinSize" )  + " = " + QString::number( data.myMinSize ) + "; ";
  valStr += tr( "GHS3DPRL_MaxSize" )  + " = " + QString::number( data.myMaxSize ) + "; ";
  valStr += tr( "GHS3DPRL_KeepFiles" )  + " = " + QString::number( data.myKeepFiles ) + "; ";
  valStr += tr( "GHS3DPRL_Background" )  + " = " + QString::number( data.myBackground ) + "; ";
  valStr += tr( "GHS3DPRL_Multithread" )  + " = " + QString::number( data.myMultithread ) + "; ";

  return valStr;
}

bool GHS3DPRLPluginGUI_HypothesisCreator::readParamsFromHypo( GHS3DPRLHypothesisData& h_data ) const
{
  GHS3DPRLPlugin::GHS3DPRLPlugin_Hypothesis_var h =
    GHS3DPRLPlugin::GHS3DPRLPlugin_Hypothesis::_narrow( initParamsHypothesis() );

  HypothesisData* data = SMESH::GetHypothesisData( hypType() );
  h_data.myName        = isCreation() && data ? hypName() : "";
  h_data.myMEDName     = SMESH::toQStr( h->GetMEDName() ); //"DOMAIN\0";
  h_data.myNbPart      = h->GetNbPart();
  h_data.myKeepFiles   = h->GetKeepFiles();
  h_data.myBackground  = h->GetBackground();
  h_data.myMultithread = h->GetMultithread();
  h_data.myGradation   = h->GetGradation();
  h_data.myMinSize     = h->GetMinSize();
  h_data.myMaxSize     = h->GetMaxSize();
  h_data.myAdvOptions  = SMESH::toQStr( h->GetAdvancedOption() );

  return true;
}

bool GHS3DPRLPluginGUI_HypothesisCreator::storeParamsToHypo( const GHS3DPRLHypothesisData& h_data ) const
{
  GHS3DPRLPlugin::GHS3DPRLPlugin_Hypothesis_var h =
    GHS3DPRLPlugin::GHS3DPRLPlugin_Hypothesis::_narrow( hypothesis() );

  bool ok = true;
  try
  {
    if ( isCreation() )
      SMESH::SetName( SMESH::FindSObject( h ), h_data.myName.toLatin1().constData() );

    h->SetMEDName       ( h_data.myMEDName.toLatin1().constData() );
    h->SetNbPart        ( h_data.myNbPart );
    h->SetKeepFiles     ( h_data.myKeepFiles );
    h->SetBackground    ( h_data.myBackground );
    h->SetMultithread   ( h_data.myMultithread );
    h->SetGradation     ( h_data.myGradation );
    h->SetMinSize       ( h_data.myMinSize );
    h->SetMaxSize       ( h_data.myMaxSize );
    h->SetAdvancedOption( h_data.myAdvOptions.toLatin1().constData() );
  }
  catch ( const SALOME::SALOME_Exception& ex )
  {
    SalomeApp_Tools::QtCatchCorbaException( ex );
    ok = false;
  }
  return ok;
}

bool GHS3DPRLPluginGUI_HypothesisCreator::readParamsFromWidgets( GHS3DPRLHypothesisData& h_data ) const
{
  h_data.myName       = myName ? myName->text() : "";
  h_data.myMEDName    = myMEDName->text().simplified().replace(' ', '_');
  h_data.myNbPart     = myNbPart->value();
  h_data.myKeepFiles  = myKeepFiles->isChecked();
  h_data.myBackground = myBackground->isChecked();
  h_data.myMultithread= myMultithread->isChecked();
  h_data.myGradation  = myGradation->value();
  h_data.myMinSize    = myMinSize->value();
  h_data.myMaxSize    = myMaxSize->value();
  h_data.myAdvOptions = myAdvTable->GetCustomOptions();
  return true;
}

QString GHS3DPRLPluginGUI_HypothesisCreator::caption() const
{
  return myIs3D ? tr( "GHS3DPRL_3D_TITLE" ) : tr( "GHS3DPRL_3D_TITLE" ); // ??? 3D/2D ???
}

QPixmap GHS3DPRLPluginGUI_HypothesisCreator::icon() const
{
  QString hypIconName = myIs3D ? tr( "ICON_DLG_GHS3DPRL_PARAMETERS" ) : tr( "ICON_DLG_GHS3DPRL_PARAMETERS" ); // ??? 3D/2D ???
  return SUIT_Session::session()->resourceMgr()->loadPixmap( "GHS3DPRLPlugin", hypIconName );
}

QString GHS3DPRLPluginGUI_HypothesisCreator::type() const
{
  return myIs3D ? tr( "GHS3DPRL_3D_HYPOTHESIS" ): tr( "GHS3DPRL_3D_HYPOTHESIS" ); // ??? 3D/2D ???
}

QString GHS3DPRLPluginGUI_HypothesisCreator::helpPage() const
{
  return "ghs3dprl_hypo_page.html";
}

