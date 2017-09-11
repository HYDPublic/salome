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
// File   : GHS3DPRLPluginGUI_HypothesisCreator.h
// Author : Christian VAN WAMBEKE (CEA) (from Hexotic plugin Lioka RAZAFINDRAZAKA)
// ---
//
#ifndef GHS3DPRLPLUGINGUI_HypothesisCreator_H
#define GHS3DPRLPLUGINGUI_HypothesisCreator_H

#ifdef WIN32
  #if defined GHS3DPRLPluginGUI_EXPORTS
    #define GHS3DPRLPLUGINGUI_EXPORT __declspec( dllexport )
  #else
    #define GHS3DPRLPLUGINGUI_EXPORT __declspec( dllimport )
  #endif
#else
  #define GHS3DPRLPLUGINGUI_EXPORT
#endif


#include <SMESHGUI_Hypotheses.h>

class QtxIntSpinBox;
class QtxDoubleSpinBox;
class QCheckBox;
class QLineEdit;
class SMESH_AdvOptionsWdg;

typedef struct
{
  QString  myName;
  QString  myMEDName;
  int      myNbPart;
  bool     myKeepFiles;
  bool     myBackground;
  bool     myMultithread;
  //bool     myToMeshHoles;
  //bool     myToMergeSubdomains;
  float     myGradation;
  float     myMinSize;
  float     myMaxSize;
  QString   myAdvOptions;
 } GHS3DPRLHypothesisData;

/*!
 * \brief Class for creation of GHS3DPRL hypotheses
*/
class GHS3DPRLPLUGINGUI_EXPORT GHS3DPRLPluginGUI_HypothesisCreator : public SMESHGUI_GenericHypothesisCreator
{
  Q_OBJECT

public:
  GHS3DPRLPluginGUI_HypothesisCreator( const QString& );
  virtual ~GHS3DPRLPluginGUI_HypothesisCreator();

  virtual bool    checkParams() const;
  virtual QString helpPage() const;

protected:
  virtual QFrame* buildFrame    ();
  virtual void    retrieveParams() const;
  virtual QString storeParams   () const;

  virtual QString caption() const;
  virtual QPixmap icon() const;
  virtual QString type() const;

private:
  bool            readParamsFromHypo( GHS3DPRLHypothesisData& ) const;
  bool            readParamsFromWidgets( GHS3DPRLHypothesisData& ) const;
  bool            storeParamsToHypo( const GHS3DPRLHypothesisData& ) const;

private:
  QLineEdit*           myName;
  QLineEdit*           myMEDName;
  QtxIntSpinBox*       myNbPart;
  QCheckBox*           myKeepFiles;
  QCheckBox*           myBackground;
  QCheckBox*           myMultithread;
  QtxDoubleSpinBox*    myGradation;
  QtxDoubleSpinBox*    myMinSize;
  QtxDoubleSpinBox*    myMaxSize;
  bool                 myIs3D;
  SMESH_AdvOptionsWdg* myAdvTable;
};

#endif // GHS3DPRLPLUGINGUI_HypothesisCreator_H
