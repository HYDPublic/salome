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
// File   : HYBRIDPlugin_HYBRID.cxx
// Author : Christian VAN WAMBEKE (CEA) (from GHS3D plugin V730)
// ---
//
#include "HYBRIDPlugin_HYBRID.hxx"
#include "HYBRIDPlugin_Hypothesis.hxx"
#include "MG_HYBRID_API.hxx"

#include <SMDS_FaceOfNodes.hxx>
#include <SMDS_LinearEdge.hxx>
#include <SMDS_VolumeOfNodes.hxx>
#include <SMESHDS_Group.hxx>
#include <SMESHDS_Mesh.hxx>
#include <SMESH_Comment.hxx>
#include <SMESH_File.hxx>
#include <SMESH_Group.hxx>
#include <SMESH_HypoFilter.hxx>
#include <SMESH_Mesh.hxx>
#include <SMESH_MeshAlgos.hxx>
#include <SMESH_MeshEditor.hxx>
#include <SMESH_MesherHelper.hxx>
#include <SMESH_ProxyMesh.hxx>
#include <SMESH_subMeshEventListener.hxx>
// #include <StdMeshers_QuadToTriaAdaptor.hxx>
// #include <StdMeshers_ViscousLayers.hxx>

#include <BRepAdaptor_Surface.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepClass3d.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRepGProp.hxx>
#include <BRepTools.hxx>
#include <BRep_Tool.hxx>
#include <Bnd_Box.hxx>
#include <GProp_GProps.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <Precision.hxx>
#include <Standard_ErrorHandler.hxx>
#include <Standard_Failure.hxx>
#include <Standard_ProgramError.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Solid.hxx>

#include <Basics_Utils.hxx>
#include <utilities.h>

#include <algorithm>

#define castToNode(n) static_cast<const SMDS_MeshNode *>( n );

#ifndef GMFVERSION
#define GMFVERSION GmfDouble
#endif
#define GMFDIMENSION 3

#define HOLE_ID -1

typedef const std::list<const SMDS_MeshFace*> TTriaList;

static const char theDomainGroupNamePrefix[] = "Domain_";

