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
// File   : ghs3dprl_mesh_wrap.cxx
// Author : Christian VAN WAMBEKE (CEA) 
// ---
//
#include "ghs3dprl_mesh_wrap.h"

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#ifndef WIN32
#include <unistd.h>
#endif

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#ifdef WIN32
#include <io.h>
#include <windows.h>
#include <time.h>
#define F_OK 0
#undef max
#undef min
#endif

#include <QFile>
#include <QRegExp>
#include <limits>

#include <med.h>
//#include <med_config.h>
//#include <med_utils.h>
//#include <med_misc.h>

//utils procedures

//************************************
std::string i2a(const int &v)
{
   std::ostringstream ss;
   ss<<v;
   return ss.str();
}

//************************************
QString endspace(QString deb,int lg)
//better fill by spaces for char unicoo[3*MED_TAILLE_PNOM+1]; etc...
{
   QString fin,spa;
   //spa.fill(' ',lg);
   //fin=deb+spa;
   //fin.truncate(lg);
   fin=deb.leftJustified(lg,' ',true);
   return fin;
}

//************************************
void charendnull(char *p, QString deb, int lg)
{
   QString fin;
   fin=deb;
   fin.truncate(lg-1);
   strcpy(p,fin.toLatin1().constData()); // 0 at end
   for (int i=fin.length();i<lg-1;i++){
       p[i]='\0';
   }
}

//class familles
//************************************
   void familles::newfam(QString nom){
      //std::cout<<"newfam "<<nom<<std::endl;
      if (fam.find(nom)!=fam.end()){
         std::cout<<"***newfam*** "<<nom.toLatin1().constData()<<" deja present\n";
         return;
      }
      fend gb;
      fam[nom]=gb;
   }

//************************************
   void familles::newgro(QString nom){
      //std::cout<<"newgro "<<nom<<std::endl;
      if (gro.find(nom)!=gro.end()){
         std::cout<<"***newgro*** "<<nom.toLatin1().constData()<<" deja present\n";
         return;
      }
      fend gb;
      gro[nom]=gb;
   }
   
//************************************
   void familles::write(){
      fend gb;
      fagr::iterator it1;
      fend::iterator it2;
      int nbf=0,nbg=0;
      for (it1=fam.begin(); it1!=fam.end(); ++it1){
         nbf++;
         std::cout<<"Family=<"<<(*it1).first.toLatin1().constData()<<">\tGroups=";
         gb=(*it1).second;
         for (it2=gb.begin(); it2!=gb.end(); ++it2){
            std::cout<<"<"<<(*it2).first.toLatin1().constData()<<"> ";
         }
         std::cout<<std::endl;
      }
      if (nbf==0) std::cout<<"no families"<<std::endl;
      for (it1=gro.begin(); it1!=gro.end(); ++it1){
         nbg++;
         std::cout<<"Group=<"<<(*it1).first.toLatin1().constData()<<">\tFamilies=";
         gb=(*it1).second;
         for (it2=gb.begin(); it2!=gb.end(); ++it2){
            std::cout<<"<"<<(*it2).first.toLatin1().constData()<<"> ";
         }
         std::cout<<std::endl;
      }
      if (nbg==0) std::cout<<"no groups"<<std::endl;
   }

//************************************
   xmlNodePtr familles::xml_groups(){
      fend gb;
      fagr::iterator it1;
      fend::iterator it2;
      int nb=0,nbf;
      std::string ss;
      xmlNodePtr res,node;
      res=xmlNewNode(NULL, BAD_CAST "groups");
      for (it1=gro.begin(); it1!=gro.end(); ++it1){
         node = xmlNewChild(res, 0, BAD_CAST "group",0);
         ss=(*it1).first.toLatin1().constData();
         xmlNewProp(node, BAD_CAST "name", BAD_CAST ss.c_str());
         nb++;
         gb=(*it1).second;
         nbf=0; ss="";
         for (it2=gb.begin(); it2!=gb.end(); ++it2){
            ss=ss+" "+(*it2).first.toLatin1().constData();
            nbf++;
         }
         xmlNewProp(node, BAD_CAST "families_number", BAD_CAST i2a(nbf).c_str());
         xmlNewProp(node, BAD_CAST "families", BAD_CAST ss.substr(1).c_str());
         //std::cout<<std::endl;
      }
      xmlNewProp(res, BAD_CAST "number", BAD_CAST i2a(nb).c_str());
      return res;
   }

//************************************
   void familles::add(QString nomfam, QString nomgro)
   {
      //std::cout<<"add family <"<<nomfam<<">\t<"<<nomgro<<">\n";
      fagr::iterator it;
      it=fam.find(nomfam);
      if (it==fam.end()){
         //std::cout<<"add new family <"<<nomfam<<">\t<"<<nomgro<<">\n";
         newfam(nomfam);
         it=fam.find(nomfam);
      }
      if (nomgro=="") return; //no group
      (*it).second[nomgro]=0;
      it=gro.find(nomgro);
      if (it==gro.end()){
         //std::cout<<"***new*** "<<nomgro<<" non present\n";
         newgro(nomgro);
         it=gro.find(nomgro);
      }
      (*it).second[nomfam]=0;

   }


//************************************
bool familles::get_number_of_new_family(int sign, med_int *ires, QString *tmp)
//if sign < 0 families faces or tria3 etc...
//if sign >= 0 family zero and family nodes
//outputs in *ires and *tmp
{
   int pas,i,ii;
   QString nomfam;
   fagr::iterator it;
   if (sign>=0) pas=1; else pas=-1;
   *tmp="0"; *ires=0;
   ii=pas;
   for (i=0;i<10000;i++) { //mefiance
      nomfam=nomfam.sprintf("%d",ii);
      it=fam.find(nomfam);
      if (it==fam.end()) {
         *tmp=nomfam; *ires=ii;
         //std::cout<<"NewFamily Found<"<<*ires<<"><"<<*tmp<<">\n";
         return true;
      }
      ii=ii+pas;
   }
   std::cerr<<"***get_number_of_new_family*** Problem new family not found"<<std::endl;
   return false;
}

//************************************
   med_int familles::find_family_on_groups(med_int fam1, med_int fam2)
   {
      med_int ires=0;
      if (fam1==fam2) {ires=fam1; return ires;}
      //find one family whith groups of fam1 and groups of fam2
      fend gb=fuse_goups(fam1,fam2);
      //find if one family have theses groups
      fagr::iterator it1;
      fend::iterator it2;
      for (it1=fam.begin(); it1!=fam.end(); ++it1){
         if (gb==(*it1).second){
            ires=(*it1).first.toLong();
            //std::cout<<"find_family_on_groups old <"<<ires<<"> from <"<<
            //       fam1<<"><"<<fam2<<">\n";
            return ires;
         }
      }
      //std::cout<<"no family found!!! - groups of "<<fam1<<" and "<<fam2<<std::endl;
      QString tmp;
      //fam1 positive for nodes negative faces & mailles
      bool oktmp=get_number_of_new_family(fam1,&ires,&tmp);
      fend::iterator it;
      for (it=gb.begin(); it!=gb.end(); ++it){
          this->add(tmp,(*it).first);
      }
      //std::cout<<"new family <"<<ires<<"> intersection of <"<<fam1<<"><"<<fam2<<">\n";
      return ires;
   }
   
//************************************
   fend familles::fuse_goups(med_int fam1, med_int fam2)
   //concatenation/fusion deux map groupes
   {
      QString nom1,nom2;
      fagr::iterator it1,it2;
      nom1=nom1.sprintf("%d",fam1);
      it1=fam.find(nom1);
      nom2=nom2.sprintf("%d",fam2);
      it2=fam.find(nom2);
      if ( (it1==fam.end())||(it2==fam.end()) ) {
         std::cerr<<"***fuse_goups*** non existing family "<<fam1<<" or "<<fam2<<std::endl;
         fend gb;
         return gb; //empty
      }
      fend gb=(*it1).second; //firt groups
      gb.insert((*it2).second.begin(),(*it2).second.end()); //other groups
      return gb;
      //for debug
      std::cout<<"fuse_goups "<<fam1<<" "<<fam2<<" ";
      fend::iterator it;
      for (it=gb.begin(); it!=gb.end(); ++it){
            std::cout<<"<"<<(*it).first.toLatin1().constData()<<"> ";
      }
      std::cout<<std::endl;
      return gb;
   }

long CVWtab::memoryuse=0; //static
long CVWtab::memorymax=1000*1000000; //static

//************************************
CVWtab::CVWtab(long nb, med_int *pmint)
//constructor with pmint allocated yet with new
{
   //std::cout"***constructor med_int CVWtab***\n";
   size=nb;
   type=1;  //only tmint valide
   tmint=pmint;
   tmflo=NULL;
   memoryuse=memoryuse+sizeof(med_int)*nb;
   //std::cout<<"memoryuse int "<<sizeof(med_int)<<" "<<nb<<" "<<memoryuse<<" "<<memorymax<<std::endl;
   if (memoryuse>memorymax) {
      std::cout<<"***WARNING*** memory max reached "<<memorymax<<std::endl;
      //std::cout<<"memoryuse int "<<sizeof(med_int)<<" "<<nb<<" "<<memoryuse<<std::endl;
   }
}

//************************************
CVWtab::CVWtab(long nb, med_float *pmflo)
//constructor with pmflo allocated yet with new
{
   //std::cout<<"***constructor med_float CVWtab***\n";
   size=nb;
   type=2;   //only tmflo valide
   tmint=NULL;
   tmflo=pmflo;
   memoryuse=memoryuse+sizeof(med_float)*nb;
   //std::cout<<"memoryuse float "<<sizeof(med_float)<<" "<<nb<<" "<<memoryuse<<" "<<memorymax<<std::endl;
   if (memoryuse>memorymax) {
      std::cout<<"***WARNING*** memory max reached "<<memorymax<<std::endl;
      //std::cout<<"memoryuse float "<<sizeof(med_float)<<" "<<nb<<" "<<memoryuse<<std::endl;
   }
}

//************************************
CVWtab::~CVWtab()
{
   bool ok;
   //std::cout<<"   destructor CVWtab *** "<<this->filename<<std::endl;
   ok=this->CVWtab_deallocate();
   //remove temporary file
   if (this->filename!="_NO_FILE")
   {
      remove(this->filename.toLatin1().constData()); //#include <stdio.h>
      //std::cout<<this->filename<<" successfully deleted\n";
   }

}

//************************************
bool CVWtab::CVWtab_deallocate()
{
   //std::cout<<"   deallocate CVWtab*** "<<size<<std::endl;
   if (size <= 0) return false;
   if (tmint)
   {
      delete[] tmint;
      memoryuse=memoryuse-sizeof(med_int)*size;
      size=-size; //precaution
   }
   if (tmflo)
   {
      delete[] tmflo;
      memoryuse=memoryuse-sizeof(med_float)*size;
      size=-size; //precaution
   }
   if (memoryuse<0) std::cout<<"ERROR: on arrays deallocate memory use < 0 "<<memoryuse<<std::endl;
   if (memoryuse==0) std::cout<<"WARNING: on arrays deallocate memory use = 0 "<<std::endl;
   return true;
}

//************************************
bool CVWtab::is_equal(CVWtab *tab2)
{
   //std::cout<<"is_equal tab1 tab2 type="<<this->type<<"  size="<<this->size<<" "<<tab2->size<<std::endl;
   //if (this->type==1) std::cout<<"med_int tab1[0]="<<this->tmint[0]<<std::endl;
   //if (this->type==2) std::cout<<"med_float tab1[0]="<<this->tmflo[0]<<std::endl;
   if (this->size!=tab2->size) return false;
   if (this->type!=tab2->type) return false;
   if (this->type==1)
   {
      if (!this->tmint)
      {  std::cout<<"***is_equal*** pb pointer NULL with tmint size="<<this->size<<std::endl;
         return false;
      }
      for (long i=0; i < this->size; i++)
         if (this->tmint[i]!=tab2->tmint[i]) return false;
   }
   if (this->type==2)
   {
      if (!this->tmflo)
      {  std::cout<<"***is_equal*** pb pointer NULL with tmflo size="<<this->size<<std::endl;
         return false;
      }
      for (long i=0; i < this->size; i++)
         if (this->tmflo[i]!=tab2->tmflo[i]) return false;
   }
   return true;
}

//************************************
bool CVW_is_equal_vertices(CVWtab *tab1, long i1,
                       CVWtab *tab2, long i2, int verbose)
//verbose 0 for no prints
//verbose 1 for print vertices not equals
//verbose 2 for print also vertices equals (debug)
{
   //std::cout<<"is_equal_vertice size="<<tab1->size<<" "<<tab2->size<<std::endl;
   bool ok=false;
   med_float *p1,*p2;
   //vertices indices from 1 not 0!
   long di1=(i1-1)*3, di2=(i2-1)*3;
   if (di1<0 || di1>=tab1->size)
   {
      std::cerr<<"BadIndice tab1 in is_equal_vertices "<<
            di1<<" not in "<<tab1->size<<std::endl;
      return false;
   }
   if (di2<0 || di2>=tab2->size)
   {
      std::cerr<<"BadIndice tab2 in is_equal_vertices "<<
            di2<<" not in "<<tab2->size<<std::endl;
      return false;
   }
   p1=(tab1->tmflo+di1);
   p2=(tab2->tmflo+di2);
   if (p1[0]==p2[0] && p1[1]==p2[1] && p1[2]==p2[2]) ok=true ;
   if (!ok && verbose>0) printf(
      "Vertices differents (%.16g %.16g %.16g) (%.16g %.16g %.16g)\n",
      p1[0],p1[1],p1[2],p2[0],p2[1],p2[2]);
   else
      if (verbose>1) printf(
      "Vertices equals     (%.16g %.16g %.16g)\n",
      p1[0],p1[1],p1[2]);
   return ok;
}

//************************************
bool CVW_FindString(const std::string &str,std::fstream &Ff, long &count)
//find in file first line with string str in first position of line
//converts count value expected after "='" in line found
{
   std::string line;
   QString tmp;
   do
   {
      if (getline(Ff,line))
      {
         if (line[0]==str[0]) //faster
         {
            if (line.find(str)==0)
            {
            tmp=line.c_str();
            bool ok;
            count=tmp.section('\'',1,1).toLong(&ok);
            return ok;
            }
         }
      }
      else
      {
         std::cerr<<"Problem line '"<<str<<"' not found in file\n"<<std::endl;
         return false;
      }
   } while (1);
   return true;
}

//************************************
bool CVW_FindStringInFirstLines(const std::string &str,std::fstream &Ff, long &maxline)
//find in file maximum maxline first lines with string str in first position of line
//converts count value expected after " " in line found
{
   std::string line;
   long nbLine=0;
   do
   {
      if (getline(Ff,line))
      {
         nbLine++;
         if (line[0]==str[0]) //faster
         {
            if (line.find(str)==0)
            {
            return true;
            }
         }
         if (nbLine>=maxline)
         {
            std::cerr<<"Problem line '"<<str<<"' not found in first "<<maxline<<" lines of file\n"<<std::endl;
            return false;
         }
      }
      else
      {
         std::cerr<<"Problem line '"<<str<<"' not found in all file\n"<<std::endl;
         return false;
      }
   } while (1);
   return true;
}

