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
// File    : SierpinskyGUI_RunDlg.cxx
// Author  : Vadim SANDLER (OCN)
// Created : 13/07/05
///////////////////////////////////////////////////////////

#include "SierpinskyGUI_RunDlg.h"
#include "SierpinskyGUI.h"
#include "SALOME_LifeCycleCORBA.hxx"
#include "SalomeApp_Application.h"

#include <QLabel>
#include <QLineEdit>
#include <QLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QThread>
#include <QProgressBar>
#include <QApplication>

#include <SUIT_Application.h>
#include <SUIT_Desktop.h>
#include <SUIT_FileDlg.h>
#include <QtxDoubleSpinBox.h>
#include <SUIT_MessageBox.h>

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(Sierpinsky)
#include CORBA_CLIENT_HEADER(Randomizer)
#include CORBA_CLIENT_HEADER(SALOMEDS)
#ifdef WITH_VISU
#include CORBA_CLIENT_HEADER(VISU_Gen)
#endif

#include <utilities.h>

/*!
 * Get Randomizer component engine
 */
static RANDOMIZER_ORB::RANDOMIZER_var getRandomizerEngine()
{
  static RANDOMIZER_ORB::RANDOMIZER_var aGen;
  if( CORBA::is_nil( aGen ) ) {
    try{
      SALOME_LifeCycleCORBA aLCC(SalomeApp_Application::namingService());
      Engines::EngineComponent_var aComponent = aLCC.FindOrLoad_Component("FactoryServerPy","RANDOMIZER");
      aGen = RANDOMIZER_ORB::RANDOMIZER::_narrow( aComponent );
    }
    catch (CORBA::Exception&){
      MESSAGE("Caught CORBA::Exception.");
    }
    catch (...){
      MESSAGE("Caught unknown exception.");
    }
  }
  return aGen;
}

/*!
 * Get Sierpinsky component engine
 */
static SIERPINSKY_ORB::SIERPINSKY_var getSierpinskyEngine()
{
  static SIERPINSKY_ORB::SIERPINSKY_var aGen;
  if( CORBA::is_nil( aGen ) ) {
    try{
      SALOME_LifeCycleCORBA aLCC(SalomeApp_Application::namingService());
      Engines::EngineComponent_var aComponent = aLCC.FindOrLoad_Component("FactoryServer","SIERPINSKY");
      aGen = SIERPINSKY_ORB::SIERPINSKY::_narrow( aComponent );
    }
    catch (CORBA::Exception&){
      MESSAGE("Caught CORBA::Exception.");
    }
    catch (...){
      MESSAGE("Caught unknown exception.");
    }
  }
  return aGen;
}

#ifdef WITH_VISU
/*!
 * Get VISU component engine
 */
static VISU::VISU_Gen_var getVisuEngine()
{
  static VISU::VISU_Gen_var aGen;
  if( CORBA::is_nil( aGen ) ) {
    try{
      SALOME_LifeCycleCORBA aLCC(SalomeApp_Application::namingService());
      Engines::EngineComponent_var aComponent = aLCC.FindOrLoad_Component("FactoryServer","VISU");
      aGen = VISU::VISU_Gen::_narrow( aComponent );
    }
    catch (CORBA::Exception&){
      MESSAGE("Caught CORBA::Exception.");
    }
    catch (...){
      MESSAGE("Caught unknown exception.");
    }

  }
  return aGen;
}
#endif

#define MY_EVENT QEvent::User + 555
class MyEvent : public QEvent
{
public:
  MyEvent( const int iter ) : QEvent( (QEvent::Type)(MY_EVENT) ), myIter( iter ) {}
  int iter() const { return myIter; }

private:
  int myIter;
};

/*!
 * Progress thread class
 */
class MyThread : public QThread
{
public:
  MyThread( SierpinskyGUI_RunDlg* parent, const double x, const double y, const int nbIter ) :
    myParent( parent ), myStopped( false ), myX( x ), myY( y ), myNbIter( nbIter ) { start(); }

