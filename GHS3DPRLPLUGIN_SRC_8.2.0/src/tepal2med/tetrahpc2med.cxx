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
// File   : tetrahpc2med.cxx
// Author : Christian VAN WAMBEKE (CEA) 
// ---
//
/*
** prog principal de ghs3dprl
*/

#include <stdio.h> /* printf clrscr fopen fread fwrite fclose */
#include <string>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#ifndef WIN32
#include <unistd.h>
#endif

#include <qstring.h>

#include <QXmlSimpleReader>
#include <QXmlInputSource>
#include <QApplication>

#include "ghs3dprl_msg_parser.h"
//#include "dlg_ghs3dmain.h"

#ifdef WIN32
#include <io.h>
#include <windows.h>
#define F_OK 0
#endif

//#include "MEDMEM_Exception.hxx"
//#include "MEDMEM_define.hxx"

#include <med.h>
//#include <med_config.h>
//#include <med_utils.h>
//#include <med_misc.h>

//************************************
med_idt ouvre_fichier_MED(char *fichier,int verbose)
{
  med_idt fid = 0;
  med_err ret = 0;
  med_int majeur,mineur,release;

  /* on regarde si le fichier existe */
  ret = (int) access(fichier,F_OK);
  if (ret < 0) return fid;

  /* on regarde s'il s'agit d'un fichier au format HDF5 */
  med_bool hdfok,medok;
  ret = MEDfileCompatibility(fichier,&hdfok,&medok);
  if (ret < 0){
     std::cerr<<"File "<<fichier<<" not MED or HDF V5 formatted\n";
     return fid;
  }

  /* Quelle version de MED est utilise par mdump ? */
  MEDlibraryNumVersion(&majeur,&mineur,&release);
  if (verbose>0)fprintf(stdout,"\nReading %s with MED V%d.%d.%d",
                        fichier,majeur,mineur,release);

  /* Ouverture du fichier MED en lecture seule */
  fid = MEDfileOpen(fichier,MED_ACC_RDONLY);
  if (ret < 0) return fid;

  MEDfileNumVersionRd(fid, &majeur, &mineur, &release);
  if (( majeur < 2 ) || ( majeur == 2 && mineur < 2 )) {
    fprintf(stderr,"File %s from MED V%d.%d.%d not assumed\n",
                   fichier,majeur,mineur,release);
    //" version est ant�ieure �la version 2.2";
    ret = MEDfileClose(fid);
    fid=0; }
  else {
    if (verbose>0)fprintf(stdout,", file from MED V%d.%d.%d\n",majeur,mineur,release); }

  return fid;
}