//************************************
bool ghs3dprl_mesh_wrap::ReadFileMSGnew(const QString FileName)
//read file .glo with no parser xml because big file (volume)
//no read of <receive> for speed (and so no test)
{
   QString tmp;
   std::fstream Ff(FileName.toLatin1().constData(),std::ios_base::in);
   std::string line;
   long i,count,nbneighbour,ineighbour;
   bool ok;

   if (!Ff.is_open())
   {
      std::cerr<<"Problem File '"<<FileName.toLatin1().constData()<<"' not open\n"<<std::endl;
      return false;
   }

   //Lit les donnees :
   if (!CVW_FindString("<neighbours count=",Ff,nbneighbour)) return false;
   if (verbose>2) std::cout<<"NeighboursCountDomain_"<<this->nofile<<"="<<nbneighbour<<std::endl;
   for (i=1; i<=nbneighbour; i++)
   {
      if (!CVW_FindString("<neighbour indice=",Ff,ineighbour)) return false;
      if (!CVW_FindString("<vertices count=",Ff,count)) return false;
      if (count>0){
         med_int *tmint=new med_int[count];
         for (int i=0; i<count; i++) Ff>>tmint[i];
         if (verbose>4) std::cout<<"Vertices "<<tmint[0]<<" "<<tmint[1]<<"... "<<tmint[count-1]<<std::endl;

         CVWtab *montab=new CVWtab(count,tmint);
         tmp=tmp.sprintf("MS%ld NE%ld VE SE",this->nofile,ineighbour);
         ok=this->insert_key(tmp,montab);
      }
      if (!CVW_FindString("<edges count=",Ff,count)) return false;
      if (count>0){
         med_int *tmint=new med_int[count];
         for (int i=0; i<count; i++) Ff>>tmint[i];
         if (verbose>4) std::cout<<"Edges "<<tmint[0]<<" "<<tmint[1]<<"... "<<tmint[count-1]<<std::endl;

         CVWtab *montab=new CVWtab(count,tmint);
         tmp=tmp.sprintf("MS%ld NE%ld ED SE",this->nofile,ineighbour);
         ok=this->insert_key(tmp,montab);
      }
      if (!CVW_FindString("<faces count=",Ff,count)) return false;
      if (count>0){
         med_int *tmint=new med_int[count];
         for (int i=0; i<count; i++) Ff>>tmint[i];
         if (verbose>4) std::cout<<"Faces "<<tmint[0]<<" "<<tmint[1]<<"... "<<tmint[count-1]<<std::endl;

         CVWtab *montab=new CVWtab(count,tmint);
         tmp=tmp.sprintf("MS%ld NE%ld FA SE",this->nofile,ineighbour);
         ok=this->insert_key(tmp,montab);
      }
      if (!CVW_FindString("<elements count=",Ff,count)) return false;
      if (count>0){
         med_int *tmint=new med_int[count];
         for (int i=0; i<count; i++) Ff>>tmint[i];
         if (verbose>4) std::cout<<"Elements "<<tmint[0]<<" "<<tmint[1]<<"... "<<tmint[count-1]<<std::endl;

         CVWtab *montab=new CVWtab(count,tmint);
         tmp=tmp.sprintf("MS%ld NE%ld EL SE",this->nofile,ineighbour);
         ok=this->insert_key(tmp,montab);
      }
   }

   //Ferme le fichier :
   Ff.close();
   this->nbfiles++;
   return true;
}

//************************************
bool ghs3dprl_mesh_wrap::TestExistingFileMESHnew(const QString FileName)
//read file .glo with no parser xml because big file (volume)
//no read of <receive> for speed (and so no test)
{
   QString tmp;
   std::fstream Ff(FileName.toLatin1().constData(),std::ios_base::in);
   std::string line;
   long i,count,meshversion,maxline;
   bool ok;

   if (!Ff.is_open())
   {
      std::cerr<<"Problem File '"<<FileName.toLatin1().constData()<<"' not open\n"<<std::endl;
      return false;
   }

   //Lit les donnees au debut du fichier, 1 lignes maxi:
   maxline=1;
   if (!CVW_FindStringInFirstLines("MeshVersionFormatted",Ff,maxline)) return false;
   if (verbose>2) std::cout<<"MeshVersionFormatted_"<<this->nofile<<" ok"<<std::endl;

   //Ferme le fichier :
   Ff.close();
   this->nbfiles++;
   return true;
}

///************************************
bool ghs3dprl_mesh_wrap::ReadFileGLO(const QString FileName)
//read file .glo with no parser xml because big file (volume)
{
   QString tmp;
   std::fstream Ff(FileName.toLatin1().constData(),std::ios_base::in);
   std::string line;
   long count;
   bool ok;

   if (!Ff.is_open())
   {
      std::cerr<<"Problem File '"<<FileName.toLatin1().constData()<<"' not open\n"<<std::endl;
      return false;
   }

   //Lit les donnees :
   if (!CVW_FindString("<vertices count=",Ff,count)) return false;
   if (verbose>3) std::cout<<"GloVerticesCount="<<count<<std::endl;
   if (count>0)
   {
      med_int *tmint=new med_int[count];
      for (int i=0; i<count; i++) Ff>>tmint[i];
      if (verbose>4) std::cout<<"Elements "<<tmint[0]<<" "<<tmint[1]<<"... "<<tmint[count-1]<<std::endl;

      CVWtab *montab=new CVWtab(count,tmint);
      tmp=tmp.sprintf("GL%ld VE",this->nofile);
      ok=this->insert_key(tmp,montab);
   }

   if (!CVW_FindString("<edges count=",Ff,count)) return false;
   if (verbose>3) std::cout<<"GloEdgesCount="<<count<<std::endl;
   if (count>0)
   {
      med_int *tmint=new med_int[count];
      for (int i=0; i<count; i++) Ff>>tmint[i];
      if (verbose>4) std::cout<<"Elements "<<tmint[0]<<" "<<tmint[1]<<"... "<<tmint[count-1]<<std::endl;

      CVWtab *montab=new CVWtab(count,tmint);
      tmp=tmp.sprintf("GL%ld ED",this->nofile);
      ok=this->insert_key(tmp,montab);
   }

   if (!CVW_FindString("<faces count=",Ff,count)) return false;
   if (verbose>3) std::cout<<"GloFacesCount="<<count<<std::endl;
   if (count>0)
   {
      med_int *tmint=new med_int[count];
      for (int i=0; i<count; i++) Ff>>tmint[i];
      if (verbose>4) std::cout<<"Elements "<<tmint[0]<<" "<<tmint[1]<<"... "<<tmint[count-1]<<std::endl;

      CVWtab *montab=new CVWtab(count,tmint);
      tmp=tmp.sprintf("GL%ld FA",this->nofile);
      ok=this->insert_key(tmp,montab);
   }

   if (!CVW_FindString("<elements count=",Ff,count)) return false;
   if (verbose>3) std::cout<<"GloElementsCount="<<count<<std::endl;
   if (count>0)
   {
      med_int *tmint=new med_int[count];
      for (int i=0; i<count; i++) Ff>>tmint[i];
      if (verbose>4) std::cout<<"Elements "<<tmint[0]<<" "<<tmint[1]<<"... "<<tmint[count-1]<<std::endl;

      CVWtab *montab=new CVWtab(count,tmint);
      tmp=tmp.sprintf("GL%ld EL",this->nofile);
      ok=this->insert_key(tmp,montab);
   }
   //Ferme le fichier :
   Ff.close();
   this->nbfiles++;
   return true;
}

///************************************
bool ghs3dprl_mesh_wrap::ReadFileGLOBAL(const QString FileName)
//read file .global ascii (no xml)
//first line: Vertices Edges Triangles Tetrahedra
{
   std::string line("                                            ");
   QString tmp;
   std::fstream Ff(FileName.toLatin1().constData(),std::ios_base::in);
   long vert=0,edge=0,tria=0,tetr=0,count=0;
   med_int *tmint;
   CVWtab *montab;
   bool ok;
   
   if (verbose>=6)std::cout<<"Read file '"<<FileName.toLatin1().constData()<<std::endl;
   if (!Ff.is_open()){
      std::cerr<<"Problem file '"<<FileName.toLatin1().constData()<<"' not open\n"<<std::endl;
      return false;
   }
   
   //Lit les donnees :
   Ff>>vert>>edge>>tria>>tetr;
   std::cout<<"First line "<<vert<<" "<<edge<<" "<<tria<<" "<<tetr<<std::endl;
   
   if (vert<0)
   {
      std::cerr<<"Problem Vertices: a positive integer is expected"<<std::endl;
      return false;
   }

   if (edge<0)
   {
      std::cerr<<"Problem Edges: a positive integer is expected"<<std::endl;
      return false;
   }

   if (tria<0)
   {
      std::cerr<<"Problem Triangles: a positive integer is expected"<<std::endl;
      return false;
   }

   if (tetr<0)
   {
      std::cerr<<"Problem Tetrahedra: a positive integer is expected"<<std::endl;
      return false;
   }

   count=vert;
   tmint=new med_int[count];
   for (int i=0; i<count; i++) Ff>>tmint[i];
   if (verbose>4) std::cout<<"Vertices ("<<count<<" ) "<<tmint[0]<<" "<<tmint[1]<<"... "<<tmint[count-1]<<std::endl;

   montab=new CVWtab(count,tmint);
   tmp=tmp.sprintf("GL%ld VE",this->nofile);
   ok=this->insert_key(tmp,montab);

   count=edge;
   tmint=new med_int[count];
   for (int i=0; i<count; i++) Ff>>tmint[i];
   if (verbose>4) std::cout<<"Edges ("<<count<<") "<<tmint[0]<<" "<<tmint[1]<<"... "<<tmint[count-1]<<std::endl;

   montab=new CVWtab(count,tmint);
   tmp=tmp.sprintf("GL%ld ED",this->nofile);
   ok=this->insert_key(tmp,montab);

   count=tria;
   tmint=new med_int[count];
   for (int i=0; i<count; i++) Ff>>tmint[i];
   if (verbose>4) std::cout<<"Triangles ("<<count<<") "<<tmint[0]<<" "<<tmint[1]<<"... "<<tmint[count-1]<<std::endl;

   montab=new CVWtab(count,tmint);
   tmp=tmp.sprintf("GL%ld FA",this->nofile);
   ok=this->insert_key(tmp,montab);

   count=tetr;
   tmint=new med_int[count];
   for (int i=0; i<count; i++) Ff>>tmint[i];
   if (verbose>4) std::cout<<"Tetrahedra ("<<count<<") "<<tmint[0]<<" "<<tmint[1]<<"... "<<tmint[count-1]<<std::endl;

   montab=new CVWtab(count,tmint);
   tmp=tmp.sprintf("GL%ld EL",this->nofile);
   ok=this->insert_key(tmp,montab);

   //Ferme le fichier :
   Ff.close();
   this->nbfiles++;
   return true;
}

//************************************
bool ghs3dprl_mesh_wrap::ReadFileFACES(const QString FileName)
//read file .faces (wrap)
{
   QString tmp;
   std::fstream Ff(FileName.toLatin1().constData(),std::ios_base::in);
   std::string line;
   long nbelem,ntype;
   bool ok;

   if (!Ff.is_open())
   {
      std::cerr<<"Problem File '"<<FileName.toLatin1().constData()<<"' not open\n"<<std::endl;
      return false;
   }

   //Lit les donnees :
   //Replace le pointeur de fichier au debut :f.seekg(0);
   if (getline(Ff,line))
   {
      tmp=line.c_str();
      nbelem=tmp.section(' ',0,0).toLong(&ok);
   }
   else
   {
      std::cerr<<"Problem on first line of file"<<std::endl;
      return false;
   }
   if (verbose>3) std::cout<<"FacesNumberOfElements="<<nbelem<<std::endl;
   med_int *tmint=new med_int[nbelem*7];
   for (int i=0; i<nbelem*7; i=i+7)
   {
      Ff>>ntype;
      if (ntype!=3) //only triangles
      {
         std::cerr<<"Problem on ntype != 3"<<std::endl;
         return false;
      }
      for (int j=0; j<7; j++) Ff>>tmint[i+j];
      //for (int j=0; j<7; j++) std::cout<<tmint[i+j]<<' '; std::cout<<std::endl;
   }
   if (verbose>4) std::cout<<"Elements "<<tmint[0]<<" "<<tmint[1]<<"... "<<tmint[nbelem*7-1]<<std::endl;

   CVWtab *montab=new CVWtab(nbelem*7,tmint);
   tmp=tmp.sprintf("FC%ld",this->nofile);
   ok=this->insert_key(tmp,montab);

   Ff.close();
   this->nbfiles++;
   return true;
}