  void init( const double x, const double y, const int nbIter ) 
  {
    myX = x; myY = y;
    myNbIter = nbIter;
    myStopped = false;
    start();
  }
  void stop() { myStopped = true; }

protected:
  void run() 
  {
    while ( !myStopped && myNbIter-- ) {
      int ni = getRandomizerEngine()->NextIteration();
      getSierpinskyEngine()->NextPoint( myX, myY, ni, myX, myY );
      QApplication::postEvent( myParent, new MyEvent( myNbIter ) );
      msleep( 50 );
    }
  }

private:
  SierpinskyGUI_RunDlg* myParent;
  bool                  myStopped;
  double                myX;
  double                myY;
  int                   myNbIter;
};

#define MARGIN_SIZE  11
#define SPACING_SIZE  6

/*!
 * Constructor
 */
SierpinskyGUI_RunDlg::SierpinskyGUI_RunDlg( QWidget* parent, SierpinskyGUI* module )
     : QDialog( parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint ),
       myThread( 0 ), myModule( module )
{
  setAttribute( Qt::WA_DeleteOnClose );

  setObjectName("SierpinskyGUI_RunDlg");
  setModal(true);

  setWindowTitle( tr( "CAPTION" ) );
  setSizeGripEnabled( true );
  
  QGridLayout* topLayout = new QGridLayout( this ); 
  topLayout->setMargin( MARGIN_SIZE ); topLayout->setSpacing( SPACING_SIZE );

  // start point group
  box1 = new QGroupBox( tr( "START_POINT" ), this );
  QGridLayout* box1Layout = new QGridLayout( box1 );
  box1Layout->setAlignment( Qt::AlignTop );
  box1Layout->setMargin( MARGIN_SIZE );
  box1Layout->setSpacing( SPACING_SIZE );

  QLabel* labX = new QLabel( "X:", box1 );
  myStartX = new QtxDoubleSpinBox( 0.0, 1.0, 0.1 ,box1);
  myStartX->setMinimumWidth( 150 );

  QLabel* labY = new QLabel( "Y:", box1 );
  myStartY = new QtxDoubleSpinBox( 0.0, 1.0, 0.1 ,box1);
  myStartY->setMinimumWidth( 150 );

  myStartRandom = new QCheckBox( tr( "RANDOM" ), box1 );

  box1Layout->addWidget( labX,          0, 0 );
  box1Layout->addWidget( myStartX,      0, 1 );
  box1Layout->addWidget( labY,          0, 2 );
  box1Layout->addWidget( myStartY,      0, 3 );
  box1Layout->addWidget( myStartRandom, 1, 1 );

  // base points group
  box2 = new QGroupBox( tr( "BASE_POINTS" ), this );
  QGridLayout* box2Layout = new QGridLayout( box2 );
  box2Layout->setAlignment( Qt::AlignTop );
  box2Layout->setMargin( MARGIN_SIZE );
  box2Layout->setSpacing( SPACING_SIZE );

  QLabel* labX1 = new QLabel( "X1:", box2 );
  myX1 = new QtxDoubleSpinBox( 0.0, 1.0, 0.1 , box2);
  myX1->setMinimumWidth( 150 );

  QLabel* labY1 = new QLabel( "Y1:", box2 );
  myY1 = new QtxDoubleSpinBox( 0.0, 1.0, 0.1 , box2);
  myY1->setMinimumWidth( 150 );

  QLabel* labX2 = new QLabel( "X2:", box2 );
  myX2 = new QtxDoubleSpinBox( 0.0, 1.0, 0.1 , box2);
  myX2->setMinimumWidth( 150 );

  QLabel* labY2 = new QLabel( "Y2:", box2 );
  myY2 = new QtxDoubleSpinBox( 0.0, 1.0, 0.1, box2);
  myY2->setMinimumWidth( 150 );

  QLabel* labX3 = new QLabel( "X3:", box2 );
  myX3 = new QtxDoubleSpinBox( 0.0, 1.0, 0.1, box2);
  myX3->setMinimumWidth( 150 );

  QLabel* labY3 = new QLabel( "Y3:", box2 );
  myY3 = new QtxDoubleSpinBox( 0.0, 1.0, 0.1, box2);
  myY3->setMinimumWidth( 150 );

  myBaseRandom  = new QCheckBox( tr( "RANDOM" ), box2 );
  myBaseDefault = new QCheckBox( tr( "DEFAULT" ), box2 );

  box2Layout->addWidget( labX1,         0, 0 );
  box2Layout->addWidget( myX1,          0, 1 );
  box2Layout->addWidget( labY1,         0, 2 );
  box2Layout->addWidget( myY1,          0, 3 );
  box2Layout->addWidget( labX2,         1, 0 );
  box2Layout->addWidget( myX2,          1, 1 );
  box2Layout->addWidget( labY2,         1, 2 );
  box2Layout->addWidget( myY2,          1, 3 );
  box2Layout->addWidget( labX3,         2, 0 );
  box2Layout->addWidget( myX3,          2, 1 );
  box2Layout->addWidget( labY3,         2, 2 );
  box2Layout->addWidget( myY3,          2, 3 );
  box2Layout->addWidget( myBaseRandom,  3, 1 );
  box2Layout->addWidget( myBaseDefault, 3, 3 );

  // number of iterations
  QLabel* labIter = new QLabel( tr( "NB_ITERATIONS" ), this );
  myIter = new QSpinBox( this );
  myIter->setMinimum(1);
  myIter->setMaximum(100000);
  myIter->setSingleStep(1);

  // results
  box3 = new QGroupBox( tr( "RESULTS" ), this );
  QGridLayout* box3Layout = new QGridLayout( box3 );
  box3Layout->setAlignment( Qt::AlignTop );
  box3Layout->setMargin( MARGIN_SIZE );
  box3Layout->setSpacing( SPACING_SIZE );

  myJpegCheck = new QCheckBox( tr( "EXPORT_2_JPEG" ), box3 );
  myJpegFile = new QLineEdit( box3 );
  myJpegFile->setMinimumWidth( 300 );

  myJpegBtn = new QPushButton( "...", box3 );
  myJpegBtn->setFixedSize( 20, 20 );

  QLabel* labJpeg = new QLabel( tr( "IMAGE_SIZE" ), box3 );
  myJpegSize = new QSpinBox( box3 );
  myJpegSize->setMinimum(0);
  myJpegSize->setMaximum(1280);
  myJpegSize->setSingleStep(10);
  myJpegSize->setMinimumWidth( 150 );

  QFrame* line = new QFrame( box3 );
  line->setFrameStyle( QFrame::HLine | QFrame::Sunken );

  myMedCheck = new QCheckBox( tr( "EXPORT_2_MED" ), box3 );
  myMedFile = new QLineEdit( box3 );
  myMedFile->setMinimumWidth( 300 );

  myMedBtn = new QPushButton( "...", box3 );
  myMedBtn->setFixedSize( 20, 20 );

  QLabel* labMed = new QLabel( tr( "MESH_SIZE" ), box3 );
  myMedSize = new QtxDoubleSpinBox( 0.0, 1000, 1.0, box3);
  myMedSize->setMinimumWidth( 150 );

  myVisuCheck = new QCheckBox( tr( "IMPORT_MED_2_VISU" ), box3 );

  box3Layout->addWidget( myJpegCheck, 0, 0, 1, 4 );
  box3Layout->addWidget( myJpegFile,  1, 0, 1, 3 );
  box3Layout->addWidget( myJpegBtn,   1, 3 );
  box3Layout->addWidget( labJpeg,     2, 0 );
  box3Layout->addWidget( myJpegSize,  2, 1 );
  box3Layout->addWidget( line,        3, 0, 1, 4 );
  box3Layout->addWidget( myMedCheck,  4, 0, 1, 4 );
  box3Layout->addWidget( myMedFile,   5, 0, 1, 3 );
  box3Layout->addWidget( myMedBtn,    5, 3 );
  box3Layout->addWidget( labMed,      6, 0 );
  box3Layout->addWidget( myMedSize,   6, 1 );
  box3Layout->addWidget( myVisuCheck, 7, 0, 1, 4 );

  // progress bar
  myProgress = new QProgressBar( this );
  myProgress->setTextVisible( false );
  myProgress->setFixedHeight( 10 );
 
  // common buttons
  myStartBtn  = new QPushButton( tr( "START_BTN" ),  this );
  myCancelBtn = new QPushButton( tr( "CANCEL_BTN" ), this );
  QHBoxLayout* btnLayout = new QHBoxLayout;
  btnLayout->addWidget( myStartBtn ); 
  btnLayout->addStretch();
  btnLayout->addWidget( myCancelBtn );

  topLayout->addWidget( box1,       0, 0, 1, 2 );
  topLayout->addWidget( box2,       1, 0, 1, 2 );
  topLayout->addWidget( labIter,    2, 0 );
  topLayout->addWidget( myIter,     2, 1 );
  topLayout->addWidget( box3,       3, 0, 1, 2 );
  topLayout->addWidget( myProgress, 4, 0, 1, 2 );
  topLayout->addLayout( btnLayout,  5, 0, 1, 2 );

  // signals and slots connections
  connect( myJpegBtn,     SIGNAL( clicked() ), this, SLOT( onBrowse() ) );
  connect( myMedBtn,      SIGNAL( clicked() ), this, SLOT( onBrowse() ) );
  connect( myStartRandom, SIGNAL( clicked() ), this, SLOT( updateState() ) );
  connect( myBaseRandom,  SIGNAL( clicked() ), this, SLOT( toggled() ) );
  connect( myBaseDefault, SIGNAL( clicked() ), this, SLOT( toggled() ) );
  connect( myJpegCheck,   SIGNAL( clicked() ), this, SLOT( updateState() ) );
  connect( myMedCheck,    SIGNAL( clicked() ), this, SLOT( updateState() ) );
  connect( myStartBtn,    SIGNAL( clicked() ), this, SLOT( accept() ) );
  connect( myCancelBtn,   SIGNAL( clicked() ), this, SLOT( close() ) );

  // initial state
  myStartX->setValue( 0.0 ); myStartY->setValue( 0.0 );
  myStartRandom->setChecked( true );
  myX1->setValue( 0.5 ); myY1->setValue( 1.0 );
  myX2->setValue( 0.0 ); myY2->setValue( 0.0 );
  myX3->setValue( 1.0 ); myY3->setValue( 0.0 );
  myBaseRandom->setChecked( false );
  myBaseDefault->setChecked( true );
  myIter->setValue( 1000 );
  myJpegCheck->setChecked( false );
  myJpegSize->setValue( 200 );
  myMedCheck->setChecked( false );
  myMedSize->setValue( 100 );
  myVisuCheck->setChecked( true );
#ifndef WITH_VISU
  myVisuCheck->setVisible( false );
#endif
  updateState();
}