//************************************
bool ReadFileMED(QString nomfilemed,ghs3dprl_mesh_wrap *mymailw)
{
   med_err ret;
   med_idt fid=0;
   med_int i,j,sdim,mdim,nmaa,/*edim,majeur_lu,mineur_lu,release_lu,nprofils,*/nstep;
   med_mesh_type type_maillage;
   char dtunit[MED_SNAME_SIZE+1];
   char axisname[MED_SNAME_SIZE+1];
   char axisunit[MED_SNAME_SIZE*3+1];
   med_sorting_type sortingtype;
   med_axis_type axistype;
   int numero=1;
   QString key,tmp;
   med_bool chan;
   med_bool tran;
   
   //version qt3
   char* chaine = (char*)malloc((nomfilemed.length()+1)*sizeof(char));
   strncpy(chaine,nomfilemed.toLatin1().constData(),nomfilemed.length()+1);
   //std::cout<<"*** ReadFileMED *** "<<chaine<<"\n";

   fid=ouvre_fichier_MED(chaine,mymailw->verbose);
   free(chaine);
   if (fid == 0) {
      std::cerr<<"Problem opening file "<<nomfilemed.toLatin1().constData()<<"\n";
      return false;
   }

   nmaa = MEDnMesh(fid);
   if (nmaa <= 0){
      std::cerr<<"No meshes in "<<nomfilemed.toLatin1().constData()<<"\n";
      ret = MEDfileClose(fid);
      return false;
   }
   if (nmaa > 1) std::cout<<"More than one mesh in "<<nomfilemed.toLatin1().constData()<<", first one taken\n";
   ret = MEDmeshInfo(fid,numero,mymailw->nommaa,&sdim,&mdim,&type_maillage,mymailw->maillage_description,
                        dtunit,&sortingtype,&nstep,&axistype,axisname,axisunit);
   if (ret < 0){
      std::cerr<<"Problem MEDmeshInfo in "<<nomfilemed.toLatin1().constData()<<"\n";
      ret = MEDfileClose(fid);
      return false;
   }
   //changed with version med: a triangles mesh in 3d is dim 2 now and 3 before 2014
   if (mdim != 2 && mdim != 3){
      std::cerr<<"Problem mesh dimension should be 2 or 3: "<<mdim<<"\n";
      ret = MEDfileClose(fid);
      return false;
   }
   if (sdim != 3){
      std::cerr<<"Problem space dimension should be 3: "<<sdim<<"\n";
      ret = MEDfileClose(fid);
      return false;
   }
   if (type_maillage != MED_UNSTRUCTURED_MESH){
      std::cerr<<"Problem type mesh should be MED_NON_STRUCTURE: "<<type_maillage<<std::endl;
      ret = MEDfileClose(fid);
      return false;
   }

   //lecture nb de noeuds
   //cf med-3.0.0_install/share/doc/html/maillage_utilisateur.html
   med_int nnoe=MEDmeshnEntity(fid,mymailw->nommaa,MED_NO_DT,MED_NO_IT,
      MED_NODE,MED_NO_GEOTYPE,MED_COORDINATE,MED_NO_CMODE,&chan,&tran);
              //(med_geometrie_element)0,(med_connectivite)0);
   if (nnoe<1){
      std::cerr<<"Problem number of Vertices < 1\n";
      ret = MEDfileClose(fid);
      return false;
   }

   //nombre d'objets MED : mailles, faces, aretes , ... 
   med_int /*nmailles[MED_N_CELL_GEO],*/nbtria3;
   //med_int nfaces[MED_N_FACE_GEO];
   med_int /*naretes[MED_N_EDGE_FIXED_GEO],*/nbseg2;
   //med_int nmailles[MED_NBR_GEOMETRIE_MAILLE],nbtria3;
   //med_int nfaces[MED_NBR_GEOMETRIE_FACE];
   //med_int naretes[MED_NBR_GEOMETRIE_ARETE],nbseg2;
   //polygones et polyedres familles equivalences joints
   med_int /*nmpolygones,npolyedres,nfpolygones,*/nfam/*,nequ,njnt*/;

   //Combien de mailles, faces ou aretes pour chaque type geometrique ?
   /*for (i=0;i<MED_NBR_GEOMETRIE_MAILLE;i++){
      nmailles[i]=MEDnEntMaa(fid,mymailw->nommaa,MED_CONN,MED_MAILLE,typmai[i],typ_con);
      //lecture_nombre_mailles_standards(fid,nommaa,typmai[i],typ_con,i);
      if (mymailw->verbose>6) std::cout<<"NumberOf"<<nommai[i]<<"="<<nmailles[i]<<std::endl;
   }*/
   
   nbtria3=MEDmeshnEntity(fid,mymailw->nommaa,MED_NO_DT,MED_NO_IT,
      MED_CELL,MED_TRIA3,MED_CONNECTIVITY,MED_NODAL,&chan,&tran);
   nbseg2=MEDmeshnEntity(fid,mymailw->nommaa,MED_NO_DT,MED_NO_IT,
      MED_CELL,MED_SEG2,MED_CONNECTIVITY,MED_NODAL,&chan,&tran);

   //combien de familles ?
   nfam=MEDnFamily(fid,mymailw->nommaa);
   if (mymailw->verbose>2) {
      std::cout<<"\nNumberOfFamilies="<<nfam<<std::endl;
      std::cout<<"NumberOfVertices="<<nnoe<<std::endl;
      std::cout<<"NumberOfMED_SEG2="<<nbseg2<<std::endl;
      std::cout<<"NumberOfMED_TRIA3="<<nbtria3<<"\n\n";
   }
   if (nbtria3<3){
      std::cerr<<"Problem number of MED_TRIA3 < 3, not a skin of a volume\n";
      ret = MEDfileClose(fid);
      return false;
   }

med_int ifamdelete=0,idelete;
std::vector<med_int> famdelete = std::vector<med_int>(nfam);
{
  med_int ngro;
  char *gro;
  char nomfam[MED_NAME_SIZE+1];
  med_int numfam;
  //char str1[MED_COMMENT_SIZE+1];
  char str2[MED_LNAME_SIZE+1];
  med_err ret = 0;
  
  for (i=0;i<nfam;i++) famdelete[i]=0;
  for (i=0;i<nfam;i++) {

    //nombre de groupes
    ngro = MEDnFamilyGroup(fid,mymailw->nommaa,i+1);
    if (ngro < 0){
       std::cerr<<"Problem reading number of groups of family\n";
       continue;
    }

    //atributs obsolete MED3
    //allocation memoire par exces
    gro = (char*) malloc(MED_LNAME_SIZE*(ngro+1));
    
    ret = MEDfamilyInfo(fid,mymailw->nommaa,i+1,nomfam,&numfam,gro);
    if (ret < 0){
       std::cerr<<"Problem reading informations of family\n";
       continue;
    }

    if (mymailw->verbose>8) {
     std::cout<<"Family "<<numfam<<" have "<<ngro<<" groups\n";
     //affichage des resultats
     for (j=0;j<ngro;j++) {
      if (j==0) std::cout<<"  Groups :\n";
      strncpy(str2,gro+j*MED_LNAME_SIZE,MED_LNAME_SIZE);
      str2[MED_LNAME_SIZE] = '\0';
      fprintf(stdout,"    name = %s\n",str2);
     }
     if (i==nfam-1) std::cout<<std::endl;
    }
    QString sfam,sgro;
    sfam=sfam.sprintf("%d",numfam);
    idelete=0;
    for (j=0;j<ngro;j++){
       strncpy(str2,gro+j*MED_LNAME_SIZE,MED_LNAME_SIZE);
       str2[MED_LNAME_SIZE]='\0';
       sgro=str2;
       if (sgro.contains(mymailw->deletegroups)>0) {
          //std::cout<<"idelete++ "<<sgro<<std::endl;
          idelete++;
       }
    }

    if (idelete==ngro && ngro>0) { //only delete family whith all delete groups
       //std::cout<<"famdelete++ "<<numfam<<" "<<ifamdelete<<" "<<ngro<<std::endl;
       famdelete[ifamdelete]=numfam;
       ifamdelete++;
    }

    else {
     for (j=0;j<ngro;j++){
       strncpy(str2,gro+j*MED_LNAME_SIZE,MED_LNAME_SIZE);
       str2[MED_LNAME_SIZE]='\0';
       sgro=str2;
       QRegExp qgroup=QRegExp("Group_Of_All",Qt::CaseSensitive,QRegExp::RegExp);
       if (sgro.contains(mymailw->deletegroups)==0){
          if (sgro.contains(qgroup)>0) {
             sgro="Skin_"+sgro; //pas sur que ce soit pertinent
          }
          if (mymailw->verbose>8) std::cout<<"families.add("<<sfam.toLatin1().constData()<<
                                        ","<<sgro.toLatin1().constData()<<")\n";
          mymailw->families.add(sfam,sgro);
       }
       else {
          //sgro="Skin_"+sgro; //pas sur que ce soit pertinent
          //std::cout<<"--deletegroups matches \""<<sfam<<","<<sgro<<"\"\n";
          if (mymailw->verbose>3) std::cout<<"--deletegroups matches \""<<
                                        sgro.toLatin1().constData()<<
                                        "\" in family "<<numfam<<std::endl;
       }
     }
    }
    
    /*for (j=0;j<ngro;j++){
       strncpy(str2,gro+j*MED_LNAME_SIZE,MED_LNAME_SIZE);
       str2[MED_LNAME_SIZE]='\0';
       sgro=str2;
       //std::cout<<"families.add("<<sfam<<","<<sgro<<")\n";
       if (sgro.contains(mymailw->deletegroups)==0){
          //sgro="Skin_"+sgro; //pas sur que ce soit pertinent
          std::cout<<"families.add("<<sfam<<","<<sgro<<")\n";
          mymailw->families.add(sfam,sgro);
       }
       else {
          std::cout<<"--deletegroups matches \""<<sgro<<"\"\n";
          famdelete[ifamdelete]=numfam
          ifamdelete++;
       }
    }*/

    //on libere la memoire
    free(gro);
  }
}

//std::cout<<"famdelete"; for (j=0;j<ifamdelete;j++) std::cout<<" "<<famdelete[j]; std::cout<<std::endl;

if (mymailw->verbose>3){
   std::cout<<"\nFamiliesAndGroupsOf "<<nomfilemed.toLatin1().constData()<<std::endl;
   mymailw->families.write();
}
   /* Allocations memoires */
   /* table des coordonnees profil : (space dimension * nombre de noeuds ) */
   med_float *coo=new med_float[nnoe*sdim];
   /* table des numeros de familles des noeuds profil : (nombre de noeuds) */
   med_int *famnodesskin=new med_int[nnoe];
   //med_int *pfltab=new med_int[1]; //inutilise car on lit tout 
   //lecture des noeuds : coordonnees
   ret=MEDmeshNodeCoordinateRd(fid,mymailw->nommaa,MED_NO_DT,MED_NO_IT,MED_FULL_INTERLACE,coo);
          //mdim,coo,mode_coo,MED_ALL,pfltab,0,&rep,mymailw->nomcoo,mymailw->unicoo);
   if (ret < 0){
      std::cerr<<"Problem reading nodes\n";
      ret = MEDfileClose(fid);
      //return false;
   }
   ret=MEDmeshEntityFamilyNumberRd(fid,mymailw->nommaa,MED_NO_DT,MED_NO_IT,MED_NODE,MED_NONE,famnodesskin);
      //famnodesskin,nnoe,MED_NOEUD,(med_geometrie_element) 0);
   if (ret < 0){
      std::cerr<<"Problem reading families of nodes\n";
      ret = MEDfileClose(fid);
      return false;
   }
   if (mymailw->verbose>9) {
     std::cout<<"\nVertices: no x y z family\n";
     for (i=0;i<nnoe*mdim;i=i+3) {
      fprintf(stdout,"%5d %13.5e %13.5e %13.5e %5d \n",
          (i/3+1), coo[i], coo[i+1], coo[i+2], famnodesskin[i/3]);
     } 
     std::cout<<std::endl;
   }

   med_int *conn2=new med_int[nbseg2*2];
   ret=MEDmeshElementConnectivityRd(fid,mymailw->nommaa,MED_NO_DT,MED_NO_IT,
           MED_CELL,MED_SEG2,MED_NODAL,MED_FULL_INTERLACE,conn2);
            //mdim,conn2,mode_coo,pfltab,0,MED_MAILLE,MED_SEG2,MED_NOD);
   if (ret < 0){
      std::cerr<<"Problem reading MED_SEG2\n";
      ret = MEDfileClose(fid);
      //return false;
   }
   med_int *famseg2skin=new med_int[nbseg2];
   ret=MEDmeshEntityFamilyNumberRd(fid,mymailw->nommaa,MED_NO_DT,MED_NO_IT,MED_CELL,MED_SEG2,famseg2skin);
      //MEDfamLire(fid,mymailw->nommaa,famseg2skin,nbseg2,MED_MAILLE,MED_SEG2);
   if (ret < 0){
      std::cerr<<"Problem reading families of MED_SEG2\n";
      ret = MEDfileClose(fid);
      return false;
   }
   if (mymailw->verbose>9) {
     std::cout<<"\nConnectivity MED_SEG2: no node1 node2 family\n";
     for (i=0;i<nbseg2*2;i=i+2) {
      fprintf(stdout,"%5d %5d %5d %5d \n",
          (i/2+1), conn2[i], conn2[i+1], famseg2skin[i/2]);
     } 
     std::cout<<std::endl;
   }
   //std::cout<<"\ncvw1 conn nbtria3 "<<nbtria3<<"dt "<<MED_NO_DT<<"it "<<MED_NO_IT<<"cell "<<MED_CELL<<"tria3 "<<MED_TRIA3<<std::endl;
   med_int *conn3=new med_int[nbtria3*3];
   ret=MEDmeshElementConnectivityRd(fid,mymailw->nommaa,MED_NO_DT,MED_NO_IT,
           MED_CELL,MED_TRIA3,MED_NODAL,MED_FULL_INTERLACE,conn3);
           //MEDconnLire(fid,mymailw->nommaa,mdim,conn3,mode_coo,pfltab,0,MED_MAILLE,MED_TRIA3,MED_NOD);
   if (ret < 0){
      std::cerr<<"Problem reading MED_TRIA3\n";
      ret = MEDfileClose(fid);
      //return false;
   }
   med_int *famtria3skin=new med_int[nbtria3];
   ret=MEDmeshEntityFamilyNumberRd(fid,mymailw->nommaa,MED_NO_DT,MED_NO_IT,MED_CELL,MED_TRIA3,famtria3skin);
        //MEDfamLire(fid,mymailw->nommaa,famtria3skin,nbtria3,MED_MAILLE,MED_TRIA3);
   if (ret < 0){
      std::cerr<<"Problem reading families of MED_TRIA3\n";
      ret = MEDfileClose(fid);
      return false;
   }
   if (mymailw->verbose>9) {
     std::cout<<"\nConnectivity MED_TRIA3: no node1 node2 node3 family\n";
     for (i=0;i<nbtria3*3;i=i+3) {
      fprintf(stdout,"%5d %5d %5d %5d %5d \n",
          (i/3+1), conn3[i], conn3[i+1], conn3[i+2], famtria3skin[i/3]);
     } 
     std::cout<<std::endl;
   }

  /*liberation memoire?
  delete[] coo;
  delete[] nomnoe;
  delete[] numnoe;
  delete[] nufano;*/

  if (ifamdelete>0) {
   //std::cout<<"!!!!!!!!nodes "<<famnodesskin[0]<<" "<<nnoe<<famdelete[1]<<std::endl;
   for (i=0;i<nnoe;i++) {
    for (j=0;j<ifamdelete;j++) {
      if (famnodesskin[i]==famdelete[j]) {
       //std::cout<<"nodes "<<famnodesskin[i]<<" "<<i<<" "<<famdelete[j]<<std::endl;
       famnodesskin[i]=0; }
    }
   }
   for (i=0;i<nbseg2;i++) {
    for (j=0;j<ifamdelete;j++) {
      if (famseg2skin[i]==famdelete[j]) famseg2skin[i]=0;
    }
   }
   for (i=0;i<nbtria3;i++) {
    for (j=0;j<ifamdelete;j++) {
      if (famtria3skin[i]==famdelete[j]) famtria3skin[i]=0;
    }
   }
  }
   //stocks data for future use 
   CVWtab *montab;
   bool ok;

   montab=new CVWtab(nnoe*mdim,coo);
   tmp="SKIN_VERTICES_COORDINATES";
   ok=mymailw->insert_key(tmp,montab);

   montab=new CVWtab(nnoe,famnodesskin);
   tmp="SKIN_VERTICES_FAMILIES";
   ok=mymailw->insert_key(tmp,montab);
   
   montab=new CVWtab(nbseg2*2,conn2);
   tmp="SKIN_SEG2_CONNECTIVITIES";
   ok=mymailw->insert_key(tmp,montab);

   montab=new CVWtab(nbtria3,famseg2skin);
   tmp="SKIN_SEG2_FAMILIES";
   ok=mymailw->insert_key(tmp,montab);

   montab=new CVWtab(nbtria3*3,conn3);
   tmp="SKIN_TRIA3_CONNECTIVITIES";
   ok=mymailw->insert_key(tmp,montab);

   montab=new CVWtab(nbtria3,famtria3skin);
   tmp="SKIN_TRIA3_FAMILIES";
   ok=mymailw->insert_key(tmp,montab);

   //if (mymailw->verbose>6) ok=mymailw->list_keys_mesh_wrap();

   ret = MEDfileClose(fid);
   if (ret < 0){
      std::cerr<<"Problem closing "<<nomfilemed.toLatin1().constData()<<"\n";
      return false;
   }
   return true;
}

