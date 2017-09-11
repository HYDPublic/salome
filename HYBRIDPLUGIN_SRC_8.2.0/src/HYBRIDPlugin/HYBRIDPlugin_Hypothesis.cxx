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

//=============================================================================
// File      : HYBRIDPlugin_Hypothesis.cxx
// Created   : Wed Apr  2 12:36:29 2008
// Author    : Edward AGAPOV (eap)
//=============================================================================
//
#include "HYBRIDPlugin_Hypothesis.hxx"

#include <SMESHDS_Mesh.hxx>

#include <TCollection_AsciiString.hxx>

#ifdef WIN32
#include <process.h>
#define getpid _getpid
#endif

//=======================================================================
//function : HYBRIDPlugin_Hypothesis
//=======================================================================

HYBRIDPlugin_Hypothesis::HYBRIDPlugin_Hypothesis(int hypId, int studyId, SMESH_Gen * gen)
  : SMESH_Hypothesis(hypId, studyId, gen),
  myToMeshHoles(DefaultMeshHoles()),
  myLayersOnAllWrap(DefaultLayersOnAllWrap()),
  myToMakeGroupsOfDomains(DefaultToMakeGroupsOfDomains()),
  myMaximumMemory(-1),
  myInitialMemory(-1),
  myOptimizationLevel(DefaultOptimizationLevel()),
  myCollisionMode(DefaultCollisionMode()),
  myBoundaryLayersGrowth(DefaultBoundaryLayersGrowth()),
  myElementGeneration(DefaultElementGeneration()),
  myKeepFiles(DefaultKeepFiles()),
  myWorkingDirectory(DefaultWorkingDirectory()),
  myVerboseLevel(DefaultVerboseLevel()),
  myToCreateNewNodes(DefaultToCreateNewNodes()),
  myToUseBoundaryRecoveryVersion(DefaultToUseBoundaryRecoveryVersion()),
  myToUseFemCorrection(DefaultToUseFEMCorrection()),
  myToRemoveCentralPoint(DefaultToRemoveCentralPoint()),
  myLogInStandardOutput(DefaultStandardOutputLog()),
  myGradation(DefaultGradation()),
  myAddMultinormals(DefaultAddMultinormals()),
  mySmoothNormals(DefaultSmoothNormals()),
  myHeightFirstLayer(DefaultHeightFirstLayer()),
  myBoundaryLayersProgression(DefaultBoundaryLayersProgression()),
  myMultinormalsAngle(DefaultMultinormalsAngle()),
  myNbOfBoundaryLayers(DefaultNbOfBoundaryLayers()),
  _enfVertexList(DefaultHYBRIDEnforcedVertexList()),
  _enfVertexCoordsSizeList(DefaultHYBRIDEnforcedVertexCoordsValues()),
  _enfVertexEntrySizeList(DefaultHYBRIDEnforcedVertexEntryValues()),
  _coordsEnfVertexMap(DefaultCoordsHYBRIDEnforcedVertexMap()),
  _geomEntryEnfVertexMap(DefaultGeomEntryHYBRIDEnforcedVertexMap()),
  _enfMeshList(DefaultHYBRIDEnforcedMeshList()),
  _entryEnfMeshMap(DefaultEntryHYBRIDEnforcedMeshListMap()),
  _enfNodes(TIDSortedNodeGroupMap()),
  _enfEdges(TIDSortedElemGroupMap()),
  _enfTriangles(TIDSortedElemGroupMap()),
  _nodeIDToSizeMap(DefaultID2SizeMap()),
  _groupsToRemove(DefaultGroupsToRemove())
{
  _name = "HYBRID_Parameters";
  _param_algo_dim = 3;
}

//=======================================================================
//function : SetLayersOnAllWrap
//=======================================================================

void HYBRIDPlugin_Hypothesis::SetLayersOnAllWrap(bool toMesh)
{
  if ( myLayersOnAllWrap != toMesh ) {
    myLayersOnAllWrap = toMesh;
    NotifySubMeshesHypothesisModification();
  }
}

//=======================================================================
//function : GetLayersOnAllWrap
//=======================================================================

bool HYBRIDPlugin_Hypothesis::GetLayersOnAllWrap(bool checkFreeOption) const
{
  return myLayersOnAllWrap;
}

//=======================================================================
//function : SetToMeshHoles
//=======================================================================

void HYBRIDPlugin_Hypothesis::SetToMeshHoles(bool toMesh)
{
  if ( myToMeshHoles != toMesh ) {
    myToMeshHoles = toMesh;
    NotifySubMeshesHypothesisModification();
  }
}

//=======================================================================
//function : GetToMeshHoles
//=======================================================================

bool HYBRIDPlugin_Hypothesis::GetToMeshHoles(bool checkFreeOption) const
{
  if (checkFreeOption && !myTextOption.empty()) {
    if ( myTextOption.find("-c 0"))
      return true;
    if ( myTextOption.find("-c 1"))
      return false;
  }
  return myToMeshHoles;
}

//=======================================================================
//function : SetToMakeGroupsOfDomains
//=======================================================================

void HYBRIDPlugin_Hypothesis::SetToMakeGroupsOfDomains(bool toMakeGroups)
{
  if ( myToMakeGroupsOfDomains != toMakeGroups ) {
    myToMakeGroupsOfDomains = toMakeGroups;
    NotifySubMeshesHypothesisModification();
  }
}

//=======================================================================
//function : GetToMakeGroupsOfDomains
//=======================================================================

bool HYBRIDPlugin_Hypothesis::GetToMakeGroupsOfDomains() const
{
  return myToMakeGroupsOfDomains;
}

//=======================================================================
//function : GetToMakeGroupsOfDomains
//=======================================================================

bool HYBRIDPlugin_Hypothesis::GetToMakeGroupsOfDomains(const HYBRIDPlugin_Hypothesis* hyp)
{
  bool res;
  if ( hyp ) res = /*hyp->GetToMeshHoles(true) &&*/ hyp->GetToMakeGroupsOfDomains();
  else       res = /*DefaultMeshHoles()        &&*/ DefaultToMakeGroupsOfDomains();
  return res;
}

//=======================================================================
//function : SetMaximumMemory
//=======================================================================

void HYBRIDPlugin_Hypothesis::SetMaximumMemory(double MB)
{
  if ( myMaximumMemory != MB ) {
    myMaximumMemory = MB;
    NotifySubMeshesHypothesisModification();
  }
}

//=======================================================================
//function : GetMaximumMemory
//           * automatic memory adjustment mode. Default is zero
//=======================================================================

double HYBRIDPlugin_Hypothesis::GetMaximumMemory() const
{
  return myMaximumMemory;
}

//=======================================================================
//function : SetInitialMemory
//=======================================================================

void HYBRIDPlugin_Hypothesis::SetInitialMemory(double MB)
{
  if ( myInitialMemory != MB ) {
    myInitialMemory = MB;
    NotifySubMeshesHypothesisModification();
  }
}

//=======================================================================
//function : GetInitialMemory
//=======================================================================

double HYBRIDPlugin_Hypothesis::GetInitialMemory() const
{
  return myInitialMemory;
}

//=======================================================================
//function : SetOptimizationLevel
//=======================================================================

void HYBRIDPlugin_Hypothesis::SetOptimizationLevel(OptimizationLevel level)
{
  if ( myOptimizationLevel != level ) {
    myOptimizationLevel = level;
    NotifySubMeshesHypothesisModification();
  }
}

//=======================================================================
//function : GetOptimizationLevel
//=======================================================================
HYBRIDPlugin_Hypothesis::OptimizationLevel HYBRIDPlugin_Hypothesis::GetOptimizationLevel() const
{
  return (OptimizationLevel) myOptimizationLevel;
}

//=======================================================================
//function : SetCollisionMode
//=======================================================================
void HYBRIDPlugin_Hypothesis::SetCollisionMode(CollisionMode mode)
{
  if ( myCollisionMode != mode ) {
    myCollisionMode = mode;
    NotifySubMeshesHypothesisModification();
  }
}

//=======================================================================
//function : GetCollisionMode
//=======================================================================
HYBRIDPlugin_Hypothesis::CollisionMode HYBRIDPlugin_Hypothesis::GetCollisionMode() const
{
  return (CollisionMode) myCollisionMode;
}

//=======================================================================
//function : SetBoundaryLayersGrowth
//=======================================================================
void HYBRIDPlugin_Hypothesis::SetBoundaryLayersGrowth(BoundaryLayersGrowth mode)
{
  if ( myBoundaryLayersGrowth != mode ) {
    myBoundaryLayersGrowth = mode;
    NotifySubMeshesHypothesisModification();
  }
}

