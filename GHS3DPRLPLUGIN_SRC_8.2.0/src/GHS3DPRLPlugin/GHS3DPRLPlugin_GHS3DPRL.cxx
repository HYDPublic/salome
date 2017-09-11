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
// File   : GHS3DPRLPlugin_GHS3DPRL.cxx
// Author : Christian VAN WAMBEKE (CEA) (from Hexotic plugin Lioka RAZAFINDRAZAKA)
// ---
//
#include "GHS3DPRLPlugin_GHS3DPRL.hxx"
#include "GHS3DPRLPlugin_Hypothesis.hxx"
#include "MG_TetraHPC_API.hxx"

#include <SMESHDS_Mesh.hxx>
#include <SMESH_Gen.hxx>
#include <SMESH_TypeDefs.hxx>
#include <SMESH_subMesh.hxx>
#include <SMESH_MesherHelper.hxx>

#include "utilities.h"

#include <list>

#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <OSD_File.hxx>
#include <Standard_ProgramError.hxx>
#include <TCollection_AsciiString.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>

#define GMFVERSION GmfDouble
#define GMFDIMENSION 3

using namespace std;

static void removeFile( const TCollection_AsciiString& fileName )
{
  try {
    OSD_File( fileName ).Remove();
  }
  catch ( Standard_ProgramError ) {
    MESSAGE("Can't remove file: " << fileName.ToCString() << " ; file does not exist or permission denied");
  }
}

//=============================================================================
GHS3DPRLPlugin_GHS3DPRL::GHS3DPRLPlugin_GHS3DPRL(int hypId, int studyId, SMESH_Gen* gen)
  : SMESH_3D_Algo(hypId, studyId, gen)
{
  MESSAGE("GHS3DPRLPlugin_GHS3DPRL::GHS3DPRLPlugin_GHS3DPRL");
  _name = "MG-Tetra Parallel";
  _shapeType = (1 << TopAbs_SHELL) | (1 << TopAbs_SOLID);// 1 bit /shape type
  _onlyUnaryInput = false; // Compute() will be called on a compound of solids
  _countSubMesh=0;
  _nodeRefNumber=0;
  _compatibleHypothesis.push_back(GHS3DPRLPlugin_Hypothesis::GetHypType());
  _requireShape=false;
}

//=============================================================================
GHS3DPRLPlugin_GHS3DPRL::~GHS3DPRLPlugin_GHS3DPRL()
{
  MESSAGE("GHS3DPRLPlugin_GHS3DPRL::~GHS3DPRLPlugin_GHS3DPRL");
}

//=============================================================================
bool GHS3DPRLPlugin_GHS3DPRL::CheckHypothesis
                         (SMESH_Mesh& aMesh,
                          const TopoDS_Shape& aShape,
                          SMESH_Hypothesis::Hypothesis_Status& aStatus)
{
  //MESSAGE("GHS3DPRLPlugin_GHS3DPRL::CheckHypothesis");
  _hypothesis = NULL;

  list<const SMESHDS_Hypothesis*>::const_iterator itl;
  const SMESHDS_Hypothesis* theHyp;

  const list<const SMESHDS_Hypothesis*>& hyps = GetUsedHypothesis(aMesh, aShape);
  int nbHyp = hyps.size();
  if (!nbHyp)
  {
    aStatus = SMESH_Hypothesis::HYP_MISSING;
    return false;  // can't work with no hypothesis
  }

  itl = hyps.begin();
  theHyp = (*itl); // use only the first hypothesis

  string hypName = theHyp->GetName();
  if (hypName == GHS3DPRLPlugin_Hypothesis::GetHypType())
  {
    _hypothesis = static_cast<const GHS3DPRLPlugin_Hypothesis*> (theHyp);
    ASSERT(_hypothesis);
    aStatus = SMESH_Hypothesis::HYP_OK;
  }
  else
    aStatus = SMESH_Hypothesis::HYP_INCOMPATIBLE;

  return aStatus == SMESH_Hypothesis::HYP_OK;
}

//=============================================================================
// Pass parameters to GHS3DPRL
void GHS3DPRLPlugin_GHS3DPRL::SetParameters(const GHS3DPRLPlugin_Hypothesis* hyp)
{
  if (hyp) {
    MESSAGE("GHS3DPRLPlugin_GHS3DPRL::SetParameters");
    _MEDName     = hyp->GetMEDName();  //"DOMAIN\0"
    _NbPart      = hyp->GetNbPart();
    _KeepFiles   = hyp->GetKeepFiles();
    _Background  = hyp->GetBackground();
    _Multithread = hyp->GetMultithread();
    _Gradation   = hyp->GetGradation();
    _MinSize     = hyp->GetMinSize();
    _MaxSize     = hyp->GetMaxSize();
    _AdvOptions  = hyp->GetAdvancedOption();
  }
}

