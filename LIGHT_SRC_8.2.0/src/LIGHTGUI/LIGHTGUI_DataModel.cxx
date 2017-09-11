// Copyright (C) 2005-2016  OPEN CASCADE
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
// File   : LIGHTGUI_DataModel.cxx
// Author : Julia DOROVSKIKH

#include "LIGHTGUI_DataModel.h"
#include "LIGHTGUI_DataObject.h"

#include <CAM_Application.h>
#include <CAM_Module.h>
#include <LightApp_Study.h>
#include <SUIT_DataObjectIterator.h>
#include <SUIT_Tools.h>

#include <QFile>
#include <QTextStream>

/*!
  \class LIGHTGUI_DataModel
  \brief LIGHT module data model.

  This class defines an internal organization of the module's data and the way
  how these data are presented in the study (e.g. in the Object Browser).

  In addition, this class includes methods which modify the data tree:
  load/save text file, insert/remove text line, etc.
*/

/*!
  \brief Constructor
  \param module parent module
*/
LIGHTGUI_DataModel::LIGHTGUI_DataModel( CAM_Module* module )
: LightApp_DataModel( module ),
  myFileName( "" ),
  myStudyURL( "" ),
  myMaxId( 0 )
{
}

/*!
  \brief Destructor.
*/
LIGHTGUI_DataModel::~LIGHTGUI_DataModel()
{
}

/*!
  \brief Open data model (read data from the files).
  \param url study file path
  \param study study pointer
  \param listOfFiles list of the (temporary) files with data
  \return operation status (\c true on success and \c false on error)
*/
bool LIGHTGUI_DataModel::open( const QString& url, CAM_Study* study, QStringList listOfFiles )
{
  LightApp_Study* aDoc = dynamic_cast<LightApp_Study*>( study );
  if ( !aDoc )
    return false;

  LightApp_DataModel::open( url, aDoc, listOfFiles );

  // The first list item contains path to a temporary
  // directory, where the persistent files was placed
  if ( listOfFiles.count() > 0 ) {
    QString aTmpDir ( listOfFiles[0] );

    // This module operates with a single persistent file
    if ( listOfFiles.size() == 2 ) {
      myStudyURL = url;
      QString aFullPath = SUIT_Tools::addSlash( aTmpDir ) + listOfFiles[1];
      loadFile( aFullPath, aDoc );
    }
  }

  return true;
}

/*!
  \brief Save data model (write data to the files).
  \param listOfFiles returning list of the (temporary) files with saved data
  \return operation status (\c true on success and \c false on error)
*/
bool LIGHTGUI_DataModel::save( QStringList& theListOfFiles )
{
  bool isMultiFile = false; // TODO: decide, how to access this parameter

  LightApp_DataModel::save( theListOfFiles );

  LightApp_Study* study = dynamic_cast<LightApp_Study*>( module()->application()->activeStudy() );

  QString aTmpDir = study->GetTmpDir( myStudyURL.toLatin1(), isMultiFile ).c_str();

  QString aFileName = SUIT_Tools::file( myStudyURL, false ) + "_LIGHTGUI.txt";
  QString aFullPath = aTmpDir + aFileName;
  dumpFile( aFullPath );

  theListOfFiles.append( aTmpDir );
  theListOfFiles.append( aFileName );

  return true;
}

/*!
  \brief Save data model (write data to the files).
  \param url study file path
  \param study study pointer
  \param listOfFiles returning list of the (temporary) files with saved data
  \return operation status (\c true on success and \c false on error)
*/
bool LIGHTGUI_DataModel::saveAs( const QString& url, CAM_Study* study, QStringList& theListOfFiles )
{
  myStudyURL = url;
  return save( theListOfFiles );
}

/*!
  \brief Close data model (remove all relevant data).
  \return operation status (\c true on success and \c false on error)
*/
bool LIGHTGUI_DataModel::close()
{
  return LightApp_DataModel::close();
}

/*!
  \brief Create empty data model.
  \param study study pointer
  \return operation status (\c true on success and \c false on error)
*/
bool LIGHTGUI_DataModel::create( CAM_Study* study )
{
  return true;
}