//=======================================================================
//function : GetBoundaryLayersGrowth
//=======================================================================
HYBRIDPlugin_Hypothesis::BoundaryLayersGrowth HYBRIDPlugin_Hypothesis::GetBoundaryLayersGrowth() const
{
  return (BoundaryLayersGrowth) myBoundaryLayersGrowth;
}

//=======================================================================
//function : SetElementGeneration
//=======================================================================
void HYBRIDPlugin_Hypothesis::SetElementGeneration(ElementGeneration mode)
{
  if ( myElementGeneration != mode ) {
    myElementGeneration = mode;
    NotifySubMeshesHypothesisModification();
  }
}

//=======================================================================
//function : GetElementGeneration
//=======================================================================
HYBRIDPlugin_Hypothesis::ElementGeneration HYBRIDPlugin_Hypothesis::GetElementGeneration() const
{
  return (ElementGeneration) myElementGeneration;
}

//=======================================================================
//function : SetAddMultinormals
//=======================================================================
void HYBRIDPlugin_Hypothesis::SetAddMultinormals(bool toAddMultinormals)
{
  if ( myAddMultinormals != toAddMultinormals ) {
    myAddMultinormals = toAddMultinormals;
    NotifySubMeshesHypothesisModification();
  }
}

//=======================================================================
//function : GetAddMultinormals
//=======================================================================

bool HYBRIDPlugin_Hypothesis::GetAddMultinormals() const
{
  return myAddMultinormals;
}

//=======================================================================
//function : SetSmoothNormals
//=======================================================================

void HYBRIDPlugin_Hypothesis::SetSmoothNormals(bool toSmoothNormals)
{
  if ( mySmoothNormals != toSmoothNormals ) {
    mySmoothNormals = toSmoothNormals;
    NotifySubMeshesHypothesisModification();
  }
}

//=======================================================================
//function : GetSmoothNormals
//=======================================================================

bool HYBRIDPlugin_Hypothesis::GetSmoothNormals() const
{
  return mySmoothNormals;
}

//=======================================================================
//function : SetHeightFirstLayer
//=======================================================================

void HYBRIDPlugin_Hypothesis::SetHeightFirstLayer(double toHeightFirstLayer)
{
  if ( myHeightFirstLayer != toHeightFirstLayer ) {
    myHeightFirstLayer = toHeightFirstLayer;
    NotifySubMeshesHypothesisModification();
  }
}

//=======================================================================
//function : GetHeightFirstLayer
//=======================================================================

double HYBRIDPlugin_Hypothesis::GetHeightFirstLayer() const
{
  return myHeightFirstLayer;
}

//=======================================================================
//function : SetBoundaryLayersProgression
//=======================================================================

void HYBRIDPlugin_Hypothesis::SetBoundaryLayersProgression(double toBoundaryLayersProgression)
{
  if ( myBoundaryLayersProgression != toBoundaryLayersProgression ) {
    myBoundaryLayersProgression = toBoundaryLayersProgression;
    NotifySubMeshesHypothesisModification();
  }
}

//=======================================================================
//function : GetBoundaryLayersProgression
//=======================================================================

double HYBRIDPlugin_Hypothesis::GetBoundaryLayersProgression() const
{
  return myBoundaryLayersProgression;
}

//=======================================================================
//function : SetMultinormalsAngle
//=======================================================================

void HYBRIDPlugin_Hypothesis::SetMultinormalsAngle(double toMultinormalsAngle)
{
  if ( myMultinormalsAngle != toMultinormalsAngle ) {
    myMultinormalsAngle = toMultinormalsAngle;
    NotifySubMeshesHypothesisModification();
  }
}

//=======================================================================
//function : GetMultinormalsAngle
//=======================================================================

double HYBRIDPlugin_Hypothesis::GetMultinormalsAngle() const
{
  return myMultinormalsAngle;
}

//=======================================================================
//function : SetNbOfBoundaryLayers
//=======================================================================

void HYBRIDPlugin_Hypothesis::SetNbOfBoundaryLayers(short toNbOfBoundaryLayers)
{
  if ( myNbOfBoundaryLayers != toNbOfBoundaryLayers ) {
    myNbOfBoundaryLayers = toNbOfBoundaryLayers;
    NotifySubMeshesHypothesisModification();
  }
}

//=======================================================================
//function : GetMultinormalsAngle
//=======================================================================

short HYBRIDPlugin_Hypothesis::GetNbOfBoundaryLayers() const
{
  return myNbOfBoundaryLayers;
}


/////////////////////////////////////////////////////////////////////////


//=======================================================================
//function : SetWorkingDirectory
//=======================================================================

void HYBRIDPlugin_Hypothesis::SetWorkingDirectory(const std::string& path)
{
  if ( myWorkingDirectory != path ) {
    myWorkingDirectory = path;
    NotifySubMeshesHypothesisModification();
  }
}

//=======================================================================
//function : GetWorkingDirectory
//=======================================================================

std::string HYBRIDPlugin_Hypothesis::GetWorkingDirectory() const
{
  return myWorkingDirectory;
}

//=======================================================================
//function : SetKeepFiles
//=======================================================================

void HYBRIDPlugin_Hypothesis::SetKeepFiles(bool toKeep)
{
  if ( myKeepFiles != toKeep ) {
    myKeepFiles = toKeep;
    NotifySubMeshesHypothesisModification();
  }
}

//=======================================================================
//function : GetKeepFiles
//=======================================================================

bool HYBRIDPlugin_Hypothesis::GetKeepFiles() const
{
  return myKeepFiles;
}

//=======================================================================
//function : SetVerboseLevel
//=======================================================================

void HYBRIDPlugin_Hypothesis::SetVerboseLevel(short level)
{
  if ( myVerboseLevel != level ) {
    myVerboseLevel = level;
    NotifySubMeshesHypothesisModification();
  }
}

//=======================================================================
//function : GetVerboseLevel
//=======================================================================

short HYBRIDPlugin_Hypothesis::GetVerboseLevel() const
{
  return myVerboseLevel;
}

//=======================================================================
//function : SetToCreateNewNodes
//=======================================================================

void HYBRIDPlugin_Hypothesis::SetToCreateNewNodes(bool toCreate)
{
  if ( myToCreateNewNodes != toCreate ) {
    myToCreateNewNodes = toCreate;
    NotifySubMeshesHypothesisModification();
  }
}

//=======================================================================
//function : GetToCreateNewNodes
//=======================================================================

bool HYBRIDPlugin_Hypothesis::GetToCreateNewNodes() const
{
  return myToCreateNewNodes;
}

//=======================================================================
//function : SetToUseBoundaryRecoveryVersion
//=======================================================================

void HYBRIDPlugin_Hypothesis::SetToUseBoundaryRecoveryVersion(bool toUse)
{
  if ( myToUseBoundaryRecoveryVersion != toUse ) {
    myToUseBoundaryRecoveryVersion = toUse;
    NotifySubMeshesHypothesisModification();
  }
}

//=======================================================================
//function : GetToUseBoundaryRecoveryVersion
//=======================================================================

bool HYBRIDPlugin_Hypothesis::GetToUseBoundaryRecoveryVersion() const
{
  return myToUseBoundaryRecoveryVersion;
}

//=======================================================================
//function : SetFEMCorrection
//=======================================================================

void HYBRIDPlugin_Hypothesis::SetFEMCorrection(bool toUseFem)
{
  if ( myToUseFemCorrection != toUseFem ) {
    myToUseFemCorrection = toUseFem;
    NotifySubMeshesHypothesisModification();
  }
}

//=======================================================================
//function : GetFEMCorrection
//=======================================================================

bool HYBRIDPlugin_Hypothesis::GetFEMCorrection() const
{
  return myToUseFemCorrection;
}

//=======================================================================
//function : SetToRemoveCentralPoint
//=======================================================================

void HYBRIDPlugin_Hypothesis::SetToRemoveCentralPoint(bool toRemove)
{
  if ( myToRemoveCentralPoint != toRemove ) {
    myToRemoveCentralPoint = toRemove;
    NotifySubMeshesHypothesisModification();
  }
}

//=======================================================================
//function : GetToRemoveCentralPoint
//=======================================================================

bool HYBRIDPlugin_Hypothesis::GetToRemoveCentralPoint() const
{
  return myToRemoveCentralPoint;
}