/*
mg-tetra_hpc_mpi.exe --help

    =================================================
    MG-Tetra_HPC -- MeshGems 2.1-11 (September, 2015)
    =================================================

        Distene SAS
           Campus Teratec
           2, rue de la Piquetterie
           91680 Bruyeres le Chatel
           FRANCE
        Phone: +33(0)970-650-219   Fax: +33(0)169-269-033
        EMail: <support@distene.com>

        Running MG-Tetra_HPC (Copyright 2014 by Distene SAS)
           date of run: 31-May-2016 AT 09:05:29
           running on : Linux 2.6.32-431.11.2.el6.centos.plus.x86_64 x86_64
           using modules: 
                MeshGems-Core 2.1-11

        MeshGems is a Registered Trademark of Distene SAS



Usage: tetra_hpc_mpi.exe [options]

Options: 

     Short option (if it exists)
    /    Long option
   |    /   Description
   |   |   /
   v   v  v

     --gradation <g>
          Sets the size gradation value.
          <gradation> is the desired maximum ratio between 2 adjacent
          tetrahedron edges. The closer it is to 1.0, the more uniform the mesh
          will be.
          Default: 1.05

     --help
          Prints this help.
          
     --in <input mesh file name>
          Sets the input file.
          (MANDATORY)

     --max_size <maximum size>
          Sets the desired maximum cell size value.
          Default: 0 (no maximum size).

     --min_size <minimum size>
          Default: 0 (no minimum size).
          Sets the desired minimum cell size value.

     --out <output mesh file name>
          Sets the output file.
          If unset, _tetra_hpc is appended to the input file basename.
          Using an existing file is forbidden.
          Using the same file as --in is forbidden.
          
     --verbose <verbose>
          Set the verbosity level, increasing from 0 to 10.
           <verbose> values are increasing from 0 to 10 :
             0 : no details
            10 : very detailed
          Default: 3


================================================================================
               MG-Tetra_HPC -- MeshGems 2.1-11 (September, 2015)
         END OF SESSION - MG-Tetra_HPC (Copyright 2014 by Distene SAS)
                   compiled Sep  3 2015 12:50:47 on Linux_64
               MeshGems is a Registered Trademark of Distene SAS
================================================================================
       ( Distene SAS
        Phone: +33(0)970-650-219   Fax: +33(0)169-269-033
        EMail: <support@distene.com> )
*/