//=======================================================================
//before launching salome
//SALOME_TMP_DIR (for keep tepal intermediates files) could be set in user's directories
static TCollection_AsciiString getTmpDir()
{
  TCollection_AsciiString aTmpDir;
  char *Tmp_dir = getenv("SALOME_TMP_DIR");
  if (Tmp_dir == NULL) Tmp_dir = getenv("TMP");
  if(Tmp_dir != NULL)
  {
    aTmpDir = Tmp_dir;
#ifdef WIN32
      if(aTmpDir.Value(aTmpDir.Length()) != '\\') aTmpDir+='\\';
#else
      if(aTmpDir.Value(aTmpDir.Length()) != '/') aTmpDir+='/';
#endif
  }
  else
  {
#ifdef WIN32
      aTmpDir = TCollection_AsciiString("C:\\");
#else
      aTmpDir = TCollection_AsciiString("/tmp/");
#endif
  }
  return aTmpDir;
}

//=============================================================================
// Write a skin mesh into a GMF file or pass it to MG-TetraHPC API
static void exportGMF(MG_TetraHPC_API*    theTetraInput,
                      const char*         theFile,
                      const SMESHDS_Mesh* theMeshDS)
{
  int meshID = theTetraInput->GmfOpenMesh( theFile, GmfWrite, GMFVERSION, GMFDIMENSION);
  
  // nodes
  int iN = 0, nbNodes = theMeshDS->NbNodes();
  theTetraInput->GmfSetKwd( meshID, GmfVertices, nbNodes );
  std::map< const SMDS_MeshNode*, int, TIDCompare > node2IdMap;
  SMDS_NodeIteratorPtr nodeIt = theMeshDS->nodesIterator();
  SMESH_TNodeXYZ n;
  while ( nodeIt->more() )
  {
    n.Set( nodeIt->next() );
    theTetraInput->GmfSetLin( meshID, GmfVertices, n.X(), n.Y(), n.Z(), n._node->getshapeId() );
    node2IdMap.insert( node2IdMap.end(), std::make_pair( n._node, ++iN ));
  }

  // triangles
  SMDS_ElemIteratorPtr elemIt = theMeshDS->elementGeomIterator( SMDSGeom_TRIANGLE );
  if ( elemIt->more() )
  {
    int nbTria = theMeshDS->GetMeshInfo().NbElements( SMDSGeom_TRIANGLE );
    theTetraInput->GmfSetKwd(meshID, GmfTriangles, nbTria );
    for ( int gmfID = 1; elemIt->more(); ++gmfID )
    {
      const SMDS_MeshElement* tria = elemIt->next();
      theTetraInput->GmfSetLin(meshID, GmfTriangles, 
                              node2IdMap[ tria->GetNode( 0 )],
                              node2IdMap[ tria->GetNode( 1 )],
                              node2IdMap[ tria->GetNode( 2 )],
                              tria->getshapeId() );
    }
  }
  theTetraInput->GmfCloseMesh( meshID );
}

//=============================================================================
// Here we are going to use the GHS3DPRL mesher for tetra-hpc (formerly tepal in v3 (2014))
bool GHS3DPRLPlugin_GHS3DPRL::Compute(SMESH_Mesh&         theMesh,
                                      const TopoDS_Shape& theShape)
{
  SMESH_MesherHelper helper( theMesh );
  bool ok = Compute( theMesh, &helper );
  return ok;
}