//************************************
bool ghs3dprl_mesh_wrap::ReadFileMESH(const QString FileName)
//read file .mesh from tetra_hpc (with volume)
{
   std::string line("                                            ");
   QString tmp;
   std::fstream Ff(FileName.toLatin1().constData(),std::ios_base::in);
   long Mversion=0,Mdim=0,Mvert=0,Mtria=0,Medge=0,Mtetra=0,count=0;
   med_int garbage;
   med_int *tmint;
   bool ok;
   
   if (verbose>=6)std::cout<<"Read file '"<<FileName.toLatin1().constData()<<std::endl;
   if (!Ff.is_open()){
      std::cerr<<"Problem file '"<<FileName.toLatin1().constData()<<"' not open\n"<<std::endl;
      return false;
   }

   //lit les données :
   if (getline(Ff,line) && (line.find("MeshVersionFormatted")==0))
   {
      tmp=line.c_str();
      Mversion=tmp.section(' ',1,1).toLong(&ok);
   }
   else
   {
      std::cerr<<"Problem on line 1 of file: 'MeshVersionFormatted' expected"<<std::endl;
      return false;
   }
   
   getline(Ff,line);

   if (getline(Ff,line) && (line.find("Dimension 3")==0))
   {
      tmp=line.c_str();
      Mdim=tmp.section(' ',1,1).toLong(&ok);
   }
   else
   {
      std::cerr<<"Problem on line 3 of file: Dimension 3 expected"<<std::endl;
      return false;
   }
   
   getline(Ff,line);

   if (!(getline(Ff,line) && (line.find("Vertices")==0)))
   {
      std::cerr<<"Problem on line 5 of file: 'Vertices' expected"<<std::endl;
      return false;
   }
   if (getline(Ff,line))
   {
      tmp=line.c_str();
      Mvert=tmp.toLong(&ok);
   }
   else
   {
      std::cerr<<"Problem Vertices: a positive integer is expected"<<std::endl;
      return false;
   }
   if (Mvert<=0)
   {
      std::cerr<<"Problem Vertices: a positive integer is expected"<<std::endl;
      return false;
   }

   count=Mvert;
   med_float *tmflo=new med_float[count*3];
   for (int i=0; i<count*3; i=i+3) Ff>>tmflo[i]>>tmflo[i+1]>>tmflo[i+2]>>garbage;
   if (verbose>4) std::cout<<"Vertices ("<<count<<") "<<tmflo[0]<<" "<<tmflo[1]<<"... "<<tmflo[count*3-1]<<std::endl;

   CVWtab *montab=new CVWtab(count*3,tmflo);
   tmp=tmp.sprintf("NB%ld VC",this->nofile);
   ok=this->insert_key(tmp,montab);

   getline(Ff,line);
   getline(Ff,line);
   
   if (!(getline(Ff,line) && (line.find("Edges")==0)))
   {
      std::cerr<<"Problem on line 'Edges' of file '"<<FileName.toLatin1().constData()<<"' :found '"<<line<<"' instead"<<std::endl;
      return false;
   }
   if (getline(Ff,line))
   {
      tmp=line.c_str();
      Medge=tmp.toLong(&ok);
   }
   else
   {
      std::cerr<<"Problem on line 'Edges' of file: a positive integer is expected"<<std::endl;
      return false;
   }
   if (Medge<=0)
   {
      std::cerr<<"Problem on line 'Edges' of file: a positive integer is expected"<<std::endl;
      return false;
   }

   count=Medge;
   tmint=new med_int[count*2]; //*3
   for (int i=0; i<count*2; i=i+2) {
     Ff>>tmint[i]>>tmint[i+1]>>garbage;
   }
   if (verbose>4) std::cout<<"Edges ("<<count<<") "<<tmint[0]<<" "<<tmint[1]<<"... "<<tmint[count*2-1]<<std::endl;
   montab=new CVWtab(count*2,tmint);
   tmp=tmp.sprintf("NB%ld ED",this->nofile); //TODO see if it could serve
   ok=this->insert_key(tmp,montab);
   
   getline(Ff,line);
   getline(Ff,line);

   if (!(getline(Ff,line) && (line.find("Triangles")==0)))
   {
      std::cerr<<"Problem on line 'Triangles' of file '"<<FileName.toLatin1().constData()<<"' :found '"<<line<<"' instead"<<std::endl;
      return false;
   }
   if (getline(Ff,line))
   {
      tmp=line.c_str();
      Mtria=tmp.toLong(&ok);
   }
   else
   {
      std::cerr<<"Problem on line 'Triangles' of file: a positive integer is expected"<<std::endl;
      return false;
   }
   if (Mtria<=0)
   {
      std::cerr<<"Problem on line 'Triangles' of file: a positive integer is expected"<<std::endl;
      return false;
   }

   count=Mtria;
   tmint=new med_int[count*3]; //*7 as older files .faces, obsolete
   for (int i=0; i<count*3; i=i+3) {
     Ff>>tmint[i]>>tmint[i+1]>>tmint[i+2]>>garbage;
   }
   if (verbose>4) std::cout<<"Triangles ("<<count<<") "<<tmint[0]<<" "<<tmint[1]<<"... "<<
       tmint[count*3-5]<<" "<<tmint[count*3-4]<<" "<<tmint[count*3-3]<<" "<<
       tmint[count*3-2]<<" "<<tmint[count*3-1]<<std::endl;

   montab=new CVWtab(count*3,tmint);
   tmp=tmp.sprintf("FC%ld",this->nofile);
   ok=this->insert_key(tmp,montab);
   
   getline(Ff,line);
   getline(Ff,line);

   if (!(getline(Ff,line) && (line.find("Tetrahedra")==0)))
   {
      std::cerr<<"Problem on line 'Tetrahedra' of file: not found"<<std::endl;
      return false;
   }
   if (getline(Ff,line))
   {
      tmp=line.c_str();
      Mtetra=tmp.toLong(&ok);
   }
   else
   {
      std::cerr<<"Problem on line 'Tetrahedra' of file: a positive integer is expected"<<std::endl;
      return false;
   }
   if (Mtetra<=0)
   {
      std::cerr<<"Problem on line 'Tetrahedra' of file: a positive integer is expected"<<std::endl;
      return false;
   }

   if (verbose>=2)
   {
      std::cout<<"MeshVersionFormatted="<<Mversion<<std::endl;
      std::cout<<"MeshDimension="<<Mdim<<std::endl;
      std::cout<<"MeshNumberOfVertices="<<Mvert<<std::endl;
      std::cout<<"MeshNumberOfEdges="<<Medge<<std::endl;
      std::cout<<"MeshNumberOfTriangles="<<Mtria<<std::endl;
      std::cout<<"MeshNumberOfTetrahedra="<<Mtetra<<std::endl;
   }

   count=Mtetra;
   tmint=new med_int[count*4];
   for (int i=0; i<count*4; i=i+4) Ff>>tmint[i]>>tmint[i+1]>>tmint[i+2]>>tmint[i+3]>>garbage;
   if (verbose>4) std::cout<<"Tetrahedra ("<<count<<") "<<tmint[0]<<" "<<tmint[1]<<"... "<<tmint[count*4-1]<<std::endl;

   montab=new CVWtab(count*4,tmint);
   tmp=tmp.sprintf("NB%ld EV",this->nofile);
   ok=this->insert_key(tmp,montab);

   //swap on file if too big for memory in one cpu
   //default 1GOctet/8(for double)/10(for arrays in memory at the same time)
   if (Mvert*3>this->nbelem_limit_swap)
     this->SwapOutOfMemory_key_mesh_wrap(QRegExp("NB",Qt::CaseSensitive,QRegExp::RegExp));
   //beware record 6 lenght 1
   //ferme le fichier :
   Ff.close();
   this->nbfiles++;
   return true;
}

//************************************
bool ghs3dprl_mesh_wrap::ReadFileNOBOITE(const QString FileName)
//read file .noboite (volume)
//for huge files it could be better use ReadFileNOBOITEB (B=binary format)
//(parameter option of ghs3d but NOT tepal)
{
   QString tmp;
   std::fstream Ff(FileName.toLatin1().constData(),std::ios_base::in);
   long ne,np,npfixe,subnumber,reste;
   bool ok;

   if (!Ff.is_open()){
      std::cerr<<"Problem File '"<<FileName.toLatin1().constData()<<"' not open\n"<<std::endl;
      return false;
   }

   //lit les donnees :
   Ff>>ne>>np>>npfixe;
   if (verbose>3){
      std::cout<<"NoboiteNumberOfElements="<<ne<<std::endl;
      std::cout<<"NoboiteNumberOfVertices="<<np<<std::endl;
      std::cout<<"NoboiteNumberOfSpecifiedPoints="<<npfixe<<std::endl;
   }

   for (int i=1; i<=17-3; i++) Ff>>reste;
   //printf("reste %ld\n",reste);
   med_int *tmint=new med_int[ne*4];
   for (int i=0; i<ne*4; i++) Ff>>tmint[i];
   if (verbose>4) std::cout<<"Elements "<<tmint[0]<<" "<<tmint[1]<<"... "<<tmint[ne*4-1]<<std::endl;

   CVWtab *montab=new CVWtab(ne*4,tmint);
   tmp=tmp.sprintf("NB%ld EV",this->nofile);
   ok=this->insert_key(tmp,montab);

   med_float *tmflo=new med_float[np*3];
   for (int i=0; i<np*3; i++) Ff>>tmflo[i];
   if (verbose>4) std::cout<<"Vertices "<<tmflo[0]<<" "<<tmflo[1]<<"... "<<tmflo[np*3-1]<<std::endl;

   montab=new CVWtab(np*3,tmflo);
   tmp=tmp.sprintf("NB%ld VC",this->nofile);
   ok=this->insert_key(tmp,montab);

   Ff>>subnumber;
   if (verbose>2) std::cout<<"NumberOfSubdomains="<<subnumber<<std::endl;
   //tmint=new med_int[subnumber*3];
   tmint=new  med_int[subnumber*3];
   long onelong,maxint;
   maxint=std::numeric_limits<int>::max();
   //pb from tepalv2
   bool isproblem=true;
   for (int i=0; i<subnumber*3; i++) {
     Ff>>onelong;
     //pb from tepalv2
     if (onelong<0) {
      if (isproblem && verbose>1) std::cout<<"There is one or more negative med_int value in NumberOfSubdomains "<<onelong<<std::endl;
      isproblem=false;
      onelong=-1;
      }
     if (onelong>maxint) {
      if (isproblem && verbose>1) std::cout<<"There is one or more truncated med_int value in NumberOfSubdomains "<<onelong<<std::endl;
      isproblem=false;
      onelong=-2;
      }
     tmint[i]=(int)onelong;
     }
   if (verbose>4) std::cout<<"Subdomains "<<tmint[0]<<" "<<tmint[1]<<"... "<<tmint[subnumber*3-1]<<std::endl;

   montab=new CVWtab(subnumber*3,tmint);
   tmp=tmp.sprintf("NB%ld SN",this->nofile);
   ok=this->insert_key(tmp,montab);

   //swap on file if too big for memory in one cpu
   //default 1GOctet/8(for double)/10(for arrays in memory at the same time)
   if (np*3>this->nbelem_limit_swap)
     this->SwapOutOfMemory_key_mesh_wrap(QRegExp("NB",Qt::CaseSensitive,QRegExp::RegExp));
   //beware record 6 lenght 1
   //ferme le fichier :
   Ff.close();
   this->nbfiles++;
   return true;
}

//************************************
bool ghs3dprl_mesh_wrap::ReadFileNOBOITEB(const QString FileName)
//read file .noboiteb (volume)
//for huge files it could be better use ReadFileNOBOITEB (B=binary format)
//but NOT parameter option of tepal
//idem ReadFileNOBOITE with read unformatted
{
   bool ok;
   QString tmp;
   std::cerr<<"Problem function ReadFileNOBOITEB\n"
       <<"(no FORTRAN binary format files in tepal)\n\n";
   //file binary
   FILE *Ff=fopen(FileName.toLatin1().constData(),"rb");
   long ne,np,npfixe,reste,subnumber;

   //http://www.math.utah.edu/software/c-with-fortran.html
   //record 1 from format FORTRAN begins and ends with lengh of record
   //=> 2*long(68)     (68=17*4octets)
   long r1[17+2];
   if (!Ff){
      std::cerr<<"Problem File '"<<FileName.toLatin1().constData()<<"' not open\n"<<std::endl;
      return false;
   }
   //read datas :
   fread(&r1,sizeof(long),17+2,Ff);
   for (long i=1; i<18; i++) std::cout<<"R1("<<i<<")="<<r1[i]<<std::endl;

   if (r1[0]!=68){
      std::cerr<<"First FORTRAN record of File '"<<FileName.toLatin1().constData()<<"' not length 17*long"<<std::endl;
      return false;
   }
   ne=r1[1];
   np=r1[2];
   npfixe=r1[3];
   if (verbose>3){
      std::cout<<"NoboitebNumberOfElements="<<ne<<std::endl;
      std::cout<<"NoboitebNumberOfVertices="<<np<<std::endl;
      std::cout<<"NoboitebNumberOfSpecifiedPoints="<<npfixe<<std::endl;
   }
   //etc...could be done if necessary not debugged
   fread(&reste,sizeof(long),1,Ff);
   long *tlong=new long[ne*4];
   med_int *tmint=new med_int[ne*4];
   fread(tlong,sizeof(long),ne*4,Ff);
   fread(&reste,sizeof(long),1,Ff);
   for (long i=0; i<ne*4; i++) tmint[i]=tlong[i];
   delete tlong;
   if (verbose>4) std::cout<<"Elements "<<tmint[0]<<" "<<tmint[1]<<"... "<<tmint[ne*4-1]<<std::endl;

   CVWtab *montab=new CVWtab(ne*4,tmint);
   tmp=tmp.sprintf("NB%ld EV",this->nofile);
   ok=this->insert_key(tmp,montab);

   fread(&reste,sizeof(long),1,Ff);
   //std::cout<<"info "<<reste<<" "<<np*3<<" "<<sizeof(med_float)<<std::endl;
   float *tfloat=new float[np*3];
   med_float *tmflo=new med_float[np*3];
   fread(tfloat,sizeof(float),np*3,Ff);
   fread(&reste,sizeof(long),1,Ff);
   for (long i=0; i<np*3; i++) tmflo[i]=tfloat[i];
   delete tfloat;
   if (verbose>4) printf("Vertices %g %g ... %g \n",tmflo[0],tmflo[1],tmflo[np*3-1]);

   montab=new CVWtab(np*3,tmflo);
   tmp=tmp.sprintf("NB%ld VC",this->nofile);
   ok=this->insert_key(tmp,montab);

   fread(&reste,sizeof(long),1,Ff);
   fread(&subnumber,sizeof(long),1,Ff);
   fread(&reste,sizeof(long),1,Ff);
   if (verbose>2) std::cout<<"NumberOfSubdomains="<<subnumber<<std::endl;
   fread(&reste,sizeof(long),1,Ff);
   tlong=new long[subnumber*3];
   fread(tlong,sizeof(long),subnumber*3,Ff);
   fread(&reste,sizeof(long),1,Ff);
   if (verbose>4) printf("Subdomains %ld %ld ... %ld \n",tlong[0],tlong[1],tlong[subnumber*3-1]);

   tmint=new med_int[subnumber*3];
   for (long i=0; i<subnumber*3; i++) tmint[i]=tlong[i];
   delete tlong;
   montab=new CVWtab(subnumber*3,tmint);
   tmp=tmp.sprintf("NB%ld SN",this->nofile);
   ok=this->insert_key(tmp,montab);

   //swap on file if too big for memory in one cpu
   //default 1GOctet/8(for double)/10(for arrays in memory at the same time)
   if (np*3>this->nbelem_limit_swap)
     this->SwapOutOfMemory_key_mesh_wrap(QRegExp("NB",Qt::CaseSensitive,QRegExp::RegExp));

   //beware record 6 lenght 1
   //ferme le fichier :
   fclose(Ff);
   this->nbfiles++;
   return true;
}

//************************************
bool ghs3dprl_mesh_wrap::ReadFilePOINTS(const QString FileName)
//read file .points (wrap)
{
   QString tmp;
   long nb;
   long maxlen=128;
   bool ok=true;

   //Lit les donnees :
   QFile Ff(FileName);
   //NOT Raw because Raw=non-buffered file access
   //qt3 ok=Ff.open(IO_ReadOnly|IO_Translate);
   ok=Ff.open(QIODevice::ReadOnly|QIODevice::Text);
   if (!ok){
      std::cerr<<"Problem File '"<<FileName.toLatin1().constData()<<"' not open\n"<<std::endl;
      return false;
   }
   tmp=Ff.readLine(maxlen);
   tmp=tmp.simplified();
   nb=tmp.toLong(&ok);
   if (!ok){
      std::cerr<<"Problem conversion File '"<<FileName.toLatin1().constData()<<"\n"<<std::endl;
      return false;
   }
   if (verbose>2) std::cout<<"NumberOfVertices="<<nb<<std::endl;
   med_float *tmflo=new med_float[3*nb]; //coordinates
   med_int *tmint=new med_int[nb];         //nrs (attribute of point)
   long il3=0;
   for ( long il=0; il<nb; il++ ){
      tmp=Ff.readLine(maxlen);
      tmp=tmp.simplified();
      for ( int j=0; j<3; j++ ){
         tmflo[il3]=tmp.section(' ',j,j).toDouble(&ok);
         //std::cout<<"cv '"<<tmflo[il3]<<"' "<<il3<<std::endl;
         il3++;
         if (!ok){
            std::cerr<<"Problem conversion File '"<<FileName.toLatin1().constData()<<"\n"<<std::endl;
            return false;
         }
      }
      //nrs is vertex attribute
      tmint[il]=tmp.section(' ',3,3).toLong(&ok);
      if (!ok){
         std::cerr<<"Problem conversion File '"<<FileName.toLatin1().constData()<<"\n"<<std::endl;
         return false;
      }
   }
   //beware no examples with each specified points (if any) here
   CVWtab *montab=new CVWtab(nb,tmint); //init montab->tmint nrs
   tmp=tmp.sprintf("PO%ld NRS",this->nofile);
   ok=this->insert_key(tmp,montab);

   montab=new CVWtab(nb,tmflo); //init montab->tmflo xyz
   tmp=tmp.sprintf("PO%ld XYZ",this->nofile);
   ok=this->insert_key(tmp,montab);

   //Ferme le fichier :
   Ff.close();
   this->nbfiles++;
   return true;
}