//************************************
int main(int argc, char *argv[])
{
   bool ok;
   int i,nb,nbfiles,limit_swap,nbelem_limit_swap,limit_swap_defaut,verbose;
   float gradation,min_size,max_size;
   QString path,pathini,casename,casenamemed,fileskinmed,
           tmp,cmd,format,format_tetra,
           test,launchtetra,background,multithread,deletegroups,
           version="V4.0 (MED3 + tetra-hpc v2.3 Sept 2016)";
   
   char *chelp=NULL,
        *ccasename=NULL,
        *cnumber=NULL,
        *cmedname=NULL,
        *climitswap=NULL,
        *cverbose=NULL,
        *claunchtetra=NULL,
        *cgradation=NULL,
        *cmin_size=NULL,
        *cmax_size=NULL,
        *cbackground=NULL,
        *cmultithread=NULL,
        *cdeletegroups=NULL;

   for (i = 0; i < argc; i++){
      if (!strncmp (argv[i], "--help", sizeof ("--help"))) chelp = &(argv[i][0]);
      else if (!strncmp (argv[i], "--casename=", sizeof ("--casename"))) ccasename = &(argv[i][sizeof ("--casename")]);
      else if (!strncmp (argv[i], "--number=", sizeof ("--number"))) cnumber = &(argv[i][sizeof ("--number")]);
      else if (!strncmp (argv[i], "--limitswap=", sizeof ("--limitswap"))) climitswap = &(argv[i][sizeof ("--limitswap")]);
      else if (!strncmp (argv[i], "--medname=", sizeof ("--medname"))) cmedname = &(argv[i][sizeof ("--medname")]);
      else if (!strncmp (argv[i], "--verbose=", sizeof ("--verbose"))) cverbose = &(argv[i][sizeof ("--verbose")]);
      else if (!strncmp (argv[i], "--launchtetra=", sizeof ("--launchtetra"))) claunchtetra = &(argv[i][sizeof ("--launchtetra")]);
      else if (!strncmp (argv[i], "--gradation=", sizeof ("--gradation"))) cgradation = &(argv[i][sizeof ("--gradation")]);
      else if (!strncmp (argv[i], "--min_size=", sizeof ("--min_size"))) cmin_size = &(argv[i][sizeof ("--min_size")]);
      else if (!strncmp (argv[i], "--max_size=", sizeof ("--max_size"))) cmax_size = &(argv[i][sizeof ("--max_size")]);
      else if (!strncmp (argv[i], "--background=", sizeof ("--background"))) cbackground = &(argv[i][sizeof ("--background")]);
      else if (!strncmp (argv[i], "--multithread=", sizeof ("--multithread"))) cmultithread = &(argv[i][sizeof ("--multithread")]);
      else if (!strncmp (argv[i], "--deletegroups=", sizeof ("--deletegroups"))) cdeletegroups = &(argv[i][sizeof ("--deletegroups")]);
      }

   if (argc < 2 || chelp){
      std::cout<<"tetrahpc2med "<<version.toLatin1().constData()<<" Available options:\n"
      "   --help               : produces this help message\n"<<
      "   --casename           : path and name of input tetrahpc2med files which are\n"<<
      "                           - output files of GHS3DPRL_Plugin .mesh\n"<<
      "                           - output file of GHS3DPRL_Plugin casename_skin.med (optional)\n"<<
      "                          with initial skin and its initial groups\n"<<
      "   --number             : number of partitions\n"<<
      "   --medname            : path and name of output MED files\n"<<
      "   --limitswap          : max size of working cpu memory (Mo) (before swapping on .temp files)\n"<<
      "   --verbose            : trace of execution (0->6)\n"<<
      "   --launchtetra        : launch mg_tetra_hpc on files casename.mesh and option number,\n"<<
      "                            else only use existing input/output files\n"<<
      "   --gradation          : the desired maximum ratio between 2 adjacent tetrahedron edges (Default 1.05). The closer it is to 1.0\n"<<
      "   --min_size           : the desired maximum cell size value (Default: 0 no maximum size)\n"<<
      "   --max_size           : the desired minimum cell size value (Default: 0 no minimum size)\n"<<
      "   --background         : force background mode from launch tetra-hpc and generation of final MED files (big meshes)\n"<<
      "   --multithread        : launch mg_tetra_hpc multithread version, else mpi version\n"<<
      "   --deletegroups       : regular expression (see QRegExp) which matches unwanted groups in final MED files\n"<<
      "                            (try --deletegroups=\"(\\bJOINT)\"\n"<<
      "                            (try --deletegroups=\"(\\bAll_Nodes|\\bAll_Faces)\"\n"<<
      "                            (try --deletegroups=\"((\\bAll_|\\bNew_)(N|F|T))\"\n";
      std::cout<<"example:\n   tetrahpc2med --casename=/tmp/GHS3DPRL --number=2 --medname=DOMAIN "<<
                 "--verbose=0 --launchtetra=no\n\n";
      return 1;  //no output files
   }
   
   if (!ccasename){
      std::cerr<<"--casename: a path/name is expected\n\n";
      return 1;
   }
   casename=ccasename;
   if (!cnumber){
      std::cerr<<"--number: an integer is expected\n\n";
      return 1;
   }
   tmp=cnumber;
   nbfiles=tmp.toLong(&ok,10);
   if (!ok){
      std::cerr<<"--number: an integer is expected\n\n";
      return 1;
   }
   if (nbfiles<=0){
      std::cerr<<"--number: a positive integer is expected\n\n";
      return 1;
   }
   if (nbfiles>2048){ //delirium in 2016
      std::cerr<<"--number: a positive integer <= 2048 is expected\n\n";
      return 1;
   }
   if (!cmedname) cmedname=ccasename;
   casenamemed=cmedname;

   limit_swap_defaut=1000; //1000Mo
   limit_swap=limit_swap_defaut;
   if (climitswap){
      tmp=climitswap;
      limit_swap=tmp.toLong(&ok,10);
      if (!ok){
         std::cerr<<"--limitswap: an integer is expected. try 1000\n\n";
         return 1;
      }
      if (limit_swap<1 || limit_swap>32000){
         std::cerr<<"--limitswap: [1->32000] expected. try 1000\n\n";
         return 1;
      }
   }
   
   verbose=1; //default
   if (cverbose){
      tmp=cverbose;
      verbose=tmp.toLong(&ok,10);
      if (!ok){
         std::cerr<<"--verbose: an integer is expected\n\n";
         return 1;
      }
      if (verbose<0){
         std::cerr<<"--verbose: a positive integer is expected\n\n";
         return 1;
      }
   }

   launchtetra="no"; //default
   if (claunchtetra){
      tmp=claunchtetra;
      if (tmp=="yes") launchtetra="yes";
   }

   gradation=1.05; //default
   if (cgradation){
      tmp=cgradation;
      gradation=tmp.toFloat(&ok);
      if (!ok){
         std::cerr<<"--gradation: a float { 0; ]1,3] } is expected\n\n";
         return 1;
      }
      if (gradation>3.){
         std::cerr<<"--gradation: a float <= 3. is expected\n\n";
         return 1;
      }
      if (gradation<0.){
         std::cerr<<"--gradation: a float >= 0. is expected\n\n";
         return 1;
      }
   }

   if (cmin_size){
      tmp=cmin_size;
      min_size=tmp.toFloat(&ok);
      if (!ok){
         std::cerr<<"--min_size: a float >= 0. is expected\n\n";
         return 1;
      }
      if (gradation<0.){
         std::cerr<<"--min_size: a float >= 0. is expected\n\n";
         return 1;
      }
   }

   if (cmax_size){
      tmp=cmax_size;
      max_size=tmp.toFloat(&ok);
      if (!ok){
         std::cerr<<"--max_size: a float >= 0. is expected\n\n";
         return 1;
      }
      if (gradation<0.){
         std::cerr<<"--max_size: a float >= 0. is expected\n\n";
         return 1;
      }
   }

   background="no"; //default
   if (cbackground){
      tmp=cbackground;
      if (tmp=="yes") background="yes";
   }

   multithread="no"; //default
   if (cmultithread){
      tmp=cmultithread;
      if (tmp=="yes") multithread="yes";
   }


   int n=casenamemed.count('/');
   if (n>0)
      path=casenamemed.section('/',-n-1,-2)+"/";
   else
      path="./";
      casenamemed=casenamemed.section('/',-1);
   if (casenamemed.length()>20){
      std::cerr<<"--medname truncated (no more 20 characters)"<<std::endl;
      casenamemed.truncate(20);
   }

   n=casename.count('/');
   if (n>0)
      pathini=casename.section('/',-n-1,-2)+"/";
   else
      pathini="./";
      casename=casename.section('/',-1);
   if (casename.length()>20){
      std::cerr<<"--casename truncated (no more 20 characters)"<<std::endl;
      casename.truncate(20);
   }

   //std::cout<<"CaseNameMed="<<casenamemed.toLatin1().constData()<<std::endl;
   //std::cout<<"PathMed="<<path.toLatin1().constData()<<std::endl;

   deletegroups="(\\bxyz)"; //default improbable name
   if (cdeletegroups){
      deletegroups=cdeletegroups;
   }
   
   //verbose=5;
   if (verbose>0)
   std::cout<<
           "tetrahpc2med "<<version.toLatin1().constData()<<" parameters:"<<
         "\n   --casename="<<pathini.toLatin1().constData()<<casename.toLatin1().constData()<<
         "\n   --number="<<nbfiles<<
         "\n   --medname="<<path.toLatin1().constData()<<casenamemed.toLatin1().constData()<<
         "\n   --verbose="<<verbose<<
         "\n   --launchtetra="<<launchtetra.toLatin1().constData()<<
         "\n   --gradation="<<gradation<<
         "\n   --min_size="<<min_size<<
         "\n   --max_size="<<max_size<<
         "\n   --background="<<background.toLatin1().constData()<<
         "\n   --multithread="<<multithread.toLatin1().constData()<<
         "\n   --deletegroups=\""<<deletegroups.toLatin1().constData()<<"\"\n";
         
   if (launchtetra=="yes"){
     
      //call tetra_hpc.py is pthon script which assumes mpirun or else if no multithread
      //after compilation openmpi or else acrobatic DISTENE_LICENCE change...
      
      cmd="mg-tetra_hpc.py --number=" + QString::number(nbfiles)+
          " --in=" + pathini+casename + ".mesh" +
          " --out=" + pathini+casename + "_out.mesh" +
          " --gradation=" + QString::number(gradation) +
          " --min_size=" + QString::number(min_size) +
          " --max_size=" + QString::number(max_size) +
          " --multithread=" + multithread.toLatin1().constData() +
          " > " + path + "tetrahpc.log";
      std::cout<<"\nlaunch tetra_hpc command:"<<
                 "\n   "<<cmd.toLatin1().constData()<<"\n"<<std::endl;
   }
   else if ( std::ifstream(( pathini+casename + "_out.mesh").toLatin1().constData() ).is_open() )
   {
     // mesh file exists (created by using MG as a library), copy it to *.000001.mesh
     QString copyCmd = ( QString("cp -f %1_out.mesh %2_out.000001.mesh")
                         .arg( pathini+casename ).arg( pathini+casename ));
     system( copyCmd.toLatin1().constData()); // run
   }
   
   //utile si appel par plugin ghs3dprl sur big meshes et tetra_hpc sur plusieurs jours
#ifndef WIN32
   if (background=="yes"){
      pid_t pid = fork();
      if (pid > 0) {
         //Process father
         exit(0); //temporary ok for plugin
      }
      //process children
      //On rend le fils independant de tout terminal
      //from here everything in background: tetrahpc AND generation of final MED files
      setsid();
      system("sleep 10");  //for debug
   }
#else
   printf("background mode is not supported on win32 platform !\n");
#endif

   if (launchtetra=="yes"){
      //sometimes it is better to wait flushing files on slow filesystem...
      system("sleep 3");
      system(cmd.toLatin1().constData()); // run
      system("sleep 3");
   }
   ghs3dprl_mesh_wrap *mymailw=new ghs3dprl_mesh_wrap;
   //no constructor, later maybe
   mymailw->nbfiles=0;
   mymailw->nbfilestot=nbfiles;
   //for huge cases big array swap in huge binary files
   mymailw->verbose=verbose;
   mymailw->casename=casename;
   mymailw->medname=casenamemed;
   mymailw->path=path;
   mymailw->pathini=pathini;
   mymailw->deletegroups=QRegExp(deletegroups,Qt::CaseSensitive,QRegExp::RegExp);
   mymailw->for_multithread=false;
   if (multithread=="yes") mymailw->for_multithread=true;
   ghs3dprl_msg_parser handler;
   //constructor later maybe
   //handler.verbose=true;
   handler.mailw=mymailw;
   mymailw->families.no=1;
   //std::cout<<"coucou1 "<<mymailw->families.no<<std::endl;
   //mymailw->families.add(casename,casenamemed);
   format=format.sprintf("%d",nbfiles);
   int nbf=format.length();
   format=format.sprintf(".%%0%dd.%%0%dd",nbf,nbf);
   format_tetra=".%06d";
   if (verbose>10)std::cout<<"format "<<format.toLatin1().constData()<<std::endl;
   if (verbose>10)std::cout<<"format_tetra "<<format_tetra.toLatin1().constData()<<std::endl;
   mymailw->format=format;
   mymailw->format_tetra=format_tetra;
   mymailw->for_tetrahpc=true; //to know what files to read: .noboite or .mesh
   
   //default 1GOctet/8(for float)
   nbelem_limit_swap=limit_swap*1000000; //100%
   CVWtab::memorymax=nbelem_limit_swap;
   mymailw->nbelem_limit_swap=nbelem_limit_swap;

   
   //something like "/home/wambeke/tmp/GHS3DPRL_skin.med"
   fileskinmed=pathini+casename+"_skin.med";
   //fileskinmed="/home/wambeke/tmp/GHS3DPRL_skin.med";
   /*for debug
   {
   char ctmp[fileskinmed.length()+1] ; strcpy(ctmp,fileskinmed);
   int res=dumpMED(&ctmp[0],1);
   }*/
   int ret = access(fileskinmed.toLatin1().constData(),F_OK); //on regarde si le fichier existe
   if (ret >= 0) {
      ok=ReadFileMED(fileskinmed,mymailw); }
   else {
      if (verbose>0)std::cout<<"Initial skin file <"<<fileskinmed.toLatin1().constData()<<"> does not exist\n"; }
   

//if test quickly read all files before (or only small files)
 if (test=="yes"){
   if (verbose>0) std::cout<<"\nReading output files of tetrahpc as input files of tetrahpc2med...\n";
   //only read beginning of files .xxxxx.mesh
   //supposed big files big arrays so only see first lines
   mymailw->nbfiles=0;
   for (int i=1; i<=nbfiles; i++){
      mymailw->nofile=i;
      tmp=pathini+casename+tmp.sprintf(format_tetra.toLatin1().constData(),i)+".mesh";
      if (verbose>0) std::cout<<"FileName="<<tmp.toLatin1().constData()<<std::endl;
      ok=mymailw->TestExistingFileMESHnew(tmp);
   }
   if (verbose>0)
      std::cout<<"NumberOfFilesMESHTested="<<mymailw->nbfiles<<": ok\n\n";
   if (mymailw->nbfiles != nbfiles){
      std::cerr<<"NumberOfFiles != NumberOfFilesTested is unexpected\n\n";
      return 1;
   }
 }  //end if test
 
   ok=mymailw->Write_MEDfiles_v2(true); //deletekeys=true
   
   nb=mymailw->remove_all_keys_mesh_wrap();
   if (verbose>3)std::cout<<"***remove_all_key_mesh_wrap*** "<<nb<<" keys removed\n";
   if (verbose>0)std::cout<<std::endl<<"===end of "<<argv[0]<<"==="<<std::endl;

   //for debug
   //int res=dumpMED("/home/wambeke/tmp/DOMAIN_1.med",1);

   return 0; //ok
}