/*!
  \brief Get 'modified' flag.
  \return \c true if data is changed from the last saving
*/
bool LIGHTGUI_DataModel::isModified() const
{
  // return false to avoid masking study's isModified()
  return false;
}

/*!
  \brief Get 'saved' flag.
  \return \c true if data has been saved
*/
bool LIGHTGUI_DataModel::isSaved() const
{
  // return true to avoid masking study's isSaved()
  return true;
}

/*!
  \brief Update data model.
  \param obj data object (starting for the update)
  \param study study pointer
*/
void LIGHTGUI_DataModel::update( LightApp_DataObject* /*obj*/, LightApp_Study* /*study*/ )
{
  // Nothing to do here: we always keep the data tree in the up-to-date state
  // The only goal of this method is to hide default behavior from LightApp_DataModel
  return;
}

/*
  \brief Load text data from the file.
  \param filename file path
  \param study study pointer
  \return \c true if data is loaded successfully
*/  
bool LIGHTGUI_DataModel::loadFile( const QString& filename, CAM_Study* study )
{
  if ( filename.isEmpty() ) return false;

  myFileName = filename;
  
  QStringList lines;
  QFile file ( myFileName );
  if ( file.open( QIODevice::ReadOnly ) ) {
    QTextStream stream ( &file );
    QString line;
    while ( !stream.atEnd() ) {
      line = stream.readLine(); // line of text excluding '\n'
      lines += line;
    }
    file.close();

    if  ( !study )
      study = (CAM_Study*)module()->application()->activeStudy();
    buildTree( study->root(), lines );

    return true;
  }

  return false;
}

/*!
  \brief Save text data to the file
  \param filename file path
  \return \c true if data is loaded successfully
*/
bool LIGHTGUI_DataModel::dumpFile( const QString& filename )
{
  QString aFileName = filename;
  if ( aFileName.isEmpty() )
    aFileName = myFileName;

  if ( aFileName.isEmpty() ) return false;

  QStringList lines;
  for ( SUIT_DataObjectIterator it( root(), SUIT_DataObjectIterator::DepthLeft ); it.current(); ++it ) {
    LIGHTGUI_DataObject* obj = dynamic_cast<LIGHTGUI_DataObject*>( it.current() );
    if ( obj && obj->lineNb() > 0 )
      lines.insert( obj->lineNb()-1, obj->lineText() );
  }

  QFile file ( aFileName );
  if ( file.open( QIODevice::WriteOnly ) ) {
    QTextStream stream ( &file );
    QStringList::Iterator it = lines.begin();
    for ( ; it != lines.end(); ++it ) {
      stream << *it << "\n";
    }
    file.close();

    myFileName = aFileName;
    return true;
  }

  return false;
}

/*!
  \brief Get latest loaded or saved text file's name.
  \return file name (empty string if file has not been loaded or saved)
*/
QString LIGHTGUI_DataModel::fileName() const
{
  return myFileName;
}

/*!
  \brief Get text for the specified line.
  \param id line identifier
  \return string text for the line with identifier \a id
          (empty string is \a id is invalid)
*/
QString LIGHTGUI_DataModel::getLineText( const int id ) const
{
  QString aText;

  LIGHTGUI_DataObject* obj = findObject( id );
  if ( obj )
    aText = obj->lineText();

  return aText;
}