//************************************
bool ghs3dprl_mesh_wrap::list_keys_mesh_wrap()
{
   QHashIterator<QString,CVWtab*> it( this->mestab);
   while ( it.hasNext() ) {
     it.next();
     QString nom = it.key().leftJustified(32,' ');
     std::cout<<nom.toLatin1().constData()<<"-> size="<<it.value()->size<<std::endl;
   }
   return true;
}

//************************************
long ghs3dprl_mesh_wrap::remove_all_keys_mesh_wrap()
{
   long nb=this->remove_key_mesh_wrap(QRegExp(".",Qt::CaseSensitive,QRegExp::RegExp));
   return nb;
}

//************************************
long ghs3dprl_mesh_wrap::remove_key_mesh_wrap(const QRegExp &rxp)
{
   long nbremove=0;
   QMutableHashIterator<QString,CVWtab*> it(this->mestab);
   while ( it.hasNext() ){
     it.next();
     if (it.key().contains(rxp)) {
        nbremove++;
        if (this->verbose>6) std::cout<<"remove key "<<it.key().toLatin1().constData()<<std::endl;
        delete it.value();
        it.remove();
     }
   }
   return nbremove;
}

//************************************
long ghs3dprl_mesh_wrap::nb_key_mesh_wrap(const QRegExp &rxp)
{
   long nbremove=0;
   //std::cout<<"nb_key_mesh_wrap on "<<std::endl;
   QMutableHashIterator<QString,CVWtab*> it(this->mestab);
   while ( it.hasNext() ){
     it.next();
     if (it.key().contains(rxp)) nbremove++;
   }
   //std::cout<<"nb_key_mesh_wrap found "<<nbremove<<std::endl;
   return nbremove;
}

//************************************
bool SwapOnFile(const QString &key,const QString &path,CVWtab *tab,int verbose)
//
{
   //return true;
   if (tab->filename=="_NO_FILE"){
      tab->filename=path+key+".tmp";
      tab->filename.replace(" ","_"); //replace " " by "_"

      //swap disque binaire
      //montab->tmint=new long[10]; //for test
      //for (int i=0; i<10; i++) montab->tmint[i]=i*2;
      FILE *fichier=fopen(tab->filename.toLatin1().constData(),"wb");
      long taille;
      taille=tab->size;
      fwrite(&taille,sizeof(taille),1,fichier);
      if (tab->tmint){
         if (verbose>3)
         std::cout<<"SwapOnFile_binary "<<tab->filename.toLatin1().constData()<<
         " NbElements "<<taille<<
         " SizeElement_med_int   "<<sizeof(med_int)<<
         " TotalSizeBinary " <<taille*sizeof(med_int)<<std::endl;
         fwrite(tab->tmint,sizeof(med_int),taille,fichier);
         //fread(&gagnants,sizeof(gagnants),1,fichier);
      }
      if (tab->tmflo){
         if (verbose>3)
         std::cout<<"SwapOnFile_binary "<<tab->filename.toLatin1().constData()<<
         " NbElements "<<taille<<
         " SizeElement_med_float "<<sizeof(med_float)<<
         " TotalSizeBinary " <<taille*sizeof(med_float)<<std::endl;
         fwrite(tab->tmflo,sizeof(med_float),taille,fichier);
      }
      fclose(fichier);
   }
   else{
      if (verbose>3) std::cout<<"SwapOnFile in binary file done yet "<<
         tab->filename.toLatin1().constData()<<std::endl;
   }
   //deallocate because swap disk binary mode
   tab->CVWtab_deallocate(); //free memory
   return true;
}

//************************************
long ghs3dprl_mesh_wrap::SwapOutOfMemory_key_mesh_wrap(const QRegExp &rxp,
                                                       long ifgreaterthan)
//swap on file if not yet and if size greater than ifgreaterthan
{
   long nb=0;
   bool ok;
   QHashIterator<QString,CVWtab*> it(this->mestab);
   while ( it.hasNext() ) {
     it.next();
     if (it.key().contains(rxp)) {
        nb++;
        if ((it.value()->size>0)&&(it.value()->size>ifgreaterthan)){
           if (verbose>3)
              std::cout<<"SwapOutOfMemory_key_mesh_wrap on demand "<<
                   it.key().toLatin1().constData()<<
                   " size "<<it.value()->size<<">"<<ifgreaterthan<<std::endl;
           //free memory
           ok=SwapOnFile(it.key(),this->path,it.value(),this->verbose);
       }
     }
   }
   return nb;
}
//************************************
bool ghs3dprl_mesh_wrap::list_onekey_mesh_wrap(const QString &key)
{
   CVWtab *montab=this->mestab[key];
   if (montab){
      //std::cout<<"key "<<key<<"trouvee -> size="<<montab->size<<std::endl;
      if (montab->type==1)
         for ( long i=0; i<montab->size; i++ )
            std::cout<<montab->tmint[i]<<" ";
      if (montab->type==2)
         for ( long i=0; i<montab->size; i++ )
            std::cout<<montab->tmflo[i]<<" ";
      std::cout<<std::endl;
   }
   else
      std::cout<<"key "<<key.toLatin1().constData()<<" not found"<<std::endl;
   return true;
}

//************************************
bool ghs3dprl_mesh_wrap::insert_key(const QString &key,CVWtab *tab)
//insertion conditionnee par limite this->nbelem_limit_swap
//si tableaux contenus on dimension superieure
//alors swap disque dans getenv(tmp) fichier temporaire binaire
{
   bool ok;
   if (verbose>4)
      std::cout<<"insert key "<<key.toLatin1().constData()<<
            " size="<<tab->size<<std::endl;
   tab->filename="_NO_FILE";
   if (this->nbelem_limit_swap<tab->size) {
      if (verbose>3) std::cout<<"insert key automatic SwapOnFile "<<
                           key.toLatin1().constData()<<std::endl;
      ok=SwapOnFile(key,this->path,tab,this->verbose);
   }
   this->mestab.insert(key,tab);
   return true;
}
//************************************
CVWtab* ghs3dprl_mesh_wrap::restore_key(const QString &key)
//retauration conditionnee par limite nbelem
//si tableaux contenus on dimension superieure a nbelem
//alors swap disque dans getenv(tmp) fichier temporaire
//alors lecture du fichier (et reallocate memory)
{
   CVWtab *tab=NULL;
   tab=this->mestab[key];
   /*if (tab) std::cout<<" -> size in proc "<<tab->size<<std::endl;
   else std::cout<<" -> tab NULL\n";*/
   if (!tab) //it is NOT a problem
   {
      if (verbose>6) std::cout<<"restore key not found "<<key.toLatin1().constData()<<std::endl;
      return NULL;
   }
   if (tab->size > 0){
      if (verbose>5) std::cout<<"restore key direct from memory "<<key.toLatin1().constData()<<" size="<<tab->size<<std::endl;
      return tab;
   }
   //restore from binary file
   if ((tab->type<1)||(tab->type>2)){
      std::cerr<<"Problem restore key from binary file "<<tab->filename.toLatin1().constData()<<
               " type unexpexted "<<tab->type<<std::endl;
      return NULL;
   }
   //std::cout<<"restore_key from binary file "<<tab->filename<<std::endl;

   //swap disque binaire
   FILE *fichier=fopen(tab->filename.toLatin1().constData(),"rb");
   long taille;
   fread(&taille,sizeof(long),1,fichier);
   if (taille!=-tab->size){
      std::cerr<<"Problem restore_key from binary file "<<tab->filename.toLatin1().constData()<<
            " size unexpexted "<<taille<<" expected "<<-tab->size<<std::endl;
      fclose(fichier);
      return NULL;
   }
   if (tab->type==1){
      if (verbose>5)
      std::cout<<"restore key from binary file "<<tab->filename.toLatin1().constData()<<
            " number of elements "<<taille<<
            " size_element med_float "<<sizeof(med_float)<<
            " total_size_binary " <<taille*sizeof(med_float)<<std::endl;

      //allocate because swap disque binaire
      tab->tmint=new med_int[taille]; //allocate memory
      fread(tab->tmint,sizeof(med_int),taille,fichier);
   }
   if (tab->type==2){
      if (verbose>5)
      std::cout<<"restore key from binary file "<<tab->filename.toLatin1().constData()<<
            " number of elements "<<taille<<
            " size_element med_float "<<sizeof(med_float)<<
            " total_size_binary " <<taille*sizeof(med_float)<<std::endl;
      //allocate because swap disque binaire
      tab->tmflo=new med_float[taille]; //allocate memory
      for (int i=0; i<taille ; i++) tab->tmflo[i]=-1e0;
      fread(tab->tmflo,sizeof(med_float),taille,fichier);
      /*for (int i=0; i<taille ; i++) std::cout<<tab->tmflo[i]<<"/";
      std::cout<<std::endl;*/
   }
   fclose(fichier);
   tab->size=-tab->size;
   return tab;
}

//************************************
bool ghs3dprl_mesh_wrap::test_msg_wrap()
//tests sur resultats fichiers msg
{
   QString key1,key2,typ="FA VE ED EL"; //pour faces vertice edges elements
   CVWtab *tab1,*tab2;
   bool ok=true;
   //test send=receive
   //numerotations locales sont identiques
   long nb=typ.count(' ',Qt::CaseSensitive) + 1; //nb chiffres detectes
   for (long i=0; i < nb; i++)
   for (long ifile=1; ifile <= this->nbfiles; ifile++)
   for (long ineig=1; ineig <= this->nbfiles; ineig++)
   {
      if (ifile==ineig) continue; //impossible
      key1=key1.sprintf("MS%ld NE%ld ",ifile,ineig)+typ.section(' ',i,i)+" SE";
      key2=key2.sprintf("MS%ld NE%ld ",ifile,ineig)+typ.section(' ',i,i)+" RE";
      //std::cout<<"key "<<key1<<" et key "<<key2<<std::endl;
      tab1=this->restore_key(key1);
      //tab1=this->mestab[key1];
      tab2=this->restore_key(key2);
      //tab2=this->mestab[key2];
      //std::cout<<"sortie key "<<key1<<" et key "<<key2<<std::endl;
      if (!tab1 && !tab2) continue; //case not neighbours
      if (!tab1)
      {  std::cout<<"key "<<key1.toLatin1().constData()<<" inexistante avec key "<<key2.toLatin1().constData()<<" existante"<<std::endl;
         ok=false;
      }
      else
      {
       if (!tab2)
       {  std::cout<<"key "<<key2.toLatin1().constData()<<" inexistante avec key "<<key1.toLatin1().constData()<<" existante"<<std::endl;
          ok=false;
       }
       else
        if (!tab1->is_equal(tab2))
        {  std::cout<<"key "<<key1.toLatin1().constData()<<" et key "<<key2.toLatin1().constData()<<" de contenu differents"<<std::endl;
           ok=false;
        }
      }
      /*else
         printf("key '%s' et key '%s' identiques \n",
                           (const char *)key2,(const char *)key1);*/
   }

   //test size neighbourg=ifile
   //numerotations locales sont differentes mais de tailles identiques
   //pas besoin de verifier " RE " car deja fait au dessus
   for (long i=0; i < nb; i++)
   for (long ifile=1; ifile <= this->nbfiles; ifile++)
   for (long ineig=ifile+1; ineig <= this->nbfiles; ineig++)
   {
      if (ifile==ineig) continue; //cas impossible
      key1=key1.sprintf("MS%ld NE%ld ",ifile,ineig)+typ.section(' ',i,i)+" SE";
      tab1=this->restore_key(key1); //tab1=this->mestab[key1];
      key2=key2.sprintf("MS%ld NE%ld ",ineig,ifile)+typ.section(' ',i,i)+" SE";
      tab2=this->restore_key(key2); //tab2=this->mestab[key2];
      if (!tab1 && !tab2) continue; //case not neighbours
      if (!tab1)
      {  std::cout<<"key "<<key1.toLatin1().constData()<<" inexistante avec key "<<key2.toLatin1().constData()<<" existante"<<std::endl;
         ok=false;
      }
      else
      {
       if (!tab2)
       {  std::cout<<"key "<<key2.toLatin1().constData()<<" inexistante avec key "<<key1.toLatin1().constData()<<" existante"<<std::endl;
          ok=false;
       }
       else
        if ((tab1->type!=tab2->type)||(tab1->size!=tab2->size))
        {  std::cout<<"key "<<key1.toLatin1().constData()<<" et key "<<key2.toLatin1().constData()<<" de type ou tailles differents"<<std::endl;
           ok=false;
        }
      }
   }
   return ok;
}