//=============================================================================
// Here we are going to use the GHS3DPRL mesher for tetra-hpc (formerly tepal in v3 (2014))
bool GHS3DPRLPlugin_GHS3DPRL::Compute(SMESH_Mesh&         theMesh,
                                      SMESH_MesherHelper* theHelper)
{
  bool Ok=false;
  TCollection_AsciiString pluginerror("ghs3dprl: ");
  SMESHDS_Mesh* meshDS = theMesh.GetMeshDS();
  if ( theMesh.NbTriangles() == 0 )
    return error( COMPERR_BAD_INPUT_MESH, "No triangles in the mesh" );

  if (_hypothesis==NULL){
    pluginerror += "No existing parameters/hypothesis for GHS3DPRL";
    cout <<"\n"<<pluginerror<<"\n\n";
    error(COMPERR_ALGO_FAILED, pluginerror.ToCString() );
    return false;
  }
  SetParameters(_hypothesis);
  cout << "\n" << _name << " parameters:" << endl;
  cout << "   generic path/name of MED Files = " << _MEDName << endl;
  cout << "   number of partitions = " << _NbPart << endl;
  cout << "   gradation = " << _Gradation << endl;
  cout << "   min_size = " << _MinSize << endl;
  cout << "   max_size = " << _MaxSize << endl;
  cout << "   keep intermediates files (from tetra-hpc) = " << _KeepFiles << endl;
  cout << "   background (from tetra-hpc) = " << _Background << "\n";
  cout << "   multithread = " << _Multithread << "\n\n";
  cout << "   adv. options = '" << _AdvOptions << "'\n\n";

  TCollection_AsciiString
    tmpDir=getTmpDir(),
    GHS3DPRL_In,
    GHS3DPRL_Out,
    GHS3DPRL_Out_Mesh,
    GHS3DPRL_Outxml,
    logFileName,
    run_GHS3DPRL("tetrahpc2med "),
    rm("rm -f "),
    run_nokeep_files,
    NbPart,
    Gradation,
    MinSize,
    MaxSize,
    fileskinmed(""),
    fileskinmesh(""),
    path,
    casenamemed;  //_MEDName.c_str());

  casenamemed += (char *)_MEDName.c_str();
  int n=casenamemed.SearchFromEnd('/');
  if (n>0) {
    path=casenamemed.SubString(1,n);
    casenamemed=casenamemed.SubString(n+1,casenamemed.Length());
  }
  else
    path=tmpDir;

  if (casenamemed.Length()>20){
    casenamemed=casenamemed.SubString(1,20);
    cerr<<"MEDName truncated (no more 20 characters) = "<<casenamemed<<endl;
  }

  map <int,int> aSmdsToGHS3DPRLIdMap;
  map <int,const SMDS_MeshNode*> aGHS3DPRLIdToNodeMap;
  GHS3DPRL_In = path + "GHS3DPRL";
  GHS3DPRL_Out = path + casenamemed;
  GHS3DPRL_Out_Mesh = path + casenamemed + "_out.mesh";
  GHS3DPRL_Outxml = path + casenamemed + ".xml"; //master file
  logFileName = path + casenamemed + ".log"; // MG library output
  NbPart=_NbPart;
  Gradation=_Gradation;
  MinSize=_MinSize;
  MaxSize=_MaxSize;

  //an example:
  //tetrahpc2med --casename=/home/whoami/tmp/GHS3DPRL --number=5 --medname=DOMAIN
  //             --gradation=1.05 --min_size=1e-3 --max_size=1e-2
  //             --verbose=0 --menu=no --launchtetra=yes;

  run_GHS3DPRL = run_GHS3DPRL +
    " --casename=" + GHS3DPRL_In +
    " --number=" + NbPart +
    " --medname=" + GHS3DPRL_Out +
    " --launchtetra=yes" +
    " --gradation=" + Gradation +
    " --min_size=" + MinSize +
    " --max_size=" + MaxSize +
    " --verbose=3" +
    " " + _AdvOptions.c_str();
  if (_Background) run_GHS3DPRL += " --background=yes"; else run_GHS3DPRL += " --background=no";
  if (_Multithread) run_GHS3DPRL += " --multithread=yes"; else run_GHS3DPRL += " --multithread=no";
  run_nokeep_files = rm +GHS3DPRL_In + "* " + path + "tetrahpc.log";
  system( run_nokeep_files.ToCString() ); //clean files
  run_nokeep_files = rm + GHS3DPRL_In + "* ";

  fileskinmesh=path + "GHS3DPRL.mesh";
  GHS3DPRL_Out = path + casenamemed;
  removeFile( GHS3DPRL_Outxml ); //only the master xml file

  MG_TetraHPC_API mgTetraHPC( _computeCanceled, _progress );
  bool useLib = ( mgTetraHPC.IsLibrary() && !_Background && _Multithread );
  if ( !useLib )
    mgTetraHPC.SetUseExecutable();

  exportGMF( &mgTetraHPC, fileskinmesh.ToCString(), meshDS );

  if ( useLib )
  {
    TCollection_AsciiString cmd = TCollection_AsciiString("mg-tetra_hpc.exe") + 
      " --number_of_subdomains=" + NbPart +
      " --gradation=" + Gradation +
      " --min_size=" + MinSize +
      " --max_size=" + MaxSize +
      " --verbose=3" +
      " --out=" + GHS3DPRL_Out_Mesh +
      " " + _AdvOptions.c_str();

    cout << endl
         << "  Run mg-tetra_hpc as library. Creating a mesh file " << GHS3DPRL_Out_Mesh << endl
         << "  Creating a log file : " << logFileName << endl << endl;
    mgTetraHPC.SetLogFile( logFileName.ToCString() );

    mgTetraHPC.Compute( cmd.ToCString() );

    std::string log = mgTetraHPC.GetLog();
    if ( log.find(" Dlim "   ) != std::string::npos ||
         log.find(" license ") != std::string::npos )
      return error("License problem");

    // set --launchtetra=no
    int yesPos = run_GHS3DPRL.Search("launchtetra") + sizeof("launchtetra");
    run_GHS3DPRL.SetValue( yesPos+0, 'n' );
    run_GHS3DPRL.SetValue( yesPos+1, 'o' );
    run_GHS3DPRL.SetValue( yesPos+2, ' ' );
  }
  else
  {
    cout<<"  Write input file for mg-tetra_hpc "<<fileskinmesh<<"...";
    cout<<" ...done\n";
  }
  fileskinmed=path + "GHS3DPRL_skin.med";
  cout<<"  Write file "<<fileskinmed<<"...";
  theMesh.ExportMED(fileskinmed.ToCString(),"SKIN_INITIAL",true,1);
  cout<<" ...done\n\n";

  cout<<"GHS3DPRL command :\n  "<<run_GHS3DPRL.ToCString()<<endl;
  //sometimes it is better to wait flushing files on slow filesystem...
  system( "sleep 3" );
  //launch tetrahpc2med which launch mg-tetra_hpc.py which launch mg-tetra_hpc(_mpi?).exe
  system( run_GHS3DPRL.ToCString() );
  system( "sleep 3" );

  if (_Background) {
    pluginerror = pluginerror + "backgrounding... plugin is not waiting for output files "+ casenamemed + "_*.med";
    cout<<pluginerror<<endl;
    error(COMPERR_ALGO_FAILED, pluginerror.ToCString());
    return false; //but it is not a problem but if true my message is overwritten
    //return true; //but it is not a problem,
  }

  // read a result, GHS3DPRL_Out is the name of master file (previous xml format)
  FILE * aResultFile = fopen( GHS3DPRL_Outxml.ToCString(), "r" );
  if (aResultFile){
    Ok = false; //but it is not a problem but if true my message is overwritten
    fclose(aResultFile);
    cout<<"GHS3DPRL OK output master file "<<casenamemed<<".xml exist !\n\n";
    pluginerror = pluginerror + "new tetraedra not in memory, but stored in files "+ casenamemed + "_*.med";
    cout<<pluginerror<<endl;
    error(COMPERR_ALGO_FAILED, pluginerror.ToCString());
    if (!_KeepFiles) system( run_nokeep_files.ToCString() );
  }
  else{
    Ok = false; //it is a problem AND my message is NOT overwritten
    pluginerror = pluginerror + "output master file " + casenamemed + ".xml do not exist";
    cout<<pluginerror<<endl;
    error(COMPERR_ALGO_FAILED, pluginerror.ToCString() );
    cout<<"GHS3DPRL KO output files "<<GHS3DPRL_Out<<" do not exist ! see intermediates files keeped:\n";
    TCollection_AsciiString run_list_files("ls -alt ");
    run_list_files +=  GHS3DPRL_Out + "* " + GHS3DPRL_In + "* " + path + "tetrahpc.log";
    system( run_list_files.ToCString() );
    cout<<endl;
  }

  return Ok;
}



//=============================================================================
/*!
 *
 */
//=============================================================================
bool GHS3DPRLPlugin_GHS3DPRL::Evaluate(SMESH_Mesh& aMesh,
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
    const TopoDS_Edge& E = TopoDS::Edge(exp.Current());
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
  int nbVols = (int)aVolume/tetrVol/CoeffQuality;
  int nb1d_f = (nbtri*3 + nbqua*4 - nb1d_e) / 2;
  int nb1d_in = (int) ( nbVols*6 - nb1d_e - nb1d_f ) / 5;
  std::vector<int> aVec(SMDSEntity_Last);
  for(int i=0; i<SMDSEntity_Last; i++) aVec[i]=0;
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