/*!
  \brief Set text for the specified line.
  \param id line identifier
  \param txt new string text for the line
  \return \c true on success or \c false if \a id is invalid
*/
bool LIGHTGUI_DataModel::setLineText( const int id, const QString& txt )
{
  LIGHTGUI_DataObject* obj = findObject( id );
  if ( obj ) {
    if ( (  txt.trimmed().isEmpty() && !obj->lineText().trimmed().isEmpty() ) ||
         ( !txt.trimmed().isEmpty() &&  obj->lineText().trimmed().isEmpty() ) ) {
      if ( obj->lineText().trimmed().isEmpty() ) {
	// paragraph becomes a text line
	SUIT_DataObject* newParent = obj->prevBrother();
	if ( newParent ) {
	  obj->setParent( newParent );
	  while ( SUIT_DataObject* first = obj->firstChild() )
	    first->setParent( newParent );
	}
      }
      else {
	// text line becomes a paragraph
	SUIT_DataObject* parent = obj->parent();
	SUIT_DataObject* modelRoot = parent ? parent->parent() : 0;
	if ( modelRoot && parent ) {
	  int pos = parent->childPos( obj );
	  modelRoot->insertChild( obj, modelRoot->childPos( parent ) + 1 );
	  while ( SUIT_DataObject* next = parent->childObject( pos ) )
	    obj->appendChild( next );
	}
      }
    }
    obj->setLineText( txt );
    return true;
  }
  return false;
}

/*!
  \brief Insert new text line.
  \param id identifier of the line after which new one should be inserted
         (if <=0, new line is added to the end)
  \param txt string text
  \return \c true on success or \c false if \a id is invalid
*/
bool LIGHTGUI_DataModel::insertLineBefore( const int id, const QString& txt )
{
  // Insert new line before the item at position in the list,
  // or at the end() if position is out of range
  LightApp_DataObject* modelRoot = dynamic_cast<LightApp_DataObject*>( root() );
  LightApp_Study* study = dynamic_cast<LightApp_Study*>( module()->application()->activeStudy() );
  if ( id > 0 ) {
    if ( !modelRoot )
      return false;
    LIGHTGUI_DataObject* obj = findObject( id );
    if ( !obj || !obj->parent() )
      return false;
    SUIT_DataObject* parent = obj->parent();
    if ( txt.trimmed().isEmpty() ) {
      // insert new paragraph
      if ( obj->lineText().trimmed().isEmpty() ) {
        int pos = modelRoot->childPos( obj );
	modelRoot->insertChild( new LIGHTGUI_DataObject( generateId(), txt, 0 ), pos );
      }
      else {
	int pos = parent->childPos( obj );
	LIGHTGUI_DataObject* newObj = new LIGHTGUI_DataObject( generateId(), txt, 0 );
	modelRoot->insertChild( newObj, modelRoot->childPos( parent ) + 1 );
	while ( SUIT_DataObject* next = parent->childObject( pos ) )
	  newObj->appendChild( next );
      }
    }
    else {
      // insert new text line
      if ( obj->lineText().trimmed().isEmpty() ) {
	SUIT_DataObject* prevParent = obj->prevBrother();
	if ( prevParent )
	  prevParent->appendChild( new LIGHTGUI_DataObject( generateId(), txt, prevParent ) );
      }
      else {
        int pos = parent->childPos( obj );
	parent->insertChild( new LIGHTGUI_DataObject( generateId(), txt, 0 ), pos );
      }
    }
  }
  else {
    // append new paragraph/line
    if ( modelRoot )
      txt.trimmed().isEmpty() ? new LIGHTGUI_DataObject( generateId(), txt, modelRoot ) : 
	                        new LIGHTGUI_DataObject( generateId(), txt, modelRoot->lastChild() );
    else {
      QStringList lines;
      if ( !txt.trimmed().isEmpty() ) 
	lines.append( txt );
      buildTree( study->root(), lines );
    }
      
  }
  return true;
}

/*!
  \brief Remove text line.
  \param id line identifier
  \return \c true on success or \c false if \a id is invalid
*/
bool LIGHTGUI_DataModel::deleteTextLine( const int id )
{
  LightApp_DataObject* modelRoot = dynamic_cast<LightApp_DataObject*>( root() );

  if ( !modelRoot )
    return false;

  LIGHTGUI_DataObject* obj = findObject( id );
  if ( !obj || !obj->parent() )
    return false;
  
  if ( obj->lineText().trimmed().isEmpty() ) {
    // if paragraph : put all child lines to the previous paragraph
    SUIT_DataObject* newParent = obj->prevBrother();
    if ( newParent ) {
      while ( SUIT_DataObject* first = obj->firstChild() )
	first->setParent( newParent );
    }
  }
  obj->parent()->removeChild( obj );

  return true;
}

