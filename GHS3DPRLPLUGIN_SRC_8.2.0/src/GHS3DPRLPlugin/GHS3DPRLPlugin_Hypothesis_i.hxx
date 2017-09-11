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
// File   : GHS3DPRLPlugin_Hypothesis_i.hxx
// Author : Christian VAN WAMBEKE (CEA) (from Hexotic plugin Lioka RAZAFINDRAZAKA)
// ---
//
#ifndef _GHS3DPRLPlugin_Hypothesis_i_HXX_
#define _GHS3DPRLPlugin_Hypothesis_i_HXX_

#include "GHS3DPRLPlugin_Defs.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(GHS3DPRLPlugin_Algorithm)

#include "SMESH_Hypothesis_i.hxx"
#include "GHS3DPRLPlugin_Hypothesis.hxx"

class SMESH_Gen;

// GHS3DPRLPlugin parameters hypothesis

class GHS3DPRLPLUGIN_EXPORT GHS3DPRLPlugin_Hypothesis_i:
  public virtual POA_GHS3DPRLPlugin::GHS3DPRLPlugin_Hypothesis,
  public virtual SMESH_Hypothesis_i
{
 public:
  // Constructor
  GHS3DPRLPlugin_Hypothesis_i (PortableServer::POA_ptr thePOA,
                               int                     theStudyId,
                               ::SMESH_Gen*            theGenImpl);
  // Destructor
  virtual ~GHS3DPRLPlugin_Hypothesis_i();

  // Methods for setting and getting parameters values
  void SetMEDName(const char* theVal);
  char * GetMEDName();

  void SetNbPart(CORBA::Long theVal);
  CORBA::Long GetNbPart();

  void SetKeepFiles(CORBA::Boolean theVal);
  CORBA::Boolean GetKeepFiles();

  void SetBackground(CORBA::Boolean theVal);
  CORBA::Boolean GetBackground();

  void SetMultithread(CORBA::Boolean theVal);
  CORBA::Boolean GetMultithread();

//  void SetToMergeSubdomains(CORBA::Boolean theVal);
//  CORBA::Boolean GetToMergeSubdomains();

  void SetGradation(CORBA::Float theVal);
  CORBA::Float GetGradation();

  void SetMinSize(CORBA::Float theVal);
  CORBA::Float GetMinSize();

  void SetMaxSize(CORBA::Float theVal);
  CORBA::Float GetMaxSize();

  void SetAdvancedOption(const char* theOptAndVals );
  char* GetAdvancedOption();

  // Get implementation
  ::GHS3DPRLPlugin_Hypothesis* GetImpl();

  // Verify whether hypothesis supports given entity type
  CORBA::Boolean IsDimSupported( SMESH::Dimension type );
};

#endif