/*!
 * Destructor
 */
SierpinskyGUI_RunDlg::~SierpinskyGUI_RunDlg()
{
  if ( myThread ) {
    myThread->stop();
    myThread->wait();
    delete myThread;
  }
}

/*!
 * Browse file 
 */
void SierpinskyGUI_RunDlg::onBrowse()
{
  const QObject* snd = sender();
  bool jpg = snd == myJpegBtn;
  QString aFileName = SUIT_FileDlg::getFileName( this, 
						 jpg ? myJpegFile->text() : myMedFile->text(), 
						 (jpg ? tr( "JPEG_FILES") : tr( "MED_FILES" )).split( ":", QString::SkipEmptyParts ),
						 jpg ? tr( "EXPORT_JPEG_FILE" ) : tr( "EXPORT_MED_FILE" ),
						 false );
  if ( !aFileName.isEmpty() ) {
    jpg ? myJpegFile->setText( aFileName ) : myMedFile->setText( aFileName );
  }
}

/*!
 * Update widgets state
 */
void SierpinskyGUI_RunDlg::updateState()
{
  myStartX->setEnabled( !myStartRandom->isChecked() );
  myStartY->setEnabled( !myStartRandom->isChecked() );

  myX1->setEnabled( !myBaseRandom->isChecked() && !myBaseDefault->isChecked() );
  myY1->setEnabled( !myBaseRandom->isChecked() && !myBaseDefault->isChecked() );
  myX2->setEnabled( !myBaseRandom->isChecked() && !myBaseDefault->isChecked() );
  myY2->setEnabled( !myBaseRandom->isChecked() && !myBaseDefault->isChecked() );
  myX3->setEnabled( !myBaseRandom->isChecked() && !myBaseDefault->isChecked() );
  myY3->setEnabled( !myBaseRandom->isChecked() && !myBaseDefault->isChecked() );

  myJpegFile->setEnabled( myJpegCheck->isChecked() );
  myJpegBtn->setEnabled( myJpegCheck->isChecked() );
  myJpegSize->setEnabled( myJpegCheck->isChecked() );

  myMedFile->setEnabled( myMedCheck->isChecked() );
  myMedBtn->setEnabled( myMedCheck->isChecked() );
  myMedSize->setEnabled( myMedCheck->isChecked() );
  myVisuCheck->setEnabled( myMedCheck->isChecked() );
}