//=======================================================================
//function : SetAdvancedOption
//=======================================================================

void HYBRIDPlugin_Hypothesis::SetAdvancedOption(const std::string& option)
{
  if ( myTextOption != option ) {
    myTextOption = option;
    NotifySubMeshesHypothesisModification();
  }
}

//=======================================================================
//function : GetAdvancedOption
//=======================================================================

std::string HYBRIDPlugin_Hypothesis::GetAdvancedOption() const
{
  return myTextOption;
}

//=======================================================================
//function : SetGradation
//=======================================================================

void HYBRIDPlugin_Hypothesis::SetGradation(double gradation)
{
  if ( myGradation != gradation ) {
    myGradation = gradation;
    NotifySubMeshesHypothesisModification();
  }
}

//=======================================================================
//function : GetGradation
//=======================================================================

double HYBRIDPlugin_Hypothesis::GetGradation() const
{
  return myGradation;
}

//=======================================================================
//function : SetStandardOutputLog
//=======================================================================

void HYBRIDPlugin_Hypothesis::SetStandardOutputLog(bool logInStandardOutput)
{
  if ( myLogInStandardOutput != logInStandardOutput ) {
    myLogInStandardOutput = logInStandardOutput;
    NotifySubMeshesHypothesisModification();
  }
}

//=======================================================================
//function : GetStandardOutputLog
//=======================================================================

bool HYBRIDPlugin_Hypothesis::GetStandardOutputLog() const
{
  return myLogInStandardOutput;
}

//=======================================================================
//function : SetRemoveLogOnSuccess
//=======================================================================

void HYBRIDPlugin_Hypothesis::SetRemoveLogOnSuccess(bool removeLogOnSuccess)
{
  if ( myRemoveLogOnSuccess != removeLogOnSuccess ) {
    myRemoveLogOnSuccess = removeLogOnSuccess;
    NotifySubMeshesHypothesisModification();
  }
}

//=======================================================================
//function : GetRemoveLogOnSuccess
//=======================================================================

bool HYBRIDPlugin_Hypothesis::GetRemoveLogOnSuccess() const
{
  return myRemoveLogOnSuccess;
}

//=======================================================================
//function : SetEnforcedVertex
//=======================================================================

bool HYBRIDPlugin_Hypothesis::SetEnforcedVertex(std::string theName, std::string theEntry, std::string theGroupName,
                                               double size, double x, double y, double z, bool isCompound)
{
  MESSAGE("HYBRIDPlugin_Hypothesis::SetEnforcedVertex(\""<< theName << "\", \""<< theEntry << "\", \"" << theGroupName << "\", "
                                                      << size << ", " << x << ", " << y << ", " << z  << ", "<< isCompound << ")");

  bool toNotify = false;
  bool toCreate = true;

  THYBRIDEnforcedVertex *oldEnVertex;
  THYBRIDEnforcedVertex *newEnfVertex = new THYBRIDEnforcedVertex();
  newEnfVertex->name = theName;
  newEnfVertex->geomEntry = theEntry;
  newEnfVertex->coords.clear();
  if (!isCompound) {
    newEnfVertex->coords.push_back(x);
    newEnfVertex->coords.push_back(y);
    newEnfVertex->coords.push_back(z);
  }
  newEnfVertex->groupName = theGroupName;
  newEnfVertex->size = size;
  newEnfVertex->isCompound = isCompound;
  
  
  // update _enfVertexList
  THYBRIDEnforcedVertexList::iterator it = _enfVertexList.find(newEnfVertex);
  if (it != _enfVertexList.end()) {
    toCreate = false;
    oldEnVertex = (*it);
    MESSAGE("Enforced Vertex was found => Update");
    if (oldEnVertex->name != theName) {
      MESSAGE("Update name from \"" << oldEnVertex->name << "\" to \"" << theName << "\"");
      oldEnVertex->name = theName;
      toNotify = true;
    }
    if (oldEnVertex->groupName != theGroupName) {
      MESSAGE("Update group name from \"" << oldEnVertex->groupName << "\" to \"" << theGroupName << "\"");
      oldEnVertex->groupName = theGroupName;
      toNotify = true;
    }
    if (oldEnVertex->size != size) {
      MESSAGE("Update size from \"" << oldEnVertex->size << "\" to \"" << size << "\"");
      oldEnVertex->size = size;
      toNotify = true;
    }
    if (toNotify) {
      // update map coords / enf vertex if needed
      if (oldEnVertex->coords.size()) {
        _coordsEnfVertexMap[oldEnVertex->coords] = oldEnVertex;
        _enfVertexCoordsSizeList[oldEnVertex->coords] = size;
      }

      // update map geom entry / enf vertex if needed
      if (oldEnVertex->geomEntry != "") {
        _geomEntryEnfVertexMap[oldEnVertex->geomEntry] = oldEnVertex;
        _enfVertexEntrySizeList[oldEnVertex->geomEntry] = size;
      }
    }
  }

//   //////// CREATE ////////////
  if (toCreate) {
    toNotify = true;
    MESSAGE("Creating new enforced vertex");
    _enfVertexList.insert(newEnfVertex);
    if (theEntry == "") {
      _coordsEnfVertexMap[newEnfVertex->coords] = newEnfVertex;
      _enfVertexCoordsSizeList[newEnfVertex->coords] = size;
    }
    else {
      _geomEntryEnfVertexMap[newEnfVertex->geomEntry] = newEnfVertex;
      _enfVertexEntrySizeList[newEnfVertex->geomEntry] = size;
    }
  }

  if (toNotify)
    NotifySubMeshesHypothesisModification();

  MESSAGE("HYBRIDPlugin_Hypothesis::SetEnforcedVertex END");
  return toNotify;
}


//=======================================================================
//function : SetEnforcedMesh
//=======================================================================
bool HYBRIDPlugin_Hypothesis::SetEnforcedMesh(SMESH_Mesh& theMesh, SMESH::ElementType elementType, std::string name, std::string entry, std::string groupName)
{
  TIDSortedElemSet theElemSet;
  SMDS_ElemIteratorPtr eIt = theMesh.GetMeshDS()->elementsIterator(SMDSAbs_ElementType(elementType));
  while ( eIt->more() )
    theElemSet.insert( eIt->next() );
  MESSAGE("Add "<<theElemSet.size()<<" types["<<elementType<<"] from source mesh");
  bool added = SetEnforcedElements( theElemSet, elementType, groupName);
  if (added) {
    THYBRIDEnforcedMesh* newEnfMesh = new THYBRIDEnforcedMesh();
    newEnfMesh->persistID   = theMesh.GetMeshDS()->GetPersistentId();
    newEnfMesh->name        = name;
    newEnfMesh->entry       = entry;
    newEnfMesh->elementType = elementType;
    newEnfMesh->groupName   = groupName;
    
    THYBRIDEnforcedMeshList::iterator it = _enfMeshList.find(newEnfMesh);
    if (it == _enfMeshList.end()) {
      _entryEnfMeshMap[entry].insert(newEnfMesh);
      _enfMeshList.insert(newEnfMesh);
    }
    else {
      delete newEnfMesh;
    }
  }
  return added;
}

//=======================================================================
//function : SetEnforcedGroup
//=======================================================================
bool HYBRIDPlugin_Hypothesis::SetEnforcedGroup(const SMESHDS_Mesh* theMeshDS, SMESH::long_array_var theIDs, SMESH::ElementType elementType, std::string name, std::string entry, std::string groupName)
{
  MESSAGE("HYBRIDPlugin_Hypothesis::SetEnforcedGroup");
  TIDSortedElemSet theElemSet;
    if ( theIDs->length() == 0 ){MESSAGE("The source group is empty");}
    for ( CORBA::ULong i = 0; i < theIDs->length(); i++) {
      CORBA::Long ind = theIDs[i];
      if (elementType == SMESH::NODE)
      {
        const SMDS_MeshNode * node = theMeshDS->FindNode(ind);
        if (node)
          theElemSet.insert( node );
      }
      else
      {
        const SMDS_MeshElement * elem = theMeshDS->FindElement(ind);
        if (elem)
          theElemSet.insert( elem );
      }
    }

//   SMDS_ElemIteratorPtr it = theGroup->GetGroupDS()->GetElements();
//   while ( it->more() ) 
//     theElemSet.insert( it->next() );

  MESSAGE("Add "<<theElemSet.size()<<" types["<<elementType<<"] from source group ");
  bool added = SetEnforcedElements( theElemSet, elementType, groupName);
  if (added) {
    THYBRIDEnforcedMesh* newEnfMesh = new THYBRIDEnforcedMesh();
    newEnfMesh->name = name;
    newEnfMesh->entry = entry;
    newEnfMesh->elementType = elementType;
    newEnfMesh->groupName = groupName;
    
    THYBRIDEnforcedMeshList::iterator it = _enfMeshList.find(newEnfMesh);
    if (it == _enfMeshList.end()) {
      _entryEnfMeshMap[entry].insert(newEnfMesh);
      _enfMeshList.insert(newEnfMesh);
    }
  }
  return added;
}