//************************************
bool ghs3dprl_mesh_wrap::test_vertices_wrap()
//tests sur vertices
{
   QString key1,key2,key11,key22,key11old,key22old;
   CVWtab *tab1,*tab2,*tab11,*tab22;
   bool ok=true;
   key11old="_NO_KEY";key22old="_NO_KEY";
   //test size neighbourg=ifile
   //numerotations locales sont differentes mais de tailles identiques
   //pas besoin de verifier " RE " car deja fait au dessus
   //for (int ifile=1; ifile <= this->nbfiles; ifile++)
   //for (int ineig=ifile+1; ineig <= this->nbfiles; ineig++)
   bool swap=false;
   for (int ifile=this->nbfiles; ifile >= 1; ifile--)
   for (int ineig=this->nbfiles; ineig >= ifile+1; ineig--)
   {
      if (ifile==ineig) continue; //cas impossible
      key1=key1.sprintf("MS%d NE%d VE SE",ifile,ineig);
      key11=key11.sprintf("NB%d VC",ifile);
      tab1=this->restore_key(key1); //tab1=this->mestab[key1];
      key2=key2.sprintf("MS%d NE%d VE SE",ineig,ifile);
      key22=key22.sprintf("NB%d VC",ineig);
      tab2=this->restore_key(key2); //tab2=this->mestab[key2];
      if (!tab1 && !tab2) continue; //cas non voisins
      if (!tab1)
      {
         std::cerr<<"TestEqualityCoordinates key "<<key1.toLatin1().constData()<<
               " NOT existing but key "<<key2.toLatin1().constData()<<" existing"<<std::endl;
         ok=false; continue;
      }
      if (!tab2)
      {
         std::cerr<<"TestEqualityCoordinates key "<<key2.toLatin1().constData()<<
               " NOT existing but key "<<key1.toLatin1().constData()<<" existing"<<std::endl;
         ok=false; continue;
      }
      if (tab1->size!=tab2->size)
      {
         std::cerr<<"TestEqualityCoordinates key "<<key1.toLatin1().constData()<<
               " and key "<<key2.toLatin1().constData()<<" NOT same size"<<std::endl;
         ok=false; continue;
      }
      if (ok)
      {
         if (swap) {
            //Swap out of memory if no use
            if ((key11old!=key11)&&(key11old!=key22))
               this->SwapOutOfMemory_key_mesh_wrap(QRegExp(key11old,Qt::CaseSensitive,QRegExp::RegExp));
            if ((key22old!=key11)&&(key22old!=key22))
               this->SwapOutOfMemory_key_mesh_wrap(QRegExp(key22old,Qt::CaseSensitive,QRegExp::RegExp));
         }
         tab11=this->restore_key(key11); //tab11=this->mestab[key11];
         tab22=this->restore_key(key22); //tab22=this->mestab[key22];
         if (tab11->size>this->nbelem_limit_swap ||
             tab22->size>this->nbelem_limit_swap) swap=true ;
         long i1,i2;
         bool ok1=true;
         //test on equality of xyz_coordinates of commons vertices
         for  (long j=0; j < tab1->size-1; j++)
         {
            i1=tab1->tmint[j];
            i2=tab2->tmint[j];
            //1 for print vertices not equals
            if (!CVW_is_equal_vertices(tab11,i1,tab22,i2,1))
            {
               std::cerr<<j<<" Vertice "<<i1<<" != Vertice "<<i2<<"\n"<<std::endl;
               ok=false; ok1=false;
            }
         }
         if ((verbose>2)&&(ok1))
            std::cout<<"TestEqualityCoordinates "<<tab1->size<<
                  " Vertices "<<key1.toLatin1().constData()<<" and "<<key2.toLatin1().constData()<<" ok"<<std::endl;
         if (!ok1)
            std::cerr<<"TestEqualityCoordinates "<<tab1->size<<
                  " Vertices "<<key1.toLatin1().constData()<<" and "<<key2.toLatin1().constData()<<" NO_OK"<<std::endl;
         key11old=key11; key22old=key22;
      }
   }
   //Swap out of memory (supposed no use?)
   //NO because NB1&NB2 VC supposed future use
   //YES precaution
   if (swap) {
      this->SwapOutOfMemory_key_mesh_wrap(QRegExp(key11old,Qt::CaseSensitive,QRegExp::RegExp));
      this->SwapOutOfMemory_key_mesh_wrap(QRegExp(key22old,Qt::CaseSensitive,QRegExp::RegExp));
   }
   return ok;
}

//************************************
bool ghs3dprl_mesh_wrap::Find_VerticesDomainToVerticesSkin()
//initialise correspondances vertice skin et vertices locaux pour chaque domaine
//calcule un med_int new tab[nb_vertices_of_domain]
//avec nieme vertice of skin=tab[ieme vertice de domain]
//apres verification tepal garde bien dans la global numbering "GLi VE"
//les indices initiaux des noeuds (attention: de 1 a nbNodes) 
{
   QString key1,key2,tmp;
   CVWtab *cooskin,*coodom,*glodom,*montab;
   bool ok=true;
   med_float *p1,*p2;
   med_int i,nb,jd,js;

   cooskin=this->restore_key(QString("SKIN_VERTICES_COORDINATES"));
   if (!cooskin) return false;
   if (verbose>4)std::cout<<"NumberVerticesSKIN="<<cooskin->size/3<<std::endl;
   //ici pourrait creer BBtree sur skin
   for (int ifile=1; ifile<=this->nbfiles; ifile++)
   {
      key1=key1.sprintf("NB%ld VC",ifile);
      coodom=this->restore_key(key1);
      if (!coodom) continue; //Problem
      key2=key2.sprintf("GL%ld VE",ifile);
      glodom=this->restore_key(key2);
      if (verbose>4)
         std::cout<<"NumberVerticesDOMAIN_"<<ifile<<"="<<glodom->size<<std::endl;
      if (coodom->size!=glodom->size*3)
      {
         std::cerr<<"Find_VerticesDomainToVerticesSkin key "<<key1.toLatin1().constData()<<
               " and key "<<key2.toLatin1().constData()<<" NOT coherent sizes"<<std::endl;
         ok=false; continue;
      }
      //test on equality of xyz_coordinates of commons vertices
      med_int *tab=new med_int[glodom->size];
      i=0;
      nb=0; //nb equals vertices
    if (verbose>8){
      std::cout<<"\nglobal numbering nodes: no iglo\n";
      for  (jd=0; jd < glodom->size; jd++) 
           std::cout<<"\t"<<jd<<"\t"<<glodom->tmint[jd]<<std::endl;
      std::cout<<"\nresults: no i js iglo\n";
      for  (jd=0; jd < coodom->size; jd=jd+3)
      {
         p2=(coodom->tmflo+jd);
         tab[i]=0;
         //ici pourrait utiliser BBtree
         for  (js=0; js < cooskin->size; js=js+3)
         {
            p1=(cooskin->tmflo+js);
            if (p1[0]==p2[0] && p1[1]==p2[1] && p1[2]==p2[2]) 
            {
               std::cout<<"\t"<<nb<<"\t"<<i<<"\t"<<js/3<<"\t"<<glodom->tmint[i]-1<<
                 key2.sprintf("\t%13.5e%13.5e%13.5e",p1[0],p1[1],p1[2]).toLatin1().constData()<<std::endl;
               tab[i]=js/3; nb++; continue;
            }
         }
         i++;
      }
      montab=new CVWtab(glodom->size,tab);
      tmp=tmp.sprintf("NB%ld GL_SKIN",ifile);
      ok=this->insert_key(tmp,montab);
      if (verbose>4){
         std::cout<<"NumberOfEqualsVerticesDOMAIN_"<<ifile<<"="<<nb<<std::endl;
      }
    }
   }
   return ok;
}

//fin utils procedures

//************************************
bool ghs3dprl_mesh_wrap::Write_masterxmlMEDfile()
{
   QString tmp;

   //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!first call
   if (idom==1)
   {
   //define master file (.xml) in memory
   tmp=path+medname+".xml";
   filemaster=tmp.toLatin1().constData();
   domainname=medname.toLatin1().constData();
   char buff[256];

   //Creating the XML document
   master_doc = xmlNewDoc(BAD_CAST "1.0");
   root_node = xmlNewNode(0, BAD_CAST "root");
   xmlDocSetRootElement(master_doc,root_node);

   //Creating child nodes
   //Version tag
   med_int majeur,mineur,release;
   //Quelle version de MED est utilisee
   MEDlibraryNumVersion(&majeur,&mineur,&release);
   if (verbose>0) fprintf(stdout,"Files write with MED V%d.%d.%d\n",majeur,mineur,release);
   node = xmlNewChild(root_node, 0, BAD_CAST "version",0);
   //xmlNewProp(node, BAD_CAST "maj", BAD_CAST int2string2(majeur).c_str());
   xmlNewProp(node, BAD_CAST "maj", BAD_CAST i2a(majeur).c_str());
   xmlNewProp(node, BAD_CAST "min", BAD_CAST i2a(mineur).c_str());
   xmlNewProp(node, BAD_CAST "ver", BAD_CAST i2a(release).c_str());

   //Description tag
   node = xmlNewChild(root_node,0, BAD_CAST "description",0);
   xmlNewProp(node, BAD_CAST "what", BAD_CAST "tetrahedral mesh by MeshGems-Tetra-hpc (formerly tepal)");
#ifdef WIN32
  SYSTEMTIME  present;
  GetLocalTime ( &present );
  sprintf(buff,"%04d/%02d/%02d %02dh%02dm",
          present.wYear,present.wMonth,present.wDay,
          present.wHour,present.wMinute);
#else
   time_t present;
   time(&present);
   struct tm *time_asc = localtime(&present);
   sprintf(buff,"%04d/%02d/%02d %02dh%02dm",
           time_asc->tm_year+1900,time_asc->tm_mon+1,time_asc->tm_mday,
           time_asc->tm_hour,time_asc->tm_min);
#endif
   xmlNewProp(node, BAD_CAST "when", BAD_CAST buff);
   xmlNewProp(node, BAD_CAST "from", BAD_CAST "tepal2med");

   //Content tag
   node =xmlNewChild(root_node,0, BAD_CAST "content",0);
   node2 = xmlNewChild(node, 0, BAD_CAST "mesh",0);
   xmlNewProp(node2, BAD_CAST "name", BAD_CAST domainname.c_str());
   info_node = xmlNewChild(node, 0, BAD_CAST "tepal2med_info",0);

   //Splitting tag
   node=xmlNewChild(root_node,0,BAD_CAST "splitting",0);
   node2=xmlNewChild(node,0,BAD_CAST "subdomain",0);
   xmlNewProp(node2, BAD_CAST "number", BAD_CAST i2a(nbfilestot).c_str());
   node2=xmlNewChild(node,0,BAD_CAST "global_numbering",0);
   xmlNewProp(node2, BAD_CAST "present", BAD_CAST "yes");

   //Files tag
   files_node=xmlNewChild(root_node,0,BAD_CAST "files",0);

   //Mapping tag
   node = xmlNewChild(root_node,0,BAD_CAST "mapping",0);
   mesh_node = xmlNewChild(node, 0, BAD_CAST "mesh",0);
   xmlNewProp(mesh_node, BAD_CAST "name", BAD_CAST domainname.c_str());
   }

   //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!all calls
   {
   char *hostname = getenv("HOSTNAME");
   node = xmlNewChild(files_node,0,BAD_CAST "subfile",0);
   xmlNewProp(node, BAD_CAST "id", BAD_CAST i2a(idom).c_str());
   node2 = xmlNewChild(node, 0, BAD_CAST "name", BAD_CAST distfilename);
   if (hostname == NULL)
      node2 = xmlNewChild(node, 0, BAD_CAST "machine",BAD_CAST "localhost");
   else
      node2 = xmlNewChild(node, 0, BAD_CAST "machine",BAD_CAST hostname);

   node = xmlNewChild(mesh_node,0,BAD_CAST "chunk",0);
   xmlNewProp(node, BAD_CAST "subdomain", BAD_CAST i2a(idom).c_str());
   node2 = xmlNewChild(node, 0, BAD_CAST "name", BAD_CAST nomfinal);

   //tepal2med_info
   node = xmlNewChild(info_node, 0, BAD_CAST "chunk",0);
   xmlNewProp(node, BAD_CAST "subdomain", BAD_CAST i2a(idom).c_str());
   xmlNewProp(node, BAD_CAST "nodes_number", BAD_CAST i2a(nbnodes).c_str());
   xmlNewProp(node, BAD_CAST "faces_number", BAD_CAST i2a(nbtria3).c_str());
   xmlNewProp(node, BAD_CAST "tetrahedra_number", BAD_CAST i2a(nbtetra4).c_str());
   //node2 = xmlNewChild(node, 0, BAD_CAST "name", BAD_CAST nomfinal);

   //node2 = xmlNewChild(node, 0, BAD_CAST "nodes", 0);
   //xmlNewProp(node2, BAD_CAST "number", BAD_CAST i2a(nbnodes).c_str());
   //node2 = xmlNewChild(node, 0, BAD_CAST "faces", 0);
   //xmlNewProp(node2, BAD_CAST "number", BAD_CAST i2a(nbtria3).c_str());
   //node2 = xmlNewChild(node, 0, BAD_CAST "tetrahedra", 0);
   //xmlNewProp(node2, BAD_CAST "number", BAD_CAST i2a(nbtetra4).c_str());

   //tepal2med_info about joints of one subdomain
   xmlAddChild(node,joints_node);
   //tepal2med_info about groups and families of one subdomain
   xmlAddChild(node,families.xml_groups());
   }

   //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!last call
   if (idom==nbfilestot)
   {
   node2 = xmlNewChild(info_node, 0, BAD_CAST "global",0);
   xmlNewProp(node2, BAD_CAST "tetrahedra_number", BAD_CAST i2a(nbtetrastotal).c_str());
   //save masterfile
   xmlSaveFormatFileEnc(filemaster.c_str(), master_doc, "UTF-8", 1);
   xmlFreeDoc(master_doc);
   xmlCleanupParser();
   }
   return true;
}