/*!
 * Toggle 'Base points' mode
 */
void SierpinskyGUI_RunDlg::toggled()
{
  const QObject* snd = sender();
  if ( snd == myBaseRandom  &&  myBaseRandom->isChecked() ) myBaseDefault->setChecked( false );
  if ( snd == myBaseDefault && myBaseDefault->isChecked() ) myBaseRandom->setChecked( false );
  updateState();
}

/*!
 * Called when user presses 'Start' button
 */
void SierpinskyGUI_RunDlg::accept()
{
  RANDOMIZER_ORB::RANDOMIZER_var aRandGen  = getRandomizerEngine();
  SIERPINSKY_ORB::SIERPINSKY_var aSierpGen = getSierpinskyEngine();

  if( CORBA::is_nil( aRandGen ) || CORBA::is_nil( aSierpGen ) ) {
    SUIT_MessageBox::critical( this,
			       tr( "ERR_ERROR" ),
			       tr( "EXEC_ERROR" ),
			       tr( "OK_BTN") );
    return;
  }

  // JPEG file
  QString aJPEGFile;
  int aJPEGSize = 0;
  if ( myJpegCheck->isChecked() ) {
    aJPEGFile = myJpegFile->text().trimmed();
    aJPEGSize = myJpegSize->value();
    if ( aJPEGFile.isEmpty() || aJPEGSize <= 0 ) {
      int res = SUIT_MessageBox::warning( this,
					  tr( "WRN_WARNING" ),
					  tr( "JPEG_PARAMETERS_ERROR" ),
					  tr( "YES_BTN"), tr( "NO_BTN" ), 0, 1, 0 );
      if ( res ) return;
    }
  }

  // MED file
  QString aMEDFile;
  double aMEDSize = 0;
  if ( myMedCheck->isChecked() ) {
    aMEDFile = myMedFile->text().trimmed();
    aMEDSize = myMedSize->value();
    if ( aMEDFile.isEmpty() || aMEDSize <= 0 ) {
      int res = SUIT_MessageBox::warning( this,
					  tr( "WRN_WARNING" ),
					  tr( "MED_PARAMETERS_ERROR" ),
					  tr( "YES_BTN"), tr( "NO_BTN" ), 0, 1, 0 );
      if ( res ) return;
    }
  }

  // start point
  double x = myStartX->value();
  double y = myStartY->value();
  if ( myStartRandom->isChecked() )
    aRandGen->InitPoint( x, y );

  // base points
  if ( myBaseDefault->isChecked() ) {
    aSierpGen->Reset();  // use default values from engine
  }
  else { 
    double x1 = myX1->value();
    double y1 = myY1->value();
    double x2 = myX2->value();
    double y2 = myY2->value();
    double x3 = myX3->value();
    double y3 = myY3->value();
    if ( myBaseRandom->isChecked() ) {
      aRandGen->InitPoint( x1, y1 );
      aRandGen->InitPoint( x2, y2 );
      aRandGen->InitPoint( x3, y3 );
    }
    aSierpGen->Init( x1, y1, x2, y2, x3, y3 );
  }

  // nb of iterations
  int iter = myIter->value();

  // start execution
  myProgress->setMaximum( iter );
  myProgress->setValue( 0 );
  if ( !myThread )
    myThread = new MyThread( this, x, y, iter );
  else
    myThread->init( x, y, iter );
  disconnect( myStartBtn, SIGNAL( clicked() ), this, SLOT( accept() ) );
  connect(    myStartBtn, SIGNAL( clicked() ), this, SLOT( stop() ) );
  myStartBtn->setText( tr( "STOP_BTN" ) );
  box1->setEnabled( false );
  box2->setEnabled( false );
  box3->setEnabled( false );
  myIter->setEnabled( false );
}