//=======================================================================
//function : SetEnforcedElements
//=======================================================================
bool HYBRIDPlugin_Hypothesis::SetEnforcedElements(TIDSortedElemSet theElemSet, SMESH::ElementType elementType, std::string groupName)
{
  MESSAGE("HYBRIDPlugin_Hypothesis::SetEnforcedElements");
  TIDSortedElemSet::const_iterator it = theElemSet.begin();
  const SMDS_MeshElement* elem;
  const SMDS_MeshNode* node;
  bool added = true;
  std::pair<TIDSortedNodeGroupMap::iterator,bool> nodeRet;
  std::pair<TIDSortedElemGroupMap::iterator,bool> elemRet;

  for (;it != theElemSet.end();++it)
  {
    elem = (*it);
    switch (elementType) {
      case SMESH::NODE:
        node = dynamic_cast<const SMDS_MeshNode*>(elem);
        if (node) {
          nodeRet = _enfNodes.insert(make_pair(node,groupName));
          added = added && nodeRet.second;
          std::string msg = added ? "yes":"no";
          MESSAGE( "Node (" <<node->X()<<","<<node->Y()<<","<<node->Z()<< ") with ID " << node->GetID() <<" added ? " << msg);
        }
        else {
          SMDS_ElemIteratorPtr nodeIt = elem->nodesIterator();
          for (;nodeIt->more();) {
            node = dynamic_cast<const SMDS_MeshNode*>(nodeIt->next());
            nodeRet = _enfNodes.insert(make_pair(node,groupName));
            added = added && nodeRet.second;
          }
//          added = true;s
        }
        break;
      case SMESH::EDGE:
        if (elem->GetType() == SMDSAbs_Edge) {
          elemRet = _enfEdges.insert(make_pair(elem,groupName));
          added = added && elemRet.second;
        }
        else if (elem->GetType() > SMDSAbs_Edge) {
          SMDS_ElemIteratorPtr it = elem->edgesIterator();
          for (;it->more();) {
            const SMDS_MeshElement* anEdge = it->next();
            elemRet = _enfEdges.insert(make_pair(anEdge,groupName));
            added = added && elemRet.second;
          }
        }
        break;
      case SMESH::FACE:
        if (elem->GetType() == SMDSAbs_Face)
        {
          if (elem->NbCornerNodes() == 3) {
            elemRet = _enfTriangles.insert(make_pair(elem,groupName));
            added = added && elemRet.second;
          }
        }
        else if (elem->GetType() > SMDSAbs_Face) { // Group of faces
          SMDS_ElemIteratorPtr it = elem->facesIterator();
          for (;it->more();) {
            const SMDS_MeshElement* aFace = it->next();
            if (aFace->NbCornerNodes() == 3) {
              elemRet = _enfTriangles.insert(make_pair(aFace,groupName));
              added = added && elemRet.second;
            }
          }
        }
        break;
      default:
        break;
    };
  }
  if (added)
    NotifySubMeshesHypothesisModification();
  return added;
}


//=======================================================================
//function : GetEnforcedVertex
//=======================================================================

HYBRIDPlugin_Hypothesis::THYBRIDEnforcedVertex* HYBRIDPlugin_Hypothesis::GetEnforcedVertex(double x, double y, double z)
  throw (std::invalid_argument)
{
  std::vector<double> coord(3);
  coord[0] = x;
  coord[1] = y;
  coord[2] = z;
  if (_coordsEnfVertexMap.count(coord)>0)
    return _coordsEnfVertexMap[coord];
  std::ostringstream msg ;
  msg << "No enforced vertex at " << x << ", " << y << ", " << z;
  throw std::invalid_argument(msg.str());
}

HYBRIDPlugin_Hypothesis::THYBRIDEnforcedVertex* HYBRIDPlugin_Hypothesis::GetEnforcedVertex(const std::string theEntry)
  throw (std::invalid_argument)
{
  if (_geomEntryEnfVertexMap.count(theEntry)>0)
    return _geomEntryEnfVertexMap[theEntry];
  
  std::ostringstream msg ;
  msg << "No enforced vertex with entry " << theEntry;
  throw std::invalid_argument(msg.str());
}

//=======================================================================
//function : RemoveEnforcedVertex
//=======================================================================

bool HYBRIDPlugin_Hypothesis::RemoveEnforcedVertex(double x, double y, double z, const std::string theEntry)
  throw (std::invalid_argument)
{
  bool toNotify = false;
  std::ostringstream msg;
  THYBRIDEnforcedVertex *oldEnfVertex;
  std::vector<double> coords(3);
  coords[0] = x;
  coords[1] = y;
  coords[2] = z;
  
  // check that enf vertex with given enf vertex entry exists
  TGeomEntryHYBRIDEnforcedVertexMap::iterator it_enfVertexEntry = _geomEntryEnfVertexMap.find(theEntry);
  if (it_enfVertexEntry != _geomEntryEnfVertexMap.end()) {
    // Success
    MESSAGE("Found enforced vertex with geom entry " << theEntry);
    oldEnfVertex = it_enfVertexEntry->second;
    _geomEntryEnfVertexMap.erase(it_enfVertexEntry);
  } else {
    // Fail
    MESSAGE("Enforced vertex with geom entry " << theEntry << " not found");
    // check that enf vertex with given coords exists
    TCoordsHYBRIDEnforcedVertexMap::iterator it_coords_enf = _coordsEnfVertexMap.find(coords);
    if (it_coords_enf != _coordsEnfVertexMap.end()) {
      // Success
      MESSAGE("Found enforced vertex with coords " << x << ", " << y << ", " << z);
      oldEnfVertex = it_coords_enf->second;
      _coordsEnfVertexMap.erase(it_coords_enf);
      _enfVertexCoordsSizeList.erase(_enfVertexCoordsSizeList.find(coords));
    } else {
      // Fail
      MESSAGE("Enforced vertex with coords " << x << ", " << y << ", " << z << " not found");
      throw std::invalid_argument(msg.str());
    }
  }

  MESSAGE("Remove enf vertex from _enfVertexList");

  // update _enfVertexList
  THYBRIDEnforcedVertexList::iterator it = _enfVertexList.find(oldEnfVertex);
  if (it != _enfVertexList.end()) {
    if ((*it)->groupName != "")
      _groupsToRemove.insert((*it)->groupName);
    _enfVertexList.erase(it);
    toNotify = true;
    MESSAGE("Done");
  }

  if (toNotify)
    NotifySubMeshesHypothesisModification();

  return toNotify;
}

//=======================================================================
//function : ClearEnforcedVertices
//=======================================================================
void HYBRIDPlugin_Hypothesis::ClearEnforcedVertices()
{
  THYBRIDEnforcedVertexList::const_iterator it = _enfVertexList.begin();
  for(;it != _enfVertexList.end();++it) {
    if ((*it)->groupName != "")
      _groupsToRemove.insert((*it)->groupName);
  }
  _enfVertexList.clear();
  _coordsEnfVertexMap.clear();
  _geomEntryEnfVertexMap.clear();
  _enfVertexCoordsSizeList.clear();
  _enfVertexEntrySizeList.clear();
  NotifySubMeshesHypothesisModification();
}

//=======================================================================
//function : ClearEnforcedMeshes
//=======================================================================
void HYBRIDPlugin_Hypothesis::ClearEnforcedMeshes()
{
  THYBRIDEnforcedMeshList::const_iterator it = _enfMeshList.begin();
  for(;it != _enfMeshList.end();++it) {
    if ((*it)->groupName != "")
      _groupsToRemove.insert((*it)->groupName);
  }
  _enfNodes.clear();
  _enfEdges.clear();
  _enfTriangles.clear();
  _nodeIDToSizeMap.clear();
  _enfMeshList.clear();
  _entryEnfMeshMap.clear();
  NotifySubMeshesHypothesisModification();
}

//================================================================================
/*!
 * \brief At mesh loading, restore enforced elements by just loaded enforced meshes
 */
