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
// File   : GHS3DPRLPlugin_i.cxx
// Author : Christian VAN WAMBEKE (CEA) (from Hexotic plugin Lioka RAZAFINDRAZAKA)
// ---
//
#include "GHS3DPRLPlugin_Defs.hxx"
#include "SMESH_Hypothesis_i.hxx"

#include "utilities.h"

#include "GHS3DPRLPlugin_GHS3DPRL_i.hxx"
#include "GHS3DPRLPlugin_Hypothesis_i.hxx"

template <class T> class GHS3DPRLPlugin_Creator_i:public HypothesisCreator_i<T>
{
  // as we have 'module GHS3DPRLPlugin' in GHS3DPRLPlugin_Algorithm.idl
  virtual std::string GetModuleName() { return "GHS3DPRLPlugin"; }
};

//=============================================================================
/*!
 *
 */
//=============================================================================

extern "C"
{

  GHS3DPRLPLUGIN_EXPORT GenericHypothesisCreator_i* GetHypothesisCreator (const char* aHypName);

  GHS3DPRLPLUGIN_EXPORT GenericHypothesisCreator_i* GetHypothesisCreator (const char* aHypName)
  {
    MESSAGE("GetHypothesisCreator " << aHypName);

    GenericHypothesisCreator_i* aCreator = 0;

    // Algorithms
    if (strcmp(aHypName, "GHS3DPRL_3D") == 0 ||
       strcmp(aHypName, "MG-Tetra Parallel") == 0)
      aCreator = new GHS3DPRLPlugin_Creator_i<GHS3DPRLPlugin_GHS3DPRL_i>;
    // Hypotheses
    else if (strcmp(aHypName, "GHS3DPRL_Parameters") == 0 ||
             strcmp(aHypName, "MG-Tetra Parallel Parameters") == 0)
      aCreator = new GHS3DPRLPlugin_Creator_i<GHS3DPRLPlugin_Hypothesis_i>;
    else ;

    return aCreator;
  }
}