//************************************
bool ghs3dprl_mesh_wrap::Write_MEDfiles_v2(bool deletekeys)
//deletekeys=true to delete non utils keys and arrays "au fur et a mesure"
{
   bool ok=true,oktmp;
   QString tmp,cmd;
   char description[MED_COMMENT_SIZE];
   char dtunit[MED_SNAME_SIZE+1]="_NO_UNIT";
   char axisname[MED_SNAME_SIZE*3+1]="x               y               z               ";
   char axisunit[MED_SNAME_SIZE*3+1]="_NO_UNIT        _NO_UNIT        _NO_UNIT        ";
   med_int nb;
   
   //remove path
   //precaution because casename->med_nomfinal no more 32 character
   //if path, in this->path.
   //20 preserve for add postfixes "_idom" etc...
   if (verbose>0)std::cout<<"\nWrite_MEDfiles_v2\n";
   if (verbose>6){std::cout<<"\nInitialFamilies\n"; families.write();}

   medname=medname.section('/',-1);
   if (medname.length()>20) {
      std::cerr<<"CaseNameMed truncated (no more 20 characters)"<<std::endl;
      medname.truncate(20);
   }

   //create file resume DOMAIN.joints.med of all joints for quick display (...may be...)
   tmp=path+medname+tmp.sprintf("_joints.med",idom);
   charendnull(distfilename,tmp,MED_COMMENT_SIZE);
   fidjoint=MEDfileOpen(distfilename,MED_ACC_CREAT);
   if (fidjoint<0) std::cerr<<"Problem MEDfileOpen "<<distfilename<<std::endl;
   if (verbose>0) std::cout<<"CreateMEDFile for all joints <"<<distfilename<<">\n";

   //copy file source/GHS3DPRL_skin.med as destination/DOMAIN.skin.med
   tmp=path+medname+"_skin.med";
   cmd=pathini+casename+"_skin.med";
   int ret = access(cmd.toLatin1().constData(),F_OK); //on regarde si le fichier existe
   if (ret >= 0) {
#ifdef WIN32
      cmd="copy ";
#else 
      cmd="cp ";
#endif
      cmd = cmd+pathini+casename+"_skin.med "+tmp;
      //std::cout<<"Copy skin.med Command = "<<cmd<<std::endl;
      system(cmd.toLatin1().constData()); 
      if (verbose>0) std::cout<<"CreateMEDFile for initial skin <"<<tmp.toLatin1().constData()<<">\n"; }
   else {
      if (verbose>0) std::cout<<"No CreateMEDFile <"<<tmp.toLatin1().constData()<<"> for initial skin because <"<<
                                                cmd.toLatin1().constData()<<"> does not exist\n"; }

   //define family 0 if not existing, no groups
   //La famille FAMILLE_ZERO n'a pas été trouvée, elle est obligatoire
   families.add("0","FAMILLE_ZERO");
   //define family Group_of_New_Nodes (which not exists before tetrahedra)
   famallnodes=0;
   if (QString("All_Nodes").contains(deletegroups)==0){
      oktmp=families.get_number_of_new_family(1,&famallnodes,&tmp);
      families.add(tmp,"All_Nodes");
   }
   else if (verbose>3) std::cout<<"--deletegroups matches \"All_Nodes\"\n";
   
   famalltria3=0;
   if (QString("All_Faces").contains(deletegroups)==0){
      oktmp=families.get_number_of_new_family(-1,&famalltria3,&tmp);
      families.add(tmp,"All_Faces");
   }
   else if (verbose>3) std::cout<<"--deletegroups matches \"All_Faces\"\n";

   famalltetra4=0;
   if (QString("All_Tetrahedra").contains(deletegroups)==0){
      oktmp=families.get_number_of_new_family(-1,&famalltetra4,&tmp);
      families.add(tmp,"All_Tetrahedra");
   }
   else if (verbose>3) std::cout<<"--deletegroups matches \"All_Tetrahedra\"\n";

   famnewnodes=0;
   if (QString("New_Nodes").contains(deletegroups)==0){
      oktmp=families.get_number_of_new_family(1,&famnewnodes,&tmp);
      families.add(tmp,"New_Nodes");
   }
   else if (verbose>3) std::cout<<"--deletegroups matches \"New_Nodes\"\n";
   
   famnewtria3=0;
   if (QString("New_Faces").contains(deletegroups)==0){
      oktmp=families.get_number_of_new_family(-1,&famnewtria3,&tmp);
      families.add(tmp,"New_Faces");
   }
   else if (verbose>3) std::cout<<"--deletegroups matches \"New_Faces\"\n";
   
   famnewtetra4=0;
   if (QString("New_Tetrahedra").contains(deletegroups)==0){
      oktmp=families.get_number_of_new_family(-1,&famnewtetra4,&tmp);
      families.add(tmp,"New_Tetrahedra");
   }
   else if (verbose>3) std::cout<<"--deletegroups matches \"New_Tetrahedra\"\n";

   if (verbose>6){std::cout<<"\nIntermediatesFamilies\n"; families.write();}
   //if (verbose>6) std::cout<<"\nNumber0fFiles="<<nbfilestot<<std::endl;
   familles intermediatesfamilies=families;
   //initialisations on all domains
   nbtetrastotal=0;

   //loop on the domains
   //for (idom=1; idom<=nbfilestot; idom++) {
   if (for_multithread) {
     nbfilestot=1;
     std::cout<<"\nset Number0fFiles as multithread="<<nbfilestot<<std::endl;
   }
   if (verbose>6) std::cout<<"\nNumber0fFiles="<<nbfilestot<<std::endl;
   for (idom=1; idom<=nbfilestot; idom++) {
   
      this->nofile=idom;
      //restore initial context of families
      if (idom>1) families=intermediatesfamilies;
      //if (idom>1) continue;
      tmp=path+medname+tmp.sprintf("_%d.med",idom);
      charendnull(distfilename,tmp,MED_COMMENT_SIZE);

      //std::cout<<"<"<<distfilename<<">"<<std::endl;
      fid=MEDfileOpen(distfilename,MED_ACC_CREAT);
      if (fid<0) {std::cerr<<"Problem MEDfileOpen "<<distfilename<<std::endl; goto erreur;}
      if (verbose>0){
         if (verbose>2) std::cout<<std::endl;
         std::cout<<"CreateMEDFile "<<idom<<" <"<<distfilename<<">\n";
      }
 
      //create mesh
      tmp=medname+tmp.sprintf("_%d",idom);
      charendnull(nomfinal,tmp,MED_NAME_SIZE);
      tmp=tmp.sprintf("domain %d among %d",idom,nbfilestot);
      charendnull(description,tmp,MED_COMMENT_SIZE);

      if (verbose>4) std::cout<<"Description : "<<description<<std::endl;
      err=MEDmeshCr(fid,nomfinal,3,3,MED_UNSTRUCTURED_MESH,description,dtunit,MED_SORT_DTIT,MED_CARTESIAN,axisname,axisunit);
      if (err<0) {std::cerr<<"Problem MEDmeshCr"<<nomfinal<<std::endl; goto erreur;}

      if (!idom_nodes()) {std::cerr<<"Problem on Nodes"<<std::endl; goto erreur;}
      if (verbose>4) std::cout<<"\nEnd of Nodes ***\n"<<std::endl;
      if (!idom_edges()) {std::cerr<<"Problem on Edges"<<std::endl; goto erreur;}
      if (verbose>4) std::cout<<"\nEnd of Edges ***\n"<<std::endl;
      if (!idom_faces()) {std::cerr<<"Problem on Faces"<<std::endl; goto erreur;}
      if (verbose>4) std::cout<<"\nEnd of Faces ***\n"<<std::endl;
      if (!idom_tetras()) {std::cerr<<"Problem on tetrahedra"<<std::endl; goto erreur;}
      if (verbose>4) std::cout<<"\nEnd of Tetrahedra ***\n"<<std::endl;
      //non existing files msg mh-tetra_hpc v2.1.11
      //if (!idom_joints()) {std::cerr<<"Problem on Joints"<<std::endl; goto erreur;}

      if (!for_multithread) {
        if (verbose>6){std::cout<<"\nFinalsFamilies\n"; families.write();}
        //for nodes families
        nb=create_families(fid,1);
        if (verbose>5)std::cout<<"NumberOfFamiliesNodes="<<nb<<std::endl;

        err=MEDmeshEntityFamilyNumberWr(fid,nomfinal,MED_NO_DT,MED_NO_IT,MED_NODE,MED_UNDEF_GEOMETRY_TYPE,nbnodes,famnodes);
        if (verbose>8)
          std::cout<<"MEDmeshEntityFamilyNumberWr nodes "<<nbnodes<<":"<<
                famnodes[0]<<"..."<<famnodes[nbnodes-1]<<" "<<std::endl;
        delete[] famnodes;
        if (err<0) std::cerr<<"Problem MEDmeshEntityFamilyNumberWr nodes"<<std::endl;

        //for others families
        nb=create_families(fid,-1);
        if (verbose>5)std::cout<<"NumberOfFamiliesFacesAndEdgesEtc="<<nb<<std::endl;

        err=MEDmeshEntityFamilyNumberWr(fid,nomfinal,MED_NO_DT,MED_NO_IT,MED_CELL,MED_TRIA3,nbtria3,famtria3);
        if (verbose>8)
          std::cout<<"MEDmeshEntityFamilyNumberWr tria3 "<<nbtria3<<":"<<
                famtria3[0]<<"..."<<famtria3[nbtria3-1]<<" "<<std::endl;
        delete[] famtria3;
        if (err<0) std::cerr<<"Problem MEDmeshEntityFamilyNumberWr tria3"<<std::endl;

        err=MEDmeshEntityFamilyNumberWr(fid,nomfinal,MED_NO_DT,MED_NO_IT,MED_CELL,MED_TETRA4,nbtetra4,famtetra4);
        if (verbose>8)
          std::cout<<"MEDmeshEntityFamilyNumberWr tetra4 "<<nbtetra4<<":"<<
                famtetra4[0]<<"..."<<famtetra4[nbtria3-1]<<" "<<std::endl;
        delete[] famtetra4;
        if (err<0) std::cerr<<"Problem MEDmeshEntityFamilyNumberWr tria3"<<std::endl;
      }
    
      MEDfileClose(fid); //no error
      //master.xml writings
      //oktmp=Write_masterxmlMEDfile();
      continue;       //and loop on others domains

      erreur:         //error
      ok=false;
      MEDfileClose(fid); //but loop on others domains

   }
   
   MEDfileClose(fidjoint); //no error
   if (verbose>0)std::cout<<"\nTotalNumberOftetrahedra="<<nbtetrastotal<<std::endl;
   ok = true;
   
   return ok;
}

//************************************
bool ghs3dprl_mesh_wrap::idom_nodes()
{
   bool ok=true;
   QString tmp,key,key1,key2,key3;
   CVWtab *tab,*tab1,*tab2,*tab3;
   med_int i,j,*arrayi;
   int xx;

      //writing node(vertices) coordinates
      //NBx VC=files.NoBoite Vertex Coordinates
      key=key.sprintf("NB%d VC",idom); //files.NoBoite Vertex Coordinates
      tab=this->restore_key(key); //tab1=this->mestab[key1];
      if (!tab) {
         if (!for_tetrahpc) {
            tmp=pathini+casename+tmp.sprintf(format.toLatin1().constData(),nbfilestot,idom)+".noboite";
            ok=this->ReadFileNOBOITE(tmp);
         }
         if (for_tetrahpc) {
            tmp=pathini+casename+"_out"+tmp.sprintf(format_tetra.toLatin1().constData(),idom)+".mesh";
            ok=this->ReadFileMESH(tmp);
         }
         tab=this->restore_key(key); //tab1=this->mestab[key1];
         if (!tab) return false;
      }
      tmp=tmp.sprintf("NB%d SN",idom);
      //qt3 xx=this->remove_key_mesh_wrap(QRegExp(tmp,true,true));
      xx=this->remove_key_mesh_wrap(QRegExp(tmp,Qt::CaseSensitive,QRegExp::RegExp));
      nbnodes=tab->size/3;
      err=MEDmeshNodeCoordinateWr(fid,nomfinal,MED_NO_DT,MED_NO_IT,0.,MED_FULL_INTERLACE,nbnodes,tab->tmflo);
      if (err<0) {std::cerr<<"Problem MEDmeshNodeCoordinateWr"<<std::endl; return false;}
      if (verbose>4) std::cout<<"NumberOfNodes="<<nbnodes<<std::endl;

      //writing indices of nodes
      arrayi=new med_int[nbnodes];
      for (i=0; i<nbnodes ; i++) arrayi[i]=i+1;
      err=MEDmeshEntityNumberWr(fid,nomfinal,MED_NO_DT,MED_NO_IT,MED_NODE,MED_UNDEF_GEOMETRY_TYPE,nbnodes,arrayi);
      delete[] arrayi;
      if (err<0) {std::cerr<<"Problem MEDmeshEntityNumberWr of nodes"<<std::endl; return false;}

    if (!for_multithread) {
      key1=key1.sprintf("GL%d VE",idom); //global numerotation 
      tab1=this->restore_key(key1); //tab1=this->mestab[key1];
      if (!tab1) {
         if (!for_tetrahpc) {
            tmp=pathini+casename+tmp.sprintf(format.toLatin1().constData(),nbfilestot,idom)+".glo";
            ok=this->ReadFileGLO(tmp);
         }
         if (for_tetrahpc) {
            tmp=pathini+casename+"_out"+tmp.sprintf(format_tetra.toLatin1().constData(),idom)+".global";
            ok=this->ReadFileGLOBAL(tmp);
         }
         if (!ok) {std::cerr<<"Problem file "<<tmp.toLatin1().constData()<<std::endl; return false;}
         tab1=this->restore_key(key1); //tab1=this->mestab[key1];
         if (!tab1) return false;
      }
      if (nbnodes!=tab1->size){std::cerr<<"Problem size GLi VE!=nbnodes!"<<std::endl; return false;}

      key2=key2.sprintf("SKIN_VERTICES_FAMILIES",idom); //on global numerotation 
      tab2=this->restore_key(key2); //tab1=this->mestab[key1];
      med_int nbskin=0; 
      if (tab2) med_int nbskin=tab2->size;
      //for (i=0; i<nbskin; i++) std::cout<<i<<" "<<tab2->tmint[i]<<std::endl;

      //set families of nodes existing in GHS3DPRL_skin.med
      med_int nb=nbnodes;
      famnodes=new med_int[nb];
      for (i=0; i<nb ; i++) famnodes[i]=famallnodes;
      med_int * fammore=new med_int[nb];
      for (i=0; i<nb ; i++) fammore[i]=famnewnodes;

      //set families of nodes of skin
      for (i=0; i<nb ; i++){
         j=tab1->tmint[i]-1; //
         if (j<nbskin){
            fammore[i]=tab2->tmint[j];
         }
      }
      ok=set_one_more_family(famnodes,fammore,nb);
      delete[] fammore;

      //std::cout<<"nodes loc "<<i<<" = gl "<<j<<"\t << "<<tab2->tmint[j]<<
      //      tmp.sprintf("\t%23.15e%23.15e%23.15e",tab3->tmflo[i*3],
      //      tab3->tmflo[i*3+1],tab3->tmflo[i*3+2])<<std::endl;

      //writing nodes(vertices) global numbering
      err=MEDmeshGlobalNumberWr(fid,nomfinal,MED_NO_DT,MED_NO_IT,MED_NODE,MED_UNDEF_GEOMETRY_TYPE,nbnodes,tab1->tmint);
      if (err<0){std::cerr<<"Problem MEDmeshGlobalNumberWr nodes"<<std::endl; return false;}
    }
    
   return ok;
}

/*
//************************************
bool ghs3dprl_mesh_wrap::set_one_more_family_old(med_int *fami, med_int *more, med_int nb)
//fuse array of med_int families more et fami as kind of groups 
//because there are possibilities of intersections
{
   QString tmp;
   med_int i,newfam,morfam,oldfam;
   for (i=0; i<nb ; i++) {
      if (more[i]==0) continue;
      if (fami[i]==0) {
         fami[i]=more[i];
         //std::cout<<"sur "<<i<<" en plus "<<more[i]<<std::endl;
      }
      else { //intersection
         if (fami[i]==more[i]) continue; //same families
         oldfam=fami[i];
         morfam=more[i];
         //create new family intersection if needed
         newfam=families.find_family_on_groups(oldfam,morfam);
         //std::cout<<"oldfam "<<oldfam<<" morfam "<<morfam<<" -> newfam "<<newfam<<std::endl;
         fami[i]=newfam;
      }
   }
   return true;
}*/

//************************************
bool ghs3dprl_mesh_wrap::set_one_more_family(med_int *fami, med_int *more, med_int nb)
//fuse array of med_int families more et fami as kind of groups 
//because there are possibilities of intersections
{
   QString tmp;
   med_int i,ii,j,newfam,morfam,oldfam,morfami,oldfami,i_zero,nb_fam,nb_max,nb_tot,nb_mess;
   med_int *newfami;

   nb_fam=families.fam.size(); //on families negative and positive
   //std::cout<<"size families "<<nb_fam<<std::endl;
   if (nb_fam<=0) nb_fam=5;    //precaution
   i_zero=nb_fam*2;            //offset for negative indices of families
   nb_max=nb_fam*4;
   if (nb_fam>300) std::cout<<
      "***set_one_more_family*** warning many initial families could decrease speed "<<nb_fam<<std::endl;
   nb_tot=nb_max*nb_max;       //max oversizing *2 on families
   //newfami is for speed (avoid calls find_family_on_groups)
   //it is an array[nb_fam*4][nb_fam*4] implemented on vector[nb_max]
   //to memorize newfam in array[oldfam][morfam]
   newfami=new med_int[nb_tot];
   for (i=0; i<nb_tot ; i++) newfami[i]=0; //not yet met!

   nb_mess=0;
   for (i=0; i<nb ; i++) {
      if (more[i]==0) continue;
      if (fami[i]==0) {
         fami[i]=more[i];
         //std::cout<<"sur "<<i<<" en plus "<<more[i]<<std::endl;
      }
      else { //intersection
         if (fami[i]==more[i]) continue; //same families
         oldfam=fami[i]; oldfami=oldfam+i_zero;
         morfam=more[i]; morfami=morfam+i_zero;
         //not yet met?
         ii=oldfami+morfami*nb_max; //array 2d on vector
         if ((ii>=0)&&(ii<nb_tot)) {
            newfam=newfami[ii];
         }
         else {
            if (nb_mess<3) {
               nb_mess++;
               std::cout<<"***set_one_more_family*** warning many new families decrease speed "<<nb_fam<<std::endl;
            }
            ii=-1;
            newfam=0;
         }
         if (newfam==0) {
            //create new family intersection if needed
            newfam=families.find_family_on_groups(oldfam,morfam);
            //std::cout<<"new oldfam "<<oldfam<<" morfam "<<morfam<<" -> newfam "<<newfam<<std::endl;
            if (ii>=0) newfami[ii]=newfam;
         }
         /*else {
            std::cout<<"!!! oldfam "<<oldfam<<" morfam "<<morfam<<" -> newfam "<<newfam<<std::endl;
         }*/
         fami[i]=newfam;
      }
   }
   delete[] newfami;
   return true;
}

