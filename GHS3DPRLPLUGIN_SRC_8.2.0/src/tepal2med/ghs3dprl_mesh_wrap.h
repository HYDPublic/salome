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
// File   : ghs3dprl_mesh_wrap.h
// Author : Christian VAN WAMBEKE (CEA) (from Hexotic plugin Lioka RAZAFINDRAZAKA)
// ---
//
#ifndef GHS3DPRL_MESH_WRAP_H
#define GHS3DPRL_MESH_WRAP_H

#include <string>
#include <map>
#include <QHash>
#include <QRegExp>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

//Med File V 2.2 attributes
//#undef MED_H
//#undef MED_PROTO_H

#include <med.h>
//#include <med_proto.h>

class CVWtab
//contains size=size of vector and the vector (med_int or med_float)
{

public:
   static long memoryuse;
   static long memorymax;
   long size,type;
   med_int *tmint;   //integer med
   med_float *tmflo; //float med
   QString filename;

   CVWtab(long nb, med_int *pmint);
   CVWtab(long nb, med_float *pmflo);
   ~CVWtab();
   bool CVWtab_deallocate();
   bool is_equal(CVWtab *tab2);
};

typedef std::map<QString,  int> fend;
typedef std::map<QString, fend> fagr;
class familles{
   private:
   void newfam(QString nom);
   void newgro(QString nom);
   public:
   int no;
   fagr fam;
   fagr gro;
   void write();
   xmlNodePtr xml_groups();
   void add(QString nomfam, QString nomgro);
   void addgro();
   bool get_number_of_new_family(int sign,med_int *num,QString *tmp);
   med_int find_family_on_groups(med_int fam1, med_int fam2);
   fend fuse_goups(med_int fam1, med_int fam2);
};

class ghs3dprl_mesh_wrap
{
public:
   QString 
      medname,
      casename,
      path,
      pathini,
      filemed,
      format,
      format_tetra;
   bool for_tetrahpc;    //to know what files to read: .noboite or .mesh
   bool for_multithread; //to know what files to read: one or more
   QRegExp deletegroups; //regular expression
   long
      nbtetrastotal,
      nofile,nbfiles,nbfilestot,
      nbelem_limit_swap,
      verbose;
   med_err err;
   med_idt fid,fidjoint;
   med_int idom;

   //master.xml
   std::string filemaster,domainname;
   char distfilename[MED_COMMENT_SIZE];
   xmlDocPtr master_doc;
   xmlNodePtr root_node,node,node2,
              joints_node,info_node,files_node,mesh_node;

   QHash<QString,CVWtab*> mestab;
   familles families;

   //from skin.med
   char nommaa[MED_NAME_SIZE+1];
   char maillage_description[MED_COMMENT_SIZE+1];
   char nomcoo[3*MED_SNAME_SIZE+1];
   char unicoo[3*MED_SNAME_SIZE+1];
   med_int *famnodesskin;    //from skin.med...
   med_int *famseg2skin;     //...valid on global index/numerotation
   med_int *famtria3skin;

   //to final files .med with tetrahedra
   char nomfinal[MED_NAME_SIZE+1];
   med_int *famnodes,nbnodes,famnewnodes,famallnodes;  //to final files .med with tetrahedra
   med_int *famseg2,nbseg2,famnewseg2,famallseg2;
   med_int *famtria3,nbtria3,famnewtria3,famalltria3;
   med_int *famtetra4,nbtetra4,famnewtetra4,famalltetra4;

   //low level
   bool list_keys_mesh_wrap(); //list keys
   bool list_onekey_mesh_wrap(const QString &key);
   long remove_key_mesh_wrap(const QRegExp &rxp);
   long nb_key_mesh_wrap(const QRegExp &rxp);
   long remove_all_keys_mesh_wrap();
   bool insert_key(const QString &key,CVWtab *tab);
   CVWtab* restore_key(const QString &key);

   //family level
   bool set_one_more_family(med_int *fami, med_int *more, med_int nb);
   med_int create_families(med_idt fid, int sign);
   med_int create_family_zero(med_idt fid, QString nameMesh);
   void add_family(med_int num,QString newgro);
   void cout_families_and_groups();
   bool idom_nodes();
   bool idom_edges();
   bool idom_faces();
   bool idom_joints();
   bool idom_tetras();

   //test level
   bool test_msg_wrap();
   bool test_vertices_wrap();

   //hight level
   long SwapOutOfMemory_key_mesh_wrap(const QRegExp &rxp,long ifgreaterthan=0);
   bool ReadFileMSGnew(const QString FileName);
   bool TestExistingFileMESHnew(const QString FileName);
   bool ReadFileGLO(const QString FileName);
   bool ReadFileGLOBAL(const QString FileName); //mg-tetra v2.1.11
   bool ReadFileFACES(const QString FileName);
   bool ReadFileMESH(const QString FileName);
   bool ReadFileNOBOITE(const QString FileName);
   bool ReadFileNOBOITEB(const QString FileName);
   bool ReadFilePOINTS(const QString FileName);
   bool Find_VerticesDomainToVerticesSkin();
   bool Write_masterxmlMEDfile();
   bool Write_MEDfiles_v0(bool deletekeys=false); 
   bool Write_MEDfiles_v1(bool deletekeys=false); 
   bool Write_MEDfiles_v2(bool deletekeys=false);
};

#endif
