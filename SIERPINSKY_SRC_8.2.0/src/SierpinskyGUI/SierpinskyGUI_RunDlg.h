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
// File    : SierpinskyGUI_RunDlg.h
// Author  : Vadim SANDLER (OCN)
// Created : 13/07/05
///////////////////////////////////////////////////////////
//
#ifndef __SIERPINSKYGUI_RUNDLG_H
#define __SIERPINSKYGUI_RUNDLG_H

#include "SierpinskyGUI.hxx"

#include <QDialog>

class QLineEdit;
class QCheckBox;
class QSpinBox;
class QPushButton;
class QGroupBox;
class QtxDoubleSpinBox;
class MyThread;
class QProgressBar;
class SierpinskyGUI;

class SIERPINSKY_EXPORT SierpinskyGUI_RunDlg : public QDialog
{
  Q_OBJECT

public:
  // Constructor
  SierpinskyGUI_RunDlg( QWidget*, SierpinskyGUI* );
  // Destructor
  ~SierpinskyGUI_RunDlg();

  // Progress thread calls this method to update progress bar state
  void nextStep( const int );

protected:
  // process progress event
  void customEvent( QEvent* );

protected slots:
  void accept();          // Called when user presses 'Start' button
  void stop();            // Called when user presses 'Stop' button
  void onBrowse();        // Browse file
  void updateState();     // Update widgets state
  void toggled();         // Toggle 'Base points' mode

private:
  // module
  SierpinskyGUI*     myModule;
  
  // widgets
  QGroupBox*         box1;
  QtxDoubleSpinBox*  myStartX;
  QtxDoubleSpinBox*  myStartY;
  QCheckBox*         myStartRandom;

  QGroupBox*         box2;
  QtxDoubleSpinBox*  myX1;
  QtxDoubleSpinBox*  myY1;
  QtxDoubleSpinBox*  myX2;
  QtxDoubleSpinBox*  myY2;
  QtxDoubleSpinBox*  myX3;
  QtxDoubleSpinBox*  myY3;
  QCheckBox*         myBaseRandom;
  QCheckBox*         myBaseDefault;

  QSpinBox*          myIter;

  QGroupBox*         box3;
  QCheckBox*         myJpegCheck;
  QLineEdit*         myJpegFile;
  QPushButton*       myJpegBtn;
  QSpinBox*          myJpegSize;
  QCheckBox*         myMedCheck;
  QLineEdit*         myMedFile;
  QPushButton*       myMedBtn;
  QtxDoubleSpinBox*  myMedSize;
  QCheckBox*         myVisuCheck;

  QPushButton*       myStartBtn;
  QPushButton*       myCancelBtn;

  QProgressBar*      myProgress;

  // progress thread
  MyThread*          myThread;
};

#endif // __SIERPINSKYGUI_RUNDLG_H
