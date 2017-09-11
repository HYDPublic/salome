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
// File   : GHS3DPRLPlugin_Hypothesis_i.cxx
// Author : Christian VAN WAMBEKE (CEA) (from Hexotic plugin Lioka RAZAFINDRAZAKA)
// ---
//
#include "GHS3DPRLPlugin_Hypothesis_i.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_PythonDump.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

//=============================================================================
/*!
 *  GHS3DPRLPlugin_Hypothesis_i::GHS3DPRLPlugin_Hypothesis_i
 *
 *  Constructor
 */
//=============================================================================
GHS3DPRLPlugin_Hypothesis_i::
GHS3DPRLPlugin_Hypothesis_i (PortableServer::POA_ptr thePOA,
                             int                     theStudyId,
                             ::SMESH_Gen*            theGenImpl)
  : SALOME::GenericObj_i( thePOA ),
    SMESH_Hypothesis_i( thePOA )
{
  MESSAGE( "GHS3DPRLPlugin_Hypothesis_i::GHS3DPRLPlugin_Hypothesis_i" );
  myBaseImpl = new ::GHS3DPRLPlugin_Hypothesis (theGenImpl->GetANewId(),
                                                theStudyId,
                                                theGenImpl);
}

//=============================================================================
/*!
 *  GHS3DPRLPlugin_Hypothesis_i::~GHS3DPRLPlugin_Hypothesis_i
 *
 *  Destructor
 */
//=============================================================================
GHS3DPRLPlugin_Hypothesis_i::~GHS3DPRLPlugin_Hypothesis_i()
{
  MESSAGE( "GHS3DPRLPlugin_Hypothesis_i::~GHS3DPRLPlugin_Hypothesis_i" );
}

//=============================================================================
/*!
 *  GHS3DPRLPlugin_Hypothesis_i::SetMEDName
 *  GHS3DPRLPlugin_Hypothesis_i::SetNbPart
 *  GHS3DPRLPlugin_Hypothesis_i::SetKeepFiles
 *  GHS3DPRLPlugin_Hypothesis_i::SetBackground
 *  GHS3DPRLPlugin_Hypothesis_i::SetMultithread
 *  GHS3DPRLPlugin_Hypothesis_i::SetGradation
 *  GHS3DPRLPlugin_Hypothesis_i::SetMinSize
 *  GHS3DPRLPlugin_Hypothesis_i::SetMaxSize
 */
//=============================================================================

void GHS3DPRLPlugin_Hypothesis_i::SetMEDName (const char *theValue)
{
  MESSAGE("GHS3DPRLPlugin_Hypothesis_i::SetMEDName");
  ASSERT(myBaseImpl);
  this->GetImpl()->SetMEDName(theValue);
  SMESH::TPythonDump() << _this() << ".SetMEDName( '" << theValue << "' )";
}

void GHS3DPRLPlugin_Hypothesis_i::SetNbPart (CORBA::Long theValue)
{
  MESSAGE("GHS3DPRLPlugin_Hypothesis_i::SetNbPart");
  ASSERT(myBaseImpl);
  this->GetImpl()->SetNbPart(theValue);
  SMESH::TPythonDump() << _this() << ".SetNbPart( " << theValue << " )";
}

void GHS3DPRLPlugin_Hypothesis_i::SetKeepFiles (CORBA::Boolean theValue)
{
  MESSAGE("GHS3DPRLPlugin_Hypothesis_i::SetKeepFiles");
  ASSERT(myBaseImpl);
  this->GetImpl()->SetKeepFiles(theValue);
  SMESH::TPythonDump() << _this() << ".SetKeepFiles( " << theValue << " )";
}

void GHS3DPRLPlugin_Hypothesis_i::SetBackground (CORBA::Boolean theValue)
{
  MESSAGE("GHS3DPRLPlugin_Hypothesis_i::SetBackground");
  ASSERT(myBaseImpl);
  this->GetImpl()->SetBackground(theValue);
  SMESH::TPythonDump() << _this() << ".SetBackground( " << theValue << " )";
}

void GHS3DPRLPlugin_Hypothesis_i::SetMultithread (CORBA::Boolean theValue)
{
  MESSAGE("GHS3DPRLPlugin_Hypothesis_i::SetMultithread");
  ASSERT(myBaseImpl);
  this->GetImpl()->SetMultithread(theValue);
  SMESH::TPythonDump() << _this() << ".SetMultithread( " << theValue << " )";
}

//void GHS3DPRLPlugin_Hypothesis_i::SetToMergeSubdomains (CORBA::Boolean theValue)
//{
//  MESSAGE("GHS3DPRLPlugin_Hypothesis_i::SetToMergeSubdomains");
//  ASSERT(myBaseImpl);
//  this->GetImpl()->SetToMergeSubdomains(theValue);
//  SMESH::TPythonDump() << _this() << ".SetToMergeSubdomains( " << theValue << " )";
//}

void GHS3DPRLPlugin_Hypothesis_i::SetGradation (CORBA::Float theValue)
{
  MESSAGE("GHS3DPRLPlugin_Hypothesis_i::SetGradation");
  ASSERT(myBaseImpl);
  this->GetImpl()->SetGradation(theValue);
  SMESH::TPythonDump() << _this() << ".SetGradation( " << theValue << " )";
}

void GHS3DPRLPlugin_Hypothesis_i::SetMinSize (CORBA::Float theValue)
{
  MESSAGE("GHS3DPRLPlugin_Hypothesis_i::SetMinSize");
  ASSERT(myBaseImpl);
  this->GetImpl()->SetMinSize(theValue);
  SMESH::TPythonDump() << _this() << ".SetMinSize( " << theValue << " )";
}