/*!
  \brief Remove all lines.
*/
void LIGHTGUI_DataModel::clearAll()
{
  CAM_Study* study = (CAM_Study*)module()->application()->activeStudy();
  buildTree( study->root(), QStringList() );
}

/*!
  \brief Getnerate unique line identifier.
*/
int LIGHTGUI_DataModel::generateId()
{
  return ++myMaxId;
}

/*!
  \brief Get line identifier from the object entry.
  \param entry object entry
  \return object ID or -1 if \a entry is invalid
*/
int LIGHTGUI_DataModel::id( const QString& entry )
{
  int id = -1;
  QStringList ids = entry.split( ":", QString::SkipEmptyParts );
  if ( ids.count() == 2 && ids[0] == "LIGHTGUI" ) {
    bool bOk;
    int p = ids[1].toInt( &bOk );
    if ( bOk ) 
      id = p;
  }
  return id;
}

/*!
  \brief Get object entry for the line with sepcified identifier.
  \param id object ID
  \return object entry corresponding to the given \a id
*/
QString LIGHTGUI_DataModel::entry( const int id )
{
  QString entry;
  if ( id > 0 )
    entry = QString( "LIGHTGUI:%1" ).arg( id );
  return entry;
}

/*!
  \brief Get line number for the object with the specified entry.
  \param entry object entry
  \return line number or 0 if \a entry is invalid
*/
int LIGHTGUI_DataModel::lineNb( const QString& entry )
{
  return lineNb( LIGHTGUI_DataModel::id( entry ) );
}

/*!
  \brief Get line number for the specified identifier.
  \param id line identifier
  \return line number or 0 if \a id is invalid
*/
int LIGHTGUI_DataModel::lineNb( const int id )
{
  LIGHTGUI_DataObject* obj = findObject( id );
  return obj ? obj->lineNb() : 0;
}

/*!
  \brief Get all existing lines identifiers.
  \return list of lines identifiers
*/
QList<int> LIGHTGUI_DataModel::getIds() const
{
  QList<int> l;
  for ( SUIT_DataObjectIterator it( root(), SUIT_DataObjectIterator::DepthLeft ); it.current(); ++it ) {
    LIGHTGUI_DataObject* obj = dynamic_cast<LIGHTGUI_DataObject*>( it.current() );
    if ( obj )
      l.append( obj->id() );
  }
  return l;
}

/*!
  \brief Rebuild data tree.
  \param studyRoot study root data object
  \param lines string data
*/
void LIGHTGUI_DataModel::buildTree( SUIT_DataObject* studyRoot, const QStringList& lines )
{
  if ( !studyRoot )
    return;

  CAM_ModuleObject* modelRoot = createModuleObject( studyRoot );
  LIGHTGUI_DataObject* aParaObject;
  LIGHTGUI_DataObject* aLineObject;

  aParaObject = new LIGHTGUI_DataObject ( generateId(), "", modelRoot );

  QStringList::ConstIterator it1 = lines.begin(),
                             it2 = lines.end();
  for ( ; it1 != it2; ++it1 ) {
    if ( (*it1).trimmed().isEmpty() ) {
      aParaObject = new LIGHTGUI_DataObject ( generateId(), *it1, modelRoot );
    }
    else {
      aLineObject = new LIGHTGUI_DataObject ( generateId(), *it1, aParaObject );
    }
  }

  modelRoot->setDataModel( this );
  setRoot( modelRoot );
}

/*!
  \brief Search data object corresponding to the specified line number.
  \param id line identifier
  \return data object or 0 if \a id is invalid
*/
LIGHTGUI_DataObject* LIGHTGUI_DataModel::findObject( const int id ) const
{
  for ( SUIT_DataObjectIterator it( root(), SUIT_DataObjectIterator::DepthLeft ); it.current(); ++it ) {
    LIGHTGUI_DataObject* obj = dynamic_cast<LIGHTGUI_DataObject*>( it.current() );
    if ( obj && obj->id() == id )
      return obj;
  }
  return 0;
}
