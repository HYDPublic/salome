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
// File   : GHS3DPRLPlugin_Hypothesis.cxx
// Author : Christian VAN WAMBEKE (CEA) (from Hexotic plugin Lioka RAZAFINDRAZAKA)
// ---
//
#include "GHS3DPRLPlugin_Hypothesis.hxx"
#include <utilities.h>

//=============================================================================
/*!
 *
 */
//=============================================================================
GHS3DPRLPlugin_Hypothesis::GHS3DPRLPlugin_Hypothesis (int hypId, int studyId, SMESH_Gen * gen)
  : SMESH_Hypothesis(hypId, studyId, gen),
    _MEDName( GetDefaultMEDName() ),
    _NbPart( GetDefaultNbPart() ),
    _KeepFiles( GetDefaultKeepFiles() ),
    _Background( GetDefaultBackground() ),
    _Multithread( GetDefaultMultithread() ),
    //_ToMergeSubdomains( GetDefaultToMergeSubdomains() ),
    _Gradation( GetDefaultGradation() ),
    _MinSize( GetDefaultMinSize() ),
    _MaxSize( GetDefaultMaxSize() )
{
  MESSAGE("GHS3DPRLPlugin_Hypothesis::GHS3DPRLPlugin_Hypothesis");
  _name = GetHypType();
  _param_algo_dim = 3;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
static std::string cutOrReplaceBlancs(std::string theIn)
{
  // cut all blancs at the beginning and at the end of the string,
  // replace each blancs sequence of maximum length inside the string by one '_' symbol,
  // as blancs consider: 9 (TAB), 10 (LF), 11 (VT), 12 (FF), 13 (CR) and 32 (Space)
  int len = theIn.length();
  int lastgood = 0;
  const char* str1 = theIn.c_str();
  char* str2 = new char [len + 1];
  bool del = true;

  for (int i = 0, j = 0; i < len; i++)
  {
    unsigned short ucs = (unsigned short)(str1[i]);
    if ((9 <= ucs && ucs <= 13) || ucs == 32)
    {
      if (!del)
      {
        str2[j++] = '_';
        del = true;
      }
    }
    else
    {
      str2[j++] = str1[i];
      lastgood = j;
      del = false;
    }
  }
  str2[lastgood] = '\0';
  std::string anOut = str2;
  return anOut;
}

void GHS3DPRLPlugin_Hypothesis::SetMEDName(std::string theVal) {
  //without whitespaces! ..from python?
  std::string tmp1 = cutOrReplaceBlancs(theVal);
  std::string tmp2 = _MEDName;
  if (tmp1 != tmp2) {
    _MEDName = tmp1.c_str();
    NotifySubMeshesHypothesisModification();
  }
}

void GHS3DPRLPlugin_Hypothesis::SetNbPart(int theVal) {
  if (theVal != _NbPart) {
    _NbPart = theVal;
    NotifySubMeshesHypothesisModification();
  }
}

void GHS3DPRLPlugin_Hypothesis::SetKeepFiles(bool theVal) {
  if (theVal != _KeepFiles) {
    _KeepFiles = theVal;
    NotifySubMeshesHypothesisModification();
  }
}

void GHS3DPRLPlugin_Hypothesis::SetBackground(bool theVal) {
  if (theVal != _Background) {
    _Background = theVal;
    NotifySubMeshesHypothesisModification();
  }
}

void GHS3DPRLPlugin_Hypothesis::SetMultithread(bool theVal) {
  if (theVal != _Multithread) {
    _Multithread = theVal;
    NotifySubMeshesHypothesisModification();
  }
}

//void GHS3DPRLPlugin_Hypothesis::SetToMergeSubdomains(bool theVal) {
//  if (theVal != _ToMergeSubdomains) {
//    _ToMergeSubdomains = theVal;
//    NotifySubMeshesHypothesisModification();
//  }
//}

void GHS3DPRLPlugin_Hypothesis::SetGradation(float theVal) {
  if (theVal != _Gradation) {
    _Gradation = theVal;
    NotifySubMeshesHypothesisModification();
  }
}

void GHS3DPRLPlugin_Hypothesis::SetMinSize(float theVal) {
  if (theVal != _MinSize) {
    _MinSize = theVal;
    NotifySubMeshesHypothesisModification();
  }
}

void GHS3DPRLPlugin_Hypothesis::SetMaxSize(float theVal) {
  if (theVal != _MaxSize) {
    _MaxSize = theVal;
    NotifySubMeshesHypothesisModification();
  }
}

void GHS3DPRLPlugin_Hypothesis::SetAdvancedOption(const char* theOptAndVals )
{
  if ( _AdvOptions != theOptAndVals )
  {
    _AdvOptions = theOptAndVals;
    NotifySubMeshesHypothesisModification();
  }
}



//=============================================================================
/*!
 *
 */
//=============================================================================
std::ostream& GHS3DPRLPlugin_Hypothesis::SaveTo(std::ostream& save)
{
  //explicit outputs for future code compatibility of saved .hdf
  //save without any whitespaces!
  save<<"MEDName="<<_MEDName<<";";
  save<<"NbPart="<<_NbPart<<";";
  //save<<"ToMeshHoles="<<(int) _ToMeshHoles<<";";
  //save<<"ToMergeSubdomains="<<(int) _ToMergeSubdomains<<";";
  save<<"Gradation="<<(float) _Gradation<<";";
  save<<"MinSize="<<(float) _MinSize<<";";
  save<<"MaxSize="<<(float) _MaxSize<<";";
  save<<"KeepFiles="<<(int) _KeepFiles<<";";
  save<<"Background="<<(int) _Background<<";";
  save<<"Multithread="<<(int) _Multithread<<";";
  save<<" " << _AdvOptions.size() << " " << _AdvOptions;
  return save;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
std::istream& GHS3DPRLPlugin_Hypothesis::LoadFrom(std::istream& load)
{
   //explicit inputs for future code compatibility of saved .hdf
   bool isOK = true;
   std::string str1,str2,str3,str4;

   //save without any whitespaces!
   isOK = static_cast<bool>(load >> str1);
   if (!(isOK)) {
     //defaults values assumed
     load.clear(std::ios::badbit | load.rdstate());
     return load;
   }
   int pos = 0;
   int len = str1.length();
   while (pos < len) {
     int found = str1.find(';',pos);
     str2 = str1.substr(pos,found-pos);
     int eqpos = str2.find('=',0);
     str3 = str2.substr(0,eqpos);
     str4 = str2.substr(eqpos+1);
     pos = found + 1;

     if (str3=="MEDName")     _MEDName     = str4.c_str();
     if (str3=="NbPart")      _NbPart      = atoi(str4.c_str());
     if (str3=="KeepFiles")   _KeepFiles   = (bool) atoi(str4.c_str());
     if (str3=="Gradation")   _Gradation   = atof(str4.c_str());
     if (str3=="MinSize")     _MinSize     = atof(str4.c_str());
     if (str3=="MaxSize")     _MaxSize     = atof(str4.c_str());
     if (str3=="Background")  _Background  = (bool) atoi(str4.c_str());
     if (str3=="Multithread") _Multithread = (bool) atoi(str4.c_str());
   }

   len = 0;
   isOK = static_cast<bool>(load >> len >> std::ws);
   if ( isOK && len > 0 )
   {
     _AdvOptions.resize( len );
     load.get( &_AdvOptions[0], len + 1);
   }
   return load;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
std::ostream& operator <<(std::ostream& save, GHS3DPRLPlugin_Hypothesis& hyp)
{
  return hyp.SaveTo( save );
}

//=============================================================================
/*!
 *
 */
//=============================================================================
std::istream& operator >>(std::istream& load, GHS3DPRLPlugin_Hypothesis& hyp)
{
  return hyp.LoadFrom( load );
}


//================================================================================
/*!
 * \brief Does nothing
 * \param theMesh - the built mesh
 * \param theShape - the geometry of interest
 * \retval bool - always false
 */
//================================================================================
bool GHS3DPRLPlugin_Hypothesis::SetParametersByMesh(const SMESH_Mesh*   theMesh,
                                                    const TopoDS_Shape& theShape)
{
  return false;
}
//================================================================================
/*!
 * \brief Initialize my parameter values by default parameters.
 *  \retval bool - true if parameter values have been successfully defined
 */
//================================================================================

bool GHS3DPRLPlugin_Hypothesis::SetParametersByDefaults(const TDefaults&  /*dflts*/,
                                                        const SMESH_Mesh* /*theMesh*/)
{
  return false;
}

//=============================================================================
std::string GHS3DPRLPlugin_Hypothesis::GetDefaultMEDName()
{
  return "DOMAIN\0";
}

//=============================================================================
int GHS3DPRLPlugin_Hypothesis::GetDefaultNbPart()
{
  return 16;
}

//=============================================================================
bool GHS3DPRLPlugin_Hypothesis::GetDefaultKeepFiles()
{
  return false;
}

//=============================================================================
bool GHS3DPRLPlugin_Hypothesis::GetDefaultBackground()
{
  return false;
}

//=============================================================================
bool GHS3DPRLPlugin_Hypothesis::GetDefaultMultithread()
{
  return false;
}

//=============================================================================
//bool GHS3DPRLPlugin_Hypothesis::GetDefaultToMeshHoles()
//{
//  return false;
//}

//=============================================================================
//bool GHS3DPRLPlugin_Hypothesis::GetDefaultToMergeSubdomains()
//{
//  return false;
//}

//=============================================================================
float GHS3DPRLPlugin_Hypothesis::GetDefaultGradation()
{
  return 1.05;
}
//=============================================================================
float GHS3DPRLPlugin_Hypothesis::GetDefaultMinSize()
{
  return 0.;
}
//=============================================================================
float GHS3DPRLPlugin_Hypothesis::GetDefaultMaxSize()
{
  return 0.;
}

