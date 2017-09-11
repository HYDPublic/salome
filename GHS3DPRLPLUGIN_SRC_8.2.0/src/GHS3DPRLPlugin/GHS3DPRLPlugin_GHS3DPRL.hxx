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
// File   : GHS3DPRLPlugin_GHS3DPRL.hxx
// Author : Christian VAN WAMBEKE (CEA) (from Hexotic plugin Lioka RAZAFINDRAZAKA)
// ---
//
#ifndef _GHS3DPRLPlugin_GHS3DPRL_HXX_
#define _GHS3DPRLPlugin_GHS3DPRL_HXX_

#include "GHS3DPRLPlugin_Defs.hxx"

#include "SMESH_Algo.hxx"
#include "SMESH_Mesh.hxx"
#include "Utils_SALOME_Exception.hxx"

class SMESH_Mesh;
class GHS3DPRLPlugin_Hypothesis;

class GHS3DPRLPLUGIN_EXPORT GHS3DPRLPlugin_GHS3DPRL: public SMESH_3D_Algo
{
public:
  GHS3DPRLPlugin_GHS3DPRL(int hypId, int studyId, SMESH_Gen* gen);
  virtual ~GHS3DPRLPlugin_GHS3DPRL();

  virtual bool CheckHypothesis(SMESH_Mesh&                          aMesh,
                               const TopoDS_Shape&                  aShape,
                               SMESH_Hypothesis::Hypothesis_Status& aStatus);

  void SetParameters(const GHS3DPRLPlugin_Hypothesis* hyp);

  virtual bool Compute(SMESH_Mesh&         aMesh,
                       const TopoDS_Shape& aShape);

  virtual bool Compute(SMESH_Mesh & aMesh, SMESH_MesherHelper* aHelper);

  virtual bool Evaluate(SMESH_Mesh&         aMesh,
                        const TopoDS_Shape& aShape,
                        MapShapeNbElems&    aResMap);

protected:
  const GHS3DPRLPlugin_Hypothesis* _hypothesis;
private:
  int   _countSubMesh;
  int   _countTotal;
  int   _nodeRefNumber;

  std::string _MEDName;
  int         _NbPart;      //number of partitions
  bool        _KeepFiles;   //tepal file .noboite binary or not
  bool        _Background;  //true for big meshes
  bool        _Multithread; //true for mg_tetra_hpc false for mg_tetra_hpc_mpi
  //ToMergeSubdomains ToTagSubdomains ToOutputInterfaces ToDiscardSubdomains
  float       _Gradation;
  float       _MinSize;
  float       _MaxSize;
  std::string _AdvOptions;
};

#endif