void GHS3DPRLPlugin_Hypothesis_i::SetMaxSize (CORBA::Float theValue)
{
  MESSAGE("GHS3DPRLPlugin_Hypothesis_i::SetMaxSize");
  ASSERT(myBaseImpl);
  this->GetImpl()->SetMaxSize(theValue);
  SMESH::TPythonDump() << _this() << ".SetMaxSize( " << theValue << " )";
}





//=============================================================================
/*!
 *  GHS3DPRLPlugin_Hypothesis_i::GetMEDName
 *  GHS3DPRLPlugin_Hypothesis_i::GetNbPart
 *  GHS3DPRLPlugin_Hypothesis_i::GetKeepFiles
 *  GHS3DPRLPlugin_Hypothesis_i::GetBackground
 *  GHS3DPRLPlugin_Hypothesis_i::GetMultithread
 *  GHS3DPRLPlugin_Hypothesis_i::GetGradation
 *  GHS3DPRLPlugin_Hypothesis_i::GetMinSize
 *  GHS3DPRLPlugin_Hypothesis_i::GetMaxSize
 */
//=============================================================================

char * GHS3DPRLPlugin_Hypothesis_i::GetMEDName()
{
  MESSAGE("GHS3DPRLPlugin_Hypothesis_i::GetMEDName");
  ASSERT(myBaseImpl);
  CORBA::String_var c_s =
  CORBA::string_dup(this->GetImpl()->GetMEDName().c_str());
  return c_s._retn();
}

CORBA::Long GHS3DPRLPlugin_Hypothesis_i::GetNbPart()
{
  MESSAGE("GHS3DPRLPlugin_Hypothesis_i::GetNbPart");
  ASSERT(myBaseImpl);
  return this->GetImpl()->GetNbPart();
}

CORBA::Boolean GHS3DPRLPlugin_Hypothesis_i::GetKeepFiles()
{
  MESSAGE("GHS3DPRLPlugin_Hypothesis_i::GetKeepFiles");
  ASSERT(myBaseImpl);
  return this->GetImpl()->GetKeepFiles();
}

CORBA::Boolean GHS3DPRLPlugin_Hypothesis_i::GetBackground()
{
  MESSAGE("GHS3DPRLPlugin_Hypothesis_i::GetBackground");
  ASSERT(myBaseImpl);
  return this->GetImpl()->GetBackground();
}

CORBA::Boolean GHS3DPRLPlugin_Hypothesis_i::GetMultithread()
{
  MESSAGE("GHS3DPRLPlugin_Hypothesis_i::GetMultithread");
  ASSERT(myBaseImpl);
  return this->GetImpl()->GetMultithread();
}

//CORBA::Boolean GHS3DPRLPlugin_Hypothesis_i::GetToMergeSubdomains()
//{
//  MESSAGE("GHS3DPRLPlugin_Hypothesis_i::GetToMergeSubdomains");
//  ASSERT(myBaseImpl);
//  return this->GetImpl()->GetToMergeSubdomains();
//}

CORBA::Float GHS3DPRLPlugin_Hypothesis_i::GetGradation()
{
  MESSAGE("GHS3DPRLPlugin_Hypothesis_i::GetGradation");
  ASSERT(myBaseImpl);
  return this->GetImpl()->GetGradation();
}

CORBA::Float GHS3DPRLPlugin_Hypothesis_i::GetMinSize()
{
  MESSAGE("GHS3DPRLPlugin_Hypothesis_i::GetMinSize");
  ASSERT(myBaseImpl);
  return this->GetImpl()->GetMinSize();
}

CORBA::Float GHS3DPRLPlugin_Hypothesis_i::GetMaxSize()
{
  MESSAGE("GHS3DPRLPlugin_Hypothesis_i::GetMaxSize");
  ASSERT(myBaseImpl);
  return this->GetImpl()->GetMaxSize();
}

void GHS3DPRLPlugin_Hypothesis_i::SetAdvancedOption(const char* theOptAndVals )
{
  if ( theOptAndVals && GetImpl()->GetAdvancedOption() != theOptAndVals )
  {
    GetImpl()->SetAdvancedOption( theOptAndVals );
    SMESH::TPythonDump() << _this() << ".SetAdvancedOption( '" << theOptAndVals << "' )";
  }
}

char* GHS3DPRLPlugin_Hypothesis_i::GetAdvancedOption()
{
  return CORBA::string_dup( GetImpl()->GetAdvancedOption().c_str() );
}

//=============================================================================
/*!
 *  GHS3DPRLPlugin_Hypothesis_i::GetImpl
 *
 *  Get implementation
 */
//=============================================================================
::GHS3DPRLPlugin_Hypothesis* GHS3DPRLPlugin_Hypothesis_i::GetImpl()
{
  MESSAGE("GHS3DPRLPlugin_Hypothesis_i::GetImpl");
  return (::GHS3DPRLPlugin_Hypothesis*)myBaseImpl;
}

//================================================================================
/*!
 * \brief Verify whether hypothesis supports given entity type
 * \param type - dimension (see SMESH::Dimension enumeration)
 * \retval CORBA::Boolean - TRUE if dimension is supported, FALSE otherwise
 *
 * Verify whether hypothesis supports given entity type (see SMESH::Dimension enumeration)
 */
//================================================================================
CORBA::Boolean GHS3DPRLPlugin_Hypothesis_i::IsDimSupported( SMESH::Dimension type )
{
  return type == SMESH::DIM_3D;
}