/*!
 * Called when user presses 'Stop' button
 */
void SierpinskyGUI_RunDlg::stop()
{
  myThread->stop();
  disconnect( myStartBtn, SIGNAL( clicked() ), this, SLOT( stop() ) );
  connect(    myStartBtn, SIGNAL( clicked() ), this, SLOT( accept() ) );
  myStartBtn->setText( tr( "START_BTN" ) );
  myProgress->setValue( 0 );
  box1->setEnabled( true );
  box2->setEnabled( true );
  box3->setEnabled( true );
  myIter->setEnabled( true );
}

/*!
 * Progress thread calls this method to update progress bar state
 */
void SierpinskyGUI_RunDlg::nextStep( const int step )
{
  if ( !step ) {
    disconnect( myStartBtn, SIGNAL( clicked() ), this, SLOT( stop() ) );
    connect(    myStartBtn, SIGNAL( clicked() ), this, SLOT( accept() ) );
    myStartBtn->setText( tr( "START_BTN" ) );
    myProgress->setValue( 0 );
    box1->setEnabled( true );
    box2->setEnabled( true );
    box3->setEnabled( true );
    myIter->setEnabled( true );

    // export to JPEG
    if ( myJpegCheck->isChecked() ) {
      QString aJPEGFile = myJpegFile->text().trimmed();
      int aJPEGSize = myJpegSize->value();
      if ( !aJPEGFile.isEmpty() && aJPEGSize > 0 ) {
        bool res = getSierpinskyEngine()->ExportToJPEG( aJPEGFile.toLatin1().data(), aJPEGSize );
	if ( !res ) {
	  // error: can't create JPEG image
	}
      }
    }
    // export to MED
    if ( myMedCheck->isChecked() ) {
      QString aMEDFile = myMedFile->text().trimmed();
      double aMEDSize = myMedSize->value();
      if ( !aMEDFile.isEmpty() && aMEDSize > 0 ) {
        bool res = getSierpinskyEngine()->ExportToMED( aMEDFile.toLatin1().data(), aMEDSize );
#ifdef WITH_VISU
	if ( res && myVisuCheck->isChecked() ) {
          VISU::VISU_Gen_var aVisuGen = getVisuEngine();
          if ( !CORBA::is_nil( aVisuGen ) ) {
            VISU::Result_var aResult = aVisuGen->ImportFile( aMEDFile.toLatin1().data() );
            if ( !CORBA::is_nil( aResult ) )
              aVisuGen->MeshOnEntity( aResult, "Sierpinsky", VISU::NODE );
	    myModule->getApp()->updateObjectBrowser( true );
          }
        }
#endif
      }
    }
  }
  else {
    myProgress->setValue( myProgress->maximum() - step );
  }
}

/*!
 *  Process progress event
 */
void SierpinskyGUI_RunDlg::customEvent( QEvent* e )
{
  if ( e->type() == MY_EVENT ) {
    MyEvent* me = (MyEvent*)e;
    nextStep( me->iter() );
  }
}