//************************************
bool ghs3dprl_mesh_wrap::idom_edges()
{
   bool ok=true;
   QString tmp;
   nbseg2=0;
   return ok;
}

//************************************
bool ghs3dprl_mesh_wrap::idom_faces()
{
   bool ok=true;
   QString tmp,key,key1,key2,key3;
   CVWtab *tab,*tab1,*tab2,*tab3;
   med_int ii,i,j,*arrayi;
   int xx;

      //writing connectivity of faces triangles of wrap by nodes
      key1=key1.sprintf("FC%d",idom); //files.FaCes faces (wrap and triangles only)
      tab1=this->restore_key(key1); //tab1=this->mestab[key1];
      if (!tab1) {
         tmp=pathini+casename+tmp.sprintf(format.toLatin1().constData(),nbfilestot,idom)+".faces";
         ok=this->ReadFileFACES(tmp);
         tab1=this->restore_key(key1);
         if (!tab1) return false;
      }
      nbtria3=tab1->size/3;
      if (verbose>4) std::cout<<"NumberOfTriangles="<<nbtria3<<std::endl;
      //arrayi=new med_int[nbtria3*3];
      //ii=0,i=0 ;
      //for (j=0; j<nbtria3 ; j++){
      //   arrayi[ii]=tab1->tmint[i]; ii++;
      //   arrayi[ii]=tab1->tmint[i+1]; ii++;
      //   arrayi[ii]=tab1->tmint[i+2]; ii++;
      //   i=i+7;
      //}
      //err=MEDmeshElementConnectivityWr(fid,nomfinal,MED_NO_DT,MED_NO_IT,0.,MED_CELL,MED_TRIA3,MED_NODAL,MED_FULL_INTERLACE,nbtria3,arrayi);
      err=MEDmeshElementConnectivityWr(fid,nomfinal,MED_NO_DT,MED_NO_IT,0.,MED_CELL,MED_TRIA3,MED_NODAL,MED_FULL_INTERLACE,nbtria3,tab1->tmint);
      //delete[] arrayi; //need immediately more little array
      if (err<0){std::cerr<<"Problem MEDmeshElementConnectivityWr for triangles connectivity"<<std::endl; return false;}
      
      //writing indices of faces triangles of wrap
      //caution!
      //generate "overlapping of numbers of elements" in "import med file" in salome
      //if not in "//writing indices of tetrahedra" -> arrayi[i]=!NBFACES!+i+1
      arrayi=new med_int[nbtria3]; 
      for (i=0; i<nbtria3 ; i++) arrayi[i]=nbseg2+i+1;
      err=MEDmeshEntityNumberWr(fid,nomfinal,MED_NO_DT,MED_NO_IT,MED_CELL,MED_TRIA3,nbtria3,arrayi);
      delete[] arrayi;
      if (err<0){std::cerr<<"Problem MEDmeshEntityNumberWr of triangles"<<std::endl; return false;}

      //GLx FA=files.GLo FAces
      if (!for_multithread) {
        key1=key1.sprintf("GL%d FA",idom);
        tab1=this->restore_key(key1); //tab1=this->mestab[key1];
        if (nbtria3!=tab1->size){std::cerr<<"Problem size GLi FA!=nbtria3!"<<std::endl; return false;}
      
      
        key2=key2.sprintf("SKIN_TRIA3_FAMILIES",idom); //on global numerotation 
        tab2=this->restore_key(key2); //tab1=this->mestab[key1];
        med_int nbskin=0; 
        if (tab2) nbskin=tab2->size;
        
        //set families of faces existing in GHS3DPRL_skin.med
        med_int nb=nbtria3;
        famtria3=new med_int[nb];
        for (i=0; i<nb ; i++) famtria3[i]=famalltria3;
        med_int * fammore=new med_int[nb];
        for (i=0; i<nb ; i++) fammore[i]=famnewtria3;

        //set families of faces of skin
        for (i=0; i<nb ; i++){
          j=tab1->tmint[i]-1; //
          if (j<nbskin){
              fammore[i]=tab2->tmint[j];
          }
        }
        ok=set_one_more_family(famtria3,fammore,nb);
        delete[] fammore;
        
        //writing faces(triangles) global numbering
        if (verbose>2)
          std::cout<<"CreateMEDglobalNumerotation_Faces "<<key1.toLatin1().constData()<<" "<<tab1->size<<std::endl;
        err=MEDmeshGlobalNumberWr(fid,nomfinal,MED_NO_DT,MED_NO_IT,MED_CELL,MED_TRIA3,tab1->size,tab1->tmint);
        if (err<0){std::cerr<<"Problem MEDmeshGlobalNumberWr faces"<<std::endl; return false;}

        //xx=this->remove_key_mesh_wrap(QRegExp("FC*",true,true));
        tmp=tmp.sprintf("GL%d FA",idom);
        //qt3 xx=this->remove_key_mesh_wrap(QRegExp(tmp,true,true));
        xx=this->remove_key_mesh_wrap(QRegExp(tmp,Qt::CaseSensitive,QRegExp::RegExp));
        tmp=tmp.sprintf("GL%d VE",idom);
        //qt3 xx=this->remove_key_mesh_wrap(QRegExp(tmp,true,true));
        xx=this->remove_key_mesh_wrap(QRegExp(tmp,Qt::CaseSensitive,QRegExp::RegExp));
      }

   return ok;
}

//************************************
bool ghs3dprl_mesh_wrap::idom_joints()
{
   bool ok=true;
   QString tmp,namejoint,key,key1,key2;
   CVWtab *tab,*tab1,*tab2;
   med_int ineig,ii,jj,i,j,k,*arrayi,nb,famjoint,*fammore,*inodes,*arrayfaces;
   med_float *arraynodes;
   char namejnt[MED_NAME_SIZE+1];  //no more 32
   char namedist[MED_NAME_SIZE+1];
   char descjnt[MED_COMMENT_SIZE+1];
   med_int numfam_ini_wrap=100;
   joints_node=xmlNewNode(NULL, BAD_CAST "joints");  //masterfile.xml
   med_int nbjoints=0,nbnodesneig,nbtria3neig;
   std::string sjoints=""; //which domains are neighbourg
   int xx;
   char dtunit[MED_SNAME_SIZE+1]="_NO_UNIT";
   char axisname[MED_SNAME_SIZE*3+1]="x               y               z               ";
   char axisunit[MED_SNAME_SIZE*3+1]="_NO_UNIT        _NO_UNIT        _NO_UNIT        ";

      tmp=tmp.sprintf("MS%d *",idom);
      //read file .msg if not done
      //qt3 if (this->nb_key_mesh_wrap(QRegExp(tmp,true,true))<=0) {
      if (this->nb_key_mesh_wrap(QRegExp(tmp,Qt::CaseSensitive,QRegExp::RegExp))<=0) {
         this->nofile=idom;
         
         if (!for_tetrahpc) {
            tmp=pathini+casename+tmp.sprintf(format.toLatin1().constData(),nbfilestot,idom)+".msg";
         }
         if (for_tetrahpc) {
            tmp=pathini+casename+tmp.sprintf(format_tetra.toLatin1().constData(),idom)+".msg";
         }

         ok=this->ReadFileMSGnew(tmp);
         if (!ok) {
            std::cerr<<"Problem in file "<<tmp.toLatin1().constData()<<std::endl;
            return false;
         }
      }

      //writing joints
      for (ineig=1; ineig <= nbfilestot; ineig++) {
         if (idom==ineig) continue; //impossible

         //!*************nodes
         //std::cout<<"\n    nodes joints\n";
         key1=key1.sprintf("MS%d NE%d VE SE",idom,ineig); //SE or RE identicals
         tab1=restore_key(key1);
         if (!tab1) continue; //case (ifile,ineig) are not neighbours=>no joints
         key1=key1.sprintf("MS%d NE%d VE SE",ineig,idom); //SE or RE identicals
         tab2=this->restore_key(key1);
         //if not yet loaded (first time) try to load necessary file msg of neighbourg
         if (!tab2) {
            
            if (!for_tetrahpc) {
               tmp=pathini+casename+tmp.sprintf(format.toLatin1().constData(),nbfilestot,ineig)+".msg";
            }
            if (for_tetrahpc) {
               tmp=pathini+casename+tmp.sprintf(format_tetra.toLatin1().constData(),ineig)+".msg";
            }
            
            this->nofile=ineig; //neighbourg file
            ok=this->ReadFileMSGnew(tmp);
            this->nofile=idom;  //restaure initial domain
            if (!ok) {
               std::cerr<<"Problem in file "<<tmp.toLatin1().constData()<<std::endl;
               continue;
            }
            tab2=this->restore_key(key1);
         }
         if (!tab2) std::cerr<<"Problem existing nodes joint in domain "<<idom<<
                          " with none in neighbourg "<<ineig<<" files .msg"<<std::endl;
         nb=tab1->size; nbnodesneig=tab2->size;
         if (nb!=nbnodesneig) {
            std::cerr<<"Problem in file "<<tmp.toLatin1().constData()<<
                  " number of nodes of joint "<<idom<<"<->"<<ineig<<" not equals"<<std::endl;
            continue;
         }

         nbjoints++; //one more joint for this domain
         sjoints=sjoints+" "+i2a(ineig);
         if (verbose>4)
            std::cout<<"NumberOfNodesOfJoint_"<<idom<<"_"<<ineig<<"="<<nb<<std::endl;
         namejoint=namejoint.sprintf("JOINT_%d_%d_Nodes",idom,ineig);
         strcpy(namejnt,namejoint.toLatin1().constData());
         tmp=tmp.sprintf("JOINT_%d_%d among %d domains of ",idom,ineig,nbfilestot)+nomfinal;
         strcpy(descjnt,tmp.toLatin1().constData());
         tmp=medname+tmp.sprintf("_%d",ineig);
         strcpy(namedist,tmp.toLatin1().constData());
         err=MEDsubdomainJointCr(fid,nomfinal,namejnt,descjnt,ineig,namedist);
         if (err<0) std::cerr<<"Problem MEDsubdomainJointCr"<<std::endl;

         famjoint=0;
         if (namejoint.contains(deletegroups)==0){
            ok=families.get_number_of_new_family(1,&famjoint,&tmp);
            families.add(tmp,namejoint);
         }

         key=key.sprintf("NB%d VC",idom); //files.NoBoite Vertex Coordinates
         tab=this->restore_key(key); //tab1=this->mestab[key1];
         //nbnodes=tab->size/3;

         //writing correspondence nodes-nodes
         //two indices for one correspondence
         arrayi=new med_int[nb*2];
         arraynodes=new med_float[nbnodesneig*3];  //for file DOMAIN_join.med
         inodes=new med_int[nbnodes];              //for file DOMAIN_join.med
         med_int * fammore=new med_int[nbnodes];
         for (i=0; i<nbnodes ; i++) {fammore[i]=0; inodes[i]=-2;}  //precautions
         ii=0; jj=0; k=1;
         for (i=0; i<nb ; i++){
            //no need because <send> equals <receive> tab1->tmint[i]==tab2->tmint[i]
            j=tab1->tmint[i]-1; //contents of tab1 1->nb, j 0->nb-1
            inodes[j]=k; k++;   //contents of inodes 1->n ,nodes of joint from nodes of domain
            arraynodes[jj]=tab->tmflo[j*3]; jj++;
            arraynodes[jj]=tab->tmflo[j*3+1]; jj++;
            arraynodes[jj]=tab->tmflo[j*3+2]; jj++;

            fammore[j]=famjoint;
            arrayi[ii]=tab1->tmint[i]; ii++;
            arrayi[ii]=tab2->tmint[i]; ii++;
         }
         if (namejoint.contains(deletegroups)==0){
            ok=set_one_more_family(famnodes,fammore,nbnodes);
         }
         delete[] fammore;

         err=MEDsubdomainCorrespondenceWr(fid,nomfinal,namejnt,MED_NO_DT,MED_NO_IT,
                 MED_NODE,MED_UNDEF_GEOMETRY_TYPE,MED_NODE,MED_UNDEF_GEOMETRY_TYPE,nb,arrayi);
         if (err<0) std::cerr<<"Problem MEDsubdomainCorrespondenceWr nodes"<<std::endl;
         delete[] arrayi;

         //!*************TRIA3
         //writing correspondence triangles-triangles
         //std::cout<<"\n    faces joints\n";
         nbtria3neig=0;
         key1=key1.sprintf("MS%d NE%d FA SE",idom,ineig); //SE or RE identicals
         tab1=this->restore_key(key1); //tab1=this->mestab[key1];
         if (!tab1){
            if (verbose>4)
               std::cout<<"NumberOfTrianglesOfJoint_"<<idom<<"_"<<ineig<<"=0"<<std::endl;
            //continue; //case (ifile,ineig) are not neighbours=>no joints
         }
         else //have to set xml may be no faces but nodes in a joint!
         {
         key1=key1.sprintf("MS%d NE%d FA SE",ineig,idom); //SE or RE identicals
         tab2=this->restore_key(key1);
         if (!tab2) std::cerr<<"Problem existing triangles of joint in domain "<<idom<<
                               " with none in neighbourg "<<ineig<<" files .msg"<<std::endl;
         nb=tab1->size; nbtria3neig=tab2->size;
         if (nb!=nbtria3neig) {
            std::cerr<<"Problem in file "<<tmp.toLatin1().constData()<<
                  " number of triangles of joint "<<idom<<"<->"<<ineig<<" not equals"<<std::endl;
            continue;
         }
         namejoint=namejoint.sprintf("JOINT_%d_%d_Faces",idom,ineig);
         
         famjoint=0;
         if (namejoint.contains(deletegroups)==0){
            ok=families.get_number_of_new_family(-1,&famjoint,&tmp);
            families.add(tmp,namejoint);
         }

         key=key.sprintf("FC%d",idom); //files.FaCes faces (wrap and triangles only)
         tab=this->restore_key(key); //tab1=this->mestab[key1];

         med_int nb=tab1->size; nbtria3neig=nb;
         //if (verbose>=0) std::cout<<"NumberOfTrianglesOfJoint_"<<idom<<"_"<<ineig<<"="<<nb<<std::endl;
         arrayi=new med_int[nb*2]; //correspondance indices triangles in 2 domains
         arrayfaces=new med_int[nbtria3neig*3];  //for file DOMAIN_join.med
         for (i=0; i<nbtria3neig*3 ; i++) arrayfaces[i]=-1; //precaution
         fammore=new med_int[nbtria3];
         for (i=0; i<nbtria3 ; i++) fammore[i]=0;
         ii=0; jj=0;
         for (i=0; i<nb ; i++){
            arrayi[ii]=tab1->tmint[i]; ii++;
            arrayi[ii]=tab2->tmint[i]; ii++; //correspondance indices triangles in 2 domains
       
            fammore[tab1->tmint[i]-1]=famjoint;
            //famtria3[tab1->tmint[i]-1]=famjoint;
            
            k=tab1->tmint[i]-1; //indice of node connectivity
            //std::cout<<"k="<<k<<std::endl;
            k=k*7; //indice of node connectivity in tab of triangles
            
            arrayfaces[jj]=inodes[tab->tmint[k]-1]; jj++;
            arrayfaces[jj]=inodes[tab->tmint[k+1]-1]; jj++;
            arrayfaces[jj]=inodes[tab->tmint[k+2]-1]; jj++;
         }
         int happens=0;
         for (i=0; i<nbtria3neig*3 ; i++) {
           if (arrayfaces[i]<=0) {
             std::cerr<<"Problem file X_joints.med unknown node in joint "<<idom<<"_"<<ineig<<" face "<<i/3+1<<std::endl; //precaution
             happens=1;
           }
         }
         /*TODO DEBUG may be bug distene?
         if (happens==1) {
            std::cout<<"\nNumberOfTrianglesOfJoint_"<<idom<<"_"<<ineig<<"="<<nb<<std::endl;
            for (i=0; i<nbnodes ; i++) std::cout<<"inode i "<<i+1<<" "<<inodes[i]<<std::endl;
            for (i=0; i<tab1->size ; i++) std::cout<<"triangle i "<<i+1<<" "<<tab1->tmint[i]<<std::endl;
            for (i=0; i<tab->size ; i=i+7) std::cout<<"conn i "<<i/7+1<<" : "<<tab->tmint[i]<<" "<<tab->tmint[i+1]<<" "<<tab->tmint[i+2]<<std::endl;
         }
         */
         if (namejoint.contains(deletegroups)==0){
            ok=set_one_more_family(famtria3,fammore,nbtria3);
         }
         delete[] fammore;

         err=MEDsubdomainCorrespondenceWr(fid,nomfinal,namejnt,MED_NO_DT,MED_NO_IT,
                 MED_CELL,MED_TRIA3,MED_CELL,MED_TRIA3,nb,arrayi);
         if (err<0) std::cerr<<"Problem MEDsubdomainCorrespondenceWr triangles"<<std::endl;
         delete[] arrayi;
         }

         //!write in file resume DOMAIN.joints.med of all joints for quick display (...may be...)
         if (idom<=ineig) { //no duplicate joint_1_2 and joint_2_1
          //create mesh
          namejoint=namejoint.sprintf("JOINT_%d_%d",idom,ineig);
          charendnull(namejnt,namejoint,MED_NAME_SIZE);
          tmp=tmp.sprintf("joint between %d and %d",idom,ineig);
          charendnull(descjnt,tmp,MED_COMMENT_SIZE);
          err=MEDmeshCr(fidjoint,namejnt,3,3,MED_UNSTRUCTURED_MESH,descjnt,dtunit,MED_SORT_DTIT,MED_CARTESIAN,axisname,axisunit);
          if (err<0) std::cerr<<"Problem MEDmeshCr "<<namejnt<<std::endl;
          //write nodes
          err=MEDmeshNodeCoordinateWr(fidjoint,namejnt,MED_NO_DT,MED_NO_IT,0.,MED_FULL_INTERLACE,nbnodesneig,arraynodes);
          if (err<0) std::cerr<<"Problem MEDmeshNodeCoordinateWr "<<namejnt<<std::endl;
          arrayi=new med_int[nbnodesneig];
          for (i=0; i<nbnodesneig ; i++) arrayi[i]=i+1;
          err=MEDmeshEntityNumberWr(fidjoint,namejnt,MED_NO_DT,MED_NO_IT,MED_NODE,MED_UNDEF_GEOMETRY_TYPE,nbnodesneig,arrayi);
          delete[] arrayi;
          if (err<0) std::cerr<<"Problem MEDmeshEntityNumberWr of nodes "<<namejnt<<std::endl;
          //families zero in file fidjoint ???
          //La famille FAMILLE_ZERO n'a pas été trouvée, elle est obligatoire
          nb=create_family_zero(fidjoint,namejnt);
          
          //write tria3
          if (nbtria3neig>0) {
           //for (i=0; i<nbtria3neig ; i++) std::cout<<i+1<<" "<<
           //    arrayfaces[i*3]<<" "<<arrayfaces[i*3+1]<<" "<<arrayfaces[i*3+2]<<std::endl;
           err=MEDmeshElementConnectivityWr(fidjoint,namejnt,MED_NO_DT,MED_NO_IT,0.,
                   MED_CELL,MED_TRIA3,MED_NODAL,MED_FULL_INTERLACE,nbtria3neig,arrayfaces);
           if (err<0) std::cerr<<"Problem MEDmeshElementConnectivityWr for triangles connectivity "<<namejnt<<std::endl;
           //writing indices of faces triangles of joint
           arrayi=new med_int[nbtria3neig]; 
           for (i=0; i<nbtria3neig ; i++) arrayi[i]=i+1;
           err=MEDmeshEntityNumberWr(fidjoint,namejnt,MED_NO_DT,MED_NO_IT,MED_CELL,MED_TRIA3,nbtria3neig,arrayi);
           delete[] arrayi;
           if (err<0) std::cerr<<"Problem MEDmeshEntityNumberWr of triangles "<<namejnt<<std::endl;
          }
         }

         delete[] arraynodes;
         if (nbtria3neig>0) delete[] arrayfaces;
         delete[] inodes;

         //!masterfile.xml
         node=xmlNewChild(joints_node, 0, BAD_CAST "joint", 0);
         xmlNewProp(node, BAD_CAST "id", BAD_CAST i2a(ineig).c_str());
         xmlNewProp(node, BAD_CAST "nodes_number", BAD_CAST i2a(nbnodesneig).c_str());
         xmlNewProp(node, BAD_CAST "faces_number", BAD_CAST i2a(nbtria3neig).c_str());
         //node2 = xmlNewChild(node, 0, BAD_CAST "nodes", 0);
         //xmlNewProp(node2, BAD_CAST "number", BAD_CAST i2a(nbnodesneig).c_str());
         //node2 = xmlNewChild(node, 0, BAD_CAST "faces", 0);
         //xmlNewProp(node2, BAD_CAST "number", BAD_CAST i2a(nbtria3neig).c_str());
      }

   //masterfile.xml
   xmlNewProp(joints_node, BAD_CAST "number", BAD_CAST i2a(nbjoints).c_str());
   xmlNewChild(joints_node, 0, BAD_CAST "id_neighbours", BAD_CAST sjoints.substr(1).c_str());
   
   tmp=tmp.sprintf("NB%d VC",idom);
   //qt3 xx=this->remove_key_mesh_wrap(QRegExp(tmp,true,true));
   xx=this->remove_key_mesh_wrap(QRegExp(tmp,Qt::CaseSensitive,QRegExp::RegExp));
   //tmp=tmp.sprintf("MS%d NE*",idom);
   //qt3 xx=this->remove_key_mesh_wrap(QRegExp(tmp,true,true));
   //xx=this->remove_key_mesh_wrap(QRegExp(tmp,Qt::CaseSensitive,QRegExp::RegExp));
   tmp=tmp.sprintf("FC%d",idom);
   //qt3 xx=this->remove_key_mesh_wrap(QRegExp(tmp,true,true));
   xx=this->remove_key_mesh_wrap(QRegExp(tmp,Qt::CaseSensitive,QRegExp::RegExp));
   tmp=tmp.sprintf("GL%d *",idom);
   //qt3 xx=this->remove_key_mesh_wrap(QRegExp(tmp,true,true));
   xx=this->remove_key_mesh_wrap(QRegExp(tmp,Qt::CaseSensitive,QRegExp::RegExp));
   return ok;
}