//================================================================================

void HYBRIDPlugin_Hypothesis::RestoreEnfElemsByMeshes()
{
  THYBRIDEnforcedMeshList::const_iterator it = _enfMeshList.begin();
  for(;it != _enfMeshList.end();++it) {
    THYBRIDEnforcedMesh* enfMesh = *it;
    if ( SMESH_Mesh* mesh = GetMeshByPersistentID( enfMesh->persistID ))
      SetEnforcedMesh( *mesh,
                       enfMesh->elementType,
                       enfMesh->name,
                       enfMesh->entry,
                       enfMesh->groupName );
    enfMesh->persistID = -1; // not to restore again
  }
}

//=======================================================================
//function : SetGroupsToRemove
//=======================================================================

void HYBRIDPlugin_Hypothesis::ClearGroupsToRemove()
{
  _groupsToRemove.clear();
}


//=======================================================================
//function : DefaultLayersOnAllWrap
//=======================================================================

bool HYBRIDPlugin_Hypothesis::DefaultLayersOnAllWrap()
{
  return true;
}

//=======================================================================
//function : DefaultMeshHoles
//=======================================================================

bool HYBRIDPlugin_Hypothesis::DefaultMeshHoles()
{
  return false; // PAL19680
}

//=======================================================================
//function : DefaultToMakeGroupsOfDomains
//=======================================================================

bool HYBRIDPlugin_Hypothesis::DefaultToMakeGroupsOfDomains()
{
  return false; // issue 0022172
}

//=======================================================================
//function : DefaultMaximumMemory
//=======================================================================

#ifndef WIN32
#include <sys/sysinfo.h>
#else
#include <windows.h>
#endif

double  HYBRIDPlugin_Hypothesis::DefaultMaximumMemory()
{
#ifndef WIN32
  struct sysinfo si;
  int err = sysinfo( &si );
  if ( err == 0 ) {
    int ramMB = si.totalram * si.mem_unit / 1024 / 1024;
    return ( 0.7 * ramMB );
  }
#else
  // See http://msdn.microsoft.com/en-us/library/aa366589.aspx
  MEMORYSTATUSEX statex;
  statex.dwLength = sizeof (statex);
  int err = GlobalMemoryStatusEx (&statex);
  if (err != 0) {
    int totMB = 
      statex.ullTotalPhys / 1024 / 1024 +
      statex.ullTotalPageFile / 1024 / 1024 +
      statex.ullTotalVirtual / 1024 / 1024;
    return ( 0.7 * totMB );
  }
#endif
  return 1024;
}

//=======================================================================
//function : DefaultInitialMemory
//=======================================================================

double  HYBRIDPlugin_Hypothesis::DefaultInitialMemory()
{
  return DefaultMaximumMemory();
}

//=======================================================================
//function : DefaultCollisionMode
//=======================================================================
short  HYBRIDPlugin_Hypothesis::DefaultCollisionMode()
{
  return Decrease;
}

//=======================================================================
//function : DefaultBoundaryLayersGrowth
//=======================================================================
short  HYBRIDPlugin_Hypothesis::DefaultBoundaryLayersGrowth()
{
  return Layer_Growth_Inward;
}

//=======================================================================
//function : DefaultElementGeneration
//=======================================================================
short  HYBRIDPlugin_Hypothesis::DefaultElementGeneration()
{
  return Generation_Tetra_Dominant;
}

//=======================================================================
//function : DefaultOptimizationLevel
//=======================================================================
short  HYBRIDPlugin_Hypothesis::DefaultOptimizationLevel()
{
  return Medium;
}

//=======================================================================
//function : DefaultWorkingDirectory
//=======================================================================

std::string HYBRIDPlugin_Hypothesis::DefaultWorkingDirectory()
{
  TCollection_AsciiString aTmpDir;

  char *Tmp_dir = getenv("SALOME_TMP_DIR");
  if(Tmp_dir != NULL) {
    aTmpDir = Tmp_dir;
  }
  else {
#ifdef WIN32
    aTmpDir = TCollection_AsciiString("C:\\");
#else
    aTmpDir = TCollection_AsciiString("/tmp/");
#endif
  }
  return aTmpDir.ToCString();
}

//=======================================================================
//function : DefaultKeepFiles
//=======================================================================

bool   HYBRIDPlugin_Hypothesis::DefaultKeepFiles()
{
  return false;
}

//=======================================================================
//function : DefaultRemoveLogOnSuccess
//=======================================================================

bool   HYBRIDPlugin_Hypothesis::DefaultRemoveLogOnSuccess()
{
  return false;
}


//=======================================================================
//function : DefaultVerboseLevel
//=======================================================================

short  HYBRIDPlugin_Hypothesis::DefaultVerboseLevel()
{
  return 10;
}

//=======================================================================
//function : DefaultToCreateNewNodes
//=======================================================================

bool HYBRIDPlugin_Hypothesis::DefaultToCreateNewNodes()
{
  return true;
}

//=======================================================================
//function : DefaultToUseBoundaryRecoveryVersion
//=======================================================================

bool HYBRIDPlugin_Hypothesis::DefaultToUseBoundaryRecoveryVersion()
{
  return false;
}

//=======================================================================
//function : DefaultToUseFEMCorrection
//=======================================================================

bool HYBRIDPlugin_Hypothesis::DefaultToUseFEMCorrection()
{
  return false;
}

//=======================================================================
//function : DefaultToRemoveCentralPoint
//=======================================================================

bool HYBRIDPlugin_Hypothesis::DefaultToRemoveCentralPoint()
{
  return false;
}

//=======================================================================
//function : DefaultGradation
//=======================================================================

double HYBRIDPlugin_Hypothesis::DefaultGradation()
{
  return 1.05;
}

//=======================================================================
//function : DefaultStandardOutputLog
//=======================================================================

bool HYBRIDPlugin_Hypothesis::DefaultStandardOutputLog()
{
  return false;
}

// //=======================================================================
// //function : DefaultID2SizeMap
// //=======================================================================
// 
// HYBRIDPlugin_Hypothesis::TID2SizeMap HYBRIDPlugin_Hypothesis::DefaultID2SizeMap()
// {
//   return HYBRIDPlugin_Hypothesis::TID2SizeMap();
// }

//=======================================================================
//function : DefaultAddMultinormals
//=======================================================================
bool HYBRIDPlugin_Hypothesis::DefaultAddMultinormals()
{
  return false;
}

//=======================================================================
//function : DefaultSmoothNormals
//=======================================================================
bool HYBRIDPlugin_Hypothesis::DefaultSmoothNormals()
{
  return false;
}

//=======================================================================
//function : DefaultHeightFirstLayer
//=======================================================================
double HYBRIDPlugin_Hypothesis::DefaultHeightFirstLayer()
{
  return 0.0; //or epsilon?
}

//=======================================================================
//function : DefaultBoundaryLayersProgression
//=======================================================================
double HYBRIDPlugin_Hypothesis::DefaultBoundaryLayersProgression()
{
  return 1.0;
}

//=======================================================================
//function : DefaultMultinormalsAngle
//=======================================================================
double HYBRIDPlugin_Hypothesis::DefaultMultinormalsAngle()
{
  return 30.0;
}

//=======================================================================
//function : DefaultNbOfBoundaryLayers
//=======================================================================
short HYBRIDPlugin_Hypothesis::DefaultNbOfBoundaryLayers()
{
  return 1;
}

//=======================================================================
//function : SaveTo
//=======================================================================