static void removeFile( const TCollection_AsciiString& fileName )
{
  try {
    SMESH_File( fileName.ToCString() ).remove();
  }
  catch ( ... ) {
    MESSAGE("Can't remove file: " << fileName.ToCString() << " ; file does not exist or permission denied");
  }
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

HYBRIDPlugin_HYBRID::HYBRIDPlugin_HYBRID(int hypId, int studyId, SMESH_Gen* gen)
  : SMESH_3D_Algo(hypId, studyId, gen)
{
  MESSAGE("HYBRIDPlugin_HYBRID::HYBRIDPlugin_HYBRID");
  _name = Name();
  _shapeType = (1 << TopAbs_SHELL) | (1 << TopAbs_SOLID);// 1 bit /shape type
  _onlyUnaryInput = true; // Compute() will be called on each solid
  _iShape=0;
  _nbShape=0;
  _compatibleHypothesis.push_back( HYBRIDPlugin_Hypothesis::GetHypType());
  //_compatibleHypothesis.push_back( StdMeshers_ViscousLayers::GetHypType() );
  _requireShape = false; // can work without shape_studyId

  smeshGen_i = SMESH_Gen_i::GetSMESHGen();
  CORBA::Object_var anObject = smeshGen_i->GetNS()->Resolve("/myStudyManager");
  SALOMEDS::StudyManager_var aStudyMgr = SALOMEDS::StudyManager::_narrow(anObject);

  MESSAGE("studyid = " << _studyId);

  myStudy = NULL;
  myStudy = aStudyMgr->GetStudyByID(_studyId);
  if (!myStudy->_is_nil())
    MESSAGE("myStudy->StudyId() = " << myStudy->StudyId());
  
  _computeCanceled = false;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

HYBRIDPlugin_HYBRID::~HYBRIDPlugin_HYBRID()
{
  MESSAGE("HYBRIDPlugin_HYBRID::~HYBRIDPlugin_HYBRID");
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

bool HYBRIDPlugin_HYBRID::CheckHypothesis ( SMESH_Mesh&         aMesh,
                                          const TopoDS_Shape& aShape,
                                          Hypothesis_Status&  aStatus )
{
  aStatus = SMESH_Hypothesis::HYP_OK;

  _hyp = 0;
  //_viscousLayersHyp = 0;
  _keepFiles = false;
  _removeLogOnSuccess = true;
  _logInStandardOutput = false;

  const std::list <const SMESHDS_Hypothesis * >& hyps =
    GetUsedHypothesis(aMesh, aShape, /*ignoreAuxiliary=*/false);
  std::list <const SMESHDS_Hypothesis* >::const_iterator h = hyps.begin();
  for ( ; h != hyps.end(); ++h )
  {
    if ( !_hyp )
      _hyp = dynamic_cast< const HYBRIDPlugin_Hypothesis*> ( *h );
    // if ( !_viscousLayersHyp )
    //   _viscousLayersHyp = dynamic_cast< const StdMeshers_ViscousLayers*> ( *h );
  }
  if ( _hyp )
  {
    _keepFiles = _hyp->GetKeepFiles();
    _removeLogOnSuccess = _hyp->GetRemoveLogOnSuccess();
    _logInStandardOutput = _hyp->GetStandardOutputLog();
  }

  return true;
}


//=======================================================================
//function : entryToShape
//purpose  : 
//=======================================================================

TopoDS_Shape HYBRIDPlugin_HYBRID::entryToShape(std::string entry)
{
  MESSAGE("HYBRIDPlugin_HYBRID::entryToShape "<<entry );
  if ( myStudy->_is_nil() )
    throw SALOME_Exception("MG-HYBRID plugin can't work w/o publishing in the study");
  GEOM::GEOM_Object_var aGeomObj;
  TopoDS_Shape S = TopoDS_Shape();
  SALOMEDS::SObject_var aSObj = myStudy->FindObjectID( entry.c_str() );
  if (!aSObj->_is_nil() ) {
    CORBA::Object_var obj = aSObj->GetObject();
    aGeomObj = GEOM::GEOM_Object::_narrow(obj);
    aSObj->UnRegister();
  }
  if ( !aGeomObj->_is_nil() )
    S = smeshGen_i->GeomObjectToShape( aGeomObj.in() );
  return S;
}

//=======================================================================
//function : findShape
//purpose  : 
//=======================================================================

// static TopoDS_Shape findShape(const SMDS_MeshNode *aNode[],
//                               TopoDS_Shape        aShape,
//                               const TopoDS_Shape  shape[],
//                               double**            box,
//                               const int           nShape,
//                               TopAbs_State *      state = 0)
// {
//   gp_XYZ aPnt(0,0,0);
//   int j, iShape, nbNode = 4;

//   for ( j=0; j<nbNode; j++ ) {
//     gp_XYZ p ( aNode[j]->X(), aNode[j]->Y(), aNode[j]->Z() );
//     if ( aNode[j]->GetPosition()->GetTypeOfPosition() == SMDS_TOP_3DSPACE ) {
//       aPnt = p;
//       break;
//     }
//     aPnt += p / nbNode;
//   }

//   BRepClass3d_SolidClassifier SC (aShape, aPnt, Precision::Confusion());
//   if (state) *state = SC.State();
//   if ( SC.State() != TopAbs_IN || aShape.IsNull() || aShape.ShapeType() != TopAbs_SOLID) {
//     for (iShape = 0; iShape < nShape; iShape++) {
//       aShape = shape[iShape];
//       if ( !( aPnt.X() < box[iShape][0] || box[iShape][1] < aPnt.X() ||
//               aPnt.Y() < box[iShape][2] || box[iShape][3] < aPnt.Y() ||
//               aPnt.Z() < box[iShape][4] || box[iShape][5] < aPnt.Z()) ) {
//         BRepClass3d_SolidClassifier SC (aShape, aPnt, Precision::Confusion());
//         if (state) *state = SC.State();
//         if (SC.State() == TopAbs_IN)
//           break;
//       }
//     }
//   }
//   return aShape;
// }

//=======================================================================
//function : readMapIntLine
//purpose  : 
//=======================================================================

// static char* readMapIntLine(char* ptr, int tab[]) {
//   long int intVal;
//   std::cout << std::endl;

//   for ( int i=0; i<17; i++ ) {
//     intVal = strtol(ptr, &ptr, 10);
//     if ( i < 3 )
//       tab[i] = intVal;
//   }
//   return ptr;
// }

//================================================================================
/*!
 * \brief returns true if a triangle defined by the nodes is a temporary face on a
 * side facet of pyramid and defines sub-domain inside the pyramid
 */
//================================================================================

// static bool isTmpFace(const SMDS_MeshNode* node1,
//                       const SMDS_MeshNode* node2,
//                       const SMDS_MeshNode* node3)
// {
//   // find a pyramid sharing the 3 nodes
//   //const SMDS_MeshElement* pyram = 0;
//   SMDS_ElemIteratorPtr vIt1 = node1->GetInverseElementIterator(SMDSAbs_Volume);
//   while ( vIt1->more() )
//   {
//     const SMDS_MeshElement* pyram = vIt1->next();
//     if ( pyram->NbCornerNodes() != 5 ) continue;
//     int i2, i3;
//     if ( (i2 = pyram->GetNodeIndex( node2 )) >= 0 &&
//          (i3 = pyram->GetNodeIndex( node3 )) >= 0 )
//     {
//       // Triangle defines sub-domian inside the pyramid if it's
//       // normal points out of the pyram

//       // make i2 and i3 hold indices of base nodes of the pyram while
//       // keeping the nodes order in the triangle
//       const int iApex = 4;
//       if ( i2 == iApex )
//         i2 = i3, i3 = pyram->GetNodeIndex( node1 );
//       else if ( i3 == iApex )
//         i3 = i2, i2 = pyram->GetNodeIndex( node1 );

//       int i3base = (i2+1) % 4; // next index after i2 within the pyramid base
//       return ( i3base != i3 );
//     }
//   }
//   return false;
// }

//=======================================================================
//function : findShapeID
//purpose  : find the solid corresponding to HYBRID sub-domain following
//           the technique proposed in GHS3D manual (available within
//           ghs3d installation) in chapter "B.4 Subdomain (sub-region) assignment".
//           In brief: normal of the triangle defined by the given nodes
//           points out of the domain it is associated to
//=======================================================================

// static int findShapeID(SMESH_Mesh&          mesh,
//                        const SMDS_MeshNode* node1,
//                        const SMDS_MeshNode* node2,
//                        const SMDS_MeshNode* node3,
//                        const bool           toMeshHoles)
// {
//   const int invalidID = 0;
//   SMESHDS_Mesh* meshDS = mesh.GetMeshDS();

//   // face the nodes belong to
//   std::vector<const SMDS_MeshNode *> nodes(3);
//   nodes[0] = node1;
//   nodes[1] = node2;
//   nodes[2] = node3;
//   const SMDS_MeshElement * face = meshDS->FindElement( nodes, SMDSAbs_Face, /*noMedium=*/true);
//   if ( !face )
//     return isTmpFace(node1, node2, node3) ? HOLE_ID : invalidID;
// #ifdef _DEBUG_
//   std::cout << "bnd face " << face->GetID() << " - ";
// #endif
//   // geom face the face assigned to
//   SMESH_MeshEditor editor(&mesh);
//   int geomFaceID = editor.FindShape( face );
//   if ( !geomFaceID )
//     return isTmpFace(node1, node2, node3) ? HOLE_ID : invalidID;
//   TopoDS_Shape shape = meshDS->IndexToShape( geomFaceID );
//   if ( shape.IsNull() || shape.ShapeType() != TopAbs_FACE )
//     return invalidID;
//   TopoDS_Face geomFace = TopoDS::Face( shape );

//   // solids bounded by geom face
//   TopTools_IndexedMapOfShape solids, shells;
//   TopTools_ListIteratorOfListOfShape ansIt = mesh.GetAncestors(geomFace);
//   for ( ; ansIt.More(); ansIt.Next() ) {
//     switch ( ansIt.Value().ShapeType() ) {
//     case TopAbs_SOLID:
//       solids.Add( ansIt.Value() ); break;
//     case TopAbs_SHELL:
//       shells.Add( ansIt.Value() ); break;
//     default:;
//     }
//   }
//   // analyse found solids
//   if ( solids.Extent() == 0 || shells.Extent() == 0)
//     return invalidID;

//   const TopoDS_Solid& solid1 = TopoDS::Solid( solids(1) );
//   if ( solids.Extent() == 1 )
//   {
//     if ( toMeshHoles )
//       return meshDS->ShapeToIndex( solid1 );

//     // - Are we at a hole boundary face?
//     if ( shells(1).IsSame( BRepClass3d::OuterShell( solid1 )) )
//     { // - No, but maybe a hole is bound by two shapes? Does shells(1) touches another shell?
//       bool touch = false;
//       TopExp_Explorer eExp( shells(1), TopAbs_EDGE );
//       // check if any edge of shells(1) belongs to another shell
//       for ( ; eExp.More() && !touch; eExp.Next() ) {
//         ansIt = mesh.GetAncestors( eExp.Current() );
//         for ( ; ansIt.More() && !touch; ansIt.Next() ) {
//           if ( ansIt.Value().ShapeType() == TopAbs_SHELL )
//             touch = ( !ansIt.Value().IsSame( shells(1) ));
//         }
//       }
//       if (!touch)
//         return meshDS->ShapeToIndex( solid1 );
//     }
//   }
//   // find orientation of geom face within the first solid
//   TopExp_Explorer fExp( solid1, TopAbs_FACE );
//   for ( ; fExp.More(); fExp.Next() )
//     if ( geomFace.IsSame( fExp.Current() )) {
//       geomFace = TopoDS::Face( fExp.Current() );
//       break;
//     }
//   if ( !fExp.More() )
//     return invalidID; // face not found

//   // normale to triangle
//   gp_Pnt node1Pnt ( node1->X(), node1->Y(), node1->Z() );
//   gp_Pnt node2Pnt ( node2->X(), node2->Y(), node2->Z() );
//   gp_Pnt node3Pnt ( node3->X(), node3->Y(), node3->Z() );
//   gp_Vec vec12( node1Pnt, node2Pnt );
//   gp_Vec vec13( node1Pnt, node3Pnt );
//   gp_Vec meshNormal = vec12 ^ vec13;
//   if ( meshNormal.SquareMagnitude() < DBL_MIN )
//     return invalidID;

//   // get normale to geomFace at any node
//   bool geomNormalOK = false;
//   gp_Vec geomNormal;
//   SMESH_MesherHelper helper( mesh ); helper.SetSubShape( geomFace );
//   for ( int i = 0; !geomNormalOK && i < 3; ++i )
//   {
//     // find UV of i-th node on geomFace
//     const SMDS_MeshNode* nNotOnSeamEdge = 0;
//     if ( helper.IsSeamShape( nodes[i]->getshapeId() )) {
//       if ( helper.IsSeamShape( nodes[(i+1)%3]->getshapeId() ))
//         nNotOnSeamEdge = nodes[(i+2)%3];
//       else
//         nNotOnSeamEdge = nodes[(i+1)%3];
//     }
//     bool uvOK;
//     gp_XY uv = helper.GetNodeUV( geomFace, nodes[i], nNotOnSeamEdge, &uvOK );
//     // check that uv is correct
//     if (uvOK) {
//       double tol = 1e-6;
//       TopoDS_Shape nodeShape = helper.GetSubShapeByNode( nodes[i], meshDS );
//       if ( !nodeShape.IsNull() )
//         switch ( nodeShape.ShapeType() )
//         {
//         case TopAbs_FACE:   tol = BRep_Tool::Tolerance( TopoDS::Face( nodeShape )); break;
//         case TopAbs_EDGE:   tol = BRep_Tool::Tolerance( TopoDS::Edge( nodeShape )); break;
//         case TopAbs_VERTEX: tol = BRep_Tool::Tolerance( TopoDS::Vertex( nodeShape )); break;
//         default:;
//         }
//       gp_Pnt nodePnt ( nodes[i]->X(), nodes[i]->Y(), nodes[i]->Z() );
//       BRepAdaptor_Surface surface( geomFace );
//       uvOK = ( nodePnt.Distance( surface.Value( uv.X(), uv.Y() )) < 2 * tol );
//       if ( uvOK ) {
//         // normale to geomFace at UV
//         gp_Vec du, dv;
//         surface.D1( uv.X(), uv.Y(), nodePnt, du, dv );
//         geomNormal = du ^ dv;
//         if ( geomFace.Orientation() == TopAbs_REVERSED )
//           geomNormal.Reverse();
//         geomNormalOK = ( geomNormal.SquareMagnitude() > DBL_MIN * 1e3 );
//       }
//     }
//   }
//   if ( !geomNormalOK)
//     return invalidID;

//   // compare normals
//   bool isReverse = ( meshNormal * geomNormal ) < 0;
//   if ( !isReverse )
//     return meshDS->ShapeToIndex( solid1 );

//   if ( solids.Extent() == 1 )
//     return HOLE_ID; // we are inside a hole

//   return meshDS->ShapeToIndex( solids(2) );
// }


//=======================================================================
//function : addElemInMeshGroup
//purpose  : Update or create groups in mesh
//=======================================================================

static void addElemInMeshGroup(SMESH_Mesh*             theMesh,
                               const SMDS_MeshElement* anElem,
                               std::string&            groupName,
                               std::set<std::string>&  groupsToRemove)
{
  if ( !anElem ) return; // issue 0021776

  bool groupDone = false;
  SMESH_Mesh::GroupIteratorPtr grIt = theMesh->GetGroups();
  while (grIt->more()) {
    SMESH_Group * group = grIt->next();
    if ( !group ) continue;
    SMESHDS_GroupBase* groupDS = group->GetGroupDS();
    if ( !groupDS ) continue;
    if ( groupDS->GetType()==anElem->GetType() &&groupName.compare(group->GetName())==0) {
      SMESHDS_Group* aGroupDS = static_cast<SMESHDS_Group*>( groupDS );
      aGroupDS->SMDSGroup().Add(anElem);
      groupDone = true;
//       MESSAGE("Successfully added enforced element to existing group " << groupName);
      break;
    }
  }
  
  if (!groupDone)
  {
    int groupId;
    SMESH_Group* aGroup = theMesh->AddGroup(anElem->GetType(), groupName.c_str(), groupId);
    aGroup->SetName( groupName.c_str() );
    SMESHDS_Group* aGroupDS = static_cast<SMESHDS_Group*>( aGroup->GetGroupDS() );
    aGroupDS->SMDSGroup().Add(anElem);
//     MESSAGE("Successfully created enforced vertex group " << groupName);
    groupDone = true;
  }
  if (!groupDone)
    throw SALOME_Exception(LOCALIZED("A given element was not added to a group"));
}


//=======================================================================
//function : updateMeshGroups
//purpose  : Update or create groups in mesh
//=======================================================================

static void updateMeshGroups(SMESH_Mesh* theMesh, std::set<std::string> groupsToRemove)
{
  SMESH_Mesh::GroupIteratorPtr grIt = theMesh->GetGroups();
  while (grIt->more()) {
    SMESH_Group * group = grIt->next();
    if ( !group ) continue;
    SMESHDS_GroupBase* groupDS = group->GetGroupDS();
    if ( !groupDS ) continue;
    std::string currentGroupName = (std::string)group->GetName();
    if (groupDS->IsEmpty() && groupsToRemove.find(currentGroupName) != groupsToRemove.end()) {
      // Previous group created by enforced elements
      MESSAGE("Delete previous group created by removed enforced elements: " << group->GetName())
      theMesh->RemoveGroup(groupDS->GetID());
    }
  }
}

//=======================================================================
//function : removeEmptyGroupsOfDomains
//purpose  : remove empty groups named "Domain_nb" created due to
//           "To make groups of domains" option.
//=======================================================================

static void removeEmptyGroupsOfDomains(SMESH_Mesh* mesh,
                                       bool        notEmptyAsWell = false)
{
  const char* refName = theDomainGroupNamePrefix;
  const size_t refLen = strlen( theDomainGroupNamePrefix );

  std::list<int> groupIDs = mesh->GetGroupIds();
  std::list<int>::const_iterator id = groupIDs.begin();
  for ( ; id != groupIDs.end(); ++id )
  {
    SMESH_Group* group = mesh->GetGroup( *id );
    if ( !group || ( !group->GetGroupDS()->IsEmpty() && !notEmptyAsWell ))
      continue;
    const char* name = group->GetName();
    char* end;
    // check the name
    if ( strncmp( name, refName, refLen ) == 0 && // starts from refName;
         isdigit( *( name + refLen )) &&          // refName is followed by a digit;
         strtol( name + refLen, &end, 10) >= 0 && // there are only digits ...
         *end == '\0')                            // ... till a string end.
    {
      mesh->RemoveGroup( *id );
    }
  }
}

//================================================================================
/*!
 * \brief Create the groups corresponding to domains
 */
//================================================================================

static void makeDomainGroups( std::vector< std::vector< const SMDS_MeshElement* > >& elemsOfDomain,
                              SMESH_MesherHelper*                                    theHelper)
{
  // int nbDomains = 0;
  // for ( size_t i = 0; i < elemsOfDomain.size(); ++i )
  //   nbDomains += ( elemsOfDomain[i].size() > 0 );

  // if ( nbDomains > 1 )
  for ( size_t iDomain = 0; iDomain < elemsOfDomain.size(); ++iDomain )
  {
    std::vector< const SMDS_MeshElement* > & elems = elemsOfDomain[ iDomain ];
    if ( elems.empty() ) continue;

    // find existing groups
    std::vector< SMESH_Group* > groupOfType( SMDSAbs_NbElementTypes, (SMESH_Group*)NULL );
    const std::string domainName = ( SMESH_Comment( theDomainGroupNamePrefix ) << iDomain );
    SMESH_Mesh::GroupIteratorPtr groupIt = theHelper->GetMesh()->GetGroups();
    while ( groupIt->more() )
    {
      SMESH_Group* group = groupIt->next();
      if ( domainName == group->GetName() &&
           dynamic_cast< SMESHDS_Group* >( group->GetGroupDS()) )
        groupOfType[ group->GetGroupDS()->GetType() ] = group;
    }
    // create and fill the groups
    size_t iElem = 0;
    int groupID;
    do
    {
      SMESH_Group* group = groupOfType[ elems[ iElem ]->GetType() ];
      if ( !group )
        group = theHelper->GetMesh()->AddGroup( elems[ iElem ]->GetType(),
                                                domainName.c_str(), groupID );
      SMDS_MeshGroup& groupDS =
        static_cast< SMESHDS_Group* >( group->GetGroupDS() )->SMDSGroup();

      while ( iElem < elems.size() && groupDS.Add( elems[iElem] ))
        ++iElem;

    } while ( iElem < elems.size() );
  }
}

//=======================================================================
//function : readGMFFile
//purpose  : read GMF file w/o geometry associated to mesh
//=======================================================================

static bool readGMFFile(MG_HYBRID_API*                  MGOutput,
                        const char*                     theFile,
                        HYBRIDPlugin_HYBRID*            theAlgo,
                        SMESH_MesherHelper*             theHelper,
                        std::vector <const SMDS_MeshNode*> &    theNodeByHybridId,
                        std::vector <const SMDS_MeshElement*> & theFaceByHybridId,
                        std::map<const SMDS_MeshNode*,int> & theNodeToHybridIdMap,
                        std::vector<std::string> &      aNodeGroupByHybridId,
                        std::vector<std::string> &      anEdgeGroupByHybridId,
                        std::vector<std::string> &      aFaceGroupByHybridId,
                        std::set<std::string> &         groupsToRemove,
                        bool                            toMakeGroupsOfDomains=false,
                        bool                            toMeshHoles=true)
{
  std::string tmpStr;
  SMESHDS_Mesh* theMeshDS = theHelper->GetMeshDS();
  const bool hasGeom = ( theHelper->GetMesh()->HasShapeToMesh() );

  int nbInitialNodes = theNodeByHybridId.size();
  int nbMeshNodes = theMeshDS->NbNodes();
  
  const bool isQuadMesh = 
    theHelper->GetMesh()->NbEdges( ORDER_QUADRATIC ) ||
    theHelper->GetMesh()->NbFaces( ORDER_QUADRATIC ) ||
    theHelper->GetMesh()->NbVolumes( ORDER_QUADRATIC );
    
#ifdef _DEBUG_
  std::cout << "theNodeByHybridId.size(): " << nbInitialNodes << std::endl;
  std::cout << "theHelper->GetMesh()->NbNodes(): " << nbMeshNodes << std::endl;
  std::cout << "isQuadMesh: " << isQuadMesh << std::endl;
#endif
  
  // ---------------------------------
  // Read generated elements and nodes
  // ---------------------------------

  int nbElem = 0, nbRef = 0;
  int aGMFNodeID = 0;
  std::vector< const SMDS_MeshNode* > GMFNode;
#ifdef _DEBUG_
  std::map<int, std::set<int> > subdomainId2tetraId;
#endif
  std::map <GmfKwdCod,int> tabRef;
  const bool force3d = !hasGeom;
  const int  noID    = 0;

  tabRef[GmfVertices]       = 3; // for new nodes and enforced nodes
  tabRef[GmfCorners]        = 1;
  tabRef[GmfEdges]          = 2; // for enforced edges
  tabRef[GmfRidges]         = 1;
  tabRef[GmfTriangles]      = 3; // for enforced faces
  tabRef[GmfQuadrilaterals] = 4;
  tabRef[GmfTetrahedra]     = 4; // for new tetras
  tabRef[GmfPyramids]       = 5; // for new pyramids
  tabRef[GmfPrisms]         = 6; // for new prisms
  tabRef[GmfHexahedra]      = 8;

  int ver, dim;
  MESSAGE("Read " << theFile << " file");
  int InpMsh = MGOutput->GmfOpenMesh(theFile, GmfRead, &ver, &dim);
  if (!InpMsh)
    return false;
  MESSAGE("Done ");

  // Hybrid is not multi-domain => We can't (and don't need to) read ids of domains in ouput file like in GHS3DPlugin
  // We just need to get the id of the one and only solid
  int solidID = 1;
  if ( hasGeom )
  {
    if ( theHelper->GetSubShape().ShapeType() == TopAbs_SOLID )
      solidID = theHelper->GetSubShapeID();
    else
      solidID = theMeshDS->ShapeToIndex
        ( TopExp_Explorer( theHelper->GetSubShape(), TopAbs_SOLID ).Current() );
  }

  // Issue 0020682. Avoid creating nodes and tetras at place where
  // volumic elements already exist
  SMESH_ElementSearcher* elemSearcher = 0;
  std::vector< const SMDS_MeshElement* > foundVolumes;
  if ( !hasGeom && theHelper->GetMesh()->NbVolumes() > 0 )
    elemSearcher = SMESH_MeshAlgos::GetElementSearcher( *theMeshDS );
  SMESHUtils::Deleter< SMESH_ElementSearcher > elemSearcherDeleter( elemSearcher );

  // IMP 0022172: [CEA 790] create the groups corresponding to domains
  std::vector< std::vector< const SMDS_MeshElement* > > elemsOfDomain;

  int nbVertices = MGOutput->GmfStatKwd(InpMsh, GmfVertices) - nbInitialNodes;
  if ( nbVertices < 0 )
    return false;
  GMFNode.resize( nbVertices + 1 );

  std::map <GmfKwdCod,int>::const_iterator it = tabRef.begin();
  for ( ; it != tabRef.end() ; ++it)
  {
    if(theAlgo->computeCanceled()) {
      MGOutput->GmfCloseMesh(InpMsh);
      return false;
    }
    int dummy;
    GmfKwdCod token = it->first;
    nbRef           = it->second;

    nbElem = MGOutput->GmfStatKwd(InpMsh, token);
    if (nbElem > 0) {
      MGOutput->GmfGotoKwd(InpMsh, token);
      std::cout << "Read " << nbElem;
    }
    else
      continue;

    std::vector<int> id (nbElem*tabRef[token]); // node ids
    std::vector<int> domainID( nbElem ); // domain

    if (token == GmfVertices) {
      (nbElem <= 1) ? tmpStr = " vertex" : tmpStr = " vertices";
      
      int aGMFID;
      float VerTab_f[3];
      double x, y, z;
      const SMDS_MeshNode * aGMFNode;

      for ( int iElem = 0; iElem < nbElem; iElem++ ) {
        if(theAlgo->computeCanceled()) {
          MGOutput->GmfCloseMesh(InpMsh);
          return false;
        }
        if (ver == GmfFloat) {
          MGOutput->GmfGetLin(InpMsh, token, &VerTab_f[0], &VerTab_f[1], &VerTab_f[2], &dummy);
          x = VerTab_f[0];
          y = VerTab_f[1];
          z = VerTab_f[2];
        }
        else {
          MGOutput->GmfGetLin(InpMsh, token, &x, &y, &z, &dummy);
        }
        if (iElem >= nbInitialNodes) {
          if ( elemSearcher &&
                elemSearcher->FindElementsByPoint( gp_Pnt(x,y,z), SMDSAbs_Volume, foundVolumes))
            aGMFNode = 0;
          else
            aGMFNode = theHelper->AddNode(x, y, z);
          
          aGMFID = iElem -nbInitialNodes +1;
          GMFNode[ aGMFID ] = aGMFNode;
          if (aGMFID-1 < (int)aNodeGroupByHybridId.size() && !aNodeGroupByHybridId.at(aGMFID-1).empty())
            addElemInMeshGroup(theHelper->GetMesh(), aGMFNode, aNodeGroupByHybridId.at(aGMFID-1), groupsToRemove);
        }
      }
    }
    else if (token == GmfCorners && nbElem > 0) {
      (nbElem <= 1) ? tmpStr = " corner" : tmpStr = " corners";
      for ( int iElem = 0; iElem < nbElem; iElem++ )
        MGOutput->GmfGetLin(InpMsh, token, &id[iElem*tabRef[token]]);
    }
    else if (token == GmfRidges && nbElem > 0) {
      (nbElem <= 1) ? tmpStr = " ridge" : tmpStr = " ridges";
      for ( int iElem = 0; iElem < nbElem; iElem++ )
        MGOutput->GmfGetLin(InpMsh, token, &id[iElem*tabRef[token]]);
    }
    else if (token == GmfEdges && nbElem > 0) {
      (nbElem <= 1) ? tmpStr = " edge" : tmpStr = " edges";
      for ( int iElem = 0; iElem < nbElem; iElem++ )
        MGOutput->GmfGetLin(InpMsh, token, &id[iElem*tabRef[token]], &id[iElem*tabRef[token]+1], &domainID[iElem]);
    }
    else if (token == GmfTriangles && nbElem > 0) {
      (nbElem <= 1) ? tmpStr = " triangle" : tmpStr = " triangles";
      for ( int iElem = 0; iElem < nbElem; iElem++ )
        MGOutput->GmfGetLin(InpMsh, token, &id[iElem*tabRef[token]], &id[iElem*tabRef[token]+1], &id[iElem*tabRef[token]+2], &domainID[iElem]);
    }
    else if (token == GmfQuadrilaterals && nbElem > 0) {
      (nbElem <= 1) ? tmpStr = " Quadrilateral" : tmpStr = " Quadrilaterals";
      for ( int iElem = 0; iElem < nbElem; iElem++ )
        MGOutput->GmfGetLin(InpMsh, token, &id[iElem*tabRef[token]], &id[iElem*tabRef[token]+1], &id[iElem*tabRef[token]+2], &id[iElem*tabRef[token]+3], &domainID[iElem]);
    }
    else if (token == GmfTetrahedra && nbElem > 0) {
      (nbElem <= 1) ? tmpStr = " Tetrahedron" : tmpStr = " Tetrahedra";
      for ( int iElem = 0; iElem < nbElem; iElem++ ) {
        MGOutput->GmfGetLin(InpMsh, token, &id[iElem*tabRef[token]], &id[iElem*tabRef[token]+1], &id[iElem*tabRef[token]+2], &id[iElem*tabRef[token]+3], &domainID[iElem]);
#ifdef _DEBUG_
        subdomainId2tetraId[dummy].insert(iElem+1);
//         MESSAGE("subdomainId2tetraId["<<dummy<<"].insert("<<iElem+1<<")");
#endif
      }
    }
    else if (token == GmfPyramids && nbElem > 0) {
      (nbElem <= 1) ? tmpStr = " Pyramid" : tmpStr = " Pyramids";
      for ( int iElem = 0; iElem < nbElem; iElem++ )
        MGOutput->GmfGetLin(InpMsh, token, &id[iElem*tabRef[token]], &id[iElem*tabRef[token]+1], &id[iElem*tabRef[token]+2], &id[iElem*tabRef[token]+3],
                  &id[iElem*tabRef[token]+4], &domainID[iElem]);
    }
    else if (token == GmfPrisms && nbElem > 0) {
      (nbElem <= 1) ? tmpStr = " Prism" : tmpStr = " Prisms";
      for ( int iElem = 0; iElem < nbElem; iElem++ )
        MGOutput->GmfGetLin(InpMsh, token, &id[iElem*tabRef[token]], &id[iElem*tabRef[token]+1], &id[iElem*tabRef[token]+2], &id[iElem*tabRef[token]+3],
                  &id[iElem*tabRef[token]+4], &id[iElem*tabRef[token]+5], &domainID[iElem]);
    }
    else if (token == GmfHexahedra && nbElem > 0) {
      (nbElem <= 1) ? tmpStr = " Hexahedron" : tmpStr = " Hexahedra";
      for ( int iElem = 0; iElem < nbElem; iElem++ )
        MGOutput->GmfGetLin(InpMsh, token, &id[iElem*tabRef[token]], &id[iElem*tabRef[token]+1], &id[iElem*tabRef[token]+2], &id[iElem*tabRef[token]+3],
                  &id[iElem*tabRef[token]+4], &id[iElem*tabRef[token]+5], &id[iElem*tabRef[token]+6], &id[iElem*tabRef[token]+7], &domainID[iElem]);
    }
    std::cout << tmpStr << std::endl;
    //std::cout << std::endl;

    switch (token) {
    case GmfCorners:
    case GmfRidges:
    case GmfEdges:
    case GmfTriangles:
    case GmfQuadrilaterals:
    case GmfTetrahedra:
    case GmfPyramids:
    case GmfPrisms:
    case GmfHexahedra:
    {
      std::vector< const SMDS_MeshNode* > node( nbRef );
      std::vector< int >          nodeID( nbRef );
      std::vector< SMDS_MeshNode* > enfNode( nbRef );
      const SMDS_MeshElement* aCreatedElem;

      for ( int iElem = 0; iElem < nbElem; iElem++ )
      {
        if(theAlgo->computeCanceled()) {
          MGOutput->GmfCloseMesh(InpMsh);
          return false;
        }
        // Check if elem is already in input mesh. If yes => skip
        bool fullyCreatedElement = false; // if at least one of the nodes was created
        for ( int iRef = 0; iRef < nbRef; iRef++ )
        {
          aGMFNodeID = id[iElem*tabRef[token]+iRef]; // read nbRef aGMFNodeID
          if (aGMFNodeID <= nbInitialNodes) // input nodes
          {
            aGMFNodeID--;
            node[ iRef ] = theNodeByHybridId[aGMFNodeID];
          }
          else
          {
            fullyCreatedElement = true;
            aGMFNodeID -= nbInitialNodes;
            nodeID[ iRef ] = aGMFNodeID ;
            node  [ iRef ] = GMFNode[ aGMFNodeID ];
          }
        }
        aCreatedElem = 0;
        switch (token)
        {
        case GmfEdges:
          if (fullyCreatedElement) {
            aCreatedElem = theHelper->AddEdge( node[0], node[1], noID, force3d );
            if (anEdgeGroupByHybridId.size() && !anEdgeGroupByHybridId[iElem].empty())
              addElemInMeshGroup(theHelper->GetMesh(), aCreatedElem, anEdgeGroupByHybridId[iElem], groupsToRemove);
          }
          break;
        case GmfTriangles:
          if (fullyCreatedElement) {
            aCreatedElem = theHelper->AddFace( node[0], node[1], node[2], noID, force3d );
            if (aFaceGroupByHybridId.size() && !aFaceGroupByHybridId[iElem].empty())
              addElemInMeshGroup(theHelper->GetMesh(), aCreatedElem, aFaceGroupByHybridId[iElem], groupsToRemove);
          }
          break;
        case GmfQuadrilaterals:
          if (fullyCreatedElement) {
            aCreatedElem = theHelper->AddFace( node[0], node[1], node[2], node[3], noID, force3d );
          }
          break;
        case GmfTetrahedra:
          if ( hasGeom )
          {
            if ( solidID != HOLE_ID )
            {
              aCreatedElem = theHelper->AddVolume( node[1], node[0], node[2], node[3],
                                                   noID, force3d );
              theMeshDS->SetMeshElementOnShape( aCreatedElem, solidID );
              for ( int iN = 0; iN < 4; ++iN )
                if ( node[iN]->getshapeId() < 1 )
                  theMeshDS->SetNodeInVolume( node[iN], solidID );
            }
          }
          else
          {
            if ( elemSearcher ) {
              // Issue 0020682. Avoid creating nodes and tetras at place where
              // volumic elements already exist
              if ( !node[1] || !node[0] || !node[2] || !node[3] )
                continue;
              if ( elemSearcher->FindElementsByPoint((SMESH_TNodeXYZ(node[0]) +
                                                      SMESH_TNodeXYZ(node[1]) +
                                                      SMESH_TNodeXYZ(node[2]) +
                                                      SMESH_TNodeXYZ(node[3]) ) / 4.,
                                                     SMDSAbs_Volume, foundVolumes ))
                break;
            }
            aCreatedElem = theHelper->AddVolume( node[1], node[0], node[2], node[3],
                                                 noID, force3d );
          }
          break;
        case GmfPyramids:
          if ( hasGeom )
          {
            if ( solidID != HOLE_ID )
            {
              aCreatedElem = theHelper->AddVolume( node[3], node[2], node[1],
                                                   node[0], node[4],
                                                   noID, force3d );
              theMeshDS->SetMeshElementOnShape( aCreatedElem, solidID );
              for ( int iN = 0; iN < 5; ++iN )
                if ( node[iN]->getshapeId() < 1 )
                  theMeshDS->SetNodeInVolume( node[iN], solidID );
            }
          }
          else
          {
            if ( elemSearcher ) {
              // Issue 0020682. Avoid creating nodes and tetras at place where
              // volumic elements already exist
              if ( !node[1] || !node[0] || !node[2] || !node[3] || !node[4] || !node[5] )
                continue;
              if ( elemSearcher->FindElementsByPoint((SMESH_TNodeXYZ(node[0]) +
                                                      SMESH_TNodeXYZ(node[1]) +
                                                      SMESH_TNodeXYZ(node[2]) +
                                                      SMESH_TNodeXYZ(node[3]) +
                                                      SMESH_TNodeXYZ(node[4])) / 5.,
                                                     SMDSAbs_Volume, foundVolumes ))
                break;
            }
            aCreatedElem = theHelper->AddVolume( node[3], node[2], node[1],
                                                   node[0], node[4],
                                                   noID, force3d );
          }
          break;
        case GmfPrisms:
          if ( hasGeom )
          {
            if ( solidID != HOLE_ID )
            {
              aCreatedElem = theHelper->AddVolume( node[0], node[2], node[1],
                                                   node[3], node[5], node[4],
                                                   noID, force3d );
              theMeshDS->SetMeshElementOnShape( aCreatedElem, solidID );
              for ( int iN = 0; iN < 6; ++iN )
                if ( node[iN]->getshapeId() < 1 )
                  theMeshDS->SetNodeInVolume( node[iN], solidID );
            }
          }
          else
          {
            if ( elemSearcher ) {
              // Issue 0020682. Avoid creating nodes and tetras at place where
              // volumic elements already exist
              if ( !node[1] || !node[0] || !node[2] || !node[3] || !node[4] || !node[5] )
                continue;
              if ( elemSearcher->FindElementsByPoint((SMESH_TNodeXYZ(node[0]) +
                                                      SMESH_TNodeXYZ(node[1]) +
                                                      SMESH_TNodeXYZ(node[2]) +
                                                      SMESH_TNodeXYZ(node[3]) +
                                                      SMESH_TNodeXYZ(node[4]) +
                                                      SMESH_TNodeXYZ(node[5])) / 6.,
                                                     SMDSAbs_Volume, foundVolumes ))
                break;
            }
            aCreatedElem = theHelper->AddVolume( node[0], node[2], node[1],
                                                 node[3], node[5], node[4],
                                                 noID, force3d );
          }
          break;
        case GmfHexahedra:
          if ( hasGeom )
          {
            if ( solidID != HOLE_ID )
            {
              aCreatedElem = theHelper->AddVolume( node[0], node[3], node[2], node[1],
                                                   node[4], node[7], node[6], node[5],
                                                   noID, force3d );
              theMeshDS->SetMeshElementOnShape( aCreatedElem, solidID );
              for ( int iN = 0; iN < 8; ++iN )
                if ( node[iN]->getshapeId() < 1 )
                  theMeshDS->SetNodeInVolume( node[iN], solidID );
            }
          }
          else
          {
            if ( elemSearcher ) {
              // Issue 0020682. Avoid creating nodes and tetras at place where
              // volumic elements already exist
              if ( !node[1] || !node[0] || !node[2] || !node[3] || !node[4] || !node[5] || !node[6] || !node[7])
                continue;
              if ( elemSearcher->FindElementsByPoint((SMESH_TNodeXYZ(node[0]) +
                                                      SMESH_TNodeXYZ(node[1]) +
                                                      SMESH_TNodeXYZ(node[2]) +
                                                      SMESH_TNodeXYZ(node[3]) +
                                                      SMESH_TNodeXYZ(node[4]) +
                                                      SMESH_TNodeXYZ(node[5]) +
                                                      SMESH_TNodeXYZ(node[6]) +
                                                      SMESH_TNodeXYZ(node[7])) / 8.,
                                                     SMDSAbs_Volume, foundVolumes ))
                break;
            }
            aCreatedElem = theHelper->AddVolume( node[0], node[3], node[2], node[1],
                                                 node[4], node[7], node[6], node[5],
                                                 noID, force3d );
          }
          break;
        default: continue;
        } // switch (token)

        if ( aCreatedElem && toMakeGroupsOfDomains )
        {
          if ( domainID[iElem] >= (int) elemsOfDomain.size() )
            elemsOfDomain.resize( domainID[iElem] + 1 );
          elemsOfDomain[ domainID[iElem] ].push_back( aCreatedElem );
        }
      } // loop on elements of one type
      break;
    } // case ...

    default:;
    } // switch (token)
  } // loop on tabRef

  // remove nodes in holes
  if ( hasGeom )
  {
    for ( int i = 1; i <= nbVertices; ++i )
      if ( GMFNode[i]->NbInverseElements() == 0 )
        theMeshDS->RemoveFreeNode( GMFNode[i], /*sm=*/0, /*fromGroups=*/false );
  }

  MGOutput->GmfCloseMesh(InpMsh);

  // 0022172: [CEA 790] create the groups corresponding to domains
  if ( toMakeGroupsOfDomains )
    makeDomainGroups( elemsOfDomain, theHelper );

#ifdef _DEBUG_
  MESSAGE("Nb subdomains " << subdomainId2tetraId.size());
  std::map<int, std::set<int> >::const_iterator subdomainIt = subdomainId2tetraId.begin();
  TCollection_AsciiString aSubdomainFileName = theFile;
  aSubdomainFileName = aSubdomainFileName + ".subdomain";
  ofstream aSubdomainFile  ( aSubdomainFileName.ToCString()  , ios::out);

  aSubdomainFile << "Nb subdomains " << subdomainId2tetraId.size() << std::endl;
  for(;subdomainIt != subdomainId2tetraId.end() ; ++subdomainIt) {
    int subdomainId = subdomainIt->first;
    std::set<int> tetraIds = subdomainIt->second;
    MESSAGE("Subdomain #"<<subdomainId<<": "<<tetraIds.size()<<" tetrahedrons");
    std::set<int>::const_iterator tetraIdsIt = tetraIds.begin();
    aSubdomainFile << subdomainId << std::endl;
    for(;tetraIdsIt != tetraIds.end() ; ++tetraIdsIt) {
      aSubdomainFile << (*tetraIdsIt) << " ";
    }
    aSubdomainFile << std::endl;
  }
  aSubdomainFile.close();
#endif  
  
  return true;
}


static bool writeGMFFile(MG_HYBRID_API*                                  MGInput,
                         const char*                                     theMeshFileName,
                         const char*                                     theRequiredFileName,
                         const char*                                     theSolFileName,
                         const SMESH_ProxyMesh&                          theProxyMesh,
                         SMESH_MesherHelper&                             theHelper,
                         std::vector <const SMDS_MeshNode*> &            theNodeByHybridId,
                         std::vector <const SMDS_MeshElement*> &         theFaceByHybridId,
                         std::map<const SMDS_MeshNode*,int> &            aNodeToHybridIdMap,
                         std::vector<std::string> &                      aNodeGroupByHybridId,
                         std::vector<std::string> &                      anEdgeGroupByHybridId,
                         std::vector<std::string> &                      aFaceGroupByHybridId,
                         HYBRIDPlugin_Hypothesis::TIDSortedNodeGroupMap & theEnforcedNodes,
                         HYBRIDPlugin_Hypothesis::TIDSortedElemGroupMap & theEnforcedEdges,
                         HYBRIDPlugin_Hypothesis::TIDSortedElemGroupMap & theEnforcedTriangles,
                         std::map<std::vector<double>, std::string> &    enfVerticesWithGroup,
                         HYBRIDPlugin_Hypothesis::THYBRIDEnforcedVertexCoordsValues & theEnforcedVertices)
{
  //MESSAGE("writeGMFFile w/o geometry");
  std::cout << "!!!!!!!!!!!writeGMFFile w/o geometry for HYBRIDPLUGIN..." << std::endl;
  std::string tmpStr;
  int idx, idxRequired = 0, idxSol = 0;
  //tabg each dummyint
  //const int dummyint = 0;
  const int dummyint1 = 1;
  const int dummyint2 = 2;
  const int dummyint3 = 3;
  const int dummyint4 = 4;
  const int dummyint5 = 5;
  const int dummyint6 = 6; //are interesting for layers
  HYBRIDPlugin_Hypothesis::THYBRIDEnforcedVertexCoordsValues::const_iterator vertexIt;
  std::vector<double> enfVertexSizes;
  const SMDS_MeshElement* elem;
  TIDSortedElemSet anElemSetTri, anElemSetQuad, theKeptEnforcedEdges, theKeptEnforcedTriangles;
  SMDS_ElemIteratorPtr nodeIt;
  std::vector <const SMDS_MeshNode*> theEnforcedNodeByHybridId;
  std::map<const SMDS_MeshNode*,int> anEnforcedNodeToHybridIdMap, anExistingEnforcedNodeToHybridIdMap;
  std::vector< const SMDS_MeshElement* > foundElems;
  std::map<const SMDS_MeshNode*,TopAbs_State> aNodeToTopAbs_StateMap;
  int nbFoundElems;
  HYBRIDPlugin_Hypothesis::TIDSortedElemGroupMap::iterator elemIt;
  TIDSortedElemSet::iterator elemSetIt;
  bool isOK;
  SMESH_Mesh* theMesh = theHelper.GetMesh();
  const bool hasGeom = theMesh->HasShapeToMesh();
  SMESHUtils::Deleter< SMESH_ElementSearcher > pntCls
    ( SMESH_MeshAlgos::GetElementSearcher(*theMesh->GetMeshDS()));
  
  int nbEnforcedVertices = theEnforcedVertices.size();
  
  // count faces
  int nbFaces = theProxyMesh.NbFaces();
  int nbNodes;
  theFaceByHybridId.reserve( nbFaces );
  
  // groups management
  int usedEnforcedNodes = 0;
  std::string gn = "";

  if ( nbFaces == 0 )
    return false;
  
  idx = MGInput->GmfOpenMesh(theMeshFileName, GmfWrite, GMFVERSION, GMFDIMENSION);
  if (!idx)
    return false;
  
  // ========================== FACES ==========================
  // TRIANGLES ==========================
  SMDS_ElemIteratorPtr eIt =
    hasGeom ? theProxyMesh.GetFaces( theHelper.GetSubShape()) : theProxyMesh.GetFaces();
  while ( eIt->more() )
  {
    elem = eIt->next();
    nodeIt = elem->nodesIterator();
    nbNodes = elem->NbCornerNodes();
    if (nbNodes == 3)
      anElemSetTri.insert(elem);
    else if (nbNodes == 4)
      anElemSetQuad.insert(elem);
    else
    {
      std::cout << "Unexpected number of nodes: " << nbNodes << std::endl;
      throw ("Unexpected number of nodes" );
    }
    while ( nodeIt->more() && nbNodes--)
    {
      // find HYBRID ID
      const SMDS_MeshNode* node = castToNode( nodeIt->next() );
      int newId = aNodeToHybridIdMap.size() + 1; // hybrid ids count from 1
      aNodeToHybridIdMap.insert( std::make_pair( node, newId ));
    }
  }
  
  //EDGES ==========================
  
  // Iterate over the enforced edges
  for(elemIt = theEnforcedEdges.begin() ; elemIt != theEnforcedEdges.end() ; ++elemIt) {
    elem = elemIt->first;
    isOK = true;
    nodeIt = elem->nodesIterator();
    nbNodes = 2;
    while ( nodeIt->more() && nbNodes-- ) {
      // find HYBRID ID
      const SMDS_MeshNode* node = castToNode( nodeIt->next() );
      // Test if point is inside shape to mesh
      gp_Pnt myPoint(node->X(),node->Y(),node->Z());
      TopAbs_State result = pntCls->GetPointState( myPoint );
      if ( result == TopAbs_OUT ) {
        isOK = false;
        break;
      }
      aNodeToTopAbs_StateMap.insert( std::make_pair( node, result ));
    }
    if (isOK) {
      nodeIt = elem->nodesIterator();
      nbNodes = 2;
      int newId = -1;
      while ( nodeIt->more() && nbNodes-- ) {
        // find HYBRID ID
        const SMDS_MeshNode* node = castToNode( nodeIt->next() );
        gp_Pnt myPoint(node->X(),node->Y(),node->Z());
        nbFoundElems = pntCls->FindElementsByPoint(myPoint, SMDSAbs_Node, foundElems);
#ifdef _DEBUG_
        std::cout << "Node at "<<node->X()<<", "<<node->Y()<<", "<<node->Z()<<std::endl;
        std::cout << "Nb nodes found : "<<nbFoundElems<<std::endl;
#endif
        if (nbFoundElems ==0) {
          if ((*aNodeToTopAbs_StateMap.find(node)).second == TopAbs_IN) {
            newId = aNodeToHybridIdMap.size() + anEnforcedNodeToHybridIdMap.size() + 1; // hybrid ids count from 1
            anEnforcedNodeToHybridIdMap.insert( std::make_pair( node, newId ));
          }
        }
        else if (nbFoundElems ==1) {
          const SMDS_MeshNode* existingNode = (SMDS_MeshNode*) foundElems.at(0);
          newId = (*aNodeToHybridIdMap.find(existingNode)).second;
          anExistingEnforcedNodeToHybridIdMap.insert( std::make_pair( node, newId ));
        }
        else
          isOK = false;
#ifdef _DEBUG_
        std::cout << "HYBRID node ID: "<<newId<<std::endl;
#endif
      }
      if (isOK)
        theKeptEnforcedEdges.insert(elem);
    }
  }
  
  //ENFORCED TRIANGLES ==========================
  
  // Iterate over the enforced triangles
  for(elemIt = theEnforcedTriangles.begin() ; elemIt != theEnforcedTriangles.end() ; ++elemIt) {
    elem = elemIt->first;
    isOK = true;
    nodeIt = elem->nodesIterator();
    nbNodes = 3;
    while ( nodeIt->more() && nbNodes--) {
      // find HYBRID ID
      const SMDS_MeshNode* node = castToNode( nodeIt->next() );
      // Test if point is inside shape to mesh
      gp_Pnt myPoint(node->X(),node->Y(),node->Z());
      TopAbs_State result = pntCls->GetPointState( myPoint );
      if ( result == TopAbs_OUT ) {
        isOK = false;
        break;
      }
      aNodeToTopAbs_StateMap.insert( std::make_pair( node, result ));
    }
    if (isOK) {
      nodeIt = elem->nodesIterator();
      nbNodes = 3;
      int newId = -1;
      while ( nodeIt->more() && nbNodes--) {
        // find HYBRID ID
        const SMDS_MeshNode* node = castToNode( nodeIt->next() );
        gp_Pnt myPoint(node->X(),node->Y(),node->Z());
        nbFoundElems = pntCls->FindElementsByPoint(myPoint, SMDSAbs_Node, foundElems);
#ifdef _DEBUG_
        std::cout << "Nb nodes found : "<<nbFoundElems<<std::endl;
#endif
        if (nbFoundElems ==0) {
          if ((*aNodeToTopAbs_StateMap.find(node)).second == TopAbs_IN) {
            newId = aNodeToHybridIdMap.size() + anEnforcedNodeToHybridIdMap.size() + 1; // hybrid ids count from 1
            anEnforcedNodeToHybridIdMap.insert( std::make_pair( node, newId ));
          }
        }
        else if (nbFoundElems ==1) {
          const SMDS_MeshNode* existingNode = (SMDS_MeshNode*) foundElems.at(0);
          newId = (*aNodeToHybridIdMap.find(existingNode)).second;
          anExistingEnforcedNodeToHybridIdMap.insert( std::make_pair( node, newId ));
        }
        else
          isOK = false;
#ifdef _DEBUG_
        std::cout << "HYBRID node ID: "<<newId<<std::endl;
#endif
      }
      if (isOK)
        theKeptEnforcedTriangles.insert(elem);
    }
  }
  
  // put nodes to theNodeByHybridId vector
#ifdef _DEBUG_
  std::cout << "aNodeToHybridIdMap.size(): "<<aNodeToHybridIdMap.size()<<std::endl;
#endif
  theNodeByHybridId.resize( aNodeToHybridIdMap.size() );
  std::map<const SMDS_MeshNode*,int>::const_iterator n2id = aNodeToHybridIdMap.begin();
  for ( ; n2id != aNodeToHybridIdMap.end(); ++ n2id)
  {
//     std::cout << "n2id->first: "<<n2id->first<<std::endl;
    theNodeByHybridId[ n2id->second - 1 ] = n2id->first; // hybrid ids count from 1
  }

  // put nodes to anEnforcedNodeToHybridIdMap vector
#ifdef _DEBUG_
  std::cout << "anEnforcedNodeToHybridIdMap.size(): "<<anEnforcedNodeToHybridIdMap.size()<<std::endl;
#endif
  theEnforcedNodeByHybridId.resize( anEnforcedNodeToHybridIdMap.size());
  n2id = anEnforcedNodeToHybridIdMap.begin();
  for ( ; n2id != anEnforcedNodeToHybridIdMap.end(); ++ n2id)
  {
    if (n2id->second > (int)aNodeToHybridIdMap.size()) {
      theEnforcedNodeByHybridId[ n2id->second - aNodeToHybridIdMap.size() - 1 ] = n2id->first; // hybrid ids count from 1
    }
  }
  
  
  //========================== NODES ==========================
  std::vector<const SMDS_MeshNode*> theOrderedNodes, theRequiredNodes;
  std::set< std::vector<double> > nodesCoords;
  std::vector<const SMDS_MeshNode*>::const_iterator hybridNodeIt = theNodeByHybridId.begin();
  std::vector<const SMDS_MeshNode*>::const_iterator after  = theNodeByHybridId.end();
  
  (theNodeByHybridId.size() <= 1) ? tmpStr = " node" : " nodes";
  std::cout << theNodeByHybridId.size() << tmpStr << " from mesh ..." << std::endl;
  for ( ; hybridNodeIt != after; ++hybridNodeIt )
  {
    const SMDS_MeshNode* node = *hybridNodeIt;
    std::vector<double> coords;
    coords.push_back(node->X());
    coords.push_back(node->Y());
    coords.push_back(node->Z());
    nodesCoords.insert(coords);
    theOrderedNodes.push_back(node);
  }
  
  // Iterate over the enforced nodes given by enforced elements
  hybridNodeIt = theEnforcedNodeByHybridId.begin();
  after  = theEnforcedNodeByHybridId.end();
  (theEnforcedNodeByHybridId.size() <= 1) ? tmpStr = " node" : " nodes";
  std::cout << theEnforcedNodeByHybridId.size() << tmpStr << " from enforced elements ..." << std::endl;
  for ( ; hybridNodeIt != after; ++hybridNodeIt )
  {
    const SMDS_MeshNode* node = *hybridNodeIt;
    std::vector<double> coords;
    coords.push_back(node->X());
    coords.push_back(node->Y());
    coords.push_back(node->Z());
#ifdef _DEBUG_
    std::cout << "Node at " << node->X()<<", " <<node->Y()<<", " <<node->Z();
#endif
    
    if (nodesCoords.find(coords) != nodesCoords.end()) {
      // node already exists in original mesh
#ifdef _DEBUG_
      std::cout << " found" << std::endl;
#endif
      continue;
    }
    
    if (theEnforcedVertices.find(coords) != theEnforcedVertices.end()) {
      // node already exists in enforced vertices
#ifdef _DEBUG_
      std::cout << " found" << std::endl;
#endif
      continue;
    }
    
//     gp_Pnt myPoint(node->X(),node->Y(),node->Z());
//     nbFoundElems = pntCls->FindElementsByPoint(myPoint, SMDSAbs_Node, foundElems);
//     if (nbFoundElems ==0) {
//       std::cout << " not found" << std::endl;
//       if ((*aNodeToTopAbs_StateMap.find(node)).second == TopAbs_IN) {
//         nodesCoords.insert(coords);
//         theOrderedNodes.push_back(node);
//       }
//     }
//     else {
//       std::cout << " found in initial mesh" << std::endl;
//       const SMDS_MeshNode* existingNode = (SMDS_MeshNode*) foundElems.at(0);
//       nodesCoords.insert(coords);
//       theOrderedNodes.push_back(existingNode);
//     }
    
#ifdef _DEBUG_
    std::cout << " not found" << std::endl;
#endif
    
    nodesCoords.insert(coords);
    theOrderedNodes.push_back(node);
//     theRequiredNodes.push_back(node);
  }
  
  
  // Iterate over the enforced nodes
  HYBRIDPlugin_Hypothesis::TIDSortedNodeGroupMap::const_iterator enfNodeIt;
  (theEnforcedNodes.size() <= 1) ? tmpStr = " node" : " nodes";
  std::cout << theEnforcedNodes.size() << tmpStr << " from enforced nodes ..." << std::endl;
  for(enfNodeIt = theEnforcedNodes.begin() ; enfNodeIt != theEnforcedNodes.end() ; ++enfNodeIt)
  {
    const SMDS_MeshNode* node = enfNodeIt->first;
    std::vector<double> coords;
    coords.push_back(node->X());
    coords.push_back(node->Y());
    coords.push_back(node->Z());
#ifdef _DEBUG_
    std::cout << "Node at " << node->X()<<", " <<node->Y()<<", " <<node->Z();
#endif
    
    // Test if point is inside shape to mesh
    gp_Pnt myPoint(node->X(),node->Y(),node->Z());
    TopAbs_State result = pntCls->GetPointState( myPoint );
    if ( result == TopAbs_OUT ) {
#ifdef _DEBUG_
      std::cout << " out of volume" << std::endl;
#endif
      continue;
    }
    
    if (nodesCoords.find(coords) != nodesCoords.end()) {
#ifdef _DEBUG_
      std::cout << " found in nodesCoords" << std::endl;
#endif
//       theRequiredNodes.push_back(node);
      continue;
    }

    if (theEnforcedVertices.find(coords) != theEnforcedVertices.end()) {
#ifdef _DEBUG_
      std::cout << " found in theEnforcedVertices" << std::endl;
#endif
      continue;
    }
    
//     nbFoundElems = pntCls->FindElementsByPoint(myPoint, SMDSAbs_Node, foundElems);
//     if (nbFoundElems ==0) {
//       std::cout << " not found" << std::endl;
//       if (result == TopAbs_IN) {
//         nodesCoords.insert(coords);
//         theRequiredNodes.push_back(node);
//       }
//     }
//     else {
//       std::cout << " found in initial mesh" << std::endl;
//       const SMDS_MeshNode* existingNode = (SMDS_MeshNode*) foundElems.at(0);
// //       nodesCoords.insert(coords);
//       theRequiredNodes.push_back(existingNode);
//     }
//     
//     
//     
//     if (pntCls->FindElementsByPoint(myPoint, SMDSAbs_Node, foundElems) == 0)
//       continue;

//     if ( result != TopAbs_IN )
//       continue;
    
#ifdef _DEBUG_
    std::cout << " not found" << std::endl;
#endif
    nodesCoords.insert(coords);
//     theOrderedNodes.push_back(node);
    theRequiredNodes.push_back(node);
  }
  int requiredNodes = theRequiredNodes.size();
  
  int solSize = 0;
  std::vector<std::vector<double> > ReqVerTab;
  if (nbEnforcedVertices) {
//    ReqVerTab.clear();
    (nbEnforcedVertices <= 1) ? tmpStr = " node" : " nodes";
    std::cout << nbEnforcedVertices << tmpStr << " from enforced vertices ..." << std::endl;
    // Iterate over the enforced vertices
    for(vertexIt = theEnforcedVertices.begin() ; vertexIt != theEnforcedVertices.end() ; ++vertexIt) {
      double x = vertexIt->first[0];
      double y = vertexIt->first[1];
      double z = vertexIt->first[2];
      // Test if point is inside shape to mesh
      gp_Pnt myPoint(x,y,z);
      TopAbs_State result = pntCls->GetPointState( myPoint );
      if ( result == TopAbs_OUT )
        continue;
      //if (pntCls->FindElementsByPoint(myPoint, SMDSAbs_Node, foundElems) == 0)
      //continue;

//       if ( result != TopAbs_IN )
//         continue;
      std::vector<double> coords;
      coords.push_back(x);
      coords.push_back(y);
      coords.push_back(z);
      ReqVerTab.push_back(coords);
      enfVertexSizes.push_back(vertexIt->second);
      solSize++;
    }
  }
  
  
  // GmfVertices
  std::cout << "Begin writing required nodes in GmfVertices" << std::endl;
  std::cout << "Nb vertices: " << theOrderedNodes.size() << std::endl;
  MGInput->GmfSetKwd(idx, GmfVertices, theOrderedNodes.size()); //theOrderedNodes.size()+solSize)
  for (hybridNodeIt = theOrderedNodes.begin();hybridNodeIt != theOrderedNodes.end();++hybridNodeIt) {
    MGInput->GmfSetLin(idx, GmfVertices, (*hybridNodeIt)->X(), (*hybridNodeIt)->Y(), (*hybridNodeIt)->Z(), dummyint1);
  }

  std::cout << "End writing required nodes in GmfVertices" << std::endl;

  if (requiredNodes + solSize) {
    std::cout << "Begin writing in req and sol file" << std::endl;
    aNodeGroupByHybridId.resize( requiredNodes + solSize );
    idxRequired = MGInput->GmfOpenMesh(theRequiredFileName, GmfWrite, GMFVERSION, GMFDIMENSION);
    if (!idxRequired) {
      MGInput->GmfCloseMesh(idx);
      return false;
    }
    idxSol = MGInput->GmfOpenMesh(theSolFileName, GmfWrite, GMFVERSION, GMFDIMENSION);
    if (!idxSol) {
      MGInput->GmfCloseMesh(idx);
      if (idxRequired)
        MGInput->GmfCloseMesh(idxRequired);
      return false;
    }
    int TypTab[] = {GmfSca};
    double ValTab[] = {0.0};
    MGInput->GmfSetKwd(idxRequired, GmfVertices, requiredNodes + solSize);
    MGInput->GmfSetKwd(idxSol, GmfSolAtVertices, requiredNodes + solSize, 1, TypTab);
//     int usedEnforcedNodes = 0;
//     std::string gn = "";
    for (hybridNodeIt = theRequiredNodes.begin();hybridNodeIt != theRequiredNodes.end();++hybridNodeIt) {
      MGInput->GmfSetLin(idxRequired, GmfVertices, (*hybridNodeIt)->X(), (*hybridNodeIt)->Y(), (*hybridNodeIt)->Z(), dummyint2);
      MGInput->GmfSetLin(idxSol, GmfSolAtVertices, ValTab);
      if (theEnforcedNodes.find((*hybridNodeIt)) != theEnforcedNodes.end())
        gn = theEnforcedNodes.find((*hybridNodeIt))->second;
      aNodeGroupByHybridId[usedEnforcedNodes] = gn;
      usedEnforcedNodes++;
    }

    for (int i=0;i<solSize;i++) {
      std::cout << ReqVerTab[i][0] <<" "<< ReqVerTab[i][1] << " "<< ReqVerTab[i][2] << std::endl;
#ifdef _DEBUG_
      std::cout << "enfVertexSizes.at("<<i<<"): " << enfVertexSizes.at(i) << std::endl;
#endif
      double solTab[] = {enfVertexSizes.at(i)};
      MGInput->GmfSetLin(idxRequired, GmfVertices, ReqVerTab[i][0], ReqVerTab[i][1], ReqVerTab[i][2], dummyint3);
      MGInput->GmfSetLin(idxSol, GmfSolAtVertices, solTab);
      aNodeGroupByHybridId[usedEnforcedNodes] = enfVerticesWithGroup.find(ReqVerTab[i])->second;
#ifdef _DEBUG_
      std::cout << "aNodeGroupByHybridId["<<usedEnforcedNodes<<"] = \""<<aNodeGroupByHybridId[usedEnforcedNodes]<<"\""<<std::endl;
#endif
      usedEnforcedNodes++;
    }
    std::cout << "End writing in req and sol file" << std::endl;
  }

  int nedge[2], ntri[3], nquad[4];
    
  // GmfEdges
  int usedEnforcedEdges = 0;
  if (theKeptEnforcedEdges.size()) {
    anEdgeGroupByHybridId.resize( theKeptEnforcedEdges.size() );
//    idxRequired = MGInput->GmfOpenMesh(theRequiredFileName, GmfWrite, GMFVERSION, GMFDIMENSION);
//    if (!idxRequired)
//      return false;
    MGInput->GmfSetKwd(idx, GmfEdges, theKeptEnforcedEdges.size());
//    MGInput->GmfSetKwd(idxRequired, GmfEdges, theKeptEnforcedEdges.size());
    for(elemSetIt = theKeptEnforcedEdges.begin() ; elemSetIt != theKeptEnforcedEdges.end() ; ++elemSetIt) {
      elem = (*elemSetIt);
      nodeIt = elem->nodesIterator();
      int index=0;
      while ( nodeIt->more() ) {
        // find HYBRID ID
        const SMDS_MeshNode* node = castToNode( nodeIt->next() );
        std::map< const SMDS_MeshNode*,int >::iterator it = anEnforcedNodeToHybridIdMap.find(node);
        if (it == anEnforcedNodeToHybridIdMap.end()) {
          it = anExistingEnforcedNodeToHybridIdMap.find(node);
          if (it == anEnforcedNodeToHybridIdMap.end())
            throw "Node not found";
        }
        nedge[index] = it->second;
        index++;
      }
      MGInput->GmfSetLin(idx, GmfEdges, nedge[0], nedge[1], dummyint4);
      anEdgeGroupByHybridId[usedEnforcedEdges] = theEnforcedEdges.find(elem)->second;
//      MGInput->GmfSetLin(idxRequired, GmfEdges, nedge[0], nedge[1], dummyint);
      usedEnforcedEdges++;
    }
//    MGInput->GmfCloseMesh(idxRequired);
  }


  if (usedEnforcedEdges) {
    MGInput->GmfSetKwd(idx, GmfRequiredEdges, usedEnforcedEdges);
    for (int enfID=1;enfID<=usedEnforcedEdges;enfID++) {
      MGInput->GmfSetLin(idx, GmfRequiredEdges, enfID);
    }
  }

  // GmfTriangles
  int usedEnforcedTriangles = 0;
  if (anElemSetTri.size()+theKeptEnforcedTriangles.size()) {
    aFaceGroupByHybridId.resize( anElemSetTri.size()+theKeptEnforcedTriangles.size() );
    MGInput->GmfSetKwd(idx, GmfTriangles, anElemSetTri.size()+theKeptEnforcedTriangles.size());
    int k=0;
    for(elemSetIt = anElemSetTri.begin() ; elemSetIt != anElemSetTri.end() ; ++elemSetIt,++k) {
      elem = (*elemSetIt);
      theFaceByHybridId.push_back( elem );
      nodeIt = elem->nodesIterator();
      int index=0;
      for ( int j = 0; j < 3; ++j ) {
        // find HYBRID ID
        const SMDS_MeshNode* node = castToNode( nodeIt->next() );
        std::map< const SMDS_MeshNode*,int >::iterator it = aNodeToHybridIdMap.find(node);
        if (it == aNodeToHybridIdMap.end())
          throw "Node not found";
        ntri[index] = it->second;
        index++;
      }
      MGInput->GmfSetLin(idx, GmfTriangles, ntri[0], ntri[1], ntri[2], dummyint5);
      aFaceGroupByHybridId[k] = "";
    }
    
    if ( !theHelper.GetMesh()->HasShapeToMesh() ) SMESHUtils::FreeVector( theFaceByHybridId ); 
    std::cout << "Enforced triangles size " << theKeptEnforcedTriangles.size() << std::endl;
    if (theKeptEnforcedTriangles.size()) {
      for(elemSetIt = theKeptEnforcedTriangles.begin() ; elemSetIt != theKeptEnforcedTriangles.end() ; ++elemSetIt,++k) {
        elem = (*elemSetIt);
        nodeIt = elem->nodesIterator();
        int index=0;
        for ( int j = 0; j < 3; ++j ) {
          // find HYBRID ID
          const SMDS_MeshNode* node = castToNode( nodeIt->next() );
          std::map< const SMDS_MeshNode*,int >::iterator it = anEnforcedNodeToHybridIdMap.find(node);
          if (it == anEnforcedNodeToHybridIdMap.end()) {
            it = anExistingEnforcedNodeToHybridIdMap.find(node);
            if (it == anEnforcedNodeToHybridIdMap.end())
              throw "Node not found";
          }
          ntri[index] = it->second;
          index++;
        }
        MGInput->GmfSetLin(idx, GmfTriangles, ntri[0], ntri[1], ntri[2], dummyint6);
        aFaceGroupByHybridId[k] = theEnforcedTriangles.find(elem)->second;
        usedEnforcedTriangles++;
      }
    }
  }

  
  if (usedEnforcedTriangles) {
    MGInput->GmfSetKwd(idx, GmfRequiredTriangles, usedEnforcedTriangles);
    for (int enfID=1;enfID<=usedEnforcedTriangles;enfID++)
      MGInput->GmfSetLin(idx, GmfRequiredTriangles, anElemSetTri.size()+enfID);
  }
  
  if (anElemSetQuad.size()) {
    MGInput->GmfSetKwd(idx, GmfQuadrilaterals, anElemSetQuad.size());
    int k=0;
    for(elemSetIt = anElemSetQuad.begin() ; elemSetIt != anElemSetQuad.end() ; ++elemSetIt,++k) {
      elem = (*elemSetIt);
      theFaceByHybridId.push_back( elem );
      nodeIt = elem->nodesIterator();
      int index=0;
      for ( int j = 0; j < 4; ++j ) {
        // find HYBRID ID
        const SMDS_MeshNode* node = castToNode( nodeIt->next() );
        std::map< const SMDS_MeshNode*,int >::iterator it = aNodeToHybridIdMap.find(node);
        if (it == aNodeToHybridIdMap.end())
          throw "Node not found";
        nquad[index] = it->second;
        index++;
      }
      MGInput->GmfSetLin(idx, GmfQuadrilaterals, nquad[0], nquad[1], nquad[2], nquad[3], dummyint5);
      // _CEA_cbo what is it for???
      //aFaceGroupByHybridId[k] = "";
    }
  }

  MGInput->GmfCloseMesh(idx);
  if (idxRequired)
    MGInput->GmfCloseMesh(idxRequired);
  if (idxSol)
    MGInput->GmfCloseMesh(idxSol);
  
  return true;
  
}

// static bool writeGMFFile(const char*                                    theMeshFileName,
//                         const char*                                     theRequiredFileName,
//                         const char*                                     theSolFileName,
//                         SMESH_MesherHelper&                             theHelper,
//                         const SMESH_ProxyMesh&                          theProxyMesh,
//                         std::map <int,int> &                            theNodeId2NodeIndexMap,
//                         std::map <int,int> &                            theSmdsToHybridIdMap,
//                         std::map <int,const SMDS_MeshNode*> &           theHybridIdToNodeMap,
//                         TIDSortedNodeSet &                              theEnforcedNodes,
//                         TIDSortedElemSet &                              theEnforcedEdges,
//                         TIDSortedElemSet &                              theEnforcedTriangles,
// //                         TIDSortedElemSet &                              theEnforcedQuadrangles,
//                         HYBRIDPlugin_Hypothesis::THYBRIDEnforcedVertexCoordsValues & theEnforcedVertices)
// {
//   MESSAGE("writeGMFFile with geometry");
//   int idx, idxRequired, idxSol;
//   int nbv, nbev, nben, aHybridID = 0;
//   const int dummyint = 0;
//   HYBRIDPlugin_Hypothesis::THYBRIDEnforcedVertexCoordsValues::const_iterator vertexIt;
//   std::vector<double> enfVertexSizes;
//   TIDSortedNodeSet::const_iterator enfNodeIt;
//   const SMDS_MeshNode* node;
//   SMDS_NodeIteratorPtr nodeIt;
// 
//   idx = GmfOpenMesh(theMeshFileName, GmfWrite, GMFVERSION, GMFDIMENSION);
//   if (!idx)
//     return false;
//   
//   SMESHDS_Mesh * theMeshDS = theHelper.GetMeshDS();
//   
//   /* ========================== NODES ========================== */
//   // NB_NODES
//   nbv = theMeshDS->NbNodes();
//   if ( nbv == 0 )
//     return false;
//   nbev = theEnforcedVertices.size();
//   nben = theEnforcedNodes.size();
//   
//   // Issue 020674: EDF 870 SMESH: Mesh generated by Netgen not usable by HYBRID
//   // The problem is in nodes on degenerated edges, we need to skip nodes which are free
//   // and replace not-free nodes on edges by the node on vertex
//   TNodeNodeMap n2nDegen; // map a node on degenerated edge to a node on vertex
//   TNodeNodeMap::iterator n2nDegenIt;
//   if ( theHelper.HasDegeneratedEdges() )
//   {
//     set<int> checkedSM;
//     for (TopExp_Explorer e(theMeshDS->ShapeToMesh(), TopAbs_EDGE ); e.More(); e.Next())
//     {
//       SMESH_subMesh* sm = theHelper.GetMesh()->GetSubMesh( e.Current() );
//       if ( checkedSM.insert( sm->GetId() ).second && theHelper.IsDegenShape(sm->GetId() ))
//       {
//         if ( SMESHDS_SubMesh* smDS = sm->GetSubMeshDS() )
//         {
//           TopoDS_Shape vertex = TopoDS_Iterator( e.Current() ).Value();
//           const SMDS_MeshNode* vNode = SMESH_Algo::VertexNode( TopoDS::Vertex( vertex ), theMeshDS);
//           {
//             SMDS_NodeIteratorPtr nIt = smDS->GetNodes();
//             while ( nIt->more() )
//               n2nDegen.insert( std::make_pair( nIt->next(), vNode ));
//           }
//         }
//       }
//     }
//   }
//   
//   const bool isQuadMesh = 
//     theHelper.GetMesh()->NbEdges( ORDER_QUADRATIC ) ||
//     theHelper.GetMesh()->NbFaces( ORDER_QUADRATIC ) ||
//     theHelper.GetMesh()->NbVolumes( ORDER_QUADRATIC );
// 
//   std::vector<std::vector<double> > VerTab;
//   std::set<std::vector<double> > VerMap;
//   VerTab.clear();
//   std::vector<double> aVerTab;
//   // Loop from 1 to NB_NODES
// 
//   nodeIt = theMeshDS->nodesIterator();
//   
//   while ( nodeIt->more() )
//   {
//     node = nodeIt->next();
//     if ( isQuadMesh && theHelper.IsMedium( node )) // Issue 0021238
//       continue;
//     if ( n2nDegen.count( node ) ) // Issue 0020674
//       continue;
// 
//     std::vector<double> coords;
//     coords.push_back(node->X());
//     coords.push_back(node->Y());
//     coords.push_back(node->Z());
//     if (VerMap.find(coords) != VerMap.end()) {
//       aHybridID = theSmdsToHybridIdMap[node->GetID()];
//       theHybridIdToNodeMap[theSmdsToHybridIdMap[node->GetID()]] = node;
//       continue;
//     }
//     VerTab.push_back(coords);
//     VerMap.insert(coords);
//     aHybridID++;
//     theSmdsToHybridIdMap.insert( std::make_pair( node->GetID(), aHybridID ));
//     theHybridIdToNodeMap.insert( std::make_pair( aHybridID, node ));
//   }
//   
//   
//   /* ENFORCED NODES ========================== */
//   if (nben) {
//     std::cout << "Add " << nben << " enforced nodes to input .mesh file" << std::endl;
//     for(enfNodeIt = theEnforcedNodes.begin() ; enfNodeIt != theEnforcedNodes.end() ; ++enfNodeIt) {
//       double x = (*enfNodeIt)->X();
//       double y = (*enfNodeIt)->Y();
//       double z = (*enfNodeIt)->Z();
//       // Test if point is inside shape to mesh
//       gp_Pnt myPoint(x,y,z);
//       BRepClass3d_SolidClassifier scl(theMeshDS->ShapeToMesh());
//       scl.Perform(myPoint, 1e-7);
//       TopAbs_State result = scl.State();
//       if ( result != TopAbs_IN )
//         continue;
//       std::vector<double> coords;
//       coords.push_back(x);
//       coords.push_back(y);
//       coords.push_back(z);
//       if (theEnforcedVertices.find(coords) != theEnforcedVertices.end())
//         continue;
//       if (VerMap.find(coords) != VerMap.end())
//         continue;
//       VerTab.push_back(coords);
//       VerMap.insert(coords);
//       aHybridID++;
//       theNodeId2NodeIndexMap.insert( std::make_pair( (*enfNodeIt)->GetID(), aHybridID ));
//     }
//   }
//     
//   
//   /* ENFORCED VERTICES ========================== */
//   int solSize = 0;
//   std::vector<std::vector<double> > ReqVerTab;
//   ReqVerTab.clear();
//   if (nbev) {
//     std::cout << "Add " << nbev << " enforced vertices to input .mesh file" << std::endl;
//     for(vertexIt = theEnforcedVertices.begin() ; vertexIt != theEnforcedVertices.end() ; ++vertexIt) {
//       double x = vertexIt->first[0];
//       double y = vertexIt->first[1];
//       double z = vertexIt->first[2];
//       // Test if point is inside shape to mesh
//       gp_Pnt myPoint(x,y,z);
//       BRepClass3d_SolidClassifier scl(theMeshDS->ShapeToMesh());
//       scl.Perform(myPoint, 1e-7);
//       TopAbs_State result = scl.State();
//       if ( result != TopAbs_IN )
//         continue;
//       enfVertexSizes.push_back(vertexIt->second);
//       std::vector<double> coords;
//       coords.push_back(x);
//       coords.push_back(y);
//       coords.push_back(z);
//       if (VerMap.find(coords) != VerMap.end())
//         continue;
//       ReqVerTab.push_back(coords);
//       VerMap.insert(coords);
//       solSize++;
//     }
//   }
// 
//   
//   /* ========================== FACES ========================== */
//   
//   int nbTriangles = 0/*, nbQuadrangles = 0*/, aSmdsID;
//   TopTools_IndexedMapOfShape facesMap, trianglesMap/*, quadranglesMap*/;
//   TIDSortedElemSet::const_iterator elemIt;
//   const SMESHDS_SubMesh* theSubMesh;
//   TopoDS_Shape aShape;
//   SMDS_ElemIteratorPtr itOnSubMesh, itOnSubFace;
//   const SMDS_MeshElement* aFace;
//   map<int,int>::const_iterator itOnMap;
//   std::vector<std::vector<int> > tt, qt,et;
//   tt.clear();
//   qt.clear();
//   et.clear();
//   std::vector<int> att, aqt, aet;
//   
//   TopExp::MapShapes( theMeshDS->ShapeToMesh(), TopAbs_FACE, facesMap );
// 
//   for ( int i = 1; i <= facesMap.Extent(); ++i )
//     if (( theSubMesh  = theProxyMesh.GetSubMesh( facesMap(i))))
//     {
//       SMDS_ElemIteratorPtr it = theSubMesh->GetElements();
//       while (it->more())
//       {
//         const SMDS_MeshElement *elem = it->next();
//         int nbCornerNodes = elem->NbCornerNodes();
//         if (nbCornerNodes == 3)
//         {
//           trianglesMap.Add(facesMap(i));
//           nbTriangles ++;
//         }
// //         else if (nbCornerNodes == 4)
// //         {
// //           quadranglesMap.Add(facesMap(i));
// //           nbQuadrangles ++;
// //         }
//       }
//     }
//     
//   /* TRIANGLES ========================== */
//   if (nbTriangles) {
//     for ( int i = 1; i <= trianglesMap.Extent(); i++ )
//     {
//       aShape = trianglesMap(i);
//       theSubMesh = theProxyMesh.GetSubMesh(aShape);
//       if ( !theSubMesh ) continue;
//       itOnSubMesh = theSubMesh->GetElements();
//       while ( itOnSubMesh->more() )
//       {
//         aFace = itOnSubMesh->next();
//         itOnSubFace = aFace->nodesIterator();
//         att.clear();
//         for ( int j = 0; j < 3; ++j ) {
//           // find HYBRID ID
//           node = castToNode( itOnSubFace->next() );
//           if (( n2nDegenIt = n2nDegen.find( node )) != n2nDegen.end() )
//             node = n2nDegenIt->second;
//           aSmdsID = node->GetID();
//           itOnMap = theSmdsToHybridIdMap.find( aSmdsID );
//           ASSERT( itOnMap != theSmdsToHybridIdMap.end() );
//           att.push_back((*itOnMap).second);
//         }
//         tt.push_back(att);
//       }
//     }
//   }
// 
//   if (theEnforcedTriangles.size()) {
//     std::cout << "Add " << theEnforcedTriangles.size() << " enforced triangles to input .mesh file" << std::endl;
//     // Iterate over the enforced triangles
//     for(elemIt = theEnforcedTriangles.begin() ; elemIt != theEnforcedTriangles.end() ; ++elemIt) {
//       aFace = (*elemIt);
//       itOnSubFace = aFace->nodesIterator();
//       bool isOK = true;
//       att.clear();
//       
//       for ( int j = 0; j < 3; ++j ) {
//         node = castToNode( itOnSubFace->next() );
//         if (( n2nDegenIt = n2nDegen.find( node )) != n2nDegen.end() )
//           node = n2nDegenIt->second;
// //         std::cout << node;
//         double x = node->X();
//         double y = node->Y();
//         double z = node->Z();
//         // Test if point is inside shape to mesh
//         gp_Pnt myPoint(x,y,z);
//         BRepClass3d_SolidClassifier scl(theMeshDS->ShapeToMesh());
//         scl.Perform(myPoint, 1e-7);
//         TopAbs_State result = scl.State();
//         if ( result != TopAbs_IN ) {
//           isOK = false;
//           theEnforcedTriangles.erase(elemIt);
//           continue;
//         }
//         std::vector<double> coords;
//         coords.push_back(x);
//         coords.push_back(y);
//         coords.push_back(z);
//         if (VerMap.find(coords) != VerMap.end()) {
//           att.push_back(theNodeId2NodeIndexMap[node->GetID()]);
//           continue;
//         }
//         VerTab.push_back(coords);
//         VerMap.insert(coords);
//         aHybridID++;
//         theNodeId2NodeIndexMap.insert( std::make_pair( node->GetID(), aHybridID ));
//         att.push_back(aHybridID);
//       }
//       if (isOK)
//         tt.push_back(att);
//     }
//   }
// 
// 
//   /* ========================== EDGES ========================== */
// 
//   if (theEnforcedEdges.size()) {
//     // Iterate over the enforced edges
//     std::cout << "Add " << theEnforcedEdges.size() << " enforced edges to input .mesh file" << std::endl;
//     for(elemIt = theEnforcedEdges.begin() ; elemIt != theEnforcedEdges.end() ; ++elemIt) {
//       aFace = (*elemIt);
//       bool isOK = true;
//       itOnSubFace = aFace->nodesIterator();
//       aet.clear();
//       for ( int j = 0; j < 2; ++j ) {
//         node = castToNode( itOnSubFace->next() );
//         if (( n2nDegenIt = n2nDegen.find( node )) != n2nDegen.end() )
//           node = n2nDegenIt->second;
//         double x = node->X();
//         double y = node->Y();
//         double z = node->Z();
//         // Test if point is inside shape to mesh
//         gp_Pnt myPoint(x,y,z);
//         BRepClass3d_SolidClassifier scl(theMeshDS->ShapeToMesh());
//         scl.Perform(myPoint, 1e-7);
//         TopAbs_State result = scl.State();
//         if ( result != TopAbs_IN ) {
//           isOK = false;
//           theEnforcedEdges.erase(elemIt);
//           continue;
//         }
//         std::vector<double> coords;
//         coords.push_back(x);
//         coords.push_back(y);
//         coords.push_back(z);
//         if (VerMap.find(coords) != VerMap.end()) {
//           aet.push_back(theNodeId2NodeIndexMap[node->GetID()]);
//           continue;
//         }
//         VerTab.push_back(coords);
//         VerMap.insert(coords);
//         
//         aHybridID++;
//         theNodeId2NodeIndexMap.insert( std::make_pair( node->GetID(), aHybridID ));
//         aet.push_back(aHybridID);
//       }
//       if (isOK)
//         et.push_back(aet);
//     }
//   }
// 
// 
//   /* Write vertices number */
//   MESSAGE("Number of vertices: "<<aHybridID);
//   MESSAGE("Size of vector: "<<VerTab.size());
//   GmfSetKwd(idx, GmfVertices, aHybridID/*+solSize*/);
//   for (int i=0;i<aHybridID;i++)
//     GmfSetLin(idx, GmfVertices, VerTab[i][0], VerTab[i][1], VerTab[i][2], dummyint);
// //   for (int i=0;i<solSize;i++) {
// //     std::cout << ReqVerTab[i][0] <<" "<< ReqVerTab[i][1] << " "<< ReqVerTab[i][2] << std::endl;
// //     GmfSetLin(idx, GmfVertices, ReqVerTab[i][0], ReqVerTab[i][1], ReqVerTab[i][2], dummyint);
// //   }
// 
//   if (solSize) {
//     idxRequired = GmfOpenMesh(theRequiredFileName, GmfWrite, GMFVERSION, GMFDIMENSION);
//     if (!idxRequired) {
//       GmfCloseMesh(idx);
//       return false;
//     }
//     idxSol = GmfOpenMesh(theSolFileName, GmfWrite, GMFVERSION, GMFDIMENSION);
//     if (!idxSol){
//       GmfCloseMesh(idx);
//       if (idxRequired)
//         GmfCloseMesh(idxRequired);
//       return false;
//     }
//     
//     int TypTab[] = {GmfSca};
//     GmfSetKwd(idxRequired, GmfVertices, solSize);
//     GmfSetKwd(idxSol, GmfSolAtVertices, solSize, 1, TypTab);
//     
//     for (int i=0;i<solSize;i++) {
//       double solTab[] = {enfVertexSizes.at(i)};
//       GmfSetLin(idxRequired, GmfVertices, ReqVerTab[i][0], ReqVerTab[i][1], ReqVerTab[i][2], dummyint);
//       GmfSetLin(idxSol, GmfSolAtVertices, solTab);
//     }
//     GmfCloseMesh(idxRequired);
//     GmfCloseMesh(idxSol);
//   }
//   
//   /* Write triangles number */
//   if (tt.size()) {
//     GmfSetKwd(idx, GmfTriangles, tt.size());
//     for (int i=0;i<tt.size();i++)
//       GmfSetLin(idx, GmfTriangles, tt[i][0], tt[i][1], tt[i][2], dummyint);
//   }  
//   
//   /* Write edges number */
//   if (et.size()) {
//     GmfSetKwd(idx, GmfEdges, et.size());
//     for (int i=0;i<et.size();i++)
//       GmfSetLin(idx, GmfEdges, et[i][0], et[i][1], dummyint);
//   }
// 
//   /* QUADRANGLES ========================== */
//   // TODO: add pyramids ?
// //   if (nbQuadrangles) {
// //     for ( int i = 1; i <= quadranglesMap.Extent(); i++ )
// //     {
// //       aShape = quadranglesMap(i);
// //       theSubMesh = theProxyMesh.GetSubMesh(aShape);
// //       if ( !theSubMesh ) continue;
// //       itOnSubMesh = theSubMesh->GetElements();
// //       for ( int j = 0; j < 4; ++j )
// //       {
// //         aFace = itOnSubMesh->next();
// //         itOnSubFace = aFace->nodesIterator();
// //         aqt.clear();
// //         while ( itOnSubFace->more() ) {
// //           // find HYBRID ID
// //           aSmdsID = itOnSubFace->next()->GetID();
// //           itOnMap = theSmdsToHybridIdMap.find( aSmdsID );
// //           ASSERT( itOnMap != theSmdsToHybridIdMap.end() );
// //           aqt.push_back((*itOnMap).second);
// //         }
// //         qt.push_back(aqt);
// //       }
// //     }
// //   }
// // 
// //   if (theEnforcedQuadrangles.size()) {
// //     // Iterate over the enforced triangles
// //     for(elemIt = theEnforcedQuadrangles.begin() ; elemIt != theEnforcedQuadrangles.end() ; ++elemIt) {
// //       aFace = (*elemIt);
// //       bool isOK = true;
// //       itOnSubFace = aFace->nodesIterator();
// //       aqt.clear();
// //       for ( int j = 0; j < 4; ++j ) {
// //         int aNodeID = itOnSubFace->next()->GetID();
// //         itOnMap = theNodeId2NodeIndexMap.find(aNodeID);
// //         if (itOnMap != theNodeId2NodeIndexMap.end())
// //           aqt.push_back((*itOnMap).second);
// //         else {
// //           isOK = false;
// //           theEnforcedQuadrangles.erase(elemIt);
// //           break;
// //         }
// //       }
// //       if (isOK)
// //         qt.push_back(aqt);
// //     }
// //   }
// //  
//   
// //   /* Write quadrilaterals number */
// //   if (qt.size()) {
// //     GmfSetKwd(idx, GmfQuadrilaterals, qt.size());
// //     for (int i=0;i<qt.size();i++)
// //       GmfSetLin(idx, GmfQuadrilaterals, qt[i][0], qt[i][1], qt[i][2], qt[i][3], dummyint);
// //   }
// 
//   GmfCloseMesh(idx);
//   return true;
// }


//=======================================================================
//function : writeFaces
//purpose  : 
//=======================================================================

// static bool writeFaces (ofstream &              theFile,
//                         const SMESH_ProxyMesh&  theMesh,
//                         const TopoDS_Shape&     theShape,
//                         const std::map <int,int> &   theSmdsToHybridIdMap,
//                         const std::map <int,int> &   theEnforcedNodeIdToHybridIdMap,
//                         HYBRIDPlugin_Hypothesis::TIDSortedElemGroupMap & theEnforcedEdges,
//                         HYBRIDPlugin_Hypothesis::TIDSortedElemGroupMap & theEnforcedTriangles)
// {
//   // record structure:
//   //
//   // NB_ELEMS DUMMY_INT
//   // Loop from 1 to NB_ELEMS
//   // NB_NODES NODE_NB_1 NODE_NB_2 ... (NB_NODES + 1) times: DUMMY_INT

//   TopoDS_Shape aShape;
//   const SMESHDS_SubMesh* theSubMesh;
//   const SMDS_MeshElement* aFace;
//   const char* space    = "  ";
//   const int   dummyint = 0;
//   std::map<int,int>::const_iterator itOnMap;
//   SMDS_ElemIteratorPtr itOnSubMesh, itOnSubFace;
//   int nbNodes, aSmdsID;

//   TIDSortedElemSet::const_iterator elemIt;
//   int nbEnforcedEdges       = theEnforcedEdges.size();
//   int nbEnforcedTriangles   = theEnforcedTriangles.size();

//   // count triangles bound to geometry
//   int nbTriangles = 0;

//   TopTools_IndexedMapOfShape facesMap, trianglesMap;
//   TopExp::MapShapes( theShape, TopAbs_FACE, facesMap );
  
//   int nbFaces = facesMap.Extent();

//   for ( int i = 1; i <= nbFaces; ++i )
//     if (( theSubMesh  = theMesh.GetSubMesh( facesMap(i))))
//       nbTriangles += theSubMesh->NbElements();
//   std::string tmpStr;
//   (nbFaces == 0 || nbFaces == 1) ? tmpStr = " shape " : tmpStr = " shapes " ;
//   std::cout << "    " << nbFaces << tmpStr << "of 2D dimension";
//   int nbEnforcedElements = nbEnforcedEdges+nbEnforcedTriangles;
//   if (nbEnforcedElements > 0) {
//     (nbEnforcedElements == 1) ? tmpStr = "shape:" : tmpStr = "shapes:";
//     std::cout << " and" << std::endl;
//     std::cout << "    " << nbEnforcedElements 
//                         << " enforced " << tmpStr << std::endl;
//   }
//   else
//     std::cout << std::endl;
//   if (nbEnforcedEdges) {
//     (nbEnforcedEdges == 1) ? tmpStr = "edge" : tmpStr = "edges";
//     std::cout << "      " << nbEnforcedEdges << " enforced " << tmpStr << std::endl;
//   }
//   if (nbEnforcedTriangles) {
//     (nbEnforcedTriangles == 1) ? tmpStr = "triangle" : tmpStr = "triangles";
//     std::cout << "      " << nbEnforcedTriangles << " enforced " << tmpStr << std::endl;
//   }
//   std::cout << std::endl;

// //   theFile << space << nbTriangles << space << dummyint << std::endl;
//   std::ostringstream globalStream, localStream, aStream;

//   for ( int i = 1; i <= facesMap.Extent(); i++ )
//   {
//     aShape = facesMap(i);
//     theSubMesh = theMesh.GetSubMesh(aShape);
//     if ( !theSubMesh ) continue;
//     itOnSubMesh = theSubMesh->GetElements();
//     while ( itOnSubMesh->more() )
//     {
//       aFace = itOnSubMesh->next();
//       nbNodes = aFace->NbCornerNodes();

//       localStream << nbNodes << space;

//       itOnSubFace = aFace->nodesIterator();
//       for ( int j = 0; j < 3; ++j ) {
//         // find HYBRID ID
//         aSmdsID = itOnSubFace->next()->GetID();
//         itOnMap = theSmdsToHybridIdMap.find( aSmdsID );
//         // if ( itOnMap == theSmdsToHybridIdMap.end() ) {
//         //   cout << "not found node: " << aSmdsID << endl;
//         //   return false;
//         // }
//         ASSERT( itOnMap != theSmdsToHybridIdMap.end() );

//         localStream << (*itOnMap).second << space ;
//       }

//       // (NB_NODES + 1) times: DUMMY_INT
//       for ( int j=0; j<=nbNodes; j++)
//         localStream << dummyint << space ;

//       localStream << std::endl;
//     }
//   }
  
//   globalStream << localStream.str();
//   localStream.str("");

//   //
//   //        FACES : END
//   //

// //   //
// //   //        ENFORCED EDGES : BEGIN
// //   //
// //   
// //   // Iterate over the enforced edges
// //   int usedEnforcedEdges = 0;
// //   bool isOK;
// //   for(elemIt = theEnforcedEdges.begin() ; elemIt != theEnforcedEdges.end() ; ++elemIt) {
// //     aFace = (*elemIt);
// //     isOK = true;
// //     itOnSubFace = aFace->nodesIterator();
// //     aStream.str("");
// //     aStream << "2" << space ;
// //     for ( int j = 0; j < 2; ++j ) {
// //       aSmdsID = itOnSubFace->next()->GetID();
// //       itOnMap = theEnforcedNodeIdToHybridIdMap.find(aSmdsID);
// //       if (itOnMap != theEnforcedNodeIdToHybridIdMap.end())
// //         aStream << (*itOnMap).second << space;
// //       else {
// //         isOK = false;
// //         break;
// //       }
// //     }
// //     if (isOK) {
// //       for ( int j=0; j<=2; j++)
// //         aStream << dummyint << space ;
// // //       aStream << dummyint << space << dummyint;
// //       localStream << aStream.str() << std::endl;
// //       usedEnforcedEdges++;
// //     }
// //   }
// //   
// //   if (usedEnforcedEdges) {
// //     globalStream << localStream.str();
// //     localStream.str("");
// //   }
// // 
// //   //
// //   //        ENFORCED EDGES : END
// //   //
// //   //
// // 
// //   //
// //   //        ENFORCED TRIANGLES : BEGIN
// //   //
// //     // Iterate over the enforced triangles
// //   int usedEnforcedTriangles = 0;
// //   for(elemIt = theEnforcedTriangles.begin() ; elemIt != theEnforcedTriangles.end() ; ++elemIt) {
// //     aFace = (*elemIt);
// //     nbNodes = aFace->NbCornerNodes();
// //     isOK = true;
// //     itOnSubFace = aFace->nodesIterator();
// //     aStream.str("");
// //     aStream << nbNodes << space ;
// //     for ( int j = 0; j < 3; ++j ) {
// //       aSmdsID = itOnSubFace->next()->GetID();
// //       itOnMap = theEnforcedNodeIdToHybridIdMap.find(aSmdsID);
// //       if (itOnMap != theEnforcedNodeIdToHybridIdMap.end())
// //         aStream << (*itOnMap).second << space;
// //       else {
// //         isOK = false;
// //         break;
// //       }
// //     }
// //     if (isOK) {
// //       for ( int j=0; j<=3; j++)
// //         aStream << dummyint << space ;
// //       localStream << aStream.str() << std::endl;
// //       usedEnforcedTriangles++;
// //     }
// //   }
// //   
// //   if (usedEnforcedTriangles) {
// //     globalStream << localStream.str();
// //     localStream.str("");
// //   }
// // 
// //   //
// //   //        ENFORCED TRIANGLES : END
// //   //
  
//   theFile
//   << nbTriangles/*+usedEnforcedTriangles+usedEnforcedEdges*/
//   << " 0" << std::endl
//   << globalStream.str();

//   return true;
// }

//=======================================================================
//function : writePoints
//purpose  : 
//=======================================================================

// static bool writePoints (ofstream &                       theFile,
//                          SMESH_MesherHelper&              theHelper,
//                          std::map <int,int> &                  theSmdsToHybridIdMap,
//                          std::map <int,int> &                  theEnforcedNodeIdToHybridIdMap,
//                          std::map <int,const SMDS_MeshNode*> & theHybridIdToNodeMap,
//                          HYBRIDPlugin_Hypothesis::TID2SizeMap & theNodeIDToSizeMap,
//                          HYBRIDPlugin_Hypothesis::THYBRIDEnforcedVertexCoordsValues & theEnforcedVertices,
//                          HYBRIDPlugin_Hypothesis::TIDSortedNodeGroupMap & theEnforcedNodes,
//                          HYBRIDPlugin_Hypothesis::TIDSortedElemGroupMap & theEnforcedEdges,
//                          HYBRIDPlugin_Hypothesis::TIDSortedElemGroupMap & theEnforcedTriangles)
// {
//   // record structure:
//   //
//   // NB_NODES
//   // Loop from 1 to NB_NODES
//   //   X Y Z DUMMY_INT

//   SMESHDS_Mesh * theMeshDS = theHelper.GetMeshDS();
//   int nbNodes = theMeshDS->NbNodes();
//   if ( nbNodes == 0 )
//     return false;
  
//   int nbEnforcedVertices = theEnforcedVertices.size();
//   int nbEnforcedNodes    = theEnforcedNodes.size();
  
//   const TopoDS_Shape shapeToMesh = theMeshDS->ShapeToMesh();
  
//   int aHybridID = 1;
//   SMDS_NodeIteratorPtr nodeIt = theMeshDS->nodesIterator();
//   const SMDS_MeshNode* node;

//   // Issue 020674: EDF 870 SMESH: Mesh generated by Netgen not usable by HYBRID
//   // The problem is in nodes on degenerated edges, we need to skip nodes which are free
//   // and replace not-free nodes on degenerated edges by the node on vertex
//   TNodeNodeMap n2nDegen; // map a node on degenerated edge to a node on vertex
//   TNodeNodeMap::iterator n2nDegenIt;
//   if ( theHelper.HasDegeneratedEdges() )
//   {
//     std::set<int> checkedSM;
//     for (TopExp_Explorer e(theMeshDS->ShapeToMesh(), TopAbs_EDGE ); e.More(); e.Next())
//     {
//       SMESH_subMesh* sm = theHelper.GetMesh()->GetSubMesh( e.Current() );
//       if ( checkedSM.insert( sm->GetId() ).second && theHelper.IsDegenShape(sm->GetId() ))
//       {
//         if ( SMESHDS_SubMesh* smDS = sm->GetSubMeshDS() )
//         {
//           TopoDS_Shape vertex = TopoDS_Iterator( e.Current() ).Value();
//           const SMDS_MeshNode* vNode = SMESH_Algo::VertexNode( TopoDS::Vertex( vertex ), theMeshDS);
//           {
//             SMDS_NodeIteratorPtr nIt = smDS->GetNodes();
//             while ( nIt->more() )
//               n2nDegen.insert( std::make_pair( nIt->next(), vNode ));
//           }
//         }
//       }
//     }
//     nbNodes -= n2nDegen.size();
//   }

//   const bool isQuadMesh = 
//     theHelper.GetMesh()->NbEdges( ORDER_QUADRATIC ) ||
//     theHelper.GetMesh()->NbFaces( ORDER_QUADRATIC ) ||
//     theHelper.GetMesh()->NbVolumes( ORDER_QUADRATIC );
//   if ( isQuadMesh )
//   {
//     // descrease nbNodes by nb of medium nodes
//     while ( nodeIt->more() )
//     {
//       node = nodeIt->next();
//       if ( !theHelper.IsDegenShape( node->getshapeId() ))
//         nbNodes -= int( theHelper.IsMedium( node ));
//     }
//     nodeIt = theMeshDS->nodesIterator();
//   }

//   const char* space    = "  ";
//   const int   dummyint = 0;

//   std::string tmpStr;
//   (nbNodes == 0 || nbNodes == 1) ? tmpStr = " node" : tmpStr = " nodes";
//   // NB_NODES
//   std::cout << std::endl;
//   std::cout << "The initial 2D mesh contains :" << std::endl;
//   std::cout << "    " << nbNodes << tmpStr << std::endl;
//   if (nbEnforcedVertices > 0) {
//     (nbEnforcedVertices == 1) ? tmpStr = "vertex" : tmpStr = "vertices";
//     std::cout << "    " << nbEnforcedVertices << " enforced " << tmpStr << std::endl;
//   }
//   if (nbEnforcedNodes > 0) {
//     (nbEnforcedNodes == 1) ? tmpStr = "node" : tmpStr = "nodes";
//     std::cout << "    " << nbEnforcedNodes << " enforced " << tmpStr << std::endl;
//   }
//   std::cout << std::endl;
//   std::cout << "Start writing in 'points' file ..." << std::endl;

//   theFile << nbNodes << std::endl;

//   // Loop from 1 to NB_NODES

//   while ( nodeIt->more() )
//   {
//     node = nodeIt->next();
//     if ( isQuadMesh && theHelper.IsMedium( node )) // Issue 0021238
//       continue;
//     if ( n2nDegen.count( node ) ) // Issue 0020674
//       continue;

//     theSmdsToHybridIdMap.insert( std::make_pair( node->GetID(), aHybridID ));
//     theHybridIdToNodeMap.insert( std::make_pair( aHybridID, node ));
//     aHybridID++;

//     // X Y Z DUMMY_INT
//     theFile
//     << node->X() << space 
//     << node->Y() << space 
//     << node->Z() << space 
//     << dummyint;

//     theFile << std::endl;

//   }
  
//   // Iterate over the enforced nodes
//   std::map<int,double> enfVertexIndexSizeMap;
//   if (nbEnforcedNodes) {
//     HYBRIDPlugin_Hypothesis::TIDSortedNodeGroupMap::const_iterator nodeIt = theEnforcedNodes.begin();
//     for( ; nodeIt != theEnforcedNodes.end() ; ++nodeIt) {
//       double x = nodeIt->first->X();
//       double y = nodeIt->first->Y();
//       double z = nodeIt->first->Z();
//       // Test if point is inside shape to mesh
//       gp_Pnt myPoint(x,y,z);
//       BRepClass3d_SolidClassifier scl(shapeToMesh);
//       scl.Perform(myPoint, 1e-7);
//       TopAbs_State result = scl.State();
//       if ( result != TopAbs_IN )
//         continue;
//       std::vector<double> coords;
//       coords.push_back(x);
//       coords.push_back(y);
//       coords.push_back(z);
//       if (theEnforcedVertices.find(coords) != theEnforcedVertices.end())
//         continue;
        
// //      double size = theNodeIDToSizeMap.find(nodeIt->first->GetID())->second;
//   //       theHybridIdToNodeMap.insert( std::make_pair( nbNodes + i, (*nodeIt) ));
//   //       MESSAGE("Adding enforced node (" << x << "," << y <<"," << z << ")");
//       // X Y Z PHY_SIZE DUMMY_INT
//       theFile
//       << x << space 
//       << y << space 
//       << z << space
//       << -1 << space
//       << dummyint << space;
//       theFile << std::endl;
//       theEnforcedNodeIdToHybridIdMap.insert( std::make_pair( nodeIt->first->GetID(), aHybridID ));
//       enfVertexIndexSizeMap[aHybridID] = -1;
//       aHybridID++;
//   //     else
//   //         MESSAGE("Enforced vertex (" << x << "," << y <<"," << z << ") is not inside the geometry: it was not added ");
//     }
//   }
  
//   if (nbEnforcedVertices) {
//     // Iterate over the enforced vertices
//     HYBRIDPlugin_Hypothesis::THYBRIDEnforcedVertexCoordsValues::const_iterator vertexIt = theEnforcedVertices.begin();
//     for( ; vertexIt != theEnforcedVertices.end() ; ++vertexIt) {
//       double x = vertexIt->first[0];
//       double y = vertexIt->first[1];
//       double z = vertexIt->first[2];
//       // Test if point is inside shape to mesh
//       gp_Pnt myPoint(x,y,z);
//       BRepClass3d_SolidClassifier scl(shapeToMesh);
//       scl.Perform(myPoint, 1e-7);
//       TopAbs_State result = scl.State();
//       if ( result != TopAbs_IN )
//         continue;
//       MESSAGE("Adding enforced vertex (" << x << "," << y <<"," << z << ") = " << vertexIt->second);
//       // X Y Z PHY_SIZE DUMMY_INT
//       theFile
//       << x << space 
//       << y << space 
//       << z << space
//       << vertexIt->second << space 
//       << dummyint << space;
//       theFile << std::endl;
//       enfVertexIndexSizeMap[aHybridID] = vertexIt->second;
//       aHybridID++;
//     }
//   }
  
  
//   std::cout << std::endl;
//   std::cout << "End writing in 'points' file." << std::endl;

//   return true;
// }

//=======================================================================
//function : readResultFile
//purpose  : readResultFile with geometry
//=======================================================================

// static bool readResultFile(const int                       fileOpen,
// #ifdef WIN32
//                            const char*                     fileName,
// #endif
//                            HYBRIDPlugin_HYBRID*            theAlgo,
//                            SMESH_MesherHelper&             theHelper,
//                            TopoDS_Shape                    tabShape[],
//                            double**                        tabBox,
//                            const int                       nbShape,
//                            std::map <int,const SMDS_MeshNode*>& theHybridIdToNodeMap,
//                            std::map <int,int> &            theNodeId2NodeIndexMap,
//                            bool                            toMeshHoles,
//                            int                             nbEnforcedVertices,
//                            int                             nbEnforcedNodes,
//                            HYBRIDPlugin_Hypothesis::TIDSortedElemGroupMap & theEnforcedEdges,
//                            HYBRIDPlugin_Hypothesis::TIDSortedElemGroupMap & theEnforcedTriangles,
//                            bool                            toMakeGroupsOfDomains)
// {
//   MESSAGE("HYBRIDPlugin_HYBRID::readResultFile()");
//   Kernel_Utils::Localizer loc;
//   struct stat status;
//   size_t      length;
  
//   std::string tmpStr;

//   char *ptr, *mapPtr;
//   char *tetraPtr;
//   char *shapePtr;

//   SMESHDS_Mesh* theMeshDS = theHelper.GetMeshDS();

//   int nbElems, nbNodes, nbInputNodes;
//   int nbTriangle;
//   int ID, shapeID, hybridShapeID;
//   int IdShapeRef = 1;
//   int compoundID =
//     nbShape ? theMeshDS->ShapeToIndex( tabShape[0] ) : theMeshDS->ShapeToIndex( theMeshDS->ShapeToMesh() );

//   int *tab, *tabID, *nodeID, *nodeAssigne;
//   double *coord;
//   const SMDS_MeshNode **node;

//   tab    = new int[3];
//   nodeID = new int[4];
//   coord  = new double[3];
//   node   = new const SMDS_MeshNode*[4];

//   TopoDS_Shape aSolid;
//   SMDS_MeshNode * aNewNode;
//   std::map <int,const SMDS_MeshNode*>::iterator itOnNode;
//   SMDS_MeshElement* aTet;
// #ifdef _DEBUG_
//   std::set<int> shapeIDs;
// #endif

//   // Read the file state
//   fstat(fileOpen, &status);
//   length   = status.st_size;

//   // Mapping the result file into memory
// #ifdef WIN32
//   HANDLE fd = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ,
//                          NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
//   HANDLE hMapObject = CreateFileMapping(fd, NULL, PAGE_READONLY,
//                                         0, (DWORD)length, NULL);
//   ptr = ( char* ) MapViewOfFile(hMapObject, FILE_MAP_READ, 0, 0, 0 );
// #else
//   ptr = (char *) mmap(0,length,PROT_READ,MAP_PRIVATE,fileOpen,0);
// #endif
//   mapPtr = ptr;

//   ptr      = readMapIntLine(ptr, tab);
//   tetraPtr = ptr;

//   nbElems      = tab[0];
//   nbNodes      = tab[1];
//   nbInputNodes = tab[2];

//   nodeAssigne = new int[ nbNodes+1 ];

//   if (nbShape > 0)
//     aSolid = tabShape[0];

//   // Reading the nodeId
//   for (int i=0; i < 4*nbElems; i++)
//     strtol(ptr, &ptr, 10);

//   MESSAGE("nbInputNodes: "<<nbInputNodes);
//   MESSAGE("nbEnforcedVertices: "<<nbEnforcedVertices);
//   MESSAGE("nbEnforcedNodes: "<<nbEnforcedNodes);
//   // Reading the nodeCoor and update the nodeMap
//   for (int iNode=1; iNode <= nbNodes; iNode++) {
//     if(theAlgo->computeCanceled())
//       return false;
//     for (int iCoor=0; iCoor < 3; iCoor++)
//       coord[ iCoor ] = strtod(ptr, &ptr);
//     nodeAssigne[ iNode ] = 1;
//     if ( iNode > (nbInputNodes-(nbEnforcedVertices+nbEnforcedNodes)) ) {
//       // Creating SMESH nodes
//       // - for enforced vertices
//       // - for vertices of forced edges
//       // - for hybrid nodes
//       nodeAssigne[ iNode ] = 0;
//       aNewNode = theMeshDS->AddNode( coord[0],coord[1],coord[2] );
//       theHybridIdToNodeMap.insert(theHybridIdToNodeMap.end(), std::make_pair( iNode, aNewNode ));
//     }
//   }

//   // Reading the number of triangles which corresponds to the number of sub-domains
//   nbTriangle = strtol(ptr, &ptr, 10);

//   tabID = new int[nbTriangle];
//   for (int i=0; i < nbTriangle; i++) {
//     if(theAlgo->computeCanceled())
//       return false;
//     tabID[i] = 0;
//     // find the solid corresponding to HYBRID sub-domain following
//     // the technique proposed in HYBRID manual in chapter
//     // "B.4 Subdomain (sub-region) assignment"
//     int nodeId1 = strtol(ptr, &ptr, 10);
//     int nodeId2 = strtol(ptr, &ptr, 10);
//     int nodeId3 = strtol(ptr, &ptr, 10);
//     if ( nbTriangle > 1 ) {
//       const SMDS_MeshNode* n1 = theHybridIdToNodeMap[ nodeId1 ];
//       const SMDS_MeshNode* n2 = theHybridIdToNodeMap[ nodeId2 ];
//       const SMDS_MeshNode* n3 = theHybridIdToNodeMap[ nodeId3 ];
//       if (!n1 || !n2 || !n3) {
//         tabID[i] = HOLE_ID;
//         continue;
//       }
//       try {
//         OCC_CATCH_SIGNALS;
// //         tabID[i] = findShapeID( theHelper, n1, n2, n3, toMeshHoles );
//         tabID[i] = findShapeID( *theHelper.GetMesh(), n1, n2, n3, toMeshHoles );
//         // -- 0020330: Pb with hybrid as a submesh
//         // check that found shape is to be meshed
//         if ( tabID[i] > 0 ) {
//           const TopoDS_Shape& foundShape = theMeshDS->IndexToShape( tabID[i] );
//           bool isToBeMeshed = false;
//           for ( int iS = 0; !isToBeMeshed && iS < nbShape; ++iS )
//             isToBeMeshed = foundShape.IsSame( tabShape[ iS ]);
//           if ( !isToBeMeshed )
//             tabID[i] = HOLE_ID;
//         }
//         // END -- 0020330: Pb with hybrid as a submesh
// #ifdef _DEBUG_
//         std::cout << i+1 << " subdomain: findShapeID() returns " << tabID[i] << std::endl;
// #endif
//       }
//       catch ( Standard_Failure & ex)
//       {
// #ifdef _DEBUG_
//         std::cout << i+1 << " subdomain: Exception caugt: " << ex.GetMessageString() << std::endl;
// #endif
//       }
//       catch (...) {
// #ifdef _DEBUG_
//         std::cout << i+1 << " subdomain: unknown exception caught " << std::endl;
// #endif
//       }
//     }
//   }

//   shapePtr = ptr;

//   if ( nbTriangle <= nbShape ) // no holes
//     toMeshHoles = true; // not avoid creating tetras in holes

//   // IMP 0022172: [CEA 790] create the groups corresponding to domains
//   std::vector< std::vector< const SMDS_MeshElement* > > elemsOfDomain( Max( nbTriangle, nbShape ));

//   // Associating the tetrahedrons to the shapes
//   shapeID = compoundID;
//   for (int iElem = 0; iElem < nbElems; iElem++) {
//     if(theAlgo->computeCanceled())
//       return false;
//     for (int iNode = 0; iNode < 4; iNode++) {
//       ID = strtol(tetraPtr, &tetraPtr, 10);
//       itOnNode = theHybridIdToNodeMap.find(ID);
//       node[ iNode ] = itOnNode->second;
//       nodeID[ iNode ] = ID;
//     }
//     // We always run HYBRID with "to mesh holes"==TRUE but we must not create
//     // tetras within holes depending on hypo option,
//     // so we first check if aTet is inside a hole and then create it 
//     //aTet = theMeshDS->AddVolume( node[1], node[0], node[2], node[3] );
//     hybridShapeID = 0; // domain ID
//     if ( nbTriangle > 1 ) {
//       shapeID = HOLE_ID; // negative shapeID means not to create tetras if !toMeshHoles
//       hybridShapeID = strtol(shapePtr, &shapePtr, 10) - IdShapeRef;
//       if ( tabID[ hybridShapeID ] == 0 ) {
//         TopAbs_State state;
//         aSolid = findShape(node, aSolid, tabShape, tabBox, nbShape, &state);
//         if ( toMeshHoles || state == TopAbs_IN )
//           shapeID = theMeshDS->ShapeToIndex( aSolid );
//         tabID[ hybridShapeID ] = shapeID;
//       }
//       else
//         shapeID = tabID[ hybridShapeID ];
//     }
//     else if ( nbShape > 1 ) {
//       // Case where nbTriangle == 1 while nbShape == 2 encountered
//       // with compound of 2 boxes and "To mesh holes"==False,
//       // so there are no subdomains specified for each tetrahedron.
//       // Try to guess a solid by a node already bound to shape
//       shapeID = 0;
//       for ( int i=0; i<4 && shapeID==0; i++ ) {
//         if ( nodeAssigne[ nodeID[i] ] == 1 &&
//              node[i]->GetPosition()->GetTypeOfPosition() == SMDS_TOP_3DSPACE &&
//              node[i]->getshapeId() > 1 )
//         {
//           shapeID = node[i]->getshapeId();
//         }
//       }
//       if ( shapeID==0 ) {
//         aSolid = findShape(node, aSolid, tabShape, tabBox, nbShape);
//         shapeID = theMeshDS->ShapeToIndex( aSolid );
//       }
//     }
//     // set new nodes and tetrahedron onto the shape
//     for ( int i=0; i<4; i++ ) {
//       if ( nodeAssigne[ nodeID[i] ] == 0 ) {
//         if ( shapeID != HOLE_ID )
//           theMeshDS->SetNodeInVolume( node[i], shapeID );
//         nodeAssigne[ nodeID[i] ] = shapeID;
//       }
//     }
//     if ( toMeshHoles || shapeID != HOLE_ID ) {
//       aTet = theHelper.AddVolume( node[1], node[0], node[2], node[3],
//                                   /*id=*/0, /*force3d=*/false);
//       theMeshDS->SetMeshElementOnShape( aTet, shapeID );
//       if ( toMakeGroupsOfDomains )
//       {
//         if ( int( elemsOfDomain.size() ) < hybridShapeID+1 )
//           elemsOfDomain.resize( hybridShapeID+1 );
//         elemsOfDomain[ hybridShapeID ].push_back( aTet );
//       }
//     }
// #ifdef _DEBUG_
//     shapeIDs.insert( shapeID );
// #endif
//   }
//   if ( toMakeGroupsOfDomains )
//     makeDomainGroups( elemsOfDomain, &theHelper );
  
//   // Add enforced elements
//   HYBRIDPlugin_Hypothesis::TIDSortedElemGroupMap::const_iterator elemIt;
//   const SMDS_MeshElement* anElem;
//   SMDS_ElemIteratorPtr itOnEnfElem;
//   std::map<int,int>::const_iterator itOnMap;
//   shapeID = compoundID;
//   // Enforced edges
//   if (theEnforcedEdges.size()) {
//     (theEnforcedEdges.size() <= 1) ? tmpStr = " enforced edge" : " enforced edges";
//     std::cout << "Add " << theEnforcedEdges.size() << tmpStr << std::endl;
//     std::vector< const SMDS_MeshNode* > node( 2 );
//     // Iterate over the enforced edges
//     for(elemIt = theEnforcedEdges.begin() ; elemIt != theEnforcedEdges.end() ; ++elemIt) {
//       anElem = elemIt->first;
//       bool addElem = true;
//       itOnEnfElem = anElem->nodesIterator();
//       for ( int j = 0; j < 2; ++j ) {
//         int aNodeID = itOnEnfElem->next()->GetID();
//         itOnMap = theNodeId2NodeIndexMap.find(aNodeID);
//         if (itOnMap != theNodeId2NodeIndexMap.end()) {
//           itOnNode = theHybridIdToNodeMap.find((*itOnMap).second);
//           if (itOnNode != theHybridIdToNodeMap.end()) {
//             node.push_back((*itOnNode).second);
// //             shapeID =(*itOnNode).second->getshapeId();
//           }
//           else
//             addElem = false;
//         }
//         else
//           addElem = false;
//       }
//       if (addElem) {
//         aTet = theHelper.AddEdge( node[0], node[1], 0,  false);
//         theMeshDS->SetMeshElementOnShape( aTet, shapeID );
//       }
//     }
//   }
//   // Enforced faces
//   if (theEnforcedTriangles.size()) {
//     (theEnforcedTriangles.size() <= 1) ? tmpStr = " enforced triangle" : " enforced triangles";
//     std::cout << "Add " << theEnforcedTriangles.size() << " enforced triangles" << std::endl;
//     std::vector< const SMDS_MeshNode* > node( 3 );
//     // Iterate over the enforced triangles
//     for(elemIt = theEnforcedTriangles.begin() ; elemIt != theEnforcedTriangles.end() ; ++elemIt) {
//       anElem = elemIt->first;
//       bool addElem = true;
//       itOnEnfElem = anElem->nodesIterator();
//       for ( int j = 0; j < 3; ++j ) {
//         int aNodeID = itOnEnfElem->next()->GetID();
//         itOnMap = theNodeId2NodeIndexMap.find(aNodeID);
//         if (itOnMap != theNodeId2NodeIndexMap.end()) {
//           itOnNode = theHybridIdToNodeMap.find((*itOnMap).second);
//           if (itOnNode != theHybridIdToNodeMap.end()) {
//             node.push_back((*itOnNode).second);
// //             shapeID =(*itOnNode).second->getshapeId();
//           }
//           else
//             addElem = false;
//         }
//         else
//           addElem = false;
//       }
//       if (addElem) {
//         aTet = theHelper.AddFace( node[0], node[1], node[2], 0,  false);
//         theMeshDS->SetMeshElementOnShape( aTet, shapeID );
//       }
//     }
//   }

//   // Remove nodes of tetras inside holes if !toMeshHoles
//   if ( !toMeshHoles ) {
//     itOnNode = theHybridIdToNodeMap.find( nbInputNodes );
//     for ( ; itOnNode != theHybridIdToNodeMap.end(); ++itOnNode) {
//       ID = itOnNode->first;
//       if ( nodeAssigne[ ID ] == HOLE_ID )
//         theMeshDS->RemoveFreeNode( itOnNode->second, 0 );
//     }
//   }

  
//   if ( nbElems ) {
//     (nbElems <= 1) ? tmpStr = " tetrahedra" : " tetrahedrons";
//     cout << nbElems << tmpStr << " have been associated to " << nbShape;
//     (nbShape <= 1) ? tmpStr = " shape" : " shapes";
//     cout << tmpStr << endl;
//   }
// #ifdef WIN32
//   UnmapViewOfFile(mapPtr);
//   CloseHandle(hMapObject);
//   CloseHandle(fd);
// #else
//   munmap(mapPtr, length);
// #endif
//   close(fileOpen);

//   delete [] tab;
//   delete [] tabID;
//   delete [] nodeID;
//   delete [] coord;
//   delete [] node;
//   delete [] nodeAssigne;

// #ifdef _DEBUG_
//   shapeIDs.erase(-1);
//   if ((int) shapeIDs.size() != nbShape ) {
//     (shapeIDs.size() <= 1) ? tmpStr = " solid" : " solids";
//     std::cout << "Only " << shapeIDs.size() << tmpStr << " of " << nbShape << " found" << std::endl;
//     for (int i=0; i<nbShape; i++) {
//       shapeID = theMeshDS->ShapeToIndex( tabShape[i] );
//       if ( shapeIDs.find( shapeID ) == shapeIDs.end() )
//         std::cout << "  Solid #" << shapeID << " not found" << std::endl;
//     }
//   }
// #endif

//   return true;
// }


//=============================================================================
/*!
 *Here we are going to use the HYBRID mesher with geometry
 */
//=============================================================================

bool HYBRIDPlugin_HYBRID::Compute(SMESH_Mesh&         theMesh,
                                  const TopoDS_Shape& theShape)
{
  bool Ok = false;

  // a unique working file name
  // to avoid access to the same files by eg different users
  _genericName = HYBRIDPlugin_Hypothesis::GetFileName(_hyp);
  TCollection_AsciiString aGenericName((char*) _genericName.c_str() );
  TCollection_AsciiString aGenericNameRequired = aGenericName + "_required";

  TCollection_AsciiString aLogFileName    = aGenericName + ".log";    // log
  TCollection_AsciiString aResultFileName;

  TCollection_AsciiString aGMFFileName, aRequiredVerticesFileName, aSolFileName, aResSolFileName;
//#ifdef _DEBUG_
  aGMFFileName              = aGenericName + ".mesh"; // GMF mesh file
  aResultFileName           = aGenericName + "Vol.mesh"; // GMF mesh file
  aResSolFileName           = aGenericName + "Vol.sol"; // GMF mesh file
  aRequiredVerticesFileName = aGenericNameRequired + ".mesh"; // GMF required vertices mesh file
  aSolFileName              = aGenericNameRequired + ".sol"; // GMF solution file
//#else
//  aGMFFileName    = aGenericName + ".meshb"; // GMF mesh file
//  aResultFileName = aGenericName + "Vol.meshb"; // GMF mesh file
//  aRequiredVerticesFileName    = aGenericNameRequired + ".meshb"; // GMF required vertices mesh file
//  aSolFileName    = aGenericNameRequired + ".solb"; // GMF solution file
//#endif
  
  std::map <int,int> aNodeId2NodeIndexMap, aSmdsToHybridIdMap, anEnforcedNodeIdToHybridIdMap;
  //std::map <int,const SMDS_MeshNode*> aHybridIdToNodeMap;
  std::map <int, int> nodeID2nodeIndexMap;
  std::map<std::vector<double>, std::string> enfVerticesWithGroup;
  HYBRIDPlugin_Hypothesis::THYBRIDEnforcedVertexCoordsValues coordsSizeMap = HYBRIDPlugin_Hypothesis::GetEnforcedVerticesCoordsSize(_hyp);
  HYBRIDPlugin_Hypothesis::TIDSortedNodeGroupMap enforcedNodes = HYBRIDPlugin_Hypothesis::GetEnforcedNodes(_hyp);
  HYBRIDPlugin_Hypothesis::TIDSortedElemGroupMap enforcedEdges = HYBRIDPlugin_Hypothesis::GetEnforcedEdges(_hyp);
  HYBRIDPlugin_Hypothesis::TIDSortedElemGroupMap enforcedTriangles = HYBRIDPlugin_Hypothesis::GetEnforcedTriangles(_hyp);
//   TIDSortedElemSet enforcedQuadrangles = HYBRIDPlugin_Hypothesis::GetEnforcedQuadrangles(_hyp);
  HYBRIDPlugin_Hypothesis::TID2SizeMap nodeIDToSizeMap = HYBRIDPlugin_Hypothesis::GetNodeIDToSizeMap(_hyp);

  HYBRIDPlugin_Hypothesis::THYBRIDEnforcedVertexList enfVertices = HYBRIDPlugin_Hypothesis::GetEnforcedVertices(_hyp);
  HYBRIDPlugin_Hypothesis::THYBRIDEnforcedVertexList::const_iterator enfVerIt = enfVertices.begin();
  std::vector<double> coords;

  for ( ; enfVerIt != enfVertices.end() ; ++enfVerIt)
  {
    HYBRIDPlugin_Hypothesis::THYBRIDEnforcedVertex* enfVertex = (*enfVerIt);
//     if (enfVertex->geomEntry.empty() && enfVertex->coords.size()) {
    if (enfVertex->coords.size()) {
      coordsSizeMap.insert(std::make_pair(enfVertex->coords,enfVertex->size));
      enfVerticesWithGroup.insert(std::make_pair(enfVertex->coords,enfVertex->groupName));
//       MESSAGE("enfVerticesWithGroup.insert(std::make_pair(("<<enfVertex->coords[0]<<","<<enfVertex->coords[1]<<","<<enfVertex->coords[2]<<"),\""<<enfVertex->groupName<<"\"))");
    }
    else {
//     if (!enfVertex->geomEntry.empty()) {
      TopoDS_Shape GeomShape = entryToShape(enfVertex->geomEntry);
//       GeomType = GeomShape.ShapeType();

//       if (!enfVertex->isCompound) {
// //       if (GeomType == TopAbs_VERTEX) {
//         coords.clear();
//         aPnt = BRep_Tool::Pnt(TopoDS::Vertex(GeomShape));
//         coords.push_back(aPnt.X());
//         coords.push_back(aPnt.Y());
//         coords.push_back(aPnt.Z());
//         if (coordsSizeMap.find(coords) == coordsSizeMap.end()) {
//           coordsSizeMap.insert(std::make_pair(coords,enfVertex->size));
//           enfVerticesWithGroup.insert(std::make_pair(coords,enfVertex->groupName));
//         }
//       }
//
//       // Group Management
//       else {
//       if (GeomType == TopAbs_COMPOUND){
        for (TopoDS_Iterator it (GeomShape); it.More(); it.Next()){
          coords.clear();
          if (it.Value().ShapeType() == TopAbs_VERTEX){
            gp_Pnt aPnt = BRep_Tool::Pnt(TopoDS::Vertex(it.Value()));
            coords.push_back(aPnt.X());
            coords.push_back(aPnt.Y());
            coords.push_back(aPnt.Z());
            if (coordsSizeMap.find(coords) == coordsSizeMap.end()) {
              coordsSizeMap.insert(std::make_pair(coords,enfVertex->size));
              enfVerticesWithGroup.insert(std::make_pair(coords,enfVertex->groupName));
//               MESSAGE("enfVerticesWithGroup.insert(std::make_pair(("<<coords[0]<<","<<coords[1]<<","<<coords[2]<<"),\""<<enfVertex->groupName<<"\"))");
            }
          }
        }
//       }
    }
  }
  int nbEnforcedVertices = coordsSizeMap.size();
  int nbEnforcedNodes = enforcedNodes.size();
  
  std::string tmpStr;
  (nbEnforcedNodes <= 1) ? tmpStr = "node" : "nodes";
  std::cout << nbEnforcedNodes << " enforced " << tmpStr << " from hypo" << std::endl;
  (nbEnforcedVertices <= 1) ? tmpStr = "vertex" : "vertices";
  std::cout << nbEnforcedVertices << " enforced " << tmpStr << " from hypo" << std::endl;

  SMESH_MesherHelper helper( theMesh );
  helper.SetSubShape( theShape );

  std::vector <const SMDS_MeshNode*> aNodeByHybridId, anEnforcedNodeByHybridId;
  std::vector <const SMDS_MeshElement*> aFaceByHybridId;
  std::map<const SMDS_MeshNode*,int> aNodeToHybridIdMap;
  std::vector<std::string> aNodeGroupByHybridId, anEdgeGroupByHybridId, aFaceGroupByHybridId;

  SMESH_ProxyMesh::Ptr proxyMesh( new SMESH_ProxyMesh( theMesh ));

  MG_HYBRID_API mgHybrid( _computeCanceled, _progress );

  Ok = writeGMFFile(&mgHybrid,
                    aGMFFileName.ToCString(),
                    aRequiredVerticesFileName.ToCString(),
                    aSolFileName.ToCString(),
                    *proxyMesh, helper,
                    aNodeByHybridId, aFaceByHybridId, aNodeToHybridIdMap,
                    aNodeGroupByHybridId, anEdgeGroupByHybridId, aFaceGroupByHybridId,
                    enforcedNodes, enforcedEdges, enforcedTriangles, /*enforcedQuadrangles,*/
                    enfVerticesWithGroup, coordsSizeMap);

  // Write aSmdsToHybridIdMap to temp file
  TCollection_AsciiString aSmdsToHybridIdMapFileName;
  aSmdsToHybridIdMapFileName = aGenericName + ".ids";  // ids relation
  ofstream aIdsFile  ( aSmdsToHybridIdMapFileName.ToCString()  , ios::out);
  Ok = aIdsFile.rdbuf()->is_open();
  if (!Ok) {
    INFOS( "Can't write into " << aSmdsToHybridIdMapFileName);
    return error(SMESH_Comment("Can't write into ") << aSmdsToHybridIdMapFileName);
  }
  INFOS( "Writing ids relation into " << aSmdsToHybridIdMapFileName);
  aIdsFile << "Smds Hybrid" << std::endl;
  std::map <int,int>::const_iterator myit;
  for (myit=aSmdsToHybridIdMap.begin() ; myit != aSmdsToHybridIdMap.end() ; ++myit) {
    aIdsFile << myit->first << " " << myit->second << std::endl;
  }

  aIdsFile.close();

  if ( ! Ok ) {
    if ( !_keepFiles ) {
      removeFile( aGMFFileName );
      removeFile( aRequiredVerticesFileName );
      removeFile( aSolFileName );
      removeFile( aSmdsToHybridIdMapFileName );
    }
    return error(COMPERR_BAD_INPUT_MESH);
  }
  removeFile( aResultFileName ); // needed for boundary recovery module usage

  // -----------------
  // run hybrid mesher
  // -----------------

  TCollection_AsciiString cmd( (char*)HYBRIDPlugin_Hypothesis::CommandToRun( _hyp ).c_str() );

  if ( mgHybrid.IsExecutable() )
  {
    cmd += TCollection_AsciiString(" --in ") + aGMFFileName;
    //if ( nbEnforcedVertices + nbEnforcedNodes)
    //  cmd += TCollection_AsciiString(" --required_vertices ") + aGenericNameRequired;
    cmd += TCollection_AsciiString(" --out ") + aResultFileName;
  }
  std::cout << std::endl;
  std::cout << "Hybrid execution with geometry..." << std::endl;
  std::cout << cmd;
  if ( !_logInStandardOutput )
  {
    mgHybrid.SetLogFile( aLogFileName.ToCString() );
    if ( mgHybrid.IsExecutable() )
      cmd += TCollection_AsciiString(" 1>" ) + aLogFileName;  // dump into file
    std::cout << " 1> " << aLogFileName;
  }
  std::cout << std::endl;

  _computeCanceled = false;

  std::string errStr;
  Ok = mgHybrid.Compute( cmd.ToCString(), errStr ); // run

  if ( _logInStandardOutput && mgHybrid.IsLibrary() )
    std::cout << std::endl << mgHybrid.GetLog() << std::endl;
  if ( Ok )
    std::cout << "End of Hybrid execution !" << std::endl;

  // --------------
  // read a result
  // --------------

  // Mapping the result file

    HYBRIDPlugin_Hypothesis::TSetStrings groupsToRemove = HYBRIDPlugin_Hypothesis::GetGroupsToRemove(_hyp);
    bool toMeshHoles =
      _hyp ? _hyp->GetToMeshHoles(true) : HYBRIDPlugin_Hypothesis::DefaultMeshHoles();
    const bool toMakeGroupsOfDomains = HYBRIDPlugin_Hypothesis::GetToMakeGroupsOfDomains( _hyp );

    helper.IsQuadraticSubMesh( theShape );
    helper.SetElementsOnShape( false );
                        
    Ok = readGMFFile(&mgHybrid, aResultFileName.ToCString(),
                     this,
                     &helper, aNodeByHybridId, aFaceByHybridId, aNodeToHybridIdMap,
                     aNodeGroupByHybridId, anEdgeGroupByHybridId, aFaceGroupByHybridId,
                     groupsToRemove, toMakeGroupsOfDomains, toMeshHoles);

    removeEmptyGroupsOfDomains( helper.GetMesh(), !toMakeGroupsOfDomains );




  // ---------------------
  // remove working files
  // ---------------------

  if ( Ok )
  {
    if ( _removeLogOnSuccess )
      removeFile( aLogFileName );

    // if ( _hyp && _hyp->GetToMakeGroupsOfDomains() )
    //   error( COMPERR_WARNING, "'toMakeGroupsOfDomains' is ignored since the mesh is on shape" );
  }
  else if ( mgHybrid.HasLog() )
  {
    // get problem description from the log file
    _Ghs2smdsConvertor conv( aNodeByHybridId );
    storeErrorDescription( _logInStandardOutput ? 0 : aLogFileName.ToCString(),
                           mgHybrid.GetLog(), conv );
  }
  else if ( !errStr.empty() )
  {
    // the log file is empty
    removeFile( aLogFileName );
    INFOS( "HYBRID Error, " << errStr );
    error(COMPERR_ALGO_FAILED, errStr );
  }

  if ( !_keepFiles ) {
    if (! Ok && _computeCanceled)
      removeFile( aLogFileName );
    removeFile( aGMFFileName );
    removeFile( aRequiredVerticesFileName );
    removeFile( aSolFileName );
    removeFile( aResSolFileName );
    removeFile( aResultFileName );
    removeFile( aSmdsToHybridIdMapFileName );
  }
  if ( mgHybrid.IsExecutable() )
  {
    std::cout << "<" << aResultFileName.ToCString() << "> HYBRID output file ";
    if ( !Ok )
      std::cout << "not ";
    std::cout << "treated !" << std::endl;
    std::cout << std::endl;
  }
  else
  {
    std::cout << "MG-HYBRID " << ( Ok ? "succeeded" : "failed") << std::endl;
  }

  return Ok;
}

//=============================================================================
/*!
 *Here we are going to use the HYBRID mesher w/o geometry
 */
//=============================================================================
bool HYBRIDPlugin_HYBRID::Compute(SMESH_Mesh&         theMesh,
                                  SMESH_MesherHelper* theHelper)
{
  MESSAGE("HYBRIDPlugin_HYBRID::Compute()");

  theHelper->IsQuadraticSubMesh( theHelper->GetSubShape() );

  // a unique working file name
  // to avoid access to the same files by eg different users
  _genericName = HYBRIDPlugin_Hypothesis::GetFileName(_hyp);
  TCollection_AsciiString aGenericName((char*) _genericName.c_str() );
  TCollection_AsciiString aGenericNameRequired = aGenericName + "_required";

  TCollection_AsciiString aLogFileName    = aGenericName + ".log";    // log
  TCollection_AsciiString aResultFileName;
  bool Ok;

  TCollection_AsciiString aGMFFileName, aRequiredVerticesFileName, aSolFileName, aResSolFileName;
//#ifdef _DEBUG_
  aGMFFileName              = aGenericName + ".mesh"; // GMF mesh file
  aResultFileName           = aGenericName + "Vol.mesh"; // GMF mesh file
  aResSolFileName           = aGenericName + "Vol.sol"; // GMF mesh file
  aRequiredVerticesFileName = aGenericNameRequired + ".mesh"; // GMF required vertices mesh file
  aSolFileName              = aGenericNameRequired + ".sol"; // GMF solution file
//#else
//  aGMFFileName    = aGenericName + ".meshb"; // GMF mesh file
//  aResultFileName = aGenericName + "Vol.meshb"; // GMF mesh file
//  aRequiredVerticesFileName    = aGenericNameRequired + ".meshb"; // GMF required vertices mesh file
//  aSolFileName    = aGenericNameRequired + ".solb"; // GMF solution file
//#endif

  std::map <int, int> nodeID2nodeIndexMap;
  std::map<std::vector<double>, std::string> enfVerticesWithGroup;
  HYBRIDPlugin_Hypothesis::THYBRIDEnforcedVertexCoordsValues coordsSizeMap;
  TopoDS_Shape GeomShape;
//   TopAbs_ShapeEnum GeomType;
  std::vector<double> coords;
  gp_Pnt aPnt;
  HYBRIDPlugin_Hypothesis::THYBRIDEnforcedVertex* enfVertex;

  HYBRIDPlugin_Hypothesis::THYBRIDEnforcedVertexList enfVertices = HYBRIDPlugin_Hypothesis::GetEnforcedVertices(_hyp);
  HYBRIDPlugin_Hypothesis::THYBRIDEnforcedVertexList::const_iterator enfVerIt = enfVertices.begin();

  for ( ; enfVerIt != enfVertices.end() ; ++enfVerIt)
  {
    enfVertex = (*enfVerIt);
//     if (enfVertex->geomEntry.empty() && enfVertex->coords.size()) {
    if (enfVertex->coords.size()) {
      coordsSizeMap.insert(std::make_pair(enfVertex->coords,enfVertex->size));
      enfVerticesWithGroup.insert(std::make_pair(enfVertex->coords,enfVertex->groupName));
//       MESSAGE("enfVerticesWithGroup.insert(std::make_pair(("<<enfVertex->coords[0]<<","<<enfVertex->coords[1]<<","<<enfVertex->coords[2]<<"),\""<<enfVertex->groupName<<"\"))");
    }
    else {
//     if (!enfVertex->geomEntry.empty()) {
      GeomShape = entryToShape(enfVertex->geomEntry);
//       GeomType = GeomShape.ShapeType();

//       if (!enfVertex->isCompound) {
// //       if (GeomType == TopAbs_VERTEX) {
//         coords.clear();
//         aPnt = BRep_Tool::Pnt(TopoDS::Vertex(GeomShape));
//         coords.push_back(aPnt.X());
//         coords.push_back(aPnt.Y());
//         coords.push_back(aPnt.Z());
//         if (coordsSizeMap.find(coords) == coordsSizeMap.end()) {
//           coordsSizeMap.insert(std::make_pair(coords,enfVertex->size));
//           enfVerticesWithGroup.insert(std::make_pair(coords,enfVertex->groupName));
//         }
//       }
//
//       // Group Management
//       else {
//       if (GeomType == TopAbs_COMPOUND){
        for (TopoDS_Iterator it (GeomShape); it.More(); it.Next()){
          coords.clear();
          if (it.Value().ShapeType() == TopAbs_VERTEX){
            aPnt = BRep_Tool::Pnt(TopoDS::Vertex(it.Value()));
            coords.push_back(aPnt.X());
            coords.push_back(aPnt.Y());
            coords.push_back(aPnt.Z());
            if (coordsSizeMap.find(coords) == coordsSizeMap.end()) {
              coordsSizeMap.insert(std::make_pair(coords,enfVertex->size));
              enfVerticesWithGroup.insert(std::make_pair(coords,enfVertex->groupName));
//               MESSAGE("enfVerticesWithGroup.insert(std::make_pair(("<<coords[0]<<","<<coords[1]<<","<<coords[2]<<"),\""<<enfVertex->groupName<<"\"))");
            }
          }
        }
//       }
    }
  }

//   const SMDS_MeshNode* enfNode;
  HYBRIDPlugin_Hypothesis::TIDSortedNodeGroupMap enforcedNodes = HYBRIDPlugin_Hypothesis::GetEnforcedNodes(_hyp);
//   HYBRIDPlugin_Hypothesis::TIDSortedNodeGroupMap::const_iterator enfNodeIt = enforcedNodes.begin();
//   for ( ; enfNodeIt != enforcedNodes.end() ; ++enfNodeIt)
//   {
//     enfNode = enfNodeIt->first;
//     coords.clear();
//     coords.push_back(enfNode->X());
//     coords.push_back(enfNode->Y());
//     coords.push_back(enfNode->Z());
//     if (enfVerticesWithGro
//       enfVerticesWithGroup.insert(std::make_pair(coords,enfNodeIt->second));
//   }


  HYBRIDPlugin_Hypothesis::TIDSortedElemGroupMap enforcedEdges = HYBRIDPlugin_Hypothesis::GetEnforcedEdges(_hyp);
  HYBRIDPlugin_Hypothesis::TIDSortedElemGroupMap enforcedTriangles = HYBRIDPlugin_Hypothesis::GetEnforcedTriangles(_hyp);
//   TIDSortedElemSet enforcedQuadrangles = HYBRIDPlugin_Hypothesis::GetEnforcedQuadrangles(_hyp);
  HYBRIDPlugin_Hypothesis::TID2SizeMap nodeIDToSizeMap = HYBRIDPlugin_Hypothesis::GetNodeIDToSizeMap(_hyp);

  std::string tmpStr;

  int nbEnforcedVertices = coordsSizeMap.size();
  int nbEnforcedNodes = enforcedNodes.size();
  (nbEnforcedNodes <= 1) ? tmpStr = "node" : tmpStr = "nodes";
  std::cout << nbEnforcedNodes << " enforced " << tmpStr << " from hypo" << std::endl;
  (nbEnforcedVertices <= 1) ? tmpStr = "vertex" : tmpStr = "vertices";
  std::cout << nbEnforcedVertices << " enforced " << tmpStr << " from hypo" << std::endl;

  std::vector <const SMDS_MeshNode*> aNodeByHybridId, anEnforcedNodeByHybridId;
  std::vector <const SMDS_MeshElement*> aFaceByHybridId;
  std::map<const SMDS_MeshNode*,int> aNodeToHybridIdMap;
  std::vector<std::string> aNodeGroupByHybridId, anEdgeGroupByHybridId, aFaceGroupByHybridId;

  SMESH_ProxyMesh::Ptr proxyMesh( new SMESH_ProxyMesh( theMesh ));

  MG_HYBRID_API mgHybrid( _computeCanceled, _progress );

  Ok = writeGMFFile(&mgHybrid,
                    aGMFFileName.ToCString(),
                    aRequiredVerticesFileName.ToCString(), aSolFileName.ToCString(),
                    *proxyMesh, *theHelper,
                    aNodeByHybridId, aFaceByHybridId, aNodeToHybridIdMap,
                    aNodeGroupByHybridId, anEdgeGroupByHybridId, aFaceGroupByHybridId,
                    enforcedNodes, enforcedEdges, enforcedTriangles,
                    enfVerticesWithGroup, coordsSizeMap);

  // -----------------
  // run hybrid mesher
  // -----------------

  TCollection_AsciiString cmd = HYBRIDPlugin_Hypothesis::CommandToRun( _hyp, false ).c_str();

  if ( mgHybrid.IsExecutable() )
  {
    cmd += TCollection_AsciiString(" --in ") + aGMFFileName;
    //if ( nbEnforcedVertices + nbEnforcedNodes)
    //  cmd += TCollection_AsciiString(" --required_vertices ") + aGenericNameRequired;
    cmd += TCollection_AsciiString(" --out ") + aResultFileName;
  }
  if ( !_logInStandardOutput )
  {
    cmd += TCollection_AsciiString(" 1> " ) + aLogFileName;  // dump into file
    mgHybrid.SetLogFile( aLogFileName.ToCString() );
  }
  std::cout << std::endl;
  std::cout << "Hybrid execution w/o geometry..." << std::endl;
  std::cout << cmd << std::endl;

  _computeCanceled = false;

  std::string errStr;
  Ok = mgHybrid.Compute( cmd.ToCString(), errStr ); // run

  if ( _logInStandardOutput && mgHybrid.IsLibrary() )
    std::cout << std::endl << mgHybrid.GetLog() << std::endl;
  if ( Ok )
    std::cout << "End of Hybrid execution !" << std::endl;

  // --------------
  // read a result
  // --------------
  HYBRIDPlugin_Hypothesis::TSetStrings groupsToRemove = HYBRIDPlugin_Hypothesis::GetGroupsToRemove(_hyp);
  const bool toMakeGroupsOfDomains = HYBRIDPlugin_Hypothesis::GetToMakeGroupsOfDomains( _hyp );

  Ok = Ok && readGMFFile(&mgHybrid,
                         aResultFileName.ToCString(),
                         this,
                         theHelper, aNodeByHybridId, aFaceByHybridId, aNodeToHybridIdMap,
                         aNodeGroupByHybridId, anEdgeGroupByHybridId, aFaceGroupByHybridId,
                         groupsToRemove, toMakeGroupsOfDomains);

  updateMeshGroups(theHelper->GetMesh(), groupsToRemove);
  //removeEmptyGroupsOfDomains( theHelper->GetMesh(), notEmptyAsWell );
  removeEmptyGroupsOfDomains( theHelper->GetMesh(), !toMakeGroupsOfDomains );

  if ( Ok ) {
    HYBRIDPlugin_Hypothesis* that = (HYBRIDPlugin_Hypothesis*)this->_hyp;
    if (that)
      that->ClearGroupsToRemove();
  }
  // ---------------------
  // remove working files
  // ---------------------

  if ( Ok )
  {
    if ( _removeLogOnSuccess )
      removeFile( aLogFileName );

    //if ( !toMakeGroupsOfDomains && _hyp && _hyp->GetToMakeGroupsOfDomains() )
    //error( COMPERR_WARNING, "'toMakeGroupsOfDomains' is ignored since 'toMeshHoles' is OFF." );
  }
  else if ( mgHybrid.HasLog() )
  {
    // get problem description from the log file
    _Ghs2smdsConvertor conv( aNodeByHybridId );
    storeErrorDescription( _logInStandardOutput ? 0 : aLogFileName.ToCString(),
                           mgHybrid.GetLog(), conv );
  }
  else {
    // the log file is empty
    removeFile( aLogFileName );
    INFOS( "HYBRID Error, command '" << cmd.ToCString() << "' failed" );
    error(COMPERR_ALGO_FAILED, "hybrid: command not found" );
  }

  if ( !_keepFiles )
  {
    if (! Ok && _computeCanceled)
      removeFile( aLogFileName );
    removeFile( aGMFFileName );
    removeFile( aResultFileName );
    removeFile( aRequiredVerticesFileName );
    removeFile( aSolFileName );
    removeFile( aResSolFileName );
  }
  return Ok;
}

void HYBRIDPlugin_HYBRID::CancelCompute()
{
  _computeCanceled = true;
#ifdef WIN32
#else
  std::string cmd = "ps xo pid,args | grep " + _genericName;
  //cmd += " | grep -e \"^ *[0-9]\\+ \\+" + HYBRIDPlugin_Hypothesis::GetExeName() + "\"";
  cmd += " | awk '{print $1}' | xargs kill -9 > /dev/null 2>&1";
  system( cmd.c_str() );
#endif
}

//================================================================================
/*!
 * \brief Provide human readable text by error code reported by hybrid
 */
//================================================================================

static const char* translateError(const int errNum)
{
  switch ( errNum ) {
  case 0:
    return "error distene 0";
  case 1:
    return "error distene 1";
  }
  return "unknown distene error";
}

//================================================================================
/*!
 * \brief Retrieve from a string given number of integers
 */
//================================================================================

static char* getIds( char* ptr, int nbIds, std::vector<int>& ids )
{
  ids.clear();
  ids.reserve( nbIds );
  while ( nbIds )
  {
    while ( !isdigit( *ptr )) ++ptr;
    if ( ptr[-1] == '-' ) --ptr;
    ids.push_back( strtol( ptr, &ptr, 10 ));
    --nbIds;
  }
  return ptr;
}

//================================================================================
/*!
 * \brief Retrieve problem description form a log file
 *  \retval bool - always false
 */
//================================================================================

bool HYBRIDPlugin_HYBRID::storeErrorDescription(const char*                logFile,
                                                const std::string&         log,
                                                const _Ghs2smdsConvertor & toSmdsConvertor )
{
  if(_computeCanceled)
    return error(SMESH_Comment("interruption initiated by user"));

  char* ptr = const_cast<char*>( log.c_str() );
  char* buf = ptr, * bufEnd = ptr + log.size();

  SMESH_Comment errDescription;

  enum { NODE = 1, EDGE, TRIA, VOL, SKIP_ID = 1 };

  // look for MeshGems version
  // Since "MG-TETRA -- MeshGems 1.1-3 (January, 2013)" error codes change.
  // To discriminate old codes from new ones we add 1000000 to the new codes.
  // This way value of the new codes is same as absolute value of codes printed
  // in the log after "MGMESSAGE" string.
  int versionAddition = 0;
  {
    char* verPtr = ptr;
    while ( ++verPtr < bufEnd )
    {
      if ( strncmp( verPtr, "MG-TETRA -- MeshGems ", 21 ) != 0 )
        continue;
      if ( strcmp( verPtr, "MG-TETRA -- MeshGems 1.1-3 " ) >= 0 )
        versionAddition = 1000000;
      ptr = verPtr;
      break;
    }
  }

  // look for errors "ERR #"

  std::set<std::string> foundErrorStr; // to avoid reporting same error several times
  std::set<int>         elemErrorNums; // not to report different types of errors with bad elements
  while ( ++ptr < bufEnd )
  {
    if ( strncmp( ptr, "ERR ", 4 ) != 0 )
      continue;

    std::list<const SMDS_MeshElement*> badElems;
    std::vector<int> nodeIds;

    ptr += 4;
    char* errBeg = ptr;
    int   errNum = strtol(ptr, &ptr, 10) + versionAddition;
    // we treat errors enumerated in [SALOME platform 0019316] issue
    // and all errors from a new (Release 1.1) MeshGems User Manual
    switch ( errNum ) {
    case 0015: // The face number (numfac) with vertices (f 1, f 2, f 3) has a null vertex.
    case 1005620 : // a too bad quality face is detected. This face is considered degenerated.
      ptr = getIds(ptr, SKIP_ID, nodeIds);
      ptr = getIds(ptr, TRIA, nodeIds);
      badElems.push_back( toSmdsConvertor.getElement(nodeIds));
      break;
    case 1005621 : // a too bad quality face is detected. This face is degenerated.
      // hence the is degenerated it is invisible, add its edges in addition
      ptr = getIds(ptr, SKIP_ID, nodeIds);
      ptr = getIds(ptr, TRIA, nodeIds);
      badElems.push_back( toSmdsConvertor.getElement(nodeIds));
      {
        std::vector<int> edgeNodes( nodeIds.begin(), --nodeIds.end() ); // 01
        badElems.push_back( toSmdsConvertor.getElement(edgeNodes));
        edgeNodes[1] = nodeIds[2]; // 02
        badElems.push_back( toSmdsConvertor.getElement(edgeNodes));
        edgeNodes[0] = nodeIds[1]; // 12
      }      
      break;
    case 1000: // Face (f 1, f 2, f 3) appears more than once in the input surface mesh.
      // ERR  1000 :  1 3 2
    case 1002: // Face (f 1, f 2, f 3) has a vertex negative or null
    case 3019: // Constrained face (f 1, f 2, f 3) cannot be enforced
    case 1002211: // a face has a vertex negative or null.
    case 1005200 : // a surface mesh appears more than once in the input surface mesh.
    case 1008423 : // a constrained face cannot be enforced (regeneration phase failed).
      ptr = getIds(ptr, TRIA, nodeIds);
      badElems.push_back( toSmdsConvertor.getElement(nodeIds));
      break;
    case 1001: // Edge (e1, e2) appears more than once in the input surface mesh
    case 3009: // Constrained edge (e1, e2) cannot be enforced (warning).
      // ERR  3109 :  EDGE  5 6 UNIQUE
    case 3109: // Edge (e1, e2) is unique (i.e., bounds a hole in the surface)
    case 1005210 : // an edge appears more than once in the input surface mesh.
    case 1005820 : // an edge is unique (i.e., bounds a hole in the surface).
    case 1008441 : // a constrained edge cannot be enforced.
      ptr = getIds(ptr, EDGE, nodeIds);
      badElems.push_back( toSmdsConvertor.getElement(nodeIds));
      break;
    case 2004: // Vertex v1 and vertex v2 are too close to one another or coincident (warning).
    case 2014: // at least two points whose distance is dist, i.e., considered as coincident
    case 2103: // Vertex v1 and vertex v2 are too close to one another or coincident (warning).
      // ERR  2103 :  16 WITH  3
    case 1005105 : // two vertices are too close to one another or coincident.
    case 1005107: // Two vertices are too close to one another or coincident.
      ptr = getIds(ptr, NODE, nodeIds);
      badElems.push_back( toSmdsConvertor.getElement(nodeIds));
      ptr = getIds(ptr, NODE, nodeIds);
      badElems.push_back( toSmdsConvertor.getElement(nodeIds));
      break;
    case 2012: // Vertex v1 cannot be inserted (warning).
    case 1005106 : // a vertex cannot be inserted.
      ptr = getIds(ptr, NODE, nodeIds);
      badElems.push_back( toSmdsConvertor.getElement(nodeIds));
      break;
    case 3103: // The surface edge (e1, e2) intersects another surface edge (e3, e4)
    case 1005110 : // two surface edges are intersecting.
      // ERR  3103 :  1 2 WITH  7 3
      ptr = getIds(ptr, EDGE, nodeIds);
      badElems.push_back( toSmdsConvertor.getElement(nodeIds));
      ptr = getIds(ptr, EDGE, nodeIds);
      badElems.push_back( toSmdsConvertor.getElement(nodeIds));
      break;
    case 3104: // The surface edge (e1, e2) intersects the surface face (f 1, f 2, f 3)
      // ERR  3104 :  9 10 WITH  1 2 3
    case 3106: // One surface edge (say e1, e2) intersects a surface face (f 1, f 2, f 3)
    case 1005120 : // a surface edge intersects a surface face.
      ptr = getIds(ptr, EDGE, nodeIds);
      badElems.push_back( toSmdsConvertor.getElement(nodeIds));
      ptr = getIds(ptr, TRIA, nodeIds);
      badElems.push_back( toSmdsConvertor.getElement(nodeIds));
      break;
    case 3105: // One boundary point (say p1) lies within a surface face (f 1, f 2, f 3)
      // ERR  3105 :  8 IN  2 3 5
    case 1005150 : // a boundary point lies within a surface face.
      ptr = getIds(ptr, NODE, nodeIds);
      badElems.push_back( toSmdsConvertor.getElement(nodeIds));
      ptr = getIds(ptr, TRIA, nodeIds);
      badElems.push_back( toSmdsConvertor.getElement(nodeIds));
      break;
    case 3107: // One boundary point (say p1) lies within a surface edge (e1, e2) (stop).
      // ERR  3107 :  2 IN  4 1
    case 1005160 : // a boundary point lies within a surface edge.
      ptr = getIds(ptr, NODE, nodeIds);
      badElems.push_back( toSmdsConvertor.getElement(nodeIds));
      ptr = getIds(ptr, EDGE, nodeIds);
      badElems.push_back( toSmdsConvertor.getElement(nodeIds));
      break;
    case 9000: // ERR  9000
      //  ELEMENT  261 WITH VERTICES :  7 396 -8 242
      //  VOLUME   : -1.11325045E+11 W.R.T. EPSILON   0.
      // A too small volume element is detected. Are reported the index of the element,
      // its four vertex indices, its volume and the tolerance threshold value
      ptr = getIds(ptr, SKIP_ID, nodeIds);
      ptr = getIds(ptr, VOL, nodeIds);
      badElems.push_back( toSmdsConvertor.getElement(nodeIds));
      // even if all nodes found, volume it most probably invisible,
      // add its faces to demonstrate it anyhow
      {
        std::vector<int> faceNodes( nodeIds.begin(), --nodeIds.end() ); // 012
        badElems.push_back( toSmdsConvertor.getElement(faceNodes));
        faceNodes[2] = nodeIds[3]; // 013
        badElems.push_back( toSmdsConvertor.getElement(faceNodes));
        faceNodes[1] = nodeIds[2]; // 023
        badElems.push_back( toSmdsConvertor.getElement(faceNodes));
        faceNodes[0] = nodeIds[1]; // 123
        badElems.push_back( toSmdsConvertor.getElement(faceNodes));
      }
      break;
    case 9001: // ERR  9001
      //  %% NUMBER OF NEGATIVE VOLUME TETS  :  1
      //  %% THE LARGEST NEGATIVE TET        :   1.75376581E+11
      //  %%  NUMBER OF NULL VOLUME TETS     :  0
      // There exists at least a null or negative volume element
      break;
    case 9002:
      // There exist n null or negative volume elements
      break;
    case 9003:
      // A too small volume element is detected
      break;
    case 9102:
      // A too bad quality face is detected. This face is considered degenerated,
      // its index, its three vertex indices together with its quality value are reported
      break; // same as next
    case 9112: // ERR  9112
      //  FACE   2 WITH VERTICES :  4 2 5
      //  SMALL INRADIUS :   0.
      // A too bad quality face is detected. This face is degenerated,
      // its index, its three vertex indices together with its inradius are reported
      ptr = getIds(ptr, SKIP_ID, nodeIds);
      ptr = getIds(ptr, TRIA, nodeIds);
      badElems.push_back( toSmdsConvertor.getElement(nodeIds));
      // add triangle edges as it most probably has zero area and hence invisible
      {
        std::vector<int> edgeNodes(2);
        edgeNodes[0] = nodeIds[0]; edgeNodes[1] = nodeIds[1]; // 0-1
        badElems.push_back( toSmdsConvertor.getElement(edgeNodes));
        edgeNodes[1] = nodeIds[2]; // 0-2
        badElems.push_back( toSmdsConvertor.getElement(edgeNodes));
        edgeNodes[0] = nodeIds[1]; // 1-2
        badElems.push_back( toSmdsConvertor.getElement(edgeNodes));
      }
      break;
    case 1005103 : // the vertices of an element are too close to one another or coincident.
      ptr = getIds(ptr, TRIA, nodeIds);
      if ( nodeIds.back() == 0 ) // index of the third vertex of the element (0 for an edge)
        nodeIds.resize( EDGE );
      badElems.push_back( toSmdsConvertor.getElement(nodeIds));
      break;
    }

    bool isNewError = foundErrorStr.insert( std::string( errBeg, ptr )).second;
    if ( !isNewError )
      continue; // not to report same error several times

//     const SMDS_MeshElement* nullElem = 0;
//     bool allElemsOk = ( find( badElems.begin(), badElems.end(), nullElem) == badElems.end());

//     if ( allElemsOk && !badElems.empty() && !elemErrorNums.empty() ) {
//       bool oneMoreErrorType = elemErrorNums.insert( errNum ).second;
//       if ( oneMoreErrorType )
//         continue; // not to report different types of errors with bad elements
//     }

    // store bad elements
    //if ( allElemsOk ) {
      std::list<const SMDS_MeshElement*>::iterator elem = badElems.begin();
      for ( ; elem != badElems.end(); ++elem )
        addBadInputElement( *elem );
      //}

    // make error text
    std::string text = translateError( errNum );
    if ( errDescription.find( text ) == text.npos ) {
      if ( !errDescription.empty() )
        errDescription << "\n";
      errDescription << text;
    }

  } // end while

  if ( errDescription.empty() ) { // no errors found
    char msgLic1[] = "connection to server failed";
    char msgLic2[] = " Dlim ";
    if ( std::search( &buf[0], bufEnd, msgLic1, msgLic1 + strlen(msgLic1)) != bufEnd ||
         std::search( &buf[0], bufEnd, msgLic2, msgLic2 + strlen(msgLic2)) != bufEnd )
      errDescription << "Licence problems.";
    else
    {
      char msg2[] = "SEGMENTATION FAULT";
      if ( std::search( &buf[0], bufEnd, msg2, msg2 + strlen(msg2)) != bufEnd )
        errDescription << "hybrid: SEGMENTATION FAULT. ";
    }
  }

  if ( logFile && logFile[0] )
  {
    if ( errDescription.empty() )
      errDescription << "See " << logFile << " for problem description";
    else
      errDescription << "\nSee " << logFile << " for more information";
  }
  return error( errDescription );
}

//================================================================================
/*!
 * \brief Creates _Ghs2smdsConvertor
 */
//================================================================================

_Ghs2smdsConvertor::_Ghs2smdsConvertor( const std::map <int,const SMDS_MeshNode*> & ghs2NodeMap)
  :_ghs2NodeMap( & ghs2NodeMap ), _nodeByGhsId( 0 )
{
}

//================================================================================
/*!
 * \brief Creates _Ghs2smdsConvertor
 */
//================================================================================

_Ghs2smdsConvertor::_Ghs2smdsConvertor( const std::vector <const SMDS_MeshNode*> &  nodeByGhsId)
  : _ghs2NodeMap( 0 ), _nodeByGhsId( &nodeByGhsId )
{
}

//================================================================================
/*!
 * \brief Return SMDS element by ids of HYBRID nodes
 */
//================================================================================

const SMDS_MeshElement* _Ghs2smdsConvertor::getElement(const std::vector<int>& ghsNodes) const
{
  size_t nbNodes = ghsNodes.size();
  std::vector<const SMDS_MeshNode*> nodes( nbNodes, 0 );
  for ( size_t i = 0; i < nbNodes; ++i ) {
    int ghsNode = ghsNodes[ i ];
    if ( _ghs2NodeMap ) {
      std::map <int,const SMDS_MeshNode*>::const_iterator in = _ghs2NodeMap->find( ghsNode);
      if ( in == _ghs2NodeMap->end() )
        return 0;
      nodes[ i ] = in->second;
    }
    else {
      if ( ghsNode < 1 || ghsNode > (int)_nodeByGhsId->size() )
        return 0;
      nodes[ i ] = (*_nodeByGhsId)[ ghsNode-1 ];
    }
  }
  if ( nbNodes == 1 )
    return nodes[0];

  if ( nbNodes == 2 ) {
    const SMDS_MeshElement* edge= SMDS_Mesh::FindEdge( nodes[0], nodes[1] );
    if ( !edge )
      edge = new SMDS_LinearEdge( nodes[0], nodes[1] );
    return edge;
  }
  if ( nbNodes == 3 ) {
    const SMDS_MeshElement* face = SMDS_Mesh::FindFace( nodes );
    if ( !face )
      face = new SMDS_FaceOfNodes( nodes[0], nodes[1], nodes[2] );
    return face;
  }
  if ( nbNodes == 4 )
    return new SMDS_VolumeOfNodes( nodes[0], nodes[1], nodes[2], nodes[3] );

  return 0;
}


//=============================================================================
/*!
 *
 */
//=============================================================================
bool HYBRIDPlugin_HYBRID::Evaluate(SMESH_Mesh& aMesh,
                                 const TopoDS_Shape& aShape,
                                 MapShapeNbElems& aResMap)
{
  int nbtri = 0, nbqua = 0;
  double fullArea = 0.0;
  for (TopExp_Explorer exp(aShape, TopAbs_FACE); exp.More(); exp.Next()) {
    TopoDS_Face F = TopoDS::Face( exp.Current() );
    SMESH_subMesh *sm = aMesh.GetSubMesh(F);
    MapShapeNbElemsItr anIt = aResMap.find(sm);
    if( anIt==aResMap.end() ) {
      SMESH_ComputeErrorPtr& smError = sm->GetComputeError();
      smError.reset( new SMESH_ComputeError(COMPERR_ALGO_FAILED,
                                            "Submesh can not be evaluated",this));
      return false;
    }
    std::vector<int> aVec = (*anIt).second;
    nbtri += Max(aVec[SMDSEntity_Triangle],aVec[SMDSEntity_Quad_Triangle]);
    nbqua += Max(aVec[SMDSEntity_Quadrangle],aVec[SMDSEntity_Quad_Quadrangle]);
    GProp_GProps G;
    BRepGProp::SurfaceProperties(F,G);
    double anArea = G.Mass();
    fullArea += anArea;
  }

  // collect info from edges
  int nb0d_e = 0, nb1d_e = 0;
  bool IsQuadratic = false;
  bool IsFirst = true;
  TopTools_MapOfShape tmpMap;
  for (TopExp_Explorer exp(aShape, TopAbs_EDGE); exp.More(); exp.Next()) {
    TopoDS_Edge E = TopoDS::Edge(exp.Current());
    if( tmpMap.Contains(E) )
      continue;
    tmpMap.Add(E);
    SMESH_subMesh *aSubMesh = aMesh.GetSubMesh(exp.Current());
    MapShapeNbElemsItr anIt = aResMap.find(aSubMesh);
    std::vector<int> aVec = (*anIt).second;
    nb0d_e += aVec[SMDSEntity_Node];
    nb1d_e += Max(aVec[SMDSEntity_Edge],aVec[SMDSEntity_Quad_Edge]);
    if(IsFirst) {
      IsQuadratic = (aVec[SMDSEntity_Quad_Edge] > aVec[SMDSEntity_Edge]);
      IsFirst = false;
    }
  }
  tmpMap.Clear();

  double ELen = sqrt(2.* ( fullArea/(nbtri+nbqua*2) ) / sqrt(3.0) );

  GProp_GProps G;
  BRepGProp::VolumeProperties(aShape,G);
  double aVolume = G.Mass();
  double tetrVol = 0.1179*ELen*ELen*ELen;
  double CoeffQuality = 0.9;
  int nbVols = int(aVolume/tetrVol/CoeffQuality);
  int nb1d_f = (nbtri*3 + nbqua*4 - nb1d_e) / 2;
  int nb1d_in = (int) ( nbVols*6 - nb1d_e - nb1d_f ) / 5;
  std::vector<int> aVec(SMDSEntity_Last);
  for(int i=SMDSEntity_Node; i<SMDSEntity_Last; i++) aVec[i]=0;
  if( IsQuadratic ) {
    aVec[SMDSEntity_Node] = nb1d_in/6 + 1 + nb1d_in;
    aVec[SMDSEntity_Quad_Tetra] = nbVols - nbqua*2;
    aVec[SMDSEntity_Quad_Pyramid] = nbqua;
  }
  else {
    aVec[SMDSEntity_Node] = nb1d_in/6 + 1;
    aVec[SMDSEntity_Tetra] = nbVols - nbqua*2;
    aVec[SMDSEntity_Pyramid] = nbqua;
  }
  SMESH_subMesh *sm = aMesh.GetSubMesh(aShape);
  aResMap.insert(std::make_pair(sm,aVec));

  return true;
}

bool HYBRIDPlugin_HYBRID::importGMFMesh(const char* theGMFFileName, SMESH_Mesh& theMesh)
{
  SMESH_ComputeErrorPtr err = theMesh.GMFToMesh( theGMFFileName, /*makeRequiredGroups =*/ true );

  theMesh.GetMeshDS()->Modified();

  return ( !err || err->IsOK());
}

namespace
{
  //================================================================================
  /*!
   * \brief Sub-mesh event listener setting enforced elements as soon as an enforced
   *        mesh is loaded
   */
  struct _EnforcedMeshRestorer : public SMESH_subMeshEventListener
  {
    _EnforcedMeshRestorer():
      SMESH_subMeshEventListener( /*isDeletable = */true, Name() )
    {}

    //================================================================================
    /*!
     * \brief Returns an ID of listener
     */
    static const char* Name() { return "HYBRIDPlugin_HYBRID::_EnforcedMeshRestorer"; }

    //================================================================================
    /*!
     * \brief Treat events of the subMesh
     */
    void ProcessEvent(const int                       event,
                      const int                       eventType,
                      SMESH_subMesh*                  subMesh,
                      SMESH_subMeshEventListenerData* data,
                      const SMESH_Hypothesis*         hyp)
    {
      if ( SMESH_subMesh::SUBMESH_LOADED == event &&
           SMESH_subMesh::COMPUTE_EVENT  == eventType &&
           data &&
           !data->mySubMeshes.empty() )
      {
        // An enforced mesh (subMesh->_father) has been loaded from hdf file
        if ( HYBRIDPlugin_Hypothesis* hyp = GetGHSHypothesis( data->mySubMeshes.front() ))
          hyp->RestoreEnfElemsByMeshes();
      }
    }
    //================================================================================
    /*!
     * \brief Returns HYBRIDPlugin_Hypothesis used to compute a subMesh
     */
    static HYBRIDPlugin_Hypothesis* GetGHSHypothesis( SMESH_subMesh* subMesh )
    {
      SMESH_HypoFilter ghsHypFilter( SMESH_HypoFilter::HasName( "HYBRID_Parameters" ));
      return (HYBRIDPlugin_Hypothesis* )
        subMesh->GetFather()->GetHypothesis( subMesh->GetSubShape(),
                                             ghsHypFilter,
                                             /*visitAncestors=*/true);
    }
  };

  //================================================================================
  /*!
   * \brief Sub-mesh event listener removing empty groups created due to "To make
   *        groups of domains".
   */
  struct _GroupsOfDomainsRemover : public SMESH_subMeshEventListener
  {
    _GroupsOfDomainsRemover():
      SMESH_subMeshEventListener( /*isDeletable = */true,
                                  "HYBRIDPlugin_HYBRID::_GroupsOfDomainsRemover" ) {}
    /*!
     * \brief Treat events of the subMesh
     */
    void ProcessEvent(const int                       event,
                      const int                       eventType,
                      SMESH_subMesh*                  subMesh,
                      SMESH_subMeshEventListenerData* data,
                      const SMESH_Hypothesis*         hyp)
    {
      if (SMESH_subMesh::ALGO_EVENT == eventType &&
          !subMesh->GetAlgo() )
      {
        removeEmptyGroupsOfDomains( subMesh->GetFather(), /*notEmptyAsWell=*/true );
      }
    }
  };
}

//================================================================================
/*!
 * \brief Set an event listener to set enforced elements as soon as an enforced
 *        mesh is loaded
 */
//================================================================================

void HYBRIDPlugin_HYBRID::SubmeshRestored(SMESH_subMesh* subMesh)
{
  if ( HYBRIDPlugin_Hypothesis* hyp = _EnforcedMeshRestorer::GetGHSHypothesis( subMesh ))
  {
    HYBRIDPlugin_Hypothesis::THYBRIDEnforcedMeshList enfMeshes = hyp->_GetEnforcedMeshes();
    HYBRIDPlugin_Hypothesis::THYBRIDEnforcedMeshList::iterator it = enfMeshes.begin();
    for(;it != enfMeshes.end();++it) {
      HYBRIDPlugin_Hypothesis::THYBRIDEnforcedMesh* enfMesh = *it;
      if ( SMESH_Mesh* mesh = GetMeshByPersistentID( enfMesh->persistID ))
      {
        SMESH_subMesh* smToListen = mesh->GetSubMesh( mesh->GetShapeToMesh() );
        // a listener set to smToListen will care of hypothesis stored in SMESH_EventListenerData
        subMesh->SetEventListener( new _EnforcedMeshRestorer(),
                                   SMESH_subMeshEventListenerData::MakeData( subMesh ),
                                   smToListen);
      }
    }
  }
}

//================================================================================
/*!
 * \brief Sets an event listener removing empty groups created due to "To make
 *        groups of domains".
 * \param subMesh - submesh where algo is set
 *
 * This method is called when a submesh gets HYP_OK algo_state.
 * After being set, event listener is notified on each event of a submesh.
 */
//================================================================================

void HYBRIDPlugin_HYBRID::SetEventListener(SMESH_subMesh* subMesh)
{
  subMesh->SetEventListener( new _GroupsOfDomainsRemover(), 0, subMesh );
}