//************************************
bool ghs3dprl_mesh_wrap::idom_tetras()
{
   bool ok=true;
   QString tmp,key1;
   CVWtab *tab1;
   med_int i,*arrayi;
   int xx;

      //writing connectivity of tetrahedra by nodes
      key1=key1.sprintf("NB%d EV",idom); //files.NoBoite Elements Vertices (tetra only)
      tab1=this->restore_key(key1); //tab1=this->mestab[key1];
      nbtetra4=tab1->size/4;
      nbtetrastotal=nbtetrastotal + nbtetra4;
      if (verbose>5)std::cout<<"NumberOftetrahedra="<<nbtetra4<<std::endl;
      err=MEDmeshElementConnectivityWr(fid,nomfinal,MED_NO_DT,MED_NO_IT,0.,MED_CELL,MED_TETRA4,MED_NODAL,MED_FULL_INTERLACE,nbtetra4,tab1->tmint);
      if (err<0){std::cerr<<"Problem MEDmeshElementConnectivityWr for tetra connectivity"<<std::endl; return false;}

      //writing indices of tetrahedra
      arrayi=new med_int[nbtetra4];
      for (i=0; i<nbtetra4 ; i++) arrayi[i]=nbseg2+nbtria3+i+1;
      //for (i=0; i<nbtria3 ; i++) std::cout<<i<<" "<<arrayi[i]<<std::endl;
      err=MEDmeshEntityNumberWr(fid,nomfinal,MED_NO_DT,MED_NO_IT,MED_CELL,MED_TETRA4,nbtetra4,arrayi);
      delete[] arrayi;
      if (err<0){std::cerr<<"Problem MEDmeshEntityNumberWr of tetrahedra"<<std::endl; return false;}

      famtetra4=new med_int[nbtetra4];
      for (i=0; i<nbtetra4 ; i++) famtetra4[i]=famnewtetra4;

      if (!for_multithread) {
        //writing tetrahedra global numbering
        //GLx EL=files.GLo ELements
        key1=key1.sprintf("GL%d EL",idom);
        tab1=this->restore_key(key1); //tab1=this->mestab[key1];
        if (!tab1) {
          //tmp=pathini+casename+tmp.sprintf(format.toLatin1().constData(),nbfilestot,idom)+".glo";
          //ok=this->ReadFileGLO(tmp);
          //tab1=this->restore_key(key1);
          //if (!tab1) return false;

          if (!for_tetrahpc) {
              tmp=pathini+casename+tmp.sprintf(format.toLatin1().constData(),nbfilestot,idom)+".glo";
              ok=this->ReadFileGLO(tmp);
          }
          if (for_tetrahpc) {
              tmp=pathini+casename+"_out"+tmp.sprintf(format_tetra.toLatin1().constData(),idom)+".global";
              ok=this->ReadFileGLOBAL(tmp);
          }
          tab1=this->restore_key(key1);
          if (!tab1) return false;

        }

        if (tab1->size!=nbtetra4){
          std::cerr<<"Problem incorrect size of tetrahedra global numbering"<<std::endl; return false;}
        if (verbose>2)
          std::cout<<"CreateMEDglobalNumerotation_tetrahedra "<<key1.toLatin1().constData()<<" "<<tab1->size<<std::endl;
        err=MEDmeshGlobalNumberWr(fid,nomfinal,MED_NO_DT,MED_NO_IT,MED_CELL,MED_TETRA4,tab1->size,tab1->tmint);
        if (err<0){std::cerr<<"Problem MEDmeshGlobalNumberWr tetrahedra"<<std::endl; return false;}
      }

      tmp=tmp.sprintf("NB%d EV",idom);
      //qt3 xx=this->remove_key_mesh_wrap(QRegExp(tmp,true,true));
      xx=this->remove_key_mesh_wrap(QRegExp(tmp,Qt::CaseSensitive,QRegExp::RegExp));
   return ok;
}

//************************************
med_int ghs3dprl_mesh_wrap::create_families(med_idt fid, int sign)
//if sign < 0 families faces or tria3 etc...
//if sign >= 0 family zero and family nodes
{
   med_int pas,ires;
   char nomfam[MED_NAME_SIZE+1];  //it.current()->name;
   char attdes[MED_COMMENT_SIZE+1]="_NO_DESCRIPTION";
   char *gro;
   med_int i,attide=1,attval=1,natt=1,num,ngro;
   
   if (sign>=0) pas=1; else pas=-1;
   ires=0;
   fend gb;
   fagr::iterator it1;
   fend::iterator it2;
   for (it1=families.fam.begin(); it1!=families.fam.end(); ++it1){
      num=(*it1).first.toLong();
      if ((pas==-1) && (num>=0)) continue; //not good families
      if ((pas== 1) && (num< 0)) continue; //not good families
      charendnull(nomfam,(*it1).first,MED_NAME_SIZE);
      ires++;
      //med_int natt=0;
      ngro=(*it1).second.size();
      if (verbose>5) 
         std::cout<<"CreateFamilyInMEDFile <"<<nomfam<<">\tNbGroups="<<ngro;
      gro=new char[MED_LNAME_SIZE*ngro+2];
      gb=(*it1).second;
      i=0;
      for (it2=gb.begin(); it2!=gb.end(); ++it2){
         charendnull(&gro[i*MED_LNAME_SIZE],(*it2).first,MED_LNAME_SIZE);
         if (verbose>5)std::cout<<" <"<<&gro[i*MED_LNAME_SIZE]<<"> ";
         i++;
      }
      if (verbose>5)std::cout<<std::endl;
      err=MEDfamilyCr(fid,nomfinal,nomfam,num,ngro,gro);
      delete[] gro;
      if (err<0) std::cerr<<"Problem MEDfamilyCr of "<<nomfam<<std::endl;
   }
   return ires;
}

med_int ghs3dprl_mesh_wrap::create_family_zero(med_idt fid, QString nameMesh)
{
   med_int pas,ires;
   ires=0;
   char nomfam[MED_NAME_SIZE+1]="FAMILLE_ZERO";  //it.current()->name;
   char attdes[MED_COMMENT_SIZE+1]="_NO_DESCRIPTION";
   
   char *gro;
   med_int i,attide=1,attval=1,natt=1,num=0,ngro=0;
   
   gro=new char[MED_LNAME_SIZE*ngro+2];
   if (verbose>3)std::cout<<"\ncreate_family_ZERO "<<nameMesh.toLatin1().constData()<<std::endl;
   err=MEDfamilyCr(fid,nameMesh.toLatin1().constData(),nomfam,num,ngro,gro);
   if (err<0) std::cerr<<"Problem MEDfamilyCr FAMILLE_ZERO of "<<nameMesh.toLatin1().constData()<<std::endl;
   delete[] gro;
   return ires;
}