std::ostream & HYBRIDPlugin_Hypothesis::SaveTo(std::ostream & save)
{
  save << (int) myBoundaryLayersGrowth << " ";
  save << (int) myElementGeneration << " ";
  save << (int) myAddMultinormals << " ";
  save << (int) mySmoothNormals << " ";
  save << (int) myLayersOnAllWrap << " ";

  save << myNbOfBoundaryLayers << " ";
  save << myHeightFirstLayer << " ";
  save << myBoundaryLayersProgression << " ";
  save << myMultinormalsAngle << " ";

  save << (int) myKeepFiles << " ";
  save << myWorkingDirectory << " ";
  save << myVerboseLevel << " ";
  if (!myTextOption.empty()) {
    save << "__OPTIONS_BEGIN__ ";
    save << myTextOption << " ";
    save << "__OPTIONS_END__ ";
  }
  

  THYBRIDEnforcedVertexList::iterator it  = _enfVertexList.begin();
  if (it != _enfVertexList.end()) {
    save << " " << "__ENFORCED_VERTICES_BEGIN__ ";
    for ( ; it != _enfVertexList.end(); ++it ) {
      THYBRIDEnforcedVertex *enfVertex = (*it);
      save << " " << "__BEGIN_VERTEX__";
      if (!enfVertex->name.empty()) {
        save << " " << "__BEGIN_NAME__";
        save << " " << enfVertex->name;
        save << " " << "__END_NAME__";
      }
      if (!enfVertex->geomEntry.empty()) {
        save << " " << "__BEGIN_ENTRY__";
        save << " " << enfVertex->geomEntry;
        save << " " << enfVertex->isCompound;
        save << " " << "__END_ENTRY__";
      }
      if (!enfVertex->groupName.empty()) {
        save << " " << "__BEGIN_GROUP__";
        save << " " << enfVertex->groupName;
        save << " " << "__END_GROUP__";
      }
      if (enfVertex->coords.size()) {
        save << " " << "__BEGIN_COORDS__";
        for ( size_t i = 0; i < enfVertex->coords.size(); i++ )
          save << " " << enfVertex->coords[i];
        save << " " << "__END_COORDS__";
      }
      save << " " << "__BEGIN_SIZE__";
      save << " " << enfVertex->size;
      save << " " << "__END_SIZE__";
      save << " " << "__END_VERTEX__";
    }
    save << " " << "__ENFORCED_VERTICES_END__ ";
  }

  THYBRIDEnforcedMeshList::iterator it_mesh  = _enfMeshList.begin();
  if (it_mesh != _enfMeshList.end()) {
    save << " " << "__ENFORCED_MESHES_BEGIN__ ";
    for ( ; it_mesh != _enfMeshList.end(); ++it_mesh ) {
      THYBRIDEnforcedMesh *enfMesh = (*it_mesh);
      save << " " << "__BEGIN_ENF_MESH__";

      save << " " << "__BEGIN_NAME__";
      save << " " << enfMesh->name;
      save << " " << "__END_NAME__";

      save << " " << "__BEGIN_ENTRY__";
      save << " " << enfMesh->entry;
      save << " " << "__END_ENTRY__";

      save << " " << "__BEGIN_ELEM_TYPE__";
      save << " " << (int)enfMesh->elementType;
      save << " " << "__END_ELEM_TYPE__";

      if (!enfMesh->groupName.empty()) {
        save << " " << "__BEGIN_GROUP__";
        save << " " << enfMesh->groupName;
        save << " " << "__END_GROUP__";
      }
      save << " " << "__PERSIST_ID__";
      save << " " << enfMesh->persistID;
      save << " " << "__END_ENF_MESH__";
      std::cout << "Saving of enforced mesh " << enfMesh->name.c_str() << " done" << std::endl;
    }
    save << " "  << "__ENFORCED_MESHES_END__ ";
  }
  return save;
}

//=======================================================================
//function : LoadFrom
//=======================================================================

std::istream & HYBRIDPlugin_Hypothesis::LoadFrom(std::istream & load)
{
  bool isOK = true;
  int i;
  double d;

  isOK = static_cast<bool>(load >> i);
  if (isOK)
    myBoundaryLayersGrowth = (short) i;
  else
    load.clear(ios::badbit | load.rdstate());

  isOK = static_cast<bool>(load >> i);
  if (isOK)
    myElementGeneration = (short) i;
  else
    load.clear(ios::badbit | load.rdstate());

  isOK = static_cast<bool>(load >> i);
  if (isOK)
    myAddMultinormals = (bool) i;
  else
    load.clear(ios::badbit | load.rdstate());

  isOK = static_cast<bool>(load >> i);
  if (isOK)
    mySmoothNormals = (bool) i;
  else
    load.clear(ios::badbit | load.rdstate());

  isOK = static_cast<bool>(load >> i);
  if (isOK)
    myLayersOnAllWrap = (bool) i;
  else
    load.clear(ios::badbit | load.rdstate());

  isOK = static_cast<bool>(load >> i);
  if (isOK)
    myNbOfBoundaryLayers = (short) i;
  else
    load.clear(ios::badbit | load.rdstate());

  isOK = static_cast<bool>(load >> d);
  if (isOK)
    myHeightFirstLayer = d;
  else
    load.clear(ios::badbit | load.rdstate());

  isOK = static_cast<bool>(load >> d);
  if (isOK)
    myBoundaryLayersProgression = d;
  else
    load.clear(ios::badbit | load.rdstate());

  isOK = static_cast<bool>(load >> d);
  if (isOK)
    myMultinormalsAngle = d;
  else
    load.clear(ios::badbit | load.rdstate());

  isOK = static_cast<bool>(load >> i);
  if (isOK)
    myKeepFiles = (bool) i;
  else
    load.clear(ios::badbit | load.rdstate());

  isOK = static_cast<bool>(load >> myWorkingDirectory);
  if (isOK) {
    if ( myWorkingDirectory == "0") { // myWorkingDirectory was empty
      myKeepFiles = false;
      myWorkingDirectory.clear();
    }
    else if ( myWorkingDirectory == "1" ) {
      myKeepFiles = true;
      myWorkingDirectory.clear();
    }
  }
  else
    load.clear(ios::badbit | load.rdstate());

  isOK = static_cast<bool>(load >> i);
  if (isOK)
    myVerboseLevel = (short) i;
  else
    load.clear(ios::badbit | load.rdstate());


  std::string separator;
  bool hasOptions = false;
  bool hasEnforcedVertices = false;
  bool hasEnforcedMeshes = false;
  isOK = static_cast<bool>(load >> separator);

  if ( isOK && ( separator == "0" || separator == "1" ))
  {
    myToMakeGroupsOfDomains = ( separator == "1" );
    isOK = static_cast<bool>(load >> separator);
  }

  if (isOK) {
    if (separator == "__OPTIONS_BEGIN__")
      hasOptions = true;
    else if (separator == "__ENFORCED_VERTICES_BEGIN__")
      hasEnforcedVertices = true;
    else if (separator == "__ENFORCED_MESHES_BEGIN__")
      hasEnforcedMeshes = true;
  }

  if (hasOptions) {
    std::string txt;
    while (isOK) {
      isOK = static_cast<bool>(load >> txt);
      if (isOK) {
        if (txt == "__OPTIONS_END__") {
          if (!myTextOption.empty()) {
            // Remove last space
            myTextOption.erase(myTextOption.end()-1);
          }
          isOK = false;
          break;
        }
        myTextOption += txt;
        myTextOption += " ";
      }
    }
  }

  if (hasOptions) {
    isOK = static_cast<bool>(load >> separator);
    if (isOK && separator == "__ENFORCED_VERTICES_BEGIN__")
      hasEnforcedVertices = true;
    if (isOK && separator == "__ENFORCED_MESHES_BEGIN__")
      hasEnforcedMeshes = true;
  }

  if (hasEnforcedVertices) {
    std::string txt, name, entry, groupName;
    double size, coords[3];
    bool isCompound;
    bool hasCoords = false;
    isOK = static_cast<bool>(load >> txt);  // __BEGIN_VERTEX__
    while (isOK) {
      if (txt == "__ENFORCED_VERTICES_END__")
        isOK = false;

      THYBRIDEnforcedVertex *enfVertex = new THYBRIDEnforcedVertex();
      while (isOK) {
        isOK = static_cast<bool>(load >> txt);
        if (txt == "__END_VERTEX__") {
          enfVertex->name = name;
          enfVertex->geomEntry = entry;
          enfVertex->isCompound = isCompound;
          enfVertex->groupName = groupName;
          enfVertex->coords.clear();
          if (hasCoords)
            enfVertex->coords.assign(coords,coords+3);

          _enfVertexList.insert(enfVertex);

          if (enfVertex->coords.size())
            _coordsEnfVertexMap[enfVertex->coords] = enfVertex;
          if (!enfVertex->geomEntry.empty())
            _geomEntryEnfVertexMap[enfVertex->geomEntry] = enfVertex;

          name.clear();
          entry.clear();
          groupName.clear();
          hasCoords = false;
          isOK = false;
        }

        if (txt == "__BEGIN_NAME__") {  // __BEGIN_NAME__
          while (isOK && (txt != "__END_NAME__")) {
            isOK = static_cast<bool>(load >> txt);
            if (txt != "__END_NAME__") {
              if (!name.empty())
                name += " ";
              name += txt;
            }
          }
          MESSAGE("name: " <<name);
        }

        if (txt == "__BEGIN_ENTRY__") {  // __BEGIN_ENTRY__
          isOK = static_cast<bool>(load >> entry);
          isOK = static_cast<bool>(load >> isCompound);
          isOK = static_cast<bool>(load >> txt); // __END_ENTRY__
          if (txt != "__END_ENTRY__")
            throw std::exception();
          MESSAGE("entry: " << entry);
        }

        if (txt == "__BEGIN_GROUP__") {  // __BEGIN_GROUP__
          while (isOK && (txt != "__END_GROUP__")) {
            isOK = static_cast<bool>(load >> txt);
            if (txt != "__END_GROUP__") {
              if (!groupName.empty())
                groupName += " ";
              groupName += txt;
            }
          }
          MESSAGE("groupName: " << groupName);
        }

        if (txt == "__BEGIN_COORDS__") {  // __BEGIN_COORDS__
          hasCoords = true;
          isOK = static_cast<bool>(load >> coords[0] >> coords[1] >> coords[2]);
          isOK = static_cast<bool>(load >> txt); // __END_COORDS__
          if (txt != "__END_COORDS__")
            throw std::exception();
          MESSAGE("coords: " << coords[0] <<","<< coords[1] <<","<< coords[2]);
        }

        if (txt == "__BEGIN_SIZE__") {  // __BEGIN_ENTRY__
          isOK = static_cast<bool>(load >> size);
          isOK = static_cast<bool>(load >> txt); // __END_ENTRY__
          if (txt != "__END_SIZE__") {
            throw std::exception();
          }
          MESSAGE("size: " << size);
        }
      }
      isOK = static_cast<bool>(load >> txt);  // __BEGIN_VERTEX__
    }
  }

  if (hasEnforcedVertices) {
    isOK = static_cast<bool>(load >> separator);
    if (isOK && separator == "__ENFORCED_MESHES_BEGIN__")
      hasEnforcedMeshes = true;
  }

  if (hasEnforcedMeshes) {
    std::string txt, name, entry, groupName;
    int elementType = -1, persistID = -1;
    isOK = static_cast<bool>(load >> txt);  // __BEGIN_ENF_MESH__
    while (isOK) {
      //                if (isOK) {
      if (txt == "__ENFORCED_MESHES_END__")
        isOK = false;

      THYBRIDEnforcedMesh *enfMesh = new THYBRIDEnforcedMesh();
      while (isOK) {
        isOK = static_cast<bool>(load >> txt);
        if (txt == "__END_ENF_MESH__") {
          enfMesh->name = name;
          enfMesh->entry = entry;
          enfMesh->elementType = (SMESH::ElementType)elementType;
          enfMesh->groupName = groupName;
          enfMesh->persistID = persistID;

          _enfMeshList.insert(enfMesh);
          std::cout << "Restoring of enforced mesh " <<name  << " done" << std::endl;

          name.clear();
          entry.clear();
          elementType = -1;
          groupName.clear();
          persistID = -1;
          isOK = false;
        }

        if (txt == "__BEGIN_NAME__") {  // __BEGIN_NAME__
          while (isOK && (txt != "__END_NAME__")) {
            isOK = static_cast<bool>(load >> txt);
            if (txt != "__END_NAME__") {
              if (!name.empty())
                name += " ";
              name += txt;
            }
          }
          MESSAGE("name: " <<name);
        }

        if (txt == "__BEGIN_ENTRY__") {  // __BEGIN_ENTRY__
          isOK = static_cast<bool>(load >> entry);
          isOK = static_cast<bool>(load >> txt); // __END_ENTRY__
          if (txt != "__END_ENTRY__")
            throw std::exception();
          MESSAGE("entry: " << entry);
        }

        if (txt == "__BEGIN_ELEM_TYPE__") {  // __BEGIN_ELEM_TYPE__
          isOK = static_cast<bool>(load >> elementType);
          isOK = static_cast<bool>(load >> txt); // __END_ELEM_TYPE__
          if (txt != "__END_ELEM_TYPE__")
            throw std::exception();
          MESSAGE("elementType: " << elementType);
        }

        if (txt == "__BEGIN_GROUP__") {  // __BEGIN_GROUP__
          while (isOK && (txt != "__END_GROUP__")) {
            isOK = static_cast<bool>(load >> txt);
            if (txt != "__END_GROUP__") {
              if (!groupName.empty())
                groupName += " ";
              groupName += txt;
            }
          } // while
          MESSAGE("groupName: " << groupName);
        } // if

        if (txt == "__PERSIST_ID__") {
          isOK = static_cast<bool>(load >> persistID);
          MESSAGE("persistID: " << persistID);
        }
        std::cout << "isOK: " << isOK << std::endl;
      } // while
      //                } // if
      isOK = static_cast<bool>(load >> txt);  // __BEGIN_ENF_MESH__
    } // while
  } // if

  return load;
}

//=======================================================================
//function : SetParametersByMesh
//=======================================================================

bool HYBRIDPlugin_Hypothesis::SetParametersByMesh(const SMESH_Mesh* ,const TopoDS_Shape&)
{
  return false;
}


//================================================================================
/*!
 * \brief Sets myToMakeGroupsOfDomains depending on whether theMesh is on shape or not
 */
//================================================================================

bool HYBRIDPlugin_Hypothesis::SetParametersByDefaults(const TDefaults&  dflts,
                                                     const SMESH_Mesh* /*theMesh*/)
{
  myToMakeGroupsOfDomains = ( !dflts._shape || dflts._shape->IsNull() );
  return true;
}

//================================================================================
/*!
 * \brief Return command to run hybrid mesher excluding file prefix (-f)
 */
//================================================================================

std::string HYBRIDPlugin_Hypothesis::CommandToRun(const HYBRIDPlugin_Hypothesis* hyp,
                                                  const bool         hasShapeToMesh)
{
  TCollection_AsciiString cmd = GetExeName().c_str();
  // check if any option is overridden by hyp->myTextOption
  bool p_h     = ( hyp && hyp->myTextOption.find("-h")  != std::string::npos );
  bool p_v     = ( hyp && hyp->myTextOption.find("-v ")  != std::string::npos );
  bool p_i     = ( hyp && hyp->myTextOption.find("-i")  != std::string::npos );
  bool p_o     = ( hyp && hyp->myTextOption.find("-o")  != std::string::npos );
  bool p_mnot  = ( hyp && hyp->myTextOption.find("--max_number_of_threads ")  != std::string::npos );
  bool p_blsi  = ( hyp && hyp->myTextOption.find("--boundary_layer_surface_tags ")  != std::string::npos );
  bool p_blii  = ( hyp && hyp->myTextOption.find("--boundary_layer_imprint_tags ")  != std::string::npos );
  bool p_blsd  = ( hyp && hyp->myTextOption.find("--normal_direction ")  != std::string::npos );
  bool p_hotfl = ( hyp && hyp->myTextOption.find("--boundary_layer_global_initial_height ")  != std::string::npos );
  bool p_nobl  = ( hyp && hyp->myTextOption.find("--number_of_boundary_layers ")  != std::string::npos );
  bool p_blgp  = ( hyp && hyp->myTextOption.find("--boundary_layer_geometric_progression ")  != std::string::npos );
  bool p_eg    = ( hyp && hyp->myTextOption.find("--element_generation ")  != std::string::npos );
  bool p_cm    = ( hyp && hyp->myTextOption.find("--collision_mode ")  != std::string::npos );
  bool p_am    = ( hyp && hyp->myTextOption.find("--add_multinormals ")  != std::string::npos );
  bool p_mat   = ( hyp && hyp->myTextOption.find("--multinormal_angle_threshold ")  != std::string::npos );
  bool p_sn    = ( hyp && hyp->myTextOption.find("--smooth_normals ")  != std::string::npos );

  //missing options :
  //- global_physical_size
  //- boundary_layer_size_mode
  //- boundary_layer_initial_height_on_surface_tags
  //- boundary_layer_max_element_angle

  bool nolayers = false;
  bool layersOnAllWrap = hyp ? hyp->myLayersOnAllWrap : DefaultLayersOnAllWrap();

  //help mode
  if ( p_h ) {
    cmd += " --help ";
#ifdef WIN32
    cmd += " < NUL";
#endif
    std::cout << "!!!!! CommandToRun help only !!!! " << cmd.ToCString() << std::endl;
    return cmd.ToCString();
  }

  if ( !p_v && hyp ) {
    cmd += " --verbose ";
    cmd += hyp->myVerboseLevel;
  }

  if ( !p_mnot && hyp ) {
    cmd += " --max_number_of_threads ";
    cmd += 8; //TODO getenv NB CPU
  }
  
  //if ( !p_blsi && hyp ) {
  //  cmd += " --boundary_layers_surface_ids ";
  //  cmd += 0; //TODO hyp->my;
  //}
  
  //if ( !p_blii && hyp ) {
  //  cmd += " --boundary_layers_imprint_ids ";
  //  cmd += 0; //TODO hyp->my;
  //}
  
  //no layers?
  if ( !p_nobl && hyp ) {
    if ( hyp->myNbOfBoundaryLayers < 1 ) nolayers = true;
  }
  if ( !p_hotfl && hyp ) {
    if ( hyp->myHeightFirstLayer < 1e-50 ) nolayers = true;
  }
    
  if ( !p_blsd && hyp ) {
    if ( hyp->myBoundaryLayersGrowth >= 0 && hyp->myBoundaryLayersGrowth <= 1 ) {
      const char* value[] = { "1" , "-1" };
      cmd += " --normal_direction ";
      cmd += value[ hyp->myBoundaryLayersGrowth ];
    }
  }
  
  if ( !p_hotfl && hyp ) {
    cmd += " --boundary_layer_global_initial_height ";
    cmd += hyp->myHeightFirstLayer;
  }
  
  if ( !p_nobl && hyp ) {
    cmd += " --number_of_boundary_layers ";
    if ( nolayers )
      cmd += 0;
    else
      cmd += hyp->myNbOfBoundaryLayers;
  }
  
  if ( !p_blgp && hyp ) {
    cmd += " --boundary_layer_geometric_progression ";
    cmd += hyp->myBoundaryLayersProgression;
  }

  //--boundary_layers_surface_ids 5 #for all wrap shell
  //--boundary_layers_surface_ids 6 #for all enfMeshList
  //TODO? if ( !nolayers ) cmd += " --boundary_layers_surface_ids 5,6 "; //as all wrap shell and triangles of enforced mesh
  if ( !nolayers ) {
    if (layersOnAllWrap)
      cmd += " --boundary_layer_surface_tags 5 "; //as triangles of all wrap
    else
      cmd += " --boundary_layer_surface_tags 6 "; //as triangles of enforced mesh
  }

  if ( !p_eg && hyp ) {
    if ( hyp->myElementGeneration >= 0 && hyp->myElementGeneration <= 1 ) {
      const char* value[] = { "tetra-dominant" , "hexa-dominant" };
      cmd += " --element_generation ";
      cmd += value[ hyp->myElementGeneration ];
    }
  }
  
  if ( !p_cm && hyp ) {
    if ( hyp->myCollisionMode >= 0 && hyp->myCollisionMode <= 1 ) {
      const char* value[] = { "decrease" , "stop" };
      cmd += " --collision_mode ";
      cmd += value[ hyp->myCollisionMode ];
    }
  }
  
  if ( !p_am && hyp ) {
    int res = hyp->myAddMultinormals ? 0 : 1 ;
    const char* value[] = { "yes" , "no" };
    cmd += " --add_multinormals ";
    cmd += value[ res ];
  }
  
  if ( !p_mat && hyp ) {
    cmd += " --multinormal_angle_threshold ";
    cmd += hyp->myMultinormalsAngle;
  }
  
  if ( !p_sn && hyp ) {
    int res = hyp->mySmoothNormals ? 0 : 1 ;
    const char* value[] = { "yes" , "no" };
    cmd += " --smooth_normals ";
    cmd += value[ res ];
  }
  
#ifdef WIN32
  cmd += " < NUL";
#endif
  //std::cout << "!!!!!CommandToRun end " << cmd.ToCString() << std::endl;
    
  return cmd.ToCString();
}

//================================================================================
/*!
 * \brief Return a unique file name
 */
//================================================================================

std::string HYBRIDPlugin_Hypothesis::GetFileName(const HYBRIDPlugin_Hypothesis* hyp)
{
  std::string aTmpDir = hyp ? hyp->GetWorkingDirectory() : DefaultWorkingDirectory();
  const char lastChar = *aTmpDir.rbegin();
#ifdef WIN32
    if(lastChar != '\\') aTmpDir+='\\';
#else
    if(lastChar != '/') aTmpDir+='/';
#endif

  TCollection_AsciiString aGenericName = (char*)aTmpDir.c_str();
  aGenericName += "HYBRID_";
  aGenericName += getpid();
  aGenericName += "_";
  aGenericName += Abs((Standard_Integer)(long) aGenericName.ToCString());

  return aGenericName.ToCString();
}

//================================================================================
/*
 * Return the name of executable
 */
//================================================================================

std::string HYBRIDPlugin_Hypothesis::GetExeName()
{
  //call mg-hybrid.bash is script which assumes new project version(s) mg-hybrid.exe in the prerequisite base and special? licence.
#ifdef WIN32
  return "mg-hybrid.exe";
#else
  return "mg-hybrid.bash";
#endif
}

//================================================================================
/*!
* \brief Return the enforced vertices
*/
//================================================================================

HYBRIDPlugin_Hypothesis::THYBRIDEnforcedVertexList HYBRIDPlugin_Hypothesis::GetEnforcedVertices(const HYBRIDPlugin_Hypothesis* hyp)
{
  return hyp ? hyp->_GetEnforcedVertices():DefaultHYBRIDEnforcedVertexList();
}

HYBRIDPlugin_Hypothesis::THYBRIDEnforcedVertexCoordsValues HYBRIDPlugin_Hypothesis::GetEnforcedVerticesCoordsSize (const HYBRIDPlugin_Hypothesis* hyp)
{  
  return hyp ? hyp->_GetEnforcedVerticesCoordsSize(): DefaultHYBRIDEnforcedVertexCoordsValues();
}

HYBRIDPlugin_Hypothesis::THYBRIDEnforcedVertexEntryValues HYBRIDPlugin_Hypothesis::GetEnforcedVerticesEntrySize (const HYBRIDPlugin_Hypothesis* hyp)
{  
  return hyp ? hyp->_GetEnforcedVerticesEntrySize(): DefaultHYBRIDEnforcedVertexEntryValues();
}

HYBRIDPlugin_Hypothesis::TCoordsHYBRIDEnforcedVertexMap HYBRIDPlugin_Hypothesis::GetEnforcedVerticesByCoords (const HYBRIDPlugin_Hypothesis* hyp)
{  
  return hyp ? hyp->_GetEnforcedVerticesByCoords(): DefaultCoordsHYBRIDEnforcedVertexMap();
}

HYBRIDPlugin_Hypothesis::TGeomEntryHYBRIDEnforcedVertexMap HYBRIDPlugin_Hypothesis::GetEnforcedVerticesByEntry (const HYBRIDPlugin_Hypothesis* hyp)
{  
  return hyp ? hyp->_GetEnforcedVerticesByEntry(): DefaultGeomEntryHYBRIDEnforcedVertexMap();
}

HYBRIDPlugin_Hypothesis::TIDSortedNodeGroupMap HYBRIDPlugin_Hypothesis::GetEnforcedNodes(const HYBRIDPlugin_Hypothesis* hyp)
{
  return hyp ? hyp->_GetEnforcedNodes():DefaultIDSortedNodeGroupMap();
}

HYBRIDPlugin_Hypothesis::TIDSortedElemGroupMap HYBRIDPlugin_Hypothesis::GetEnforcedEdges(const HYBRIDPlugin_Hypothesis* hyp)
{
  return hyp ? hyp->_GetEnforcedEdges():DefaultIDSortedElemGroupMap();
}

HYBRIDPlugin_Hypothesis::TIDSortedElemGroupMap HYBRIDPlugin_Hypothesis::GetEnforcedTriangles(const HYBRIDPlugin_Hypothesis* hyp)
{
  return hyp ? hyp->_GetEnforcedTriangles():DefaultIDSortedElemGroupMap();
}

HYBRIDPlugin_Hypothesis::TID2SizeMap HYBRIDPlugin_Hypothesis::GetNodeIDToSizeMap(const HYBRIDPlugin_Hypothesis* hyp)
{
  return hyp ? hyp->_GetNodeIDToSizeMap(): DefaultID2SizeMap();
}

HYBRIDPlugin_Hypothesis::TSetStrings HYBRIDPlugin_Hypothesis::GetGroupsToRemove(const HYBRIDPlugin_Hypothesis* hyp)
{
  return hyp ? hyp->_GetGroupsToRemove(): DefaultGroupsToRemove();
}
